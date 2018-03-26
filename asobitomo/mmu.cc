
#include "mmu.h"

#include "mbc1.h"
#include "mbc3.h"

byte& MMU::_read_mem(word loc) {
  byte* result = mbc->get(loc);
  if (result != nullptr) {
    return *result;
  }
  
  // TODO: ff03 should have the lower byte of timer div
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
    handle_joypad();
    mem[loc] = joypad;
    return joypad;
  }
  
  if (loc <= 0x00ff) {
    if (rom_mapped)
      return rom[loc];
    else
      return cart[loc];
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

void MMU::set(word loc, byte value) {
  if (loc == 0x1e21) {
  ;
  }
  if (mbc->set(loc, value)) {
    return;
  }
  mem[loc] = value;
  
  // serial write
  if (loc == 0xff01) {
    last = value;
  }
  // serial read
  if (loc == 0xff02) {
//    std::cout << (char)last;
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
  
  if (loc == 0xff40) { // LCD stat
    ppu.stat(value);
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

void
MMU::handle_joypad() {
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
}
