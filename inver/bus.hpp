#pragma once

#include <memory>
#include <array>

#include "types.h"
#include "cpu6502.hpp"

class Bus {
public:
  Bus(std::shared_ptr<CPU6502> cpu): cpu(cpu) {
    cpu->connect(this);
    
    std::fill(cart.begin(), cart.end(), 0);
    std::fill(ram.begin(), ram.end(), 0);
  }
  
  void map(byte* ptr, size_t len);
  
  byte read(word addr);
  void write(word addr, byte value);
  
public:
  std::shared_ptr<CPU6502> cpu;
  
  std::array<byte, 0x4000> cart;
  std::array<byte, 0x0800> ram;
};

