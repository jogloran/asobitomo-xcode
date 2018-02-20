#pragma once

#include "types.h"

class CPU;

class PPU {
public:
  PPU(CPU& cpu): line(0), mode(Mode::OAM), ncycles(0), vblank_ncycles(0), cpu(cpu) {}

  enum class Mode : byte {
    HBLANK = 0,
    VBLANK = 1,
    OAM = 2,
    VRAM = 3,
  };

  void step(long delta) {
    ncycles += delta;

/* 456*144 + 4560 = 70224
 *                                    \
 * OAM         VRAM          hblank   |
 * ------- -------------- ----------  > 144 times per frame
 * 80           172           204     |
 * ---------------------------------  |
 *              456                   /
 *
 *                                    \
 *            vblank                  |
 * -----------------------------------> 10 times per frame
 *              456                   |
 *                                    /
 */
    switch (mode) {
      case Mode::OAM:
        if (ncycles >= 80) {
          mode = Mode::VRAM;
          ncycles -= 80;
        }

        update_stat_register();
        break;
      case Mode::VRAM:
        if (ncycles >= 172) {
          mode = Mode::HBLANK;
          ncycles -= 172;
          // write one row to framebuffer
        }

        update_stat_register();

        break;
      case Mode::HBLANK:
        if (ncycles >= 204) {
          ++line;
          ncycles -= 204;

          if (line == 144) {
            mode = Mode::VBLANK;
            // last hblank: blit buffer
          } else {
            mode = Mode::OAM;
          }
          update_stat_register();
        }
        break;
      case Mode::VBLANK:
        // vblank happens every 4560 cycles

        // One vblank line
        if (ncycles >= 456) {
          ncycles -= 456;
          ++line;

          if (line > 153) {
            mode = Mode::OAM;
            line = 0;
          }

          update_stat_register();
        }

        break;
    }
  }
private:

  void update_stat_register();

  byte line;
  Mode mode;
  long ncycles;
  long vblank_ncycles;

  CPU& cpu;
};
