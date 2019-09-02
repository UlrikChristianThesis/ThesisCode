// STL includes
#include <iostream>
#include "Tdouble.hpp"
#include "Seq.hpp"
#include "interp.hpp"
#include "Node.hpp"
#include "BS.hpp"
#include <iomanip>
#include "Bates_cf.hpp"
#include "Mrg32k.hpp"
#include "Model.hpp"
#include "Surface.hpp"
#include "MC.hpp"

//PARAMETERS
#define spot_               100.
#define strike_             110.
#define vol_                0.2
#define mat_                3.
#define coupon_             10.
#define upper_              120.
#define lower_              50.
#define anchor_             100.

#define r_                  0.   // Assumes to to be 0 throughout the thesis
#define q_                  0.   // Assumes to to be 0 throughout the thesis
#define v0_                 0.04 // 0.04
#define vT_                 0.05 // 0.05
#define rho_                -.7  // -.7
#define k_                  1.   // 1.
#define sigma_              0.2  // 0.2
#define intens_             1.   // 1.
#define jump_mean_          0.05 // 0.05
#define jump_std_           0.05 // 0.05

// Call Option Params 
#define call_spot_          spot_
#define call_vol_           vol_
#define call_strike_        strike_
#define call_mat_           mat_

// Surface parameters
#define strikes_low_        40.
#define strikes_high_       200.
#define strikes_steps_      33.

#define mats_low_           0.
#define mats_high_          mat_
#define mats_steps_         72.

#define freq_               mat_/mats_steps_

// MC parameters
#define nPaths_             300000
#define smooth_factor_      5.


int main()
{

	std::cout << "Generating surfaces...." << std::endl;
    /// Params 
    double 
        spot = spot_, strike = strike_, r = r_, q = q_, mat = mat_,
        v0 = v0_, vT = vT_, rho = rho_, k = k_, sigma = sigma_, intens = intens_, jump_mean = jump_mean_, jump_std = jump_std_;        
    double 
        call_strike = call_strike_, call_mat = call_mat_, call_vol = call_vol_, call_spot = call_spot_;

    auto strikes    = tools::seq(strikes_low_, strikes_high_, strikes_steps_ );
    auto spots      = tools::seq(strikes_low_, strikes_high_, strikes_steps_ );
    auto mats       = tools::seq(mats_low_   , mats_high_   , mats_steps_    );

    size_t paths = nPaths_;

    // Model 
    Bates my_model(spot, r, q, v0, vT, rho, k , sigma, intens, jump_mean, jump_std);
    
    // Products
    products::European_Call         my_call(call_mat); 
    
    // Surfaces 
    Surface_results surface   = Generate_surface(my_model, spots, mats, my_call   );
    

	std::cout << "Calculating AAD prices...." << std::endl;

    Tdouble::tape->clear();
    Tdouble 
        Tspot = spot, Tstrike = strike, Tr = r, Tq = q, Tmat = mat;

    auto  Tsurf_call = Convert_to_Tdouble(surface);
    
	RNG::Mrg32k_RNG rng;
    double call_price_AAD = MC_European_CallOption_AAD(
        Tspot, Tr, Tq, Tstrike, Tmat, Tsurf_call, rng, paths);
	
	std::cout << "MC call price  = " << call_price_AAD << std::endl;
    std::cout << "AAD Call Delta = " << Tspot.get_adjoint() << std::endl; 

    return 0;
}
