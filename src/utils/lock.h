#include <mutex>
#include <atomic>

#ifndef LOCK_H
#define LOCK_H

namespace lsmkv
{
    template <typename T>
    class ScopedLock
    {
    public:
        ScopedLock(T &t) : local_lock(t)
        {
            local_lock.lock();
            is_locked = true;
        }
        ~ScopedLock()
        {
            unlock();
        }

        void lock()
        {
            if (!is_locked)
            {
                local_lock.lock();
                is_locked = true;
            }
        }

        void unlock()
        {
            if (is_locked)
            {
                local_lock.unlock();
                is_locked = false;
            }
        }

    private:
        T &local_lock;
        bool is_locked = false;
    };

    class NullLock final
    {
    public:
        NullLock() = default;
        ~NullLock() = default;

        void lock() {}
        void unlock() {}
    };

    class MutexLock final
    {
    public:
        MutexLock() = default;
        ~MutexLock() = default;

        void lock()
        {
            _mutex.lock();
        }
        void unlock()
        {
            _mutex.unlock();
        }

    private:
        std::mutex _mutex;
    };

    class SpinLock final
    {
    public:
        SpinLock() = default;
        SpinLock(const SpinLock &) = delete;
        SpinLock &operator=(const SpinLock &) = delete;

        void lock()
        {
            while (flag.test_and_set())
                ;
        }

        void unlock()
        {
            flag.clear();
        }

    private:
        std::atomic_flag flag{0};
    };
}

#endif