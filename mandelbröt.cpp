
#include <iostream>
#include <cassert> /// for assert
#include <utility> /// for std::swap

#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_render.h>
#include <bits/unique_ptr.h>
#include <cstring>

struct Main {
};

using Real = float;

// Precise method, which guarantees v = v1 when t = 1.
Real lerp(Real v0, Real v1, Real t) {
  return (1 - t) * v0 + t * v1;
}

Real scale(Real value, Real src_min, Real src_max, Real dst_min, Real dst_max) {
  return dst_min + ((dst_max - dst_min) * value) / (src_max - src_min);
}

unsigned mandelbrot_escape_time(unsigned Px, unsigned Py, unsigned max_x, unsigned max_y) {
  const Real mandelbrot_min_x = -2.5;
  const Real mandelbrot_max_x = +1.0;

  const Real mandelbrot_min_y = -1.0;
  const Real mandelbrot_max_y = +1.0;

  /// scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.5, 1))
  const Real x0 = scale(Px, 0, max_x, mandelbrot_min_x, mandelbrot_max_x);
  /// scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1, 1))
  const Real y0 = scale(Py, 0, max_y, mandelbrot_min_y, mandelbrot_max_y);

  Real x = 0.0;
  Real y = 0.0;

  unsigned max_iteration = 1000;

  for (int iteration = 0; iteration < max_iteration; iteration++) {
    const Real xtemp = x*x - y*y + x0;

    if (xtemp > (x*x + y*y <= 2*2)) {
      return iteration;
    }

    y = 2*x*y + y0;
    x = xtemp;
  }

  return max_iteration;
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
      (unsigned char) ::lerp(v0.R, v1.R, t),
      (unsigned char) ::lerp(v0.G, v1.G, t),
      (unsigned char) ::lerp(v0.B, v1.B, t),
      (unsigned char) ::lerp(v0.A, v1.A, t)
    );
  }

  unsigned char A;
  unsigned char B;
  unsigned char G;
  unsigned char R;
};

inline static const Colour WHITE = Colour::rgb(255, 255, 255);
inline static const Colour BLACK = Colour::rgb(0, 0, 0);

struct Image {
  Image(uint32_t width, uint32_t height) : _width{width}, _height{height} {
    pixels.reset(new Colour[width * height]);
  }

  Colour &operator()(unsigned x, unsigned y) {
    assert(x < _width);
    assert(y < _height);

    return pixels[y * _height + x];
  }

  void clear(Colour C = BLACK) {
    std::fill(pixels.get(), pixels.get() + width() * height(), C);
  }

  unsigned width() const { return _width; }
  unsigned height() const { return _height; }

  const void *data() const { return pixels.get(); }
private:
  const uint32_t _width, _height;
  std::unique_ptr<Colour[]> pixels;
};

struct Color_Pallette {
  static constexpr inline unsigned BAD  =    0;
  static constexpr inline unsigned GOOD =  500;
  static constexpr inline unsigned BEST = 1000;

  Colour operator()(unsigned N) const {
    N = std::min(N, BEST);

    const Colour C_BAD  = Colour::rgb(255, 0, 0);
    const Colour C_OK   = Colour::rgb(0, 255, 0);
    const Colour C_GOOD = Colour::rgb(0, 0, 255);
    const Colour C_BEST = Colour::rgb(255, 0, 255);

    if (N <= BAD) {
      return Colour::lerp(C_BAD, C_OK, scale(N, 0, BAD, 0, 1));
    }

    if (N <= GOOD) {
      return Colour::lerp(C_OK, C_GOOD, scale(N, BAD, GOOD, 0, 1));
    }

    if (N <= BEST) {
      return Colour::lerp(C_GOOD, C_BEST, scale(N, GOOD, BEST, 0, 1));
    }

    return BLACK;
  }
};

int main(int argc, const char **argv) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << "\n";
    return 1;
  }

  const unsigned WIN_WIDTH  = 640;
  const unsigned WIN_HEIGHT = 480;

  const uint32_t window_flags = SDL_WINDOW_RESIZABLE;

  SDL_Window *window = SDL_CreateWindow(
    "Rendering to a texture!",
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

  const int texture_width  = WIN_WIDTH;
  const int texture_height = WIN_HEIGHT;

  //Make a target texture to render too
  SDL_Texture *texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGBA8888,
    SDL_TEXTUREACCESS_TARGET,
    texture_width, texture_height
  );

  Image pixels{texture_width, texture_height};
  pixels.clear();

  Color_Pallette pallette;

  const uint64_t performance_frequency = SDL_GetPerformanceFrequency();

  SDL_Event event;
  bool running = true;
  while (true) {
    const uint64_t start = SDL_GetPerformanceCounter();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    while(SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          goto quit;
        case SDL_KEYDOWN:
          if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
            goto quit;
          }
          break;
        quit:
          running = false;
          break;
      }
    }

    if (!running) { break; }

    /// splat down some random pixels
    pixels.clear();

    const unsigned MIN_X = 0;
    const unsigned MAX_X = texture_width;

    const unsigned MIN_Y = 0;
    const unsigned MAX_Y = texture_height;

    for (unsigned y = MIN_Y; y < MAX_Y; y++) {
      for (unsigned x = MIN_X; x < MAX_X; x++) {
        const unsigned N = mandelbrot_escape_time(x, y, texture_width, texture_height);

        pixels(x, y) = pallette(N);
      }
    }

    /// pitch ... the number of bytes in a row of pixel data, including padding between lines
    const int pitch = texture_width * 4;

    SDL_UpdateTexture(
      texture,
      NULL,
      pixels.data(),
      pitch
    );

    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    {
      const uint64_t end           = SDL_GetPerformanceCounter();
      const double   seconds       = (end - start) / double(performance_frequency);
      const double   milli_seconds = seconds * 1000;

      const unsigned frames_per_second  = 5;
      const double   target_frame_time = (1.0 / frames_per_second) * 1000;

      if (milli_seconds < target_frame_time) {
        SDL_Delay(target_frame_time - milli_seconds);
      } else if (milli_seconds > target_frame_time) {
        std::cout << "Dropped a frame! Frame time: " << milli_seconds << "ms" << std::endl;
      }
    }
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}
