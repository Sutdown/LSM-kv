#include <string>

#ifndef OPTIONS_H
#define OPTIONS_H

namespace lsmkv {
  struct Options{
    /*DB的配置信息*/
    std::string DB_DIR = "./ouptput/db_storage"; // 数据库存储目录
    std::string STORAGE_DIR = "./output/wal_log.txt"; // wal log

    std::string WAL_DIR = "./output/wal_log.txt";

    size_t MEM_TABLE_MAM_SIZE = 4 * 1024 * 1024; // max memtable size
    uint32_t CACHE_SIZE = 4096; // key-value numbers in cache

    uint32_t LISST_NUM = 0;
  };

  inline Options MakeOptionsForDebugging() {
    return Options{};
  }

  inline Options MakeOptionsForProduction() {
    return Options{};
  }

  // read options
  struct ReadOptions{
  };

  // write options
  struct WriteOptions
  {
    /* data */
    /*
     * 注：C库缓冲 --fflush--> 内核缓冲 --fsync--> 磁盘
     * 解释：
     * 1. fsync系统调用可以强制每次写入都被更新到磁盘中，在open()中添加O_SYNC也由此效果；
     * 2. fflush是一个在C语言标准输入输出库中的函数，功能是冲洗流中的信息，该函数通常用于
     *    处理磁盘文件。fflush()会强迫将缓冲区内的数据写回参数stream 指定的文件中。
     * 一般地，fsync也不能保证100%安全，因为现在的磁盘也有缓存（比如固态硬盘可能有外置DRAM缓存），
     * 如果断电数据也可能会丢失。但是企业级硬盘一般有备用电源，并且很多固态的缓存是用的SLC颗粒(断电不丢失)，
     * 所以基本可以认为fsync可以保证数据安全。
     *
     * */
    // 此处的flush和fflush语义相同，实际上flush不需要设置为true，因为WAL已经保证了数据安全(fsync)。
    bool Flush = false;
  };
}

#endif