
#include <iostream>    /// for std::cout
#include <cassert>     /// for assert
#include <utility>     /// for std::swap
#include <type_traits> /// for std::is_base_of

#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_render.h>
#include <bits/unique_ptr.h>
#include <cstring>
#include <complex>

using Real = float;

constexpr const size_t TEXTURE_WIDTH  = 1 << 10;
constexpr const size_t TEXTURE_HEIGHT = 1 << 10;

inline Real scale(Real value, Real src_min, Real src_max, Real dst_min, Real dst_max) {
  return dst_min + ((dst_max - dst_min) * value) / (src_max - src_min);
}

struct Colour {
  Colour() = default;
  constexpr Colour(unsigned char R, unsigned char G, unsigned char B, unsigned char A)
  : A{A}, B{B}, G{G}, R{R} {}

  static constexpr Colour rgb(unsigned char R, unsigned char G, unsigned char B) {
    return rgba(R, G, B, SDL_ALPHA_OPAQUE);
  }
  static constexpr Colour rgba(unsigned char R, unsigned char G, unsigned char B, unsigned char A) {
    return Colour{R, G, B, A};
  }

  static Colour lerp(Colour v0, Colour v1, Real t) {
    return rgba(
      lerp(v0.R, v1.R, t),
      lerp(v0.G, v1.G, t),
      lerp(v0.B, v1.B, t),
      lerp(v0.A, v1.A, t)
    );
  }

  // Precise method, which guarantees v = v1 when t = 1.
  static unsigned char lerp(unsigned char v0, unsigned char v1, Real t) {
    return (1 - t) * v0 + t * v1;
  }

  unsigned char A;
  unsigned char B;
  unsigned char G;
  unsigned char R;
};

inline static const Colour WHITE = Colour::rgb(255, 255, 255);
inline static const Colour BLACK = Colour::rgb(0, 0, 0);
inline static const Colour RED   = Colour::rgb(255, 0, 0);
inline static const Colour GREEN = Colour::rgb(0, 255, 0);

template<size_t Width, size_t Height>
struct Image {
  Colour &operator()(unsigned x, unsigned y) {
    assert(x < Width);
    assert(y < Height);

    return pixels[y * Width + x];
  }

  void clear(Colour C) {
    pixels.fill(C);
  }

  unsigned width() const { return Width; }
  unsigned height() const { return Height; }

  const void *data() const { return pixels.data(); }
private:
  std::array<Colour, Width * Height> pixels;
};

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

template<typename Sub_Class>
struct Fractal {
  Fractal() {
    static_assert(std::is_base_of_v<Fractal, Sub_Class>);

    static_assert(std::is_same_v<decltype(&Sub_Class::_name), std::string (Sub_Class::*)() const>);
    static_assert(std::is_same_v<decltype(&Sub_Class::_parameter), std::string (Sub_Class::*)() const>);

    static_assert(std::is_same_v<decltype(&Sub_Class::_step_parameter), void (Sub_Class::*)()>);
    static_assert(std::is_same_v<decltype(&Sub_Class::_max_iterations), unsigned (Sub_Class::*)() const>);

    static_assert(std::is_same_v<decltype(&Sub_Class::_escape_time), unsigned (Sub_Class::*)(Real, Real) const>);
  }

  template<size_t Width, size_t Height>
  void draw(Image<Width, Height> &pixels, Real zoom, Real x_pos, Real y_pos) {
    const unsigned MAX_X = pixels.width();
    const unsigned MAX_Y = pixels.height();

    const Real mandelbrot_min_x = -3.5 * zoom + x_pos;
    const Real mandelbrot_max_x = +3.5 * zoom + x_pos;

    const Real mandelbrot_min_y = -3.5 * zoom + y_pos;
    const Real mandelbrot_max_y = +3.5 * zoom + y_pos;

    #pragma omp parallel for collapse(2)
    for (unsigned x = 0; x < MAX_X; x++) {

      for (unsigned y = 0; y < MAX_Y; y++) {
        /// scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.5, 1))
        const Real x0 = scale(x, MAX_X, mandelbrot_min_x, mandelbrot_max_x);

        /// scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1, 1))
        const Real y0 = scale(y, MAX_Y, mandelbrot_min_y, mandelbrot_max_y);

        const unsigned N = escape_time(x0, y0);

        pixels(x, y) = colour_pallete(self().max_iterations(), N);
      }
    }
  }

