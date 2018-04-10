#pragma once

#include "ppu.h"

void flatten(std::array<PPU::PaletteIndex, 168>& raster_row, typename std::array<PPU::TileRow, 21>::iterator src);

void flatten_single(std::array<PPU::PaletteIndex, 64>& raster_row, typename std::array<PPU::TileRow, 8>::iterator src);

void
compare_oams(OAM* current_oam, OAM* old_oam);
