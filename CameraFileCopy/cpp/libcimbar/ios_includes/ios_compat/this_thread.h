/* iOS线程兼容层 - this_thread.h
 * 提供与std::this_thread兼容的API，完全避免使用标准库实现
 */
#pragma once

#include "thread.h" // 包含我们的线程定义
#include <unistd.h>  // 用于usleep
#include <sys/time.h> // 用于gettimeofday
#include <functional>
#include <ratio>

namespace ios_compat {
namespace this_thread {

// 获取当前线程的ID
inline thread::id get_id() noexcept {
    return thread::id(pthread_self());
}

// 让出CPU时间片
inline void yield() noexcept {
    sched_yield();
}

// 睡眠指定时间 - 通用模板
template <typename Rep, typename Period>
void sleep_for(const std::chrono::duration<Rep, Period>& sleep_duration) {
    // 转换为微秒
    using namespace std::chrono;
    auto usecs = duration_cast<microseconds>(sleep_duration).count();
    
    // 使用usleep实现延迟
    if (usecs > 0) {
        usleep(static_cast<useconds_t>(usecs));
    }
}

// 睡眠到指定时间点
template <typename Clock, typename Duration>
void sleep_until(const std::chrono::time_point<Clock, Duration>& sleep_time) {
    auto now = Clock::now();
    if (now < sleep_time) {
        sleep_for(sleep_time - now);
    }
}

} // namespace this_thread
} // namespace ios_compat
