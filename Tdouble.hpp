#ifndef TDOUBLE_HPP
#define TDOUBLE_HPP

// STL includes
#include "math.h"

// user includes
#include "Tape.hpp"
#include "Gaussian.hpp"

// Taped double class. Acts as a double but is instrumented to record the DAG created when used instead of double.
// Can be propagated to calculate derived values of previous Tdoubles, either to start or to a marked Tdouble 
// for checkpointing.
// Information is accessed through the Node (see Node.hpp) member that holds the corresponding adjoint, pointers to child nodes 
// (or Tdoubles) and the partial derivatives. These are stored on Tape (see Tape.hpp), using the container ListArray (see ListArray.hpp)
// 
// Leafs in the DAG are initialized by the standard CTOR 'Tdouble someTdouble(double(someValue));'
// Tdoubles constructed from other Tdoubles can be assigned like doubles. 
class Tdouble{
// Private members  
private:
    double my_value;
    Node* my_node;

// Public members
public:
    static Tape* tape;

// Member functions  
public:
    // CTORS
    Tdouble(){};
    Tdouble(double value_)           : my_value(value_){my_node = tape->record_node(0);}
    Tdouble(double value_, size_t N) : my_value(value_){my_node = tape->record_node(N);}

    // Assignment operator - Recording the Leaf Node on tape   
    Tdouble& operator=(const double val)
    {
		my_value = val;                     // store value
		my_node = tape->record_node(0);     // store address for node on tape
        return *this;
    }

    // Explicit conversion to double 
    explicit operator double&()      {return my_value;}
    explicit operator double() const {return my_value;}

    // Getters
    double& get_value()       {return my_value;} 
    double  get_value() const {return my_value;}
    Node&   get_node()        {return *my_node;}
    
    // Access to Node's members 
    double&  get_adjoint() const {return my_node->get_adjoint();}
    double*  weights()           {return my_node->get_weights();}
    double** child_adjoint()     {return my_node->get_child_adjoint();}

    // set mark in tape at the CURRENT position 
    static void set_mark(){tape->mark_tape();}
    static void set_to_mark(){tape->set_to_mark();}

    
// ---------------------------------------------------------------
// - PROPAGATION  
// ---------------------------------------------------------------
private:
    // Main machine for propagating Tdoubles/nodes.
    // Takes 2 iterators as input and propagates through the tape between these.  
    void propagate(Tape::iterator it_from, Tape::iterator it_to, bool set_adjoint = false)
    {   
        if (set_adjoint)
        {
            it_from->set_adjoint_to_1();
        }
        
        ++it_from; // match end() for use of reverse iterators
        auto rbegin = std::make_reverse_iterator<containers::List_array<Node, LA_node_size>
            ::iterator>(it_from);
        
        auto rend   = std::make_reverse_iterator<containers::List_array<Node, LA_node_size>
            ::iterator>(it_to);

        // 'std::for_each' dereferences the iterator, when input is given to the lambda function.
        // This means we dont dereference by  '->' but simply call '.' instead.
        std::for_each(rbegin, rend, [&](auto& some_node) {some_node.propagate_node();});
    }

    // Propagate from current Tdouble to iterator given 
    void propagate_to(Tape::iterator it_to)
    {
        propagate(tape->find(get_node()), it_to, true);
    }

public:
    //Propagate from current Tdouble to first Tdouble
    void propagate_to_start(){
        propagate_to(tape->begin());
    }

    //Propagate from current Tdouble to marked Tdouble
    void propagate_to_mark(){
        if (!tape->check_for_mark())
        {
            std::__throw_runtime_error("Tape is not marked!");
        }
        
        propagate_to(tape->marked_it());
    }

    //Propagate from marked Tdouble to first Tdouble
    void propagate_from_mark_to_start(){
        if (!tape->check_for_mark())
        {
            std::__throw_runtime_error("Tape is not marked!");
        }
        propagate(--tape->marked_it(), tape->begin(), false);
    }

// ---------------------------------------------------------------
// - OVERLOADING of operators etc.  
// ---------------------------------------------------------------

    // Overload of the +/- operators 
    Tdouble operator-() const { return 0.0 - *this;}
    Tdouble operator+() const { return *this;}

// ---------------------------------------------------------------
// - BOOLS
// ---------------------------------------------------------------
    // <=, >= 
    inline friend bool operator>=(const Tdouble& l_arg, const Tdouble& r_arg)
    {
        return l_arg.my_value >= r_arg.my_value; 
    }

