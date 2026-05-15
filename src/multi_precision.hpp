#pragma once

#include <boost/multiprecision/mpfr.hpp>
#include <boost/multiprecision/gmp.hpp>

#include <variant>

namespace mp = boost::multiprecision;

// typedef boost::multiprecision::mpfr_float ast_num_t;
typedef boost::multiprecision::mpq_rational ast_num_t;              // AST number type
using mp_num_t = std::variant<mp::mpfr_float, mp::mpq_rational>;    // interpreter number type

constexpr int MPFR_PRECISION       = 192;
constexpr int MPFR_FORMAT_DIGITS   = 18;
constexpr int MPQ_PRECISION_DIGITS = 6;

const boost::multiprecision::mpfr_float MPFR_EPSILON = 1e-14;
const mp_num_t                          NAN_VALUE    = mp_num_t{mp::mpfr_float{"nan"}};

mp_num_t  operator-(const mp_num_t& v);
mp_num_t  operator-(const mp_num_t& lhs, const mp_num_t& rhs);
mp_num_t  operator+(const mp_num_t& lhs, const mp_num_t& rhs);
mp_num_t  operator*(const mp_num_t& lhs, const mp_num_t& rhs);
mp_num_t  operator*(const mp_num_t& lhs, int rhs);
mp_num_t  operator/(const mp_num_t& lhs, const mp_num_t& rhs);
mp_num_t  operator/(const mp_num_t& lhs, int rhs);
mp_num_t& operator+=(mp_num_t& lhs, const mp_num_t& rhs);
mp_num_t& operator-=(mp_num_t& lhs, const mp_num_t& rhs);
mp_num_t& operator/=(mp_num_t& lhs, const mp_num_t& rhs);
bool      operator==(const mp_num_t& lhs, const mp_num_t& rhs);
bool      operator==(const mp_num_t& lhs, const int& rhs);
bool      operator==(const mp_num_t& lhs, const double& rhs);
bool      operator<(const mp_num_t& lhs, const mp_num_t& rhs);
bool      operator<(const mp_num_t& lhs, int rhs);
bool      operator<=(const mp_num_t& lhs, const mp_num_t& rhs);
bool      operator>(const mp_num_t& lhs, const mp_num_t& rhs);
bool      operator>(const mp_num_t& lhs, int rhs);
bool      operator>=(const mp_num_t& lhs, const mp_num_t& rhs);
// bool       operator<=(const mp_num_t& lhs, double rhs);

// mp_num_t& operator=(mp_num_t& lhs, int rhs); // TODO refactor into a class to have it
// inline operator mp::mpfr_float(); // TODO: refactor into a class so it can be used automatically by the compiler
mp::mpfr_float to_mpfr_float(const mp_num_t& x);

bool             mp_isWeird_rational(const mp::mpq_rational& q);
mp::mpq_rational mp_parse_decimal(const std::string& s);
mp::mpfr_float   mp_round_near_mpfr(const mp::mpfr_float& x);
mp::mpq_rational mp_mpq_sqrt(const mp::mpq_rational& x);
mp::mpq_rational mp_mpq_cbrt(const mp::mpq_rational& x);

template <typename T>
static bool mp_isWeird(const T& x)
{
    if constexpr (std::is_same_v<T, mp_num_t>)
    {
        if (auto q = std::get_if<mp::mpq_rational>(&x))
            return mp_isWeird(*q);
    }
    else if constexpr (std::is_same_v<T, mp::mpq_rational>)
    {
        return mp_isWeird_rational(x);
    }

    return false;
}

template <typename T, typename U>
static auto mp_pow(const T& l, const U& r)
{
    if constexpr (std::is_same_v<T, U>)
    {
        if constexpr (std::is_same_v<mp_num_t, T>)
        {
            return std::visit([](auto&& a, auto&& b) -> mp_num_t {
                using V = std::decay_t<decltype(a)>;
                using W = std::decay_t<decltype(b)>;
                return mp_num_t{mp_pow(V(a), W(b))};
            },
                              l,
                              r);
        }
        else if constexpr (std::is_same_v<mp::mpq_rational, T>)
        {
            mp::mpq_rational q;

            mp::mpfr_float rf = r;
            mp::mpfr_float lf = l;
            mp::mpfr_float z  = mp::pow(lf, rf);

            mpfr_get_q(q.backend().data(), z.backend().data());
            return q;
        }

        else    // if constexpr (std::is_same_v<mp::mpfr_float, T>)
        {
            return mp::pow(l, r);
        }
    }
    else if constexpr (std::is_same_v<mp_num_t, T>)
    {
        return std::visit([&](auto&& a) -> mp_num_t {
            using V = std::decay_t<decltype(a)>;
            return mp_num_t{mp_pow(V(a), r)};
        },
                          l);
    }
    else
    {
        // fallback
        mp::mpfr_float rf = r;
        mp::mpfr_float lf = l;
        mp::mpfr_float z  = mp::pow(lf, rf);

        return z;
    }
}

