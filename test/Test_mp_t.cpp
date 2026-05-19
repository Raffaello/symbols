#include <gtest/gtest.h>

#include <mp_t.hpp>

#include "assertion_mp.hpp"

class Test_mp_t : public ::testing::TestWithParam<std::tuple<mp_t, mp_t, mp_t, mp_t, mp_t, mp_t>>
{
public:
    const mp_t a      = std::get<0>(GetParam());
    const mp_t b      = std::get<1>(GetParam());
    const mp_t expAdd = std::get<2>(GetParam());
    const mp_t expSub = std::get<3>(GetParam());
    const mp_t expMul = std::get<4>(GetParam());
    const mp_t expDiv = std::get<5>(GetParam());
};

TEST_P(Test_mp_t, add_sub_mul_div)
{
    auto add = a + b;
    auto sub = a - b;
    auto mul = a * b;
    auto div = a / b;


    EXPECT_MP_T_NEAR(expAdd, add, MPFR_EPSILON);
    EXPECT_MP_T_NEAR(expSub, sub, MPFR_EPSILON);
    EXPECT_MP_T_NEAR(expMul, mul, MPFR_EPSILON);
    EXPECT_MP_T_NEAR(expDiv, div, MPFR_EPSILON);
};

INSTANTIATE_TEST_SUITE_P(
    mp_t_TestSuite,
    Test_mp_t,
    ::testing::Values(
        std::make_tuple(mp::mpq_rational{1}, mp::mpq_rational{2}, mp::mpq_rational{3}, mp::mpq_rational{-1}, mp::mpq_rational{2}, mp::mpq_rational{"1/2"}),
        std::make_tuple(mp::mpfr_float{1}, mp::mpq_rational{2}, mp::mpq_rational{3}, mp::mpq_rational{-1}, mp::mpq_rational{2}, mp::mpq_rational{"1/2"}),
        std::make_tuple(mp::mpq_rational{1}, mp::mpfr_float{2}, mp::mpq_rational{3}, mp::mpq_rational{-1}, mp::mpq_rational{2}, mp::mpq_rational{"1/2"}),
        std::make_tuple(mp::mpq_rational{1}, mp::mpfr_float{1.5}, mp::mpfr_float{2.5}, mp::mpfr_float{-0.5}, mp::mpfr_float{1.5}, mp::mpq_rational{"2/3"}),
        std::make_tuple(mp::mpfr_float{1}, mp::mpq_rational{"3/2"}, mp::mpq_rational{"5/2"}, mp::mpq_rational{"-1/2"}, mp::mpq_rational{"3/2"}, mp::mpq_rational{"2/3"}),
        std::make_tuple(mp::mpq_rational{3}, mp::mpq_rational{"3"}, mp::mpq_rational{"6"}, mp::mpq_rational{"0"}, mp::mpq_rational{"9"}, mp::mpq_rational{"1"}),
        std::make_tuple(mp::mpq_rational{9}, mp::mpq_rational{"3"}, mp::mpq_rational{"12"}, mp::mpq_rational{"6"}, mp::mpq_rational{"27"}, mp::mpq_rational{"3"})

            ));

TEST(Test_mp_t, logic)
{
    const mp_t a  = mp::mpfr_float{10.3};
    const mp_t b  = mp::mpq_rational{10};
    const mp_t b2 = mp::mpfr_float{10.0};

    EXPECT_TRUE(b < a);
    EXPECT_TRUE(b <= a);
    EXPECT_FALSE(a == b);
    EXPECT_TRUE(b == b2);
    EXPECT_TRUE(-b == -b2);
    EXPECT_TRUE(b2 == b);
    EXPECT_TRUE(-b2 == -b);
    EXPECT_TRUE(a > b);
    EXPECT_TRUE(a >= b);

    EXPECT_TRUE(b == b2);    // mp::mpfr_float()
    EXPECT_TRUE(b2 == b);

    mp_t w = mp::mpq_rational{"10000/9999"};
    EXPECT_FALSE(w.isWeird());

    w = mp::mpq_rational("123456/654321");
    EXPECT_TRUE(w.isWeird());

    w = mp::mpq_rational{"1000000000000000/999999999999999"};
    EXPECT_TRUE(w.isWeird());

    auto [q, r] = mp_t::convert_to_mpz_int(w);
    EXPECT_EQ(q, 1);
    EXPECT_EQ(r, mp::mpq_rational("1/999999999999999"));


    EXPECT_FALSE(w == mp_t(1));    // TODO: review when removing multi_precision.hpp
    w.roundNear();
    EXPECT_TRUE(w == mp_t(1));

    mp_t aa = a;
    aa.clamp(mp::mpq_rational{10}, mp::mpq_rational{10});
    EXPECT_TRUE(aa == 10);
    aa = a;
    aa.clamp(mp_t{mp::mpq_rational{10.5}}, mp_t{mp::mpq_rational{11}});
    EXPECT_TRUE(aa == 10.5);
    aa = a;
    aa.clamp(mp_t{mp::mpq_rational{10}}, mp_t{mp::mpq_rational{11}});
    EXPECT_TRUE(aa == 10.3);

    EXPECT_TRUE(mp_t::abs(-a) == a);
    EXPECT_TRUE(mp_t::abs(-b) == b);
    EXPECT_TRUE(mp_t::abs(a) == a);
    EXPECT_TRUE(mp_t::abs(b) == b);

    EXPECT_FALSE(a.isZero());
    EXPECT_FALSE(b.isZero());
    EXPECT_TRUE(mp_t{mp::mpq_rational{0}}.isZero());
    EXPECT_TRUE(mp_t{mp::mpfr_float{0}}.isZero());
};

