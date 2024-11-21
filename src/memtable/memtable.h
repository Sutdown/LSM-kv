#include <memory>
#include <string>
#include <optional>
#include "memtable_config.h"
#include "op_type.h"
#include "../log/log.h"

#ifndef MEMTABLE_H
#define MEMTABLE_H

namespace lsmkv
{
  template <typename Key, typename Value>
  class SkipList;

  class FreeListAllocate;

  class MemTable final
  {
  public:
    explicit MemTable(std::shared_ptr<FreeListAllocate> alloc);
    ~MemTable() = default;
    MemTable() = delete;

    MemTable(const MemTable &) = delete;
    MemTable &operator=(const MemTable &) = delete;

    inline void Add(const std::string_view &key, const std::string_view &value)
    {
      this->Insert(OpType::KAdd, key, value);
    }

    inline void Update(const std::string_view &key, const std::string_view &value)
    {
      this->Insert(OpType::KUpdate, key, value);
    }

    inline void Delete(const std::string_view &key, const std::string_view &value)
    {
      this->Insert(OpType::KDeletion, key, value);
    }

    bool Contains(const std::string_view &key);

    std::optional<std::string> Get(const std::string_view &key);

  private:
    // add,update,delete都属于insert
    // delete时，value为空，kv存储时的删除不是挨个删除，而是最终一起遍历
    void Insert(OpType op_type, const std::string_view &key, const std::string_view &value);

  private:
    std::shared_ptr<SkipList<std::string, std::string>> ordered_table;
    std::shared_ptr<FreeListAllocate> alloc;
    std::shared_ptr<spdlog::logger> logger;
  };
}
#endif