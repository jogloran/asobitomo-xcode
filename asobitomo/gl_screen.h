#pragma once

#include "screen.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

#include <SDL2/SDL.h>
#include <gflags/gflags.h>

DECLARE_bool(limit_framerate);

class CPU;

class GL : public Screen {
public:
  CPU& cpu_;
  
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  
  GL(CPU& cpu, int scale=4): cpu_(cpu), buf(), scale_(scale), last_(std::chrono::high_resolution_clock::now()) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    window_ = SDL_CreateWindow("test", 0, 0,
      Screen::BUF_WIDTH*scale_, Screen::BUF_HEIGHT*scale_, 0);
    renderer_ = SDL_CreateRenderer(window_, -1, 0);
    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "2");
    SDL_RenderSetLogicalSize(renderer_, Screen::BUF_WIDTH, Screen::BUF_HEIGHT);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, 1, Screen::BUF_WIDTH, Screen::BUF_HEIGHT);
  }
  
  void blit();
  
  std::array<byte, BUF_WIDTH * BUF_HEIGHT * 4> buf;
  int scale_;
  
  std::chrono::time_point<std::chrono::high_resolution_clock> last_;

};
