#include <optional>
#include <cassert>
#include "sst_parser.h"
#include "../filter/bloom_filter.h"
#include "../filter/filter_policy.h"
#include "../utils/codec.h"
#include "restart_point.h"

namespace lsmkv{
  // public
  SSTParser::SSTParser(const std::string &sst_filepath){
    fs::path path = sst_filepath;
    assert(fs::exists(path));
    is = path;
    logger = log::get_logger();
  }

  void SSTParser::Parser(){

  }

  std::optional<std::string> SSTParser::Seek(const std::string_view &key){

  }

  // private
  int SSTParser::FindKeyDBIndex(const std::string_view &key) {
    int index = -1;
    for (int i = 0; i < index_blocks.size();++i){
      if(key < index_blocks[i]._shortest_key){
        index = i;
        break;
      }
    }
    return index;
  }

  std::optional<std::string> SSTParser::FindKeyInDB(const std::string_view &key, char *START, char *END) {
    /* 从datablock中找到重启点数目和偏移量 */
    const int Restart_Num = static_cast<int>(utils::DecodeFixed32(END - 12));
    auto a_ = utils::DecodeFixed64(END - 8);
    OffsetInfo Restart_Offset = *reinterpret_cast<OffsetInfo *>(&a_);

    /* 解析单个重启点,记录之后的数量和开始的偏移量 */
    auto parse_rp = [](char *rp_start_ptr)
    {
      int record_num = static_cast<int>(utils::DecodeFixed32(rp_start_ptr));
      auto _a = utils::DecodeFixed64(rp_start_ptr + 4);
      OffsetInfo rp_offset = *reinterpret_cast<OffsetInfo *>(&_a);
      return std::make_tuple(record_num, rp_offset);
    };

    /* 查找键位于哪个重启点之后 */
    std::vector<RestartPoint> rps;
    for (int i = 0; i < Restart_Num;++i) {
      // 每个重启点12字节
      const auto &[record_num, rp_offset] = parse_rp(START + Restart_Offset.offset + i * 12);
      auto fullkey = FindFullKeyByRP(START, record_num, rp_offset);
      if(key<fullkey){
        // 由于是顺序排列的，当出现fullkey大于key时，说明key一定在这个重启点之后
        break;
      }
      RestartPoint rp{record_num, rp_offset, fullkey};
      rps.push_back(std::move(rp));
    }

    if(rps.empty()){
      logger->info("A misjudgment occurred by filter.");
      return std::nullopt;
    }

    // rps最后一个RestartPoint就是key所在的位置
    const auto &RP = rps.back();
    const std::string_view FULLKEY = RP.fullkey;
    assert(key >= FULLKEY);
    // 顺序遍历该 RG
    std::string value;
    char *RG_START = START + RP.rp_offset.offset;

    /* 逐一查找该重启点之后的所有键 */
    // 解析record schema, ptr是该record的起始地址
    auto parse_record = [](char *ptr)
    {
      int shared_key_len = static_cast<int>(utils::DecodeFixed32(ptr));
      int unshared_key_len = static_cast<int>(utils::DecodeFixed32(ptr));
      int value_len = static_cast<int>(utils::DecodeFixed32(ptr));
      int record_size = 12 + unshared_key_len + value_len;

      std::string unshared_key_content, value_content;
      unshared_key_content = std::string(ptr + 12, unshared_key_len);
      value_content = std::string(ptr + 12 + unshared_key_len, value_len);
      return std::make_tuple(record_size, shared_key_len, unshared_key_content, value_content);
    };

    for (int i = 0; i < RP.record_num; ++i) {
      const auto &[record_size, shared_key_len, unshared_key_content, value_content] = parse_record(RG_START);
      RG_START += record_size;
      // 差值压缩的逆过程
      // todo: 此处的.data()拼接可能不够高效，需要优化
      auto repaired_key = FULLKEY.substr(0, shared_key_len).data() + unshared_key_content;
      if (repaired_key == key) {
        value = value_content; // 找到了
        break;
      }
    }
    assert(RG_START <= START + RP.rp_offset.offset + RP.rp_offset.size);

    if (value.empty()) {
      return std::nullopt;
    }
    return value;
  }

  // 找到完整的键
  std::string SSTParser::FindFullKeyByRP(char *START, int record_num, const OffsetInfo &rg_offset) {
    assert(record_num > 0);
    auto first_record_ptr = START + rg_offset.offset;
    int shared_key_len = static_cast<int>(utils::DecodeFixed32(first_record_ptr));

    assert(shared_key_len == 0); // 第一条key共享长度一定为0
    int unshared_key_len = static_cast<int>(utils::DecodeFixed32(first_record_ptr + 4));

    assert(unshared_key_len > 0);
    std::string fullkey = std::string(first_record_ptr + 12, unshared_key_len);
    return fullkey;
  }

  // 将is输入流中存储的添加到sst_content中
  void SSTParser::ReadALL() {
    std::string tmp;
    while(getline(is,tmp)) {
      sst_content.append(tmp);
      tmp.clear();
    }
  }

  bool SSTParser::Exists(const std::string_view &key) {
    return filterPolicy->exists(key);
  }
}