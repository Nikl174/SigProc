#include <stdint.h>

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

/*! \brief Fancy print header
 * \param h header to print
 */
void print_riff_header(const RIFF_Header *h);

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
                         int *out_num_samples);
