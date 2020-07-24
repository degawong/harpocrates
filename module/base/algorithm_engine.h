/*
 * @Description: 
 * @Autor: degawong
 * @Date: 2020-04-15 17:24:12
 * @LastEditors: degawong
 * @LastEditTime: 2020-05-12 15:05:06
 */
#pragma once

#include <singleton_pattern/singleton_pattern.h>

namespace harpocrates {

	using handle = void *;

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

	namespace algorithm {
		class BaseAlgorithm {
		public:
			BaseAlgorithm() = default;
			virtual ~BaseAlgorithm() = default;
		};
	};
};