#include "timer.h"
#include "cpu.h"

void
Timer::step(long delta) {
  divider += delta;
  divider_hi = divider >> 8;
  
  if (!enabled) return;
  
  bool overflowed = (divider & ((1 << speed) - 1)) == 0;
  if (overflowed) {
    ++counter;
    if (counter == 0) {
      fire_timer_interrupt();
      counter = modulo;
    }
  }
}

byte&
Timer::div() {
  return divider_hi;
}

void
Timer::reset_div() {
  divider_hi = divider = 0;
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
    speed = 4;
    break;
  case 1:
    speed = 10;
    break;
  case 2:
    speed = 8;
    break;
  case 3:
    speed = 6;
    break;
  default:
    break;
  }
  
  tac_ = value;
}
