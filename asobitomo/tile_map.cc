#include "tile_map.h"

#include "ppu.h"
#include "cpu.h"

void
TM::show() {
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
  SDL_Rect rect { scx, scy, 160*2, 144*2 }; // TODO: 4 = scale
  
  SDL_UpdateTexture(texture_, NULL, buf.data(), TM_WIDTH * 4);
  SDL_RenderClear(renderer_);
  SDL_RenderCopy(renderer_, texture_, NULL, NULL);
  
  SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 255);
  SDL_RenderDrawRect(renderer_, &rect);
  
  SDL_RenderPresent(renderer_);
}
