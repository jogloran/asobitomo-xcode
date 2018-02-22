#include <sstream>

#include "cpu.h"

#include "cpu_macros.h"
#include "cpu_bc_macros.h"
#include "op_names.h"

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
 UNIMPL() /* TODO: DAA */ /* 0x27 */,
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

 LD_REG_REG8(b) /*bcdehl (hl) a*/ /* 0x40 */,
 LD_REG_REG8(c)/*bcdehl (hl) a*/ /* 0x41 */,
 LD_REG_REG8(d) /*bcdehl (hl) a*/ /* 0x42 */,
 LD_REG_REG8(e) /*bcdehl (hl) a*/ /* 0x43 */,
 LD_REG_REG8(h) /*bcdehl (hl) a*/ /* 0x44 */,
 LD_REG_REG8(l) /*bcdehl (hl) a*/ /* 0x45 */,
 LD_HL_SPECIAL()/*  LD (HL), ... and HALT */ /* 0x46 */,
 LD_REG_REG8(a) /*bcdehl (hl) a*/ /* 0x47 */,
 ADD_A8() /* 0x48 */,
 ADC_A8() /* 0x49 */,
 GEN8(-) /* 0x4a */,
 SBC_A8() /* 0x4b */,
 GEN8(&) /* 0x4c */,
 GEN8(^) /* 0x4d */,
 GEN8(|) /* 0x4e */,
 CP8() /* 0x4f */,

 RET_COND((cpu.Z() == 0)) /* 0x50 */,
 POP_WORD(b, c) /* 0x51 */,
 JP_COND_a16((cpu.Z() == 0)) /* 0x52 */,
 JP_COND_a16((true)) /* 0x53 */,
 CALL_COND_a16((cpu.Z() == 0)) /* 0x54 */,
 PUSH_WORD(b, c) /* 0x55 */,
 ADD_A_d8() /* 0x56 */,
 RST(0x00) /* 0x57 */,
 RET_COND((cpu.Z() != 0)) /* 0x58 */,
 RET_COND((true)) /* 0x59 */,
 JP_COND_a16((cpu.Z() != 0)) /* 0x5a */,
 CPU::handle_cb /* prefix CB */ /* 0x5b */,
 CALL_COND_a16((cpu.Z() != 0)) /* 0x5c */,
 CALL_COND_a16((true)) /* 0x5d */,
 ADC_A_d8() /* 0x5e */,
 RST(0x08) /* 0x5f */,

 RET_COND((cpu.C() == 0)) /* 0x60 */,
 POP_WORD(d, e) /* 0x61 */,
 JP_COND_a16((cpu.C() == 0)) /* 0x62 */,
 INVALID() /* 0x63 */,
 CALL_COND_a16((cpu.C() == 0)) /* 0x64 */,
 PUSH_WORD(d, e) /* 0x65 */,
 SUB_A_d8() /* 0x66 */,
 RST(0x10) /* 0x67 */,
 RET_COND((cpu.C() != 0)) /* 0x68 */,
 RETI() /* 0x69 */,
 JP_COND_a16((cpu.C() != 0)) /* 0x6a */,
 INVALID() /* 0x6b */,
 CALL_COND_a16((cpu.C() != 0)) /* 0x6c */,
 INVALID() /* 0x6d */,
 SBC_A_d8() /* 0x6e */,
 RST(0x18) /* 0x6f */,

 LDH_a8_A() /* LDH (a8), A */ /* 0x70 */,
 POP_WORD(h, l) /* 0x71 */,
 LDH_ADDR_A(c) /* LD (C), A */ /* 0x72 */,
 INVALID() /* 0x73 */,
 INVALID() /* 0x74 */,
 PUSH_WORD(h, l) /* 0x75 */,
 OP_d8(&) /* 0x76 */,
 RST(0x20) /* 0x77 */,
 ADD_SP_r8() /* 0x78 */,
 JP_HL() /* 0x79 */,
 LD_a16_A() /* 0x7a */,
 INVALID() /* 0x7b */,
 INVALID() /* 0x7c */,
 INVALID() /* 0x7d */,
 OP_d8(^) /* 0x7e */,
 RST(0x28) /* 0x7f */,

 LDH_A_a8() /* LDH A, (a8) */ /* 0x80 */,
 POP_WORD(a, f) /* 0x81 */,
 LDH_A_ADDR(c) /* LD A, (C) */ /* 0x82 */,
 DI() /* DI */ /* 0x83 */,
 INVALID() /* 0x84 */,
 PUSH_WORD(a, f) /* 0x85 */,
 OP_d8(|) /* 0x86 */,
 RST(0x30) /* 0x87 */,
 LD_HL_SP_plus_r8() /* 0x88 */,
 LD_SP_HL() /* 0x89 */,
 LD_A_a16() /* 0x8a */,
 EI() /* EI */ /* 0x8b */,
 INVALID() /* 0x8c */,
 INVALID() /* 0x8d */,
 CP_d8() /* 0x8e */,
 RST(0x38) /* 0x8f */,

};

