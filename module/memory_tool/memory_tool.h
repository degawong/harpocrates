/*
 * @Description: 
 * @Autor: degawong
 * @Date: 2019-03-23 19:45:33
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-08-08 01:18:57
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

#include <base/base.h>

#include <singleton_pattern/singleton_pattern.h>

namespace harpocrates {

	using namespace type;
	using namespace operator_reload;

	//down align
	//size - (size % align) = size & ~(align - 1)
	//up align
	//(size + (align - 1)) - ((size + (align - 1))% align) = (size + (align - 1))& ~(align - 1)
	size_t div_up(size_t value, size_t divider) {
        return (value + divider - 1) / divider;
    }

	size_t align_up_any(size_t size, size_t align) {
        return (size + align - 1) / align * align;
    }

	size_t align_down_any(size_t size, size_t align) {
        return size / align * align;
    }

	size_t align_up(size_t size, size_t align) {
        return (size + align - 1) & ~(align - 1);
    }

	void * align_up(const void * ptr, size_t align) {
        return (void *)((((size_t)ptr) + align - 1) & ~(align - 1));
    }

	size_t align_down(size_t size, size_t align) {
        return size & ~(align - 1);
    }

	void * align_down(const void * ptr, size_t align) {
        return (void *)(((size_t)ptr) & ~(align - 1));
    }

	bool is_aligned(size_t size, size_t align) {
        return size == align_down(size, align);
    }

	bool is_aligned(const void * ptr, size_t align) {
        return ptr == align_down(ptr, align);
    }

	size_t __align_size(size_t size, int n) {
		return (size + n - 1) & -n;
	}

	template<typename _type, int _align_size = 64>
	_type* __align_pointer(_type* ptr, int align_size = _align_size) {
		return (_type*)(((size_t)ptr + align_size - 1) & -align_size);
	}

	template<typename _type, int _align_size = 64>
	struct AllignAllocator : public base_type<_type> {
	public:
		AllignAllocator() = default;
		~AllignAllocator() = default;
	public:
		template <typename _other>
		// construct from a related allocator
		AllignAllocator(const AllignAllocator<_other> &) noexcept {
		}
		template<typename _other>
		// convert this type to AllignAllocator<_other>
		struct rebind {
			using other = AllignAllocator<_other>;
		};
	public:
		decltype(auto) allocator() noexcept {
		}
		template<typename _other>
		decltype(auto) allocator(const AllignAllocator<_other>&) noexcept {
		}
		decltype(auto) allocate(size_t size) const {
			return static_cast<pointer>(__align_allocator(size));
		}
		decltype(auto) allocate(size_t size, const void *) const {
			// allocate array of _Count elements, ignore hint
			return allocate(size);
		}
		decltype(auto) deallocate(void *ptr, size_t) const noexcept {
			return __align_dellocator(ptr, 0);
		}
	public:
		auto max_size() const noexcept {
			// estimate maximum array size
			return (static_cast<size_t>(-1) / sizeof(value_type));
		}
	private:
		decltype(auto) __align_allocator(size_t size) const {
			// sizeof(void*) is the u_ptr address size
			pointer u_ptr = (pointer)std::malloc(size * sizeof(value_type) + sizeof(void*) + _align_size);
			pointer* a_ptr = __align_pointer<pointer, _align_size>((pointer*)u_ptr + 1, _align_size);
			a_ptr[-1] = u_ptr;
			return (pointer)a_ptr;
		}
		void __align_dellocator(void *ptr, size_t) const {
			auto u_ptr = ((pointer*)ptr)[-1];
			std::free(u_ptr);
		}
	public:
		bool operator==(const AllignAllocator &) const noexcept {
			return true; 
		}
		bool operator!=(const AllignAllocator &) const noexcept { 
			return false; 
		}
	};

	template<>
	// generic allocator for type void
	struct AllignAllocator<void> : public base_type<void> {
		template<class _other>
		struct rebind {
			using other = AllignAllocator<_other>;
		};
	};

	template<typename _type, typename _other>
	inline bool operator== (
		const AllignAllocator<_type>&,
		const AllignAllocator<_other>&) noexcept {
		return (true);
	}

	template<typename _type, typename _other>
	inline bool operator!=(
		const AllignAllocator<_type>&,
		const AllignAllocator<_other>&) noexcept {
		return (false);
	}

	template<class _derived>
	class ReferCount {
	protected:
		auto _shallow_clean() {
			__release();
		}
		auto _get_refer() {
			return __refer_count;
		}
		auto _get_refer() const {
			return __refer_count;
		}
		auto _add_ref_count() {
			(nullptr != __refer_count) | [this]() {
				(*__refer_count)++;
			};
		}
		auto _dec_ref_count() {
			(__derived().__shareable) | [this]() {
				(nullptr != __refer_count) | [this]() {
					(1 == ((*__refer_count)--)) | [this]() {
						__uninit();
					};
					__release();
				};
			};
		}
		auto _init(int* ref_count) {
			__refer_count = ref_count;
		}
	private:
		_derived& __derived() { 
			return *static_cast<_derived*>(this); 
		}
		const _derived& __derived() const {
			return *static_cast<const _derived*>(this); 
		}
		auto __release() {
			__refer_count = nullptr;
			__derived().__shallow_clean();
		}
		auto __uninit() {
			delete __refer_count;
			__derived().__dellocator();
		}		
	protected:
		int* __refer_count;
	};

	template<typename _type, typename _allocator = AllignAllocator<_type>>
	class AutoBuff : public base_type<_type>, public ReferCount<AutoBuff<_type, _allocator>> {
	public:
		~AutoBuff() {
			_dec_ref_count();
		}
		AutoBuff(int cols = 3000, int rows = 2, int channels = 3) : 
		    __cols(cols), __rows(rows), __channels(channels), __data(nullptr), __shareable(true), __allocator(_allocator()) {
			_init(new int(1));
			__data = __allocator.allocate(__cols * __rows * __channels);
		}
	public:
		AutoBuff(AutoBuff&& auto_buff) noexcept {
			_shallow_clean();
			_init(auto_buff._get_refer());
			__cols = auto_buff.__cols;
			__rows = auto_buff.__rows;
			__data = auto_buff.__data;
			__channels = auto_buff.__channels;
			__shareable = auto_buff.__shareable;
			auto_buff._shallow_clean();
		}
		AutoBuff(const AutoBuff& auto_buff) noexcept {
			_shallow_clean();
			_init(auto_buff._get_refer());
			__cols = auto_buff.__cols;
			__rows = auto_buff.__rows;
			__data = auto_buff.__data;
			__channels = auto_buff.__channels;
			__shareable = auto_buff.__shareable;
			_add_ref_count();
		}
	public:
		AutoBuff& operator= (AutoBuff&& auto_buff) {
			if (&object != this) {
				_dec_ref_count();
				_shallow_clean();
				_init(object._get_refer());
				__cols = auto_buff.__cols;
				__rows = auto_buff.__rows;
				__data = auto_buff.__data;
				__shareable = auto_buff.__shareable;
				__channels = auto_buff.__channels;
				auto_buff._shallow_clean();
			}
		}
		AutoBuff& operator= (const AutoBuff& auto_buff) {
			if (&object != this) {
				_dec_ref_count();
				_shallow_clean();
				_init(object._get_refer());
				_add_ref_count();
				__cols = auto_buff.__cols;
				__rows = auto_buff.__rows;
				__data = auto_buff.__data;
				__channels = auto_buff.__channels;
				__shareable = auto_buff.__shareable;
				auto_buff._shallow_clean();
			}
		}
	public:
		operator pointer () {
			return __data;
		}
		operator const_pointer () const {
			return __data; 
		}
		reference operator[](int index) {
			return __data[index];
		}
		const_reference operator[](int index) const {
			return __data[index];
		}
	public:
		decltype(auto) get_data(int index = 0) {
			return &__data[index * __cols * __channels];
		}
	private:
		decltype(auto) __dellocator() {
			__allocator.deallocate(__data, 0);
		}
		decltype(auto) __release() {
		}
		decltype(auto) __shallow_clean() {
			__cols = 0;
			__rows = 0;
			__channels = 0;
			__data = nullptr;
		}
	private:
		decltype(auto) __reallocator() {
			__allocator.deallocate(__data, 0);
			__data = __allocator.allocate(__cols * __rows * __channels);
		}
	private:
		int __cols;
		int __rows;
		int __channels;
		pointer __data;
	private:
		bool __shareable;
	private:
		_allocator __allocator;
	private:
		friend ReferCount<AutoBuff>;
	};

	template<typename _type, typename _allocator = AllignAllocator<_type>>
	class FixedBuff : public base_type<_type>, public SingletonPattern<FixedBuff<_type>> {
	public:
		~FixedBuff() {
			__allocator.deallocate(__data, 0);
		}
	private:
		FixedBuff(int cols = 1024, int rows = 1024, int channels = 1) : 
		    __cols(cols), __rows(rows), __channels(channels), __data(nullptr), __allocator(_allocator()) {
			__data = __allocator.allocate(__cols * __rows * __channels);
		}
	public:
		decltype(auto) get_data() {
			return __data;
		}
		decltype(auto) get_data(int cols, int rows, int channels) {
			((cols * rows * channels) > (__cols * __rows * __channels)) | [&,this]() {
				__cols = cols;
				__rows = rows;
				__channels = channels;
				__reallocator();
			};
			return __data;
		}
	private:
		decltype(auto) __reallocator() {
			__allocator.deallocate(__data, 0);
			__data = __allocator.allocate(__cols * __rows * __channels);
		}
	private:
		int __cols;
		int __rows;
		int __channels;
		pointer __data;
	private:
		_allocator __allocator;
	private:
		friend SingletonPattern<FixedBuff>;
	};

}
