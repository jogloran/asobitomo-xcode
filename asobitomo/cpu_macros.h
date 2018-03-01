#pragma once

#include "types.h"
#include "flags.h"
#include "cpu.h"

#define NOOP [](CPU&) {}

#define LD_WORD_d16(hi, lo) [](CPU& cpu) { \
  cpu.hi = cpu.mmu[cpu.pc + 1]; \
  cpu.lo = cpu.mmu[cpu.pc]; \
  cpu.pc += 2; \
}

#define LD_WWORD_d16(wword) [](CPU& cpu) { \
  cpu.wword = cpu.get_word(); \
  cpu.pc += 2; \
}

#define LD_ADDR_REG(hi, lo, reg) [](CPU& cpu) { \
  word loc = (cpu.hi << 8) | cpu.lo; \
  cpu.mmu.set(loc, cpu.reg); \
}

#define LD_LOC_SP() [](CPU& cpu) { \
  word loc = cpu.get_word(); \
  cpu.mmu.set(loc, cpu.sp); \
  cpu.pc += 2; \
}

#define INC_WORD(hi, lo) [](CPU& cpu) { \
  word bc = (cpu.hi << 8) | cpu.lo; \
  ++bc; \
  cpu.hi = bc >> 8; \
  cpu.lo = bc & 0xff; \
  /* Flags not affected */ \
}

#define INC_WWORD(wword) [](CPU& cpu) { \
  ++cpu.wword; \
  /* Flags not affected */ \
}

#define DEC_WORD(hi, lo) [](CPU& cpu) { \
  word bc = (cpu.hi << 8) | cpu.lo; \
  --bc; \
  cpu.hi = bc >> 8; \
  cpu.lo = bc & 0xff; \
  /* Flags not affected */ \
}

#define DEC_WWORD(wword) [](CPU& cpu) { \
  --cpu.wword; \
  /* Flags not affected */ \
}

#define INC_REG(byte) [](CPU& cpu) { \
  if ((((cpu.byte & 0xf) + ((cpu.byte + 1) & 0xf)) & 0x10) == 0x10) { \
    cpu.set_flags(Hf); \
  } else { \
    cpu.unset_flags(Hf); \
  } \
  ++cpu.byte; \
  cpu.unset_flags(Nf); \
  if (cpu.byte == 0x0) { \
    cpu.set_flags(Zf); \
  } else { \
    cpu.unset_flags(Zf); \
  } \
}

#define DEC_REG(byte) [](CPU& cpu) { \
  if ((((cpu.byte & 0xf) + ((cpu.byte - 1) & 0xf)) & 0x10) == 0x10) { \
    cpu.set_flags(Hf); \
  } else { \
  cpu.unset_flags(Hf); \
  } \
  --cpu.byte; \
  cpu.set_flags(Nf); \
  if (cpu.byte == 0x0) { \
    cpu.set_flags(Zf); \
  } else { \
    cpu.unset_flags(Zf); \
  } \
}

#define INC_ADDR(hi, lo) [](CPU& cpu) { \
  word loc = (cpu.hi << 8) | cpu.lo; \
  cpu.mmu.set(loc, cpu.mmu[loc] + 1); \
}

#define DEC_ADDR(hi, lo) [](CPU& cpu) { \
  word loc = (cpu.hi << 8) | cpu.lo; \
  cpu.mmu.set(loc, cpu.mmu[loc] + 1); \
}

#define LD_REG_d8(reg) [](CPU& cpu) { \
  byte d8 = cpu.mmu[cpu.pc]; \
  cpu.reg = d8; \
  cpu.pc += 1; \
}

#define LD_ADDR_d8(hi, lo) [](CPU& cpu) { \
  word loc = (cpu.hi << 8) | cpu.lo; \
  byte d8 = cpu.mmu[cpu.pc]; \
  cpu.mmu.set(loc, d8); \
  cpu.pc += 1; \
}

