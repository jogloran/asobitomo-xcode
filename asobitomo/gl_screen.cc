#include "gl_screen.h"
#include "cpu.h"

void
GL::blit() {
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
  
  int nkeys;
  const uint8_t* keystates = SDL_GetKeyboardState(&nkeys);
  if (keystates[SDL_SCANCODE_D]) {
    cpu_.dump_state();
  }
  
  SDL_Event event;
  if (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      exit(0);
    }
  }
}
