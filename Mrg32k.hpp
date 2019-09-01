#pragma once
#include<vector>
#include<cassert>
#include "RNG_base.hpp"
#include "Gaussian.hpp"

namespace RNG
{
	class Mrg32k_RNG : public RNG::RNG_base
	{
		size_t my_dim;

		// members 
		const double my_alpha, my_beta;
		double x0, x1, x2;
		double y0, y1, y2;

		// reuse randoms=
		bool my_anti = false;

		
		std::vector<double> my_unif;
		std::vector<double> my_gaus;

		//  Constants
		static constexpr double	m1 = 4294967087; //(2^32 - 209)
		static constexpr double	m2 = 4294944443; //(2^32 - 28532)

		// a1 vector 
		static constexpr double	a10 = 0;
		static constexpr double	a11 = 1403580;
		static constexpr double	a12 = 810728;
		// a2 vector
		static constexpr double	a20 = 527612;
		static constexpr double	a21 = 0;
		static constexpr double	a22 = 1370589;

		static constexpr double my_max = 4294967088;

		double next_unif()
		{
			double x = a11 * x1 + a12 * x2;
			x -= int((x / m1))*m1;
			x0 = x;
			x1 = x0;
			x2 = x1;

			double y = a20 * y0 + a22 * y2;
			y -= int(y / m1) * m1;
			y0 = y;
			y1 = y0;
			y2 = y1;
		
			return x > y ? 
				(x - y) / my_max : 
				(x - y + m1) / my_max;
		}

	public:
		Mrg32k_RNG(const unsigned A = 12345, const unsigned B = 54321): my_alpha(A), my_beta(B)
		{
			assert(A < m1 && B < m2);
			reset_members();
		}

		~Mrg32k_RNG()
		{
		}

		void init(const size_t simDim) override
		{
			my_dim = simDim;
			my_unif.resize(my_dim);
			my_gaus.resize(my_dim);
		}
		void reset_members() override
		{
			x0 = x1 = x2 = my_alpha;
			y0 = y1 = y2 = my_beta;
			my_anti = false;
		}

		void nextU(std::vector<double>& uVec) override
		{
			if (my_anti)
			{
				std::transform(my_unif.begin(), my_unif.end(), my_unif.begin(), [](const double entry)
				{return 1 - entry;});

				my_anti = false;
				uVec = my_unif;
				return;
			}
			else
			{
				std::generate(uVec.begin(), uVec.end(), [this]()
				{return next_unif();});

				my_anti = true;
				my_unif = uVec;
			}
		};

		virtual void nextG(std::vector<double>& gVec) override
		{
			if(my_anti)
				{
					std::transform(gVec.begin(), gVec.end(), gVec.begin(), [] (const double entry)
					{return -entry;});
					my_anti = false;
				}
				else
				{
					std::generate(gVec.begin(), gVec.end(), [this](){return gaussian::invNormalCdf(next_unif());});
					my_anti = true;
				}
		};
	};
}
