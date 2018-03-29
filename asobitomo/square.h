#pragma once

#include "types.h"

#include <array>
#include <iostream>

class Square : public Voice {
public:
  enum Duty : byte {
//    Eighth = 0x7f,
//    Quarter = 0x3f,
//    Half = 0xf,
//    ThreeQuarters = 0x3
    Eighth = 0x1,
    Quarter = 0x81,
    Half = 0x87,
    ThreeQuarters = 0x7e,
  };
  
  Square(): timer(8192), duty_index(0) {}
  
  void tick() {
//    std::cout << int(timer) << ' ' << int(duty_index) << std::endl;
    if (timer > 0) --timer;
    if (timer == 0) {
      duty_index = (duty_index + 1) % 8;
      timer = (2048 - freq()) * 4;
    }
  }
  
  int16_t operator()() {
//  std::cout << enabled << std::endl;
//    if (!enabled) return 0;
    bool duty_on = (static_cast<byte>(duty) & (1 << (7 - duty_index))) != 0;
    
    if (duty_on) {
      return initial_volume;
    } else {
      return 0;
    }
  }
  
  inline word freq() {
    return (freq_hi << 8) | freq_lo;
  }
  
  void set_sweep(byte value) {
    values[0] = value;
    sweep_time = (value & 0x70) >> 4;
    increasing = (value & 0x8) == 0;
    sweep_shift = value & 0x7;
  }
  void set_length_and_duty(byte value) {
    values[1] = value;
    byte duty_value = (value >> 6);
    switch (duty_value) {
      case 0: duty = Duty::Eighth; break;
      case 1: duty = Duty::Quarter; break;
      case 2: duty = Duty::Half; break;
      case 3: duty = Duty::ThreeQuarters; break;
    }
    
    length = value & 0x1f;
  }
  void set_envelope(byte value) {
    values[2] = value;
    initial_volume = value >> 4;
    increasing = (value & 0x8) != 0;
    sweep_number = value & 0x7;
  }
  void set_frequency_low(byte value) {
    values[3] = value;
    freq_lo = value;
  }
  void set_frequency_high(byte value) {
    values[4] = value;
  // initial
    freq_hi = value & 0x7;
    counter_selection = (value & 0x40) != 0;
  }
  
//private:
  byte sweep_time;
  byte sweep_shift;
  
  Duty duty;
  
  byte freq_lo;
  byte freq_hi;
  
  byte initial_volume;
  bool increasing;
  
  byte sweep_number;

  
  std::array<byte, 5> values;
  
  word timer;
  byte duty_index;
};

class Square1 : public Square {

};

class Square2 : public Square {

};

