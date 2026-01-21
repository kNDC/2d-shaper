#pragma once

#include <cmath>

struct Precision
{
    constexpr static double eps = 1e-10;
};

struct DblLess
{
    constexpr bool operator()(double val1, 
        double val2) const noexcept
    {
        return val1 < val2 && 
            std::abs(val1 - val2) > Precision::eps;
    }
};

struct DblGreater
{
    constexpr bool operator()(double val1, 
        double val2) const noexcept
    {
        return val1 > val2 && 
            std::abs(val1 - val2) > Precision::eps;
    }
};

struct DblEquals
{
    constexpr bool operator()(double val1, 
        double val2) const noexcept
    {
        return std::abs(val1 - val2) <= Precision::eps;
    }
};