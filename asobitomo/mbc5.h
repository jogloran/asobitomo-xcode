#pragma once

#include <array>
#include <algorithm>

#include "types.h"
#include "mbc_base.h"

class MMU;

class MBC5 : public MBCBase {
public:
  MBC5(MMU& mmu) :
    mmu(mmu),
    nbanks(1),
    bank(1), // default bank for 0x4000 is 1, so that 0x4000 acccesses 0x4000
    bank_hi(0),
    ram_bank(0), select_external_ram(false), external_ram_enabled(false),
    external_ram() {
    std::fill(external_ram.begin(), external_ram.end(), 0);
  }
  
  byte* get(word loc);
  bool set(word loc, byte value);
  
  int bank_no();
  
  MMU& mmu;
  
  int nbanks;
  byte bank;
  byte bank_hi;
  byte ram_bank;
  bool select_external_ram;
  bool external_ram_enabled;
  
  std::array<byte, 0x2000 * 16> external_ram;
};