std::string CPU::binary(byte b) {
  std::stringstream s;
  int n = 0;
  while (n++ < 8) {
    if (b & 0x1) {
      s << '1';
    } else {
      s << '0';
    }
    b >>= 1;
  }
  auto result = s.str();
  reverse(result.begin(), result.end());
  return result;
}

std::string CPU::op_name_for(word loc) {
  if (mmu[loc] == 0xcb) {
    return cb_op_names[mmu[loc+1]];
  } else {
    return op_names[mmu[loc]];
  }
}

void CPU::dump_state() {
  byte instr = mmu[pc];
  cout << setfill('0') <<
    "pc: 0x" << setw(4) << hex << pc << ' ' <<
    "sp: 0x" <<                   sp << ' ' <<
    "op: 0x" << setw(2) << hex << static_cast<int>(instr) << ' ' <<
    "(" << op_name_for(pc) << ")" << endl;
  cout << "LY: " << setw(2) << hex << static_cast<int>(mmu._read_mem(0xff44))
    << "\tLYC: " << setw(2) << hex << static_cast<int>(mmu._read_mem(0xff45)) 
    << "\tSCY: " << setw(2) << hex << static_cast<int>(mmu._read_mem(0xff42))
    << "\tSCX: " << setw(2) << hex << static_cast<int>(mmu._read_mem(0xff43)) << endl;
  cout << "LCDC: " << binary(mmu._read_mem(0xff40))
    << "\t\tSTAT: " << binary(mmu._read_mem(0xff41)) << endl;
  cout <<
    "a: " << setw(2) << hex << static_cast<int>(a) << ' ' <<
    "f: " << setw(2) << hex << static_cast<int>(f) << ' ' <<
    "b: " << setw(2) << hex << static_cast<int>(b) << ' ' <<
    "c: " << setw(2) << hex << static_cast<int>(c) << ' ' << endl;
  cout <<
    "d: " << setw(2) << hex << static_cast<int>(d) << ' ' <<
    "e: " << setw(2) << hex << static_cast<int>(e) << ' ' <<
    "h: " << setw(2) << hex << static_cast<int>(h) << ' ' <<
    "l: " << setw(2) << hex << static_cast<int>(l) << ' ' << endl;
  cout <<
    "Z: " << setw(2) << hex << static_cast<int>(Z()) << ' ' <<
    "N: " << setw(2) << hex << static_cast<int>(N()) << ' ' <<
    "H: " << setw(2) << hex << static_cast<int>(H()) << ' ' <<
    "C: " << setw(2) << hex << static_cast<int>(C()) << ' ' << endl;
  cout << endl;
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
  if (candidate_interrupts & 0x1) {
    handler = 0x40; // vblank interrupt
  } else if (candidate_interrupts & 0x2) {
    handler = 0x48; // STAT interrupt
  } else if (candidate_interrupts & 0x4) {
    handler = 0x50; // timer interrupt
  } else if (candidate_interrupts & 0x8) {
    handler = 0x58; // serial interrupt
  } else if (candidate_interrupts & 0x10) {
    handler = 0x60; // joypad interrupt
  } else {
    return;
  }

  interrupt_enabled = InterruptState::Disabled;
  mmu[sp - 1] = pc >> 8;
  mmu[sp] = pc & 0xff;
  sp -= 2;
  pc = handler;
}

void CPU::step(bool debug)  {
  if (halted) {
    throw std::runtime_error("cpu halted");
  }

  update_interrupt_state();
  fire_interrupts();
  
  if (!mmu.rom_mapped && pc == 0x40) {
    ;
  }

  byte instr = mmu[pc];

  byte ly = static_cast<int>(mmu._read_mem(0xff44));

  if (debug) {
    dump_state();
  }

  ++pc;

  long old_cycles = cycles;
  ops[instr](*this);
  cycles += ncycles[instr];

  ppu.step(cycles - old_cycles);
}
