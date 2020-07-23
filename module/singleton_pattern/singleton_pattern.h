/*
 * @Description: 
 * @Autor: degawong
 * @Date: 2020-03-23 16:41:29
 * @LastEditors: degawong
 * @LastEditTime: 2020-05-12 15:03:32
 */
#pragma once

#include <deque>
#include <regex>
#include <mutex>
#include <queue>
#include <atomic>
#include <future>
#include <thread>
#include <vector>
#include <numeric>
#include <iostream>
#include <filesystem>
#include <functional>
#include <condition_variable>

namespace harpocrates {

	template<typename _derived>
	class SingletonPattern {
	public:
		template<typename ..._args>
		static std::shared_ptr<_derived> get_instance(_args&&... args) {
			std::lock_guard<std::mutex> lock(__mutex);
			if (nullptr == __instance) {
				auto ptr = new(std::nothrow) _derived(std::forward<_args>(args)...);
				__instance = std::shared_ptr<_derived>(ptr);
				// still i don't know why, is there any other class needed to construct the instance
				//__instance = std::make_shared<_derived>(std::forward<_args>(args)...);
			}
			return __instance;
		}
	protected:
		SingletonPattern() = default;
		virtual ~SingletonPattern() {}
	protected:
		SingletonPattern(SingletonPattern<_derived>&& anther) = delete;
		SingletonPattern(const SingletonPattern<_derived>& anther) = delete;
		SingletonPattern<_derived> &operator=(SingletonPattern<_derived>&& anther) = delete;
		SingletonPattern<_derived> &operator=(const SingletonPattern<_derived>& anther) = delete;
	private:
		static std::mutex __mutex;
		static std::shared_ptr<_derived> __instance;
	};

	template<typename _derived>
	std::mutex SingletonPattern<_derived>::__mutex;

	template<typename _derived>
	std::shared_ptr<_derived> SingletonPattern<_derived>::__instance = nullptr;

	//// declearation may meed typedef typename ***::*** ***
	//// is this the typename declearation ???
	//template<typename _derived>
	//typename SingletonPattern<_derived>::Dummy SingletonPattern<_derived>::__dummy;

	//template<typename _derived>
	//inline SingletonPattern<_derived>::Dummy::~Dummy() {
	//	// why i have a _derived entity, but i still cannot enter this interface
	//	// even i cannot enter the deconstruction function of the _derived entity
	//	// when i give back the memory to system, can not it call memory leak ???
	//	if (nullptr != SingletonPattern<_derived>::__instance) {
	//		delete SingletonPattern<_derived>::__instance;
	//		SingletonPattern<_derived>::__instance = nullptr;
	//	}
	//}

	template<typename _derived>
	class Singleton {
	public:
		template<typename ..._args>
		static std::shared_ptr<_derived> get_instance(_args&&... args) {
			auto _instance = __instance.load(std::memory_order_relaxed);
			std::atomic_thread_fence(std::memory_order_acquire);
			if (nullptr == _instance) {
				std::lock_guard<std::mutex> lock(__mutex);
				_instance = __instance.load(std::memory_order_relaxed);
				if (nullptr == _instace) {
					_instance = new(std::nothrow) _derived(std::forward<_args>(args)...);
					std::atomic_thread_fence(std::memory_order_release);
					__instance.store(_instance, std::memory_order_relaxed);
				}
			}
			return _instance;
		}
	protected:
		Singleton() {}
		virtual ~Singleton() {}
	protected:
		Singleton(Singleton<_derived>&& _anther) = delete;
		Singleton(const Singleton<_derived>& _anther) = delete;
		Singleton<_derived> &operator=(Singleton<_derived>&& _anther) = delete;
		Singleton<_derived> &operator=(const Singleton<_derived>& _anther) = delete;
	protected:
		class Dummy {
		public:
			~Dummy() {
				auto _instance = __instance.load(std::memory_order_relaxed);
				if (nullptr != _instance) {
					delete _instance;
				}
			}
		};
	private:
		static Dummy __dummy;
	private:
		static std::mutex __mutex;
		static std::atomic<Singleton<_derived>*> __instance;
	};

	template<typename _derived>
	std::mutex Singleton<_derived>::__mutex;

	template<typename _derived>
	std::atomic<Singleton<_derived>*> Singleton<_derived>::__instance = nullptr;

	class SingletonMY {
	public:
		static SingletonMY& get_instance() {
			static SingletonMY __instance;
			return __instance;
		}
	private:
		SingletonMY();
		SingletonMY(const SingletonMY& other);
	};

	template<typename _derived>
	class SingletonMeyers {
	public:
		~SingletonMeyers() = default;
		static _derived& getInstance() {
			static _derived __instance;
			return __instance;
		}
	private:
		SingletonMeyers(const SingletonMeyers&) = delete;
		SingletonMeyers& operator = (const SingletonMeyers&) = delete;
	private:
		SingletonMeyers() = default;
	private:
		friend typename _derived;
	};

	template<typename _derived>
	class SingletonAtomic {
	public:
		~SingletonAtomic() = default;
		template<typename ..._args>
		static _derived* getInstance() {
			static std::once_flag _once_flag;
			std::call_once(_once_flag, [&] {
				__instance = new(std::nothrow) _derived(std::forward<_args>(args)...); 
			});
			return __instance;
		}
	private:
		SingletonAtomic(const SingletonAtomic&) = delete;
		SingletonAtomic& operator = (const SingletonAtomic&) = delete;
	private:
		SingletonAtomic() = default;
	private:
		friend typename _derived;
	};
}