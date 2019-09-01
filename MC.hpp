#ifndef MC_HPP
#define MC_HPP

#include "Surface.hpp"
#include "RNG_base.hpp"
#include "interp.hpp"
#include "Tdouble.hpp"

template <typename T>
T smoother(const T x, const T x_pos, const T x_neg, const double eps)
{
    using namespace std;

    if( eps > 0.0001) return x_neg + (x_pos - x_neg)/eps * max<T>( T(0.0), min<T>(eps, x + eps/2.) );
    else              return x > 0 ? x_pos : x_neg;
}

// ------------------------------------------------------------------------------
//                              CALL OPTION
// ------------------------------------------------------------------------------

double MC_European_CallOption(
    const double& spot,
    const double& rate,
    const double& divs,
    double& strike,
    double& mat, 
    Surface_results<double>& surface, 
    RNG::RNG_base& some_rng, 
    const size_t& paths)
{
    // Get timeline from surface (incorporates the products key times)
    auto timeline = surface.mats;
    size_t steps = timeline.size();

    std::vector<double> 
        gaussians(steps),
        dts(steps);
    some_rng.init(steps);

    // Reverse timeline for dts 
    if (steps > 1) if (timeline[0] > timeline[1]) std::reverse(timeline.begin(), timeline.end());
    // find common steps (mat for european) 
    auto prod_steps = CommomValues(timeline, {mat});

    // Set dts 
    dts[0] = timeline[0];
    for (size_t i = 1; i < steps; ++i)
        { dts[i] = timeline[i] - timeline[i - 1]; }


    // Monte Carlo simulation
    // Loop over paths
    const double mu = rate - divs;
    double price = 0;
    for (size_t i = 0; i < paths; ++i)
    {
        // Reset counter for executable times of option
        size_t prod_step = 1;
        // Get new gaussians numbers
        some_rng.nextG(gaussians);
        double runningSpot = spot;
        double res = 0.0; 
        // Loop over steps in time
        for (size_t j = 0; j < steps; ++j)
        {
            // Simulate dynamics. Get volatility, calc running spot.
            double vol = interp(
                surface.spots.begin(),
                surface.spots.end(),
                surface.lVol[j],
                surface.lVol[j] + surface.spots.size(),
                runningSpot);
            runningSpot *= exp((mu - 0.5 * vol * vol) * dts[j] + vol * sqrt(dts[j]) * gaussians[j]);

            // If product can be exercised or add to value, check:
            if (prod_steps[j])
            {
                // Exercise at maturity
                res += payoffs::europeans::Call_standard(runningSpot, strike);
                prod_step++;
            }
        }
        price += res;
    }
    // Take average over prices and return 
    return price / double(paths);
}

double MC_European_CallOption_AAD(
    Tdouble& spot,
    Tdouble& rate,
    Tdouble& divs,
    Tdouble& strike,
    Tdouble& mat, 
    Surface_results<Tdouble>& surface, 
    RNG::RNG_base& some_rng, 
    const size_t& paths)
{
    // Get timeline from surface (incorporates the products key times)
    auto timeline = surface.mats;
    size_t steps = timeline.size();

    std::vector<double> 
        gaussians(steps),
        dts(steps);
    some_rng.init(steps);

    // Reverse timeline for dts 
    if (steps > 1) if (timeline[0] > timeline[1]) std::reverse(timeline.begin(), timeline.end());
    // find common steps (mat for european) 
    auto prod_steps = CommomValues(timeline, {mat.get_value()});

    // Set dts 
    dts[0] = timeline[0];
    for (size_t i = 1; i < steps; ++i)
        { dts[i] = timeline[i] - timeline[i - 1]; }

    // Monte Carlo simulation
    // Loop over paths
    Tdouble mu = rate - divs;
    
    // Mark Tape! 
    Tdouble res; 
    res.set_mark();

    double price = 0;
    for (size_t i = 0; i < paths; ++i)
    {
        // Get new gaussians numbers
        some_rng.nextG(gaussians);
        
        Tdouble runningSpot = spot;
        // Loop over steps in time
        for (size_t j = 0; j < steps; ++j)
        {
            // Simulate dynamics. Get volatility, calc running spot.
            Tdouble vol = interp(
                surface.spots.begin(),
                surface.spots.end(),
                surface.lVol[j],
                surface.lVol[j] + surface.spots.size(),
                runningSpot);
            runningSpot *= exp((mu - 0.5 * vol * vol) * dts[j] + vol * sqrt(dts[j]) * gaussians[j]);

            // Exercise at maturity
            if (prod_steps[j])
            {
                res = (runningSpot > strike) ? (runningSpot - strike) / paths : 0.0;
                break;
            }    
        }
        price += res.get_value();
        res.propagate_to_mark(); 
        res.set_to_mark();
    }

    // Propagate the rest of the way 
    res.propagate_from_mark_to_start();
    // res.propagate_to_start();
    return price;
}