TEST(Test_mp_t, pow_sqrt_cbrt)
{
    const mp_t a  = mp::mpfr_float{2};
    const mp_t b  = mp::mpq_rational{-2};
    const mp_t c1 = mp::mpq_rational{25};
    const mp_t c2 = mp::mpfr_float{25};
    const mp_t d  = mp::mpq_rational(64);

    EXPECT_TRUE(-a == b);
    EXPECT_TRUE(a == -b);
    EXPECT_TRUE(-b == a);
    EXPECT_TRUE(b == -a);
    EXPECT_TRUE(c1 == c2);
    EXPECT_TRUE(c2 == c1);

    mp_t pa  = a ^ 2;
    mp_t pb  = b ^ 2;
    mp_t pc1 = c1 ^ 2;
    mp_t pc2 = c2 ^ 2;

    EXPECT_TRUE(pa == pb);
    EXPECT_TRUE(pc1 == pc2);
    mp_t::num_t pc1n = pc1;
    EXPECT_TRUE(std::holds_alternative<mp::mpq_rational>(pc1n));
    EXPECT_TRUE(std::holds_alternative<mp::mpq_rational>(mp_t::num_t{pc1}));
    pc1n = mp::mpq_rational{10};
    EXPECT_FALSE(pc1 == 10);

    auto nan = mp_t(mp::mpq_rational{0}) ^ -1;
    EXPECT_TRUE(nan.is_nan());

    mp_t sa  = mp_t::sqrt(a);
    mp_t sb  = mp_t::sqrt(-b);
    mp_t sc1 = mp_t::sqrt(c1);
    mp_t sc2 = mp_t::sqrt(c2);

    EXPECT_TRUE(sa == sb);
    EXPECT_TRUE(sc1 == sc2);
    EXPECT_TRUE(sc1 == 5);

    mp_t ca  = mp_t::cbrt(a);
    mp_t cb  = mp_t::cbrt(-b);
    mp_t cc1 = mp_t::cbrt(c1);
    mp_t cc2 = mp_t::cbrt(c2);
    mp_t cd  = mp_t::cbrt(d);

    EXPECT_TRUE(ca == cb);
    EXPECT_TRUE(cc1 == cc2);
    EXPECT_TRUE(cd == 4);
};

TEST(Test_mp_t, solve_factor_p__convert_to)
{
    mp_t a = mp::mpq_rational(3);
    mp_t b = mp::mpq_rational{3};

    mp_t c = b - (a * a / 3);
    EXPECT_TRUE(c == 0);

    a           = mp::mpq_rational("17/5");
    auto [q, r] = mp_t::convert_to_mpz_int(a);
    EXPECT_EQ(q, 3);
    EXPECT_EQ(r, mp::mpq_rational("2/5"));
}

TEST(Test_mp_t, formatters)
{
    mp_t a = mp::mpq_rational{"1/3"};
    EXPECT_STRCASEEQ("1/3", std::format("{}", a).c_str());

    a = mp_t::sqrt(mp::mpq_rational{2});
    EXPECT_STRCASEEQ("1.41421356237309505", std::format("{}", a).c_str());
    EXPECT_STRCASEEQ("1.414", std::format("{:.3}", a).c_str());
    EXPECT_STRCASEEQ("1", std::format("{:.0}", a).c_str());

    a = mp::mpq_rational{"1000000/999999"};
    EXPECT_STRCASEEQ("1.000001000001", std::format("{}", a).c_str());
    EXPECT_STRCASEEQ("1.000", std::format("{:.3}", a).c_str());
}

TEST(Test_mp_t, error)
{
    mp_t q = mp::mpq_rational(-1);

    ASSERT_THROW(mp_t::sqrt(q), std::runtime_error);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
