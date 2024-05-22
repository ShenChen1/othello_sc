/*
    Egaroucid Project

    @file thread_pool.hpp
        Thread pool for Egaroucid
    @date 2021-2023
    @author Takuto Yamana
    @license GPL-3.0 license
    @notice This code is based on https://github.com/ContentsViewer/nodec/blob/main/nodec/include/nodec/concurrent/thread_pool_executor.hpp , which is published under Apache License 2.0
*/

#pragma once
#include <iostream>
#include <future>
#include <queue>
#include <thread>
#include <atomic>

// Original code based on
//  * <https://github.com/bshoshany/thread-pool>
//  * <https://github.com/progschj/ThreadPool>
//  * <https://github.com/SandSnip3r/thread-pool>
// Thank you! :)

class Thread_pool {
    private:
        mutable std::mutex mtx;
        bool running;
        int n_thread;
        //std::atomic<int> n_idle;
        int n_idle;
        std::queue<std::function<void()>> tasks{};
        std::unique_ptr<std::thread[]> threads;
        std::condition_variable condition;

    public:
        void set_thread(int new_n_thread){
            if (new_n_thread < 0)
                new_n_thread = 0;
            n_thread = new_n_thread;
            threads.reset(new std::thread[n_thread]);
            for (int i = 0; i < n_thread; ++i)
                threads[i] = std::thread(&Thread_pool::worker, this);
            running = true;
            n_idle = 0;
        }

        void exit_thread(){
            {
                std::lock_guard<std::mutex> lock(mtx);
                running = false;
            }
            condition.notify_all();
            for (int i = 0; i < n_thread; ++i)
                threads[i].join();
            n_thread = 0;
            n_idle = 0;
        }

        Thread_pool(){
            set_thread(0);
        }

        Thread_pool(int new_n_thread){
            set_thread(new_n_thread);
        }

        ~Thread_pool(){
            exit_thread();
        }

        void resize(int new_n_thread){
            exit_thread();
            set_thread(new_n_thread);
        }

        int size() const {
            return n_thread;
        }

        int get_n_idle() const {
            return n_idle;
        }

        #if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
            template<typename F, typename... Args, typename R = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>>
        #else
            template<typename F, typename... Args, typename R = typename std::result_of<std::decay_t<F>(std::decay_t<Args>...)>::type>
        #endif
        std::future<R> push(bool *pushed, F &&func, const Args &&...args){
            auto task = std::make_shared<std::packaged_task<R()>>([func, args...](){
                return func(args...);
            });
            auto future = task->get_future();
            *pushed = push_task([task](){(*task)();});
            return future;
        }

    private:

        template<typename F>
        inline bool push_task(const F &task){
            if (!running)
                throw std::runtime_error("Cannot schedule new task after shutdown.");
            bool pushed = false;
            if (n_idle > 0){
                mtx.lock();
                    if (n_idle > 0){
                        pushed = true;
                        tasks.push(std::function<void()>(task));
                        --n_idle;
                        condition.notify_one();
                    }
                mtx.unlock();
            }
            return pushed;
        }

        void worker(){
            std::function<void()> task;
            for (;;){
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    ++n_idle;
                    condition.wait(lock, [&] {return !tasks.empty() || !running;});
                    if (!running)
                        return;
                    task = std::move(tasks.front());
                    tasks.pop();
                }
                task();
            }
        }
};

Thread_pool thread_pool(0);
