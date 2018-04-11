#pragma once

#include <array>
#include <stdexcept>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <memory>

using namespace std;

#include "sdl_input.h"
#include "types.h"
#include "ppu.h"
#include "apu.h"
#include "timer.h"
#include "mbc_types.h"
#include "mbc_base.h"
#include "header_type.h"
#include "rang.hpp"
#include "util.h"

extern bool in_title;

class MMU {
public:
  MMU(std::string filename, PPU& ppu, APU& apu, Timer& timer):
    path(filename),
    cart(32768, 0), rom_mapped(true), ppu(ppu), apu(apu), timer(timer),
    joypad(0xf),
    input(), mbc() {
    fill(mem.begin(), mem.end(), 0);
    std::copy(rom.begin(), rom.end(), mem.begin());
    
    mem[0xf000] = 0xff;

    byte header_bytes[0x50];
    std::ifstream f(path);
    f.seekg(0x100);
    f.read((char*)header_bytes, 0x50);
    Header* h = reinterpret_cast<Header*>(header_bytes);
    header = *h;
      
    long rom_size = 1 << (15 + h->rom_size);
    cart.resize(rom_size);
    
    MBC cartridge_type = h->cartridge_type;
    mbc = mbc_for(cartridge_type, *this);
    auto load_path = replace_path_extension(path, ".gb", ".sav");
    mbc->load(load_path);

    ppu.screen->add_exit_handler([this]() {
      auto sav_path = replace_path_extension(path, ".gb", ".sav");
      mbc->save(sav_path);
    });
      
    f.seekg(0);
    f.read((char*)cart.data(), rom_size);
  }

  void set(word loc, byte value);

  byte& operator[](word loc);
  
  void dump_cartridge_info() {
    std::cout << rang::style::dim << rang::fg::gray << "Title\t\t" << rang::fg::black << rang::style::reset << (char*)header.title_or_manufacturer.title << rang::fg::reset << rang::style::reset << std::endl;
    std::cout << rang::style::dim << rang::fg::gray << "Type\t\t" << rang::fg::black << rang::style::reset  << header.cartridge_type << rang::fg::reset << rang::style::reset << std::endl;
    std::cout << rang::style::dim << rang::fg::gray << "ROM\t\t" << rang::fg::black << rang::style::reset << (1 << (15 + header.rom_size)) << rang::fg::reset << rang::style::reset << std::endl;
    std::cout << rang::style::dim << rang::fg::gray << "RAM\t\t" << rang::fg::black << rang::style::reset << static_cast<int>(header.ram_size) << rang::fg::reset << rang::style::reset << std::endl;
    std::cout << rang::style::dim << rang::fg::gray << "SGB\t\t" << rang::fg::black << rang::style::reset << static_cast<int>(header.sgb == 0x3) << rang::fg::reset << rang::style::reset << std::endl;
    std::cout << rang::style::dim << rang::fg::gray << "NJP\t\t" << rang::fg::black << rang::style::reset  << static_cast<int>(header.destination) << rang::fg::reset << rang::style::reset << std::endl;
  }
  
  void handle_joypad();

// private:
  static constexpr word CARTRIDGE_TYPE_OFFSET = 0x0147;

  const std::string path;

  PPU& ppu;
  APU& apu;
  Timer& timer;
  
  std::array<byte, 0x100> rom;
  std::vector<byte> cart;
  
  static constexpr int RAM_BYTES = 65536;
  std::array<byte, RAM_BYTES> mem;

  bool rom_mapped;
  byte joypad;
  
  SDLInput input;
  
  Header header;
  
  char last;
  
  std::unique_ptr<MBCBase> mbc;
};



