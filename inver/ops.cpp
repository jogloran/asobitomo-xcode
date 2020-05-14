#include "ops.hpp"
#include "op_macros.hpp"

std::array<cycle_count_t, 256> cycle_counts {
  7,6,0,0,0,3,5,0,3,2,2,0,0,4,6,0,
  2,5,0,0,0,4,6,0,2,4,0,0,0,4,7,0,
  6,6,0,0,3,3,5,0,4,2,2,0,4,4,6,0,
  2,5,0,0,0,4,6,0,2,4,0,0,0,4,7,0,
  6,6,0,0,0,3,5,0,3,2,2,0,3,4,6,0,
  2,5,0,0,0,4,6,0,2,4,0,0,0,4,7,0,
  6,6,0,0,0,3,5,0,4,2,2,0,5,4,6,0,
  2,5,0,0,0,4,6,0,2,4,0,0,0,4,7,0,
  0,6,0,0,3,3,3,0,2,0,2,0,4,4,4,0,
  2,6,0,0,4,4,4,0,2,5,2,0,0,5,0,0,
  2,6,2,0,3,3,3,0,2,2,2,0,4,4,4,0,
  2,5,0,0,4,4,4,0,2,4,2,0,4,4,4,0,
  2,6,0,0,3,3,5,0,2,2,2,0,4,4,6,0,
  2,5,0,0,0,4,6,0,2,4,0,0,0,4,7,0,
  2,6,0,0,3,3,5,0,2,2,2,0,4,4,6,0,
  2,5,0,0,0,4,6,0,2,4,0,0,0,4,7,0,
};

