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

int argc;
char** argv;
bool in_title = false;

int main(int argc_, char** argv_) {
  argc = argc_; argv = argv_;
  
//  CPU cpu("/Users/dt0/my/asobitomo/Tetris.gb");
  CPU cpu("/Users/dt0/my/asobitomo-xcode/asobitomo/Tetris.gb");

  copy(cpu.mmu.rom.begin(), cpu.mmu.rom.end(), cpu.mmu.mem.begin());
//  cpu.mmu[0xff80] = 0xf; // joypad initialis.ation
//  cpu.mmu[0xff00] = 0x37;
  
  std::deque<word> history;
  size_t repeating = 0;
  size_t last_period = 0;
  const bool debug = false;

  while (!cpu.halted && cpu.pc != 0x100) {
    cpu.step(false);
    cpu.ppu.screen.blit();
  }
  //
  int i = 0;
  bool s=false;
        bool should_dump = false;
  while (i++ <= 10000000) {
//  while (cpu.pc != 0x037e) {
//    if (cpu.pc == 0x0369) { // credits
//    if (cpu.pc == 0x0502 || cpu.pc == 0x04f3 || cpu.pc == 0x04f5 || cpu.pc == 0x0502 || cpu.pc == 0x0507) {
    if (cpu.pc == 0x4f5) {
//      should_dump = true;

//      cpu.dump_state();
    }
    
    if (s) {
//      s = true;
//      cpu.dump_state();
    }
    if (cpu.pc == 0x479) {
      in_title = true;
      ASOBITOMO_DEBUG = true;
      //      should_dump = true;
      cpu.ppu.screen.blit();
//      ASOBITOMO_DEBUG = true;
//      cpu.ppu.screen.blit();
    }
    
//    if (cpu.pc == 0x29e2) {
//      cpu.dump_state();
//    }
    
//    if (cpu.pc == 0x04a2 && cpu.mmu._read_mem(0xff81) != 0x0) {
//      should_dump = true;
//      cpu.dump_state();
//    }
    if (cpu.pc == 0x02d3) {
//      should_dump = true;
    }
    
    if (cpu.pc >= 0x04a2 && cpu.pc <= 0x0501) {
//      cpu.dump_state();
    }
    
    history.emplace_back(cpu.pc);
    if (history.size() >= 20) {
      history.pop_front();
    }
    
    if (debug) {
      auto period = history_repeating(history);
      if (period) {
        repeating++;
        should_dump = (repeating == 1);
      } else {
        if (repeating > 0) {
          std::cout << "\t... [last " << dec << last_period << " ops repeated " << dec << repeating << " times]" << std::endl << std::endl;
        }
        
        repeating = 0;
        should_dump = true;
      }
      last_period = period;
    }
    
    cpu.step(should_dump);
  }

  cpu.dump_state();
  cpu.ppu.rasterise_line();

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
