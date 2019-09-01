#pragma once

#include <algorithm>
using namespace std;

// Borrowed from Antoine Savine Github repo
template <class ITX, class ITY, class T>
inline auto interp(
    ITX                         xBegin,
    ITX                         xEnd,
    ITY                         yBegin,
    ITY                         yEnd,
    const T&					x0)
    ->remove_reference_t<decltype(*yBegin)>
{
    auto it = upper_bound(xBegin, xEnd, x0);

    if (it == xEnd) return *(yEnd - 1);
    if (it == xBegin) return *yBegin;

    size_t n = distance(xBegin, it) - 1;
    auto x1 = xBegin[n];
    auto y1 = yBegin[n];
    auto x2 = xBegin[n + 1];
    auto y2 = yBegin[n + 1];

    auto t = (x0 - x1) / (x2 - x1);

	return y1 + (y2 - y1) * t;
}
