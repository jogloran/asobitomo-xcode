#include <iostream>
#include <iomanip>

#include "cpu6502.hpp"

#include "bus.hpp"
#include "ops.hpp"
#include "op_names.hpp"

constexpr byte INITIAL_STATUS_REG = 0x24;

byte CPU6502::read(word address) {
  return bus->read(address);
}

void CPU6502::write(word address, byte value) {
  return bus->write(address, value);
}

void CPU6502::set_pc(word address) {
  pc = address;
}

void CPU6502::reset() {
  a = x = y = 0;
  sp = 0xfd;
  pc = read(0xfffc) | (read(0xfffd) << 8);
  std::cerr << "setting pc to " << std::hex << pc << std::endl;
  p.reg = INITIAL_STATUS_REG;
}

const char* to_flag_string(byte f)  {
  static char buf[9] = "________";
  static const char* flags = "NVstDIZC";
  for (int i = 0; i < 8; ++i) {
    buf[7 - i] = (f & 1) ? flags[7 - i] : '_';
    f >>= 1;
  }
  return buf;
}

void CPU6502::tick() {
  using std::hex;
  using std::dec;
  using std::setw;
  using std::setfill;
  using std::left;
  
  if (cycles_left == 0) {
    byte opcode = read(pc);
    std::cout << hex << setw(4) << setfill('0') << pc << ": "
      << hex << setw(2) << setfill('0') << int(opcode) << ' '
      << setw(24) << left << setfill(' ')
      << instruction_at_pc(*this);
    ++pc;
    
    std::cout << " sp: " << hex << setw(2) << setfill('0') << static_cast<int>(sp) << ' ';
    std::cout << to_flag_string(p.reg) << " (" << hex << setw(2) << setfill('0') << int(p.reg) << ')';
    std::cout << std::right
              << " a: " << hex << setw(2) << setfill('0') << static_cast<int>(a)
              << " x: " << hex << setw(2) << setfill('0') << static_cast<int>(x)
              << " y: " << hex << setw(2) << setfill('0') << static_cast<int>(y);
    
    std::cout << " cyc: " << dec << setw(8) << setfill(' ') << (ncycles * 3) % 341
              << std::endl;
    
    cycle_count_t extra_cycles = ops[opcode](*this);
    cycles_left = cycle_counts[opcode] + extra_cycles;
  }
  
  --cycles_left;
  ++ncycles;
}

void CPU6502::push_word(word address) {
  bus->write(SP_BASE + sp - 1, address & 0xff);
  bus->write(SP_BASE + sp, address >> 8);
  sp -= 2;
}

void CPU6502::push(byte data) {
  bus->write(SP_BASE + sp, data);
  --sp;
}

void CPU6502::rts() {
  // Note the + 1 for the special behaviour of RTS
  pc = pop_word() + 1;
}

word CPU6502::pop_word() {
  auto result = (bus->read(SP_BASE + sp + 1) | (bus->read(SP_BASE + sp + 2) << 8));
  sp += 2;
  return result;
}

byte CPU6502::pop() {
  auto result = bus->read(SP_BASE + sp + 1);
  ++sp;
  return result;
}

word CPU6502::read_word() {
  word result = bus->read(pc++);
  return result | (bus->read(pc++) << 8);
}

cycle_count_t CPU6502::branch_with_offset() {
  sbyte offset = static_cast<sbyte>(bus->read(pc++));
  pc += offset;
  return ((pc - offset) & 0xff00) == (pc & 0xff00) ? 0 : 1;
}
