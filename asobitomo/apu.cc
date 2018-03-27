#include "apu.h"

void
APU::set(word loc, byte value) {
  // ff10-ff14 Square 1
  // ff16-ff19 Square 2
  // ff1a-ff1e Wave
  // ff20-ff23 Noise
  // ff24-ff26 Sound control
  // ff30-ff3f Wave data
  
  switch (loc) {
    case 0xff10:
      ch1.set_sweep(value);
      break;
    case 0xff11:
      ch1.set_length_and_duty(value);
      break;
    case 0xff12:
      ch1.set_envelope(value);
      break;
    case 0xff13:
      ch1.set_frequency_low(value);
      break;
    case 0xff14:
      ch1.set_frequency_high(value);
      break;
    case 0xff16:
      ch2.set_length_and_duty(value);
      break;
    case 0xff17:
      ch2.set_envelope(value);
      break;
    case 0xff18:
      ch2.set_frequency_low(value);
      break;
    case 0xff19:
      ch2.set_frequency_high(value);
      break;
    case 0xff1a:
      ch3.set_enabled(value);
      break;
    case 0xff1b:
      ch3.set_length(value);
      break;
    case 0xff1c:
      ch3.set_output_level(value);
      break;
    case 0xff1d:
      ch3.set_wave_low(value);
      break;
    case 0xff1e:
      ch3.set_wave_high_and_control(value);
      break;
    case 0xff30:
    case 0xff31:
    case 0xff32:
    case 0xff33:
    case 0xff34:
    case 0xff35:
    case 0xff36:
    case 0xff37:
    case 0xff38:
    case 0xff39:
    case 0xff3a:
    case 0xff3b:
    case 0xff3c:
    case 0xff3d:
    case 0xff3e:
    case 0xff3f:
      ch3.set_wave_pattern(value, loc - 0xff30);
      break;
    case 0xff20:
      ch4.set_length(value);
      break;
    case 0xff21:
      ch4.set_volume_envelope(value);
      break;
    case 0xff22:
      ch4.set_polynomial_counter(value);
      break;
    case 0xff23:
      ch4.set_counter_consecutive(value);
      break;
    case 0xff24:
      set_channel_levels(value);
      break;
    case 0xff25:
      set_sound_output(value);
      break;
    case 0xff26:
      set_channel_control(value);
      break;
  }
}

byte*
APU::get(word loc) {


  return nullptr;
}
