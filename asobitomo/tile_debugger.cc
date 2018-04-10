
#include <vector>

#include "tile_debugger.h"

#include "mmu.h"
#include "ppu.h"
#include "cpu.h"
#include "ppu_util.h"

void TD::show() {
  if (!enabled_) {
    return;
  }
  
  word tile_offset = ppu_.bg_window_tile_data_offset;
  for (int i = 0; i < 256; ++i) {
    word tile_start = tile_offset + i * 16;
    std::array<PPU::PaletteIndex, 64> tile_pixels;
    std::array<PPU::TileRow, 8> rows;
    
    for (int m = 0; m < 8; ++m) {
      rows[m] = ppu_.decode(tile_start, m);
    }
    flatten_single(tile_pixels, rows.begin());
    
    int row = i / 16;
    int col = i % 16;
    for (int k = 0; k < tile_pixels.size(); ++k) {
      int off_y = k / 8;
      int off_x = k % 8;
      
      int offset = (row*8 + off_y) * (TD_WIDTH * 4) + ((col*8 + off_x) * 4);
      buf[offset++] = tile_pixels[k] * 64;
      buf[offset++] = tile_pixels[k] * 64;
      buf[offset++] = tile_pixels[k] * 64;
      buf[offset++] = 255;
    }
  }
  
  SDL_UpdateTexture(texture_, NULL, buf.data(), TD_WIDTH * 4);
  SDL_RenderClear(renderer_);
  SDL_RenderCopy(renderer_, texture_, NULL, NULL);
  SDL_RenderPresent(renderer_);
}