#define RLCA() [](CPU& cpu) { \
  byte hibit = cpu.a >> 7; \
  if (hibit == 1) { \
    cpu.set_flags(Cf); \
  } else { \
    cpu.unset_flags(Cf); \
  } \
  cpu.a <<= 1; \
  cpu.a |= hibit; \
  cpu.unset_flags(Nf | Hf | Zf); \
}

#define RRCA() [](CPU& cpu) { \
  byte lobit = cpu.a & 0x1; \
  if (lobit) { \
    cpu.set_flags(Cf); \
  } else { \
    cpu.unset_flags(Cf); \
  } \
  cpu.a >>= 1; \
  cpu.a |= lobit << 7; \
  cpu.unset_flags(Nf | Hf | Zf); \
}

 #define RLA() [](CPU& cpu) { \
   byte carry = cpu.C(); \
   byte hibit = cpu.a >> 7; \
   if (hibit) { \
     cpu.set_flags(Cf); \
   } else { \
     cpu.unset_flags(Cf); \
   } \
   cpu.a <<= 1; \
   cpu.a |= carry; \
   cpu.unset_flags(Nf | Hf | Zf); \
 }

#define RRA() [](CPU& cpu) { \
   byte carry = cpu.C(); \
   byte lobit = cpu.a & 0x1; \
   if (lobit) { \
     cpu.set_flags(Cf); \
   } else { \
     cpu.unset_flags(Cf); \
   } \
   cpu.a >>= 1; \
   cpu.a |= carry << 7; \
   cpu.unset_flags(Nf | Hf | Zf); \
}

#define ADD_WORD_WORD(hi1, lo1, hi2, lo2) [](CPU& cpu) { \
  word hl = (cpu.hi1 << 8) | cpu.lo1; \
  word bc = (cpu.hi2 << 8) | cpu.lo2; \
  hl += bc; \
  cpu.hi1 = hl >> 8; \
  cpu.lo1 = hl & 0xff; \
  cpu.unset_flags(Nf); \
  /* set H, C conditionally */ \
}

#define ADD_WORD_WWORD(hi, lo, wword) [](CPU& cpu) { \
  word hl = (cpu.hi << 8) | cpu.lo; \
  word bc = cpu.wword; \
  hl += bc; \
  cpu.hi = hl >> 8; \
  cpu.lo = hl & 0xff; \
  cpu.unset_flags(Nf); \
  /* set H, C conditionally */ \
}

#define LD_REG_LOC(reg, hi, lo) [](CPU& cpu) { \
  word bc = (cpu.hi << 8) | cpu.lo; \
  cpu.reg = cpu.mmu[bc]; \
}

#define LD_REG_REG8(dest) LD_REG_REG8_helper(dest, b), \
LD_REG_REG8_helper(dest, c), \
LD_REG_REG8_helper(dest, d), \
LD_REG_REG8_helper(dest, e), \
LD_REG_REG8_helper(dest, h), \
LD_REG_REG8_helper(dest, l), \
LD_REG_REG8_HL_LOC_helper(dest), \
LD_REG_REG8_helper(dest, a)

#define LD_REG_REG8_helper(dest, src) [](CPU& cpu) { \
  cpu.dest = cpu.src; \
}

#define LD_REG_REG8_HL_LOC_helper(dest) [](CPU& cpu) { \
  word loc = (cpu.h << 8) | cpu.l; \
  cpu.dest = cpu.mmu[loc]; \
}

#define HALT() [](CPU& cpu) { \
  cpu.halt(); \
}

#define LD_HL_SPECIAL() LD_HL_SPECIAL_helper(b), \
LD_HL_SPECIAL_helper(c), \
LD_HL_SPECIAL_helper(d), \
LD_HL_SPECIAL_helper(e), \
LD_HL_SPECIAL_helper(h), \
LD_HL_SPECIAL_helper(l), \
HALT(), \
LD_HL_SPECIAL_helper(a)

#define LD_HL_SPECIAL_helper(src) [](CPU& cpu) { \
  word loc = (cpu.h << 8) | cpu.l; \
  cpu.mmu.set(loc, cpu.src); \
}

