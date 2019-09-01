#ifndef BATES_CF_HPP
#define BATES_CF_HPP

#include "cf_funcs.hpp"

namespace Bates_cf
{
    std::complex<double> cfBates(std::complex<double> om, double S, double mat, double r, double q, double v0, double vT, double rho, double k, double sigma, double intens, double jump_mean, double jump_std)
    {
        auto d = sqrt( pow(rho * sigma * 1i * om - k, 2.) + sigma * sigma * (1i * om + om * om));
        auto g2 = (k - rho * sigma * 1i * om - d) / (k - rho * sigma * 1i * om + d);
        auto cf1 = 1i * om * (log(S) + (r - q) * mat);
        auto cf2 = vT * k / (sigma * sigma) * ((k - rho * sigma * 1i * om - d) * mat - 2. * log((1. - g2 * exp(-d * mat)) / (1. - g2)));
        auto cf3 = v0 / (sigma * sigma) * (k - rho * sigma * 1i * om - d) * (1. - exp(-d * mat)) / (1. - g2 * exp(-d * mat));
        auto cf4 = -intens * jump_mean * 1i * om * mat + intens * mat *(pow(1. + jump_mean, 1i * om) * exp(0.5 * jump_std * jump_std * 1i * om * (1i*om - 1.)) - 1.);
        return exp(cf1 + cf2 + cf3 + cf4);
    }

    double P1(double om, double S, double X, double mat, double r, double q, double v0, double vT, double rho, double k, double sigma, double intens, double jump_mean, double jump_std)
    {
        return Re(exp(-1i * log(X) * om) * cfBates(om - 1i, S, mat, r, q, v0, vT, rho, k, sigma, intens, jump_mean, jump_std) / (1i * om * S * exp((r - q) * mat)));
    }

    double P2(double om, double S, double X, double mat, double r, double q, double v0, double vT, double rho, double k, double sigma, double intens, double jump_mean, double jump_std)
    {
        return Re(exp(-1i * log(X) * om) * cfBates(om, S, mat, r, q, v0, vT, rho, k, sigma, intens, jump_mean, jump_std) / (1i * om));
    }

    auto bind_args_P1(double S, double X, double mat, double r, double q, double v0, double vT, double rho, double k, double sigma, double intens, double jump_mean, double jump_std)
    {
        return [=] (double om_)
        {
            return P1(om_, S, X, mat, r, q, v0, vT, rho, k, sigma, intens, jump_mean, jump_std);
        };
    }

    auto bind_args_P2(double S, double X, double mat, double r, double q, double v0, double vT, double rho, double k, double sigma, double intens, double jump_mean, double jump_std)
    {
        return [=] (double om_)
        {
            return P2(om_, S, X, mat, r, q, v0, vT, rho, k, sigma, intens, jump_mean, jump_std);
        };
    }
} // end of namespace

double Batescf_call(double S, double X, double mat, double r, double q, double v0, double vT, double rho, double k, double sigma, double intens, double jump_mean, double jump_std)
{
    double from = 0., to = 200.;
    int steps = 400; // ?no clue?
    auto vP1 = 0.5 + 1. / Pi * integral(Bates_cf::bind_args_P1(S, X, mat, r, q, v0, vT, rho, k, sigma, intens, jump_mean, jump_std), from, to, steps);
    auto vP2 = 0.5 + 1. / Pi * integral(Bates_cf::bind_args_P2(S, X, mat, r, q, v0, vT, rho, k, sigma, intens, jump_mean, jump_std), from, to, steps);
    return exp(-q * mat) * S * vP1 - exp(-r * mat) * X * vP2;
}

#endif