#include "ppu.h"
#include "cpu.h"

void
PPU::update_stat_register()  {
  byte lyc = cpu.mmu[0xff45];

  byte stat = cpu.mmu[0xff41];
  if (lyc == line) {
    stat |= 0x4; // coincidence flag (bit 2)
  } else {
    stat &= 0xfb;
  }
  
  cpu.mmu[0xff41] = static_cast<byte>(mode);
  cpu.mmu[0xff44] = static_cast<byte>(line);
  // cpu.mmu.set(0xff41, static_cast<byte>(stat));
  // cpu.mmu.set(0xff44, static_cast<byte>(line));
  
  if ((stat & 0x40) && (lyc == line)) { // lyc=ly coincidence interrupt
    cpu.fire_interrupts();
  }
  
  if ((stat & 0x20) && mode == Mode::OAM) {
    cpu.fire_interrupts();
  }
  if ((stat & 0x10) && mode == Mode::VBLANK) {
    cpu.fire_interrupts();
  }
  if ((stat & 0x08) && mode == Mode::HBLANK) {
    cpu.fire_interrupts();
  }
}
