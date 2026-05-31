#include "fft.h"
#include <complex.h>
#include <fftw3.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h> // ??

// Hann windowing function -> to reduce spectral leakage ???????
double hann_window(double n, uint num_samples) {
  return (1 - cos(2 * M_PI * n / num_samples)) / 2;
}

// calculate euclidean distance from complex number
double complex_magnitude(complex double num) {
  return sqrt(creal(num) * creal(num) + cimag(num) * cimag(num));
}

// convert the discrete, real signal into frequency domain
SpectrumPoint *to_spectrum(double *data, uint n_samples, uint f_sampling) {
  fftw_complex *dft_out;
  double *windowed_data;
  SpectrumPoint *points = NULL;
  fftw_plan p;
  const int n_freq = n_samples / 2 + 1;

  // TODO Log?
  if (!data) {
    printf("Error, no data provided for spectrum calculation\n");
    return NULL;
  }

  // allocate memory
  dft_out = fftw_malloc(sizeof(fftw_complex) * n_freq);
  points = calloc(n_freq, sizeof(SpectrumPoint));
  windowed_data = calloc(n_samples, sizeof(double));

  if (!dft_out || !points || !windowed_data) {
    printf("Error allocating memory for spectrum\n");
    goto exit;
  }

  // TODO efficiency
  // apply windowing function
  for (int i = 0; i < n_samples; i++) {
    windowed_data[i] = data[i] * hann_window(i, n_samples);
  }

  // prepair an fft plan, estimating optimisation parameter in the run
  p = fftw_plan_dft_r2c_1d(n_samples, windowed_data, dft_out, FFTW_ESTIMATE);

  // execute the plan and calculate DFT complex values
  fftw_execute(p);
  // assert out != empty, if everything went right

  // TODO efficiency?
  // compute frequencies and actual magnitude
  for (int k = 0; k < n_freq; k++) {
    points[k].frequency = (double)k * f_sampling / n_samples;
    // because the fft return complex number, actual magnitude needs to be
    // calculated (euclidean distance) and normalised for the hann window ???
    points[k].magnitude =
        complex_magnitude(dft_out[k]) / ((double)n_samples * 1 / 2);

    // apply appropriate scaling for ??? TODO
    if (k > 0 && k < n_samples / 2)
      points[k].magnitude *= 2.0;
  }


exit:
  // free self managed memory
  fftw_destroy_plan(p);
  free(windowed_data);
  fftw_free(dft_out);
  return points;
}
