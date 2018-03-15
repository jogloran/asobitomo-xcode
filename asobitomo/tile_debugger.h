#pragma once

#include "types.h"

#include <SDL2/SDL.h>
#include <gflags/gflags.h>
#include <array>

class PPU;

class TD {
public:
  TD(PPU& ppu): ppu_(ppu) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    window_ = SDL_CreateWindow("test", 0, 0, TD_WIDTH * 4, TD_HEIGHT * 4, 0);
    renderer_ = SDL_CreateRenderer(window_, -1, 0);
    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "2");
    SDL_RenderSetLogicalSize(renderer_, TD_WIDTH * 4, TD_HEIGHT * 4);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, 1, TD_WIDTH, TD_HEIGHT);
  }
  
  void show();
  
private:
  PPU& ppu_;
  
  std::array<byte, (16*8*4) * (16*8) * 4> buf;
  
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  
  constexpr static int TD_WIDTH = 16*8;
  constexpr static int TD_HEIGHT = 16*8;
};
