#include "../db/offset_info.h"
#include <string>

#ifndef RESTART_POINT_H
#define RESTART_POINT_H

namespace lsmkv{
  struct RestartPoint{
    int record_num = 0;
    OffsetInfo rp_offset;
    std::string fullkey;
  };
}

#endif