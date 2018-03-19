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
  ncycles += delta;
  
  if (!cpu.ppu.lcd_on) return;
  
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
          debugger.show();
          tilemap.show();
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
    stat &= ~0x4;
  }
  
  cpu.mmu[0xff41] = (stat & 0xfc) | static_cast<byte>(mode);
  cpu.mmu[0xff44] = static_cast<byte>(line);
  
  byte IF = cpu.mmu[0xff0f];
  bool set_lcd_interrupt =
    ((stat & 0x40) && (lyc == line)) ||
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
  
  // bg and window tile data comes from either 0x8000-0x8fff (1) or 0x8800-0x97ff (0)
  // according to LCDC bit 4
  byte scx = cpu.mmu._read_mem(0xff43);
  byte scy = cpu.mmu._read_mem(0xff42);

  // Background palette
  byte palette = cpu.mmu._read_mem(0xff47);
  // For the sprite palettes, the lowest two bits should always map to 0
  byte obp0 = cpu.mmu._read_mem(0xff48) & 0xfc;
  byte obp1 = cpu.mmu._read_mem(0xff49) & 0xfc;
  
  if (bg_display) {
    // get the sequence of tiles which are touched
    auto row_touched = ((line + scy) % 144) / 8; // % 144 for scy wrap around
  
    // create sequence of tiles to use (these can wrap around)
    auto starting_index = scx / 8;
    std::vector<word> row_tiles(20, 0);
    for (int i = 0; i < 20; ++i) {
      row_tiles[i] = cpu.mmu[bg_tilemap_offset + row_touched * 32 + ((starting_index + i) % 32)];
    }
    
    // if bg_window_tile_data_offset is 0x8000,
    //     tile data ranges from 0x8000 (index 0) to 0x8fff (index 0xff)
    // else if bg_window_tile_data_offset is 0x8800,
    //     tile data ranges from 0x8800 (index -127) to 0x9000 (index 0) to 0x97ff (index 128)
    
    // there are 0x1000 bytes of tile data -- each entry is 0x10 bytes, so there are 0x100 entries
    
    std::vector<std::vector<PaletteIndex>> tile_data;
    // These are pointers into the tile map
    auto begin = row_tiles.begin();
    auto end = row_tiles.end();
    std::transform(begin, end, std::back_inserter(tile_data),
                   [this, scx, scy](byte index) {
                     // This takes each tile map index and retrieves
                     // the corresponding line of the corresponding tile
                     // 2 bytes per row, 16 bytes per tile
                     // in each row, first byte is LSB of palette indices
                     //              second byte is MSB
                     if (bg_window_tile_data_offset == 0x8000) {
                       return decode(bg_window_tile_data_offset + index*16,
                                     (line + scy) % 8, scx % 8);
                     } else {
                       // add 0x800 to interpret the tile map index as a signed index starting in the middle
                       // of the tile data range (0x8800-97FF)
                       return decode(bg_window_tile_data_offset + 0x800 + (static_cast<signed char>(index))*16,
                                     (line + scy) % 8, scx % 8);
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
    
//    std::rotate(raster_row.begin(), raster_row.begin(), raster_row.end());
    
    // write to raster
    typedef std::vector<byte>::size_type diff;

    auto offset = static_cast<int>(scx % 8);
    
    auto fin = std::copy(raster_row.begin() + offset, raster_row.end(), raster.begin());
    std::copy(raster_row.begin(), raster_row.begin() + offset, fin);
  }
  
  if (window_display) {
    byte wx = cpu.mmu._read_mem(0xff4b);
    byte wy = cpu.mmu._read_mem(0xff4a);
    
    if (line >= wy) {
      byte row_touched = (line - wy) / 8;
      
      std::vector<word> row_tiles(20, 0);
      
      for (int i = 0; i < 20; ++i) {
        row_tiles[i] = cpu.mmu[window_tilemap_offset + row_touched * 32 + (i % 32)];
      }
      
      std::vector<std::vector<PaletteIndex>> tile_data;
      // These are pointers into the tile map
      auto begin = row_tiles.begin();
      auto end = row_tiles.end();
      std::transform(begin, end, std::back_inserter(tile_data),
                     [this, wx, wy](byte index) {
                       // This takes each tile map index and retrieves
                       // the corresponding line of the corresponding tile
                       // 2 bytes per row, 16 bytes per tile
                       // in each row, first byte is LSB of palette indices
                       //              second byte is MSB
                       if (window_tilemap_offset == 0x8000) {
                         return decode(bg_window_tile_data_offset + index*16,
                                       (line - wy) % 8, (wx - 7) % 8);
                       } else {
                         // add 0x800 to interpret the tile map index as a signed index starting in the middle
                         // of the tile data range (0x8800-97FF)
                         return decode(bg_window_tile_data_offset + 0x800 + (static_cast<signed char>(index))*16,
                                       (line - wy) % 8, (wx - 7) % 8);
                       }
                     });
      
      std::vector<PaletteIndex> raster_row = flatten(tile_data);
      
      // write to raster
      typedef std::vector<byte>::size_type diff;
      
      auto offset = static_cast<int>(wx - 7);
      std::copy_n(raster_row.begin(), 160 - offset, raster.begin() + offset);
    }
  }
  
  std::vector<RenderedSprite> visible;
  if (sprite_display) {
    std::array<byte, 160> sprite_row;
    std::fill(sprite_row.begin(), sprite_row.end(), 0);
    
//    compare_oams(oam, old_oam.data());
    // sprite OAM is at 0xfe00 - 0xfea0 (40 sprites, 4 bytes each)
    for (size_t j = 0; j < 40; ++j) {
      OAM entry = oam[j];
//      if (entry.x != 0)
//      std::cout << j << ": " << entry << std::endl;
  
      auto sprite_height = sprite_mode == SpriteMode::S8x8 ? 8 : 16;
  
      for (int x = 0; x < Screen::BUF_WIDTH; ++x) {
        if (entry.x != 0 && entry.x < 168 && entry.y != 0 && entry.y < 160 &&
            line + 16 >= entry.y && line + 16 < entry.y + sprite_height) {
          // sprite is visible on this line
          // get tile data for sprite
          
          auto tile_index = entry.tile_index;
          if (sprite_mode == SpriteMode::S8x16) {
            auto tile_y = (line - (entry.y - 16));
            if (tile_y < 8) {
              tile_index &= 0xfe;
            } else {
              tile_y -= 8;
              tile_index |= 0x01;
            }
          }
          
          // sprite tiles start at 0x8000 and go to 0x8fff, 16 bytes per tile (each 2 bytes represent one of the 8 rows)
          word tile_data = 0x8000 + tile_index * 16;
          
          // sprites are not necessarily aligned to the 8x8 grid
          // we need to be able to tell which line of the sprite
          // intersects the current scanline
          
          // if we are in this section, then entry.y - 16 <= line < entry.y - 8
          // (entry.y - 16 - line) % 8?
          
          // need to get the relevant row in the tile
          byte row_offset_within_tile = (line - (entry.y - 16)) % 8;
          if (entry.flags & (1 << 6)) { // y flip
            row_offset_within_tile = 8 - row_offset_within_tile - 1; // TODO: account for 8x16 tiles
          }
          word tile_data_address = tile_data + row_offset_within_tile * 2;
          
          byte b1 = cpu.mmu._read_mem(tile_data_address);
          byte b2 = cpu.mmu._read_mem(tile_data_address + 1);
          
          // for example, entry.y = 0x80, so the top left is at 0x80-0x10 = 0x70 = 112
          // when line = 115, we have 112 <= line < 120, so we are inside the sprite.
          // we are on row index 3 of the sprite, so the row offset is (line - (entry.y - 16)) % 8
          
          // Map the sprite indices through the palette map
          auto decoded = unpack_bits(b1, b2, 0);
          byte sprite_palette = (entry.flags & (1 << 4)) ? obp1 : obp0;
          std::transform(decoded.begin(), decoded.end(), decoded.begin(),
                         [sprite_palette](PaletteIndex pidx) {
                           switch (pidx) {
                             case 0:
                               return sprite_palette & 3;
                             case 1:
                               return (sprite_palette >> 2) & 3;
                             case 2:
                               return (sprite_palette >> 4) & 3;
                             case 3:
                               return (sprite_palette >> 6) & 3;
                             default:
                               throw std::runtime_error("invalid palette index");
                           }
                         });
          
          if (entry.flags & (1 << 5)) {
            std::reverse(decoded.begin(), decoded.end());
          }
          
          visible.emplace_back(RenderedSprite(entry, j, decoded));
          break;
        }
      }
    }

    std::sort(visible.begin(), visible.end(), [](RenderedSprite s1, RenderedSprite s2) {
      return s1.oam_.x == s2.oam_.x ? s1.oam_index_ < s2.oam_index_ : s1.oam_.x < s2.oam_.x;
    });
    
    int sprites_rendered = 0;
    for (auto sprite : visible) {
      if (sprites_rendered++ == 10) {
        break;
      }
      
      auto raster_ptr = raster.begin() + sprite.oam_.x - 8;
      auto sprite_ptr = sprite.pixels_.begin();

      bool sprite_behind_bg = (sprite.oam_.flags & (1 << 7)) != 0;
      
      int n = 0;
      while (raster_ptr < raster.end() && sprite_ptr < sprite.pixels_.end()) {
        // hack to prevent invalid array access when a sprite starts before column 0
        if (raster_ptr < raster.begin()) {
          ++raster_ptr; ++sprite_ptr;
          continue;
        }
        
        auto raster_byte = *raster_ptr;
        auto sprite_byte = *sprite_ptr;

        if (raster_byte == 0) {
          *raster_ptr = sprite_byte;
        } else {
          if (sprite_behind_bg && sprite_byte == 0) {
            ;
          } else if (sprite_byte != 0) {
            *raster_ptr = sprite_byte;
          }
        }

        ++raster_ptr; ++sprite_ptr;
        ++n;
      }
//      std::cout << "wrote " << n << " bytes to raster starting at offset " << static_cast<void*>(raster.begin() + sprite.oam_.x - 8 ) << "(begin=" << static_cast<void*>(raster.begin()) << ", end=" << static_cast<void*>(raster.end()) << ")" << std::endl;
    }
    
    std::copy(oam, oam + 40, old_oam.begin());
  }
  
  
}

std::vector<PPU::PaletteIndex>
PPU::decode(word start_loc, byte start_y, byte start_x) {
  // start_y is from 0 to 7
  // we want row start_y of the tile
  // 2 bytes per row, 8 rows
  
  byte b1 = cpu.mmu._read_mem(start_loc + start_y*2);
  byte b2 = cpu.mmu._read_mem(start_loc + start_y*2 + 1);
  
  // b1/b2 is packed:
  // b1            b2
  // 00 01 10 11 | 10 11 00 10 LSB
  // 01 00 10 10 | 10 10 01 01 MSB
  // -------------------------
  // 02 02 30 31   30 31 02 12 <- we want to get this sequence
  
  return unpack_bits(b1, b2, start_x);
}

std::vector<PPU::PaletteIndex>
PPU::unpack_bits(byte lsb, byte msb, byte start_x) {
  std::vector<PPU::PaletteIndex> result(8, 0);
  
  for (int i = 7; i >= 0; --i) {
    byte m = msb & 0x1;
    byte l = lsb & 0x1;
    
    result[i] = (m << 1) | l;
    
    lsb >>= 1; msb >>= 1;
  }
  
  // trippy
//  std::rotate(result.begin(), result.begin() + start_x, result.end());
  
  return result;
}

inline void
PPU::set_lcd_on(bool on) {
//  std::cout << "lcd " << on << " at 0x" << hex << setw(4) << setfill('0') << cpu.pc << std::endl;
  lcd_on = on;
}

inline void
PPU::set_window_tilemap_offset(word offset) {
  window_tilemap_offset = offset;
}

inline void
PPU::set_bg_window_tile_data_offset(word offset) {
  bg_window_tile_data_offset = offset;
}

inline void
PPU::set_window_display(bool on) {
  window_display = on;
}

inline void
PPU::set_bg_tilemap_offset(word offset) {
  bg_tilemap_offset = offset;
}

inline void
PPU::set_sprite_mode(SpriteMode mode) {
  sprite_mode = mode;
}

inline void
PPU::set_sprite_display(bool on) {
  sprite_display = on;
}

inline void
PPU::set_bg_display(bool on) {
  bg_display = on;
}
