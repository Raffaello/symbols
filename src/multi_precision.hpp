#pragma once

#include <boost/multiprecision/mpfr.hpp>
#include <boost/multiprecision/gmp.hpp>

namespace mp = boost::multiprecision;

// typedef boost::multiprecision::mpfr_float ast_num_t;
typedef boost::multiprecision::mpq_rational ast_num_t;

// constexpr int                           REPL_PRECISION = 14;
constexpr int MPFR_PRECISION       = 192;
constexpr int MPQ_PRECISION_DIGITS = 4;

inline constexpr bool is_ast_t_rational = std::is_same_v<ast_num_t, mp::mpq_rational>;

const boost::multiprecision::mpfr_float MPFR_EPSILON = 1e-14;
const ast_num_t                         NAN_VALUE    = is_ast_t_rational ? ast_num_t("0/0") : ast_num_t("nan");

static auto mp_isWeird_rational(const mp::mpq_rational& q)
{
    const auto q_ = q.backend().data();

    if (mpz_sizeinbase(&q_->_mp_num, 10) > MPQ_PRECISION_DIGITS)
        return true;
    if (mpz_sizeinbase(&q_->_mp_den, 10) > MPQ_PRECISION_DIGITS)
        return true;

    auto a = mpz_sizeinbase(&q_->_mp_den, 2);
    if (q_->_mp_den._mp_size != 1)
        return true;

    return false;
}

template <typename T>
static auto mp_isWeird(const T& x)
{
    if constexpr (std::is_same_v<T, mp::mpq_rational>)
    {
        return mp_isWeird_rational(x);
    }
    else
        return false;
}

static auto mp_parse_decimal(const std::string& s)
{
    auto dot = s.find('.');
    if (dot == std::string::npos)
        return ast_num_t(s);

    const std::string int_part = s.substr(0, dot);
    const std::string dec_part = s.substr(dot + 1);

    if (dec_part.empty())
        return ast_num_t(int_part);

    // 4.2 => 42 / 10
    const std::string num = int_part + dec_part;
    std::string       den(dec_part.size() + 1, '0');
    den[0] = '1';
    mp::mpq_rational z(num + "/" + den);
    mpq_canonicalize(z.backend().data());
    return ast_num_t(z);
}

template <typename T>
static auto mp_pow(const T& l, const T& r)
{
    if constexpr (std::is_same_v<mp::mpq_rational, T>)
    {
        mp::mpq_rational q;

        mp::mpfr_float rf = r;
        mp::mpfr_float lf = l;
        mp::mpfr_float z  = mp::pow(lf, rf);

        mpfr_get_q(q.backend().data(), z.backend().data());
        return q;
    }
    else
    {
        return mp::pow(l, r);
    }
}

static mp::mpfr_float mp_round_near_mpfr(const mp::mpfr_float& x)
{
    const mp::mpfr_float near = mp::round(x);
    if (mp::fabs(x - near) < MPFR_EPSILON)
        return near;

    return x;
}

template <typename T>
static T mp_roundNear(const T& x)
{
    if constexpr (std::is_same_v<mp::mpq_rational, T>)
    {
        return x;
    }
    else if constexpr (std::is_same_v<mp::mpfr_float, T>)
    {
        // const ast_num_t near = mp::round(x);
        // if (mp::fabs(x - near) < MPFR_EPSILON)
        //     return near;

        // return x;
        return mp_round_near_mpfr(x);
    }
    else
    {
        return std::round(x);
    }
}

static auto mp_mpq_sqrt(const mp::mpq_rational& x)
{
    const mpz_srcptr num = &x.backend().data()->_mp_num;
    const mpz_srcptr den = &x.backend().data()->_mp_den;

    mp::mpq_rational q;
    mpz_t            num_root, den_root;
    mpz_init(num_root);
    mpz_init(den_root);

    // mpz_root returns 1 if exact, 0 if not
    if (mpz_root(num_root, num, 2) && mpz_root(den_root, den, 2))
    {
        q = mp::mpq_rational(num_root, den_root);
    }
    else
    {
        mp::mpfr_float z = x;
        z                = mp::sqrt(z);
        mpfr_get_q(q.backend().data(), z.backend().data());
    }

    mpz_clear(num_root);
    mpz_clear(den_root);
    return q;
}

template <typename T>
static auto mp_sqrt(const T& x)
{
    if constexpr (std::is_same_v<mp::mpq_rational, T>)
    {
        return mp_mpq_sqrt(x);
    }
    else
    {
        return mp::sqrt(x);
    }
}

static auto mp_mpq_cbrt(const mp::mpq_rational& x)
{
    const mpz_srcptr num = &x.backend().data()->_mp_num;
    const mpz_srcptr den = &x.backend().data()->_mp_den;

    mp::mpq_rational q;
    mpz_t            num_root, den_root;
    mpz_init(num_root);
    mpz_init(den_root);

    // mpz_root returns 1 if exact, 0 if not
    if (mpz_root(num_root, num, 3) && mpz_root(den_root, den, 3))
    {
        q = mp::mpq_rational(num_root, den_root);
    }
    else
    {
        mp::mpfr_float z = x;
        z                = mp::cbrt(z);
        mpfr_get_q(q.backend().data(), z.backend().data());
    }

    mpz_clear(num_root);
    mpz_clear(den_root);
    return q;
}

template <typename T>
static auto mp_cbrt(const T& x)
{
    if constexpr (std::is_same_v<mp::mpq_rational, T>)
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
    // if constexpr (std::is_same_v<mp::mpq_rational, ast_num_t>)
    // {
    if (x < min)
        return min;
    else if (x > max)
        return max;
    else
        return x;
    // }
    // else
    // {

    // }
}

template <typename T>
static auto mp_isZero(const T& x)
{
    if constexpr (std::is_same_v<mp::mpq_rational, T>)
    {
        return x == 0;
    }
    else
    {
        return (mp::fabs(x) < MPFR_EPSILON);    // if delta is zero (approx.)
    }
}
