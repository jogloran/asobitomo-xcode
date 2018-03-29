#pragma once

#include <array>
#include <iostream>
#include <SDL2/SDL.h>

class Wave : public Voice {
public:
  void tick() {}
  int16_t operator()() { return 0; }
  
  void set_enabled(byte value) {
    playback_enabled = (value & 0x80) != 0;
  }
  
  void set_length(byte value) {
    length = value;
  }
  
  void set_output_level(byte value) {
    output_level = value;
  }
  
  void set_wave_low(byte value) {
    freq = value;
  }
  
  void set_wave_high_and_control(byte value) {
    freq_hi = value & 0x7;
  }
  
  void set_wave_pattern(byte value, byte offset) {
    wave[offset] = value;
  }
  
private:
  std::array<byte, 32> wave;
  bool playback_enabled;
  byte output_level;
  
  byte freq;
  byte freq_hi; // 1 high bit
};
