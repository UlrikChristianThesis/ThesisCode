#ifndef TAPE_HPP
#define TAPE_HPP

#include<vector>
#include<memory>

#include "Node.hpp"
#include "List_array.hpp"

#define LA_node_size    40000
#define LA_dou_size     80000
#define LA_douptr_size  80000

class Tape
{
private:    
    containers::List_array<Node, LA_node_size>      my_nodes;
    containers::List_array<double, LA_dou_size>     my_nodes_weights;
    containers::List_array<double*, LA_douptr_size> my_child_adjoints;

public:
    Tape() {}
    ~Tape() {}

    Node* record_node(size_t some_n)
    {
	// in-place construction of node
        Node* res_node = my_nodes.emplace_back<Node>(some_n);

        if (some_n)
        {
	    // request space in the containers
            res_node->get_weights() = my_nodes_weights.emplace_back_n(some_n);
            res_node->get_child_adjoint() = my_child_adjoints.emplace_back_n(some_n);
        }

        return res_node;
    }

    containers::List_array<Node, LA_node_size>& get_nodes(){return my_nodes;}

    using iterator = containers::List_array<Node, LA_node_size>::iterator;

    iterator begin()
    {
        return my_nodes.begin();
    }

    iterator end()
    {
        return my_nodes.end();
    }

    iterator marked_it()
    {
        return my_nodes.mark();
    }

    void mark_tape()
    {
        my_nodes.set_mark();
        my_nodes_weights.set_mark();
        my_child_adjoints.set_mark();
    }

    void set_to_mark()
    {
        my_nodes.go_to_mark();
        my_nodes_weights.go_to_mark();
        my_child_adjoints.go_to_mark();
    }

    bool check_for_mark()
    {
        return my_nodes.marked;
    }
    
    iterator find(Node& f_node)
    {
        return my_nodes.find(f_node);
    }

    void clear()
    {
        my_nodes.clear();
        my_child_adjoints.clear();
        my_nodes_weights.clear();

    }
};

#endif
