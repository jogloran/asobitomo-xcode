#include <memory>
#include <fstream>
#include <iostream>

#include "main.hpp"
#include "cpu6502.hpp"
#include "bus.hpp"
#include "ops.hpp"

#include <gflags/gflags.h>

//DEFINE_bool(dis, false, "Dump disassembly");
//DEFINE_bool(cloop, false, "When dumping disassembly, detect and condense loops");
//DEFINE_bool(headless, false, "No display");
//DEFINE_bool(limit_framerate, true, "Limit framerate to 59.7 fps");
//DEFINE_int32(us_per_frame, 17'500, "ms per frame limit");
//DEFINE_int32(run_for_n, -1, "Run for n instructions");
//DEFINE_int32(run_for_cycles, -1, "Run for n cycles");
//DEFINE_string(dump_states_to_file, "", "Dump states to file");
//DEFINE_string(dis_instrs, "", "Instructions to dump for");
//DEFINE_string(dis_pcs, "", "ROM locations to dump for");
//DEFINE_bool(dis_dump_from_pc, false, "Start dumping once pc has gotten to one of the values in dis_pcs");
//DEFINE_bool(fake_boot, true, "Initialise registers to post-ROM values");
//DEFINE_string(model, "", "Model to emulate");
//DEFINE_bool(td, false, "Show tile debugger");
//DEFINE_bool(tm, false, "Show tile map");
//DEFINE_bool(no_load, false, "Don't load external RAM from file");
//DEFINE_bool(no_save, false, "Don't save external RAM to file");
//DEFINE_bool(audio, false, "Enable sound");
//
//DEFINE_bool(xx, false, "Debug");

using namespace std;

struct Header {
  byte header[4];
  byte prg_rom_size_lsb;
  byte chr_rom_size_lsb;
  byte flags6;
  byte system_flags;
  byte mapper_flags;
  byte prg_rom_size_msb;
  byte prg_ram_size;
  byte padding[5];
} __attribute__((packed, aligned(1)));

void inspect_header(Header* h) {
  std::cout << "PRG-ROM size: " << (((h->prg_rom_size_msb & 0xf) << 8) | h->prg_rom_size_lsb) * 16384 << std::endl
  << "CHR-ROM size: " << ( ((h->prg_rom_size_msb >> 4) << 8) | h->prg_rom_size_lsb) * 8192 << std::endl
  << "Mapper: " << ((h->system_flags & 0xf0) | (h->flags6 >> 4) | ((h->mapper_flags >> 4) << 8)) << std::endl;
}

int main(int argc, char** argv) {
  gflags::SetUsageMessage("A NES emulator");
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  
  if (argc != 2) {
    std::cerr << "Expecting a ROM filename." << std::endl;
    exit(1);
  }

  auto cpu = std::make_shared<CPU6502>();
  Bus bus(cpu);
  
  std::ifstream f(argv[1], ios::in);
  if (f) {
    std::cerr << "reading" << std::endl;
    byte header_bytes[16];
    f.read((char*)header_bytes, 16);
    Header* h = reinterpret_cast<Header*>(header_bytes);
    
    inspect_header(h);
    
    std::cout << "header: " << (char)h->header[0] << (char)h->header[1] << (char)h->header[2] << std::endl;
    
    std::array<byte, 0x4000> cart;
    f.seekg(0x10);
    f.read((char*)cart.data(), 0x4000);
    
    bus.map(cart.begin(), 0x4000);
  }
  
  cpu->reset();
  cpu->set_pc(0xc000);
  while (true) {
    cpu->tick();
  }
  
  return 0;
}
