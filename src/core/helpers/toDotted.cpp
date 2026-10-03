#include "helpers.h"

// replaces (/)s with (.)s
std::string toDotted(const std::string &s) {
  std::string out = s;
  for (auto &c : out)
    if (c == '/')
      c = '.';
  return out;
}