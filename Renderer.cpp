
#include "Renderer.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_render.h>

using namespace mandelbroet;

mandelbroet::Renderer::Renderer(const char *title, unsigned width, unsigned height) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    fprintf(stderr, "error: Could not initialize SDL video system: %s\n", SDL_GetError());
    exit(1);
  }

  const uint32_t window_flags = SDL_WINDOW_RESIZABLE; // | SDL_WINDOW_MAXIMIZED;

  _window = SDL_CreateWindow(
    title,
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    width,
    height,
    window_flags
  );
  if (!_window) {
    fprintf(stderr, "error: Could not open SDL window: %s\n", SDL_GetError());
    exit(1);
  }

  const int      renderer_driver_index = -1; /// take any valid renderer driver
  const uint32_t renderer_flags        = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;

  _renderer = SDL_CreateRenderer(
    (SDL_Window*) _window,
    renderer_driver_index,
    renderer_flags
  );
  if (!_renderer) {
    fprintf(stderr, "error: Could not open SDL renderer: %s\n", SDL_GetError());
    exit(1);
  }
}

mandelbroet::Renderer::~Renderer() {
  if (_window) {
    SDL_DestroyWindow((SDL_Window*) _window);
    _window = nullptr;
  }
  if (_renderer) {
    SDL_DestroyRenderer((SDL_Renderer*) _renderer);
    _renderer = nullptr;
  }

  SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void mandelbroet::Renderer::set_window_title(const char *title) {
  SDL_SetWindowTitle((SDL_Window*) _window, title);
}

void mandelbroet::Renderer::swap_buffers() {
  SDL_RenderPresent((SDL_Renderer*) _renderer);
}

void mandelbroet::Renderer::clear() {
  SDL_Renderer *const renderer = (SDL_Renderer*) _renderer;

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);
}

Texture mandelbroet::Renderer::create_texture(unsigned width, unsigned height) {
  SDL_Texture *texture = SDL_CreateTexture(
    (SDL_Renderer*) _renderer,
    SDL_PIXELFORMAT_RGBA8888,
    SDL_TEXTUREACCESS_TARGET,
    width, height
  );
  if (!texture) {
    fprintf(stderr, "error: Could not create %ux%u texture: %s\n", width, height, SDL_GetError());
    exit(1);
  }

  return Texture{width, height, texture};
}

mandelbroet::Texture::~Texture() {
  SDL_DestroyTexture((SDL_Texture*) _texture);
}

void mandelbroet::Renderer::draw_texture(mandelbroet::Texture &tex) {
  SDL_RenderCopy((SDL_Renderer*) _renderer, (SDL_Texture*) tex._texture, nullptr, nullptr);
}

void mandelbroet::Texture::update(const Colour *pixels) {
  /// pitch ... the number of bytes in a row of pixel data, including padding between lines
  const int pitch = _width * sizeof(Colour);

  SDL_UpdateTexture(
    (SDL_Texture*) _texture,
    NULL,
    pixels,
    pitch
  );
}