#define JR_COND_r8(cond) [](CPU& cpu) { \
  if ((cond)) { \
    int8_t r8 = static_cast<int8_t>(cpu.mmu[cpu.pc]); \
    cpu.pc += r8 + 1; \
    cpu.cycles += 4; \
  } else { \
    cpu.pc += 1; \
  } \
}

#define CPL() [](CPU& cpu) { \
  cpu.a = ~cpu.a; \
  cpu.set_flags(Nf | Hf); \
}

#define SCF() [](CPU& cpu) { \
  cpu.set_flags(Cf); \
  cpu.unset_flags(Nf | Hf); \
}

#define CCF() [](CPU& cpu) { \
  cpu.toggle_flags(Cf); \
  cpu.unset_flags(Nf | Hf); \
}

#define ADD_A8_HELPER(src) [](CPU& cpu) { \
  cpu.a += cpu.src; \
  cpu.unset_flags(Nf); \
  if (cpu.a == 0x0) { \
    cpu.set_flags(Zf); \
  } else { \
    cpu.unset_flags(Zf); \
  } \
  /* need to set H, C conditionally */ \
}

#define ADD_A8() ADD_A8_HELPER(b), \
ADD_A8_HELPER(c), \
ADD_A8_HELPER(d), \
ADD_A8_HELPER(e), \
ADD_A8_HELPER(h), \
ADD_A8_HELPER(l), \
ADD_A8_HL_LOC_HELPER(), \
ADD_A8_HELPER(a)

#define ADD_A8_HL_LOC_HELPER() [](CPU& cpu) { \
  word loc = (cpu.h << 8) | cpu.l; \
  cpu.a += cpu.mmu[loc]; \
  cpu.unset_flags(Nf); \
  if (cpu.a == 0x0) { \
    cpu.set_flags(Zf); \
  } else { \
    cpu.unset_flags(Zf); \
  } \
  /* need to set H, C conditionally */ \
}

#define ADC_A8_HELPER(src) [](CPU& cpu) { \
  if (cpu.src + cpu.C() <= 0xff && cpu.src + cpu.C() + cpu.a >= 0x0) { \
    cpu.set_flags(Cf); \
  } else { \
    cpu.unset_flags(Cf); \
  } \
  cpu.a += (cpu.src + cpu.C()); \
  cpu.unset_flags(Nf); \
  if (cpu.a == 0x0) { \
    cpu.set_flags(Zf); \
  } else { \
    cpu.unset_flags(Zf); \
  } \
  /* need to set H, C conditionally */ \
}

#define ADC_A8_HL_LOC_HELPER() [](CPU& cpu) { \
  word loc = (cpu.h << 8) | cpu.l; \
  cpu.a += (cpu.mmu[loc] + cpu.C()); \
  cpu.unset_flags(Nf); \
  if (cpu.a == 0x0) { \
    cpu.set_flags(Zf); \
  } else { \
    cpu.unset_flags(Zf); \
  } \
  /* need to set H, C conditionally */ \
}

#define ADC_A8() ADC_A8_HELPER(b), \
ADC_A8_HELPER(c), \
ADC_A8_HELPER(d), \
ADC_A8_HELPER(e), \
ADC_A8_HELPER(h), \
ADC_A8_HELPER(l), \
ADC_A8_HL_LOC_HELPER(), \
ADC_A8_HELPER(a)

#define GEN8_HELPER(op, src) [](CPU& cpu) { \
  cpu.a = cpu.a op cpu.src; \
  cpu.set_flags(Nf); \
  if (cpu.a == 0x0) { \
    cpu.set_flags(Zf); \
  } else { \
    cpu.unset_flags(Zf); \
  } \
  /* need to set H, C conditionally */ \
}

