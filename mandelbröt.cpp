
#include <iostream>    /// for std::cout
#include <cassert>     /// for assert
#include <utility>     /// for std::swap
#include <type_traits> /// for std::is_base_of, std::is_same
#include <memory>      /// for std::unique_ptr
#include <complex>     /// for std::complex

#include "Renderer.hpp"
#include "Input.hpp"
#include "Performance_Counter.hpp"
#include "Fractal.hpp"

using namespace mandelbroet;

int main(int argc, const char **argv) {
  mandelbroet::Mandelbrot fractal;
//  mandelbroet::Multi_Brot fractal;

  const unsigned WIN_WIDTH  = 640;
  const unsigned WIN_HEIGHT = 480;

  mandelbroet::Renderer renderer{
    fractal.name().c_str(),
    WIN_WIDTH,
    WIN_HEIGHT,
  };

  constexpr const size_t TEXTURE_WIDTH  = 1 << 10;
  constexpr const size_t TEXTURE_HEIGHT = 1 << 10;

  mandelbroet::Texture texture = renderer.create_texture(TEXTURE_WIDTH, TEXTURE_HEIGHT);

  mandelbroet::Input input;

  std::unique_ptr<Colour[]> pixels{new Colour[TEXTURE_WIDTH * TEXTURE_HEIGHT]};
  std::uninitialized_fill(pixels.get(), pixels.get() + TEXTURE_WIDTH * TEXTURE_HEIGHT, BLACK);

  const uint64_t performance_frequency = performance_counter_frequency();

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

  bool running = true;
  while (true) {
    const uint64_t start = mandelbroet::read_performance_counter();

    /// ***** handle events

    input.poll_events();
    if (input.quit()) {
      break;
    }

    if (input.move_left()) {
      x_pos -= x_speed * zoom;
    }
    if (input.move_right()) {
      x_pos += x_speed * zoom;
    }
    if (input.move_up()) {
      y_pos -= y_speed * zoom;
    }
    if (input.move_down()) {
      y_pos += x_speed * zoom;
    }
    if (input.zoom_in()) {
      zoom *= zoom_in_speed;
    }
    if (input.zoom_out()) {
      zoom *= zoom_out_speed;
    }

    /// ***** calculate fractal

    fractal.draw(TEXTURE_WIDTH, TEXTURE_HEIGHT, pixels.get(), zoom, x_pos, y_pos);

    fractal.step_parameter();

    /// ***** set window title

    std::string title = fractal.name() + "! (" + fractal.parameter() + ") (x="
                        + std::to_string(x_pos) + ", y=" + std::to_string(y_pos)
                        + ", zoom=" + std::to_string(zoom) + ")";

    renderer.set_window_title(title.c_str());

    /// ***** render fractal

    texture.update(pixels.get());

//    renderer.clear();
    renderer.draw_texture(texture);
    renderer.swap_buffers();

    /// ***** control frame rate
    {
      const uint64_t end           = mandelbroet::read_performance_counter();
      const double   seconds       = (end - start) / double(performance_frequency);
      const double   milli_seconds = seconds * 1000;

      const unsigned frames_per_second  = 15;
      const double   target_frame_time = (1.0 / frames_per_second) * 1000;

      if (milli_seconds < target_frame_time) {
        mandelbroet::wait(target_frame_time - milli_seconds);
      } else if (milli_seconds > target_frame_time) {
        std::cout << "Dropped a frame! Frame time: " << milli_seconds << "ms " << title << "\n";
      }
    }
  }

  return 0;
}
