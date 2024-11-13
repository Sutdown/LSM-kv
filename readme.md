# LSM-KV

> 本项目预计基于LSM结构实现一个kv存储引擎。预计要实现：

- [x] 日志
- [x] 布隆过滤器
- [x] 内存分配器
- [x] cache
- [ ] 文件读写
- [ ] sstable
- [ ] WAL
- [ ] memtable

## 日志

> 实现方案：利用单例模式实现日志。

[单例模式的实现方式](https://blog.csdn.net/unonoi/article/details/121138176)；[spdlog - C++日志库](https://blog.xiyoulinux.com/blog/104106245)；[lock_guard and unique_ptr](https://www.cnblogs.com/linuxAndMcu/p/14576646.html)

参考的库是典型的**c++日志库`spdlog`**，它使用内存映射文件和异步日志记录技术，能够快速记录；支持多线程，能够保障线程安全（手段为互斥锁）；具有多种日志级别，采取了灵活的日志格式化选项，支持跨平台，多后端。

对于`spdlog::logger`加上**共享指针`shared_ptr`**，便携化资源释放。

对于创建一个日志单例采取的是最为经典的**静态局部变量的懒汉单例**，`static`既能保证共享性下只存在一个实例，也能保证变量的创建不被打乱，相比于双重锁和智能指针实现的更完善也更简单。

## 布隆过滤器

> 和leveldb中的实现大致类似。
>
> 利用googletest进行单元测试

相比于`levelDB`中根据键的位数确立哈希函数个数，这里采用**键的数量和假阳性率**直接确立最佳的**哈希函数数量和位数组大小**，这是一篇讲解`bloom filter`中的结论得到的。

创建过滤器时，会通过键和**双重哈希增量**模拟每次的哈希函数，哈希的方法采用的是murmur_hash方法，最终存储到相应的位中。

同时，判断是否存在其中也是对齐进行哈希计算，只要判断出一次不存在那就一定不存在与数组中，但是全部存在也不能证明一定在数组中。

## 内存分配器

> 它只会为小于4kb的内存块分配内存，
>
> - 内存池：一大块预分配的内存区域
>
> - 内存槽 + 内存块：对内存池进行组织和分类内存池中大小不同内存块的工具；
>
>   内存槽的核心在于记录和组织内存池中的空闲内存块
>
> ```cpp
>     private:
>         std::array<BlockNode *, SLOT_NUM> memory_slot;
>         char *mem_pool_start = nullptr;
>         int32_t mem_pool_size = 0;
> ```

内存池总共是4MB

组成：slot1(8字节/块)，slot2(16字节/块)，…，slot512(4096字节/块)

每个sloti指向的是该链表第一个空闲块

### 构造和析构

### 公共接口：allocate，deallocate，reallocate

- allocate：满足大于0小于4kb时，找到相应的内存槽填入内存槽，内存槽指针指向下一个内存块。
- deallocate：满足大于0小于4kb时，找到相应的内存槽，将该内存加入其中，内存槽指针指向新加入的位置，因为一般是头插法。
- reallocate：先deallocate，再allocate

### 私有函数：fill_slot，fill_mem_pool

- 第二个函数用于分配新的4MB的内存池。
- 第一个函数用于填内存槽，也就是当需要分配空间但是内存槽中没有相应字节的内存管理时，寻找到对应的sloti，一次性分配`FILL_BLOCK_CNT`也就是10个块。填内存槽时会出现三种情况：
  - 内存池容量足够分配所有块，直接分配
  - 内存池容量只够分配一部分块，先分配一部分块
  - 内存池容量一块也不够，剩余部分挂在到slot上，重新申请内存池（内存池会修改指针和大小，但是slot作为某种形式的数组/链表，是始终存在的，挂载也可以避免内存碎片；之后重新运行该函数，继续分配。

## Cache

**核心**是实现分片的LRU缓存

**Cache中持有N（默认为5）个指向CachePolicy的指针，相当于5个分片，可以减少哈希冲突以及减少锁的范围；LRUCache和LFUCache都是CachePolicy的子类。**

> 为什么使用LRU缓存，LFU呢，其它缓存为什么不行？

- LRU:淘汰最久未被访问的数据项
- LFU:淘汰访问频率最低的缓存项

LRU实现简单,并且更加适合具有局部性原理的访问模式.

> leveldb中为什么存在lru链表和in-use链表，两个链表？

一个存放lru中的结点,另一个存放被lru淘汰的结点,结点被淘汰了但是不一定没有被其它引用,贸然删除可能造成不好的结果,每隔一段时间,遍历in-use链表,如果引用计数变为0,将其删除.

> 两个哈希函数的作用；回调函数的作用。

这两个哈希和leveldb中两个链表是一样的作用.

回调函数: **在缓存条目被删除或替换时，执行一些清理操作**，通常是在缓存项被淘汰、移除或替换后进行特定的资源释放或额外的操作。

```cpp
std::unordered_map<K, typename std::std::list<Node<K, V> *>::iterator> index; // 保存键到结点的映射
/*
 * 用于跟踪当前正在被使用的缓存条目
 * 确保这些条目不会被替换。这有助于提高性能，因为活跃的条目通常是程序当前需要的数据。
  通过将活跃条目与不活跃条目分开管理，可以减少对整个缓存的锁竞争
*/
std::unordered_map<K, Node<K, V> *> garbage_station;  // 待删除列表，从index利用LRU策略删除后记录在此处
std::function<void(const K &key, V *val)> destructor; // 回调函数
```

[lrucache-leetcode](https://leetcode.cn/problems/lru-cache/)

## 参考

1. [leveldb - google](https://github.com/google/leveldb)
