#include <utility>

#include "db_impl.h"
#include "../cache/cache.h"
#include "../utils/codec.h"
#include "../memory/allocate.h"
#include "../memtable/memtable.h"
#include "../wal/wal_writer.h"
#include "../file/file_writer.h"
#include "../table/sstable_builder.h"
#include "../sst_parser/sst_parser.h"

namespace lsmkv{
  // public
  DBImpl::DBImpl(Options options):options_(std::move(options)){
    alloc = std::make_shared<FreeListAllocate>();
    mem_table = std::make_shared<MemTable>(alloc);
    logger = log::get_logger();

    // 创建WAL
    wal_writer = std::make_shared<WALWriter>(std::make_shared<FileWriter>(options.WAL_DIR));

    cache = std::make_shared<Cache<std::string, std::string>>(options_.CACHE_SIZE);
    cache->register_clean_handle([](const std::string &key, std::string *val)
                                 { delete val; });
  }

  /*
   * 写逻辑：
   * 1. 写WAL(fsync同步);
   * 2. 写memtable;
   * 3. 写缓存(提高读性能);
   * 4. 如果memtable超限，应该落盘，并且开启一个新的memtable;
   *
   * */
  DBStatus DBImpl::Put(const WriteOptions &options,
               const std::string_view &key,
               const std::string_view &value){
    assert(closed == false);
    std::unique_lock<std::shared_mutex> wlock(rwlock_);

    // 1.wal
    char buf[8 + key.size()];
    EncodeKV(key, value, buf);
    wal_writer->ADDLog(buf);

    // 2.memtable
    if(mem_table->Contains(key)){
      mem_table->Update(key, value);
    }else{
      mem_table->Add(key, value);
    }

    // 3.写缓存
    cache->insert(key.data(),new std::string(value.data()));

    // 4.观察memtable是否超限
    if (mem_table->GetMemUsage() >= options_.MEM_TABLE_MAM_SIZE) {
      MemTableToSST(); // 将memtable转为sst

      // 开启写的memtable
      mem_table = std::make_shared<MemTable>(alloc);
      logger->info("[DBImpl::Put] A new mem_table is created.");
    }
    return Status::Success;
  }

  /*
   * 删除逻辑：
   * 1. 写WAL;
   * 2. 写memtable;
   * 3. 删除缓存;
   * 4. 如果memtable超限，应该落盘，并且开启一个新的memtable;
   * */
  DBStatus DBImpl::Delete(const WriteOptions &options,
                  const std::string_view &key){
    assert(closed == false);
    std::unique_lock<std::shared_mutex> wlock(rwlock_);

    // 1. 写WAL
    char buf[8 + key.size()]; // 用vel_len=0表示val为空
    EncodeKV(key, "", buf);
    wal_writer->ADDLog(buf);

    // 2. 写memtable
    if (mem_table->Contains(key))
    { // 原地标记val=""表示删除
      mem_table->Delete(key);
    }
    else
    {
      mem_table->Add(key, ""); // 墓碑机制
    }

    // 3. 删除缓存
    cache->erase(key.data());

    // 4. 检查memtable是否超限
    if (mem_table->GetMemUsage() >= options_.MEM_TABLE_MAM_SIZE)
    {
      MemTableToSST(); // 将memtable转为sst

      // 开启写的memtable
      mem_table = std::make_shared<MemTable>(alloc);
      logger->info("[DBImpl::Delete] A new mem_table is created.");
    }
    return Status::Success;
  }

  /*
   * 读逻辑：
   * 1. 读cache，有则直接返回，否则进入2;
   * 2. 依次从memtable、sst文件向下查找;
   * 3. 找到的数据写入缓存;
   * 4. 返回结果;
   *
   * */
  DBStatus DBImpl::Get(const ReadOptions &options,
               const std::string_view &key,
               std::string *ret_value_ptr){
    // 1. 读cache
    if (cache->contains(key.data()))
    {
      *ret_value_ptr = *(cache->get(key.data())->val);
      logger->info("Cache hit.");
      return Status::Success;
    }

    // 2. 读memtable
    if (mem_table->Contains(key))
    {
      auto val = mem_table->Get(key);
      *ret_value_ptr = mem_table->Get(key.data()).value();
      return Status::Success;
    }

    // 3. 依次读sst文件
    // todo: 实际上需要从manifest获取sst文件，此处直接硬编码一个文件: level_0_sst_0.sst
    std::string sst_path = options_.STORAGE_DIR + "level_0_sst_0.sst";
    auto sst_parser = SSTParser(sst_path);
    sst_parser.Parser();
    auto val = sst_parser.Seek(key);
    if (!val.has_value())
    {
      return Status::NotFound;
    }
    ret_value_ptr->append(val.value());

    // 4. 找到的数据写入缓存
    // 这里cache保存value指针，然而val可能是临时值，临时new 一下，性能有待加强
    auto val_ = new std::string(val.value());
    cache->insert(key.data(), val_);

    return Status::Success;
  }

  DBStatus DBImpl::Close(){
    if (!closed && mem_table->GetSize() > 0) {
      // memtable中有数据，就应该落盘
      MemTableToSST();

      closed = true;
    }
    logger->info("DB is closed.");
    return Status::Success;
  }

  DBStatus DBImpl::BatchPut(const WriteOptions &options){
    std::unique_lock<std::shared_mutex> wlock(rwlock_);
    assert(closed == false);
    // todo: 稍后实现
    return Status::NotImpl;
  }

  DBStatus DBImpl::BatchDelete(const ReadOptions &options){
    std::unique_lock<std::shared_mutex> wlock(rwlock_);
    assert(closed == false);
    // todo: 稍后实现
    return Status::NotImpl;
  }

  // private
  void DBImpl::EncodeKV(const std::string_view &key,
                        const std::string_view &value,
                        char *buf){
    /*| key_len(4B) | key | val_len(4B) | val |*/
    assert(value.size() < UINT32_MAX);
    utils::EncodeFixed32(buf, key.size());
    memcpy(buf + 4, key.data(), key.size());

    utils::EncodeFixed32(buf + 4 + key.size(), value.size());
    memcpy(buf + 4 + key.size() + 4, value.data(),value.size());
  }

  void DBImpl::MemTableToSST(){
    // 同步
    // 建立sst文件路径
    auto sst_filepath = options_.STORAGE_DIR + "/" + utils::BuildSSTPath(0, options_.LISST_NUM);
    logger->info("DBImpl::MemTableToSST() is called. sst_filepath={}", sst_filepath);

    // 将路径作为参数创建文件写类
    auto file_writer = std::make_shared<FileWriter>(sst_filepath);
    // 将memtable中的输入写入文件写，同时创建ssttable
    auto sstable_builder = std::make_shared<SSTableBuilder>(mem_table->GetSize(), file_writer);
    // 将memtable中的数据sstable_builder，写入到sst_filepath中
    mem_table->ConvertToL1SST(sst_filepath, sstable_builder);

    ++options_.LISST_NUM; // 下一个sst文件序号+1
  }
}