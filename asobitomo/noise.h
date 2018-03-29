#pragma once

#include <array>
#include <iostream>
#include <SDL2/SDL.h>

class Noise : public Voice {
public:
  void tick() {}
  int16_t operator()() { return 0; }
  
  void set_length(byte value) {
    length = value & 0x1f;
  }
  
  void set_volume_envelope(byte value) {
    initial_volume = value >> 4;
    increasing = (value & 0x8) != 0;
    sweep_number = value & 0x7;
  }
  
  void set_polynomial_counter(byte value) {
    shift_clock_frequency = value >> 4;
    counter_width_is_15 = (value & 0x8) == 0;
    dividing_ratio = value & 0x7;
  }
  void set_counter_consecutive(byte value) {
    counter_selection = (value & 0x40) != 0;
  }
  
private:
  byte initial_volume;
  bool increasing;
  byte sweep_number;
  
  byte shift_clock_frequency;
  bool counter_width_is_15;
  byte dividing_ratio;
  
  bool counter_selection;
};
