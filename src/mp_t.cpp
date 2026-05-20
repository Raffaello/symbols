#include "mp_t.hpp"

mp_t& mp_t::operator=(const mp_t& rhs)
{
    m_value = rhs.m_value;
    return *this;
}

mp_t mp_t::operator-() const
{
    return std::visit([](auto&& a) -> num_t {
        using T = std::decay_t<decltype(a)>;
        return -T(a);
    },
                      m_value);
}

mp_t mp_t::operator-(const mp_t& rhs) const
{
    return std::visit([](auto&& a, auto&& b) -> num_t {
        using T = std::decay_t<decltype(a)>;
        using U = std::decay_t<decltype(b)>;
        return T(a) - U(b);
    },
                      m_value,
                      rhs.m_value);
}

mp_t mp_t::operator+(const mp_t& rhs) const
{
    return std::visit([](auto&& a, auto&& b) -> num_t {
        using T = std::decay_t<decltype(a)>;
        using U = std::decay_t<decltype(b)>;
        return T(a) + U(b);
    },
                      m_value,
                      rhs.m_value);
}

mp_t mp_t::operator*(const mp_t& rhs) const
{
    return std::visit([](auto&& a, auto&& b) -> num_t {
        using T = std::decay_t<decltype(a)>;
        using U = std::decay_t<decltype(b)>;
        return T(a) * U(b);
    },
                      m_value,
                      rhs.m_value);
}

mp_t mp_t::operator/(const mp_t& rhs) const
{
    return std::visit([](auto&& a, auto&& b) -> num_t {
        using T = std::decay_t<decltype(a)>;
        using U = std::decay_t<decltype(b)>;
        return T(a) / U(b);
    },
                      m_value,
                      rhs.m_value);
}

mp_t mp_t::operator^(const mp_t& rhs) const
{
    if (auto q_ = std::get_if<mp::mpq_rational>(&m_value))
    {
        auto l = *q_;
        if (auto v_ = std::get_if<mp::mpq_rational>(&rhs.m_value))
        {
            auto r = *v_;

            return num_pow_(l, r);
        }
    }

    // all other cases
    mp::mpfr_float l = *this;
    return std::visit([&](auto&& r) -> num_t {
        using T = std::decay_t<decltype(r)>;
        return mp::pow(l, T(r));
    },
                      rhs.m_value);
}

mp_t& mp_t::operator+=(const mp_t& rhs)
{
    *this = *this + rhs;
    return *this;
}

mp_t& mp_t::operator-=(const mp_t& rhs)
{
    *this = *this - rhs;
    return *this;
}

mp_t& mp_t::operator/=(const mp_t& rhs)
{
    *this = *this / rhs;
    return *this;
}

bool mp_t::operator==(const mp_t& rhs) const
{
    return std::visit([](auto&& a, auto&& b) -> bool {
        using T = std::decay_t<decltype(a)>;
        using U = std::decay_t<decltype(b)>;
        return T(a) == U(b);
    },
                      m_value,
                      rhs.m_value);
}

bool mp_t::operator<(const mp_t& rhs) const
{
    return std::visit([](auto&& a, auto&& b) {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;

        return A(a) < B(b);
    },
                      m_value,
                      rhs.m_value);
}

bool mp_t::operator<=(const mp_t& rhs) const
{
    return *this < rhs || *this == rhs;
}

bool mp_t::operator>(const mp_t& rhs) const
{
    return std::visit([](auto&& a, auto&& b) {
        using A = std::decay_t<decltype(a)>;
        using B = std::decay_t<decltype(b)>;

        return A(a) > B(b);
    },
                      m_value,
                      rhs.m_value);
}

bool mp_t::operator>=(const mp_t& rhs) const
{
    return *this > rhs || *this == rhs;
}

mp_t::operator const mp::mpfr_float() const
{
    return std::visit([](auto&& x_) -> mp::mpfr_float {
        return x_.template convert_to<mp::mpfr_float>();
    },
                      m_value);
}

mp_t::operator const mp::mpq_rational() const
{
    return std::visit([](auto&& x_) -> mp::mpq_rational {
        return x_.template convert_to<mp::mpq_rational>();
    },
                      m_value);
}

