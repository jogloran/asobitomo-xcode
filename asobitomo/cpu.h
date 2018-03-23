#pragma once

#include "types.h"
#include "mmu.h"
#include "ppu.h"
#include <array>
#include <iomanip>
#include "flags.h"

extern bool ASOBITOMO_DEBUG;

constexpr int NINSTR = 256;

constexpr long ncycles_cb[NINSTR] = {
// x0 x1 x2 x3 x4 x5  x6 x7 x8 x9 xA xB xC xD  xE xF
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 0x
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 1x
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 2x
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 3x
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 4x
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 5x
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 6x
    8, 8, 8, 8, 8, 8, 12, 8, 8, 8, 8, 8, 8, 8, 12, 8, // 7x
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 8x
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // 9x
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // Ax
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // Bx
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // Cx
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // Dx
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // Ex
    8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8, // Fx
};

constexpr long ncycles[NINSTR] = {
// C9 (RET) is treated as 4 cycles so that we can reuse the conditional
// RET logic (which adds 12 for a true branch)
// same with C3 (JP a16), CD (CALL a16, treat as 12 cycles), 18 (JR r8)
// x0   x1 x2  x3  x4  x5  x6  x7  x8  x9  xA  xB  xC  xD  xE  xF
    4,  12, 8,  8,  4,  4,  8,  4,  20, 8,  8,  8,  4,  4,  8,  4, // 0x
    4,  12, 8,  8,  4,  4,  8,  4,  8,  8,  8,  8,  4,  4,  8,  4, // 1x
    8,  12, 8,  8,  4,  4,  8,  4,  8,  8,  8,  8,  4,  4,  8,  4, // 2x
    8,  12, 8,  8,  12, 12, 12, 4,  8,  8,  8,  8,  4,  4,  8,  4, // 3x
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, // 4x
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, // 5x
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, // 6x
    8,  8,  8,  8,  8,  8,  4,  8,  4,  4,  4,  4,  4,  4,  8,  4, // 7x
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, // 8x
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, // 9x
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, // Ax
    4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4, // Bx
    8,  12, 12, 12, 12, 16, 8,  16, 8,  4, 12,  0,  12, 12, 8,  16,// Cx
    8,  12, 12, 0,  12, 16, 8,  16, 8, 16, 12,  0,  12, 0,  8,  16,// Dx
    12, 12, 8,  0,  0,  16, 8,  16, 16, 4,  16, 0,  0,  0,  8,  16,// Ex
    12, 12, 8,  4,  0,  16, 8,  16, 12, 8,  16, 4,  0,  0,  8,  16,// Fx
};

typedef void (*op)(CPU&);

class CPU {
public:
  CPU(std::string path): a(0), f(0), b(0), c(0), d(0), e(0), h(0), l(0),
    pc(0x0000), sp(0x0000), cycles(0), timer(*this),
    ppu(*this), mmu(path, ppu, timer),
    halted(false), in_halt_bug(false), interrupt_flags_before_halt(0),
    interrupt_enabled(InterruptState::Disabled), in_cb(false) {
  }

  enum class InterruptState {
    Disabled,
    DisableNext,
    EnableNext,
    Enabled,
  };

  bool Z() { return (f & Zf) == Zf; }
  bool N() { return (f & Nf) == Nf; }
  bool H() { return (f & Hf) == Hf; }
  bool C() { return (f & Cf) == Cf; }

  void unset_flags(int flags) {
    f &= ~flags;
  }

  void set_flags(int flags) {
    f |= flags;
  }

  void toggle_flags(int flags) {
    f ^= flags;
  }

  word get_word() {
    return (mmu[pc + 1] << 8) | mmu[pc];
  }

  word get_word(byte hi, byte lo) {
    return (hi << 8) | lo;
  }
  
  bool wake_if_interrupt_requested();
  
  void halt();
  void stop();

  std::string op_name_for(word loc);
  void dump_registers_to_file(std::ofstream& out);
  void dump_state();

  void step(bool debug = false);
  void inst();
  
  void fake_boot();

  void update_interrupt_state();
  void fire_interrupts();

  void enable_interrupts() {
    interrupt_enabled = InterruptState::Enabled;
  }
  void enable_interrupts_next_instruction() {
    interrupt_enabled = InterruptState::EnableNext;
  }
  void disable_interrupts_next_instruction() {
    interrupt_enabled = InterruptState::DisableNext;
  }

  void initiate_dma(word src);

  byte a, f, b, c, d, e, h, l;
  word pc, sp;
  long cycles;

  Timer timer;
  PPU ppu;
  MMU mmu;
  
  bool halted;
  bool in_halt_bug;
  byte interrupt_flags_before_halt;

  InterruptState interrupt_enabled;
  std::string ppu_state_as_string(PPU::Mode mode);
  std::string interrupt_state_as_string(InterruptState state);
  
  bool in_cb;

  static constexpr size_t NINSTR = 256;

  void _handle_cb() {
    in_cb = true;
    
    byte instr = mmu[pc];
    ++pc;
    cb_ops[instr](*this);
    cycles += ncycles_cb[instr];
  }

  static void handle_cb(CPU& cpu) {
    cpu._handle_cb();
  }
  
  void check_zero(byte value) {
    if (value == 0x0) {
      set_flags(Zf);
    } else {
      unset_flags(Zf);
    }
  }
  
  void check_half_carry(byte reg, byte addend, byte carry=0x0) {
    if (((reg & 0xf) + (addend & 0xf) + carry) > 0xf) {
      set_flags(Hf);
    } else {
      unset_flags(Hf);
    }
  }
  
  void check_half_carry_sub(byte reg, byte operand, byte carry=0x0) {
    if ((reg & 0xf) < ((operand & 0xf) + carry)) {
      set_flags(Hf);
    } else {
      unset_flags(Hf);
    }
  }

  static std::array<op, NINSTR> cb_ops;
  static std::array<op, NINSTR> ops;
};
