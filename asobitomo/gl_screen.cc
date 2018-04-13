#include "gl_screen.h"
#include "cpu.h"
#include "ppu.h"

GL::GL(CPU& cpu, int scale)
: cpu_(cpu), buf(), scale_(scale), last_(std::chrono::high_resolution_clock::now()) {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_InitSubSystem(SDL_INIT_VIDEO);
  window_ = SDL_CreateWindow("Game", 0, 0,
                             Screen::BUF_WIDTH*scale_,
                             Screen::BUF_HEIGHT*scale_,
                             0);
  renderer_ = SDL_CreateRenderer(window_, -1, 0);
  SDL_SetHint("SDL_HINT_RENDER_SCALE_QUALITY", "2");
  SDL_RenderSetLogicalSize(renderer_, Screen::BUF_WIDTH, Screen::BUF_HEIGHT);
  texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_ARGB8888, 1, Screen::BUF_WIDTH, Screen::BUF_HEIGHT);
  
}

std::array<byte, 3> decode(word w) {
  return {
    static_cast<byte>((w & 0x1f) * 8),
    static_cast<byte>(((w >> 5) & 0x1f) * 8),
    static_cast<byte>(((w >> 10) & 0x1f) * 8)
  };
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
    
    // to hack this in, we could
    // find out the 8x8 grid the current pixel belongs to
    // access mmu.vram_bank_mem[0x9800 + ...] to get palette num
    // access mmu.bgp[8 * palette_num + ...] to get the 8 bytes
    // representing the bg palette
    // unpack these and populate the buffer with them according
    // to the value of _b_
    
    int i = 0;
    int pos = 0;
    // fb has dims 160x144
    for (byte b: fb) {
      int col = pos % 160;
      int row = pos / 160;
      
      row /= 8; col /= 8;
      // this doesn't work since we need to get it from the tile map, not the viewport
      
      //      std::cout << "accessing " << row << " " << col << std::endl;
      byte palette_num = 7;//cpu_.mmu.vram_bank_mem[(0x9800 + (32*row) + col) - 0x8000];
      if (palette_num != 0) {
        //        std::cout << "row " << row << " col " << col << " palette " << int(palette_num) << std::endl;
      }
      // select a palette value byte according to 'b'
      if (row==0&&col==0) {
        //        std::cout << "bytes: " << setw(2) << setfill('0') << int(cpu_.mmu.bgp[8 * palette_num + (2 * b)]) << ' ' <<
        //        int((cpu_.mmu.bgp[8 * palette_num + (2 * b + 1)])) << std::endl;
      }
      
      word bgp = cpu_.mmu.bgp[8 * palette_num + (2 * b)] |
      (cpu_.mmu.bgp[8 * palette_num + (2 * b + 1)] << 8);
      //      std::cout << hex << setfill('0') << setw(2) << int(cpu_.mmu.bgp[8 * palette_num + (2 * b)]) << ' '
      //      << int(cpu_.mmu.bgp[8 * palette_num + (2 * b + 1)]) << std::endl;
      // decode the colour values
      auto rgb = decode(bgp);
      if (row==0&&col==0) {
//        std::cout << int(palette_num ) << ' ' << int(b) << std::endl;
//
//        std::cout << hex << int(rgb[0]/8) << ' ' << int(rgb[1]/8) << ' ' << int(rgb[2]/8) << std::endl;
      }
      
      buf[i++] = rgb[2];
      buf[i++] = rgb[1];
      buf[i++] = rgb[0];
      buf[i++] = 255;
      
      ++pos;
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
    for (word addr = 0x9800; addr <= 0x9bff; ++addr) {
      std::cout << hex << setfill('0') << setw(2)
      << int(cpu_.mmu.vram_bank_mem[addr - 0x8000])
      << ' ';
      if ((addr-0x9800) % 32 == 31) {
        std::cout << std::endl;
      }
    }
    std::cout << std::endl;
    exit(0);
  } else if (keystates[SDL_SCANCODE_Y]) {
    // Dump bg palette data
    std::cout << hex << setw(2) << setfill('0');
    for (int i = 0; i < 64; ++i) {
      std::cout << setw(2) << setfill('0') << int(cpu_.mmu.bgp[i]) << ' ';
      if (i % 8 == 7) std::cout << std::endl;
    }
    std::cout << std::endl << std::endl;
    
    std::cout << hex << setw(2) << setfill('0');
    for (int i = 0; i < 64; ++i) {
      std::cout << setw(2) << setfill('0') << int(cpu_.mmu.obp[i]) << ' ';
      if (i % 8 == 7) std::cout << std::endl;
    }
    std::cout << std::endl;
    exit(0);
  }
  
  SDL_Event event;
  if (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      notify_exiting();
    }
  }
}
