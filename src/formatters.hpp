#pragma once

#include "multi_precision.hpp"

#include <format>
#include <string>
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
            int  precision = 0;
            auto dot       = spec.find('.');
            if (dot != std::string::npos)
                precision = std::stoi(spec.substr(dot + 1));

            s = x.str(precision, std::ios_base::fmtflags(0));
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
    auto format(const int_num_t& n, std::format_context& ctx) const
    {
        if (auto q_ = std::get_if<mp::mpq_rational>(&n))
        {
            auto& q = *q_;
            if (mp_isWeird(q))
            {
                mp::mpfr_float z = q;
                return std::format_to(ctx.out(), "{}", mp_roundNear(z));
            }

            return std::format_to(ctx.out(), "{}", q);
        }

        auto x = std::get<mp::mpfr_float>(n);
        return std::format_to(ctx.out(), "{}", x);
    }
};
