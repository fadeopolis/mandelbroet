///
/// Rendering backend
///

#pragma once

#include "common.hpp"
#include "Colour.hpp"

namespace mandelbroet {

struct Texture {
  Texture(const Texture&) = delete;

  ~Texture();

  void update(const Colour *pixels);
private:
  explicit Texture(unsigned width, unsigned height, void *texture)
  : _width{width}, _height{height}, _texture{texture} {}

  unsigned _width, _height;
  void *_texture;

  friend struct Renderer;
};


struct Renderer {
  Renderer(const char *title, unsigned width = 640, unsigned height = 480);

  ~Renderer();

  void set_window_title(const char *title);

  void swap_buffers();

  void clear();

  Texture create_texture(unsigned width, unsigned height);

  void draw_texture(Texture &tex);
private:
  void *_window = nullptr;
  void *_renderer = nullptr;
};

} /// end namespace mandelbroet
