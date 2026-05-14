#pragma once

#include <boost/multiprecision/mpfr.hpp>
#include <boost/multiprecision/gmp.hpp>

typedef boost::multiprecision::mpfr_float ast_num_t;
// typedef boost::multiprecision::mpq_rational ast_num_t;

// constexpr int                           REPL_PRECISION = 14;
constexpr int                           MPFR_PRECISION = 210;
const boost::multiprecision::mpfr_float MPFR_EPSILON   = 1e-5;


namespace mp = boost::multiprecision;

template <typename T>
static auto mp_pow(const T& l, const T& r)
{
    if constexpr (std::is_same_v<mp::mpq_rational, ast_num_t>)
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

template <typename T>
static auto mp_roundNear(const T& x)
{
    if constexpr (std::is_same_v<mp::mpq_rational, ast_num_t>)
    {
        return x;
    }
    else
    {
        const ast_num_t near = mp::round(x);
        if (mp::fabs(x - near) < MPFR_EPSILON)
            return near;

        return x;
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
    if constexpr (std::is_same_v<mp::mpq_rational, ast_num_t>)
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
    if constexpr (std::is_same_v<mp::mpq_rational, ast_num_t>)
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
    if constexpr (std::is_same_v<mp::mpq_rational, ast_num_t>)
    {
        return x == 0;
    }
    else
    {
        return (mp::fabs(x) < MPFR_EPSILON);    // if delta is zero (approx.)
    }
}
