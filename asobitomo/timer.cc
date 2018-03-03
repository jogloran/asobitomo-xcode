#include "timer.h"
#include "cpu.h"

void
Timer::inc_tima() {
  if (counter == 0xff) {
    counter = modulo;
    fire_timer_interrupt();
  } else {
    ++counter;
  }
}

void
Timer::step(long delta) {
  // increment divider_hi once every 256 cycles
  divider += delta;
  divider_hi = divider >> 8;
  
  if (enabled) {
    counter_cycles += delta;
    if (counter_cycles >= speed) {
      inc_tima();
//      std::cout << "counter: " << dec << static_cast<int>(counter) << ", cycles = " << dec << counter_cycles << " (+" << delta << ")" << std::endl;
      counter_cycles -= speed;
    }
  }
}

byte&
Timer::div() {
  return divider_hi;
}

void
Timer::reset_div() {
  divider = 0;
}

void
Timer::fire_timer_interrupt() {
  byte IF = cpu.mmu[0xff0f];
  IF |= 1 << 2;
  cpu.mmu.set(0xff0f, IF);
}

byte&
Timer::tac() {
  return tac_;
}

void
Timer::set_tac(byte value) {
  enabled = (value & 0x4) == 0x4;
  
  byte frequency_selector = value & 0x3;
  switch (frequency_selector) {
  case 0:
    speed = 1024;
    break;
  case 1:
    speed = 256;
    break;
  case 2:
    speed = 64;
    break;
  case 3:
    speed = 16;
    break;
  default:
    break;
  }
  
  std::cout << "Timer setting speed: " << static_cast<int>(frequency_selector) << std::endl;
  tac_ = value;
}
