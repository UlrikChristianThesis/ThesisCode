#ifndef SEQ_HPP
#define SEQ_HPP

#include <vector>
#include <numeric>
#include <algorithm>
#include <cassert>

namespace tools
{
    // returns vector with equidistant increments and includes from and to.
    // leaves out 0 because mat=0 or spot=0 doesn't make sense to price.
    std::vector<double> seq(double from, double to, const size_t length_out, bool reverse = false)
    {
        // from <= to. if reverse is wanted set reverse to true.
        if(from > to) std::__throw_runtime_error("from > to!. Set reverse = true for decreasing elements");
        std::vector<double> res(length_out);

        double delta;
        if(from == 0. | to == 0.)
        {
            delta = (to - from) / length_out;
            std::iota(res.begin(), res.end(), 0 + 1);
        }
        else
        {
            delta = (to - from) / (length_out - 1);
            std::iota(res.begin(), res.end(), 0);
        }

        std::transform(res.begin(), res.end(), res.begin(), [&] (double ele)
            {
                return from + delta * ele;
            }
        );

        if(reverse) std::reverse(res.begin(), res.end());

        return res;
    }
} // end of namespace

#endif
