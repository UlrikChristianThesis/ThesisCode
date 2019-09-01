#include "Tape.hpp"
#include "Tdouble.hpp"

// globaltape_t is instantiated and assigned to the Tdouble class
Tape globaltape_t; 
Tape* Tdouble::tape = &globaltape_t; 
