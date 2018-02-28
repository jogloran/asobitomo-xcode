#pragma once

#include "screen.h"
#include <iostream>
#include <sstream>

#include <GLUT/glut.h>
#include <SDL2/SDL.h>

class GL : public Screen {
public:
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  
  GL() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    window_ = SDL_CreateWindow("test", 0, 0, Screen::BUF_WIDTH, Screen::BUF_HEIGHT, 0);
    renderer_ = SDL_CreateRenderer(window_, -1, 0);
    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "2");
//    SDL_RenderSetLogicalSize(renderer_, Screen::BUF_WIDTH, Screen::BUF_HEIGHT);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, 1, Screen::BUF_WIDTH, Screen::BUF_HEIGHT);
  }
  
  void blit() {
    static byte pal[] = { 0, 128, 192, 255 };

    std::vector<byte> data(fb.size() * 4);
    int i = 0;
    for (byte b: fb) {
      data[i++] = pal[b];
      data[i++] = pal[b];
      data[i++] = pal[b];
      data[i++] = 255;
    }
    SDL_UpdateTexture(texture_, NULL, data.data(), Screen::BUF_WIDTH * 4);
    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, NULL, NULL);
    SDL_RenderPresent(renderer_);
    
    SDL_PumpEvents();

    SDL_Event event;
    if (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        exit(0);
      }
    }
  }
  
  void draw() {

  }
};
