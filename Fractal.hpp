///
/// fractal computation & rendering
///

#pragma once

#include "common.hpp"
#include "Colour.hpp"
#include <string>
#include <cassert>

namespace mandelbroet {

struct Fractal {
  virtual ~Fractal();

  virtual void draw(unsigned width, unsigned height, Colour *pixels, Real zoom, Real x_pos, Real y_pos) = 0;

  virtual void step_parameter() = 0;

  virtual std::string name() const = 0;

  virtual std::string parameter() const = 0;

  ssize_t max_escape_time() const {
    const ssize_t time = _max_escape_time();
    assert(time >= 0);
    assert(time < MAX_ESCAPE_TIME);
    return time;
  }
private:
  virtual int _max_escape_time() const = 0;
};

struct Mandelbrot : Fractal {
  void draw(unsigned width, unsigned height, Colour *pixels, Real zoom, Real x_pos, Real y_pos) override;

  std::string name() const override { return "Mandelbrot"; }

  std::string parameter() const override {
    return "max_iterations=" + std::to_string(_current_max_escape_time);
  }

  void step_parameter() override {
    _current_max_escape_time++;
    if (_current_max_escape_time >= MAX_ESCAPE_TIME) { _current_max_escape_time = 0; }
  }
private:
  int _max_escape_time() const override {
    return _current_max_escape_time;
  }

  int _current_max_escape_time = 0;
};


struct Multi_Brot : Fractal {
  void draw(unsigned width, unsigned height, Colour *pixels, Real zoom, Real x_pos, Real y_pos) override;

  std::string name() const override { return "Multibrot"; }

  std::string parameter() const override {
    return "d=" + std::to_string(d);
  }

  void step_parameter() override {
    d += 0.1;
    if (d > max_d) { d = min_d; }
  }
private:
  int _max_escape_time() const override {
    return 64;
  }

  Real d = min_d;
  const Real min_d = 0.5;
  const Real max_d = 5.0;
};

} /// end namespace mandelbroet
