#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
typedef struct {
  // RIFF Header chunk
  char file_type_bloc_id[4]; //! 'RIFF'
  uint32_t file_size;        //! overall file size - 8 bytes
  char file_format_id[4];    //! e.g.: 'WAVE'
  // chunk describing data format
  char format_bloc_id[4]; //! 'fmt '
  uint32_t bloc_size;     //! chunk size - 8 bytes
  uint16_t audio_format;  //! 1: PCM integer, 3: IEEE 754 float
  uint16_t num_channels;  //! number of channels
  uint32_t frequency;     //! sampling rate
  uint32_t byte_per_sec;  //! frequency * byte_per_bloc
  uint16_t byte_per_bloc; //! num_channels * byte_per_sample
  uint16_t byte_per_sample;
  // chunk of sample data
  char data_block_id[4]; //! 'data'
  uint32_t data_size;    //! size of the sample in bytes
} RIFF_Header;

union RIFF_union {
  RIFF_Header header;
  char array[44];
};

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
  printf("Bytes per Sample  : %u\n", h->byte_per_sample);

  printf("\n--- Data Chunk ---\n");
  printf("Data ID           : %.4s\n", h->data_block_id);
  printf("Data Size         : %u bytes\n", h->data_size);
}

/*! \brief read the file and parse the header for a RIFF file
 *
 *  \param [in] path path of the RIFF file
 *  \param [out] out_header parsed header (first 44 bytes)
 *  \param [out] out_sample_data the read samples from the file -> gets
 * allocated allocated
 *
 *  \ret the written size to out_sample_data on no error and -1 for a general
 * error (if the file could not open TODO, header not allocated)
 */
int parse_RIFF_file(const char *path, RIFF_Header *out_header,
                    uint8_t *out_sample_data) {
  FILE *file;
  union RIFF_union riff;

  if (out_header == NULL) {
    printf("RIFF_Header struct not allocated!\n");
    return -1;
  }
  // assert RIFF_Header allocated

  file = fopen(path, "r");

  if (file == NULL) {
    return -1;
  }
  // assert file open

  if (fgets(riff.array, sizeof(RIFF_Header), file) == NULL) {
    printf("Error reading RIFF Header!\n");
    return -1;
  }
  *out_header = riff.header;

  fclose(file);
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc > 1) {
    RIFF_Header header;

    int size = parse_RIFF_file(argv[1], &header, NULL);
    printf("return value: %i\n", size);
    print_riff_header(&header);

  } else {
    printf("Usage: %s PATH_TO_WAV_FILE\n", argv[0]);
  }
}
