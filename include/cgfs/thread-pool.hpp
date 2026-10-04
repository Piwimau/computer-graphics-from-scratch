#ifndef CGFS_THREAD_POOL_HPP
#define CGFS_THREAD_POOL_HPP

#include <algorithm>
#include <cassert>
#include <concepts>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stop_token>
#include <thread>
#include <type_traits>
#include <vector>
#include "cgfs/types.hpp"

namespace cgfs {

/** @brief A thread pool for executing tasks concurrently. */
class ThreadPool final {
private:

    /** @brief A mutex for synchronizing access to the internal state. */
    std::mutex _mutex;

    /** @brief The tasks to be executed. */
    std::queue<std::move_only_function<void()>> _tasks;

    /** @brief A condition variable to signal that tasks are available. */
    std::condition_variable_any _tasksAvailable;

    /** @brief The number of tasks queued or currently being executed. */
    isize _pending = 0;

    /** @brief A condition variable to signal that all threads are idle. */
    std::condition_variable _idle;

    /**
     * @brief The worker threads.
     *
     * @note This member must be declared last to ensure a proper destruction
     * order.
     */
    std::vector<std::jthread> _threads;

public:

    /**
     * @brief Constructs a thread pool with a specified number of threads.
     *
     * @param[in] numThreads The number of threads to create.
     *
     * @warning The behavior is undefined if `numThreads` is less than or equal
     * to zero.
     */
    explicit ThreadPool(
        isize numThreads =
            std::max<isize>(1, std::thread::hardware_concurrency())
    ) {
        assert(numThreads > 0);
        auto worker = [this](std::stop_token token) {
            while (true) {
                {
                    std::move_only_function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(_mutex);
                        _tasksAvailable.wait(
                            lock,
                            token,
                            [this]() { return !_tasks.empty(); }
                        );
                        if (token.stop_requested()) {
                            return;
                        }
                        task = std::move(_tasks.front());
                        _tasks.pop();
                    }
                    task();
                }
                {
                    std::scoped_lock<std::mutex> lock(_mutex);
                    _pending--;
                    if (_pending == 0) {
                        _idle.notify_all();
                    }
                }
            }
        };
        for (isize i = 0; i < numThreads; i++) {
            _threads.emplace_back(worker);
        }
    }

    ThreadPool(const ThreadPool&) = delete;

    ThreadPool& operator=(const ThreadPool&) = delete;

    /**
     * @brief Returns the number of threads.
     *
     * @return The number of threads.
     */
    isize size() const noexcept {
        return std::ssize(_threads);
    }

    /**
     * @brief Submits a task to be executed.
     *
     * @tparam F The type of the callable object.
     * @param[in] f The task to be executed.
     * @return A future representing the result of the submitted task.
     */
    template<typename F>
    requires std::invocable<F>
    auto submit(F&& f) -> std::future<std::invoke_result_t<F>> {
        using R = std::invoke_result_t<F>;
        std::packaged_task<R()> task(std::forward<F>(f));
        std::future<R> result = task.get_future();
        {
            std::scoped_lock<std::mutex> lock(_mutex);
            _tasks.emplace(std::move(task));
            _pending++;
        }
        _tasksAvailable.notify_one();
        return result;
    }

    /**
     * @brief Posts a task to be executed, discarding its result.
     *
     * @tparam F The type of the callable object.
     * @param[in] f The task to be executed.
     *
     * @warning The task must not throw an exception, as the program will be
     * terminated due to an uncaught exception otherwise.
     */
    template<typename F>
    requires std::invocable<F>
    void post(F&& f) {
        {
            std::scoped_lock<std::mutex> lock(_mutex);
            _tasks.emplace(std::forward<F>(f));
            _pending++;
        }
        _tasksAvailable.notify_one();
    }

    /**
     * @brief Waits until all threads are idle (i.e., all tasks have been
     * executed).
     *
     * @warning This function deadlocks if called from a thread managed by this
     * thread pool. Only call it from outside.
     */
    void wait_idle() {
        std::unique_lock<std::mutex> lock(_mutex);
        _idle.wait(lock, [this]() { return _pending == 0; });
    }

};

}

#endif