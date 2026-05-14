#pragma once


#include <gtest/gtest.h>
#include <boost/multiprecision/mpfr.hpp>
#include <format>

#include "formatters.hpp"

namespace mp = boost::multiprecision;

::testing::AssertionResult MPFRNear(const mp::mpfr_float& a, const mp::mpfr_float& b, const mp::mpfr_float& eps)
{
    if (mp::abs(a - b) <= eps)
        return ::testing::AssertionSuccess();

    return ::testing::AssertionFailure()
        << std::format("Expected: {}\n", a)
        << std::format("Actual  : {}\n", b)
        << std::format("Epsilon : {}\n", eps);
}

#define EXPECT_MPFR_NEAR(val1, val2, tol) EXPECT_TRUE(MPFRNear((val1), (val2), (tol)))
