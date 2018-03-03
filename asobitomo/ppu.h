#pragma once

#include "types.h"
#include "console_screen.h"
#include "gl_screen.h"

#include <array>

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
  
  PPU(CPU& cpu): raster(), screen(),
    line(0), mode(Mode::OAM), ncycles(0), vblank_ncycles(0), cpu(cpu), lcd_on(true),
    window_tilemap_offset(0), window_display(false),
    bg_window_tile_data_offset(0),
    bg_tilemap_offset(0),
    sprite_mode(SpriteMode::S8x8), sprite_display(false),
    bg_display(false), old_oam(40) {}

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
  
  typedef byte PaletteIndex;
  std::vector<PaletteIndex> decode(word start_loc, byte start_y /* 0 to 7 */);
  std::vector<PaletteIndex> unpack_bits(byte lsb, byte msb);
    GL screen;
public:

  std::array<byte, 160> raster;
  
  
  void update_stat_register();
  


  byte line;
  Mode mode;
  long ncycles;
  long vblank_ncycles;
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
};
