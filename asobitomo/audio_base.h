#pragma once
#include "types.h"

class Channel {
public:
  bool enabled;
  byte volume;
};

class APU;

class Voice {
public:
  bool enabled;
  bool left;
  bool right;
  
  virtual void tick() = 0;
  virtual void tick_length() {
    if (length > 0) --length;
    if (length == 0) {
      enabled = false;
    }
  }
  virtual void tick_volume() {}
  virtual void tick_sweep() {}
  
  virtual int16_t operator()() = 0;
  
  friend class APU;
  
protected:
  byte length;
  bool counter_selection;
};
