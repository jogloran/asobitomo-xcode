#include <sstream>

#include "cpu.h"
#include "util.h"

#include "cpu_macros.h"
#include "cpu_bc_macros.h"
#include "op_names.h"

bool ASOBITOMO_DEBUG = false;

std::array<op, 256> CPU::cb_ops  {
  RLC8(),
  RRC8(),
  RL8(),
  RR8(),
  SLA8(),
  SRA8(),
  SWAP8(),
  SRL8(),
  BIT_GEN64(),
  RES_GEN64(),
  SET_GEN64(),
};

std::array<op, 256> CPU::ops {
 NOOP /* 0x0 */,
 LD_WORD_d16(b, c) /* 0x1 */,
 LD_ADDR_REG(b, c, a) /* 0x2 */,
 INC_WORD(b, c) /* 0x3 */,
 INC_REG(b) /* 0x4 */,
 DEC_REG(b) /* 0x5 */,
 LD_REG_d8(b) /* 0x6 */,
 RLCA() /* 0x7 */,
 LD_LOC_SP() /* 0x8 */,
 ADD_WORD_WORD(h, l, b, c) /* 0x9 */,
 LD_REG_LOC(a, b, c) /* 0xa */,
 DEC_WORD(b, c) /* 0xb */,
 INC_REG(c) /* 0xc */,
 DEC_REG(c) /* 0xd */,
 LD_REG_d8(c) /* 0xe */,
 RRCA() /* 0xf */,

 STOP() /* 0x10 */,
 LD_WORD_d16(d, e) /* 0x11 */,
 LD_ADDR_REG(d, e, a) /* 0x12 */,
 INC_WORD(d, e) /* 0x13 */,
 INC_REG(d) /* 0x14 */,
 DEC_REG(d) /* 0x15 */,
 LD_REG_d8(d) /* 0x16 */,
 RLA() /* 0x17 */,
 JR_COND_r8((true)) /* 0x18 */,
 ADD_WORD_WORD(h, l, d, e) /* 0x19 */,
 LD_REG_LOC(a, d, e) /* 0x1a */,
 DEC_WORD(d, e) /* 0x1b */,
 INC_REG(e) /* 0x1c */,
 DEC_REG(e) /* 0x1d */,
 LD_REG_d8(e) /* 0x1e */,
 RRA() /* 0x1f */,

 JR_COND_r8((cpu.Z() == 0)) /* JR NZ */ /* 0x20 */,
 LD_WORD_d16(h, l) /* 0x21 */,
 LD_LOC_REG_AUG(+, h, l, a) /* LD (HL+), A todo */ /* 0x22 */,
 INC_WORD(h, l) /* 0x23 */,
 INC_REG(h) /* 0x24 */,
 DEC_REG(h) /* 0x25 */,
 LD_REG_d8(h) /* 0x26 */,
 DAA() /* TODO: DAA */ /* 0x27 */,
 JR_COND_r8((cpu.Z() != 0)) /* JR Z */ /* 0x28 */,
 ADD_WORD_WORD(h, l, h, l) /* 0x29 */,
 LD_REG_LOC_AUG(a, +, h, l) /* LD A, (HL+) */ /* 0x2a */,
 DEC_WORD(h, l) /* 0x2b */,
 INC_REG(l) /* 0x2c */,
 DEC_REG(l) /* 0x2d */,
 LD_REG_d8(l) /* 0x2e */,
 CPL() /* CPL */ /* 0x2f */,

 JR_COND_r8((cpu.C() == 0)) /* todo */ /* 0x30 */,
 LD_WWORD_d16(sp) /* 0x31 */,
 LD_LOC_REG_AUG(-, h, l, a) /* LD (HL-),A todo */ /* 0x32 */,
 INC_WWORD(sp) /* 0x33 */,
 INC_ADDR(h, l) /* 0x34 */,
 DEC_ADDR(h, l) /* 0x35 */,
 LD_ADDR_d8(h, l) /* 0x36 */,
 SCF() /* SCF */ /* 0x37 */,
 JR_COND_r8((cpu.C() != 0)) /* JR C */ /* 0x38 */,
 ADD_WORD_WWORD(h, l, sp) /* 0x39 */,
 LD_REG_LOC_AUG(a, -, h, l) /* LD A,(HL-) */ /* 0x3a */,
 DEC_WWORD(sp) /* 0x3b */,
 INC_REG(a) /* 0x3c */,
 DEC_REG(a) /* 0x3d */,
 LD_REG_d8(a) /* 0x3e */,
 CCF() /* CCF */ /* 0x3f */,

 LD_REG_REG8(b) /*bcdehl (hl) a*/ /* 0x40-47 */,
 LD_REG_REG8(c)/*bcdehl (hl) a*/  /* 0x48-4f */,
 LD_REG_REG8(d) /*bcdehl (hl) a*/ /* 0x50-57 */,
 LD_REG_REG8(e) /*bcdehl (hl) a*/ /* 0x58-5f */,
 LD_REG_REG8(h) /*bcdehl (hl) a*/ /* 0x60-67 */,
 LD_REG_REG8(l) /*bcdehl (hl) a*/ /* 0x68-6f */,
 LD_HL_SPECIAL()/*  LD (HL), ... and HALT */ /* 0x70-0x77 */,
 LD_REG_REG8(a) /*bcdehl (hl) a*/ /* 0x78-7f */,
 ADD_A8() /* 0x80-87 */,
 ADC_A8() /* 0x88-8f */,
 SUB_A8() /* 0x90-97 */,
 SBC_A8() /* 0x98-9f */,
 AND_A8() /* 0xa0-a7 */,
 GEN8(^)  /* 0xa8-af */,
 GEN8(|)  /* 0xb0-b7 */,
 CP8()    /* 0xb8-bf */,

 RET_COND((cpu.Z() == 0)) /* 0xc0 */,
 POP_WORD(b, c) /* 0xc1 */,
 JP_COND_a16((cpu.Z() == 0)) /* 0xc2 */,
 JP_COND_a16((true)) /* 0xc3 */,
 CALL_COND_a16((cpu.Z() == 0)) /* 0xc4 */,
 PUSH_WORD(b, c) /* 0xc5 */,
 ADD_A_d8() /* 0xc6 */,
 RST(0x00) /* 0xc7 */,
 RET_COND((cpu.Z() != 0)) /* 0xc8 */,
 RET_COND((true)) /* 0xc9 */,
 JP_COND_a16((cpu.Z() != 0)) /* 0xca */,
 CPU::handle_cb /* prefix CB */ /* 0xcb */,
 CALL_COND_a16((cpu.Z() != 0)) /* 0xcc */,
 CALL_COND_a16((true)) /* 0xcd */,
 ADC_A_d8() /* 0xce */,
 RST(0x08) /* 0xcf */,

 RET_COND((cpu.C() == 0)) /* 0xd0 */,
 POP_WORD(d, e) /* 0xd1 */,
 JP_COND_a16((cpu.C() == 0)) /* 0xd2 */,
 INVALID() /* 0xd3 */,
 CALL_COND_a16((cpu.C() == 0)) /* 0xd4 */,
 PUSH_WORD(d, e) /* 0xd5 */,
 SUB_A_d8() /* 0xd6 */,
 RST(0x10) /* 0xd7 */,
 RET_COND((cpu.C() != 0)) /* 0xd8 */,
 RETI() /* 0xd9 */,
 JP_COND_a16((cpu.C() != 0)) /* 0xda */,
 INVALID() /* 0xdb */,
 CALL_COND_a16((cpu.C() != 0)) /* 0xdc */,
 INVALID() /* 0xdd */,
 SBC_A_d8() /* 0xde */,
 RST(0x18) /* 0xdf */,

 LDH_a8_A() /* LDH (a8), A */ /* 0xe0 */,
 POP_WORD(h, l) /* 0xe1 */,
 LDH_ADDR_A(c) /* LD (C), A */ /* 0xe2 */,
 INVALID() /* 0xe3 */,
 INVALID() /* 0xe4 */,
 PUSH_WORD(h, l) /* 0xe5 */,
 AND_d8() /* 0xe6 */,
 RST(0x20) /* 0xe7 */,
 ADD_SP_r8() /* 0xe8 */,
 JP_HL() /* 0xe9 */,
 LD_a16_A() /* 0xea */,
 INVALID() /* 0xeb */,
 INVALID() /* 0xec */,
 INVALID() /* 0xed */,
 OP_d8(^) /* 0xee */,
 RST(0x28) /* 0xef */,

 LDH_A_a8() /* LDH A, (a8) */ /* 0xf0 */,
 POP_WORD(a, f) /* 0xf1 */,
 LDH_A_ADDR(c) /* LD A, (C) */ /* 0xf2 */,
 DI() /* DI */ /* 0xf3 */,
 INVALID() /* 0xf4 */,
 PUSH_WORD(a, f) /* 0xf5 */,
 OP_d8(|) /* 0xf6 */,
 RST(0x30) /* 0xf7 */,
 LD_HL_SP_plus_r8() /* 0xf8 */,
 LD_SP_HL() /* 0xf9 */,
 LD_A_a16() /* 0xfa */,
 EI() /* EI */ /* 0xfb */,
 INVALID() /* 0xfc */,
 INVALID() /* 0xfd */,
 CP_d8() /* 0xfe */,
 RST(0x38) /* 0xff */,

};

