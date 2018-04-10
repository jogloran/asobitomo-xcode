#pragma once

#include "ppu.h"

template <unsigned long A>
void flatten(std::array<PPU::PaletteIndex, A>& raster_row, typename std::array<PPU::TileRow, A>::iterator src) {
  auto ptr = raster_row.begin();
  for (int i = 0; i < A; ++i) {
    for (int j = 7; j >= 0; --j) {
      auto mask = (1 << 2*j) | (1 << (2*j+1));
      *ptr++ = (*src & mask) >> 2*j;
    }
    src++;
  }
}

void
compare_oams(OAM* current_oam, OAM* old_oam);
