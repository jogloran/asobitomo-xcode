#pragma once

#include "types.h"

class MBCBase {
public:
  virtual byte* get(word loc) = 0;
  virtual bool set(word loc, byte value) = 0;
};
