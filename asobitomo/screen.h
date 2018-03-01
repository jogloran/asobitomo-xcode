#pragma once

#include <vector>
#include <algorithm>

#include "types.h"

class Screen {
public:
  Screen(): fb(BUF_WIDTH * BUF_HEIGHT, 0), should_draw(true) {}

  template <typename T>
  void set_row(size_t row, T begin, T end) {
    std::copy(begin, end, fb.begin() + row * BUF_WIDTH);
  }
  virtual void blit() = 0;
  
  void off() { should_draw = false; }
  void on() { should_draw = true; }
  
  static constexpr int BUF_WIDTH = 160;
  static constexpr int BUF_HEIGHT = 144;
  
protected:
  bool should_draw;
  std::vector<byte> fb;
};
