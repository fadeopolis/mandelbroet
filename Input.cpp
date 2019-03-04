
#include "Input.hpp"

#include <SDL2/SDL.h>

using namespace mandelbroet;

mandelbroet::Input::Input() {
  if (SDL_Init(SDL_INIT_EVENTS) != 0) {
    fprintf(stderr, "error: Could not initialize SDL event system: %s\n", SDL_GetError());
    exit(1);
  }
}

mandelbroet::Input::~Input() {
  SDL_QuitSubSystem(SDL_INIT_EVENTS);
}

void mandelbroet::Input::poll_events() {
  SDL_Event event;

  while(SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        _quit = true;
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.scancode) {
          case SDL_SCANCODE_ESCAPE:
            _quit = true;
            break;
          case SDL_SCANCODE_LEFT:
          case SDL_SCANCODE_KP_4:
          case SDL_SCANCODE_A:
            _move_left = true;
            break;
          case SDL_SCANCODE_RIGHT:
          case SDL_SCANCODE_KP_6:
          case SDL_SCANCODE_D:
            _move_right = true;
            break;
          case SDL_SCANCODE_UP:
          case SDL_SCANCODE_KP_8:
          case SDL_SCANCODE_W:
            _move_up = true;
            break;
          case SDL_SCANCODE_DOWN:
          case SDL_SCANCODE_KP_2:
          case SDL_SCANCODE_S:
            _move_down = true;
            break;
          case SDL_SCANCODE_KP_PLUS:
          case SDL_SCANCODE_E:
          case SDL_SCANCODE_PAGEUP:
            _zoom_in = true;
            break;
          case SDL_SCANCODE_KP_MINUS:
          case SDL_SCANCODE_Q:
          case SDL_SCANCODE_PAGEDOWN:
            _zoom_out = true;
            break;
        }
        break;
      case SDL_KEYUP:
        switch (event.key.keysym.scancode) {
          case SDL_SCANCODE_LEFT:
          case SDL_SCANCODE_KP_4:
          case SDL_SCANCODE_A:
            _move_left = false;
            break;
          case SDL_SCANCODE_RIGHT:
          case SDL_SCANCODE_KP_6:
          case SDL_SCANCODE_D:
            _move_right = false;
            break;
          case SDL_SCANCODE_UP:
          case SDL_SCANCODE_KP_8:
          case SDL_SCANCODE_W:
            _move_up = false;
            break;
          case SDL_SCANCODE_DOWN:
          case SDL_SCANCODE_KP_2:
          case SDL_SCANCODE_S:
            _move_down = false;
            break;
          case SDL_SCANCODE_KP_PLUS:
          case SDL_SCANCODE_E:
          case SDL_SCANCODE_PAGEUP:
            _zoom_in = false;
            break;
          case SDL_SCANCODE_KP_MINUS:
          case SDL_SCANCODE_Q:
          case SDL_SCANCODE_PAGEDOWN:
            _zoom_out = false;
            break;
        }
        break;
    }
  }
}

