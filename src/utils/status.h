#include <cstdint>

#ifndef STATUS_H
#define STATUS_H

namespace lsmkv
{
    struct DBStatus
    {
        int32_t code;
        const char *err_msg;
    };

    bool operator==(const DBStatus &a, const DBStatus &b);
    bool operator!=(const DBStatus &a, const DBStatus &b);

    struct Status
    {
        Status() = delete;
        ~Status() = delete;

        static constexpr DBStatus Success = {1, "Success"};
        static constexpr DBStatus InvalidArgs = {2, "Invalid args"};
        static constexpr DBStatus ExecFailed = {3, "Exex Failed"};
    };
}

#endif