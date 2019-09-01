#ifndef CF_FUNCS_HPP
#define CF_FUNCS_HPP

#include <complex>
#include <math.h>
#include <functional>

#define Pi 3.14159265358979323846

using namespace std::literals::complex_literals; // so 1i is well-defined

template<typename F>
double integral(F f, double a, double b, int n)
{
    double step = (b - a) / n;  // width of each small rectangle
    double area = 0.0;  // signed area
    for (int i = 0; i < n; i ++)
    {
        area += f(a + (i + 0.5) * step) * step; // sum up each small rectangle
    }
    return area;
}

double Re(const std::complex<double>& arg)
{
    return arg.real();
}

#endif