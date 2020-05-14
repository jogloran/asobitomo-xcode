#include "bus.hpp"

void
Bus::map(byte *ptr, size_t len) {
  std::copy(ptr, ptr + len, cart.begin());
}

void
Bus::write(word addr, byte value) {
  if (addr <= 0x7ff) {
    ram[addr] = value;
  } else if (addr <= 0x1fff) {
    ram[addr % 0x800] = value;
  } else if (addr >= 0x8000 && addr <= 0xbfff) {
    cart[addr - 0x8000] = value;
  } else if (addr >= 0xc000 && addr <= 0xffff) {
    cart[addr - 0xc000] = value;
  }
}

byte
Bus::read(word addr) {
  if (addr <= 0x7ff) {
    return ram[addr];
  } else if (addr <= 0x1fff) {
    return ram[addr % 0x800];
  } else if (addr <= 0x3fff) { // ppu
    return 0;
  } else if (addr <= 0x4017) { // apu
    return 0;
  } else if (addr <= 0x401f) { // apu and I/O
    return 0;
  } else if (addr >= 0x8000 && addr <= 0xbfff) {
    return cart[addr - 0x8000];
  } else if (addr >= 0xc000 && addr <= 0xffff) {
    return cart[addr - 0xc000];
  }
  
  throw std::range_error("out of range read");
}
