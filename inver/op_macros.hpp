#pragma once

// Macros for generating ops:
// set N, Z
#define OP(op, mode) [](CPU6502& cpu) {\
  byte operand = cpu.deref_##mode(); \
  cpu.a = cpu.a op operand; \
  cpu.check_zn_flags(cpu.a); \
  return cpu.observe_crossed_page(); \
}

#define GEN_A(body_macro_name) [](CPU6502& cpu) {\
  body_macro_name(cpu.a); \
  return cpu.observe_crossed_page(); \
}

#define GEN(body_macro_name, mode) [](CPU6502& cpu) {\
  word addr = cpu.addr_##mode(); \
  byte operand = cpu.read(addr); \
  body_macro_name(operand); \
  cpu.write(addr, operand); \
  return cpu.observe_crossed_page(); \
}

// set I=1
#define BRK [](CPU6502& cpu) { \
  cpu.p.I = 1; \
  cpu.push_word(cpu.pc + 2); \
  cpu.push(cpu.p.reg); \
  return 0; \
}

#define NOP [](CPU6502& cpu) { return 0; }
#define XXX [](CPU6502& cpu) { return 0; }

#define JMP(mode) [](CPU6502& cpu) {\
  cpu.pc = cpu.addr_##mode(); \
  return 0; \
}

#define PHP [](CPU6502& cpu) {\
  cpu.push(cpu.p.reg | 0b00110000); \
  return 0; \
}

#define PHA [](CPU6502& cpu) {\
  cpu.push(cpu.a); \
  return 0; \
}

#define ADC_GEN(mode, unary_op) [](CPU6502& cpu) {\
  byte operand = unary_op cpu.deref_##mode(); \
  byte acc = cpu.a; \
  byte addend = operand + cpu.p.C; \
  int result = cpu.a + addend; \
  cpu.p.C = result > 255; \
  cpu.a = static_cast<byte>(result); \
  cpu.check_zn_flags(cpu.a); \
  cpu.p.V = ((acc ^ cpu.a) & (operand ^ cpu.a) & 0x80) != 0; \
  return cpu.observe_crossed_page(); \
}

// set N, Z, C, V
#define ADC(mode) ADC_GEN(mode, +)
#define SBC(mode) ADC_GEN(mode, ~)

#define ASL_BODY(operand) \
  bool msb = operand & 0x80; \
  operand = (operand << 1) & 0xfe; \
  cpu.p.C = msb; \
  cpu.check_zn_flags(operand);

// set N=0, Z, C
#define LSR_BODY(operand) \
  bool lsb = operand & 0b1; \
  operand = (operand >> 1) & 0x7f; \
  cpu.p.C = lsb; \
  cpu.p.Z = (operand == 0); \
  cpu.p.N = 0;

#define ROL_BODY(operand) \
  bool msb = (operand & 0x80) != 0; \
  operand = cpu.p.C | (cpu.a << 1); \
  cpu.p.C = msb; \
  cpu.check_zn_flags(operand);

#define ROR_BODY(operand) \
  bool lsb = operand & 0b1; \
  operand = (cpu.p.C << 7) | (operand >> 1); \
  cpu.p.C = lsb; \
  cpu.check_zn_flags(operand);

#define ROL_A GEN_A(ROL_BODY)
#define ROL(mode) GEN(ROL_BODY, mode)

#define ROR_A GEN_A(ROR_BODY)
#define ROR(mode) GEN(ROR_BODY, mode)

#define ASL_A GEN_A(ASL_BODY)
#define ASL(mode) GEN(ASL_BODY, mode)

#define LSR_A GEN_A(LSR_BODY)
#define LSR(mode) GEN(LSR_BODY, mode)

// Exceptionally, STA incurs the same number of cycles
// regardless of whether the access crosses a page boundary.
#define ST_NO_PAGE_CHECK(reg, mode) [](CPU6502& cpu) {\
  word addr = cpu.addr_##mode(); \
  cpu.write(addr, cpu.reg); \
  cpu.reset_crossed_page(); \
  return 0; \
}

