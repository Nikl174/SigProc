#include <complex.h>
#include <sys/types.h>
/*! \struct SpectrumPoint
 *  \brief point of a spectrum containing frequency and magnitude
 */
typedef struct {
  double magnitude;
  double frequency;
} SpectrumPoint;

SpectrumPoint *to_spectrum(double *data, uint n_samples, uint f_sampling);
