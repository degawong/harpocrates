/*
 * @Description: 
 * @Autor: degawong
 * @Date: 2020-03-23 16:41:29
 * @LastEditors: degawong
 * @LastEditTime: 2020-05-12 15:04:05
 */
#pragma once

#include <deque>
#include <queue>
#include <mutex>
#include <chrono>
#include <future>
#include <vector>
#include <thread>
#include <atomic>
#include <numeric>
#include <iostream>
#include <functional>
#include <condition_variable>

namespace harpocrates {
	class TimeClock {
	public:
		TimeClock() {
			__tic();
		}
		~TimeClock() = default;
	public:
		template<typename _type = std::chrono::microseconds>
		long long time_duration() {
			return std::chrono::duration_cast<_type>(__toc()).count();
		}
	private:
		void __tic() {
			__clock = std::chrono::high_resolution_clock::now();
		}
		std::chrono::nanoseconds __toc() {
			auto _return_count = std::chrono::high_resolution_clock::now() - __clock;
			__clock = std::chrono::high_resolution_clock::now();
			return _return_count;
		}
	private:
		std::chrono::time_point<std::chrono::high_resolution_clock> __clock;
	};
}
