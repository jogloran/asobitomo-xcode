#include "util.h"
#include "rang.hpp"

#include <sstream>
#include <ios>

std::string binary(byte b) {
  std::stringstream s;
  
  for (int i = 7; i >= 0; --i) {
    if ((b >> i) & 0x1) {
      s << rang::style::bold << rang::fgB::gray << '1' << rang::fg::reset << rang::style::reset;
    } else {
      s << rang::style::dim << '0' << rang::style::reset;
    }
  }
  
  auto result = s.str();
  return result;
}

static std::string interrupt_names[] {
  "vblank",
  "stat",
  "timer",
  "serial",
  "joypad",
};

std::string interrupt_flags_to_description(byte flags) {
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

two_byte_fmt_manip two_byte_fmt(byte b1, byte b2) {
  return two_byte_fmt_manip(b1, b2);
}

std::ostream& operator<<(std::ostream& out, const two_byte_fmt_manip& manip) {
  manip(out);
  return out;
}

size_t history_repeating(std::deque<word> history) {
  for (auto i = 20; i >= 2; --i) {
    if (history.size() < 2*i) continue;
    
    if (std::equal(history.end() - i, history.end(), history.end() - 2*i)) {
      return i;
    }
  }
  
  return 0;
}
