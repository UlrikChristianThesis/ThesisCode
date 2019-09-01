#ifndef MYNODE_HPP
#define MYNODE_HPP

#include<iostream>
// #include<math.h>

class Node
{
private:
    // Number of children 
    size_t   n;
    // Adjoint value 
    double   my_adjoint = 0;
    // Pointer to Node's corresponding Tdouble partial derivatives (weights) 
    double*  my_weights;
    // Pointer to the address of childrens adjoints 
    double** my_child_adj;
    
public:
    // CTORs, DTOR
    Node() {} // Default: needed for array default constructor
    Node(size_t n_) : n(n_) {}
    ~Node() {}

    // getters
    double&      get_adjoint() {return my_adjoint;}
    double*&     get_weights() {return my_weights;}
    double**&    get_child_adjoint() {return my_child_adj;}

    // Helper for propagating in Tdouble Class 
    void set_adjoint_to_1() {my_adjoint = 1;}

    // Propagate node 
    void propagate_node()
    {
        // If no childs or adjoint = 0, skip
        if(!my_adjoint || !n) {return;}

        for (size_t i = 0; i < n; i++)
        {
            *my_child_adj[i] += my_adjoint * my_weights[i];
        }
    }

    // Needed for the stl::find for our iterators 
    bool operator==(const Node& rhs)
    {
        return &*this == &rhs;
    }
    bool operator==(const Node& rhs) const
    {
        return &*this == &rhs;
    }
};
#endif
