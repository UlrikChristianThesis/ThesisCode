#ifndef LIST_ARRAY_HPP
#define LIST_ARRAY_HPP

// STL includes
#include <array>
#include <list>
#include <iterator>
#include <algorithm>

namespace containers
{
    template<typename T, size_t SIZE>
    class List_array
    {
    private:
        // main container
        std::list<std::array<T, SIZE>> container;

        // iterator types inferred using decltype
        using list_iterator  = decltype(container.begin());
        using array_iterator = decltype(container.front().begin());

        // iterators
        // current array
        list_iterator  current_array;
        // last array in list
        list_iterator last_array;
        // current entry in current_array
        array_iterator current_entry;
        // last entry in the current_array
        array_iterator last_entry;

        // marked iterator
        list_iterator mark_array;
        // marked entry in array
        array_iterator mark_entry;
    
        // Adds new array to the list. 
        void extend_list()
        {
            container.emplace_back();
            current_array = std::prev(container.end());
            last_array    = current_array;
            current_entry = current_array->begin();
            last_entry    = current_array->end();
        }

        // moves to the next array
        void next_array()
        {
            if(current_array == last_array)
            {
                extend_list();
            }
            else
            {
                ++current_array;
                current_entry = current_array->begin();
                last_entry    = current_array->end();
            }
        }

    public:
        bool marked = false;
        // CTOR: Immediately construct an array in the list and set List_array iterators. 
        List_array()
        {
            extend_list();
        }

        void set_mark()
        {
            marked = true;
            mark_array = current_array;
            mark_entry = current_entry;
        }

        void go_to_mark()
        {
            if (!marked) {std::__throw_runtime_error("Tape not marked, cannot call 'List_array::go_to_mark()'");}
            current_array = mark_array;
            current_entry = mark_entry;
            last_entry    = current_array->end();
        }

        // places object in next entry and returns pointer to this
        template<typename ...Args>
        T* emplace_back(Args&& ...args)
        {
            // need new array?
            if(current_entry == last_entry)
            {
                next_array();
            }
            // https://en.cppreference.com/w/cpp/utility/forward
            // In-place construction using new
            T* object = new (&*current_entry) T(std::forward<Args>(args)...);
            ++current_entry;

            return object;
        }

        template<size_t n>
        T* emplace_back_n()
        {
            if(std::distance(current_entry, last_entry) < n)
            {
                next_array();
            }

            auto prev_entry = current_entry;
            current_entry += n;

            return &*prev_entry;
        }
        
        T* emplace_back_n(const size_t n)
        {
            if((const unsigned long)(std::distance(current_entry, last_entry)) < n)
            {
                next_array();
            }

            auto prev_entry = current_entry;
            current_entry  += n;
            
            return &*prev_entry;
        }

        ~List_array() {}


        //print marked
        void print_mark()
        {
            std::cout << &*mark_array << std::endl;
            std::cout << &*mark_entry << std::endl;
        }


        // nested ITERATOR class 
        class iterator
        {
            list_iterator  current_array;
            array_iterator current_entry;
            array_iterator first_entry;
            array_iterator last_entry;

        public:
            // https://en.cppreference.com/w/cpp/iterator/iterator_traits
            // see section "Specializations" -> "T* specialization member types"
            // needed to use STL algorithms such as distance, for_each, etc...
            using difference_type = std::ptrdiff_t;
            using value_type = T;
            using pointer = T*;
            using reference = T&;
            using iterator_category = std::bidirectional_iterator_tag;

            iterator() {}
            iterator(list_iterator current_array_, array_iterator current_entry_,
                     array_iterator first_entry_, array_iterator last_entry_)
            : current_array(current_array_), current_entry(current_entry_),
              first_entry(first_entry_), last_entry(last_entry_) {};

            // prefix increment operator
            iterator operator++()
            {
                // increment to next entry
                ++current_entry;
                // check if increment to next array
                if(current_entry == last_entry)
                {
                    ++current_array;
                    // set iterators after incremented to next array
                    first_entry = current_array->begin();
                    last_entry = current_array->end();
                    current_entry = first_entry;
                }
                return *this;
            }

            // prefix decrement operator
            iterator operator--()
            {
                // check if decrement to previous array
                if(current_entry == first_entry)
                {
                    --current_array;
                    // sets iterators after decremented to previous array
                    first_entry = current_array->begin();
                    last_entry = current_array->end();
                    current_entry = last_entry;
                }
                // decrement current entry in any case
                // in the case the if statement was true current entry point to past-the-end element
                // else current entry should also be decremented
                --current_entry;

                return *this;
            }

            // access operators
            T& operator*()
            {
                return *current_entry;
            }

            const T& operator*() const
            {
                return *current_entry;
            }

            T* operator->()
            {
                return &*current_entry;
            }
            
            const T* operator->() const
            {
                return &*current_entry;
            }

            // comparison operators
            bool operator==(const iterator& rhs) const
            {
                return current_entry == rhs.current_entry;
            }
            
            bool operator!=(const iterator& rhs) const
            {
                return current_entry != rhs.current_entry;
            }
            
        };

        iterator begin()
        {
            auto list_begin = container.begin(); // first array of list
            auto first_entry = list_begin->begin(); // first entry of first array of list
            auto last_entry = list_begin->end(); // last entry(+1) of first array of list
            // return iterator with set state
            return iterator(list_begin, first_entry, first_entry, last_entry);
        }
        
        iterator end()
        {
            auto list_end = current_array;
            auto first_entry = list_end->begin(); // first entry of current array
            auto last_entry = list_end->end(); // last entry of current array
            // return iterator with set state where current_entry is used.
            return iterator(list_end, current_entry, first_entry, last_entry);
        }

        iterator mark()
        {
            auto marked_begin = mark_array;
            auto marked_first_entry = marked_begin->begin();
            auto marked_last_entry = marked_begin->end();
            return iterator(marked_begin, mark_entry, marked_first_entry, marked_last_entry);
        }

        iterator find(const T& f_node)
        {
            return std::find(begin(), end(), f_node);
        }

        void clear()
        {
            container.clear();
            next_array();
        }
    };
}

#endif
