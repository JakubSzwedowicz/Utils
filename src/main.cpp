#include "Calc/Calc.hpp"
#include <fmt/core.h>
#include <iostream>

int main() {
  fmt::print("Hello World!\n");
  std::cout << Calc::Calc::templateAdd(10.0f, 5.5f) << std::endl;
  return 0;
}