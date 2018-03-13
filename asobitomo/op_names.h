//
//  op_names.h
//  asobitomo
//
//  Created by jogloran-s on 20.2.18.
//  Copyright © 2018 Overpunch. All rights reserved.
//

#pragma once

#include <array>
#include <string>

#include <cstdlib>

enum class Argument {
  None, Word, U16, U8, Offset
};

std::string formatted(const char* fmt, int value) {
  char buf[64];
  std::sprintf(buf, fmt, value);
  return std::string(buf);
}

struct Op {
  byte opcode;
  std::string mnemonic;
  const char* fmt;
  Argument arg;
  
  std::string instruction_at_pc(CPU& cpu) {
    switch (arg) {
    case Argument::Word: {
      byte hi = cpu.mmu[cpu.pc+2];
      byte lo = cpu.mmu[cpu.pc+1];
      word value = (hi << 8) | lo;
      
      return formatted(fmt, value);
      break;
    }
    case Argument::U16: {
      byte hi = cpu.mmu[cpu.pc+2];
      byte lo = cpu.mmu[cpu.pc+1];
      word value = (hi << 8) | lo;
      
      return formatted(fmt, value);
      break;
    }
    case Argument::U8: {
      byte value = cpu.mmu[cpu.pc+1];
      
      return formatted(fmt, value);
      break;
    }
    case Argument::Offset: {
      byte value = cpu.mmu[cpu.pc+1];
      signed char signed_value = static_cast<signed char>(value);
      word pc_plus_offset = cpu.pc + 2 + signed_value;
      
      return formatted(fmt, pc_plus_offset);
      break;
    }
    case Argument::None:
    default:
      return mnemonic;
      break;
    }
  }
};