// ------------------------------------------------------------------------------
//                                 BARRIER
// ------------------------------------------------------------------------------

double MC_European_Barrier(
    double& spot,
    double& rate,
    double& divs,
    double& strike,
    double& mat, 
    double& upper, 
    Surface_results<double>& surface, 
    RNG::RNG_base& some_rng, 
    const size_t& paths,
    const double epsilon)
{
    // Get timeline from surface (incorporates the products key times)
    auto timeline = surface.mats;
    size_t steps = timeline.size();

    std::vector<double> 
        gaussians(steps),
        dts(steps);
    some_rng.init(steps);

    // Reverse timeline for dts 
    if (steps > 1) if (timeline[0] > timeline[1]) std::reverse(timeline.begin(), timeline.end());
    // find common steps (mat for european) 
    auto prod_steps = CommomValues(timeline, {mat});

    // Set dts 
    dts[0] = timeline[0];
    for (size_t i = 1; i < steps; ++i)
        { dts[i] = timeline[i] - timeline[i - 1]; }


    // Monte Carlo simulation
    // Loop over paths
    const double mu = rate - divs;
    double price = 0;
    for (size_t i = 0; i < paths; ++i)
    {
        // Reset counter for executable times of option
        size_t prod_step = 1;
        // Get new gaussians numbers
        some_rng.nextG(gaussians);
        double runningSpot = spot;
        double res = 0.0; 
        double alive = 1.0;
        // Loop over steps in time
        for (size_t j = 0; j < steps; ++j)
        {
            // Simulate dynamics. Get volatility, calc running spot.
            double vol = interp(
                surface.spots.begin(),
                surface.spots.end(),
                surface.lVol[j],
                surface.lVol[j] + surface.spots.size(),
                runningSpot);
            runningSpot *= exp((mu - 0.5 * vol * vol) * dts[j] + vol * sqrt(dts[j]) * gaussians[j]);

            // Smoothing 
            alive = alive * smoother<double>(runningSpot - upper, 0, 1, epsilon);

            // Exercise
            if (prod_steps[j])
            {
                // Call Smooth Payoff
                res += alive * payoffs::europeans::Call_standard<double>(runningSpot, strike);
                prod_step++;
            }
        }
        price += res;
    }
    // Take average over prices and return 
    return price / double(paths);
}

