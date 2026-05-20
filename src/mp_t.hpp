#pragma once

#include <boost/multiprecision/mpfr.hpp>
#include <boost/multiprecision/gmp.hpp>

#include <variant>
#include <string>
#include <utility>
#include <concepts>
#include <type_traits>

namespace mp = boost::multiprecision;

typedef mp::mpq_rational ast_num_t;    // AST number type

template <typename T>
concept Integral = std::is_integral_v<T>;

template <typename T>
concept FloatingPoint = std::is_floating_point_v<T>;

class mp_t
{
public:
    using num_t = std::variant<mp::mpfr_float, mp::mpq_rational>;    // multiprecision number type

    static constexpr int MPFR_PRECISION       = 192;
    static constexpr int MPFR_FORMAT_DIGITS   = 18;
    static constexpr int MPQ_PRECISION_DIGITS = 6;

private:
    num_t m_value;

public:
    mp_t();
    mp_t(mp_t&& n);

    mp_t(const mp_t& n);
    mp_t(const num_t& n);
    mp_t(const mp::mpfr_float& x);
    mp_t(const mp::mpq_rational& q);
    mp_t(Integral auto v);
    mp_t(FloatingPoint auto v);
    mp_t(const std::string& str);

    mp_t& operator=(const mp_t& rhs);
    mp_t  operator-() const;
    mp_t  operator-(const mp_t& rhs) const;
    mp_t  operator+(const mp_t& rhs) const;
    mp_t  operator*(const mp_t& rhs) const;
    mp_t  operator/(const mp_t& rhs) const;
    mp_t  operator^(const mp_t& rhs) const;

    mp_t& operator+=(const mp_t& rhs);
    mp_t& operator-=(const mp_t& rhs);
    mp_t& operator/=(const mp_t& rhs);

    bool operator==(const mp_t& rhs) const;
    bool operator<(const mp_t& rhs) const;
    bool operator<=(const mp_t& rhs) const;
    bool operator>(const mp_t& rhs) const;
    bool operator>=(const mp_t& rhs) const;

    operator const mp::mpfr_float() const;
    operator const mp::mpq_rational() const;
    operator const num_t&() const;

private:
    static bool             isWeird_rational_(const mp::mpq_rational& q);
    static mp::mpfr_float   round_near_mpfr_(const mp::mpfr_float& x);
    static mp::mpq_rational mpq_sqrt_(const mp::mpq_rational& x);
    static mp::mpq_rational mpq_cbrt_(const mp::mpq_rational& x);
    static num_t            num_pow_(const mp::mpq_rational& l, const mp::mpq_rational& r);

public:
    bool        is_nan() const noexcept;
    bool        isWeird() const noexcept;
    bool        isZero() const noexcept;
    std::string str() const;

    void roundNear();
    void clamp(const mp_t& min, const mp_t& max) noexcept;

    static mp::mpq_rational                         parse_decimal(const std::string& s);
    static std::pair<mp::mpz_int, mp::mpq_rational> convert_to_mpz_int(const mp_t& x);
    static mp_t                                     abs(const mp_t& x);
    static mp_t                                     sqrt(const mp_t& x);
    static mp_t                                     cbrt(const mp_t& x);
    static mp_t                                     cos(const mp_t& x);
    static mp_t                                     acos(const mp_t& x);
};

inline const mp_t MPFR_EPSILON = 1e-14;
inline const mp_t NAN_VALUE    = mp::mpfr_float{"nan"};

mp_t::mp_t(Integral auto v) : mp_t(mp::mpq_rational{v})
{
}

mp_t::mp_t(FloatingPoint auto v) : mp_t(mp::mpfr_float{v})
{
}

// template <std::arithmetic T>
template <typename T,
          typename = std::enable_if_t<std::is_arithmetic_v<T>>>
mp_t operator*(T lhs, const mp_t& rhs)
{
    return mp_t(lhs) * rhs;
}
