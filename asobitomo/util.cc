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
