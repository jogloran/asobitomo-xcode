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
  8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
  8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
  8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
  8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
  8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
  8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
  8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
  8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
  8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
  8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
  8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
  8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
  8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
  8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
  8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
  8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 16, 8,
};

constexpr long ncycles[NINSTR] = {
  4,  12, 8,  8,  4,  4,  8,  4,  20, 8,  8,  8,  4,  4,  8,  4,
  4,  12, 8,  8,  4,  4,  8,  4,  12, 8,  8,  8,  4,  4,  8,  4,
  12, 12, 8,  8,  4,  4,  8,  4,  12, 8,  8,  8,  4,  4,  8,  4,
  12, 12, 8,  8,  12, 12, 12, 4,  12, 8,  8,  8,  4,  4,  8,  4,
  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
  8,  8,  8,  8,  8,  8,  4,  8,  4,  4,  4,  4,  4,  4,  8,  4,
  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
  4,  4,  4,  4,  4,  4,  8,  4,  4,  4,  4,  4,  4,  4,  8,  4,
  20, 12, 16, 16, 24, 16, 8,  16, 20, 16, 16, 4,  24, 24, 8,  16,
  20, 12, 16, 0,  24, 16, 8,  16, 20, 16, 16, 0,  24, 0,  8,  16,
  12, 12, 8,  0,  0,  16, 8,  16, 16, 4,  16, 0,  0,  0,  8,  16,
  12, 12, 8,  4,  0,  16, 8,  16, 12, 8,  16, 4,  0,  0,  8,  16,
};

typedef void (*op)(CPU&);

class CPU {
public:
  CPU(std::string path): a(0), f(0), b(0), c(0), d(0), e(0), h(0), l(0),
    pc(0x0000), sp(0x0000), cycles(0), timer(*this),
    ppu(*this), mmu(path, ppu, timer),  halted(false),
    interrupt_enabled(InterruptState::Disabled) {}

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

  void halt() {
    halted = true;
  }

  void stop() {
    halted = true;
  }

  std::string op_name_for(word loc);
  void dump_state();

  void step(bool debug = false);
  void inst();

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

  InterruptState interrupt_enabled;
  std::string ppu_state_as_string(PPU::Mode mode);
  std::string interrupt_state_as_string(InterruptState state);

  static constexpr size_t NINSTR = 256;

  void _handle_cb() {
    byte instr = mmu[pc];
    ++pc;
    cb_ops[instr](*this);
    cycles += ncycles_cb[instr];
  }

  static void handle_cb(CPU& cpu) {
    cpu._handle_cb();
  }

  static std::array<op, 256> cb_ops;
  static std::array<op, 256> ops;
};