std::string CPU::op_name_for(word loc) {
  if (mmu[loc] == 0xcb) {
    return cb_op_names[mmu[loc+1]];
  } else {
//    return op_names[mmu[loc]];
    auto op = op_names[mmu[loc]];
    return op.instruction_at_pc(*this);
  }
}

std::string CPU::interrupt_state_as_string(InterruptState state) {
  static std::string state_strings[] {
    "-", "-?", "+?", "+",
  };
  return state_strings[static_cast<int>(state)];
}

std::string CPU::ppu_state_as_string(PPU::Mode mode) {
  static std::string mode_strings[] {
    "hblank", "vblank", "oam", "vram"
  };
  return mode_strings[static_cast<int>(mode)];
}

void CPU::dump_state() {
  byte instr = mmu[pc];
  cout << setfill('0') <<
    "[0x" << setw(4) << hex << pc << "]"
//    "a: " << setw(2) << hex << static_cast<int>(a) << ' ' <<
//    "f: " << setw(2) << hex << static_cast<int>(f) << ' ' <<
//    "b: " << setw(2) << hex << static_cast<int>(b) << ' ' <<
//    "c: " << setw(2) << hex << static_cast<int>(c) << ' ' <<
//    "d: " << setw(2) << hex << static_cast<int>(d) << ' ' <<
//    "e: " << setw(2) << hex << static_cast<int>(e) << ' ' <<
//    "h: " << setw(2) << hex << static_cast<int>(h) << ' ' <<
//    "l: " << setw(2) << hex << static_cast<int>(l) << ' ' <<
    << " LY|C: " << setw(2) << hex << static_cast<int>(mmu._read_mem(0xff44))
    << "|" << setw(2) << hex << static_cast<int>(mmu._read_mem(0xff45))
    << " LCDC: " << binary(mmu._read_mem(0xff40))
    << " STAT: " << binary(mmu._read_mem(0xff41))
    << " IF: " << binary(mmu._read_mem(0xff0f))
    << " IE: " << binary(mmu._read_mem(0xffff))
    << " (" << interrupt_state_as_string(interrupt_enabled) << ")"
    << " (" << ppu_state_as_string(ppu.mode) << ") " <<
      " " << op_name_for(pc) <<
      endl;
//  cout << setfill('0') <<
//    "pc: 0x" << setw(4) << hex << pc << ' ' <<
//    "sp: 0x" <<                   sp << ' ' <<
//    "op: 0x" << setw(2) << hex << static_cast<int>(instr) << ' ' <<
//    "(" << op_name_for(pc) << ")" << endl;
//  cout << "LY: " << setw(2) << hex << static_cast<int>(mmu._read_mem(0xff44))
//    << "\tLYC: " << setw(2) << hex << static_cast<int>(mmu._read_mem(0xff45))
//    << "\tSCY: " << setw(2) << hex << static_cast<int>(mmu._read_mem(0xff42))
//    << "\tSCX: " << setw(2) << hex << static_cast<int>(mmu._read_mem(0xff43)) << endl;
//  cout << "LCDC: " << binary(mmu._read_mem(0xff40))
//    << "\t\tSTAT: " << binary(mmu._read_mem(0xff41))
//    << " (" << ppu_state_as_string(ppu.mode) << ")" << endl;
//  cout << "IF: " << binary(mmu._read_mem(0xff0f))
//    << "\t\tIE: " << binary(mmu._read_mem(0xffff)) << endl;
//  cout << "Interrupts: " << interrupt_state_as_string(interrupt_enabled) << endl;
//  cout << "0xff00: " << setw(2) << hex << binary(mmu._read_mem(0xff00))
//    << "\t\t0xff81: " << setw(2) << hex << binary(mmu._read_mem(0xff81)) << endl;
//  cout <<
//    "a: " << setw(2) << hex << static_cast<int>(a) << ' ' <<
//    "f: " << setw(2) << hex << static_cast<int>(f) << ' ' <<
//    "b: " << setw(2) << hex << static_cast<int>(b) << ' ' <<
//    "c: " << setw(2) << hex << static_cast<int>(c) << ' ' << endl;
//  cout <<
//    "d: " << setw(2) << hex << static_cast<int>(d) << ' ' <<
//    "e: " << setw(2) << hex << static_cast<int>(e) << ' ' <<
//    "h: " << setw(2) << hex << static_cast<int>(h) << ' ' <<
//    "l: " << setw(2) << hex << static_cast<int>(l) << ' ' << endl;
//  cout <<
//    "Z: " << setw(2) << hex << static_cast<int>(Z()) << ' ' <<
//    "N: " << setw(2) << hex << static_cast<int>(N()) << ' ' <<
//    "H: " << setw(2) << hex << static_cast<int>(H()) << ' ' <<
//    "C: " << setw(2) << hex << static_cast<int>(C()) << ' ' << endl;
//  cout << endl;
}


