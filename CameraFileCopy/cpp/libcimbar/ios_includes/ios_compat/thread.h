/* iOS线程兼容层 - thread.h
 * 提供与std::thread兼容的API，但使用POSIX pthread直接实现
 * 完全隔离在ios_compat命名空间内，避免与系统冲突
 */
#pragma once

#include <pthread.h>
#include <functional>

namespace ios_compat {

class thread {
public:
    // 默认构造函数
    thread() noexcept : _handle(0), _joinable(false) {}

    // 移动构造函数
    thread(thread&& other) noexcept : _handle(other._handle), _joinable(other._joinable) {
        other._handle = 0;
        other._joinable = false;
    }

    // 使用函数初始化
    template<typename Function, typename... Args>
    explicit thread(Function&& f, Args&&... args) {
        auto task = std::bind(std::forward<Function>(f), std::forward<Args>(args)...);
        auto* taskPtr = new decltype(task)(std::move(task));
        
        auto threadFunc = [](void* arg) -> void* {
            auto taskPtr = static_cast<decltype(task)*>(arg);
            (*taskPtr)();
            delete taskPtr;
            return nullptr;
        };
        
        _joinable = !pthread_create(&_handle, nullptr, threadFunc, taskPtr);
    }

    // 析构函数
    ~thread() {
        if (_joinable)
            std::terminate(); // 标准库行为：未join的线程在销毁时终止程序
    }

    // 移动赋值操作符
    thread& operator=(thread&& other) noexcept {
        if (this != &other) {
            if (_joinable)
                std::terminate(); // 标准库行为
            
            _handle = other._handle;
            _joinable = other._joinable;
            
            other._handle = 0;
            other._joinable = false;
        }
        return *this;
    }

    // 禁用复制
    thread(const thread&) = delete;
    thread& operator=(const thread&) = delete;

    // 检查线程是否可join
    bool joinable() const noexcept {
        return _joinable;
    }

    // 等待线程完成
    void join() {
        if (!_joinable)
            std::terminate(); // 标准库行为：join非joinable线程将终止程序
        
        pthread_join(_handle, nullptr);
        _joinable = false;
    }

    // 分离线程
    void detach() {
        if (!_joinable)
            std::terminate(); // 标准库行为
        
        pthread_detach(_handle);
        _joinable = false;
    }

    // 获取线程ID
    class id;
    id get_id() const noexcept;

    // 交换两个线程
    void swap(thread& other) noexcept {
        std::swap(_handle, other._handle);
        std::swap(_joinable, other._joinable);
    }

    // 获取底层pthread句柄
    pthread_t native_handle() {
        return _handle;
    }
    
private:
    pthread_t _handle;
    bool _joinable;
};

// 线程ID类
class thread::id {
public:
    id() noexcept : _id(0) {}
    explicit id(pthread_t pid) noexcept : _id(pid) {}
    
    friend bool operator==(id lhs, id rhs) noexcept {
        return pthread_equal(lhs._id, rhs._id);
    }
    
    friend bool operator!=(id lhs, id rhs) noexcept {
        return !(lhs == rhs);
    }
    
    friend bool operator<(id lhs, id rhs) noexcept {
        return lhs._id < rhs._id;
    }
    
private:
    pthread_t _id;
};

// 获取线程ID的实现
inline thread::id thread::get_id() const noexcept {
    return id(_handle);
}

// 交换函数
inline void swap(thread& lhs, thread& rhs) noexcept {
    lhs.swap(rhs);
}

}  // namespace ios_compat
