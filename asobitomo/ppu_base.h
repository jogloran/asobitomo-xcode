#pragma once

#include <memory>
#include <array>

#include "types.h"
#include "screen.h"
#include "gl_screen.h"

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

class PPUBase {
public:
  PPUBase(CPU& cpu): raster(), screen(std::make_unique<GL>(cpu)),
    line(0), mode(Mode::OAM), ncycles(0), cpu(cpu), lcd_on(true),
    window_tilemap_offset(0), window_display(false),
    bg_window_tile_data_offset(0x8000),
    bg_tilemap_offset(0),
    sprite_mode(SpriteMode::S8x8), sprite_display(false),
    bg_display(false) {

  }
  
  virtual void rasterise_line() = 0;

  void step(long delta);
  
  void stat(byte value);
  
  typedef byte PaletteIndex;
  typedef std::array<PaletteIndex, 8> TileRow;
  
  enum class SpriteMode {
    S8x8, S8x16
  };
  
  enum class Mode : byte {
    HBLANK = 0,
    VBLANK = 1,
    OAM = 2,
    VRAM = 3,
  };
  
  TileRow unpack_bits(byte lsb, byte msb);
  
  void set_lcd_on(bool on);
  void set_window_tilemap_offset(word offset);
  void set_bg_window_tile_data_offset(word offset);
  void set_window_display(bool on);
  void set_bg_tilemap_offset(word offset);
  void set_sprite_mode(SpriteMode mode);
  void set_sprite_display(bool on);
  void set_bg_display(bool on);
  
  std::unique_ptr<Screen> screen;

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
};