mp_t::operator const num_t&() const
{
    return m_value;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

mp_t::mp_t() : m_value(NAN_VALUE)
{
}

mp_t::mp_t(mp_t&& n) : m_value(std::move(n.m_value))
{
}

mp_t::mp_t(const mp_t& n) : m_value(n.m_value)
{
}

mp_t::mp_t(const num_t& n) : m_value(n)
{
}

mp_t::mp_t(const mp::mpfr_float& x) : m_value(x)
{
}

mp_t::mp_t(const mp::mpq_rational& q) : m_value(q)
{
}

mp_t::mp_t(const std::string& str) : m_value(parse_decimal(str))
{
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool mp_t::isWeird_rational_(const mp::mpq_rational& q)
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

mp::mpfr_float mp_t::round_near_mpfr_(const mp::mpfr_float& x)
{
    const mp::mpfr_float near = mp::round(x);
    if (mp::fabs(x - near) < MPFR_EPSILON)
        return near;

    return x;
}

mp::mpq_rational mp_t::mpq_sqrt_(const mp::mpq_rational& x)
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

mp::mpq_rational mp_t::mpq_cbrt_(const mp::mpq_rational& x)
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

mp_t::num_t mp_t::num_pow_(const mp::mpq_rational& l, const mp::mpq_rational& r)
{
    // if r has a denominator == 1, it can be compute directly as a mpq
    if (denominator(r) == 1)
    {
        mp::mpz_int exp = numerator(r);
        const bool  neg = exp < 0;
        if (r.sign() < 0)
            exp = -exp;

        mp::mpz_int n      = 1;
        mp::mpz_int d      = 1;
        mp::mpz_int base_n = numerator(l);
        mp::mpz_int base_d = denominator(l);

        while (exp > 0)
        {
            if (exp % 2 == 1)
            {
                n *= base_n;
                d *= base_d;
            }

            base_n *= base_n;
            base_d *= base_d;

            exp /= 2;
        }

        if (neg)
        {
            if (n == 0)
                return NAN_VALUE;
            else
                return mp::mpq_rational{d, n};
        }
        else
            return mp::mpq_rational{n, d};
    }

    // Non-integer exponent — fall back to float approximation.
    mp::mpq_rational q;

    mp::mpfr_float rf = r;
    mp::mpfr_float lf = l;
    mp::mpfr_float z  = mp::pow(lf, rf);

    mpfr_get_q(q.backend().data(), z.backend().data());
    return q;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool mp_t::is_nan() const noexcept
{
    if (std::get_if<mp::mpq_rational>(&m_value))
        return false;

    const auto z = std::get<mp::mpfr_float>(m_value);
    return mpfr_nan_p(z.backend().data());
}

bool mp_t::isWeird() const noexcept
{
    if (auto q = std::get_if<mp::mpq_rational>(&m_value))
        return isWeird_rational_(*q);

    return false;
}

bool mp_t::isZero() const noexcept
{
    if (auto q = std::get_if<mp::mpq_rational>(&m_value))
    {
        return q->is_zero();
    }

    return mp::fabs(mp::mpfr_float{*this}) < MPFR_EPSILON;
}

std::string mp_t::str() const
{
    return std::visit(
        [](const auto& x) {
            return x.str();
        },
        m_value);
}

void mp_t::roundNear()
{
    if (auto q = std::get_if<mp::mpq_rational>(&m_value))
    {
        if (!isWeird())
            return;
    }

    m_value = round_near_mpfr_(*this);
}

void mp_t::clamp(const mp_t& min, const mp_t& max) noexcept
{
    if (*this < min)
        *this = min;
    else if (*this > max)
        *this = max;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

mp::mpq_rational mp_t::parse_decimal(const std::string& s)
{
    auto dot = s.find('.');
    if (dot == std::string::npos)
        return mp::mpq_rational(s);

    const std::string int_part = s.substr(0, dot);
    const std::string dec_part = s.substr(dot + 1);

    if (dec_part.empty())
        return mp::mpq_rational(int_part);

    const std::string num = int_part + dec_part;
    std::string       den(dec_part.size() + 1, '0');
    den[0] = '1';
    mp::mpq_rational z(num + "/" + den);
    mpq_canonicalize(z.backend().data());
    return z;
}

std::pair<mp::mpz_int, mp::mpq_rational> mp_t::convert_to_mpz_int(const mp_t& x)
{
    return std::visit(
        [](const auto& z) {
            auto q = z.template convert_to<mp::mpz_int>();
            auto r = z - q;

            return std::pair<mp::mpz_int, mp::mpq_rational>{q, r};
        },
        x.m_value);
}

mp_t mp_t::abs(const mp_t& x)
{
    if (auto q_ = std::get_if<mp::mpq_rational>(&x.m_value))
    {
        const mp::mpq_rational v = mp::abs(*q_);
        return v;
    }

    auto z = std::get<mp::mpfr_float>(x.m_value);
    z      = mp::fabs(z);
    return z;
}

mp_t mp_t::sqrt(const mp_t& x)
{
    if (auto q = std::get_if<mp::mpq_rational>(&x.m_value))
        return mpq_sqrt_(*q);
    else
        return mp::sqrt(mp::mpfr_float(x));
}

mp_t mp_t::cbrt(const mp_t& x)
{
    if (auto q = std::get_if<mp::mpq_rational>(&x.m_value))
        return mpq_cbrt_(*q);
    else
        return mp::cbrt(mp::mpfr_float(x));
}

mp_t mp_t::cos(const mp_t& x)
{
    mp::mpfr_float z = x;
    z                = mp::cos(z);
    return z;
}

mp_t mp_t::acos(const mp_t& x)
{
    mp::mpfr_float z = x;
    z                = mp::acos(z);
    return z;
}
