#ifndef OP_TYPE_H
#define OP_TYPE_H

namespace lsmkv
{
    enum OpType
    {
        KDeletion = 0x1,
        KAdd = 0x2,
        KUpdate = 0x3,
    };
}

#endif