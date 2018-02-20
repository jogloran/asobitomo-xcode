#include <iostream>
#include <iomanip>
#include <fstream>
#include "types.h"
#include "cpu.h"

using namespace std;

int main() {
  CPU cpu("/Users/jogloran-s/my/asobitomo/Tetris.gb");

  copy(cpu.mmu.rom.begin(), cpu.mmu.rom.end(), cpu.mmu.mem.begin());

  while (!cpu.halted && cpu.pc != 0x100) {
    // bool debug = cpu.mmu[0xff44] >= 143 && cpu.mmu[0xff44] <= 153;
    cpu.step(false);
  }
  //
  int i = 0;
  while (i++ <= 100000000) {
//    cpu.step(i >= 100000000 - 1000);
  if (cpu.pc == 0x021b) {
  ;
  }
    cpu.step(false);
  }

  cpu.dump_state();

  std::ofstream out("vram");
  copy(cpu.mmu.mem.begin() + 0x8000, cpu.mmu.mem.begin() + 0xa000, ostream_iterator<unsigned int>(out, " "));
  out.close();
  
  out.open("tiledata");
  copy(cpu.mmu.mem.begin() + 0x8800, cpu.mmu.mem.begin() + 0x9800, ostream_iterator<unsigned int>(out, " "));
  out.close();
  
  out.open("tiledata2");
  copy(cpu.mmu.mem.begin() + 0x8000, cpu.mmu.mem.begin() + 0x9000, ostream_iterator<unsigned int>(out, " "));
  out.close();

  out.open("oam");
  copy(cpu.mmu.mem.begin() + 0xfe00, cpu.mmu.mem.begin() + 0xfea0, ostream_iterator<unsigned int>(out, " "));
  out.close();

  out.open("btt");
  copy(cpu.mmu.mem.begin() + 0x9800, cpu.mmu.mem.begin() + 0x9c00, ostream_iterator<unsigned int>(out, " "));
  out.close();
  
  out.open("btt2");
  copy(cpu.mmu.mem.begin() + 0x9c00, cpu.mmu.mem.begin() + 0xa000, ostream_iterator<unsigned int>(out, " "));
  out.close();

  out.open("accessed");
  for (auto it = cpu.mmu.accessed.begin(); it != cpu.mmu.accessed.end(); ++it) {
    auto dist = it - cpu.mmu.accessed.begin();
    out << hex << (0xff00 + dist) << dec << ' ' << *it << endl;
  }
  out.close();
}
