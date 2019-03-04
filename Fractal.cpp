//
// Created by fader on 04.03.19.
//

#include <complex>
#include "Fractal.hpp"

using namespace mandelbroet;

mandelbroet::Fractal::~Fractal() {}

static inline Real scale(Real value, Real max_value, Real dst_min, Real dst_max) {
  return dst_min + ((dst_max - dst_min) * value) / max_value;
}

static inline Real scale(Real value, Real min_value, Real max_value, Real dst_min, Real dst_max) {
  return dst_min + ((dst_max - dst_min) * value) / (max_value - min_value);
}

static Colour colour_pallete(unsigned max_N, unsigned N) {
  N = std::min(N, max_N);

  Real score = Real(N) / max_N;

  const Colour C_BAD  = Colour::rgb(255, 0, 0);
  const Colour C_OK   = Colour::rgb(0, 255, 0);
  const Colour C_GOOD = Colour::rgb(0, 0, 255);
  const Colour C_BEST = Colour::rgb(255, 0, 255);

  const Real BAD  = 0.25;
  const Real GOOD = 0.75;
  const Real BEST = 0.90;

  if (score <= BAD) {
    return Colour::lerp(C_BAD, C_OK, scale(score, 0, BAD, 0, 1));
  }

  if (score <= GOOD) {
    return Colour::lerp(C_OK, C_GOOD, scale(score, BAD, GOOD, 0, 1));
  }

  if (score < BEST) {
    return Colour::lerp(C_GOOD, C_BEST, scale(score, GOOD, BEST, 0, 1));
  }

  return BLACK;
}

template<typename Parameter, unsigned(escape_time)(int, Parameter, Real, Real)>
static void draw(int width, int height, Colour *pixels, Real zoom, Real x_pos, Real y_pos, int max_escape_time, Parameter parameter) {
  const Real mandelbrot_min_x = -3.5 * zoom + x_pos;
  const Real mandelbrot_max_x = +3.5 * zoom + x_pos;

  const Real mandelbrot_min_y = -3.5 * zoom + y_pos;
  const Real mandelbrot_max_y = +3.5 * zoom + y_pos;

  #pragma omp parallel for collapse(2)
  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      /// scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.5, 1))
      const Real x0 = scale(x, 0, width, mandelbrot_min_x, mandelbrot_max_x);

      /// scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1, 1))
      const Real y0 = scale(y, 0, height, mandelbrot_min_y, mandelbrot_max_y);

      const unsigned N = escape_time(max_escape_time, parameter, x0, y0);

      pixels[y * width + x] = colour_pallete(max_escape_time, N);
    }
  }
}

static inline unsigned mandelbrot_escape_time(int max_escape_time, int dummy, Real x, Real y) {
  (void) dummy;

  std::complex<Real> Z{0, 0};
  std::complex<Real> C{x, y};

  int iteration = 0;

  for (; (iteration < max_escape_time) and (std::norm(Z) <= 2*2); iteration++) {
    Z = Z * Z + C;
  }

  return iteration;
}

void Mandelbrot::draw(unsigned width, unsigned height, Colour *pixels,
                      Real zoom, Real x_pos, Real y_pos) {
 ::draw<int, mandelbrot_escape_time>(
   width, height, pixels, zoom, x_pos, y_pos, _current_max_escape_time, _current_max_escape_time
 );
}


static inline unsigned multibrot_escape_time(int max_escape_time, Real d, Real x, Real y) {
  std::complex<Real> Z{0, 0};
  std::complex<Real> C{x, y};

  int iteration = 0;

  for (; (iteration < max_escape_time) and (std::norm(Z) <= 2*2); iteration++) {
    Z = std::pow(Z, d) + C;
  }

  return iteration;
}

void Multi_Brot::draw(unsigned width, unsigned height, Colour *pixels,
                      Real zoom, Real x_pos, Real y_pos) {
  ::draw<Real, multibrot_escape_time>(
    width, height, pixels, zoom, x_pos, y_pos, max_escape_time(), d
  );
}