#pragma once

#include "types.h"
#include "screen.h"
#include "console_screen.h"
#include "gl_screen.h"
#include "tile_debugger.h"
#include "tile_map.h"

#include <memory>
#include <array>
#include "ppu_base.h"
#include <gflags/gflags.h>

DECLARE_bool(tm);
DECLARE_bool(td);

class CPU;

class GameBoyPPU : public PPUBase {
public:
  struct RenderedSprite {
    RenderedSprite(const OAM& oam, byte oam_index, const TileRow& pixels): oam_(oam), oam_index_(oam_index), pixels_(pixels) {}
    OAM oam_;
    byte oam_index_;
    TileRow pixels_;
  };
  
  GameBoyPPU(CPU& cpu):
    PPUBase(cpu) {
//    debugger(*this), tilemap(*this) {
    visible.reserve(40);
    
//    debugger.set_enabled(FLAGS_td);
//    tilemap.set_enabled(FLAGS_tm);
  }
  
  void rasterise_line();
  
  TileRow decode(word start_loc, byte start_y=0 /* 0 to 7 */);
  TileRow tilemap_index_to_tile(byte index, byte y_offset);
  
//  TD debugger;
//  TM tilemap;
  
  // Caches
  std::array<PPUBase::PaletteIndex, 160> palette_index_row;
  std::array<word, 21> row_tiles;
  std::vector<GameBoyPPU::RenderedSprite> visible;
  std::array<PaletteIndex, 168> raster_row;
  std::array<TileRow, 21> tile_data;
};

PPUBase::PaletteIndex apply_palette(PPUBase::PaletteIndex pidx, byte sprite_palette);
