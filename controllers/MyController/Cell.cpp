//
// Created by mohamad on 6/28/2017.
//

#include "Cell.h"

std::ostream &operator<<(std::ostream &out, const Cell &rhs) {
    out << '<' << rhs.first << ", " << rhs.second << '>';
    return out;
}
