#pragma once

#include "types.h"
#include <array>
#include <iostream>
#include <SDL2/SDL.h>

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
  virtual int16_t operator()() = 0;
  
  friend class APU;
};

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
    if (timer-- == 0) {
      duty_index += 1;
      timer = (2048 - freq()) * 4;
    }
    
    if (length_timer-- == 0) {
      enabled = false;
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
  
  word freq() {
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
    
    length_timer = value & 0x1f;
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
  byte sound_length;
  
  byte freq_lo;
  byte freq_hi;
  
  byte initial_volume;
  bool increasing;
  
  byte sweep_number;
  bool counter_selection;
  
  std::array<byte, 5> values;
  
  word timer;
  word length_timer;
  byte duty_index;
};

class Square1 : public Square {

};

class Square2 : public Square {

};

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
  byte length;
  byte output_level;
  
  byte freq;
  byte freq_hi; // 1 high bit
};

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
  byte length;
  byte initial_volume;
  bool increasing;
  byte sweep_number;
  
  byte shift_clock_frequency;
  bool counter_width_is_15;
  byte dividing_ratio;
  
  bool counter_selection;
};


class APU {
public:
  APU(): seq(8192), sample_timer(87), buf(), buf_ptr(buf.begin()) {
    sdl_setup();
  }
  
  byte* get(word loc);
  bool set(word loc, byte value);
  
  void step(long delta);
  
  void set_channel_control(byte value) {
    right.volume = (value & 0x70) >> 4;
    left.volume = (value & 0x7);
    right.enabled = (value & 0x80) != 0;
    left.enabled = (value & 0x8) != 0;
  }
  
  void set_sound_output(byte value) {
    ch4.right = (value & (1 << 7)) != 0;
    ch3.right = (value & (1 << 6)) != 0;
    ch2.right = (value & (1 << 5)) != 0;
    ch1.right = (value & (1 << 4)) != 0;
    
    ch4.left = (value & (1 << 3)) != 0;
    ch3.left = (value & (1 << 2)) != 0;
    ch2.left = (value & (1 << 1)) != 0;
    ch1.left = (value & (1 << 0)) != 0;
  }
  void set_channel_levels(byte value) {
    global_enabled = (value & 0x80) != 0;
//    ch4.enabled = (value & (1 << 3)) != 0;
//    ch3.enabled = (value & (1 << 2)) != 0;
//    ch2.enabled = (value & (1 << 1)) != 0;
//    ch1.enabled = (value & (1 << 0)) != 0;
  }
  
private:
  void sdl_setup();
  
  Square1 ch1;
  Square2 ch2;
  Wave ch3;
  Noise ch4;
  
  Channel left;
  Channel right;
  
  bool global_enabled;
  
  word seq;
  word sample_timer;
  int step_count;
  
  constexpr static int SAMPLE_RATE = 48'000;
  SDL_AudioDeviceID dev;
  
  constexpr static int BUFSIZE = 1024;
  constexpr static int NCHANNELS = 2;
  std::array<int16_t, BUFSIZE * NCHANNELS> buf;
  std::array<int16_t, BUFSIZE * NCHANNELS>::iterator buf_ptr;
};

