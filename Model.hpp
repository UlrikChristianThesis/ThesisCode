#ifndef MODEL_HPP
#define MODEL_HPP

#include "BS.hpp"

class Model
{
protected:
    // holds spot for implied vol formula.
    const double S;
public:
    Model(double S_) : S(S_) {}
    // Must be implemented by derived call. Returns a EUR call price.
    virtual double call(double strike, double mat) = 0;
    // Returns the implied volatility under some derived model.
    double iVol(double strike, double mat)
    {
        // if call_price = 0 or intrisic do something!
        double call_price = call(strike, mat);
        return Black_Scholes_Ivol(S, strike, call_price, mat);
    }

    // calculates Dupires formula using FD to estimat derivaties.
    double Dupires_LV(double strike, double mat)
    {
        const double tol = 0.0001;
        // simple FD find call_T
        const double call_T = (call(strike, mat + tol) - call(strike, mat - tol)) * 1./(2. * tol);
        // simple DF find call_KK
        const double call_KK = (call(strike - tol, mat) + call(strike + tol, mat) - 2. * call(strike, mat)) * 1./(tol * tol);

        // Dupires formula
        return sqrt(2. * call_T / call_KK) / strike;
    }

    double Spot(){return S;}
};

#include "Bates_cf.hpp"

class Bates : public Model
{
    const double r, q, v0, vT, rho, kappa, sigma, intens, jump_mean, jump_std;

    public:
    Bates(const double S_,
        const double r_,
        const double q_,
        const double v0_,
        const double vT_,
        const double rho_,
        const double kappa_,
        const double sigma_,
        const double intens_,
        const double jump_mean_,
        const double jump_std_)
        : Model(S_),
        r(r_),
        q(q_),
        v0(v0_),
        vT(vT_),
        rho(rho_),
        kappa(kappa_),
        sigma(sigma_),
        intens(intens_),
        jump_mean(jump_mean_),
        jump_std(jump_std_)
        {}

        double call(double strike, double mat) override
        {
            return Batescf_call(S, strike, mat, r, q, v0, vT, rho, kappa, sigma, intens, jump_mean, jump_mean);
        }
};
#endif
