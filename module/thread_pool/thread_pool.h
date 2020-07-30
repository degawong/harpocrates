/*
 * @Description: 
 * @Autor: degawong
 * @Date: 2020-03-23 16:41:29
 * @LastEditors: degawong
 * @LastEditTime: 2020-05-12 15:04:15
 */
#pragma once

#include <deque>
#include <queue>
#include <mutex>
#include <future>
#include <vector>
#include <thread>
#include <atomic>
#include <numeric>
#include <iostream>
#include <functional>
#include <condition_variable>

#include <base/base.h>
#include <singleton_pattern/singleton_pattern.h>

namespace harpocrates {
	class ThreadPool : public SingletonPattern<ThreadPool> {
	public:
		~ThreadPool() {
			__stop_task();
		}
    private:
        ThreadPool(int thread_num = 8) : __stop(false) {
			__thread_num = (thread_num > 0 ? thread_num : std::thread::hardware_concurrency());
            for (size_t i = 0; i < __thread_num; ++i) {
				__threads.push_back(std::thread([this] {
					while (!__stop.load(std::memory_order_acquire)) {
						std::function<void()> task;
						{
							std::unique_lock<std::mutex> lock(this->__mutex);
							__cond.wait(lock, [this] {return !this->__tasks.empty() || __stop.load(std::memory_order_acquire); });
							if (__stop.load(std::memory_order_acquire)) {
								return;
							}
							task = std::move(__tasks.front());
							__tasks.pop();
						}
						task();
					}
				}));
			}
        }
	public:
        template <typename _function, typename... _args>
		// std::result_of is deprecated in c++17 and removed in c++20
		auto commit_task(_function&& function, _args&&... args) {
			typedef typename std::result_of<_function(_args...)>::type return_type;
			auto t = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<_function>(function), std::forward<_args>(args)...));
			auto ret = t->get_future();
			{
				std::unique_lock<std::mutex> lock(__mutex);
				__tasks.emplace([t] {
					(*t)();
				}); 
			}
			__cond.notify_one();
			return ret;
		}
	private:
        void __stop_task() {
			__stop.store(true, std::memory_order_release);
			__cond.notify_all();
			std::for_each(
				__threads.begin(),
				__threads.end(), 
				[&] (auto& thread) {
					if (thread.joinable()) {
						thread.join();
					}
				}
			);
		}
    private:
        int __thread_num;
        std::mutex __mutex;
		std::atomic<bool> __stop;
        std::condition_variable __cond;
        std::vector<std::thread> __threads;
		std::queue<std::function<void()>> __tasks;
	private:
		friend SingletonPattern<ThreadPool>;
    };
	
	// add map && reduce
	template <typename _function, typename... _args>
	auto multi_invoke(_function function, _args&&... args) {
		std::initializer_list<int>{ ((function(std::forward<_args>(args))), 0)...};
	};

	template<typename  _function, typename _iter_type, typename... _args, size_t _hint = 8>
	return_code parallel_execution(_iter_type begin, _iter_type end, _function&& function, _args&&... args) {
		auto tp = ThreadPool::get_instance();
		typedef typename std::result_of<_function(_iter_type, _iter_type, _args...)>::type return_type;
		std::vector<std::future<return_type>> thread_result;
		auto thread_count = min(std::thread::hardware_concurrency(), max(1, _hint));
		auto chunk_size = ((end - begin) / thread_count)>> 1 << 1;
		for (size_t i = 0; i < thread_count; ++i) {
			thread_result.push_back(
				std::move(
					tp->commit_task(
						function,
						begin + i * chunk_size,
						((i == thread_count - 1) ? end : begin + (i + 1) * chunk_size),
						std::forward<_args>(args)...
					)
				)
			);
		}
		std::for_each(
			thread_result.begin(),
			thread_result.end(),
			[&](auto& iter) {
				iter.get();
			}
		);
		return return_code::success;
	};

	// note
	// it is very not applicable for light caculation operation
	// usage
	// u can put heavy caculation into such as a vector or some other container
	template<typename _iter_type, typename  _function>
	return_code parallel_for_each(_iter_type begin, _iter_type end, _function function) {
		auto tp = ThreadPool::get_instance();
		typedef typename std::result_of<_function(_iter_type)>::type return_type;
		std::vector<std::future<return_type>> thread_result;
		for (auto iter = begin; iter < end; ++iter) {
			thread_result.push_back(
				std::move(
					tp->commit_task(
						function, 
						iter
					)
				)
			);
		}
		std::for_each(
			thread_result.begin(),
			thread_result.end(),
			[&](auto& iter) {
			    iter.get();
		    }
		);
		return return_code::success;
	};
}