void CPU::update_interrupt_state() {
  switch (interrupt_enabled) {
    case InterruptState::DisableNext:
      interrupt_enabled = InterruptState::Disabled;
      break;
    case InterruptState::EnableNext:
      interrupt_enabled = InterruptState::Enabled;
      break;
    default:
      break;
  }
}

void CPU::fire_interrupts() {
  if (interrupt_enabled == InterruptState::Disabled) {
    return;
  }

  byte interrupt_enable = mmu._read_mem(0xffff);
  byte interrupt_flags = mmu._read_mem(0xff0f);

  byte candidate_interrupts = interrupt_enable & interrupt_flags;

  word handler = 0x0;
  byte handled_interrupt;
  if (candidate_interrupts & 0x1) {
    handled_interrupt = 0x1;
    handler = 0x40; // vblank interrupt
  } else if (candidate_interrupts & 0x2) {
    handled_interrupt = 0x2;
    handler = 0x48; // STAT interrupt
  } else if (candidate_interrupts & 0x4) {
    handled_interrupt = 0x4;
    handler = 0x50; // timer interrupt
  } else if (candidate_interrupts & 0x8) {
    handled_interrupt = 0x8;
    handler = 0x58; // serial interrupt
  } else if (candidate_interrupts & 0x10) {
    handled_interrupt = 0x10;
    handler = 0x60; // joypad interrupt
  } else {
    return;
  }

  interrupt_enabled = InterruptState::Disabled;
//  std::cout << "<><> handling interrupt: " << hex << handled_interrupt << std::endl;
  mmu.set(0xff0f, interrupt_flags & ~handled_interrupt);
  mmu[sp - 1] = pc >> 8;
  mmu[sp] = pc & 0xff;
  sp -= 2;
  pc = handler;
}

