#include "ppu.h"
#include "cpu.h"
#include "util.h"
#include "ppu_util.h"

#include <numeric>
#include <iterator>
#include <algorithm>
#include <sstream>

template <typename T, unsigned long A, unsigned long B> void
flatten(const std::array<std::array<T, A>, B>& in, typename std::array<T, A*B>::iterator begin) {
  auto ptr = begin;
  for (auto it = in.begin(); it != in.end(); ++it) {
    ptr = std::copy(it->begin(), it->end(), ptr);
  }
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
  
  // Dr Mario seems to wait for vblank while in HALT, which leads to an infinite loop
  // unless the PPU is still active during LCD off (why would Dr Mario turn the LCD off?)
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
        
        rasterise_line();
        screen->set_row(line, raster.begin(), raster.end());
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
          screen->blit();
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
  byte lyc = cpu.mmu.mem[0xff45];

  byte stat = cpu.mmu.mem[0xff41];
  if (lyc == line) {
    stat |= 0x4; // coincidence flag (bit 2)
  } else {
    stat &= ~0x4;
  }
  
  cpu.mmu.mem[0xff41] = (stat & 0xfc) | static_cast<byte>(mode);
  cpu.mmu.mem[0xff44] = static_cast<byte>(line); // Not sure why, but this has to be static_cast<byte> and not just
  
  byte IF = cpu.mmu.mem[0xff0f];
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
  
  cpu.mmu.mem[0xff0f] = IF;
}

std::ostream& operator<<(std::ostream& out, const OAM& oam) {
  return out << "OAM(" << hex << static_cast<int>(oam.x) << ", "
    << static_cast<int>(oam.y)
    << ", tile_index = " << setw(2) << hex << static_cast<int>(oam.tile_index)
    << ", flags = " << binary(oam.flags) << ")";
}

PPU::TileRow
PPU::tilemap_index_to_tile(byte index, byte y_offset) {
  // There are 0x1000 bytes of tile data -- each entry is 0x10 bytes, so there are 0x100 entries
  // This takes each tile map index and retrieves
  // the corresponding line of the corresponding tile
  // 2 bytes per row, 16 bytes per tile
  // in each row, first byte is LSB of palette indices
  //              second byte is MSB
  if (bg_window_tile_data_offset == 0x8000) {
    // data ranges from 0x8000 (index 0) to 0x8fff (index 0xff)
    return decode(bg_window_tile_data_offset + index*16,
                  y_offset);
  } else {
    // add 0x800 to interpret the tile map index as a signed index starting in the middle
    // of the tile data range (0x8800-97FF)
    // data ranges from 0x8800 (index -127) to 0x9000 (index 0) to 0x97ff (index 128)
    return decode(bg_window_tile_data_offset + 0x800 + (static_cast<signed char>(index))*16,
                  y_offset);
  }
}

