#include "ppu.h"
#include "cpu.h"
#include "util.h"

#include <numeric>
#include <iterator>
#include <algorithm>
#include <sstream>

void
compare_oams(OAM* current_oam, OAM* old_oam) {
  bool first = true;
  
  std::stringstream s;
  for (int i = 0; i < 40; ++i) {
    OAM cur = current_oam[i];
    OAM old = old_oam[i];
    
    if (cur != old) {
      if (first) {
        first = false;
      } else {
        s << ", ";
      }
      s << "obj " << hex << setw(2) << setfill('0') << i << " [";
      
      bool first_change = true;
      
      if (cur.x != old.x) {
        if (first_change) first_change = false;
        else s << ", ";
        s << "x " << dec << static_cast<int>(old.x) << " -> " << static_cast<int>(cur.x) << hex;
      }
      if (cur.y != old.y) {
        if (first_change) first_change = false;
        else s << ", ";
        s << "y " << dec << static_cast<int>(old.y) << " -> " << static_cast<int>(cur.y) << hex;
      }
      if (cur.tile_index != old.tile_index) {
        if (first_change) first_change = false;
        else s << ", ";
        s << "t " << static_cast<int>(old.tile_index) << " -> " << static_cast<int>(cur.tile_index);
      }
      if (cur.flags != old.flags) {
        if (first_change) first_change = false;
        else s << ", ";
        s << "f " << static_cast<int>(old.flags) << " -> " << static_cast<int>(cur.flags);
      }
      
      s << "]";
    }
  }
  
  std::string out = s.str();
  if (out.size()) {
    std::cout << s.str() << std::endl;
  }
}

template <typename T> std::vector<T>
flatten(const std::vector<std::vector<T>>& in) {
  std::vector<T> result;
  for (auto it = in.begin(); it != in.end(); ++it) {
    std::copy(it->begin(), it->end(), std::back_inserter(result));
  }
  return result;
}

void
PPU::stat(byte value) {
  set_lcd_on(value & (1 << 7));
  set_window_tilemap_offset(value & (1 << 6) ? 0x9c00 : 0x9800);
  set_window_display(value & (1 << 5));
  set_bg_window_tile_data_offset(value & (1 << 4) ? 0x8000 : 0x8800);
  set_bg_tilemap_offset(value & (1 << 3) ? 0x9c00 : 0x9800);
  set_sprite_mode(value & (1 << 2) ?
    PPU::SpriteMode::S8x16 :
    PPU::SpriteMode::S8x8);
  set_sprite_display(value & (1 << 1));
  set_bg_display(value & 0x1);
}

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
          screen.blit();
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

std::ostream& operator<<(std::ostream& out, const OAM& oam) {
  return out << "OAM(" << hex << static_cast<int>(oam.x) << ", "
    << static_cast<int>(oam.y)
    << ", tile_index = " << setw(2) << hex << static_cast<int>(oam.tile_index)
    << ", flags = " << binary(oam.flags) << ")";
}

