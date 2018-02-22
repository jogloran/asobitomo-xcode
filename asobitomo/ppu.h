#pragma once

#include "types.h"

class CPU;

class PPU {
public:
  PPU(CPU& cpu): line(0), mode(Mode::OAM), ncycles(0), vblank_ncycles(0), cpu(cpu), lcd_on(false) {}

  enum class Mode : byte {
    HBLANK = 0,
    VBLANK = 1,
    OAM = 2,
    VRAM = 3,
  };

  void step(long delta);
  void set_lcd(bool on) {
    lcd_on = on;
  }
  
private:

  void update_stat_register();

  byte line;
  Mode mode;
  long ncycles;
  long vblank_ncycles;
  bool lcd_on;

  CPU& cpu;
};
