#include "skiplist.h"
#include "memtable.h"
#include "../table/sstable_builder.h"
#include "../utils/codec.h"
#include "../log/log.h"
#include "memtable_iterator.h"

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

  MemTableIterator *MemTable::NewIter()
  {
    return new MemTableIterator(this->ordered_table.get());
  }

  bool MemTable::Contains(const std::string_view &key) {
    return ordered_table->Contains(key.data());
  }

  std::optional<std::string> MemTable::Get(const std::string_view &key) {
    return ordered_table->Get(key.data());
  }

  void MemTable::ConvertToL1SST(const std::string &sst_filepath,
                                std::shared_ptr<SSTableBuilder> sstable_builder){
    // todo: 这里可能需要加锁。
    auto iter = NewIter();
    iter->MoveToFirst(); // 指向表头
    while (iter->Valid())
    {
      sstable_builder->add(iter->key(), iter->value());
      iter->Next();
    }
    logger->info("The L1 SST file is built.");

    // todo：后续需要改为异步落盘
    sstable_builder->finish_sst(); // sst文件写到磁盘
  }
}
