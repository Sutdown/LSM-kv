#include "db.h"
#include "db_impl.h"

namespace lsmkv {
  DB::DB(const Options &options) {
    db_impl = std::make_unique<DBImpl>(options);
  }

  DBStatus DB::Put(const WriteOptions &options,
                   const std::string_view &key,
                   const std::string_view &value) {
    return db_impl->Put(options, key, value);
  }

  DBStatus DB::Delete(const WriteOptions &options,
                      const std::string_view &key) {
    return db_impl->Delete(options, key);
  }

  DBStatus DB::Get(const ReadOptions &options,
                   const std::string_view &key,
                   std::string *ret_value_ptr) {
    return db_impl->Get(options, key, ret_value_ptr);
  }

  DBStatus DB::BatchPut(const WriteOptions &options) {
    return db_impl->BatchPut(options);
  }

  DBStatus DB::BatchDelete(const ReadOptions &options) {
    return db_impl->BatchDelete(options);
  }

  DBStatus DB::Close() {
    return db_impl->Close();
  }
}
