#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <algorithm>
#include <deque>
#include "types.h"
#include "cpu.h"

using namespace std;

size_t history_repeating(std::deque<word> history) {
  for (auto i = 10; i >= 2; --i) {
    if (history.size() < 2*i) continue;
    
    if (std::equal(history.end() - i, history.end(), history.end() - 2*i)) {
      return i;
    }
  }
  
  return 0;
}

int main() {
  CPU cpu("/Users/dt0/my/asobitomo-xcode/asobitomo/Tetris.gb");

  copy(cpu.mmu.rom.begin(), cpu.mmu.rom.end(), cpu.mmu.mem.begin());
  
  std::deque<word> history;
  size_t repeating = 0;
  size_t last_period = 0;
  const bool debug = true;

  while (!cpu.halted && cpu.pc != 0x100) {
    // bool debug = cpu.mmu[0xff44] >= 143 && cpu.mmu[0xff44] <= 153;
    cpu.step(false);
  }
  //
  int i = 0;
  while (i++ <= 1000000000) {
    if (cpu.pc == 0x7ff3) {
      ;
    }
    history.emplace_back(cpu.pc);
    if (history.size() >= 20) {
      history.pop_front();
    }
    
    bool should_dump = false;
    
    if (debug) {
      auto period = history_repeating(history);
      if (period) {
        repeating++;
        if (repeating == 1) {
          should_dump = true;
        }
      } else {
        if (repeating > 0 && should_dump) {
          std::cout << "\t... [last " << last_period << " ops repeated " << dec << repeating << " times]" << std::endl << std::endl;
        }
        
        repeating = 0;
      }
      last_period = period;
    }
    
    cpu.step(should_dump);
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
