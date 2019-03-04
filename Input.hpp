///
/// User input backend
///

namespace mandelbroet {

struct Input {
  Input();

  ~Input();

  void poll_events();

  bool quit() const { return _quit; }

  bool move_left()  const { return _move_left; }
  bool move_right() const { return _move_right; }
  bool move_up()    const { return _move_up; }
  bool move_down()  const { return _move_down; }
  bool zoom_in()    const { return _zoom_in; }
  bool zoom_out()   const { return _zoom_out; }
private:
  bool _quit = false;
  bool _move_left = false, _move_right = false;
  bool _move_up = false, _move_down = false;
  bool _zoom_in = false, _zoom_out = false;
};

} /// namespace mandelbroet
