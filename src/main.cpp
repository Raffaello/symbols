#include "REPL.hpp"

#include <iostream>
#include <format>

// #include <boost/multiprecision/mpfr.hpp>
// using boost::multiprecision::mpfr_float;

int main()
{
    // mpfr_float::default_precision(210);

    // mpfr_float x = 2;
    // mpfr_float y = boost::multiprecision::sqrt(mpfr_float(2));

    // std::cout << y.str(200, std::ios::fixed) << "\n\n";
    // std::cout << std::setprecision(200) << y << "\n\n";


    REPL repl;

    return repl.runLoop();
}