std::array<Op, 256> op_names {
Op { 0x0, "NOP", "NOP", Argument::None },
Op { 0x1, "LD BC, %04x", "LD BC, %04x", Argument::U16 },
Op { 0x2, "LD (BC), A", "LD (BC), A", Argument::None },
Op { 0x3, "INC BC", "INC BC", Argument::None },
Op { 0x4, "INC B", "INC B", Argument::None },
Op { 0x5, "DEC B", "DEC B", Argument::None },
Op { 0x6, "LD B, %02x", "LD B, %02x", Argument::U8 },
Op { 0x7, "RLCA", "RLCA", Argument::None },
Op { 0x8, "LD (0x%04x), SP", "LD (0x%04x), SP", Argument::Word },
Op { 0x9, "ADD HL, BC", "ADD HL, BC", Argument::None },
Op { 0xa, "LD A, (BC)", "LD A, (BC)", Argument::None },
Op { 0xb, "DEC BC", "DEC BC", Argument::None },
Op { 0xc, "INC C", "INC C", Argument::None },
Op { 0xd, "DEC C", "DEC C", Argument::None },
Op { 0xe, "LD C, %02x", "LD C, %02x", Argument::U8 },
Op { 0xf, "RRCA", "RRCA", Argument::None },
Op { 0x10, "STOP", "STOP", Argument::None },
Op { 0x11, "LD DE, %04x", "LD DE, %04x", Argument::U16 },
Op { 0x12, "LD (DE), A", "LD (DE), A", Argument::None },
Op { 0x13, "INC DE", "INC DE", Argument::None },
Op { 0x14, "INC D", "INC D", Argument::None },
Op { 0x15, "DEC D", "DEC D", Argument::None },
Op { 0x16, "LD D, %02x", "LD D, %02x", Argument::U8 },
Op { 0x17, "RLA", "RLA", Argument::None },
Op { 0x18, "JR 0x%04x", "JR 0x%04x", Argument::Offset },
Op { 0x19, "ADD HL, DE", "ADD HL, DE", Argument::None },
Op { 0x1a, "LD A, (DE)", "LD A, (DE)", Argument::None },
Op { 0x1b, "DEC DE", "DEC DE", Argument::None },
Op { 0x1c, "INC E", "INC E", Argument::None },
Op { 0x1d, "DEC E", "DEC E", Argument::None },
Op { 0x1e, "LD E, %02x", "LD E, %02x", Argument::U8 },
Op { 0x1f, "RRA", "RRA", Argument::None },
Op { 0x20, "JR NZ, 0x%04x", "JR NZ, 0x%04x", Argument::Offset },
Op { 0x21, "LD HL, %04x", "LD HL, %04x", Argument::U16 },
Op { 0x22, "LD (HL+), A", "LD (HL+), A", Argument::None },
Op { 0x23, "INC HL", "INC HL", Argument::None },
Op { 0x24, "INC H", "INC H", Argument::None },
Op { 0x25, "DEC H", "DEC H", Argument::None },
Op { 0x26, "LD H, %02x", "LD H, %02x", Argument::U8 },
Op { 0x27, "DAA", "DAA", Argument::None },
Op { 0x28, "JR Z, 0x%04x", "JR Z, 0x%04x", Argument::Offset },
Op { 0x29, "ADD HL, HL", "ADD HL, HL", Argument::None },
Op { 0x2a, "LD A, (HL+)", "LD A, (HL+)", Argument::None },
Op { 0x2b, "DEC HL", "DEC HL", Argument::None },
Op { 0x2c, "INC L", "INC L", Argument::None },
Op { 0x2d, "DEC L", "DEC L", Argument::None },
Op { 0x2e, "LD L, %02x", "LD L, %02x", Argument::U8 },
Op { 0x2f, "CPL", "CPL", Argument::None },
Op { 0x30, "JR NC, 0x%04x", "JR NC, 0x%04x", Argument::Offset },
Op { 0x31, "LD SP, 0x%04x", "LD SP, 0x%04x", Argument::Word },
Op { 0x32, "LD (HL-), A", "LD (HL-), A", Argument::None },
Op { 0x33, "INC SP", "INC SP", Argument::None },
Op { 0x34, "INC (HL)", "INC (HL)", Argument::None },
Op { 0x35, "DEC (HL)", "DEC (HL)", Argument::None },
Op { 0x36, "LD (HL), %02x", "LD (HL), %02x", Argument::U8 },
Op { 0x37, "SCF", "SCF", Argument::None },
Op { 0x38, "JR C, 0x%04x", "JR C, 0x%04x", Argument::Offset },
Op { 0x39, "ADD HL, SP", "ADD HL, SP", Argument::None },
Op { 0x3a, "LD A, (HL-)", "LD A, (HL-)", Argument::None },
Op { 0x3b, "DEC SP", "DEC SP", Argument::None },
Op { 0x3c, "INC A", "INC A", Argument::None },
Op { 0x3d, "DEC A", "DEC A", Argument::None },
Op { 0x3e, "LD A, %02x", "LD A, %02x", Argument::U8 },
Op { 0x3f, "CCF", "CCF", Argument::None },
Op { 0x40, "LD B, B", "LD B, B", Argument::None },
Op { 0x41, "LD B, C", "LD B, C", Argument::None },
Op { 0x42, "LD B, D", "LD B, D", Argument::None },
Op { 0x43, "LD B, E", "LD B, E", Argument::None },
Op { 0x44, "LD B, H", "LD B, H", Argument::None },
Op { 0x45, "LD B, L", "LD B, L", Argument::None },
Op { 0x46, "LD B, (HL)", "LD B, (HL)", Argument::None },
Op { 0x47, "LD B, A", "LD B, A", Argument::None },
Op { 0x48, "LD C, B", "LD C, B", Argument::None },
Op { 0x49, "LD C, C", "LD C, C", Argument::None },
Op { 0x4a, "LD C, D", "LD C, D", Argument::None },
Op { 0x4b, "LD C, E", "LD C, E", Argument::None },
Op { 0x4c, "LD C, H", "LD C, H", Argument::None },
Op { 0x4d, "LD C, L", "LD C, L", Argument::None },
Op { 0x4e, "LD C, (HL)", "LD C, (HL)", Argument::None },
Op { 0x4f, "LD C, A", "LD C, A", Argument::None },
Op { 0x50, "LD D, B", "LD D, B", Argument::None },
Op { 0x51, "LD D, C", "LD D, C", Argument::None },
Op { 0x52, "LD D, D", "LD D, D", Argument::None },
Op { 0x53, "LD D, E", "LD D, E", Argument::None },
Op { 0x54, "LD D, H", "LD D, H", Argument::None },
Op { 0x55, "LD D, L", "LD D, L", Argument::None },
Op { 0x56, "LD D, (HL)", "LD D, (HL)", Argument::None },
Op { 0x57, "LD D, A", "LD D, A", Argument::None },
Op { 0x58, "LD E, B", "LD E, B", Argument::None },
Op { 0x59, "LD E, C", "LD E, C", Argument::None },
Op { 0x5a, "LD E, D", "LD E, D", Argument::None },
Op { 0x5b, "LD E, E", "LD E, E", Argument::None },
Op { 0x5c, "LD E, H", "LD E, H", Argument::None },
Op { 0x5d, "LD E, L", "LD E, L", Argument::None },
Op { 0x5e, "LD E, (HL)", "LD E, (HL)", Argument::None },
Op { 0x5f, "LD E, A", "LD E, A", Argument::None },
Op { 0x60, "LD H, B", "LD H, B", Argument::None },
Op { 0x61, "LD H, C", "LD H, C", Argument::None },
Op { 0x62, "LD H, D", "LD H, D", Argument::None },
Op { 0x63, "LD H, E", "LD H, E", Argument::None },
Op { 0x64, "LD H, H", "LD H, H", Argument::None },
Op { 0x65, "LD H, L", "LD H, L", Argument::None },
Op { 0x66, "LD H, (HL)", "LD H, (HL)", Argument::None },
Op { 0x67, "LD H, A", "LD H, A", Argument::None },
Op { 0x68, "LD L, B", "LD L, B", Argument::None },
Op { 0x69, "LD L, C", "LD L, C", Argument::None },
Op { 0x6a, "LD L, D", "LD L, D", Argument::None },
Op { 0x6b, "LD L, E", "LD L, E", Argument::None },
Op { 0x6c, "LD L, H", "LD L, H", Argument::None },
Op { 0x6d, "LD L, L", "LD L, L", Argument::None },
Op { 0x6e, "LD L, (HL)", "LD L, (HL)", Argument::None },
Op { 0x6f, "LD L, A", "LD L, A", Argument::None },
Op { 0x70, "LD (HL), B", "LD (HL), B", Argument::None },
Op { 0x71, "LD (HL), C", "LD (HL), C", Argument::None },
Op { 0x72, "LD (HL), D", "LD (HL), D", Argument::None },
Op { 0x73, "LD (HL), E", "LD (HL), E", Argument::None },
Op { 0x74, "LD (HL), H", "LD (HL), H", Argument::None },
Op { 0x75, "LD (HL), L", "LD (HL), L", Argument::None },
Op { 0x76, "HALT", "HALT", Argument::None },
Op { 0x77, "LD (HL), A", "LD (HL), A", Argument::None },
Op { 0x78, "LD A, B", "LD A, B", Argument::None },
Op { 0x79, "LD A, C", "LD A, C", Argument::None },
Op { 0x7a, "LD A, D", "LD A, D", Argument::None },
Op { 0x7b, "LD A, E", "LD A, E", Argument::None },
Op { 0x7c, "LD A, H", "LD A, H", Argument::None },
Op { 0x7d, "LD A, L", "LD A, L", Argument::None },
Op { 0x7e, "LD A, (HL)", "LD A, (HL)", Argument::None },
Op { 0x7f, "LD A, A", "LD A, A", Argument::None },
Op { 0x80, "ADD B", "ADD B", Argument::None },
Op { 0x81, "ADD C", "ADD C", Argument::None },
Op { 0x82, "ADD D", "ADD D", Argument::None },
Op { 0x83, "ADD E", "ADD E", Argument::None },
Op { 0x84, "ADD H", "ADD H", Argument::None },
Op { 0x85, "ADD L", "ADD L", Argument::None },
Op { 0x86, "ADD (HL)", "ADD (HL)", Argument::None },
Op { 0x87, "ADD A", "ADD A", Argument::None },
Op { 0x88, "ADC B", "ADC B", Argument::None },
Op { 0x89, "ADC C", "ADC C", Argument::None },
Op { 0x8a, "ADC D", "ADC D", Argument::None },
Op { 0x8b, "ADC E", "ADC E", Argument::None },
Op { 0x8c, "ADC H", "ADC H", Argument::None },
Op { 0x8d, "ADC L", "ADC L", Argument::None },
Op { 0x8e, "ADC (HL)", "ADC (HL)", Argument::None },
Op { 0x8f, "ADC A", "ADC A", Argument::None },
Op { 0x90, "SUB B", "SUB B", Argument::None },
Op { 0x91, "SUB C", "SUB C", Argument::None },
Op { 0x92, "SUB D", "SUB D", Argument::None },
Op { 0x93, "SUB E", "SUB E", Argument::None },
Op { 0x94, "SUB H", "SUB H", Argument::None },
Op { 0x95, "SUB L", "SUB L", Argument::None },
Op { 0x96, "SUB (HL)", "SUB (HL)", Argument::None },
Op { 0x97, "SUB A", "SUB A", Argument::None },
Op { 0x98, "SBC B", "SBC B", Argument::None },
Op { 0x99, "SBC C", "SBC C", Argument::None },
Op { 0x9a, "SBC D", "SBC D", Argument::None },
Op { 0x9b, "SBC E", "SBC E", Argument::None },
Op { 0x9c, "SBC H", "SBC H", Argument::None },
Op { 0x9d, "SBC L", "SBC L", Argument::None },
Op { 0x9e, "SBC (HL)", "SBC (HL)", Argument::None },
Op { 0x9f, "SBC A", "SBC A", Argument::None },
Op { 0xa0, "AND B", "AND B", Argument::None },
Op { 0xa1, "AND C", "AND C", Argument::None },
Op { 0xa2, "AND D", "AND D", Argument::None },
Op { 0xa3, "AND E", "AND E", Argument::None },
Op { 0xa4, "AND H", "AND H", Argument::None },
Op { 0xa5, "AND L", "AND L", Argument::None },
Op { 0xa6, "AND (HL)", "AND (HL)", Argument::None },
Op { 0xa7, "AND A", "AND A", Argument::None },
Op { 0xa8, "XOR B", "XOR B", Argument::None },
Op { 0xa9, "XOR C", "XOR C", Argument::None },
Op { 0xaa, "XOR D", "XOR D", Argument::None },
Op { 0xab, "XOR E", "XOR E", Argument::None },
Op { 0xac, "XOR H", "XOR H", Argument::None },
Op { 0xad, "XOR L", "XOR L", Argument::None },
Op { 0xae, "XOR (HL)", "XOR (HL)", Argument::None },
Op { 0xaf, "XOR A", "XOR A", Argument::None },
Op { 0xb0, "OR B", "OR B", Argument::None },
Op { 0xb1, "OR C", "OR C", Argument::None },
Op { 0xb2, "OR D", "OR D", Argument::None },
Op { 0xb3, "OR E", "OR E", Argument::None },
Op { 0xb4, "OR H", "OR H", Argument::None },
Op { 0xb5, "OR L", "OR L", Argument::None },
Op { 0xb6, "OR (HL)", "OR (HL)", Argument::None },
Op { 0xb7, "OR A", "OR A", Argument::None },
Op { 0xb8, "CP B", "CP B", Argument::None },
Op { 0xb9, "CP C", "CP C", Argument::None },
Op { 0xba, "CP D", "CP D", Argument::None },
Op { 0xbb, "CP E", "CP E", Argument::None },
Op { 0xbc, "CP H", "CP H", Argument::None },
Op { 0xbd, "CP L", "CP L", Argument::None },
Op { 0xbe, "CP (HL)", "CP (HL)", Argument::None },
Op { 0xbf, "CP A", "CP A", Argument::None },
Op { 0xc0, "RET NZ", "RET NZ", Argument::None },
Op { 0xc1, "POP BC", "POP BC", Argument::None },
Op { 0xc2, "JP NZ, 0x%04x", "JP NZ, 0x%04x", Argument::Word },
Op { 0xc3, "JP 0x%04x", "JP 0x%04x", Argument::Word },
Op { 0xc4, "CALL NZ, %04x", "CALL NZ, %04x", Argument::Word },
Op { 0xc5, "PUSH BC", "PUSH BC", Argument::None },
Op { 0xc6, "ADD %02x", "ADD %02x", Argument::U8 },
Op { 0xc7, "RST 0", "RST 0", Argument::None },
Op { 0xc8, "RET Z", "RET Z", Argument::None },
Op { 0xc9, "RET", "RET", Argument::None },
Op { 0xca, "JP Z, 0x%04x", "JP Z, 0x%04x", Argument::Word },
Op { 0xcb, "CB", "CB", Argument::None },
Op { 0xcc, "CALL Z, 0x%04x", "CALL Z, 0x%04x", Argument::Word },
Op { 0xcd, "CALL 0x%04x", "CALL 0x%04x", Argument::Word },
Op { 0xce, "ADC %02x", "ADC %02x", Argument::U8 },
Op { 0xcf, "RST 8", "RST 8", Argument::None },
Op { 0xd0, "RET NC", "RET NC", Argument::None },
Op { 0xd1, "POP DE", "POP DE", Argument::None },
Op { 0xd2, "JP NC, 0x%04x", "JP NC, 0x%04x", Argument::Word },
Op { 0xd3, "(invalid)", "(invalid)", Argument::None },
Op { 0xd4, "CALL NC, 0x%04x", "CALL NC, 0x%04x", Argument::None },
Op { 0xd5, "PUSH DE", "PUSH DE", Argument::None },
Op { 0xd6, "SUB %02x", "SUB %02x", Argument::U8 },
Op { 0xd7, "RST 10", "RST 10", Argument::None },
Op { 0xd8, "RET C", "RET C", Argument::None },
Op { 0xd9, "RETI", "RETI", Argument::None },
Op { 0xda, "JP C, 0x%04x", "JP C, 0x%04x", Argument::Word },
Op { 0xdb, "(invalid)", "(invalid)", Argument::None },
Op { 0xdc, "CALL C, 0x%04x", "CALL C, 0x%04x", Argument::Word },
Op { 0xdd, "(invalid)", "(invalid)", Argument::None },
Op { 0xde, "SBC %02x", "SBC %02x", Argument::U8 },
Op { 0xdf, "RST 18", "RST 18", Argument::None },
Op { 0xe0, "LD (0xff%02x), A", "LD (0xff%02x), A", Argument::U8 },
Op { 0xe1, "POP HL", "POP HL", Argument::None },
Op { 0xe2, "LD (0xff00+C), A", "LD (0xff00+C), A", Argument::None },
Op { 0xe3, "(invalid)", "(invalid)", Argument::None },
Op { 0xe4, "(invalid)", "(invalid)", Argument::None },
Op { 0xe5, "PUSH HL", "PUSH HL", Argument::None },
Op { 0xe6, "AND %02x", "AND %02x", Argument::U8 },
Op { 0xe7, "RST 20", "RST 20", Argument::None },
Op { 0xe8, "ADD SP, s", "ADD SP, s", Argument::None },
Op { 0xe9, "JP HL", "JP HL", Argument::None },
Op { 0xea, "LD (0x%04x), A", "LD (0x%04x), A", Argument::Word },
Op { 0xeb, "(invalid)", "(invalid)", Argument::None },
Op { 0xec, "(invalid)", "(invalid)", Argument::None },
Op { 0xed, "(invalid)", "(invalid)", Argument::None },
Op { 0xee, "XOR %02x", "XOR %02x", Argument::U8 },
Op { 0xef, "RST 28", "RST 28", Argument::None },
Op { 0xf0, "LD A, (0xff%02x)", "LD A, (0xff%02x)", Argument::U8 },
Op { 0xf1, "POP AF", "POP AF", Argument::None },
Op { 0xf2, "LD A, (0xff00+C)", "LD A, (0xff00+C)", Argument::None },
Op { 0xf3, "DI", "DI", Argument::None },
Op { 0xf4, "(invalid)", "(invalid)", Argument::None },
Op { 0xf5, "PUSH AF", "PUSH AF", Argument::None },
Op { 0xf6, "OR %02x", "OR %02x", Argument::U8 },
Op { 0xf7, "RST 30", "RST 30", Argument::None },
Op { 0xf8, "LD HL, SP+s", "LD HL, SP+s", Argument::None },
Op { 0xf9, "LD SP, HL", "LD SP, HL", Argument::None },
Op { 0xfa, "LD A, (0x%04x)", "LD A, (0x%04x)", Argument::Word },
Op { 0xfb, "EI", "EI", Argument::None },
Op { 0xfc, "(invalid)", "(invalid)", Argument::None },
Op { 0xfd, "(invalid)", "(invalid)", Argument::None },
Op { 0xfe, "CP %02x", "CP %02x", Argument::U8 },
Op { 0xff, "RST 38", "RST 38", Argument::None },
};

