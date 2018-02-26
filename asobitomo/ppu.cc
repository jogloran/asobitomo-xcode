#include "ppu.h"
#include "cpu.h"

#include <numeric>
#include <iterator>

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
          std::cout << "\033[2J\033[1;1H";
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
  
  if (true) {
    // line is from 0 to 143 and 144 to 153 during vblank
    // line touches tiles in row (line + scy) / 8
    //                       columns scx / 8 to scx / 8 + 20
    // starting and ending at vertical pixel (line + scy) % 8
    //                        horizontal pixel scx % 8
    
    // get the sequence of tiles which are touched
    auto row_touched = (line + scy) / 8;
    
    auto index_touched = row_touched * 32 + (scx / 8); // index_touched... +20
    // y coordinate within a tile is (line + scy) % 8
    // starting x coordinate for the line of tiles is scx % 8
    
    word item = bg_tilemap_offset + index_touched;
    
    // if bg_window_tile_data_offset is 0x8000,
    //     tile data ranges from 0x8000 (index 0) to 0x8fff (index 0xff)
    // else if bg_window_tile_data_offset is 0x8800,
    //     tile data ranges from 0x8800 (index -127) to 0x9000 (index 0) to 0x97ff (index 128)
    
    // there are 0x1000 bytes of tile data -- each entry is 0x10 bytes, so there are 0x100 entries
    
    std::vector<std::vector<PaletteIndex>> tile_data;
    auto begin = &cpu.mmu.mem[item];
    auto end = &cpu.mmu.mem[item] + 20;
    std::transform(begin, end, std::back_inserter(tile_data),
                   [this, scy](byte index) {
                     // 2 bytes per row, 16 bytes per tile
                     // in each row, first byte is LSB of palette indices
                     //              second byte is MSB
                     if (bg_window_tile_data_offset == 0x8000) {
                       return decode(bg_window_tile_data_offset + index*4,
                                     (line + scy) % 8);
                     } else {
                       return decode(bg_window_tile_data_offset + (128-index)*4,
                                     (line + scy) % 8);
                     }
                   });
    
    // now we have tile data for each touched tile
    // need to extract line of palette indices
    
    
    
    if (ASOBITOMO_DEBUG) {
    // y coordinate within a tile is (line + scy) % 8
    // starting x coordinate for the line of tiles is scx % 8
      static const char* d[] = {
        " ", "\u2591", "\u2592", "\u2593"
      };
      for (auto it = tile_data.begin(); it != tile_data.end(); ++it) {
        for (auto it2 = it->begin(); it2 != it->end(); ++it2) {
          std::cout << d[*it2] << d[*it2];
        }
      }
      std::cout << std::endl;
      }
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

std::vector<PPU::PaletteIndex>
PPU::decode(word start_loc, byte start_y) {
  // start_y is from 0 to 7
//  std::vector<byte> data(
//                           &cpu.mmu._read_mem(bg_window_tile_data_offset + static_cast<sbyte>(start_loc)*4),
//                           &cpu.mmu._read_mem(bg_window_tile_data_offset + static_cast<sbyte>(start_loc)*4 + 16));
  // we want row start_y of the tile
  // 2 bytes per row, 8 rows
  
  // the relevant line is bg_window_tile_data_offset + static_cast<sbyte>(start_loc)*4 + start_y (+ 1)
  byte b1 = cpu.mmu._read_mem(bg_window_tile_data_offset + (start_loc)*4 + start_y*2);
  byte b2 = cpu.mmu._read_mem(bg_window_tile_data_offset + (start_loc)*4 + start_y*2 + 1);
  
  // b1/b2 is packed:
  // b1            b2
  // 00 01 10 11 | 10 11 00 10 LSB
  // 01 00 10 10 | 10 10 01 01 MSB
  // -------------------------
  // 02 02 30 31   30 31 02 12 <- we want to get this sequence
  if (ASOBITOMO_DEBUG) {
    ;
  }
  
  return unpack_bits(b1, b2);
}

std::vector<PPU::PaletteIndex>
PPU::unpack_bits(byte lsb, byte msb) {
  std::vector<PPU::PaletteIndex> result(8, 0);
  
  for (int i = 7; i >= 0; --i) {
    byte m = msb & 0x1;
    byte l = lsb & 0x1;
    
    result[i] = (m << 1) | l;
    
    lsb >>= 1; msb >>= 1;
  }
  
  return result;
}

