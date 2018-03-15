#pragma once

#include "screen.h"
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>

#include <SDL2/SDL.h>
#include <gflags/gflags.h>

DECLARE_bool(limit_framerate);

class GL : public Screen {
public:
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  
  GL(int scale=4): buf(), scale_(scale), last_(std::chrono::high_resolution_clock::now()) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    window_ = SDL_CreateWindow("test", 0, 0,
      Screen::BUF_WIDTH*scale_, Screen::BUF_HEIGHT*scale_, 0);
    renderer_ = SDL_CreateRenderer(window_, -1, 0);
    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "2");
    SDL_RenderSetLogicalSize(renderer_, Screen::BUF_WIDTH, Screen::BUF_HEIGHT);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, 1, Screen::BUF_WIDTH, Screen::BUF_HEIGHT);
  }
  
  void blit() {
    if (should_draw) { 
      static byte pal[4][4] = {
        {238, 253, 210, 255},
        {108, 162, 68, 255},
        {68, 130, 79, 255},
        {15, 39, 25, 255},
      };
      
      int i = 0;
      for (byte b: fb) {
        buf[i++] = pal[b][0];
        buf[i++] = pal[b][1];
        buf[i++] = pal[b][2];
        buf[i++] = pal[b][3];
      }
      if (i != BUF_WIDTH * BUF_HEIGHT * 4) {
        ;
      }
      SDL_UpdateTexture(texture_, NULL, buf.data(), Screen::BUF_WIDTH * 4);
      SDL_RenderClear(renderer_);
      SDL_RenderCopy(renderer_, texture_, NULL, NULL);
      SDL_RenderPresent(renderer_);
      if (FLAGS_limit_framerate) {
        auto now = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_);
        if (elapsed.count() < 16.75) {
          auto val = std::chrono::microseconds(16750);
          auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(elapsed);
          
          auto delta(val - elapsed_us);
          std::this_thread::sleep_for(delta);
        }
        last_ = std::chrono::high_resolution_clock::now();
      }
    }

// this slows emulator way down
    SDL_PumpEvents();

    SDL_Event event;
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        exit(0);
      }
    }
  }
  
  std::array<byte, BUF_WIDTH * BUF_HEIGHT * 4> buf;
  int scale_;
  
  std::chrono::time_point<std::chrono::high_resolution_clock> last_;

};