template <typename T>
static T mp_roundNear(const T& x)
{
    if constexpr (std::is_same_v<T, mp_num_t>)
    {
        if (auto q = std::get_if<mp::mpq_rational>(&x))
        {
            if (!mp_isWeird(*q))
                return x;
        }

        return mp_num_t{mp_roundNear(to_mpfr_float(x))};
    }
    else if constexpr (std::is_same_v<mp::mpq_rational, T>)
    {
        return x;
    }
    else    // if constexpr (std::is_same_v<mp::mpfr_float, T>)
    {
        return mp_round_near_mpfr(x);
    }
}

template <typename T>
static mp::mpz_int mp_extract_mpz_int(const T& x)
{
    if constexpr (std::is_same_v<T, mp_num_t>)
    {
        if (auto q = std::get_if<mp::mpq_rational>(&x))
            return mp_extract_mpz_int(*q);

        return mp_extract_mpz_int<mp::mpfr_float>(to_mpfr_float(x));
    }
    else if constexpr (std::is_same_v<T, mp::mpq_rational>)
    {
        mp::mpz_int n, d;
        mpz_srcptr  num_ptr = mpq_numref(x.backend().data());
        mpz_srcptr  den_ptr = mpq_denref(x.backend().data());
        mpz_set(n.backend().data(), num_ptr);
        mpz_set(d.backend().data(), den_ptr);
        mp::mpfr_float z = static_cast<mp::mpfr_float>(n) / static_cast<mp::mpz_int>(d);
        return mp_extract_mpz_int(z);
    }
    else
    {
        return static_cast<mp::mpz_int>(mp_roundNear<mp::mpfr_float>(x));
    }
}

template <typename T>
static auto mp_sqrt(const T& x)
{
    if constexpr (std::is_same_v<mp_num_t, T>)
    {
        return std::visit([](auto&& a) -> mp_num_t {
            using U = std::decay_t<decltype(a)>;
            return mp_num_t{mp_sqrt(U(a))};
        },
                          x);
    }
    else if constexpr (std::is_same_v<mp::mpq_rational, T>)
    {
        return mp_mpq_sqrt(x);
    }
    else
    {
        return mp::sqrt(x);
    }
}

template <typename T>
static auto mp_cbrt(const T& x)
{
    if constexpr (std::is_same_v<mp_num_t, T>)
    {
        return std::visit([](auto&& a) -> mp_num_t {
            using U = std::decay_t<decltype(a)>;
            return mp_num_t{mp_cbrt(U(a))};
        },
                          x);
    }
    else if constexpr (std::is_same_v<mp::mpq_rational, T>)
    {
        return mp_mpq_cbrt(x);
    }
    else
    {
        return mp::cbrt(x);
    }
}

template <typename T>
static auto mp_clamp(const T& x, const T& min, const T& max)
{
    if (x < min)
        return min;
    else if (max < x)
        return max;
    else
        return x;
}

template <typename T>
static T mp_abs(const T& x)
{
    if constexpr (std::is_same_v<T, mp_num_t>)
    {
        return std::visit([](auto&& a) -> mp_num_t {
            using U = std::decay_t<decltype(a)>;
            return mp_num_t{mp_abs(U(a))};
        },
                          x);
    }
    else if constexpr (std::is_same_v<mp::mpq_rational, T>)
    {
        return mp::abs(x);
    }
    else
    {
        return mp::abs(x);
    }
}

template <typename T>
static bool mp_isZero(const T& x)
{
    if constexpr (std::is_same_v<T, mp_num_t>)
    {
        if (auto q = std::get_if<mp::mpq_rational>(&x))
        {
            return *q == 0;
        }
        else
            return mp_isZero<mp::mpfr_float>(std::get<mp::mpfr_float>(x));
    }
    else if constexpr (std::is_same_v<mp::mpq_rational, T>)
    {
        return x == 0;
    }
    else
    {
        return (mp::fabs(x) < MPFR_EPSILON);    // if delta is zero (approx.)
    }
}
