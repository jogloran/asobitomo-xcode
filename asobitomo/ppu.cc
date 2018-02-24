#include "ppu.h"
#include "cpu.h"

void
PPU::step(long delta) {
  if (!cpu.ppu.lcd_on) return;
  
  ncycles += delta;
  
  /* 456*144 + 4560 = 70224
   *                                    \
   * OAM         VRAM          hblank   |
   * ------- -------------- ----------  > 144 times per frame
   * 80           172           204     |
   * ---------------------------------  |
   *              456                   /
   *
   *                                    \
   *            vblank                  |
   * -----------------------------------> 10 times per frame
   *              456                   |
   *                                    /
   */
  switch (mode) {
    case Mode::OAM:
      if (ncycles >= 80) {
        mode = Mode::VRAM;
        ncycles -= 80;
      }
      
      update_stat_register();
      break;
    case Mode::VRAM:
      if (ncycles >= 172) {
        mode = Mode::HBLANK;
        ncycles -= 172;
        // write one row to framebuffer
        
        rasterise_line();
        screen.set_row(line, raster.begin(), raster.end());
      }
      
      update_stat_register();
      
      break;
    case Mode::HBLANK:
      if (ncycles >= 204) {
        ++line;
        ncycles -= 204;
        
        if (line == 144) {
          mode = Mode::VBLANK;
          // last hblank: blit buffer
        } else {
          mode = Mode::OAM;
        }
        update_stat_register();
      }
      break;
    case Mode::VBLANK:
      // vblank happens every 4560 cycles
      
      // One vblank line
      if (ncycles >= 456) {
        ncycles -= 456;
        ++line;
        
        if (line > 153) {
          mode = Mode::OAM;
          line = 0;
        }
        
        update_stat_register();
      }
      
      break;
  }
}

void
PPU::update_stat_register()  {
  byte lyc = cpu.mmu[0xff45];

  byte stat = cpu.mmu[0xff41];
  if (lyc == line) {
    stat |= 0x4; // coincidence flag (bit 2)
  } else {
    stat &= 0xfb;
  }
  
  cpu.mmu[0xff41] = static_cast<byte>(mode);
  cpu.mmu[0xff44] = static_cast<byte>(line);
  
  byte IF = cpu.mmu[0xff0f];
  bool set_lcd_interrupt =
    ((stat & 0x40) && lyc == line) ||
    ((stat & 0x20) && mode == Mode::OAM) ||
    ((stat & 0x10) && mode == Mode::VBLANK) ||
    ((stat & 0x08) && mode == Mode::HBLANK);
  if (set_lcd_interrupt) {
    IF |= 1 << 1;
  } else {
    IF &= ~(1 << 1);
  }
  
  if (mode == Mode::VBLANK) {
    IF |= 1 << 0;
  } else {
    IF &= ~(1 << 0);
  }
  
  cpu.mmu.set(0xff0f, IF);
}

struct OAM {
  byte y;
  byte x;
  byte tile_index;
  byte flags;
};

void
PPU::rasterise_line() {
  byte& oam_ref = cpu.mmu._read_mem(0xfe00);
  OAM* oam = reinterpret_cast<OAM*>(&oam_ref);
  
  // render bg
  // render window
  // render sprites
  
  // bg
  // find bg tiles which intersect this line
  // line / 8 will intersect this line
  // bg_tilemap_offset is the beginning of bg tiles (32x32, one byte per tile)
  // if bg_tilemap_offset is 0x9800, tile IDs are unsigned offsets 0 - 255
  // if bg_tilemap_offset is 0x9c00, tile IDs are signed offsets -127 - 128
  
  // bg and window tile data comes from either 0x8000-0x8fff (1) or 0x8800-0x97ff (0)
  // according to LCDC bit 4
  byte scx = cpu.mmu._read_mem(0xff43);
  byte scy = cpu.mmu._read_mem(0xff42);
  byte palette = cpu.mmu._read_mem(0xff47);
  
  if (bg_display) {
    // line is from 0 to 143 and 144 to 153 during vblank
    word bg_tilemap_row_offset = bg_tilemap_offset + (line / 8) * 32;
  }
  
  if (window_display) {
    
  }
  
  if (sprite_display) {
    // sprite OAM is at 0xfe00 - 0xfea0 (40 sprites, 4 bytes each)
    for (size_t j = 0; j < 40; ++j) {
      OAM entry = oam[j];
      
      for (int x = 0; x < Screen::BUF_WIDTH; ++x) {
        if (entry.x != 0x0 && line + 16 >= entry.y && line + 16 < entry.y + 8) {
          // sprite is visible on this line
          raster[x] = 0;
          
        }
      }
    }
  }
  
  
}
