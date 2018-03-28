#include "apu.h"
#include <SDL2/SDL.h>
#include <iomanip>

bool
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
      set_channel_control(value);
      break;
    case 0xff25:
      set_sound_output(value);
      break;
    case 0xff26:
      set_channel_levels(value);
      break;
      
    default:
      return false;
  }
  
  return true;
}

byte*
APU::get(word loc) {
  switch (loc) {
    case 0xff10:
      return &ch1.values[0];
      break;
    case 0xff11:
      return &ch1.values[1];
      break;
    case 0xff12:
      return &ch1.values[2];
      break;
    case 0xff13:
      return &ch1.values[3];
      break;
    case 0xff14:
      return &ch1.values[4];
      break;
  }
  
  return nullptr;
}

void
APU::step(long delta) {
  ch1.tick();
  ch2.tick();
  ch3.tick();
  ch4.tick();
  
  //  std::cout << seq << ' ' << sample_timer << std::endl;
  if (seq-- == 0) {
    seq = 8192;
  }
  if (sample_timer-- == 0) {
    //    std::cout << "sample" << std::endl;
    int16_t L = 0, R = 0;
    int16_t v1 = ch1();
    int16_t v2 = ch2();
    L += v1;
    R += v1;
    L += v2;
    R += v2;
    
    L *= 1024;
    R *= 1024;
    
    *buf_ptr++ = L;
    *buf_ptr++ = R;
    
    if (buf_ptr >= buf.end()) {
//      while (SDL_GetQueuedAudioSize(dev) > buf.size() * sizeof(int16_t)) {
//        SDL_Delay(1);
//      }
      SDL_QueueAudio(dev, (void*)buf.data(), buf.size() * sizeof(int16_t));
      
      buf_ptr = buf.begin();
    }
    
    sample_timer = 87;
  }
}

void
APU::sdl_setup() {
  SDL_InitSubSystem(SDL_INIT_AUDIO);
  
  SDL_AudioSpec want, have;
  SDL_zero(want);
  want.freq = 48000;
  want.format = AUDIO_S16;
  want.channels = NCHANNELS;
  want.samples = BUFSIZE;
  want.callback = NULL;
  
  dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_ANY_CHANGE);
  SDL_PauseAudioDevice(dev, 0);
}
