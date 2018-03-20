
#include "mmu.h"

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
  //      std::cout << "joypad read: " << hex << static_cast<int>(joypad) << std::endl;
}