    inline friend bool operator>=(const double& l_arg, const Tdouble& r_arg)
    {
        return l_arg >= r_arg.my_value; 
    }

    inline friend bool operator>=(const Tdouble& l_arg, const double& r_arg)
    {
        return r_arg <= l_arg.my_value; 
    }

    inline friend bool operator<=(const Tdouble& l_arg, const Tdouble& r_arg)
    {
        return l_arg.my_value <= r_arg.my_value; 
    }

    inline friend bool operator<=(const double& l_arg, const Tdouble& r_arg)
    {
        return l_arg <= r_arg.my_value; 
    }

    inline friend bool operator<=(const Tdouble& l_arg, const double& r_arg)
    {
        return r_arg >= l_arg.my_value; 
    }

// <, > 
    inline friend bool operator>(const Tdouble& l_arg, const Tdouble& r_arg)
    {
        return l_arg.my_value > r_arg.my_value; 
    }

    inline friend bool operator>(const double& l_arg, const Tdouble& r_arg)
    {
        return l_arg > r_arg.my_value; 
    }

    inline friend bool operator>(const Tdouble& l_arg, const double& r_arg)
    {
        return r_arg < l_arg.my_value; 
    }

    inline friend bool operator<(const Tdouble& l_arg, const Tdouble& r_arg)
    {
        return l_arg.my_value < r_arg.my_value; 
    }

    inline friend bool operator<(const double& l_arg, const Tdouble& r_arg)
    {
        return l_arg < r_arg.my_value; 
    }

    inline friend bool operator<(const Tdouble& l_arg, const double& r_arg)
    {
        return r_arg > l_arg.my_value; 
    }

// !=, == 
    inline friend bool operator!=(const Tdouble& l_arg, const Tdouble& r_arg)
    {
        return l_arg.my_value != r_arg.my_value; 
    }

    inline friend bool operator!=(const double& l_arg, const Tdouble& r_arg)
    {
        return l_arg != r_arg.my_value; 
    }

    inline friend bool operator!=(const Tdouble& l_arg, const double& r_arg)
    {
        return r_arg != l_arg.my_value; 
    }

    inline friend bool operator==(const Tdouble& l_arg, const Tdouble& r_arg)
    {
        return l_arg.my_value == r_arg.my_value; 
    }

    inline friend bool operator==(const double& l_arg, const Tdouble& r_arg)
    {
        return l_arg == r_arg.my_value; 
    }

    inline friend bool operator==(const Tdouble& l_arg, const double& r_arg)
    {
        return l_arg.my_value == r_arg; 
    }

// ---------------------------------------------------------------
// - BINARY OPERATORS
// ---------------------------------------------------------------
    inline friend Tdouble operator+(const Tdouble& l_arg, const Tdouble& r_arg)
    {
        const double val = l_arg.my_value + r_arg.my_value;
        Tdouble res_Tdouble(val, 2);

        res_Tdouble.weights()[0] = 1;
        res_Tdouble.weights()[1] = 1;
        res_Tdouble.child_adjoint()[0] = &l_arg.get_adjoint();
        res_Tdouble.child_adjoint()[1] = &r_arg.get_adjoint();

        return res_Tdouble;
    }

    inline friend Tdouble operator+(const Tdouble& l_arg, const double& r_arg)
    {
        const double val = l_arg.my_value + r_arg;
        Tdouble res_Tdouble(val, 1);

        res_Tdouble.weights()[0] = 1;
        res_Tdouble.child_adjoint()[0] = &l_arg.get_adjoint();
        return res_Tdouble;
    }

    inline friend Tdouble operator+(const double& l_arg, const Tdouble& r_arg)
    {
        
        return r_arg + l_arg;
    }

    inline friend Tdouble operator-(const Tdouble& l_arg, const Tdouble& r_arg)
    {
        const double val = l_arg.my_value - r_arg.my_value;
        Tdouble res_Tdouble(val, 2);

        res_Tdouble.weights()[0] = 1;
        res_Tdouble.weights()[1] = -1;
        res_Tdouble.child_adjoint()[0] = &l_arg.get_adjoint();
        res_Tdouble.child_adjoint()[1] = &r_arg.get_adjoint();

        return res_Tdouble;
    }

