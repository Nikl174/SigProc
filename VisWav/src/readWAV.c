#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
typedef struct {
  // ---- RIFF Header chunk ----
  //! 'RIFF'
  char file_type_bloc_id[4];
  //! overall file size - 8 bytes
  uint32_t file_size;
  //! e.g.: 'WAVE'
  char file_format_id[4];
  // ---- chunk describing data format ----
  //! 'fmt '
  char format_bloc_id[4];
  //! chunk size - 8 bytes
  uint32_t bloc_size;
  //! 1: PCM integer, 3: IEEE 754 float
  uint16_t audio_format;
  //! number of channels
  uint16_t num_channels;
  //! sampling rate
  uint32_t frequency;
  //! frequency * byte_per_bloc
  uint32_t byte_per_sec;
  //! num_channels * bits_per_sample
  uint16_t byte_per_bloc;
  uint16_t bits_per_sample;
  // --- chunk of sample data ---
  //! 'data'
  char data_block_id[4];
  //! size of the sample in bytes
  uint32_t data_size;
} RIFF_Header;

union RIFF_union {
  RIFF_Header header;
  char array[44];
};

// TODO
uint8_t get_bit_bytes(uint16_t bits) {
  if (bits <= 8) {
    return 2;
  }
  if (bits <= 16) {
    return 4;
  }
  if (bits <= 32) {
    return 8;
  }
  return 0;
}

void print_riff_header(const RIFF_Header *h) {
  printf("=== RIFF Header ===\n");

  printf("File Type ID      : %.4s\n", h->file_type_bloc_id);
  printf("File Size         : %u bytes\n", h->file_size);
  printf("Format            : %.4s\n", h->file_format_id);

  printf("\n--- Format Chunk ---\n");
  printf("Format ID         : %.4s\n", h->format_bloc_id);
  printf("Chunk Size        : %u\n", h->bloc_size);
  printf("Audio Format      : %u (%s)\n", h->audio_format,
         (h->audio_format == 1)   ? "PCM"
         : (h->audio_format == 3) ? "IEEE Float"
                                  : "Other");
  printf("Channels          : %u\n", h->num_channels);
  printf("Sample Rate       : %u Hz\n", h->frequency);
  printf("Bytes per Second  : %u\n", h->byte_per_sec);
  printf("Bytes per Block   : %u\n", h->byte_per_bloc);
  printf("Bits per Sample  : %u\n", h->bits_per_sample);

  printf("\n--- Data Chunk ---\n");
  printf("Data ID           : %.4s\n", h->data_block_id);
  printf("Data Size         : %u bytes\n", h->data_size);
}

/*! \brief read the file and parse the header for a RIFF file
 *
 *  \param [in] path path of the RIFF file
 *  \param [out] out_header parsed header (first 44 bytes)
 *  \param [out] out_num_samples the number of samples read
 *
 *  \ret the pointer to the samples array and NULL on a general
 * error (if the file could not open TODO, header not allocated)
 */
int32_t *parse_RIFF_file(const char *path, RIFF_Header *out_header,
                         int *out_num_samples) {
  FILE *file;
  union RIFF_union riff;
  int32_t *samples = NULL;

  if (out_header == NULL) {
    printf("RIFF_Header struct not allocated!\n");
    return NULL;
  }
  // assert RIFF_Header allocated

  file = fopen(path, "r");

  if (file == NULL) {
    return NULL;
  }
  // assert file open

  if (fgets(riff.array, sizeof(RIFF_Header), file) == NULL) {
    printf("Error reading RIFF Header!\n");
    return NULL;
  }
  *out_header = riff.header;

  {
    // TODO check header field if it is a WAV file
    // TODO malloc?
    uint32_t data_size = riff.header.data_size;
    uint8_t byte_size = get_bit_bytes(riff.header.bits_per_sample);
    if (byte_size == 0) {
      printf("Error in order calculation\n");
      return NULL;
    }
    int num_of_samples = data_size / byte_size;
    samples = calloc(num_of_samples, byte_size);
    if (out_num_samples == NULL) {
      return NULL;
    }
    // assert out_num_samples != NULL

    *out_num_samples = fread(samples, byte_size, num_of_samples, file);
    if (*out_num_samples != num_of_samples) {
      printf("Error reading samples, did not read '%d' num_of_samples "
             "bytes\nRead: %d\n",
             num_of_samples, *out_num_samples);
      return NULL;
    }
  }

  fclose(file);
  return samples;
}


// Convert data coordinates to screen coordinates
void to_screen(float x, float y, int *sx, int *sy, float xmin, float xmax,
               float ymin, float ymax) {
  *sx = (int)((x - xmin) / (xmax - xmin) * WINDOW_WIDTH);
  *sy = (int)(WINDOW_HEIGHT - (y - ymin) / (ymax - ymin) * WINDOW_HEIGHT);
}

int main(int argc, char *argv[]) {

  RIFF_Header header;
  int read_samples = 0;
  int x_max_divisor = 400;

  int32_t *samples;
  if (argc > 1) {
    samples = parse_RIFF_file(argv[1], &header, &read_samples);
    print_riff_header(&header);
    printf("-----------------------\n");
    printf("Read samples: %i\n", read_samples);

  } else {
    printf("Usage: %s PATH_TO_WAV_FILE\n", argv[0]);
    return 1;
  }

  // SDL INIT
  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window *window = SDL_CreateWindow("SDL3 XY Plot", WINDOW_WIDTH,
                                        WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);

  SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

  bool running = true;
  SDL_Event event;

  // Plot bounds
  float xmin = 0, xmax = header.frequency * header.num_channels / x_max_divisor;
  float ymin = -INT32_MAX, ymax = INT32_MAX;

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT)
        running = false;

      // zoom into plot
      if (event.type == SDL_EVENT_MOUSE_WHEEL) {
        float zoomFactor = (event.wheel.y > 0) ? 0.9f : 1.1f;
        x_max_divisor *= zoomFactor;
        xmax = header.frequency * header.num_channels / x_max_divisor;
      }
    }

    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderClear(renderer);

    // Draw axes
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderLine(renderer, 0, WINDOW_HEIGHT - 1, WINDOW_WIDTH,
                   WINDOW_HEIGHT - 1);                // X axis
    SDL_RenderLine(renderer, 0, 0, 0, WINDOW_HEIGHT); // Y axis

    // Draw data points
    SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);

    for (int i = 0; i < xmax; i += header.num_channels) {
      int x1, y1, x2, y2;
      to_screen(i, samples[i], &x1, &y1, xmin, xmax, ymin, ymax);
      to_screen(i, 0, &x2, &y2, xmin, xmax, ymin, ymax);

      SDL_RenderLine(renderer, x1, y1, x2, y2);
    }

    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
