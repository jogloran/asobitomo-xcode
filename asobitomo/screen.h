#pragma once

#include <vector>
#include <algorithm>

#include "types.h"

class Screen {
public:
  Screen(): fb(BUF_WIDTH * BUF_HEIGHT, 0) {}

  template <typename T>
  void set_row(size_t row, T begin, T end) {
    std::copy(begin, end, fb.begin() + row * BUF_WIDTH);
  }
  
  static constexpr int BUF_WIDTH = 256;
  static constexpr int BUF_HEIGHT = 256;
  
private:
  std::vector<byte> fb;
};
