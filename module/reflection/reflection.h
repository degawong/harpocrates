/*
 * @Description: 
 * @Autor: degawong
 * @Date: 2020-04-15 15:28:40
 * @LastEditors: degawong
 * @LastEditTime: 2020-05-12 15:04:26
 */
#pragma once

#include <map>
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
#include <unordered_map>
#include <condition_variable>

#include <base/base.h>
#include <base/algorithm_engine.h>
#include <singleton_pattern/singleton_pattern.h>

namespace harpocrates {
	template<
		decltype(auto) _signature,
		typename _algorithm_type = algorithm::BaseAlgorithm,
		typename _algorithm_code = algorithm_code
	>
	class Reflection : public SingletonPattern<
		Reflection<
		_signature,
		_algorithm_type,
		_algorithm_code
		>
	> {
	private:
		Reflection() = default;
	public:
		~Reflection() = default;
	public:
		decltype(auto) regist_factory(
			_algorithm_code algorithm_code,
			std::shared_ptr<_algorithm_type> algorithm_type) {
			__map[algorithm_code] = algorithm_type;
		}
		decltype(auto) get_algorithm(_algorithm_code algorithm_code) {
			return __map[algorithm_code];
		}
	private:
		std::map<_algorithm_code, std::shared_ptr<_algorithm_type>> __map;
	private:
		friend SingletonPattern<Reflection>;
	};

	template<
		decltype(auto) _signature,
		typename _method,
		typename _return,
		typename... _args
	>
	// avoid unexpected substitution, u can use a signature sometimes
	class ImplReflection : public SingletonPattern<
		ImplReflection<
		_signature,
		_method,
		_return,
		_args...
		>
	> {
		using invoke_type = std::function<_return(_args...)>;
	public:
		decltype(auto) regist_factory(
			_method method,
			invoke_type function) {
			__map[method] = function;
		}
		decltype(auto) get_algorithm(_method method) {
			return __map[method];
		}
	private:
		std::map<_method, invoke_type> __map;
	private:
		friend SingletonPattern<ImplReflection>;
	};
}
