#include "skiplist.h"
#include "memtable.h"
#include "../utils/codec.h"
#include "../log/log.h"

namespace lsmkv
{
  MemTable::MemTable(std::shared_ptr<FreeListAllocate> alloc) : alloc(std::move(alloc))
  {
    ordered_table = std::make_shared<SkipList<std::string, std::string>>(alloc);
    logger = log::get_logger();
  }

  void MemTable::Insert(OpType op_type, const std::string_view &key, const std::string_view &value)
  {
    if(op_type==OpType::KUpdate) {
      ordered_table->Insert(key.data(), value.data());
    } else if(op_type==OpType::KUpdate) {
      // 最好直接在skiplist中找到相应键操作，而不是del-insert
      ordered_table->Delete(key.data());
      ordered_table->Insert(key.data(), value.data());
    } else if(op_type==OpType::KDeletion) {
      // 能达到效果，但是效率不佳
      if(ordered_table->Contains(key.data())){
        ordered_table->Delete(key.data());
      }
      ordered_table->Insert(key.data(), "");
    } else {
      logger->error("Unexpected op_type. op_type={}", op_type);
      return;
    }
  }

  int64_t MemTable::GetMemUsage()
  {
    return ordered_table->GetMemUsage();
  }

  int64_t MemTable::GetSize()
  {
    return ordered_table->GetSize();
  }

  bool MemTable::Contains(const std::string_view &key) {
    return ordered_table->Contains(key.data());
  }

  std::optional<std::string> MemTable::Get(const std::string_view &key) {
    return ordered_table->Get(key.data());
  }
}
