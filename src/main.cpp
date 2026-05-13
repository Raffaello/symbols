#include "REPL.hpp"

#include <iostream>
#include <format>

// #include <boost/multiprecision/mpfr.hpp>
// #include <boost/multiprecision/gmp.hpp>
// #include <mpfr.h>
// #include <gmp.h>

// namespace mp = boost::multiprecision;

int main()
{
    // mp::mpfr_float::default_precision(210);

    // mp::mpfr_float x = 2;
    // mp::mpfr_float y = 10.5;    // boost::multiprecision::sqrt(x);

    // mp::mpq_rational q(1, 3);

    // mpq_t zq;
    // mpq_init(zq);
    // mpfr_get_q(zq, y.backend().data());

    // mpq_set(q.backend().data(), zq);


    // std::cout << y.str(200, std::ios::fixed) << "\n\n";
    // std::cout << std::setprecision(200) << y << "\n\n";
    // std::cout << mp::numerator(q) << "/" << mp::denominator(q) << "\n\n";


    REPL repl;

    return repl.runLoop();
}
