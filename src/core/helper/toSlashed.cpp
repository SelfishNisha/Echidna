#include "helpers.h"

std::string toSlashed(const std::string &s) {
  std::string out = s;
  for (auto &c : out)
    if (c == '.')
      c = '/';
  return out;
}