static std::string interrupt_names[] {
  "vblank",
  "stat",
  "timer",
  "serial",
  "joypad",
};

static std::string interrupt_flags_to_description(byte flags) {
  std::stringstream s;
  bool first = true;
  
  int i = 0;
  for (auto name : interrupt_names) {
    if (flags & (1 << i)) {
      if (!first) s << ", ";
      else first = false;
      
      s << name;
    }
    ++i;
  }
  
  return s.str();
}

bool CPU::wake_if_interrupt_requested() {
  if (interrupt_flags_before_halt != mmu[0xff0f]) {
    halted = false;
//    std::cout << ">>> Awakened by " << interrupt_flags_to_description(mmu[0xff0f]) << std::endl;
    return true;
  }
  return false;
}

void CPU::step(bool debug)  {
  bool awakened_by_interrupt = false;
  if (halted) {
    awakened_by_interrupt = wake_if_interrupt_requested();
  }

  update_interrupt_state();
  fire_interrupts();
  
  if (halted) {
    ppu.step(4);
    timer.step(4);
    
    cycles += 4;
  } else {
    byte instr = mmu[pc];

    if (debug) {
      dump_state();
    }

    ++pc;

    long old_cycles = cycles;
    ops[instr](*this);
    cycles += ncycles[instr];
    
    ppu.step(cycles - old_cycles);
    timer.step(cycles - old_cycles);
  }
}
