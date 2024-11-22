#include <memory>
#include <string_view>
#include <shared_mutex>
#include "status.h"
#include "options.h"
#include "../log/log.h"

#ifndef DB_IMPL_H
#define DB_IMPL_H

namespace lsmkv{
  template <typename K, typename V>
  class Cache;

  class MemTable;
  class WALWriter;
  class FreeListAllocate;

  /* 支持并发，线程安全 */
  class DBImpl{
    public:
      explicit DBImpl(Options options);
      ~DBImpl() = default;

      DBStatus Put(const WriteOptions &options,
                   const std::string_view &key,
                   const std::string_view &value);
                   
      DBStatus Delete(const WriteOptions &options,
                      const std::string_view &key);

      DBStatus Get(const ReadOptions &options,
                   const std::string_view &key,
                   std::string *ret_value_ptr);

      DBStatus Close();

      DBStatus BatchPut(const WriteOptions &options);
      DBStatus BatchDelete(const ReadOptions &options);

    private:
      // 将KV编码到buf中，确保buf长度为8 + key.size() + value.size()
      static void EncodeKV(const std::string_view &key, const std::string_view &value, char *buf);
      // 将memtable转成sst
      void MemTableToSST();

    private:
      std::shared_ptr<MemTable> mem_table; // active memtable
      std::shared_ptr<spdlog::logger> logger; // log
      std::shared_ptr<FreeListAllocate> alloc; // memory allocator
      std::shared_ptr<WALWriter> wal_writer; // write ahead log
      std::shared_ptr<Cache<std::string, std::string>> cache; // cache

      Options options_; // option

      /*读写锁，可多个线程一起读，但是写时不允许其它线程读或者写*/
      std::shared_mutex rwlock_;

      bool closed = false; // database status
  };
}

#endif