std::array<op_t, 256> ops {
  BRK,                          // 0x00
  OP(|, x_indirect),            // 0x01
  XXX,                          // 0x02
  XXX,                          // 0x03
  XXX,                          // 0x04
  OP(|, zpg),                   // 0x05
  ASL(zpg),                     // 0x06
  XXX,                          // 0x07
  PHP,                          // 0x08
  OP(|, imm),                   // 0x09
  ASL_A,                        // 0x0a
  XXX,                          // 0x0b
  XXX,                          // 0x0c
  OP(|, abs),                   // 0x0d
  ASL(abs),                     // 0x0e
  XXX,                          // 0x0f

  BRANCH((!cpu.p.N)),           // 0x10
  OP(|, indirect_y),            // 0x11
  XXX,                          // 0x12
  XXX,                          // 0x13
  XXX,                          // 0x14
  OP(|, zpg_plus_x),            // 0x15
  ASL(zpg_plus_x),              // 0x16
  XXX,                          // 0x17
  CL(C),                        // 0x18
  OP(|, abs_plus_y),            // 0x19
  XXX,                          // 0x1a
  XXX,                          // 0x1b
  XXX,                          // 0x1c
  OP(|, abs_plus_x),            // 0x1d
  ASL(abs_plus_x),              // 0x1e
  XXX,                          // 0x1f

  JSR,                          // 0x20
  OP(&, x_indirect),            // 0x21
  XXX,                          // 0x22
  XXX,                          // 0x23
  BIT(zpg),                     // 0x24
  OP(&, zpg),                   // 0x25
  ROL(zpg),                     // 0x26
  XXX,                          // 0x27
  PLP,                          // 0x28
  OP(&, imm),                   // 0x29
  ROL_A,                        // 0x2a
  XXX,                          // 0x2b
  BIT(abs),                     // 0x2c
  OP(&, abs),                   // 0x2d
  ROL(abs),                     // 0x2e
  XXX,                          // 0x2f

  BRANCH((cpu.p.N)),            // 0x30
  OP(&, indirect_y),            // 0x31
  XXX,                          // 0x32
  XXX,                          // 0x33
  XXX,                          // 0x34
  OP(&, zpg_plus_x),            // 0x35
  ROL(zpg_plus_x),              // 0x36
  XXX,                          // 0x37
  SE(C),                        // 0x38
  OP(&, abs_plus_y),            // 0x39
  XXX,                          // 0x3a
  XXX,                          // 0x3b
  XXX,                          // 0x3c
  OP(&, abs_plus_x),            // 0x3d
  ROL(abs_plus_x),              // 0x3e
  XXX,                          // 0x3f

  RTI,                          // 0x40
  OP(^, x_indirect),            // 0x41
  XXX,                          // 0x42
  XXX,                          // 0x43
  XXX,                          // 0x44
  OP(^, zpg),                   // 0x45
  LSR(zpg),                     // 0x46
  XXX,                          // 0x47
  PHA,                          // 0x48
  OP(^, imm),                   // 0x49
  LSR_A,                        // 0x4a
  XXX,                          // 0x4b
  JMP(abs),                     // 0x4c
  OP(^, abs),                   // 0x4d
  LSR(abs),                     // 0x4e
  XXX,                          // 0x4f

  BRANCH((!cpu.p.V)),           // 0x50
  OP(^, indirect_y),            // 0x51
  XXX,                          // 0x52
  XXX,                          // 0x53
  XXX,                          // 0x54
  OP(^, zpg_plus_x),            // 0x55
  LSR(zpg_plus_x),              // 0x56
  XXX,                          // 0x57
  CL(I),                        // 0x58
  OP(^, abs_plus_y),            // 0x59
  XXX,                          // 0x5a
  XXX,                          // 0x5b
  XXX,                          // 0x5c
  OP(^, abs_plus_x),            // 0x5d
  LSR(abs_plus_x),              // 0x5e
  XXX,                          // 0x5f

  RTS,                          // 0x60
  ADC(x_indirect),              // 0x61
  XXX,                          // 0x62
  XXX,                          // 0x63
  XXX,                          // 0x64
  ADC(zpg),                     // 0x65
  ROR(zpg),                     // 0x66
  XXX,                          // 0x67
  PLA,                          // 0x68
  ADC(imm),                     // 0x69
  ROR_A,                        // 0x6a
  XXX,                          // 0x6b
  JMP(indirect),                // 0x6c
  ADC(abs),                     // 0x6d
  ROR(abs),                     // 0x6e
  XXX,                          // 0x6f

  BRANCH((cpu.p.V)),            // 0x70
  ADC(indirect_y),              // 0x71
  XXX,                          // 0x72
  XXX,                          // 0x73
  XXX,                          // 0x74
  ADC(zpg_plus_x),              // 0x75
  ROR(zpg_plus_x),              // 0x76
  XXX,                          // 0x77
  SE(I),                        // 0x78
  ADC(abs_plus_y),              // 0x79
  XXX,                          // 0x7a
  XXX,                          // 0x7b
  XXX,                          // 0x7c
  ADC(abs_plus_x),              // 0x7d
  ROR(abs_plus_x),              // 0x7e
  XXX,                          // 0x7f

  XXX,                          // 0x80
  ST(a, x_indirect),            // 0x81
  XXX,                          // 0x82
  XXX,                          // 0x83
  ST(y, zpg),                   // 0x84
  ST(a, zpg),                   // 0x85
  ST(x, zpg),                   // 0x86
  XXX,                          // 0x87
  DE(y),                        // 0x88
  XXX,                          // 0x89
  T__(x, a),                    // 0x8a
  XXX,                          // 0x8b
  ST(y, abs),                   // 0x8c
  ST(a, abs),                   // 0x8d
  ST(x, abs),                   // 0x8e
  XXX,                          // 0x8f

  BRANCH((!cpu.p.C)),           // 0x90
  ST_NO_PAGE_CHECK(a, indirect_y), // 0x91
  XXX,                          // 0x92
  XXX,                          // 0x93
  ST(y, zpg_plus_x),            // 0x94
  ST(a, zpg_plus_x),            // 0x95
  ST(x, zpg_plus_y),            // 0x96
  XXX,                          // 0x97
  T__(y, a),                    // 0x98
  ST_NO_PAGE_CHECK(a, abs_plus_y), // 0x99
  TXS,                          // 0x9a
  XXX,                          // 0x9b
  XXX,                          // 0x9c
  ST_NO_PAGE_CHECK(a, abs_plus_x), // 0x9d
  XXX,                          // 0x9e
  XXX,                          // 0x9f

  LD(y, imm),                   // 0xa0
  LD(a, x_indirect),            // 0xa1
  LD(x, imm),                   // 0xa2
  XXX,                          // 0xa3
  LD(y, zpg),                   // 0xa4
  LD(a, zpg),                   // 0xa5
  LD(x, zpg),                   // 0xa6
  XXX,                          // 0xa7
  T__(a, y),                    // 0xa8
  LD(a, imm),                   // 0xa9
  T__(a, x),                    // 0xaa
  XXX,                          // 0xab
  LD(y, abs),                   // 0xac
  LD(a, abs),                   // 0xad
  LD(x, abs),                   // 0xae
  XXX,                          // 0xaf

  BRANCH((cpu.p.C)),            // 0xb0
  LD(a, indirect_y),            // 0xb1
  XXX,                          // 0xb2
  XXX,                          // 0xb3
  LD(y, zpg_plus_x),            // 0xb4
  LD(a, zpg_plus_x),            // 0xb5
  LD(x, zpg_plus_y),            // 0xb6
  XXX,                          // 0xb7
  CL(V),                        // 0xb8
  LD(a, abs_plus_y),            // 0xb9
  T__(sp, x),                   // 0xba
  XXX,                          // 0xbb
  LD(y, abs_plus_x),            // 0xbc
  LD(a, abs_plus_x),            // 0xbd
  LD(x, abs_plus_y),            // 0xbe
  XXX,                          // 0xbf

  CP(y, imm),                   // 0xc0
  CMP(x_indirect),              // 0xc1
  XXX,                          // 0xc2
  XXX,                          // 0xc3
  CP(y, zpg),                   // 0xc4
  CMP(zpg),                     // 0xc5
  DEC(zpg),                     // 0xc6
  XXX,                          // 0xc7
  IN(y),                        // 0xc8
  CMP(imm),                     // 0xc9
  DE(x),                        // 0xca
  XXX,                          // 0xcb
  CP(y, abs),                   // 0xcc
  CMP(abs),                     // 0xcd
  DEC(abs),                     // 0xce
  XXX,                          // 0xcf

  BRANCH((!cpu.p.Z)),           // 0xd0
  CMP(indirect_y),              // 0xd1
  XXX,                          // 0xd2
  XXX,                          // 0xd3
  XXX,                          // 0xd4
  CMP(zpg_plus_x),              // 0xd5
  DEC(zpg_plus_x),              // 0xd6
  XXX,                          // 0xd7
  CL(D),                        // 0xd8
  CMP(abs_plus_y),              // 0xd9
  XXX,                          // 0xda
  XXX,                          // 0xdb
  XXX,                          // 0xdc
  CMP(abs_plus_x),              // 0xdd
  DEC(abs_plus_x),              // 0xde
  XXX,                          // 0xdf

  CP(x, imm),                   // 0xe0
  SBC(x_indirect),              // 0xe1
  XXX,                          // 0xe2
  XXX,                          // 0xe3
  CP(x, zpg),                   // 0xe4
  SBC(zpg),                     // 0xe5
  INC(zpg),                     // 0xe6
  XXX,                          // 0xe7
  IN(x),                        // 0xe8
  SBC(imm),                     // 0xe9
  NOP,                          // 0xea
  XXX,                          // 0xeb
  CP(x, abs),                   // 0xec
  SBC(abs),                     // 0xed
  INC(abs),                     // 0xee
  XXX,                          // 0xef

  BRANCH((cpu.p.Z)),            // 0xf0
  SBC(indirect_y),              // 0xf1
  XXX,                          // 0xf2
  XXX,                          // 0xf3
  XXX,                          // 0xf4
  SBC(zpg_plus_x),              // 0xf5
  INC(zpg_plus_x),              // 0xf6
  XXX,                          // 0xf7
  SE(D),                        // 0xf8
  SBC(abs_plus_y),              // 0xf9
  XXX,                          // 0xfa
  XXX,                          // 0xfb
  XXX,                          // 0xfc
  SBC(abs_plus_x),              // 0xfd
  INC(abs_plus_x),              // 0xfe
  XXX,                          // 0xff
};
