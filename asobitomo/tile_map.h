#pragma once

#include "types.h"

#include <SDL2/SDL.h>
#include <gflags/gflags.h>
#include <array>

class PPU;

class TM {
public:
  TM(PPU& ppu): ppu_(ppu) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    window_ = SDL_CreateWindow("test", 0, 0, TM_WIDTH * 2, TM_HEIGHT * 2, 0);
    renderer_ = SDL_CreateRenderer(window_, -1, 0);
    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "2");
    SDL_RenderSetLogicalSize(renderer_, TM_WIDTH * 2, TM_HEIGHT * 2);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, 1, TM_WIDTH, TM_HEIGHT);
  }
  
  void show();
  
private:
  PPU& ppu_;
  
  std::array<byte, (32*8*4) * (32*8) * 4> buf;
  
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  
  constexpr static int TM_WIDTH = 32*8;
  constexpr static int TM_HEIGHT = 32*8;

};
