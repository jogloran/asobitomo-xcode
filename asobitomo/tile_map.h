#pragma once

#include "types.h"

#include <SDL2/SDL.h>
#include <gflags/gflags.h>
#include <array>

DECLARE_bool(show_tm);

class PPU;

class TM {
public:
  TM(PPU& ppu): ppu_(ppu), enabled_(FLAGS_show_tm) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    window_ = SDL_CreateWindow("test", 0, 0, TM_WIDTH * 2, TM_HEIGHT * 2, 0);
    renderer_ = SDL_CreateRenderer(window_, -1, 0);
    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "2");
    SDL_RenderSetLogicalSize(renderer_, TM_WIDTH * 2, TM_HEIGHT * 2);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, 1, TM_WIDTH, TM_HEIGHT);
  }
  
  void show();
  
  void set_enabled(bool enabled) {
    enabled_ = enabled;
    
    if (enabled_) {
      SDL_ShowWindow(window_);
    } else {
      SDL_HideWindow(window_);
    }
  }
  
private:
  PPU& ppu_;
  
  std::array<byte, (32*8*4) * (32*8) * 4> buf;
  
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  
  constexpr static int TM_WIDTH = 32*8;
  constexpr static int TM_HEIGHT = 32*8;

  bool enabled_;
};
