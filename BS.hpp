#ifndef BS_HPP
#define BS_HPP

// STL includes
#include <math.h>

// user includes 
#include "Gaussian.hpp"
// Constants 
#define EPS 1.e-12

template<typename T>
T Black_scholes(const T& spot, const T& strike, const T& vol, const T& mat )
{
    using namespace gaussian;
    T std = sqrt(mat) * vol;
    T halfVar = 0.5 * std * std;
    T d1 = (log(spot / strike) + halfVar) / std; // doesnt work?
    T d2 = d1 - std;
    T result =  spot * normalCdf(d1) - strike * normalCdf(d2);
    return result;
}

//  Implied vol, untemplated
inline double Black_Scholes_Ivol(
    const double spot,
    const double strike,
    const double prem,
    const double mat)
{
    if (prem <= std::max(0.0, spot - strike) + EPS) return 0.0;

    double p, pu, pl;
    double u = 0.5;
    while (Black_scholes(spot, strike, u, mat) < prem) u *= 2;
    double l = 0.05;
    while (Black_scholes(spot, strike, l, mat) > prem) l /= 2;
    pu = Black_scholes(spot, strike, u, mat);
    Black_scholes(spot, strike, l, mat);

    while (u - l > 1.e-12)
    {
        const double m = 0.5 * (u + l);
        p = Black_scholes(spot, strike, m, mat);
        if (p > prem)
        {
            u = m;
            pu = p;
        }
        else
        {
            l = m;
            pl = p;
        }
    }

    return l + (prem - pl) / (pu - pl) * (u - l);
}

#endif
