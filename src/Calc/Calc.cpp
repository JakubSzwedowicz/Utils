#include <iostream>

#include "Calc.hpp"


int Calc::Calc::add(int a, int b) {
    std::cout << "Calling Calc::add" << std::endl;
    return a + b;
}