    inline friend Tdouble operator-(const Tdouble& l_arg, const double& r_arg)
    {
        const double val = l_arg.my_value - r_arg;
        Tdouble res_Tdouble(val, 1);

        res_Tdouble.weights()[0] = 1;
        res_Tdouble.child_adjoint()[0] = &l_arg.get_adjoint();

        return res_Tdouble;
    }

    inline friend Tdouble operator-(const double& l_arg, const Tdouble& r_arg)
    {
        const double val = l_arg - r_arg.my_value;
        Tdouble res_Tdouble(val, 1);

        res_Tdouble.weights()[0] = -1;
        res_Tdouble.child_adjoint()[0] = &r_arg.get_adjoint();

        return res_Tdouble;
    }

    // * 
    inline friend Tdouble operator*(const Tdouble& l_arg, const Tdouble& r_arg)
    {
        const double val = l_arg.my_value * r_arg.my_value;
        Tdouble res_Tdouble(val, 2);

        res_Tdouble.weights()[0] = r_arg.my_value;
        res_Tdouble.weights()[1] = l_arg.my_value;
        res_Tdouble.child_adjoint()[0] = &l_arg.get_adjoint();
        res_Tdouble.child_adjoint()[1] = &r_arg.get_adjoint();

        return res_Tdouble;
    }

    inline friend Tdouble operator*(const Tdouble& l_arg, const double& r_arg)
    {
        const double val = l_arg.my_value * r_arg;
        Tdouble res_Tdouble(val, 1);

        res_Tdouble.weights()[0] = r_arg;
        res_Tdouble.child_adjoint()[0] = &l_arg.get_adjoint();

        return res_Tdouble;
    }

    inline friend Tdouble operator*(const double& l_arg, const Tdouble& r_arg)
    {
        //Quick and dirty, derivs still double arg
        return r_arg * l_arg;
    }

    // / 
    inline friend Tdouble operator/(const Tdouble& l_arg, const Tdouble& r_arg)
    {
        const double val = l_arg.my_value / r_arg.my_value;
        Tdouble res_Tdouble(val, 2);

        res_Tdouble.weights()[0] = 1.0 / r_arg.my_value;
        res_Tdouble.weights()[1] = (-1.0)*(l_arg.my_value / (r_arg.my_value * r_arg.my_value));
        res_Tdouble.child_adjoint()[0] = &l_arg.get_adjoint();
        res_Tdouble.child_adjoint()[1] = &r_arg.get_adjoint();

        return res_Tdouble;
    }

    inline friend Tdouble operator/(const Tdouble& l_arg, const double& r_arg)
    {
        const double val = l_arg.my_value / r_arg;
        Tdouble res_Tdouble(val, 1);

        res_Tdouble.weights()[0] = 1.0 / r_arg;
        res_Tdouble.child_adjoint()[0] = &l_arg.get_adjoint();

        return res_Tdouble;
    }

    inline friend Tdouble operator/(const double& l_arg, const Tdouble& r_arg)
    {
        const double val = l_arg / r_arg.my_value;
        Tdouble res_Tdouble(val, 1);
        res_Tdouble.weights()[0] = (-1.0)*(l_arg / (r_arg.my_value * r_arg.my_value));
        res_Tdouble.child_adjoint()[0] = &r_arg.get_adjoint();

        return res_Tdouble;
    }

    // max 
    inline friend Tdouble max(const Tdouble& l_arg, const Tdouble& r_arg)
    {
        const bool left = l_arg.my_value > r_arg.my_value;
        Tdouble res_Tdouble(left ? l_arg.my_value : r_arg.my_value, 2);
        if (left)
        {
            res_Tdouble.weights()[0] = 1.0;
            res_Tdouble.weights()[1] = 0.;
            res_Tdouble.child_adjoint()[0] = &l_arg.get_adjoint();
            res_Tdouble.child_adjoint()[1] = &r_arg.get_adjoint();
        }
        else
        {
            res_Tdouble.weights()[0] = 0.;
            res_Tdouble.weights()[1] = 1.0;
            res_Tdouble.child_adjoint()[0] = &l_arg.get_adjoint();
            res_Tdouble.child_adjoint()[1] = &r_arg.get_adjoint();
        }
        return res_Tdouble;
    }

    inline friend Tdouble max(const double& l_arg, const Tdouble& r_arg)
    {
        const bool left = l_arg > r_arg.my_value;
        Tdouble res_Tdouble(left ? l_arg : r_arg.my_value, 1);
        if (left)
        {
            res_Tdouble.weights()[0] = 0.;
            res_Tdouble.child_adjoint()[0] = &r_arg.get_adjoint();
        }
        else
        {
            res_Tdouble.weights()[0] = 1.0;
            res_Tdouble.child_adjoint()[0] = &r_arg.get_adjoint();
        }
        return res_Tdouble;  
    }

