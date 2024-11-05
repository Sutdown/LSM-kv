#include <cstdint>

#ifndef MURMUR_HASH2_H
#define MURMUR_HASH2_H

namespace lsmkv::utils
{
    uint32_t murmur_hash2(const void *key, uint32_t len);
}

#endif