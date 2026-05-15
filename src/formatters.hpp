#pragma once

#include "multi_precision.hpp"

#include <format>
#include <string>
#include <string_view>
#include <boost/multiprecision/mpfr.hpp>
#include <boost/multiprecision/gmp.hpp>

template <>
struct std::formatter<boost::multiprecision::mpfr_float> : std::formatter<std::string>
{
    std::string spec;

    constexpr auto parse(std::format_parse_context& ctx)
    {
        auto it  = ctx.begin();
        auto end = ctx.end();

        // width
        while (it != end && *it != '}')
            spec.push_back(*it++);

        return it;
    }

    auto format(const boost::multiprecision::mpfr_float& x, std::format_context& ctx) const
    {
        std::string s;
        auto        out = ctx.out();
        if (spec.empty())
            s = x.str(MPFR_FORMAT_DIGITS, std::ios_base::fmtflags(0));
        else
        {
            std::ostringstream oss;

            int  precision = 0;
            auto dot       = spec.find('.');
            if (dot != std::string::npos)
                precision = std::stoi(spec.substr(dot + 1));

            oss << std::fixed << std::setprecision(precision) << x;
            s = oss.str();
        }

        return std::formatter<std::string>::format(s, ctx);
    }
};

template <>
struct std::formatter<boost::multiprecision::mpq_rational> : std::formatter<std::string>
{
    auto format(const boost::multiprecision::mpq_rational& q, std::format_context& ctx) const
    {
        std::string s = q.str();
        return std::formatter<std::string>::format(s, ctx);
    }
};

template <>
struct std::formatter<int_num_t> : std::formatter<std::string>
{
    std::string_view spec;

    constexpr auto parse(std::format_parse_context& ctx)
    {
        auto it  = ctx.begin();
        auto end = ctx.end();
        auto beg = it;

        while (it != end && *it != '}')
            ++it;

        spec = std::string_view(beg, it - beg);
        return it;
    }

    auto format_mpfr(const mp::mpfr_float& z, std::format_context& ctx) const
    {
        std::formatter<mp::mpfr_float, char> formatter;
        std::format_parse_context            pctx(spec);
        formatter.parse(pctx);
        return formatter.format(z, ctx);
    }

    auto format(const int_num_t& n, std::format_context& ctx) const
    {
        if (auto q_ = std::get_if<mp::mpq_rational>(&n))
        {
            auto& q = *q_;
            if (mp_isWeird(q))
            {
                mp::mpfr_float z = q;

                return format_mpfr(z, ctx);
            }

            return std::format_to(ctx.out(), "{}", q);
        }

        auto x = std::get<mp::mpfr_float>(n);
        return format_mpfr(x, ctx);
    }
};
