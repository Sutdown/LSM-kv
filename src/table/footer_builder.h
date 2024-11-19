#include <string>
#include "../log/log.h"
#include "../db/offset_info.h"
#include "../db/status.h"

#ifndef FOOTER_BUILDER_H
#define FOOTER_BUILDER_H

namespace lsmkv
{
    class FooterBuilder final
    {
    private:
        // metablock offsetinfo和indexblock offsetinfo
        std::string _data;
        std::shared_ptr<spdlog::logger> logger = log::get_instance();

    public:
        FooterBuilder() = default;
        ~FooterBuilder() = default;

        DBStatus add(const OffsetInfo &meta_block_offset_info, const OffsetInfo &index_block_offset_info);

        inline void clear() { _data.clear(); }

        inline std::string_view data() { return _data; }
    };
}
#endif