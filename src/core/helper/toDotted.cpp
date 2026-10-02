#include "helpers.h"

std::string toDotted(const std::string &s) {
  std::string out = s;
  for (auto &c : out)
    if (c == '/')
      c = '.';
  return out;
}