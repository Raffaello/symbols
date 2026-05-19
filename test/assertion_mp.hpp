#pragma once

#include <gtest/gtest.h>
#include <boost/multiprecision/mpfr.hpp>
#include <format>

#include "formatters.hpp"
#include "mp_t.hpp"

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

::testing::AssertionResult MPQEq(const mp::mpq_rational& a, const mp::mpq_rational& b)
{
    if (a == b)
        return ::testing::AssertionSuccess();

    return ::testing::AssertionFailure()
        << std::format("Expected: {}\n", a)
        << std::format("Actual  : {}\n", b);
}

#define EXPECT_MPQ_EQ(val1, val2) EXPECT_TRUE(MPQEq((val1), (val2)))

::testing::AssertionResult mp_t_Near(const mp_t& a, const mp_t& b, const mp_t& eps)
{
    if (mp_t::abs(a - b) <= eps)
        return ::testing::AssertionSuccess();

    return ::testing::AssertionFailure()
        << std::format("Expected: {}\n", a)
        << std::format("Actual  : {}\n", b)
        << std::format("Epsilon : {}\n", eps);
}

#define EXPECT_MP_T_NEAR(val1, val2, tol) EXPECT_TRUE(mp_t_Near((val1), (val2), (tol)))
