#include "status.h"

namespace lsmkv
{
    bool operator==(const DBStatus &a, const DBStatus &b)
    {
        return a.code == b.code;
    }
    bool operator!=(const DBStatus &a, const DBStatus &b)
    {
        return a.code != b.code;
    }
}