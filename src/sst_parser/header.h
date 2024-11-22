#include "../db/offset_info.h"

#ifndef HEADER_H
#define HEADER_H

namespace lsmkv{
  struct Header
  {
    OffsetInfo MetaBlock_OffsetInfo;
    OffsetInfo IndexBlock_OffsetInfo;
  };
}

#endif