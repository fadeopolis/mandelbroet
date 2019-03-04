
#include "Performance_Counter.hpp"

#include <SDL2/SDL.h>

uint64_t mandelbroet::read_performance_counter() {
  return SDL_GetPerformanceCounter();
}

uint64_t mandelbroet::performance_counter_frequency() {
  return SDL_GetPerformanceFrequency();
}

void mandelbroet::wait(uint64_t milliseconds) {
  SDL_Delay(milliseconds);
}
