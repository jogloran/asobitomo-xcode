#include "gl_screen.h"
#include "cpu.h"

GL::GL(CPU& cpu, int scale)
: cpu_(cpu), buf(), scale_(scale), last_(std::chrono::high_resolution_clock::now()) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_InitSubSystem(SDL_INIT_VIDEO);
  window_ = SDL_CreateWindow("Game", 0, 0,
    Screen::BUF_WIDTH*scale_, Screen::BUF_HEIGHT*scale_, 0);
  renderer_ = SDL_CreateRenderer(window_, -1, 0);
  SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "2");
  SDL_RenderSetLogicalSize(renderer_, Screen::BUF_WIDTH, Screen::BUF_HEIGHT);
  texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, 1, Screen::BUF_WIDTH, Screen::BUF_HEIGHT);
}

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
  } else if (keystates[SDL_SCANCODE_X]) {
    for (word addr = 0xc4f4; addr <= 0xc506; ++addr) {
      std::cout << hex << setfill('0') << setw(2) << int(cpu_.mmu._read_mem(addr)) << ' ';
    }
    std::cout << std::endl;
  } else if (keystates[SDL_SCANCODE_Y]) {
    for (word addr = 0x5b4a; addr <= 0x5b4a + 16; ++addr) {
      std::cout << hex << setfill('0') << setw(2) << int(cpu_.mmu._read_mem(addr)) << ' ';
    }
    std::cout << std::endl;
  }
  
  SDL_Event event;
  if (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      cpu_.mmu.mbc->save("eram.sav");
      exit(0);
    }
  }
}
