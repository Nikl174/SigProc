#include "read_wav.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

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
    printf("Byte size: %d\n", byte_size);
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
