///
/// colours
///

#pragma once

#include "common.hpp"

namespace mandelbroet {

struct Colour {
  Colour() = default;
  constexpr Colour(unsigned char R, unsigned char G, unsigned char B, unsigned char A)
          : A{A}, B{B}, G{G}, R{R} {}

  static constexpr Colour rgb(unsigned char R, unsigned char G, unsigned char B) {
    return rgba(R, G, B, 255);
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

} /// end namespace mandelbroet
