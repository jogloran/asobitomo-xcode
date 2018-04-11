#pragma once

#include "types.h"
#include "screen.h"
#include "console_screen.h"
#include "gl_screen.h"
#include "tile_debugger.h"
#include "tile_map.h"

#include <memory>
#include <array>
#include <gflags/gflags.h>

DECLARE_bool(tm);
DECLARE_bool(td);

struct OAM {
  byte y;
  byte x;
  byte tile_index;
  byte flags;
  
  bool operator==(const OAM& other) {
    return y == other.y && x == other.x && tile_index == other.tile_index && flags == other.flags;
  }
  
  bool operator!=(const OAM& other) {
    return !(*this == other);
  }
};

class CPU;

class PPU {
public:
  enum class SpriteMode {
    S8x8, S8x16
  };
  
  typedef byte PaletteIndex;
//  typedef std::array<PaletteIndex, 8> TileRow;
  typedef uint16_t TileRow;
  
  struct RenderedSprite {
    RenderedSprite(const OAM& oam, byte oam_index, const TileRow& pixels): oam_(oam), oam_index_(oam_index), pixels_(pixels) {}
    OAM oam_;
    byte oam_index_;
    TileRow pixels_;
  };

  
  PPU(CPU& cpu): raster(), screen(std::make_unique<GL>(cpu)),
    debugger(*this), tilemap(*this),
    line(0), mode(Mode::OAM), ncycles(0), cpu(cpu), lcd_on(true),
    window_tilemap_offset(0), window_display(false),
    bg_window_tile_data_offset(0x8000),
    bg_tilemap_offset(0),
    sprite_mode(SpriteMode::S8x8), sprite_display(false),
    bg_display(false), old_oam(40) {
      
    visible.reserve(40);
    
    debugger.set_enabled(FLAGS_td);
    tilemap.set_enabled(FLAGS_tm);
  }

  enum class Mode : byte {
    HBLANK = 0,
    VBLANK = 1,
    OAM = 2,
    VRAM = 3,
  };

  void step(long delta);
  
  void stat(byte value);
  
  void set_lcd_on(bool on);
  void set_window_tilemap_offset(word offset);
  void set_bg_window_tile_data_offset(word offset);
  void set_window_display(bool on);
  void set_bg_tilemap_offset(word offset);
  void set_sprite_mode(SpriteMode mode);
  void set_sprite_display(bool on);
  void set_bg_display(bool on);

  void rasterise_line();
  
  TileRow decode(word start_loc, byte start_y=0 /* 0 to 7 */);
  TileRow unpack_bits(byte lsb, byte msb);
  TileRow tilemap_index_to_tile(byte index, byte y_offset);
  std::unique_ptr<Screen> screen;
  TD debugger;
  TM tilemap;
  
public:
  std::array<byte, 160> raster;
  
  void update_stat_register();

  byte line;
  Mode mode;
  long ncycles;
  bool lcd_on;
  
  word window_tilemap_offset;
  bool window_display;
  word bg_window_tile_data_offset;
  word bg_tilemap_offset;
  SpriteMode sprite_mode;
  bool sprite_display;
  bool bg_display;

  CPU& cpu;
  
  friend class CPU;
  std::vector<OAM> old_oam;
  
  // Caches
  std::array<PPU::PaletteIndex, 160> palette_index_row;
  std::array<word, 21> row_tiles;
  std::vector<RenderedSprite> visible;
  std::array<byte, 160> sprite_row;
  std::array<PaletteIndex, 168> raster_row;
  std::array<TileRow, 21> tile_data;
};

PPU::PaletteIndex apply_palette(PPU::PaletteIndex pidx, byte sprite_palette);
