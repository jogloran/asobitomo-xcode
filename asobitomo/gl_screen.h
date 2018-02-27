#pragma once

#include "screen.h"
#include <iostream>
#include <sstream>

#include <GLUT/glut.h>
#include <SDL2/SDL.h>

extern int argc;
extern char** argv;


class GL : public Screen {
public:
  SDL_Window* window_;
  SDL_Renderer* renderer_;
  SDL_Texture* texture_;
  
  GL() {
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    window_ = SDL_CreateWindow("test", 0, 0, Screen::BUF_WIDTH, Screen::BUF_HEIGHT, SDL_WINDOW_RESIZABLE);
    renderer_ = SDL_CreateRenderer(window_, -1, 0);
    SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "2");
    SDL_RenderSetLogicalSize(renderer_, Screen::BUF_WIDTH * 3, Screen::BUF_HEIGHT * 3);
    texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_, 1, Screen::BUF_WIDTH * 3, Screen::BUF_HEIGHT * 3);
  }
  
  void blit() {
    
  }
  
  void draw() {
    std::cout << "Draw" << std::endl;
    SDL_UpdateTexture(texture_, NULL, fb.data(), Screen::BUF_WIDTH * 3 * 4);
    SDL_RenderClear(renderer_);
    SDL_RenderCopy(renderer_, texture_, NULL, NULL);
    SDL_RenderPresent(renderer_);
    
    SDL_ShowWindow(window_);
    SDL_RaiseWindow(window_);
    
    SDL_Delay( 5000 );
  }
};
