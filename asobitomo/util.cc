#include "util.h"

#include <sstream>

std::string binary(byte b) {
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
