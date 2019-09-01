#ifndef RNG_BASE_HPP
#define RNG_BASE_HPP

#include <memory>
#include <vector>
#include<algorithm>
namespace RNG
{
    // RNG virtual base class
    class RNG_base
    {
    public:
        virtual void init(const size_t simDim) = 0;

        virtual void nextU(std::vector<double>& uVec) = 0;
        virtual void nextG(std::vector<double>& gVed) = 0;

        virtual void reset_members(){}; 

    public:
        RNG_base() {}
        virtual ~RNG_base() {}
    }; // end of class
} // end of namespace




#endif
