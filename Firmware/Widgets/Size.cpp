#include "Size.h"

std::ostream& operator<<(std::ostream& os, const Size& s)
{
    os << '{' << s._w << 'x' << s._h << '}';

    return os;
}