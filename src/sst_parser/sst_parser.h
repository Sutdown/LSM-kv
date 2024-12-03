#include <vector>
#include <memory>
#include <fstream>
#include <filesystem>
#include <optional>
#include "header.h"
#include "index_block.h"
#include "../log/log.h"

#ifndef SST_PARSER_H
#define SST_PARSER_H

namespace lsmkv{
  /*sst文件解析器*/
  namespace fs = std::filesystem;
  class FilterPolicy;

  /*
  SSTParser()-->Parser()-->Seek(key)
  */

 class SSTParser{
  public:
    // 传入文件路径
    explicit SSTParser(const std::string &sst_filepath);
    // 解析sst文件
    void Parser();
    // 查找
    std::optional<std::string> Seek(const std::string_view &key);
  private:
    // 找到key所在的data block的序号，从0开始，没找到返回-1
    int FindKeyDBIndex(const std::string_view &key);

    // 在datablock中二分查找key，找到返回value，没找到返回nullptr
    std::optional<std::string> FindKeyInDB(const std::string_view &key, char *START, char *END);

    // 根据重启点找到对应Record Group的第一个fullkey
    std::string FindFullKeyByRP(char *START, int record_num, const OffsetInfo &rg_offset);

    // 读取全部内容到sst_content中
    void ReadALL();

    // filter只能判断false一定准确
    bool Exists(const std::string_view &key);

    private:
      std::shared_ptr<FilterPolicy> filterPolicy;
      std::vector<Index_block> index_blocks;
      std::ifstream is;

      std::string sst_content;
      uint32_t FILE_SEGMENT = 4 * 1024 * 1024;

      Header header;

      std::shared_ptr<spdlog::logger> logger;
 };
}

#endif