#define GEN8_HL_LOC_HELPER(op) [](CPU& cpu) { \
  word loc = (cpu.h << 8) | cpu.l; \
  cpu.a = cpu.a op cpu.mmu[loc]; \
  cpu.set_flags(Nf); /* TODO: N needs to be set depending on the op */ \
  if (cpu.a == 0x0) { \
    cpu.set_flags(Zf); \
  } else { \
    cpu.unset_flags(Zf); \
  } \
  /* need to set H, C conditionally */ \
}

#define GEN8(op) GEN8_HELPER(op, b), \
GEN8_HELPER(op, c), \
GEN8_HELPER(op, d), \
GEN8_HELPER(op, e), \
GEN8_HELPER(op, h), \
GEN8_HELPER(op, l), \
GEN8_HL_LOC_HELPER(op), \
GEN8_HELPER(op, a)

#define SBC_A8_HELPER(src) [](CPU& cpu) { \
  if (cpu.src + cpu.C() > cpu.a) { \
    cpu.set_flags(Cf); \
  } else { \
    cpu.unset_flags(Cf); \
  } \
  cpu.a -= (cpu.src + cpu.C()); \
  cpu.set_flags(Nf); \
  if (cpu.a == 0x0) { \
    cpu.set_flags(Zf); \
  } else { \
    cpu.unset_flags(Zf); \
  } \
  /* need to set H, C conditionally */ \
}

#define SBC_A8_HL_LOC_HELPER() [](CPU& cpu) { \
  word loc = (cpu.h << 8) | cpu.l; \
  if (cpu.mmu[loc] + cpu.C() > cpu.a) { \
    cpu.set_flags(Cf); \
  } else { \
    cpu.unset_flags(Cf); \
  } \
  cpu.a -= (cpu.mmu[loc] + cpu.C()); \
  cpu.set_flags(Nf); \
  if (cpu.a == 0x0) { \
    cpu.set_flags(Zf); \
  } else { \
    cpu.unset_flags(Zf); \
  } \
  /* need to set H, C conditionally */ \
}

#define SBC_A8() SBC_A8_HELPER(b), \
SBC_A8_HELPER(c), \
SBC_A8_HELPER(d), \
SBC_A8_HELPER(e), \
SBC_A8_HELPER(h), \
SBC_A8_HELPER(l), \
SBC_A8_HL_LOC_HELPER(), \
SBC_A8_HELPER(a)

#define CP8_HELPER(src) [](CPU& cpu) { \
  byte result = (cpu.a - cpu.src); \
  cpu.set_flags(Nf); \
  if (result == 0) { \
    cpu.set_flags(Zf); \
  } else { \
    cpu.unset_flags(Zf); \
    if (result < 0) { \
      cpu.set_flags(Cf); \
    } else { \
      cpu.unset_flags(Cf); \
    } \
  } \
  /* need to set H conditionally */ \
}

#define CP8_HL_LOC_HELPER() [](CPU& cpu) { \
  word loc = (cpu.h << 8) | cpu.l; \
  byte result = (cpu.a - cpu.mmu[loc]); \
  cpu.set_flags(Nf); \
  if (result == 0) { \
    cpu.set_flags(Zf); \
  } else { \
    cpu.unset_flags(Zf); \
    if (result < 0) { \
      cpu.set_flags(Cf); \
    } else { \
      cpu.unset_flags(Cf); \
    } \
  } \
  /* need to set H conditionally */ \
}

#define CP8() CP8_HELPER(b), \
CP8_HELPER(c), \
CP8_HELPER(d), \
CP8_HELPER(e), \
CP8_HELPER(h), \
CP8_HELPER(l), \
CP8_HL_LOC_HELPER(), \
CP8_HELPER(a)

#define RET_COND(cond) [](CPU& cpu) { \
  if (cond) { \
    word loc = (cpu.mmu[cpu.sp + 1] << 8) | cpu.mmu[cpu.sp + 2]; \
    cpu.sp += 2; \
    cpu.pc = loc; \
    cpu.cycles += 12; \
  } \
}

