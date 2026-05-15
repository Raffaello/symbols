#include "multi_precision.hpp"

mp_num_t operator-(const mp_num_t& v)
{
    return std::visit([](auto&& x) -> mp_num_t {
        using T = std::decay_t<decltype(x)>;
        return mp_num_t{T(-x)};    // force evaluation into the correct type
    },
                      v);
}

mp_num_t operator-(const mp_num_t& lhs, const mp_num_t& rhs)
{
    return std::visit([](auto&& a, auto&& b) -> mp_num_t {
        using T = std::decay_t<decltype(a)>;
        using U = std::decay_t<decltype(b)>;
        return mp_num_t{T(a) - U(b)};
    },
                      lhs,
                      rhs);
}

mp_num_t operator+(const mp_num_t& lhs, const mp_num_t& rhs)
{
    return std::visit([](auto&& a, auto&& b) -> mp_num_t {
        using T = std::decay_t<decltype(a)>;
        using U = std::decay_t<decltype(b)>;
        return mp_num_t{T(a) + U(b)};
    },
                      lhs,
                      rhs);
}

mp_num_t operator*(const mp_num_t& lhs, const mp_num_t& rhs)
{
    return std::visit([](auto&& a, auto&& b) -> mp_num_t {
        using T = std::decay_t<decltype(a)>;
        using U = std::decay_t<decltype(b)>;
        return mp_num_t{T(a) * U(b)};
    },
                      lhs,
                      rhs);
}

mp_num_t operator*(const mp_num_t& lhs, int rhs)
{
    return lhs * mp_num_t{mp::mpq_rational{rhs}};
}

mp_num_t operator/(const mp_num_t& lhs, const mp_num_t& rhs)
{
    return std::visit([](auto&& a, auto&& b) -> mp_num_t {
        using T = std::decay_t<decltype(a)>;
        using U = std::decay_t<decltype(b)>;
        return mp_num_t{T(a) / U(b)};
    },
                      lhs,
                      rhs);
}

mp_num_t operator/(const mp_num_t& lhs, int rhs)
{
    return lhs / mp_num_t{mp::mpq_rational(rhs)};
}

mp_num_t& operator+=(mp_num_t& lhs, const mp_num_t& rhs)
{
    lhs = lhs + rhs;
    return lhs;
}

mp_num_t& operator-=(mp_num_t& lhs, const mp_num_t& rhs)
{
    lhs = lhs - rhs;
    return lhs;
}

mp_num_t& operator/=(mp_num_t& lhs, const mp_num_t& rhs)
{
    lhs = lhs / rhs;
    return lhs;
}

bool operator==(const mp_num_t& lhs, const mp_num_t& rhs)
{
    return std::visit([](auto&& a, auto&& b) -> bool {
        using T = std::decay_t<decltype(a)>;
        using U = std::decay_t<decltype(b)>;
        return T(a) == U(b);
    },
                      lhs,
                      rhs);
}

bool operator==(const mp_num_t& lhs, const int& rhs)
{
    return std::visit([&](auto&& x) {
        using T = std::decay_t<decltype(x)>;
        return T(x) == T(rhs);
    },
                      lhs);
}

bool operator==(const mp_num_t& lhs, const double& rhs)
{
    return std::visit([&](auto&& x) {
        using T = std::decay_t<decltype(x)>;
        return T(x) == T(rhs);
    },
                      lhs);
}

bool operator<(const mp_num_t& lhs, const mp_num_t& rhs)
{
    return std::visit([](auto&& a, auto&& b) {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;

        return A(a) < B(b);
    },
                      lhs,
                      rhs);
}

bool operator<(const mp_num_t& lhs, int rhs)
{
    return lhs < mp_num_t{mp::mpq_rational{rhs}};
}

bool operator<=(const mp_num_t& lhs, const mp_num_t& rhs)
{
    return lhs < rhs || lhs == rhs;
}

bool operator>(const mp_num_t& lhs, const mp_num_t& rhs)
{
    return std::visit([](auto&& a, auto&& b) {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;

        return A(a) > B(b);
    },
                      lhs,
                      rhs);
}

bool operator>(const mp_num_t& lhs, int rhs)
{
    return lhs > mp_num_t{mp::mpq_rational{rhs}};
}

bool operator>=(const mp_num_t& lhs, const mp_num_t& rhs)
{
    return lhs > rhs || lhs == rhs;
}

mp::mpfr_float to_mpfr_float(const mp_num_t& x)
{
    return std::visit([](auto&& x_) -> mp::mpfr_float {
        using T = std::decay_t<decltype(x_)>;
        return static_cast<mp::mpfr_float>(T(x_));
    },
                      x);
}

///////////////////////////////////////////////////////////////////////////

bool mp_isWeird_rational(const mp::mpq_rational& q)
{
    const auto q_ = q.backend().data();

    if (mpz_sizeinbase(&q_->_mp_num, 10) > MPQ_PRECISION_DIGITS)
        return true;
    if (mpz_sizeinbase(&q_->_mp_den, 10) > MPQ_PRECISION_DIGITS)
        return true;

    if (q_->_mp_den._mp_size != 1)
        return true;

    return false;
}

mp::mpq_rational mp_parse_decimal(const std::string& s)
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

mp::mpfr_float mp_round_near_mpfr(const mp::mpfr_float& x)
{
    const mp::mpfr_float near = mp::round(x);
    if (mp::fabs(x - near) < MPFR_EPSILON)
        return near;

    return x;
}

mp::mpq_rational mp_mpq_sqrt(const mp::mpq_rational& x)
{
    const mpz_srcptr num = mpq_numref(x.backend().data());
    const mpz_srcptr den = mpq_denref(x.backend().data());

    if (mpz_sgn(num) < 0)
        throw std::runtime_error("can't compute sqrt of negative number");

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
        mpq_canonicalize(q.backend().data());
    }

    mpz_clear(num_root);
    mpz_clear(den_root);
    return q;
}

mp::mpq_rational mp_mpq_cbrt(const mp::mpq_rational& x)
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