std::array<std::string, 256> cb_op_names {
"RLC B" /* 0xcb00 */,
"RLC C" /* 0xcb01 */,
"RLC D" /* 0xcb02 */,
"RLC E" /* 0xcb03 */,
"RLC H" /* 0xcb04 */,
"RLC L" /* 0xcb05 */,
"RLC (HL)" /* 0xcb06 */,
"RLC A" /* 0xcb07 */,
"RRC B" /* 0xcb08 */,
"RRC C" /* 0xcb09 */,
"RRC D" /* 0xcb0a */,
"RRC E" /* 0xcb0b */,
"RRC H" /* 0xcb0c */,
"RRC L" /* 0xcb0d */,
"RRC (HL)" /* 0xcb0e */,
"RRC A" /* 0xcb0f */,
"RL B" /* 0xcb10 */,
"RL C" /* 0xcb11 */,
"RL D" /* 0xcb12 */,
"RL E" /* 0xcb13 */,
"RL H" /* 0xcb14 */,
"RL L" /* 0xcb15 */,
"RL (HL)" /* 0xcb16 */,
"RL A" /* 0xcb17 */,
"RR B" /* 0xcb18 */,
"RR C" /* 0xcb19 */,
"RR D" /* 0xcb1a */,
"RR E" /* 0xcb1b */,
"RR H" /* 0xcb1c */,
"RR L" /* 0xcb1d */,
"RR (HL)" /* 0xcb1e */,
"RR A" /* 0xcb1f */,
"SLA B" /* 0xcb20 */,
"SLA C" /* 0xcb21 */,
"SLA D" /* 0xcb22 */,
"SLA E" /* 0xcb23 */,
"SLA H" /* 0xcb24 */,
"SLA L" /* 0xcb25 */,
"SLA (HL)" /* 0xcb26 */,
"SLA A" /* 0xcb27 */,
"SRA B" /* 0xcb28 */,
"SRA C" /* 0xcb29 */,
"SRA D" /* 0xcb2a */,
"SRA E" /* 0xcb2b */,
"SRA H" /* 0xcb2c */,
"SRA L" /* 0xcb2d */,
"SRA (HL)" /* 0xcb2e */,
"SRA A" /* 0xcb2f */,
"SWAP B" /* 0xcb30 */,
"SWAP C" /* 0xcb31 */,
"SWAP D" /* 0xcb32 */,
"SWAP E" /* 0xcb33 */,
"SWAP H" /* 0xcb34 */,
"SWAP L" /* 0xcb35 */,
"SWAP (HL)" /* 0xcb36 */,
"SWAP A" /* 0xcb37 */,
"SRL B" /* 0xcb38 */,
"SRL C" /* 0xcb39 */,
"SRL D" /* 0xcb3a */,
"SRL E" /* 0xcb3b */,
"SRL H" /* 0xcb3c */,
"SRL L" /* 0xcb3d */,
"SRL (HL)" /* 0xcb3e */,
"SRL A" /* 0xcb3f */,
"BIT 0, B" /* 0xcb40 */,
"BIT 0, C" /* 0xcb41 */,
"BIT 0, D" /* 0xcb42 */,
"BIT 0, E" /* 0xcb43 */,
"BIT 0, H" /* 0xcb44 */,
"BIT 0, L" /* 0xcb45 */,
"BIT 0, (HL)" /* 0xcb46 */,
"BIT 0, A" /* 0xcb47 */,
"BIT 1, B" /* 0xcb48 */,
"BIT 1, C" /* 0xcb49 */,
"BIT 1, D" /* 0xcb4a */,
"BIT 1, E" /* 0xcb4b */,
"BIT 1, H" /* 0xcb4c */,
"BIT 1, L" /* 0xcb4d */,
"BIT 1, (HL)" /* 0xcb4e */,
"BIT 1, A" /* 0xcb4f */,
"BIT 2, B" /* 0xcb50 */,
"BIT 2, C" /* 0xcb51 */,
"BIT 2, D" /* 0xcb52 */,
"BIT 2, E" /* 0xcb53 */,
"BIT 2, H" /* 0xcb54 */,
"BIT 2, L" /* 0xcb55 */,
"BIT 2, (HL)" /* 0xcb56 */,
"BIT 2, A" /* 0xcb57 */,
"BIT 3, B" /* 0xcb58 */,
"BIT 3, C" /* 0xcb59 */,
"BIT 3, D" /* 0xcb5a */,
"BIT 3, E" /* 0xcb5b */,
"BIT 3, H" /* 0xcb5c */,
"BIT 3, L" /* 0xcb5d */,
"BIT 3, (HL)" /* 0xcb5e */,
"BIT 3, A" /* 0xcb5f */,
"BIT 4, B" /* 0xcb60 */,
"BIT 4, C" /* 0xcb61 */,
"BIT 4, D" /* 0xcb62 */,
"BIT 4, E" /* 0xcb63 */,
"BIT 4, H" /* 0xcb64 */,
"BIT 4, L" /* 0xcb65 */,
"BIT 4, (HL)" /* 0xcb66 */,
"BIT 4, A" /* 0xcb67 */,
"BIT 5, B" /* 0xcb68 */,
"BIT 5, C" /* 0xcb69 */,
"BIT 5, D" /* 0xcb6a */,
"BIT 5, E" /* 0xcb6b */,
"BIT 5, H" /* 0xcb6c */,
"BIT 5, L" /* 0xcb6d */,
"BIT 5, (HL)" /* 0xcb6e */,
"BIT 5, A" /* 0xcb6f */,
"BIT 6, B" /* 0xcb70 */,
"BIT 6, C" /* 0xcb71 */,
"BIT 6, D" /* 0xcb72 */,
"BIT 6, E" /* 0xcb73 */,
"BIT 6, H" /* 0xcb74 */,
"BIT 6, L" /* 0xcb75 */,
"BIT 6, (HL)" /* 0xcb76 */,
"BIT 6, A" /* 0xcb77 */,
"BIT 7, B" /* 0xcb78 */,
"BIT 7, C" /* 0xcb79 */,
"BIT 7, D" /* 0xcb7a */,
"BIT 7, E" /* 0xcb7b */,
"BIT 7, H" /* 0xcb7c */,
"BIT 7, L" /* 0xcb7d */,
"BIT 7, (HL)" /* 0xcb7e */,
"BIT 7, A" /* 0xcb7f */,
"RES 0, B" /* 0xcb80 */,
"RES 0, C" /* 0xcb81 */,
"RES 0, D" /* 0xcb82 */,
"RES 0, E" /* 0xcb83 */,
"RES 0, H" /* 0xcb84 */,
"RES 0, L" /* 0xcb85 */,
"RES 0, (HL)" /* 0xcb86 */,
"RES 0, A" /* 0xcb87 */,
"RES 1, B" /* 0xcb88 */,
"RES 1, C" /* 0xcb89 */,
"RES 1, D" /* 0xcb8a */,
"RES 1, E" /* 0xcb8b */,
"RES 1, H" /* 0xcb8c */,
"RES 1, L" /* 0xcb8d */,
"RES 1, (HL)" /* 0xcb8e */,
"RES 1, A" /* 0xcb8f */,
"RES 2, B" /* 0xcb90 */,
"RES 2, C" /* 0xcb91 */,
"RES 2, D" /* 0xcb92 */,
"RES 2, E" /* 0xcb93 */,
"RES 2, H" /* 0xcb94 */,
"RES 2, L" /* 0xcb95 */,
"RES 2, (HL)" /* 0xcb96 */,
"RES 2, A" /* 0xcb97 */,
"RES 3, B" /* 0xcb98 */,
"RES 3, C" /* 0xcb99 */,
"RES 3, D" /* 0xcb9a */,
"RES 3, E" /* 0xcb9b */,
"RES 3, H" /* 0xcb9c */,
"RES 3, L" /* 0xcb9d */,
"RES 3, (HL)" /* 0xcb9e */,
"RES 3, A" /* 0xcb9f */,
"RES 4, B" /* 0xcba0 */,
"RES 4, C" /* 0xcba1 */,
"RES 4, D" /* 0xcba2 */,
"RES 4, E" /* 0xcba3 */,
"RES 4, H" /* 0xcba4 */,
"RES 4, L" /* 0xcba5 */,
"RES 4, (HL)" /* 0xcba6 */,
"RES 4, A" /* 0xcba7 */,
"RES 5, B" /* 0xcba8 */,
"RES 5, C" /* 0xcba9 */,
"RES 5, D" /* 0xcbaa */,
"RES 5, E" /* 0xcbab */,
"RES 5, H" /* 0xcbac */,
"RES 5, L" /* 0xcbad */,
"RES 5, (HL)" /* 0xcbae */,
"RES 5, A" /* 0xcbaf */,
"RES 6, B" /* 0xcbb0 */,
"RES 6, C" /* 0xcbb1 */,
"RES 6, D" /* 0xcbb2 */,
"RES 6, E" /* 0xcbb3 */,
"RES 6, H" /* 0xcbb4 */,
"RES 6, L" /* 0xcbb5 */,
"RES 6, (HL)" /* 0xcbb6 */,
"RES 6, A" /* 0xcbb7 */,
"RES 7, B" /* 0xcbb8 */,
"RES 7, C" /* 0xcbb9 */,
"RES 7, D" /* 0xcbba */,
"RES 7, E" /* 0xcbbb */,
"RES 7, H" /* 0xcbbc */,
"RES 7, L" /* 0xcbbd */,
"RES 7, (HL)" /* 0xcbbe */,
"RES 7, A" /* 0xcbbf */,
"SET 0, B" /* 0xcbc0 */,
"SET 0, C" /* 0xcbc1 */,
"SET 0, D" /* 0xcbc2 */,
"SET 0, E" /* 0xcbc3 */,
"SET 0, H" /* 0xcbc4 */,
"SET 0, L" /* 0xcbc5 */,
"SET 0, (HL)" /* 0xcbc6 */,
"SET 0, A" /* 0xcbc7 */,
"SET 1, B" /* 0xcbc8 */,
"SET 1, C" /* 0xcbc9 */,
"SET 1, D" /* 0xcbca */,
"SET 1, E" /* 0xcbcb */,
"SET 1, H" /* 0xcbcc */,
"SET 1, L" /* 0xcbcd */,
"SET 1, (HL)" /* 0xcbce */,
"SET 1, A" /* 0xcbcf */,
"SET 2, B" /* 0xcbd0 */,
"SET 2, C" /* 0xcbd1 */,
"SET 2, D" /* 0xcbd2 */,
"SET 2, E" /* 0xcbd3 */,
"SET 2, H" /* 0xcbd4 */,
"SET 2, L" /* 0xcbd5 */,
"SET 2, (HL)" /* 0xcbd6 */,
"SET 2, A" /* 0xcbd7 */,
"SET 3, B" /* 0xcbd8 */,
"SET 3, C" /* 0xcbd9 */,
"SET 3, D" /* 0xcbda */,
"SET 3, E" /* 0xcbdb */,
"SET 3, H" /* 0xcbdc */,
"SET 3, L" /* 0xcbdd */,
"SET 3, (HL)" /* 0xcbde */,
"SET 3, A" /* 0xcbdf */,
"SET 4, B" /* 0xcbe0 */,
"SET 4, C" /* 0xcbe1 */,
"SET 4, D" /* 0xcbe2 */,
"SET 4, E" /* 0xcbe3 */,
"SET 4, H" /* 0xcbe4 */,
"SET 4, L" /* 0xcbe5 */,
"SET 4, (HL)" /* 0xcbe6 */,
"SET 4, A" /* 0xcbe7 */,
"SET 5, B" /* 0xcbe8 */,
"SET 5, C" /* 0xcbe9 */,
"SET 5, D" /* 0xcbea */,
"SET 5, E" /* 0xcbeb */,
"SET 5, H" /* 0xcbec */,
"SET 5, L" /* 0xcbed */,
"SET 5, (HL)" /* 0xcbee */,
"SET 5, A" /* 0xcbef */,
"SET 6, B" /* 0xcbf0 */,
"SET 6, C" /* 0xcbf1 */,
"SET 6, D" /* 0xcbf2 */,
"SET 6, E" /* 0xcbf3 */,
"SET 6, H" /* 0xcbf4 */,
"SET 6, L" /* 0xcbf5 */,
"SET 6, (HL)" /* 0xcbf6 */,
"SET 6, A" /* 0xcbf7 */,
"SET 7, B" /* 0xcbf8 */,
"SET 7, C" /* 0xcbf9 */,
"SET 7, D" /* 0xcbfa */,
"SET 7, E" /* 0xcbfb */,
"SET 7, H" /* 0xcbfc */,
"SET 7, L" /* 0xcbfd */,
"SET 7, (HL)" /* 0xcbfe */,
"SET 7, A" /* 0xcbff */,
};