  void step_parameter() {
    return self()._step_parameter();
  }

  unsigned max_iterations() const {
    return self()._max_iterations();
  }

  std::string name() const {
    return self()._name();
  }

  std::string parameter() const {
    return self()._parameter();
  }

  unsigned escape_time(Real x, Real y) const {
    return self()._escape_time(x, y);
  }
private:
  static Real scale(Real value, Real max_value, Real dst_min, Real dst_max) {
    return dst_min + ((dst_max - dst_min) * value) / max_value;
  }

  Sub_Class &self() { return *static_cast<Sub_Class*>(this); }
  const Sub_Class &self() const { return *static_cast<const Sub_Class*>(this); }
};


struct Mandelbrot : Fractal<Mandelbrot> {
  unsigned _escape_time(Real x, Real y) const {
    std::complex<Real> Z{0, 0};
    std::complex<Real> C{x, y};

    int iteration = 0;

    for (; (iteration < max_iterations()) and (std::norm(Z) <= 2*2); iteration++) {
      Z = Z * Z + C;
    }

    return iteration;
  }

  unsigned _max_iterations() const {
    return _current_max_iterations;
  }

  void _step_parameter() {
    _current_max_iterations++;
    if (_current_max_iterations > max_max_iterations) { _current_max_iterations = 0; }
  }

  std::string _name() const { return "Mandelbrot"; }

  std::string _parameter() const {
    return "max_iterations=" + std::to_string(max_iterations());
  }
private:
  unsigned _current_max_iterations = 0;
  static inline const unsigned max_max_iterations = 1024;
};

struct Multi_Brot : Fractal<Multi_Brot> {
  unsigned _escape_time(Real x, Real y) {
    std::complex<Real> Z{0, 0};
    std::complex<Real> C{x, y};

    int iteration = 0;

    for (; (iteration < max_iterations()) and (std::norm(Z) <= 2*2); iteration++) {
      const Real d = Real(max_iterations()) / Real(32.0);

      Z = std::pow(Z, d) + C;
    }

    return iteration;
  }

  void _step_parameter() {
    d += 0.1;
    if (d > max_d) { d = min_d; }
  }

  unsigned _max_iterations() const {
    return 64;
  }

  std::string _name() const { return "Multibrot"; }

  std::string _parameter() const {
    return "d=" + std::to_string(d);
  }
private:
  Real d = min_d;
  const Real min_d = 0.5;
  const Real max_d = 5.0;
};

