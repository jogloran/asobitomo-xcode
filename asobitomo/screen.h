#pragma once

#include <vector>
#include <array>
#include <algorithm>

#include "types.h"

class Screen {
public:
  Screen(): fb(), should_draw(true) {}

  template <typename T>
  void set_row(int row, T begin, T end) {
    if (row < 0 || row >= BUF_HEIGHT) {
      throw std::range_error("invalid row");
    }
    if (std::distance(begin, end) != BUF_WIDTH) {
      throw std::range_error("invalid range");
    }
    std::copy_n(begin, BUF_WIDTH, fb.begin() + row * BUF_WIDTH);
  }
  virtual void blit() = 0;
  
  void off() { should_draw = false; }
  void on() { should_draw = true; }
  
  static constexpr int BUF_WIDTH = 160;
  static constexpr int BUF_HEIGHT = 144;
  
protected:
  bool should_draw;
  std::array<byte, BUF_WIDTH * BUF_HEIGHT> fb;
};