#define POP_WORD(hi, lo) [](CPU& cpu) { \
  cpu.hi = cpu.mmu[cpu.sp + 1]; \
  cpu.lo = cpu.mmu[cpu.sp + 2]; \
  cpu.sp += 2; \
}

#define PUSH_WORD(hi, lo) [](CPU& cpu) { \
  cpu.mmu.set(cpu.sp - 1, cpu.hi); \
  cpu.mmu.set(cpu.sp, cpu.lo); \
  cpu.sp -= 2; \
}

#define JP_COND_a16(cond) [](CPU& cpu) { \
  if (cond) { \
    word a16 = cpu.get_word(); \
    cpu.pc = a16; \
    cpu.cycles += 4; \
  } else { \
    cpu.pc += 2; \
  } \
}

#define RST(addr) [](CPU& cpu) { \
  cpu.mmu.set(cpu.sp - 1, cpu.pc >> 8); \
  cpu.mmu.set(cpu.sp, cpu.pc & 0xff); \
  cpu.sp -= 2; \
  cpu.pc = addr; \
}

#define CALL_COND_a16(cond) [](CPU& cpu) { \
  word a16 = cpu.get_word(); \
  cpu.pc += 2; \
  if (cond) { \
    cpu.mmu.set(cpu.sp - 1, cpu.pc >> 8); \
    cpu.mmu.set(cpu.sp, cpu.pc & 0xff); \
    cpu.sp -= 2; \
    cpu.pc = a16; \
    cpu.cycles += 12; \
  } \
}

#define ADD_A_d8() [](CPU& cpu) { \
  byte d8 = cpu.mmu[cpu.pc]; \
  cpu.pc += 1; \
  cpu.a += d8; \
  if (cpu.a == 0x0) { \
    cpu.set_flags(Zf); \
  } else { \
    cpu.unset_flags(Zf); \
  } \
  cpu.unset_flags(Nf); \
  /* TODO: set H, C */ \
}

#define ADC_A_d8() [](CPU& cpu) { \
  byte d8 = cpu.mmu[cpu.pc]; \
  cpu.pc += 1; \
  cpu.a += (d8 + cpu.C()); \
  if (cpu.a == 0x0) { \
    cpu.set_flags(Zf); \
  } else { \
    cpu.unset_flags(Zf); \
  } \
  cpu.unset_flags(Nf); \
  /* TODO: set H, C */ \
}

#define SUB_A_d8() [](CPU& cpu) { \
  byte d8 = cpu.mmu[cpu.pc]; \
  cpu.pc += 1; \
  cpu.a -= d8; \
  cpu.set_flags(Nf); \
  if (cpu.a == 0x0) { \
    cpu.set_flags(Zf); \
  } else { \
    cpu.unset_flags(Zf); \
  } \
  /* TODO: set H, C */ \
}

#define SBC_A_d8() [](CPU& cpu) { \
  byte d8 = cpu.mmu[cpu.pc]; \
  cpu.pc += 1; \
  cpu.a -= (d8 + cpu.C()); \
  cpu.set_flags(Nf); \
  if (cpu.a == 0x0) { \
    cpu.set_flags(Zf); \
  } else { \
    cpu.unset_flags(Zf); \
  } \
  /* TODO: set H, C */ \
}

#define RETI() [](CPU& cpu) { \
  word loc = (cpu.mmu[cpu.sp + 1] << 8) | cpu.mmu[cpu.sp + 2]; \
  cpu.sp += 2; \
  cpu.pc = loc; \
  cpu.enable_interrupts(); \
}

#define OP_d8(op) [](CPU& cpu) { \
  byte d8 = cpu.mmu[cpu.pc]; \
  cpu.pc += 1; \
  cpu.a = cpu.a op d8; \
  if (cpu.a == 0x0) { \
    cpu.set_flags(Zf); \
  } else { \
    cpu.unset_flags(Zf); \
  } \
  /* need to set N, H, C conditionally */ \
}

