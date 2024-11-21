#include "skiplist.h"
#include "memtable.h"
#include "../table/sstable_builder.h"

#ifndef MEMTABLE_ITERATOR_H
#define MEMTABLE_ITERATOR_H
namespace lsmkv
{
  // 主要用于迭代遍历MemTable
  class MemTableIterator final
  {
  private:
    using SKIter = SkipList<std::string, std::string>::SkipListIterator;

    std::shared_ptr<SKIter> iter_;

  public:
    explicit MemTableIterator(SkipList<std::string, std::string> *list);

    // 将当前node移到表头
    // 必须要先调用此函数才可以进行迭代
    void MoveToFirst();

    void Next();

    const std::string &key();

    const std::string &value();

    // 判断当前iter指向的位置是否有效
    bool Valid();
  };
}
#endif