void
PPU::rasterise_line() {
  // OAM is at 0xfe00 - 0xfea0 (40 sprites, 4 bytes each)
  byte* oam_ptr = &cpu.mmu.mem[0xfe00];
  OAM* oam = reinterpret_cast<OAM*>(oam_ptr);
  
  byte scx = cpu.mmu.mem[0xff43];
  byte scy = cpu.mmu.mem[0xff42];

  // For the sprite palettes, the lowest two bits should always map to 0
  byte obp0 = cpu.mmu.mem[0xff48] & 0xfc;
  byte obp1 = cpu.mmu.mem[0xff49] & 0xfc;
  
  // Background palette
  byte palette = cpu.mmu.mem[0xff47];

  if (bg_display) {
    auto row_touched = ((line + scy) % 256) / 8; // % 256 for scy wrap around
  
    // Create sequence of tiles to use (% 32 to wrap around)
    // Note that we actually take 21 tiles, because if
    // scx % 8 != 0, the raster may actually span part
    // of the first tile and part of the last
    auto starting_index = scx / 8;

    // Equivalent to:
    // 0<=i<=21, row_tiles[i] = cpu.mmu[bg_tilemap_offset + row_touched * 32 + ((starting_index + i) % 32)];
    auto* base = &cpu.mmu[bg_tilemap_offset + row_touched * 32];
    auto n_copied_from_end = std::min(21, 32 - starting_index);
    auto cur = std::copy_n(base + starting_index, n_copied_from_end, row_tiles.begin());
    std::copy_n(base, 21 - n_copied_from_end, cur);
    
    std::transform(row_tiles.begin(), row_tiles.end(), tile_data.begin(), [this, scx, scy](byte index) {
      return tilemap_index_to_tile(index, (line + scy) % 8);
    });
    
    flatten(tile_data, raster_row.begin());
    
    auto offset = static_cast<int>(scx % 8);

    auto fin = std::copy(raster_row.begin() + offset, raster_row.end(), palette_index_row.begin());
    std::copy(raster_row.begin(), raster_row.begin() + offset, fin);
    
    std::transform(palette_index_row.begin(), palette_index_row.end(),
      raster.begin(), [palette](PaletteIndex idx) {
      return apply_palette(idx, palette);
    });
  }
  
  if (window_display) {
    byte wx = cpu.mmu.mem[0xff4b];
    byte wy = cpu.mmu.mem[0xff4a];
    
    if (line >= wy) {
      byte row_touched = (line - wy) / 8;

      auto* base = &cpu.mmu[window_tilemap_offset + row_touched * 32];
      std::transform(base, base + 20, tile_data.begin(), [this, wx, wy](byte index) {
        return tilemap_index_to_tile(index, (line - wy) % 8);
      });
      
      flatten(tile_data, raster_row.begin());

      std::transform(raster_row.begin(), raster_row.end(), raster_row.begin(), [palette](PaletteIndex idx) {
        return apply_palette(idx, palette);
      });
      
      // write to raster
      auto offset = static_cast<int>(wx - 7);
      std::copy_n(raster_row.begin(), 160 - std::max(0, offset), raster.begin() + std::max(0, offset));
    }
  }
  
  visible.clear();
  if (sprite_display) {
    auto sprite_height = sprite_mode == SpriteMode::S8x8 ? 8 : 16;
    
    for (size_t j = 0; j < 40; ++j) {
      OAM entry = oam[j];
  
      for (int x = 0; x < Screen::BUF_WIDTH; ++x) {
        if (entry.x != 0 && entry.x < 168 && entry.y != 0 && entry.y < 160 &&
            line + 16 >= entry.y && line + 16 < entry.y + sprite_height) {
          
          auto tile_y = line - (entry.y - 16);
          if (entry.flags & (1 << 6)) { // y flip
            tile_y = sprite_height - tile_y - 1;
          }
          
          auto tile_index = entry.tile_index;
          if (sprite_mode == SpriteMode::S8x16) {
            if (tile_y < 8) {
              tile_index &= 0xfe;
            } else {
              tile_y -= 8;
              tile_index |= 0x01;
            }
          }
          
          word tile_data_begin = 0x8000 + tile_index * 16;
          word tile_data_address = tile_data_begin + tile_y * 2;
          
          auto decoded = decode(tile_data_address);

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
      
      auto bg_palette_index_ptr = palette_index_row.begin() + sprite.oam_.x - 8;
      auto raster_ptr = raster.begin() + sprite.oam_.x - 8;
      auto sprite_ptr = sprite.pixels_.begin();
      
      byte sprite_palette = (sprite.oam_.flags & (1 << 4)) ? obp1 : obp0;

      // If set to 0, sprite is always in front of bkgd and window
      // If set to 1, if background or window is colour 1, 2, 3, background or window wins
      //              else if background or window is color 0, sprite wins
      bool sprite_behind_bg = (sprite.oam_.flags & (1 << 7)) != 0;
      
      while (raster_ptr < raster.end() && sprite_ptr < sprite.pixels_.end()) {
        // hack to prevent invalid array access when a sprite starts before column 0
        if (raster_ptr >= raster.begin()) {
          // We need to examine the original palette byte, since the bg-to-OBJ
          // priority bit in LCDC needs to examine the original palette index
          // (and not the index after palette mapping)
          
          auto sprite_byte = *sprite_ptr; // Sprite palette index
          auto bg_palette_byte = *bg_palette_index_ptr; // Background palette index
          
          if (!(sprite_behind_bg && bg_palette_byte != 0)) {
            PPU::PaletteIndex idx = apply_palette(sprite_byte, sprite_palette);
            if (sprite_byte != 0) {
              *raster_ptr = idx;
            }
          }
        }
        
        ++raster_ptr; ++sprite_ptr; ++bg_palette_index_ptr;
      }
    }
  }
}

PPU::TileRow
PPU::decode(word start_loc, byte start_y) {
  // start_y is from 0 to 7
  // we want row start_y of the tile
  // 2 bytes per row, 8 rows
  
  byte b1 = cpu.mmu[start_loc + start_y*2];
  byte b2 = cpu.mmu[start_loc + start_y*2 + 1];
  
  // b1/b2 is packed:
  // b1            b2
  // 00 01 10 11 | 10 11 00 10 LSB
  // 01 00 10 10 | 10 10 01 01 MSB
  // -------------------------
  // 02 02 30 31   30 31 02 12 <- we want to get this sequence
  
  return unpack_bits(b1, b2);
}

static const uint16_t m[256] =
{
  0x0000, 0x0001, 0x0004, 0x0005, 0x0010, 0x0011, 0x0014, 0x0015,
  0x0040, 0x0041, 0x0044, 0x0045, 0x0050, 0x0051, 0x0054, 0x0055,
  0x0100, 0x0101, 0x0104, 0x0105, 0x0110, 0x0111, 0x0114, 0x0115,
  0x0140, 0x0141, 0x0144, 0x0145, 0x0150, 0x0151, 0x0154, 0x0155,
  0x0400, 0x0401, 0x0404, 0x0405, 0x0410, 0x0411, 0x0414, 0x0415,
  0x0440, 0x0441, 0x0444, 0x0445, 0x0450, 0x0451, 0x0454, 0x0455,
  0x0500, 0x0501, 0x0504, 0x0505, 0x0510, 0x0511, 0x0514, 0x0515,
  0x0540, 0x0541, 0x0544, 0x0545, 0x0550, 0x0551, 0x0554, 0x0555,
  0x1000, 0x1001, 0x1004, 0x1005, 0x1010, 0x1011, 0x1014, 0x1015,
  0x1040, 0x1041, 0x1044, 0x1045, 0x1050, 0x1051, 0x1054, 0x1055,
  0x1100, 0x1101, 0x1104, 0x1105, 0x1110, 0x1111, 0x1114, 0x1115,
  0x1140, 0x1141, 0x1144, 0x1145, 0x1150, 0x1151, 0x1154, 0x1155,
  0x1400, 0x1401, 0x1404, 0x1405, 0x1410, 0x1411, 0x1414, 0x1415,
  0x1440, 0x1441, 0x1444, 0x1445, 0x1450, 0x1451, 0x1454, 0x1455,
  0x1500, 0x1501, 0x1504, 0x1505, 0x1510, 0x1511, 0x1514, 0x1515,
  0x1540, 0x1541, 0x1544, 0x1545, 0x1550, 0x1551, 0x1554, 0x1555,
  0x4000, 0x4001, 0x4004, 0x4005, 0x4010, 0x4011, 0x4014, 0x4015,
  0x4040, 0x4041, 0x4044, 0x4045, 0x4050, 0x4051, 0x4054, 0x4055,
  0x4100, 0x4101, 0x4104, 0x4105, 0x4110, 0x4111, 0x4114, 0x4115,
  0x4140, 0x4141, 0x4144, 0x4145, 0x4150, 0x4151, 0x4154, 0x4155,
  0x4400, 0x4401, 0x4404, 0x4405, 0x4410, 0x4411, 0x4414, 0x4415,
  0x4440, 0x4441, 0x4444, 0x4445, 0x4450, 0x4451, 0x4454, 0x4455,
  0x4500, 0x4501, 0x4504, 0x4505, 0x4510, 0x4511, 0x4514, 0x4515,
  0x4540, 0x4541, 0x4544, 0x4545, 0x4550, 0x4551, 0x4554, 0x4555,
  0x5000, 0x5001, 0x5004, 0x5005, 0x5010, 0x5011, 0x5014, 0x5015,
  0x5040, 0x5041, 0x5044, 0x5045, 0x5050, 0x5051, 0x5054, 0x5055,
  0x5100, 0x5101, 0x5104, 0x5105, 0x5110, 0x5111, 0x5114, 0x5115,
  0x5140, 0x5141, 0x5144, 0x5145, 0x5150, 0x5151, 0x5154, 0x5155,
  0x5400, 0x5401, 0x5404, 0x5405, 0x5410, 0x5411, 0x5414, 0x5415,
  0x5440, 0x5441, 0x5444, 0x5445, 0x5450, 0x5451, 0x5454, 0x5455,
  0x5500, 0x5501, 0x5504, 0x5505, 0x5510, 0x5511, 0x5514, 0x5515,
  0x5540, 0x5541, 0x5544, 0x5545, 0x5550, 0x5551, 0x5554, 0x5555
};

inline PPU::TileRow
PPU::unpack_bits(byte lsb, byte msb) {
  PPU::TileRow result;
  
  const uint64_t C = m[msb] << 1 | m[lsb];
  uint64_t data = (C & 0xc000) >> 14 |
                  (C & 0x3000) >> 4  |
                  (C & 0x0c00) << 6  |
                  (C & 0x0300) << 16 |
                  (C &   0xc0) << 26 |
                  (C &   0x30) << 36 |
                  (C &   0x0c) << 46 |
                  (C &   0x03) << 56;
  uint64_t* ptr = (uint64_t*)result.data();
  *ptr = data;
  return result;
}

inline void
PPU::set_lcd_on(bool on) {
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

inline PPU::PaletteIndex apply_palette(PPU::PaletteIndex pidx, byte sprite_palette) {
  return (sprite_palette >> (pidx * 2)) & 3;
}

