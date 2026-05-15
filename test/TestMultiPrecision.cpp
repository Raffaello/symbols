#include <gtest/gtest.h>

#include <multi_precision.hpp>
#include "assertion_mp.hpp"

#include <array>

class TestMultiPrecision : public ::testing::TestWithParam<std::tuple<mp_num_t, mp_num_t, mp_num_t, mp_num_t, mp_num_t, mp_num_t>>
{
public:
    const mp_num_t a      = std::get<0>(GetParam());
    const mp_num_t b      = std::get<1>(GetParam());
    const mp_num_t expAdd = std::get<2>(GetParam());
    const mp_num_t expSub = std::get<3>(GetParam());
    const mp_num_t expMul = std::get<4>(GetParam());
    const mp_num_t expDiv = std::get<5>(GetParam());
};

TEST_P(TestMultiPrecision, add_sub_mul_div)
{
    auto add = a + b;
    auto sub = a - b;
    auto mul = a * b;
    auto div = a / b;


    EXPECT_INT_NUM_T_NEAR(expAdd, add, MPFR_EPSILON);
    EXPECT_INT_NUM_T_NEAR(expSub, sub, MPFR_EPSILON);
    EXPECT_INT_NUM_T_NEAR(expMul, mul, MPFR_EPSILON);
    EXPECT_INT_NUM_T_NEAR(expDiv, div, MPFR_EPSILON);
};

INSTANTIATE_TEST_SUITE_P(
    MultiPrecisionTestSuite,
    TestMultiPrecision,
    ::testing::Values(
        std::make_tuple(mp::mpq_rational{1}, mp::mpq_rational{2}, mp::mpq_rational{3}, mp::mpq_rational{-1}, mp::mpq_rational{2}, mp::mpq_rational{"1/2"}),
        std::make_tuple(mp::mpfr_float{1}, mp::mpq_rational{2}, mp::mpq_rational{3}, mp::mpq_rational{-1}, mp::mpq_rational{2}, mp::mpq_rational{"1/2"}),
        std::make_tuple(mp::mpq_rational{1}, mp::mpfr_float{2}, mp::mpq_rational{3}, mp::mpq_rational{-1}, mp::mpq_rational{2}, mp::mpq_rational{"1/2"}),
        std::make_tuple(mp::mpq_rational{1}, mp::mpfr_float{1.5}, mp::mpfr_float{2.5}, mp::mpfr_float{-0.5}, mp::mpfr_float{1.5}, mp::mpq_rational{"2/3"}),
        std::make_tuple(mp::mpfr_float{1}, mp::mpq_rational{"3/2"}, mp::mpq_rational{"5/2"}, mp::mpq_rational{"-1/2"}, mp::mpq_rational{"3/2"}, mp::mpq_rational{"2/3"}),
        std::make_tuple(mp::mpq_rational{3}, mp::mpq_rational{"3"}, mp::mpq_rational{"6"}, mp::mpq_rational{"0"}, mp::mpq_rational{"9"}, mp::mpq_rational{"1"}),
        std::make_tuple(mp::mpq_rational{9}, mp::mpq_rational{"3"}, mp::mpq_rational{"12"}, mp::mpq_rational{"6"}, mp::mpq_rational{"27"}, mp::mpq_rational{"3"})

            ));

