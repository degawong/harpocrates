/*
 * @Description: 
 * @Autor: degawong
 * @Date: 2020-04-15 17:24:12
 * @LastEditors: degawong
 * @LastEditTime: 2020-05-12 15:05:06
 */
#pragma once

#include <base/base.h>
#include <singleton_pattern/singleton_pattern.h>

namespace harpocrates {

	using namespace type;

	class EnginePackage final : public SingletonPattern<EnginePackage> {
	public:
		~EnginePackage() = default;
	private:
		EnginePackage(
			handle opencl = nullptr,
			handle resize = nullptr,
			handle memory = nullptr,
			handle parallel = nullptr) :
			__opencl(opencl),
			__resize(resize),
			__memory(memory),
			__parallel(parallel) {
		};
	public:
		handle get_cl_handle() {
			return __opencl;
		}
		handle get_resize_handle() {
			return __resize;
		}
		handle get_memory_handle() {
			return __memory;
		}
		handle get_parallel_handle() {
			return __parallel;
		}
	private:
		handle __opencl;
		handle __resize;
		handle __memory;
		handle __parallel;
	private:
		friend SingletonPattern<EnginePackage>;
	};

	namespace standard {
		//template <typename _type, typename... _args>
		//std::unique_ptr<_type> make_unique_proxy(std::false_type, _args&&... args) {
		//	return std::unique_ptr<_type>(new _type(std::forward<_args>(args)...));
		//};
		//template <typename _type, typename... _args>
		//std::unique_ptr<_type> make_unique_proxy(std::false_type, _args&&... args) {
		//	static_assert(
		//		std::extent<_type>::value == 0,
		//		"make_unique<_type[_size]>() is forbidden, please use make_unique<_type[]>()."
		//	);
		//	using _clean_type = typename std::remove_extent<_type>::type;
		//	return std::unique_ptr<_type>(new _clean_type[sizeof...(_args)]{ std::forward<_args>(args)... });
		//};
		//template <typename _type, typename... _args>
		//std::unique_ptr<_type> make_unique(_args&&... args) {
		//	return make_unique_proxy<_type>(std::is_array<_type>(), std::forward<_args>(args)...);
		//};
	}

	namespace algorithm {
		class BaseAlgorithm {
		public:
			BaseAlgorithm() = default;
			virtual ~BaseAlgorithm() = default;
		};
	};
};