int main(int argc, const char **argv) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
    return 1;
  }

  Mandelbrot fractal;

  const unsigned WIN_WIDTH  = 640;
  const unsigned WIN_HEIGHT = 480;

  const uint32_t window_flags = SDL_WINDOW_RESIZABLE; // | SDL_WINDOW_MAXIMIZED;

  SDL_Window *window = SDL_CreateWindow(
    fractal.name().c_str(),
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    WIN_WIDTH,
    WIN_HEIGHT,
    window_flags
  );

  const int      renderer_driver_index = -1; /// take any valid renderer driver
  const uint32_t renderer_flags        = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;

  SDL_Renderer *renderer = SDL_CreateRenderer(
    window,
    renderer_driver_index,
    renderer_flags
  );

  const unsigned TEXTURE_WIDTH  = 1 << 12;
  const unsigned TEXTURE_HEIGHT = 1 << 12;

  SDL_Texture *texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGBA8888,
    SDL_TEXTUREACCESS_TARGET,
    TEXTURE_WIDTH, TEXTURE_HEIGHT
  );

  auto *pixels = new Image<TEXTURE_WIDTH, TEXTURE_HEIGHT>();
  pixels->clear(BLACK);

  const uint64_t performance_frequency = SDL_GetPerformanceFrequency();

  Real zoom = 1.0;
  Real x_pos = 0.0;
  Real y_pos = 0.0;

  const Real x_speed = 0.1;
  const Real y_speed = 0.1;
  const Real zoom_speed = 1.25;
  const Real zoom_in_speed = 1 / zoom_speed;
  const Real zoom_out_speed = zoom_speed;

  bool move_left = false, move_right = false;
  bool move_up = false, move_down = false;
  bool zoom_in = false, zoom_out = false;

  SDL_Event event;
  bool running = true;
  while (true) {
    const uint64_t start = SDL_GetPerformanceCounter();

    /// ***** handle events

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    while(SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          goto quit;
        case SDL_KEYDOWN:
          switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_ESCAPE:
              goto quit;
            case SDL_SCANCODE_LEFT:
            case SDL_SCANCODE_KP_4:
            case SDL_SCANCODE_A:
              move_left = true;
              break;
            case SDL_SCANCODE_RIGHT:
            case SDL_SCANCODE_KP_6:
            case SDL_SCANCODE_D:
              move_right = true;
              break;
            case SDL_SCANCODE_UP:
            case SDL_SCANCODE_KP_8:
            case SDL_SCANCODE_W:
              move_up = true;
              break;
            case SDL_SCANCODE_DOWN:
            case SDL_SCANCODE_KP_2:
            case SDL_SCANCODE_S:
              move_down = true;
              break;
            case SDL_SCANCODE_KP_PLUS:
            case SDL_SCANCODE_E:
            case SDL_SCANCODE_PAGEUP:
              zoom_in = true;
              break;
            case SDL_SCANCODE_KP_MINUS:
            case SDL_SCANCODE_Q:
            case SDL_SCANCODE_PAGEDOWN:
              zoom_out = true;
              break;
          }
          break;
        case SDL_KEYUP:
          switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_LEFT:
            case SDL_SCANCODE_KP_4:
            case SDL_SCANCODE_A:
              move_left = false;
              break;
            case SDL_SCANCODE_RIGHT:
            case SDL_SCANCODE_KP_6:
            case SDL_SCANCODE_D:
              move_right = false;
              break;
            case SDL_SCANCODE_UP:
            case SDL_SCANCODE_KP_8:
            case SDL_SCANCODE_W:
              move_up = false;
              break;
            case SDL_SCANCODE_DOWN:
            case SDL_SCANCODE_KP_2:
            case SDL_SCANCODE_S:
              move_down = false;
              break;
            case SDL_SCANCODE_KP_PLUS:
            case SDL_SCANCODE_E:
            case SDL_SCANCODE_PAGEUP:
              zoom_in = false;
              break;
            case SDL_SCANCODE_KP_MINUS:
            case SDL_SCANCODE_Q:
            case SDL_SCANCODE_PAGEDOWN:
              zoom_out = false;
              break;
          }
          break;
        quit:
          running = false;
          break;
      }
    }

    if (!running) { break; }

    if (move_left) {
      std::cout << "LEFT\n";
      x_pos -= x_speed * zoom;
    }
    if (move_right) {
      x_pos += x_speed * zoom;
    }
    if (move_up) {
      y_pos -= y_speed * zoom;
    }
    if (move_down) {
      y_pos += x_speed * zoom;
    }
    if (zoom_in) {
      std::cout << "ZOOM IN\n";
      zoom *= zoom_in_speed;
    }
    if (zoom_out) {
      std::cout << "ZOOM IN\n";
      zoom *= zoom_out_speed;
    }

    /// ***** calculate fractal

    fractal.draw(*pixels, zoom, x_pos, y_pos);

    fractal.step_parameter();

    /// ***** set window title

    std::string title = fractal.name() + "! (" + fractal.parameter() + ") (x="
                        + std::to_string(x_pos) + ", y=" + std::to_string(y_pos)
                        + ", zoom=" + std::to_string(zoom) + ")";

    SDL_SetWindowTitle(window, title.c_str());

    /// ***** render fractal

    /// pitch ... the number of bytes in a row of pixel data, including padding between lines
    const int pitch = pixels->width() * sizeof(Colour);

    SDL_UpdateTexture(
      texture,
      NULL,
      pixels->data(),
      pitch
    );

    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    /// ***** control frame rate
    {
      const uint64_t end           = SDL_GetPerformanceCounter();
      const double   seconds       = (end - start) / double(performance_frequency);
      const double   milli_seconds = seconds * 1000;

      const unsigned frames_per_second  = 15;
      const double   target_frame_time = (1.0 / frames_per_second) * 1000;

      if (milli_seconds < target_frame_time) {
        SDL_Delay(target_frame_time - milli_seconds);
      } else if (milli_seconds > target_frame_time) {
        std::cout << "Dropped a frame! Frame time: " << milli_seconds << "ms " << title << "\n";
      }
    }
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
