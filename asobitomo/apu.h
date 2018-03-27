#pragma once

#include "types.h"

class Channel {
  bool enabled;
  byte volume;
};

class Voice {
  bool enabled;
  byte dest_channel;
};

class Square : public Voice {
public:
  enum Duty : byte {
    Eighth = 0x7f,
    Quarter = 0x3f,
    Half = 0xf,
    ThreeQuarters = 0x3
  };
  
  void set_sweep(byte);
  void set_length_and_duty(byte);
  void set_envelope(byte);
  void set_frequency_low(byte);
  void set_frequency_high(byte);
  
private:
  byte freq_lo;
  byte freq_hi;
  byte sound_length;
  
  byte initial_volume;
  bool increasing;
  byte sweep_number;
};

class Square1 : public Square {

};

class Square2 : public Square {

};

class Wave : public Voice {
public:
  void set_enabled(byte);
  void set_length(byte);
  void set_output_level(byte);
  void set_wave_low(byte);
  void set_wave_high_and_control(byte);
  void set_wave_pattern(byte, byte);
  
private:
//  std::array<byte, 32>
};

class Noise : public Voice {
public:
  void set_length(byte);
  void set_volume_envelope(byte);
  void set_polynomial_counter(byte);
  void set_counter_consecutive(byte);
  
private:
};


class APU {
public:
  APU() {}
  
  byte* get(word loc);
  void set(word loc, byte value);
  
  void set_channel_control(byte);
  void set_sound_output(byte);
  void set_channel_levels(byte);
  
private:
  Square1 ch1;
  Square2 ch2;
  Wave ch3;
  Noise ch4;
  
  Channel left;
  Channel right;
  
  bool global_enabled;
};