double MC_European_Barrier_AAD(
    const Tdouble& spot,
    const Tdouble& rate,
    const Tdouble& divs,
    const Tdouble& strike,
    const Tdouble& mat, 
    const Tdouble& upper, 
    Surface_results<Tdouble>& surface, 
    RNG::RNG_base& some_rng, 
    const size_t& paths,
    const double epsilon)
{
    // Get timeline from surface (incorporates the products key times)
    auto timeline = surface.mats;
    size_t steps = timeline.size();

    std::vector<double> 
        gaussians(steps),
        dts(steps);
    some_rng.init(steps);

    // Reverse timeline for dts 
    if (steps > 1) if (timeline[0] > timeline[1]) std::reverse(timeline.begin(), timeline.end());
    // find common steps (mat for european) 
    auto prod_steps = CommomValues(timeline, {mat.get_value()});

    // Set dts 
    dts[0] = timeline[0];
    for (size_t i = 1; i < steps; ++i)
        { dts[i] = timeline[i] - timeline[i - 1]; }

    // Monte Carlo simulation
    // Loop over paths
    Tdouble mu = rate - divs;
    double price = 0;
    
    // MArk Tape 
    Tdouble res; 
    res.set_mark();
    for (size_t i = 0; i < paths; ++i)
    {
        // Get new gaussians numbers
        some_rng.nextG(gaussians);

        Tdouble alive = 1.0;
        Tdouble runningSpot = spot;
        // Loop over steps in time
        for (size_t j = 0; j < steps; ++j)
        {
            // Simulate dynamics. Get volatility, calc running spot.
            Tdouble vol = interp(
                surface.spots.begin(),
                surface.spots.end(),
                surface.lVol[j],
                surface.lVol[j] + surface.spots.size(),
                runningSpot);
            
            runningSpot *= exp((mu - 0.5 * vol * vol) * dts[j] + vol * sqrt(dts[j]) * gaussians[j]);

            // Smoothing 
            alive = alive * smoother<Tdouble>(runningSpot - upper, 0, 1, epsilon);

            // Exercise
            if (prod_steps[j])
            {
                // Call Smooth Payoff
                // res = alive * payoffs::europeans::Call_standard<Tdouble>(runningSpot, strike);
                res = (runningSpot > strike) ? alive * (runningSpot - strike) / paths : 0.0;
                break;
            }
        }
        // Add to price and propagate 
        price += res.get_value();
        res.propagate_to_mark();
        res.set_to_mark();
    }
    // Propagate to start 
    res.propagate_from_mark_to_start();
    return price;
}

// ------------------------------------------------------------------------------
//                              AUTO CALLABLE
// ------------------------------------------------------------------------------

double MC_Auto_Callable(
    double& spot,
    double& rate,
    double& divs,
    double& coupon,
    double& upper, 
    double& lower, 
    double& anchor, 
    const std::vector<double>& times,
    Surface_results<double>& surface, 
    RNG::RNG_base& some_rng, 
    const size_t& paths,
    const double epsilon)
{
    // Get timeline from surface (incorporates the products key times)
    auto timeline = surface.mats;
    size_t steps = timeline.size();

    std::vector<double> 
        gaussians(steps),
        dts(steps);
    some_rng.init(steps);

    // Reverse timeline for dts 
    if (steps > 1) if (timeline[0] > timeline[1]) std::reverse(timeline.begin(), timeline.end());
    // find common steps (mat for european) 
    auto prod_steps = CommomValues(timeline, times);

    // Set dts 
    dts[0] = timeline[0];
    for (size_t i = 1; i < steps; ++i)
        { dts[i] = timeline[i] - timeline[i - 1]; }

    // Monte Carlo simulation
    // Loop over paths
    const double mu = rate - divs;
    double price = 0;
    for (size_t i = 0; i < paths; ++i)
    {
        // Reset counter for executable times of option
        size_t prod_step = 1;
        // Get new gaussians numbers
        some_rng.nextG(gaussians);
        double runningSpot = spot;
        double res = 0.0; 
        double alive = 1.0;
        // Loop over steps in time
        for (size_t j = 0; j < steps; ++j)
        {
            // Simulate dynamics. Get volatility, calc running spot.
            double vol = interp(
                surface.spots.begin(),
                surface.spots.end(),
                surface.lVol[j],
                surface.lVol[j] + surface.spots.size(),
                runningSpot);
            runningSpot *= exp((mu - 0.5 * vol * vol) * dts[j] + vol * sqrt(dts[j]) * gaussians[j]);

            // Exercise ?
            if (prod_steps[j])
            {
                if (prod_step != times.size())
                {
                    res += alive * smoother(
                        runningSpot - upper,    // x 
                        prod_step * coupon,     // x pos 
                        0.0,                    // x neg
                        epsilon);               // smooth factor
                    
                    // Update alive variable 
                    alive = alive * smoother<double>(runningSpot - upper, 0, 1, epsilon);
                }
                else
                {
                    res += alive * smoother(
                            runningSpot - upper,                                    // x
                            prod_step * coupon,                                     // x positive 
                            // -payoffs::europeans::Put_standard(runningSpot, lower),  // x negative
                            // runningSpot < lower ? -(anchor - spot) : 0.0,  // x negative
                            0.0,
                            epsilon)
                            // + negative 
                            + smoother(lower - runningSpot, -(anchor - runningSpot), 0.0, epsilon);
                    
                    // Option is dead, break! 
                    break;
                }
                // Increment products step in times 
                prod_step++;
            }
        }
        price += res;
    }
    // Take average over prices and return 
    return price / double(paths);
}

