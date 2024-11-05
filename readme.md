# LSM-KV

> 本项目预计基于LSM结构实现一个kv存储引擎。预计要实现：

- [x] 日志
- [x] 布隆过滤器（测试还没过）
- [ ] 内存分配器
- [ ] cache
- [ ] 文件读写
- [ ] sstable
- [ ] WAL
- [ ] memtable

## 日志

> 实现方案：利用单例模式实现日志。

[单例模式的实现方式](https://blog.csdn.net/unonoi/article/details/121138176)：**目前最合适的实现方式是静态局部变量的懒汉单例**。

[spdlog - C++日志库](https://blog.xiyoulinux.com/blog/104106245)

[lock_guard and unique_ptr](https://www.cnblogs.com/linuxAndMcu/p/14576646.html)

## 布隆过滤器

> 和leveldb中的实现大致类似。
>
> 利用googletest进行单元测试

## 参考

1. [leveldb - google](https://github.com/google/leveldb)
