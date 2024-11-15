#include <string>

#ifndef COMPARATOR_H
#define COMPARATOR_H

namespace lsmkv
{
    class Comparator
    {
    public:
        Comparator() = default;
        virtual ~Comparator() = 0;

        virtual std::string name() = 0;
        virtual int32_t compare(const std::string &key_a, const std::string &key_b) = 0;
    };

    class ByteComparator final : public Comparator
    {
    public:
        ByteComparator() = default;
        ~ByteComparator() override = default;

        std::string name() override { return "ByteComparator"; };

        int32_t compare(const std::string &key_a, const std::string &key_b) override
        {
            return key_a.compare(key_b);
        }
    };
}

#endif