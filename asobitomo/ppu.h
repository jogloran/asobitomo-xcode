#pragma once

#include "types.h"
#include "screen.h"

#include <array>

class CPU;

class PPU {
public:
  enum class SpriteMode {
    S8x8, S8x16
  };
  
  PPU(CPU& cpu): raster(), screen(),
    line(0), mode(Mode::OAM), ncycles(0), vblank_ncycles(0), cpu(cpu), lcd_on(false),
    window_tilemap_offset(0), window_display(false),
    bg_window_tile_data_offset(0),
    bg_tilemap_offset(0),
    sprite_mode(SpriteMode::S8x8), sprite_display(false),
    bg_display(false) {}

  enum class Mode : byte {
    HBLANK = 0,
    VBLANK = 1,
    OAM = 2,
    VRAM = 3,
  };

  void step(long delta);
  void set_lcd_on(bool on) {
    lcd_on = on;
  }
  void set_window_tilemap_offset(word offset) {
    window_tilemap_offset = offset;
  }
  void set_bg_window_tile_data_offset(word offset) {
    bg_window_tile_data_offset = offset;
  }
  void set_window_display(bool on) {
    window_display = on;
  }
  void set_bg_tilemap_offset(word offset) {
    bg_tilemap_offset = offset;
  }
  void set_sprite_mode(SpriteMode mode) {
    sprite_mode = mode;
  }
  void set_sprite_display(bool on) {
    sprite_display = on;
  }
  void set_bg_display(bool on) {
    bg_display = on;
  }
  void rasterise_line();
private:

  std::array<byte, 256> raster;
  
  
  void update_stat_register();
  
  Screen screen;

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
};