TEST(TestMultiPrecision, logic)
{
    const mp_num_t a  = mp::mpfr_float{10.3};
    const mp_num_t b  = mp::mpq_rational{10};
    const mp_num_t b2 = mp::mpfr_float{10.0};

    EXPECT_TRUE(b < a);
    EXPECT_TRUE(b <= a);
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(b == b2);
    EXPECT_TRUE(-b == -b2);
    EXPECT_TRUE(b2 == b);
    EXPECT_TRUE(-b2 == -b);
    EXPECT_TRUE(a > b);
    EXPECT_TRUE(a >= b);

    EXPECT_TRUE(to_mpfr_float(b) == b2);

    mp_num_t w = mp::mpq_rational{"10000 / 9999"};
    EXPECT_FALSE(mp_isWeird(w));

    w = mp::mpq_rational("123456/654321");
    EXPECT_TRUE(mp_isWeird(w));

    w = mp::mpq_rational{"1000000000000000 / 999999999999999"};
    EXPECT_TRUE(mp_isWeird(w));

    mp::mpz_int zi = mp_extract_mpz_int(w);
    EXPECT_TRUE(zi == 1);

    EXPECT_FALSE(w == 1);
    EXPECT_TRUE(mp_roundNear(w) == 1);

    EXPECT_TRUE(mp_clamp(a, mp_num_t{mp::mpq_rational{10}}, mp_num_t{mp::mpq_rational{10}}) == 10);
    EXPECT_TRUE(mp_clamp(a, mp_num_t{mp::mpq_rational{10.5}}, mp_num_t{mp::mpq_rational{11}}) == 10.5);
    EXPECT_TRUE(mp_clamp(a, mp_num_t{mp::mpq_rational{10}}, mp_num_t{mp::mpq_rational{11}}) == 10.3);

    EXPECT_TRUE(mp_abs(-a) == a);
    EXPECT_TRUE(mp_abs(-b) == b);
    EXPECT_TRUE(mp_abs(a) == a);
    EXPECT_TRUE(mp_abs(b) == b);

    EXPECT_FALSE(mp_isZero(a));
    EXPECT_FALSE(mp_isZero(b));
    EXPECT_TRUE(mp_isZero(mp_num_t{mp::mpq_rational{0}}));
    EXPECT_TRUE(mp_isZero(mp_num_t{mp::mpfr_float{0}}));
};

TEST(TestMultiPrecision, pow_sqrt_cbrt)
{
    const mp_num_t a  = mp::mpfr_float{2};
    const mp_num_t b  = mp::mpq_rational{-2};
    const mp_num_t c1 = mp::mpq_rational{25};
    const mp_num_t c2 = mp::mpfr_float{25};
    const mp_num_t d  = mp::mpq_rational(64);

    EXPECT_TRUE(-a == b);
    EXPECT_TRUE(a == -b);
    EXPECT_TRUE(-b == a);
    EXPECT_TRUE(b == -a);
    EXPECT_TRUE(c1 == c2);
    EXPECT_TRUE(c2 == c1);

    mp_num_t pa  = mp_pow(a, 2);
    mp_num_t pb  = mp_pow(b, 2);
    mp_num_t pc1 = mp_pow(c1, 2);
    mp_num_t pc2 = mp_pow(c1, 2);

    EXPECT_TRUE(pa == pb);
    EXPECT_TRUE(pc1 == pc2);

    mp_num_t sa  = mp_sqrt(a);
    mp_num_t sb  = mp_sqrt(-b);
    mp_num_t sc1 = mp_sqrt(c1);
    mp_num_t sc2 = mp_sqrt(c2);

    EXPECT_TRUE(sa == sb);
    EXPECT_TRUE(sc1 == sc2);
    EXPECT_TRUE(sc1 == 5);

    mp_num_t ca  = mp_cbrt(a);
    mp_num_t cb  = mp_cbrt(-b);
    mp_num_t cc1 = mp_cbrt(c1);
    mp_num_t cc2 = mp_cbrt(c2);
    mp_num_t cd  = mp_cbrt(d);

    EXPECT_TRUE(ca == cb);
    EXPECT_TRUE(cc1 == cc2);
    EXPECT_TRUE(cd == 4);
};

TEST(TestMultiPrecision, solve_factor_p)
{
    mp_num_t a = mp::mpq_rational(3);
    mp_num_t b = mp::mpq_rational{3};

    mp_num_t c = b - (a * a / 3);
    EXPECT_TRUE(c == 0);
}

TEST(TestMultiprecision, formatters)
{
    mp_num_t a = mp::mpq_rational{"1/3"};
    EXPECT_STRCASEEQ("1/3", std::format("{}", a).c_str());

    a = mp_sqrt(mp::mpq_rational{2});
    EXPECT_STRCASEEQ("1.41421356237309505", std::format("{}", a).c_str());
    EXPECT_STRCASEEQ("1.414", std::format("{:.3}", a).c_str());
    EXPECT_STRCASEEQ("1", std::format("{:.0}", a).c_str());

    a = mp::mpq_rational{"1000000/999999"};
    EXPECT_STRCASEEQ("1.000001000001", std::format("{}", a).c_str());
    EXPECT_STRCASEEQ("1.000", std::format("{:.3}", a).c_str());
}

TEST(TestMultiprecision, error)
{
    mp_num_t q = mp::mpq_rational(-1);

    ASSERT_THROW(mp_sqrt(q), std::runtime_error);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