    inline friend Tdouble max(const Tdouble& l_arg, const double& r_arg)
    {
        const bool left = l_arg.my_value > r_arg;
        Tdouble res_Tdouble( left ? l_arg.my_value : r_arg, 1);
        if (left)
        {
            res_Tdouble.weights()[0] = 1.0;
            res_Tdouble.child_adjoint()[0] = &l_arg.get_adjoint();
        }
        else
        {
            res_Tdouble.weights()[0] = 0.;
            res_Tdouble.child_adjoint()[0] = &l_arg.get_adjoint();
        }
        return res_Tdouble;
    }
    //min
    inline friend Tdouble min(const Tdouble& l_arg, const Tdouble& r_arg)
    {
        const bool left = l_arg.my_value < r_arg.my_value;
        Tdouble res_Tdouble(left ? l_arg.my_value : r_arg.my_value, 2);
        if (left)
        {
            res_Tdouble.weights()[0] = 1.0;
            res_Tdouble.weights()[1] = 0.;
            res_Tdouble.child_adjoint()[0] = &l_arg.get_adjoint();
            res_Tdouble.child_adjoint()[1] = &r_arg.get_adjoint();
        }
        else
        {
            res_Tdouble.weights()[0] = 0.;
            res_Tdouble.weights()[1] = 1.0;
            res_Tdouble.child_adjoint()[0] = &l_arg.get_adjoint();
            res_Tdouble.child_adjoint()[1] = &r_arg.get_adjoint();
        }
        return res_Tdouble;
    }
  
    inline friend Tdouble min(const double& l_arg, const Tdouble& r_arg)
    {
        const bool left = l_arg < r_arg.my_value;
        Tdouble res_Tdouble(left ? l_arg : r_arg.my_value, 1);
        if (left)
        {
            res_Tdouble.weights()[0] = 0.;
            res_Tdouble.child_adjoint()[0] = &r_arg.get_adjoint();
        }
        else
        {
            res_Tdouble.weights()[0] = 1.0;
            res_Tdouble.child_adjoint()[0] = &r_arg.get_adjoint();
        }
        return res_Tdouble;
    }

    inline friend Tdouble min(const Tdouble& l_arg, const double& r_arg)
    {
        const bool left = l_arg.my_value < r_arg;
        Tdouble res_Tdouble( left ? l_arg.my_value : r_arg, 1);
        if (left)
        {
            res_Tdouble.weights()[0] = 1.0;
            res_Tdouble.child_adjoint()[0] = &l_arg.get_adjoint();
        }
        else
        {
            res_Tdouble.weights()[0] = 0.;
            res_Tdouble.child_adjoint()[0] = &l_arg.get_adjoint();
        }
        return res_Tdouble;
    }

    // pow,
    inline friend Tdouble pow(const Tdouble& l_arg, const Tdouble& r_arg)
	{
		const double val = pow(l_arg.my_value, r_arg.my_value);
        Tdouble res_Tdouble(val, 2);

        res_Tdouble.weights()[0] = r_arg.my_value * val / l_arg.my_value;;
        res_Tdouble.weights()[1] = log(l_arg.my_value) * val;
        res_Tdouble.child_adjoint()[0] = &l_arg.get_adjoint();
        res_Tdouble.child_adjoint()[1] = &r_arg.get_adjoint();
		return res_Tdouble;
	}

    inline friend Tdouble pow(const Tdouble& l_arg, const double& r_arg)
	{
		const double val = pow(l_arg.my_value, r_arg);
        Tdouble res_Tdouble(val, 1);

        res_Tdouble.weights()[0] = r_arg * val / l_arg.my_value;;
        res_Tdouble.child_adjoint()[0] = &l_arg.get_adjoint();	
        return res_Tdouble;
	}

    inline friend Tdouble pow(const double& l_arg, const Tdouble& r_arg)
	{
		const double val = pow(l_arg, r_arg.my_value);		
        Tdouble res_Tdouble(val, 1);

        res_Tdouble.weights()[0] = log(l_arg) * val;
        res_Tdouble.child_adjoint()[0] = &r_arg.get_adjoint();
		return res_Tdouble;
	}
    

// ---------------------------------------------------------------
// - UNARY OPERATORS
// ---------------------------------------------------------------

