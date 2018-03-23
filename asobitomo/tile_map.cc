#include "tile_map.h"

#include "ppu.h"
#include "cpu.h"

void
TM::show() {
  if (!enabled_) {
    return;
  }
  
  word tilemap_offset = ppu_.bg_tilemap_offset;
  
  for (int row = 0; row < 32; ++row) {
    for (int col = 0; col < 32; ++col) {
      int idx = row * 32 + col;
      word item = tilemap_offset + idx; // This is the address of the tile index
      byte tile_index = ppu_.cpu.mmu[item];
      
      // Get the tile data offset
      word tile_data_address;
      if (ppu_.bg_window_tile_data_offset == 0x8000) {
        tile_data_address = ppu_.bg_window_tile_data_offset + tile_index * 16;
      } else {
        tile_data_address = ppu_.bg_window_tile_data_offset + 0x800 + (static_cast<signed char>(tile_index))*16;
      }
      
      // Decode tile
      std::vector<PPU::PaletteIndex> tile_pixels;
      for (int m = 0; m < 8; ++m) {
        auto row = ppu_.decode(tile_data_address, m, 0);
        std::copy(row.begin(), row.end(), std::back_inserter(tile_pixels));
      }
      
      // Place tile at location
      for (int k = 0; k < tile_pixels.size(); ++k) {
        int off_y = k / 8;
        int off_x = k % 8;
        
        int offset = (row*8 + off_y) * (TM_WIDTH * 4) + ((col*8 + off_x) * 4);
      
        buf[offset++] = tile_pixels[k] * 64;
        buf[offset++] = tile_pixels[k] * 64;
        buf[offset++] = tile_pixels[k] * 64;
        buf[offset++] = 255;
      }
    }
  }
  
  byte scx = ppu_.cpu.mmu._read_mem(0xff43);
  byte scy = ppu_.cpu.mmu._read_mem(0xff42);
  
//  static char s[64];
//  sprintf(s, "scx %02x scy %02x", scx, scy);
//  SDL_SetWindowTitle(window_, s);
  
  SDL_UpdateTexture(texture_, NULL, buf.data(), TM_WIDTH * 4);
  SDL_RenderClear(renderer_);
  SDL_RenderCopy(renderer_, texture_, NULL, NULL);
  
  SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
  
  int alpha = std::min(256 - scx, 160);
  int beta = std::min(256 - scy, 144);
  int gamma = 160 - alpha;
  int delta = 144 - beta;
  
  SDL_Rect rects[] {
    { scx*2, scy*2, alpha*2, beta*2 },
    { 0, scy*2, gamma*2, beta*2},
    { scx*2, 0, alpha*2, delta*2},
    { 0, 0, gamma*2, delta*2 }
  };

  SDL_SetRenderDrawColor(renderer_, 0, 255, 0, 64);
  for (SDL_Rect rect : rects) {
    SDL_RenderFillRect(renderer_, &rect);
  }
  
  SDL_SetRenderDrawColor(renderer_, 244, 177, 18, 64);
  for (int i = 0; i < 256; i += 8) {
    SDL_RenderDrawLine(renderer_, 0, i*2, 256*2, i*2);
    SDL_RenderDrawLine(renderer_, i*2, 0, i*2, 256*2);
  }
  
  SDL_RenderPresent(renderer_);
  
  int mouse_x, mouse_y;
  uint8_t mouse_mask = SDL_GetMouseState(&mouse_x, &mouse_y);
  if (mouse_mask & SDL_BUTTON(1)) {
    mouse_x /= 2;
    mouse_y /= 2;
    
    std::cout << "address: " << setw(4) << setfill('0') << hex <<  (ppu_.bg_tilemap_offset + (mouse_y / 16) * 32 + (mouse_x / 16)) << std::endl;
    byte tile_index = ppu_.cpu.mmu[ppu_.bg_tilemap_offset + (mouse_y / 16) * 32 + (mouse_x / 16)];
////    byte tile_data = ppu_.cpu.mmu[ppu_.bg_window_tile_data_offset + tile_index*16];
////
     std::cout << hex << setfill('0') << setw(2) << int(mouse_x / 16) << ' ' << setw(2) << int(mouse_y / 16) << ' ' << setw(2) << int(tile_index) << std::endl;
  }
}
