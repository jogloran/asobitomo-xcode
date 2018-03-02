#pragma once

#include <array>
#include <stdexcept>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>

using namespace std;

#include "sdl_input.h"
#include "types.h"
#include "ppu.h"
#include "timer.h"

extern bool in_title;

class MMU {
public:
  std::vector<int> accessed;

  MMU(std::string path, PPU& ppu, Timer& timer):
    accessed(0xff7f - 0xff00, 0), bank(0), mem(),
    cart(32768, 0), rom_mapped(true), ppu(ppu), timer(timer),
    joypad(0xf),
    input() {
    fill(mem.begin(), mem.end(), 0);
    std::copy(rom.begin(), rom.end(), mem.begin());
    
    mem[0xf000] = 0xff;

    std::ifstream f(path);
    f.read((char*)cart.data(), 32768);
  }

  void set(word loc, byte value) {
    // std::cout << hex<<loc << std::endl;
    if (loc >= 0xff00 && loc <= 0xff7f) {
      accessed[loc - 0xff00]++;
    }
    
    mem[loc] = value;
    
    if (loc == 0xff01) {
      last = value;
    }
    if (loc == 0xff02) {
      std::cout << (char)last;
    }
    
    if (loc == 0xff04) { // timer DIV
      timer.reset_div();
    }
    if (loc == 0xff05) { // timer counter
      timer.counter = value;
    }
    if (loc == 0xff06) {
      timer.modulo = value;
    }
    if (loc == 0xff07) {
      timer.set_tac(value);
    }
    
    if (loc >= 0x2000 && loc <= 0x3fff) {
      if (value == 0x00) {
        bank = 1;
      } else {
        bank = value & 0x1f;
      }
    }
    
    if (loc == 0xff40) {
      // LCD stat
      ppu.set_lcd_on(value & (1 << 7));
      ppu.set_window_tilemap_offset(value & (1 << 6) ? 0x9c00 : 0x9800);
      ppu.set_window_display(value & (1 << 5));
      ppu.set_bg_window_tile_data_offset(value & (1 << 4) ? 0x8000 : 0x8800);
      ppu.set_bg_tilemap_offset(value & (1 << 3) ? 0x9c00 : 0x9800);
      ppu.set_sprite_mode(value & (1 << 2) ? PPU::SpriteMode::S8x16 : PPU::SpriteMode::S8x8);
      ppu.set_sprite_display(value & (1 << 1));
      ppu.set_bg_display(value & 0x1);
    }
    
    if (loc == 0xff00) { // joypad
      mem[loc] = value | 0xf;
    }

    if (loc == 0xff46) { // DMA
      word src = value << 8;
      for (word addr = 0xfe00; addr < 0xfea0; ++addr) {
        (*this)[addr] = (*this)[src++];
      }
    }
      
    if (loc == 0xff50) { // unmap rom
      rom_mapped = false;
    }
  }

  byte& operator[](int loc) {
    if (loc < 0x0000 || loc > RAM_BYTES) {
      throw std::range_error("invalid location");
    }

    byte& result = _read_mem(loc);
    // cerr << "mmu["
    //   << setfill('0') << setw(4) <<
    //   hex << loc << dec
    //   << "] = " << setfill('0') << setw(2) <<
    //   hex << static_cast<int>(result) << dec << endl;

    return result;
  }