    // +=, -=, *=, /=
    Tdouble& operator+=(const Tdouble& arg)
    {
        *this = *this + arg;
        return *this;
    }
    Tdouble& operator+=(const double& arg)
    {
        *this = *this + arg;
        return *this;
    }

    Tdouble& operator-=(const Tdouble& arg)
    {
        *this = *this - arg;
        return *this;
    }
    Tdouble& operator-=(const double& arg)
    {
        *this = *this - arg;
        return *this;
    }

    Tdouble& operator*=(const Tdouble& arg)
    {
        *this = *this * arg;
        return *this;
    }
    Tdouble& operator*=(const double& arg)
    {
        *this = *this * arg;
        return *this;
    }

    Tdouble& operator/=(const Tdouble& arg)
    {
        *this = *this / arg;
        return *this;
    }
    Tdouble& operator/=(const double& arg)
    {
        *this = *this / arg;
        return *this;
    }
 
    // sqrt
    inline friend Tdouble sqrt(const Tdouble& arg)
    {
        const double val = sqrt(arg.my_value);
        Tdouble res_Tdouble(val, 1);
        res_Tdouble.weights()[0] = 0.5 / val;
        res_Tdouble.child_adjoint()[0] = &arg.get_adjoint();

        return res_Tdouble;
    }
    
    // exp
    inline friend Tdouble exp(const Tdouble& arg)
    {
        const double val = exp(arg.my_value);
        Tdouble res_Tdouble(val, 1);

        res_Tdouble.weights()[0] = val;
        res_Tdouble.child_adjoint()[0] = &arg.get_adjoint();
        return res_Tdouble;
    }
    
    // log
    inline friend Tdouble log(const Tdouble& arg)
    {
        const double val = log(arg.my_value);
        Tdouble res_Tdouble(val, 1);

        res_Tdouble.weights()[0] = 1 / arg.my_value;
        res_Tdouble.child_adjoint()[0] = &arg.get_adjoint();
        return res_Tdouble;
    }

    // abs
    inline friend Tdouble abs(const Tdouble& arg)
    {
        const double val = abs(arg.my_value);
        Tdouble res_Tdouble(val, 1);

        res_Tdouble.weights()[0] = arg.my_value > 0 ? 1.0 : -1.0;
        res_Tdouble.child_adjoint()[0] = &arg.get_adjoint();
        return res_Tdouble;
    }
    
    inline friend Tdouble fabs(const Tdouble& arg)
    {
        const double val = fabs(arg.my_value);
        Tdouble res_Tdouble(val, 1);

        
        res_Tdouble.weights()[0] = arg.my_value > 0 ? 1.0 : -1.0;
        res_Tdouble.child_adjoint()[0] = &arg.get_adjoint();
        return res_Tdouble;
    } 

    inline friend Tdouble sin(const Tdouble& arg)
    {
        const double val = sin(arg.my_value);
        Tdouble res_Tdouble(val, 1);
        
        res_Tdouble.weights()[0] = cos(arg.my_value);
        res_Tdouble.child_adjoint()[0] = &arg.get_adjoint();
        return res_Tdouble;
    }

    inline friend Tdouble cos(const Tdouble& arg)
    {
        const double val = cos(arg.my_value);
        Tdouble res_Tdouble(val, 1);
        
        res_Tdouble.weights()[0] = -sin(arg.my_value);
        res_Tdouble.child_adjoint()[0] = &arg.get_adjoint();
        return res_Tdouble;
    }

    // utilized gaussians.hpp normalCDF
    inline friend Tdouble normalCdf(const Tdouble& arg)
    {
        const double val = gaussian::normalCdf(arg.my_value);
        Tdouble res_Tdouble(val, 1);
        res_Tdouble.weights()[0] = gaussian::normalDens(arg.my_value);
        res_Tdouble.child_adjoint()[0] = &arg.get_adjoint();

        return res_Tdouble;
    }

    // utilized gaussians.hpp normalDens
    inline friend Tdouble normalDens(const Tdouble& arg)
    {
        const double val = gaussian::normalDens(arg.my_value);
        Tdouble res_Tdouble(val, 1);

        res_Tdouble.weights()[0] = val*arg.my_value;
        res_Tdouble.child_adjoint()[0] = &arg.get_adjoint();
        return res_Tdouble;
    }

};
#endif
