#include "mbc1.h"

#include "mmu.h"

int
MBC1::bank_no() {
  return (bank_hi << 5) + bank;
}

byte*
MBC1::get(word loc) {
  if (loc <= 0x1fff) {
    return &mmu.cart[loc];
  } else if (loc <= 0x3fff) {
    return &mmu.cart[loc];
  } else if (loc <= 0x5fff) {
    int full_bank = (bank_hi << 5) + bank; // TODO: need wraparound behaviour?
    if (full_bank > 0x1f) full_bank = 0x1f;
    return &mmu.cart[full_bank * 0x4000 + (loc - 0x4000)];
  } else if (loc <= 0x7fff) {
    int full_bank = (bank_hi << 5) + bank; // TODO: need wraparound behaviour?
    if (full_bank > 0x1f) full_bank = 0x1f;
    return &mmu.cart[full_bank * 0x4000 + (loc - 0x4000)];
  } else if (loc >= 0xa000 && loc <= 0xbfff) {
    return &external_ram[loc - 0xa000];
  }
  
  return nullptr;
}

bool
MBC1::set(word loc, byte value) {
  if (loc <= 0x1fff) {
    if ((value & 0xf) == 0xa) {
      // enable
      external_ram_enabled = true;
    } else {
      // disable (default)
      external_ram_enabled = false;
    }
    
    return true; // Do not actually modify RAM
  } else if (loc <= 0x3fff) {
    if (value == 0x00) {
      bank = 1;
    } else if (value == 0x20 || value == 0x40 || value == 0x60) {
      bank = value + 0x1; // requesting banks 20h, 40, 60 gets 21h, 41, 61 instead
    } else {
      bank = value & 0x1f;
    }
    
    return true;
  } else if (loc <= 0x5fff) {
    if (select_external_ram) {
      ram_bank = value & 0x3;
    } else {
      // Is the right behaviour to ignore values that
      // are out of range?
      if (value <= 3) {
        bank_hi = value & 0x3;
      }
    }
    return true;
  } else if (loc <= 0x7fff) {
    select_external_ram = value & 0x1;
    return true;
  } else if (loc >= 0xa000 && loc <= 0xbfff) {
    external_ram[loc - 0xa000] = value;
    return true;
  }
  
  return false;
}
