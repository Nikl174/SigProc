#include "read_wav.h"
#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 800

// Convert data coordinates to screen coordinates
void to_screen(float x, float y, int *sx, int *sy, float xmin, float xmax,
               float ymin, float ymax) {
  *sx = (int)((x - xmin) / (xmax - xmin) * WINDOW_WIDTH);
  *sy = (int)(WINDOW_HEIGHT - (y - ymin) / (ymax - ymin) * WINDOW_HEIGHT);
}

void plot_wav_file(SDL_Renderer *renderer, int32_t *samples, uint8_t channels,
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

int main(int argc, char *argv[]) {

  RIFF_Header header;
  int read_samples = 0;
  uint16_t x_max_divisor = 400;
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
        xmax *= zoomFactor;
        if (xmax >= read_samples) {
          xmax = read_samples - 1;
        }
      }
    }

    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderClear(renderer);

    plot_wav_file(renderer, samples, header.num_channels, WINDOW_WIDTH,
                  WINDOW_HEIGHT, xmin, xmax, ymin, ymax);

    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }

  free(samples);
  samples = NULL;

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