double MC_Auto_Callable_AAD(
    const Tdouble& spot,
    const Tdouble& rate,
    const Tdouble& divs,
    const Tdouble& coupon,
    const Tdouble& upper, 
    const Tdouble& lower, 
    const Tdouble& anchor, 
    const std::vector<double>& times,
    Surface_results<Tdouble>& surface, 
    RNG::RNG_base& some_rng, 
    const size_t& paths,
    const double epsilon)
{
    // Get timeline from surface (incorporates the products key times)
    auto timeline = surface.mats;
    size_t steps = timeline.size();

    std::vector<double> 
        gaussians(steps),
        dts(steps);
    some_rng.init(steps);

    // Reverse timeline for dts 
    if (steps > 1) if (timeline[0] > timeline[1]) std::reverse(timeline.begin(), timeline.end());
    // find common steps (mat for european) 
    auto prod_steps = CommomValues(timeline, times);

    // Set dts 
    dts[0] = timeline[0];
    for (size_t i = 1; i < steps; ++i)
        { dts[i] = timeline[i] - timeline[i - 1]; }

    // Monte Carlo simulation
    // Loop over paths
    Tdouble mu = rate - divs;
    double price = 0;
    
    
    Tdouble res; 
    res.set_mark();
    for (size_t i = 0; i < paths; ++i)
    {
        // Reset counter for executable times of option
        size_t prod_step = 1;
        
        // Get new gaussians numbers
        some_rng.nextG(gaussians);
        
        Tdouble runningSpot = spot;
        Tdouble alive = 1.0;
        // Loop over steps in time
        for (size_t j = 0; j < steps; ++j)
        {
            // Simulate dynamics. Get volatility, calc running spot.
            Tdouble vol = interp(
                surface.spots.begin(),
                surface.spots.end(),
                surface.lVol[j],
                surface.lVol[j] + surface.spots.size(),
                runningSpot);
            runningSpot *= exp((mu - 0.5 * vol * vol) * dts[j] + vol * sqrt(dts[j]) * gaussians[j]);

            // Exercise ?
            if (prod_steps[j])
            {
                if (prod_step != times.size())
                {
                    res = alive * smoother<Tdouble>(
                        runningSpot - upper,    // x 
                        prod_step * coupon,     // x pos 
                        0.0,                    // x neg
                        epsilon) / paths;               // smooth factor
                    
                    // Update alive variable 
                    alive = alive * smoother<Tdouble>(runningSpot - upper, 0, 1, epsilon);
                    // Add value 
                    price += res.get_value();
                }
                else
                {
                    res = alive * smoother<Tdouble>(
                            runningSpot - upper,                                    // x
                            prod_step * coupon,                                     // x positive 
                            // -payoffs::europeans::Put_standard<Tdouble>(runningSpot, lower),  // x negative
                            // runningSpot < lower ? -(anchor - runningSpot): 0.0,  // x negative
                            0.0,
                            epsilon) / paths
                            // + negative
                            + smoother<Tdouble>(lower - runningSpot, -(anchor - runningSpot), 0.0, epsilon)/paths;                                               // smooth factor
                    
                    price += res.get_value();

                    // Option is dead, break! 
                    break;
                }
                // Increment products step in times 
                prod_step++;
            }
        }
        // Propagate to mark 
        res.propagate_to_mark();
        res.set_to_mark();
    }
    // Propagate to start
    res.propagate_from_mark_to_start();

    // Take average over prices and return 
    return price;
}

#endif
