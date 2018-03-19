#include <iostream>
#include <iomanip>
#include <vector>
#include <fstream>
#include <algorithm>
#include <deque>
#include "types.h"
#include "cpu.h"

#include <gflags/gflags.h>
#include "rang.hpp"

bool should_dump = false;

DEFINE_bool(dis, false, "Dump disassembly");
DEFINE_bool(dis_detect_loops, false, "When dumping disassembly, detect and condense loops");
DEFINE_bool(headless, false, "No display");
DEFINE_bool(limit_framerate, true, "Limit framerate to 59.7 fps");
DEFINE_int32(run_for_n, -1, "Run for n instructions");
DEFINE_string(dump_states_to_file, "", "Dump states to file");
DEFINE_string(dis_instrs, "", "Instructions to dump for");
DEFINE_string(dis_pcs, "", "ROM locations to dump for");
DEFINE_bool(fake_boot, true, "Initialise registers to post-ROM values");

DEFINE_bool(show_td, false, "Show tile debugger");
DEFINE_bool(show_tm, false, "Show tile map");

using namespace std;

size_t history_repeating(std::deque<word> history) {
  for (auto i = 16; i >= 2; --i) {
    if (history.size() < 2*i) continue;
    
    if (std::equal(history.end() - i, history.end(), history.end() - 2*i)) {
      return i;
    }
  }
  
  return 0;
}

bool in_title = true;

int main(int argc, char** argv) {
  gflags::SetUsageMessage("A Game Boy emulator");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  
  if (argc != 2) {
    std::cerr << "Expecting a ROM filename." << std::endl;
    exit(1);
  }
  CPU cpu(argv[1]);
  
  std::deque<word> history;
  size_t repeating = 0;
  size_t last_period = 0;
  const bool debug = FLAGS_dis_detect_loops;

  int i = 0;
  
  if (FLAGS_fake_boot) {
    cpu.fake_boot();
  } else {
    while (cpu.pc != 0x100) {
      cpu.step(false);
    }
  }
  
  in_title = false;
  
  should_dump = FLAGS_dis;
  int run_for_n = FLAGS_run_for_n;
  
  std::ofstream states_file;
  if (FLAGS_dump_states_to_file != "") {
    states_file.open(FLAGS_dump_states_to_file);
  }
  
  if (!FLAGS_headless)
    cpu.ppu.screen.on();
  
  long ninstrs = 0;
  while (run_for_n == -1 || ninstrs < run_for_n) {
    if (FLAGS_dis_detect_loops) {
      history.emplace_back(cpu.pc);
      if (history.size() >= 20) {
        history.pop_front();
      }
      
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
    if (FLAGS_dump_states_to_file != "") {
      cpu.dump_registers_to_file(states_file);
    }
    ++ninstrs;
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
