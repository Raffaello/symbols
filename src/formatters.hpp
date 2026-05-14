#pragma once

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
            s = x.str(std::numeric_limits<double>::max_digits10, std::ios_base::fmtflags(0));
        else
        {
            s              = x.str(0, std::ios_base::fmtflags(0));
            int  precision = -1;
            auto dot       = spec.find('.');
            if (dot != std::string::npos)
                precision = std::stoi(spec.substr(dot + 1));

            if (precision >= 0)
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
