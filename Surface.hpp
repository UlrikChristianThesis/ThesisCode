#ifndef SURFACE_HPP
#define SURFACE_HPP

#include "Model.hpp"
#include <vector>
#include "Matrix.hpp"
#include <math.h>

#include "Products.hpp"

template<typename T = double>
struct Surface_results
{
    std::vector<double> spots, mats;
    Matrix<T> iVol, lVol;
};

Surface_results<double> Generate_surface(Model& model, std::vector<double> spots, std::vector<double> mats)
{
    Surface_results<double> res;
    res.spots = spots;
    res.mats = mats;

    size_t n = spots.size();
    size_t m = mats.size();

    // our implied volatility surface. mats as rows and spots as cols seems confusing but makes sense in MC later.
    Matrix<double> iVol(m, n);

    // our local volatility surface. Still mats as rows and spots as cols also for MC.
    Matrix<double> lVol(m, n);


    // find ATM!
    // Split vector ATM and work out to the edges.
    // At each calculating check value and if not monoton or ± inf, say value equal to the previous.
    int floor_spot = floor(model.Spot());
    std::vector<double>::iterator it = std::find_if(spots.begin(), spots.end(), [&] (const double& ele)
    {
        return floor(ele) == floor_spot ? true : false;
    });

    size_t idx = std::distance(spots.begin(), it);

    // Need finer spot grid to get ATM 
    if(idx == n) std::__throw_runtime_error("ATM spot not found! Adjust spots vector.");

    // ATM iVol and lVol (assumption: ATM Vols are stable.)
    for(size_t i=0; i<m; ++i)
    {
        iVol[i][idx] = model.iVol(spots[idx], mats[i]);
        lVol[i][idx] = model.Dupires_LV(spots[idx], mats[i]);
    }

    double tol = 0.02;

    for(size_t i=0; i<m;++i)
    {
        bool go_flat = false;
        for(size_t j=idx; j --> 0;)
        {
            double ires = model.iVol(spots[j], mats[i]);
            double lres = model.Dupires_LV(spots[j], mats[i]);
            if(go_flat || abs(lres - lVol[i][j+1]) > tol || abs(ires - iVol[i][j+1]) > tol || isnan(lres) || isinf(lres))
            {
                go_flat = true;
                iVol[i][j] = iVol[i][j+1];
                lVol[i][j] = lVol[i][j+1];
            }
            else
            {
                iVol[i][j] = ires;
                lVol[i][j] = lres;
            }
        }
    }

    for(size_t i=0; i<m; ++i)
    {
        bool go_flat = false;
        for(size_t j=idx; j<n; ++j) // j is not decremented at loop entry so j=idx at start.
        {
            double ires = model.iVol(spots[j], mats[i]);
            double lres = model.Dupires_LV(spots[j], mats[i]);
            if(go_flat || abs(lres - lVol[i][j-1]) > tol || abs(ires - iVol[i][j-1]) > tol || isnan(lres) || isinf(lres))
            {
                go_flat = true;
                iVol[i][j] = iVol[i][j-1];
                lVol[i][j] = lVol[i][j-1];
            }
            else
            {
                iVol[i][j] = ires;
                lVol[i][j] = lres;
            }
            
        }
    }
    res.iVol = iVol;
    res.lVol = lVol;

    return res;
}

std::vector<double> make_simulation_timeline(std::vector<double> A, std::vector<double> B)
{
    std::vector<double> res;
    std::set_union(
                A.begin(), A.end(),
                B.begin(), B.end(),
                std::back_inserter(res));
    std::sort(res.rbegin(), res.rend());
    std::vector<double>::iterator newEnd;
    newEnd = std::unique(res.begin(), res.end(), [] (double ele1, double ele2)
    {
        return std::abs(ele1 - ele2) < 0.000000001;
    });
    res.erase(newEnd, res.end());
    return res;
}

Surface_results<double> Generate_surface(Model& model, std::vector<double> spots, std::vector<double> mats, products::Product<double>& product)
{
    auto mats_ = make_simulation_timeline(mats, product.timeline());

    return Generate_surface(model, spots, mats_);
}


Surface_results<Tdouble> Convert_to_Tdouble(const Surface_results<double> SR)
{
    Surface_results<Tdouble> res;
    res.spots = SR.spots;
    res.mats = SR.mats;

    size_t rows = SR.lVol.get_rows();
    size_t cols = SR.lVol.get_cols();

    Matrix<Tdouble> iVol = SR.iVol;
    Matrix<Tdouble> lVol = SR.lVol;
    res.iVol = iVol;
    res.lVol = lVol;

    return res;
}

Surface_results<double> Get_adjoints_SR(Surface_results<Tdouble>& SR)
{
    Surface_results<double> res;
    res.spots = SR.spots;
    res.mats = SR.mats;

    size_t rows = SR.lVol.get_rows();
    size_t cols = SR.lVol.get_cols();
    Matrix<double> iVol(rows, cols);
    Matrix<double> lVol(rows, cols);

    for(size_t i=0; i<rows; ++i)
    {
        for(size_t j=0; j<cols; ++j)
        {
            iVol[i][j] = SR.iVol[i][j].get_adjoint();
            lVol[i][j] = SR.lVol[i][j].get_adjoint();
        }
    }
    res.iVol = iVol;
    res.lVol = lVol;

    return res;
}

Surface_results<double> Get_value_SR(Surface_results<Tdouble>& SR)
{
    Surface_results<double> res;
    res.spots = SR.spots;
    res.mats = SR.mats;

    size_t rows = SR.lVol.get_rows();
    size_t cols = SR.lVol.get_cols();

    for(size_t i=0; i<rows; ++i)
    {
        for(size_t j=0; j<cols; ++j)
        {
            res.iVol[i][j] = SR.iVol[i][j].get_value();
            res.lVol[i][j] = SR.lVol[i][j].get_value();
        }
    }

    return res;
}

std::vector<bool> CommomValues(std::vector<double> A, std::vector<double> B)
{
    std::vector<bool> result(A.size());
    std::transform(A.begin(), A.end(), result.begin(),
    [&] (double a_value)
    {
        auto it = std::find_if(B.begin(), B.end(), [&] (double ele)
        {
            return std::abs(ele - a_value) < 0.000000001 ;
        });
        return it != B.end();
    }
    );

    return result;
}
#endif
