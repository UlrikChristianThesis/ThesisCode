#ifndef PRODUCTS_HPP
#define PRODUCTS_HPP

#include<vector>
#include<algorithm>
#include<cassert>

// Generic smoother (See Scripts and xVA's - AS 2017)
template <typename T>
T fIf(const T x, const T x_pos, const T x_neg, const double eps)
{
    using namespace std;
    return x_neg + (x_pos - x_neg)/eps * max<T>( T(0.0), min<T>(eps, x + eps/2.) );
}

// -----------------------------------------------------------------------------------------------
// -----------------------------------------PAYOFFS-----------------------------------------------
// -----------------------------------------------------------------------------------------------

namespace payoffs
{
    namespace europeans
    {
        template<typename T>
        T Call_standard(T& spot, T& strike)
        {
            T res; 
            spot > strike? 
                res = spot - strike : 
                res = 0.0;
            return res; 
        }
    } // namespace europeans
} // namespace payoffs

// -----------------------------------------------------------------------------------------------
// ----------------------------------------PRODUCTS-----------------------------------------------
// -----------------------------------------------------------------------------------------------


namespace products
{   
    // BASE CLASS - BLUEPRINT FOR PRODUCT Class
    template <typename T>
    struct Product
    {  
        virtual ~Product(){}
        
        // Products have timeline for exercision  
        virtual std::vector<double> timeline() const = 0;
    };

    // Call Option: Holds strike and maturity as T's  
    template <typename T>
    class European_Call : public Product<T>
    {
    public:
        T my_maturity;

        const size_t steps = 1;
        std::vector<double> my_timeline;

    public:
        //member 
        
        //CTOR
        European_Call(T& maturity)
        : my_maturity(maturity)
        {
            my_timeline.push_back(double(maturity));
        }
        // Access 
        std::vector<double> timeline() const override { return my_timeline; }
        ~European_Call(){}
    };

    // Up and Out Call Option (UOC)
    template<typename T>
    class Up_and_Out_Call : public Product<T>
    {
    public:
        T maturity;

        const double freq;
        std::vector<double> my_timeline;
        size_t              steps; 
    
    public:
        // CTOR
        Up_and_Out_Call(T& maturity_, const double freq_) 
            : maturity(maturity_), freq(freq_) 
        {
            double time = freq;
            // Create timeline  
            while (time < maturity)
            {
                my_timeline.push_back(time);
                time += freq;
            }
            if(my_timeline.back() != maturity) my_timeline.push_back(double(maturity));

            steps = my_timeline.size();
        }
        
        std::vector<double> timeline() const override { return my_timeline; }
        
        ~Up_and_Out_Call() {}
    };

    class Equity_AutoCallable : public Product<double>
    {
    public:
        std::vector<double> my_timeline;
        size_t steps;
    
    public:
        
        // CTOR
        Equity_AutoCallable(const std::vector<double> times)
        {
            my_timeline = times;
            steps = my_timeline.size(); 
        }

        // Access 
        std::vector<double> timeline() const override {return my_timeline;}
    };

}// namespace products 


#endif   