#define CP_d8() [](CPU& cpu) { \
  byte d8 = cpu.mmu[cpu.pc]; \
  cpu.pc += 1; \
  byte result = (cpu.a - d8); \
  cpu.set_flags(Nf); \
  if (result == 0) { \
    cpu.set_flags(Zf); \
  } else { \
    cpu.unset_flags(Zf); \
    if (result < 0) { \
      cpu.set_flags(Cf); \
    } else { \
      cpu.unset_flags(Cf); \
    } \
  } \
  /* need to set H conditionally */ \
}

#define JP_HL() [](CPU& cpu) { \
  word hl = (cpu.h << 8) | cpu.l; \
  cpu.pc = hl; \
}

#define LD_SP_HL() [](CPU& cpu) { \
  word hl = (cpu.h << 8) | cpu.l; \
  cpu.sp = cpu.mmu[hl]; \
}

#define LD_A_a16() [](CPU& cpu) { \
  word a16 = cpu.get_word(); \
  cpu.pc += 2; \
  cpu.a = cpu.mmu[a16]; \
}

#define LD_a16_A() [](CPU& cpu) { \
  word a16 = cpu.get_word(); \
  cpu.pc += 2; \
  cpu.mmu.set(a16, cpu.a); \
}

#define ADD_SP_r8() [](CPU& cpu) { \
  int8_t r8 = static_cast<int8_t>(cpu.mmu[cpu.pc + 1]); \
  cpu.pc += 1; \
  cpu.sp += r8; \
  cpu.unset_flags(Zf | Nf); \
  /* need to set H, C conditionally */ \
}

#define LD_HL_SP_plus_r8() [](CPU& cpu) { \
  int8_t r8 = static_cast<int8_t>(cpu.mmu[cpu.pc + 1]); \
  cpu.pc += 1; \
  word hl = cpu.sp + r8; \
  cpu.h = hl >> 8; \
  cpu.l = hl & 0xff; \
  cpu.unset_flags(Zf | Nf); \
  /* need to set H, C conditionally */ \
}

#define LD_LOC_REG_AUG(op, hi, lo, reg) [](CPU& cpu) { \
  word hl = (cpu.hi << 8) | cpu.lo; \
  cpu.mmu.set(hl, cpu.reg); \
  hl = hl op 1; \
  cpu.hi = hl >> 8; \
  cpu.lo = hl & 0xff; \
}

#define LDH_A_a8() [](CPU& cpu) { \
  byte a8 = cpu.mmu[cpu.pc]; \
  cpu.pc += 1; \
  cpu.a = cpu.mmu[0xff00 + a8]; \
}

#define LDH_a8_A() [](CPU& cpu) { \
  byte a8 = cpu.mmu[cpu.pc]; \
  cpu.pc += 1; \
  cpu.mmu.set(0xff00 + a8, cpu.a); \
}

#define LDH_ADDR_A(reg) [](CPU& cpu) { \
  cpu.mmu.set(0xff00 + cpu.reg, cpu.a); \
}

#define LDH_A_ADDR(reg) [](CPU& cpu) { \
  cpu.a = cpu.mmu[0xff00 + cpu.reg]; \
}

#define LD_REG_LOC_AUG(reg, op, hi, lo) [](CPU& cpu) { \
  word hl = (cpu.hi << 8) | cpu.lo; \
  cpu.reg = cpu.mmu[hl]; \
  hl = hl op 1; \
  cpu.hi = hl >> 8; \
  cpu.lo = hl & 0xff; \
}

#define INVALID() [](CPU& cpu) { \
  throw std::runtime_error("Invalid opcode"); \
}

#define UNIMPL() [](CPU& cpu) { \
  throw std::runtime_error("Unimplemented opcode"); \
}

#define DI() [](CPU& cpu) { \
  cpu.disable_interrupts_next_instruction(); \
}

#define EI() [](CPU& cpu) { \
  cpu.enable_interrupts_next_instruction(); \
}

#define STOP() [](CPU& cpu) { \
  cpu.pc += 1; \
  cpu.stop(); \
}
