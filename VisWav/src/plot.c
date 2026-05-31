#include "fft.h"
#include "read_wav.h"
#include <SDL3/SDL.h>
#include <getopt.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 900

// Convert data coordinates to screen coordinates
void to_screen(float x, float y, int *sx, int *sy, float xmin, float xmax,
               float ymin, float ymax) {
  *sx = (int)((x - xmin) / (xmax - xmin) * WINDOW_WIDTH);
  *sy = (int)(WINDOW_HEIGHT - (y - ymin) / (ymax - ymin) * WINDOW_HEIGHT);
}

void plot_points(SDL_Renderer *renderer, int32_t *samples, uint8_t channels,
                 int width, int heigth, float xmin, float xmax, float ymin,
                 float ymax) {

  // Draw axes
  SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
  SDL_RenderLine(renderer, 0, heigth - 1, width,
                 heigth - 1);                // X axis
  SDL_RenderLine(renderer, 0, 0, 0, heigth); // Y axis

  // Draw data points
  SDL_SetRenderDrawColor(renderer, 255, 100, 100, 255);

  for (int i = 0; i < xmax; i += channels) {
    int x1, y1, x2, y2;
    to_screen(i, 0, &x1, &y1, xmin, xmax, ymin, ymax);
    // swap endianes of the data for wav files
    to_screen(i, samples[i], &x2, &y2, xmin, xmax, ymin, ymax);

    SDL_RenderLine(renderer, x1, y1, x2, y2);
  }
}

void sdl_points_plot(int32_t *points, uint size, uint channels) {

  // SDL INIT
  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window *window = SDL_CreateWindow("Wave visualisation", WINDOW_WIDTH,
                                        WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE);

  SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

  bool running = true;
  SDL_Event event;

  // Plot bounds
  float xmin = 0, xmax = size / 2;
  float ymin = -INT32_MAX - 1, ymax = INT32_MAX;

  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT)
        running = false;

      // zoom into plot
      if (event.type == SDL_EVENT_MOUSE_WHEEL) {
        float zoomFactor = (event.wheel.y > 0) ? 0.9f : 1.1f;
        xmax *= zoomFactor;
        if (xmax >= size) {
          xmax = size - 1;
        }
      }
    }

    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderClear(renderer);

    plot_points(renderer, points, channels, WINDOW_WIDTH, WINDOW_HEIGHT, xmin,
                xmax, ymin, ymax);

    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void print_help(char *argv[]) {
  printf("Prints the WAV file samples\n");
  printf("Usage: %s [OPTIONS] PATH_TO_WAV_FILE\n", argv[0]);
  printf("OPTIONS:\n");
  printf("\t-s/--spectrum:\tplot the spectrum instead \n");
  printf("\t-h/--help:\tprint this help\n");
  printf("\t-v/--verbose:\tprint additional info \n");
  printf("\t-p/--print:\tprint plotted values\n");
}

int main(int argc, char *argv[]) {

  RIFF_Header header;
  int read_samples = 0;
  uint16_t x_max_divisor = 400;
  int32_t *samples;
  int32_t *spectrum;
  int fft_samples_num = 880;
  bool use_spectrum = false;
  bool print_values = false;
  bool verbose = false;

  int choice;
  while (1) {
    static struct option long_options[] = {
        /* Use flags like so:
        {"verbose", no_argument,  &verbose_flag, 'V'}*/
        /* Argument styles: no_argument, required_argument, optional_argument */
        {"verbose", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'h'},
        {"spectrum", no_argument, 0, 's'},
        {"print", no_argument, 0, 'p'},
        {0, 0, 0, 0}};

    int option_index = 0;

    /* Argument parameters:
      no_argument: " "
      required_argument: ":"
      optional_argument: "::" */

    choice = getopt_long(argc, argv, "vhsp", long_options, &option_index);

    if (choice == -1)
      break;

    switch (choice) {
    case 'v':
      verbose = true;
      break;

    case 'h':
      print_help(argv);
      return EXIT_FAILURE;
      break;

    case 's':
      use_spectrum = true;
      break;

    case 'p':
      print_values = true;
      break;

    case '?':
      /* getopt_long will have already printed an error */
      break;

    default:
      /* Not sure how to get here... */
      return EXIT_FAILURE;
    }
  }

  /* Deal with non-option arguments here */
  if (optind < argc) {
    samples = parse_RIFF_file(argv[optind], &header, &read_samples);
    fft_samples_num = header.frequency / 2;
    if (verbose)
      print_riff_header(&header);
    if (verbose)
      printf("-----------------------\n");
    if (verbose)
      printf("Read samples: %i\n", read_samples);
    if (!(samples && read_samples > fft_samples_num)) {
      printf("Error reading samples: %x, fft_samples_num: %d\n", samples,
             fft_samples_num);
      return 1;
    }
    // TODO
    double *d_samples = calloc(sizeof(double), fft_samples_num);
    spectrum = calloc(sizeof(int32_t), fft_samples_num);
    for (int i = 0; i < fft_samples_num; i++) {
      d_samples[i] = (double)samples[i];
      if (print_values)
        printf("Sample %i: %d\n", i, samples[i]);
    }

    if (use_spectrum) {
      SpectrumPoint *points =
          to_spectrum(d_samples, fft_samples_num, header.frequency);

      if (!points) {
        printf("Error while calculating the spectrum!\n");
        return 1;
      }

      for (int i = 0; i < fft_samples_num / 2; i++) {
        spectrum[i] = points[i].magnitude;
        if (print_values)
          printf("Mag: %f;\tFreq: %f;\n", points[i].magnitude,
                 points[i].frequency);
      }
      sdl_points_plot(spectrum, fft_samples_num, 1);
    } else {
      sdl_points_plot(samples, fft_samples_num, header.num_channels);
    }

    free(d_samples);
  } else {
    printf("Error: missing file!\n");
    print_help(argv);
    return EXIT_FAILURE;
  }

  free(spectrum);
  free(samples);
  return EXIT_SUCCESS;
}