  // const byte& operator[](word loc) const;
  byte& _read_mem(word loc) {
    if (loc >= 0xff00 && loc <= 0xff7f) {
      accessed[loc - 0xff00]++;
    }
    
    if (loc == 0xff04) { // timer DIV
      return timer.div();
    }
    if (loc == 0xff05) { // timer counter
      return timer.counter;
    }
    if (loc == 0xff06) {
      return timer.modulo;
    }
    if (loc == 0xff07) {
      return timer.tac();
    }

    if (loc == 0xff46) { // DMA
      return mem[loc];
    }
    
    if (loc == 0xff00) { //joypad
      input.poll();
      
      byte key_input = 0xf;
      
      byte value = mem[0xff00] | 0xf;
      if ((value & 0x20) == 0) {
        if ((input.state & Buttons::Start) == Buttons::Start) {
          key_input ^= 0x8;
        }
        if ((input.state & Buttons::Select) == Buttons::Select) {
          key_input ^= 0x4;
        }
        if ((input.state & Buttons::B) == Buttons::B) {
          key_input ^= 0x2;
        }
        if ((input.state & Buttons::A) == Buttons::A) {
          key_input ^= 0x1;
        }
      } else if ((value & 0x10) == 0) {
        if ((input.state & Buttons::D) == Buttons::D) {
          key_input ^= 0x8;
        }
        if ((input.state & Buttons::U) == Buttons::U) {
          key_input ^= 0x4;
        }
        if ((input.state & Buttons::L) == Buttons::L) {
          key_input ^= 0x2;
        }
        if ((input.state & Buttons::R) == Buttons::R) {
          key_input ^= 0x1;
        }
      }
      
      joypad = (0xf0 & value) | key_input;
//      std::cout << "joypad read: " << hex << static_cast<int>(joypad) << std::endl;
      mem[loc] = joypad;
      return joypad;
    }

    if (loc <= 0x00ff) {
      if (rom_mapped)
        return rom[loc];
      else
        return cart[loc];
    } else if (loc <= 0x014f) {
      return cart[loc]; /* header */
    } else if (loc <= 0x3fff) {
      return cart[loc]; /* rom bank 0 0x150 - 0x3fff */
    } else if (loc <= 0x7fff) {
      /* rom bank switchable 0x4000 - 0x7fff */
//      return cart[bank * 0x4000 + loc];
      return cart[loc]; /* TODO: this only works on non-MBC */
    } else if (loc <= 0x97ff) {
      return mem[loc]; /* RAM 0x8000 - 0x97ff */
    } else if (loc <= 0x9bff) {
      /* 0x9800 - 0x9bff */
      return mem[loc]; /* is this right? */
    } else if (loc <= 0x9fff) {
      /* 0x9c00 - 0x9fff */
      return mem[loc]; /* is this right? */
    } else if (loc <= 0xbfff) {
      /* 0xa000 - 0xbfff */
      return mem[loc]; /* is this right? */
    } else if (loc <= 0xcfff) {
      return mem[loc]; /* 0xc000 - 0xcfff */
    } else if (loc <= 0xdfff) {
      return mem[loc]; /* 0xd000 - 0xdfff */
    } else if (loc <= 0xfdff) {
      /* 0xe000 - 0xfdff ==
         0xc000 - 0xcdff */
      return mem[loc - 0x2000];
    } else if (loc <= 0xfe9f) {
      return mem[loc]; /* 0xfe00 - 0xfe9f */
    } else {
      return mem[loc];
    }
  }

// private:
  static constexpr int RAM_BYTES = 65536;
  static constexpr word CARTRIDGE_TYPE_OFFSET = 0x0147;

  PPU& ppu;
  Timer& timer;
  byte bank;
  std::array<byte, RAM_BYTES> mem;
  std::vector<byte> rom {
    0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB,
    0x21, 0x26, 0xFF, 0x0E, 0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3,
    0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0, 0x47, 0x11, 0x04, 0x01,
    0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
    0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22,
    0x23, 0x05, 0x20, 0xF9, 0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99,
    0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20, 0xF9, 0x2E, 0x0F, 0x18,
    0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
    0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20,
    0xF7, 0x1D, 0x20, 0xF2, 0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62,
    0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06, 0x7B, 0xE2, 0x0C, 0x3E,
    0x87, 0xF2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
    0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17,
    0xC1, 0xCB, 0x11, 0x17, 0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9,
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83,
    0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
    0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63,
    0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
    0x3c, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x4C, 0x21, 0x04, 0x01, 0x11,
    0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
    0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE,
    0x3E, 0x01, 0xE0, 0x50
  };
  std::vector<byte> cart;

  bool rom_mapped;
  byte joypad;
  
  SDLInput input;
  
  int i = 0;
  char last;
};

