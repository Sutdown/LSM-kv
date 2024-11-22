#include <stdint.h>
#include <string>
#include "../db/offset_info.h"

#ifndef INDEX_BLOCK_H
#define INDEX_BLOCK_H

namespace lsmkv{
  struct Index_block
  {
    std::string _shortest_key;
    OffsetInfo offsetInfo;
  };
}

#endif