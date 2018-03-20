#include "mbc_types.h"

std::ostream& operator<<(std::ostream& out, MBC mbc) {
  if (mbc_string.find(mbc) != mbc_string.end()) {
    out << mbc_string[mbc];
  } else {
    out << "(unknown)";
  }
  
  return out;
}