void
PPU::rasterise_line() {
  byte& oam_ref = cpu.mmu._read_mem(0xfe00);
  OAM* oam = reinterpret_cast<OAM*>(&oam_ref);
  
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

  // format of palette is a mapping
  //         LSB
  // 11 10 01 00 <- palette indices
  //  v  v  v  v
  // 01 00 11 00 <- colour values
  byte palette = cpu.mmu._read_mem(0xff47);
//  auto colour_map = interpret_colour_map(palette);
  
  if (bg_display) {
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
    
    // map this through the colour map
    
    std::vector<PaletteIndex> raster_row = flatten(tile_data);
    std::transform(raster_row.begin(), raster_row.end(), raster_row.begin(),
                   [palette](PaletteIndex pidx) {
                     switch (pidx) {
                       case 0:
                         return palette & 3;
                       case 1:
                         return (palette >> 2) & 3;
                       case 2:
                         return (palette >> 4) & 3;
                       case 3:
                         return (palette >> 6) & 3;
                       default:
                         throw std::runtime_error("invalid palette index");
                     }
                   });
    
    // write to raster
    typedef std::vector<byte>::size_type diff;
    std::rotate(raster_row.begin(), raster_row.begin() + static_cast<diff>(scx), raster_row.end()); // TODO: need bounds check
    std::copy(raster_row.begin(), raster_row.end(), raster.begin());
  }
  
  if (window_display) {
//    byte wx = cpu.mmu._read_mem(0xff4b);
//    byte wy = cpu.mmu._read_mem(0xff4a);
//    
//    // line is from 0 to 143 and 144 to 153 during vblank
//    // line touches tiles in row (line + scy) / 8
//    //                       columns scx / 8 to scx / 8 + 20
//    // starting and ending at vertical pixel (line + scy) % 8
//    //                        horizontal pixel scx % 8
//    
//    // get the sequence of tiles which are touched
//    auto row_touched = (line + wy) / 8;
//    
//    auto index_touched = row_touched * 32 + ((wx - 7) / 8); // index_touched... +20
//    // y coordinate within a tile is (line + scy) % 8
//    // starting x coordinate for the line of tiles is scx % 8
//    
//    word item = window_tilemap_offset + index_touched;
//    
//    // if bg_window_tile_data_offset is 0x8000,
//    //     tile data ranges from 0x8000 (index 0) to 0x8fff (index 0xff)
//    // else if bg_window_tile_data_offset is 0x8800,
//    //     tile data ranges from 0x8800 (index -127) to 0x9000 (index 0) to 0x97ff (index 128)
//    
//    // there are 0x1000 bytes of tile data -- each entry is 0x10 bytes, so there are 0x100 entries
//    
//    std::vector<std::vector<PaletteIndex>> tile_data;
//    auto begin = &cpu.mmu.mem[item];
//    auto end = &cpu.mmu.mem[item] + 20;
//    std::transform(begin, end, std::back_inserter(tile_data),
//                   [this, wy](byte index) {
//                     // 2 bytes per row, 16 bytes per tile
//                     // in each row, first byte is LSB of palette indices
//                     //              second byte is MSB
//                     if (bg_window_tile_data_offset == 0x8000) {
//                       return decode(bg_window_tile_data_offset + index*4,
//                                     (line + wy) % 8);
//                     } else {
//                       return decode(bg_window_tile_data_offset + (128-index)*4,
//                                     (line + wy) % 8);
//                     }
//                   });
//    
//    // map this through the colour map
//    
//    std::vector<PaletteIndex> raster_row = flatten(tile_data);
//    std::transform(raster_row.begin(), raster_row.end(), raster_row.begin(),
//                   [palette](PaletteIndex pidx) {
//                     switch (pidx) {
//                       case 0:
//                         return palette & 3;
//                       case 1:
//                         return (palette >> 2) & 3;
//                       case 2:
//                         return (palette >> 4) & 3;
//                       case 3:
//                         return (palette >> 6) & 3;
//                       default:
//                         throw std::runtime_error("invalid palette index");
//                     }
//                   });
//    
//    // write to raster
//    typedef std::vector<byte>::size_type diff;
////    std::rotate(raster_row.begin(), raster_row.begin() + static_cast<diff>(scx), raster_row.end());
//    std::copy(raster_row.begin(), raster_row.end(), raster.begin());
  }
  
  if (sprite_display) {
    std::array<byte, 160> sprite_row;
    std::fill(sprite_row.begin(), sprite_row.end(), 0);
    
    compare_oams(oam, old_oam.data());
    if (oam[4].tile_index == 0xa && old_oam[4].tile_index == 0x85 && oam[5].tile_index == 0x25 && old_oam[5].tile_index == 0x85) {
      ;
    }
    
    // sprite OAM is at 0xfe00 - 0xfea0 (40 sprites, 4 bytes each)
    for (size_t j = 0; j < 40; ++j) {
      OAM entry = oam[j];
//      if (entry.x != 0)
//      std::cout << j << ": " << entry << std::endl;

      for (int x = 0; x < Screen::BUF_WIDTH; ++x) {
        if (entry.x != 0 && entry.x < 168 && entry.y != 0 && entry.y < 160 &&
            line + 16 >= entry.y && line + 16 < entry.y + 8) {
          // sprite is visible on this line
          // get tile data for sprite
          
          // sprite tiles start at 0x8000 and go to 0x8fff, 16 bytes per tile (each 2 bytes represent one of the 8 rows)
          word tile_data = 0x8000 + entry.tile_index * 16;
          
          // sprites are not necessarily aligned to the 8x8 grid
          // we need to be able to tell which line of the sprite
          // intersects the current scanline
          
          // if we are in this section, then entry.y - 16 <= line < entry.y - 8
          // (entry.y - 16 - line) % 8?
          
          // need to get the relevant row in the tile
          byte row_offset_within_tile = (line - (entry.y - 16)) % 8;
          word tile_data_address = tile_data + row_offset_within_tile * 2;
          byte b1 = cpu.mmu._read_mem(tile_data_address);
          byte b2 = cpu.mmu._read_mem(tile_data_address + 1);
          
          // for example, entry.y = 0x80, so the top left is at 0x80-0x10 = 0x70 = 112
          // when line = 115, we have 112 <= line < 120, so we are inside the sprite.
          // we are on row index 3 of the sprite, so the row offset is (line - (entry.y - 16)) % 8
          
          auto decoded = unpack_bits(b1, b2);
          std::transform(decoded.begin(), decoded.end(), decoded.begin(),
                         [palette](PaletteIndex pidx) {
                           switch (pidx) {
                             case 0:
                               return palette & 3;
                             case 1:
                               return (palette >> 2) & 3;
                             case 2:
                               return (palette >> 4) & 3;
                             case 3:
                               return (palette >> 6) & 3;
                             default:
                               throw std::runtime_error("invalid palette index");
                           }
                         });
          
          std::copy(decoded.begin(), decoded.end(), sprite_row.begin() + entry.x - 8); // TODO: doesn't check for sprite_row oob
//          auto ptr = sprite_row.begin() + entry.x - 8;
//          for (auto it = decoded.begin(); it != decoded.end(); ) {
//            if (ptr == sprite_row.end()) {
//              break;
//            }
//            *ptr++ = *it++;
//          }
        }
      }
    }
    
//    std::copy(sprite_row.begin(), sprite_row.end(), raster.begin());
    std::transform(sprite_row.begin(), sprite_row.end(), raster.begin(), raster.begin(), [](byte sprite_byte, byte raster_byte) {
      return (sprite_byte | raster_byte) & 3;
    });
    
    std::copy(oam, oam + 40, old_oam.begin());
  }
  
  
}

std::vector<PPU::PaletteIndex>
PPU::decode(word start_loc, byte start_y) {
  // start_y is from 0 to 7
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