#define ST(reg, mode) [](CPU6502& cpu) {\
  word addr = cpu.addr_##mode(); \
  cpu.write(addr, cpu.reg); \
  return cpu.observe_crossed_page(); \
}

// set N, Z
#define LD(reg, mode) [](CPU6502& cpu) {\
  auto operand = cpu.deref_##mode(); \
  cpu.reg = operand; \
  cpu.check_zn_flags(operand); \
  return cpu.observe_crossed_page(); \
}

// set N, Z, C
#define CMP(mode) [](CPU6502& cpu) {\
  byte val = cpu.deref_##mode(); \
  byte operand = cpu.a - val; \
  cpu.check_zn_flags(operand); \
  cpu.p.C = cpu.a >= val; \
  return cpu.observe_crossed_page(); \
}

#define INC_GEN(mode, increment) [](CPU6502& cpu) {\
  word addr = cpu.addr_##mode(); \
  byte operand = cpu.read(addr); \
  byte result = operand + increment; \
  cpu.write(addr, result); \
  cpu.check_zn_flags(result); \
  return cpu.observe_crossed_page(); \
}

// set N, Z
#define DEC(mode) INC_GEN(mode, -1)
#define INC(mode) INC_GEN(mode, +1)

#define JSR [](CPU6502& cpu) {\
  cpu.push_word(cpu.pc + 1); \
  cpu.pc = cpu.read_word(); \
  return 0; \
}

// set all flags
#define RTI [](CPU6502& cpu) {\
  cpu.pop_flags(); \
  cpu.pc = cpu.pop_word(); \
  return 0; \
}

#define RTS [](CPU6502& cpu) {\
  cpu.rts(); \
  return 0; \
}

#define BRANCH(cond) [](CPU6502& cpu) {\
  if (cond) { \
    return 1 + cpu.branch_with_offset(); \
  } else { \
    ++cpu.pc; \
    return 0; \
  } \
}

//set N, Z
#define IN(reg) [](CPU6502& cpu) {\
  ++cpu.reg; \
  cpu.check_zn_flags(cpu.reg); \
  return 0; \
}

//set N, Z
#define DE(reg) [](CPU6502& cpu) {\
  --cpu.reg; \
  cpu.check_zn_flags(cpu.reg); \
  return 0; \
}

// load into N, V, set Z
#define BIT(mode) [](CPU6502& cpu) {\
  byte operand = cpu.deref_##mode(); \
  cpu.p.N = (operand & 0x80) != 0; \
  cpu.p.V = (operand & 0x40) != 0; \
  cpu.p.Z = (operand & cpu.a) == 0; \
  return 0; \
}

// set N, Z
#define PLA [](CPU6502& cpu) {\
  cpu.a = cpu.pop(); \
  cpu.check_zn_flags(cpu.a); \
  return 0; \
}

#define PLP [](CPU6502& cpu) {\
  cpu.pop_flags(); \
  return 0; \
}

#define CL(reg) [](CPU6502& cpu) {\
  cpu.p.reg = 0; \
  return 0; \
}

#define SE(reg) [](CPU6502& cpu) {\
  cpu.p.reg = 1; \
  return 0; \
}

// set N, Z, C
#define CP(reg, mode) [](CPU6502& cpu) {\
  byte val = cpu.deref_##mode(); \
  byte datum = cpu.reg - val; \
  cpu.check_zn_flags(datum); \
  cpu.p.C = (cpu.reg >= val); \
  return 0; \
}

#define TXS [](CPU6502& cpu) {\
  cpu.sp = cpu.x; \
  return 0; \
}

#define TSX [](CPU6502& cpu) {\
  cpu.x = cpu.sp; \
  return 0; \
}

// set N, Z
#define T__(src, dst) [](CPU6502& cpu) {\
  cpu.dst = cpu.src; \
  cpu.check_zn_flags(cpu.dst); \
  return 0; \
}
