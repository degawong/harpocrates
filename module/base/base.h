/*
 * @Description: 
 * @Autor: degawong
 * @Date: 2020-04-15 17:24:12
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-07-23 18:55:55
 */
#pragma once

namespace harpocrates {

	using handle = void *;

	enum class return_code {
		success,
		io_error,
		null_ptr,
		unsupport,
		not_exsit,
		not_match,
		not_directory,
		out_of_memory,
	};

	enum class algorithm_code {
		base_algorithm,
		derived_algorithm,
	};

	template <typename _type>
	_type min(_type&& value) {
		return value;
	};

	template <typename _type_1, typename _type_2>
	_type_1 min(_type_1&& first, _type_2&& second) {
		return (_type_1)(first > second ? second : first);
	};

	template <typename _type, typename... _type_rest>
	_type min(_type&& head, _type_rest&&... rest) {
		return min(head, min(rest...));
	};

	template <typename _type>
	_type max(_type&& value) {
		return value;
	};

	template <typename _type_1, typename _type_2>
	_type_1 max(_type_1&& first, _type_2&& second) {
		return (_type_1)(first < second ? second : first);
	};

	template <typename _type, typename... _type_rest>
	_type max(_type&& head, _type_rest&&... rest) {
		return max(head, max(rest...));
	};

	template <typename _type, typename... _args>
	_type sum(_args&&... args) {
		return (_type)(args + ...);
	};

	template<typename _type_1, typename _type_2>
	_type_1 clamp( _type_1 value, _type_2 low_limit, _type_2 up_limit) {
		return min(max(low_limit, value), up_limit);
	};

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

	auto any_equel = [](auto&& input, auto&&... args) -> bool {
		return ((args == input) || ...);
	};
	auto any_lower = [](auto&& input, auto&&... args) -> bool {
		return ((args > input) && ...);
	};
	auto any_larger = [](auto&& input, auto&&... args) -> bool {
		return ((args < input) && ...);
	};
	auto every_eque = [](auto&& input, auto&&... args) -> bool {
		return ((args == input) && ...);
	};
	auto every_lower = [](auto&& input, auto&&... args) -> bool {
		return ((args > input) && ...);
	};
	auto every_larger = [](auto&& input, auto&&... args) -> bool {
		return ((args < input) && ...);
	};
	auto any_not_equel = [](auto&& input, auto&&... args) -> bool {
		return ((args != input) || ...);
	};
	auto every_not_eque = [](auto&& input, auto&&... args) -> bool {
		return ((args != input) && ...);
	};

	class Noncopyable {
	protected:
		Noncopyable() = default;
		~Noncopyable() = default;
	private:
		Noncopyable(const Noncopyable&) = delete;
		const Noncopyable& operator=(const Noncopyable&) = delete;
	};

	namespace type {
		using uchar = unsigned char;
		using int8_t = signed char;
		using int16_t = short;
		using int32_t = int;
		using int64_t = long long;
		using uint8_t = unsigned char;
		using uint16_t = unsigned short;
		using uint32_t = unsigned int;
		using uint64_t = unsigned long long;

		using int_least8_t = signed char;
		using int_least16_t = short;
		using int_least32_t = int;
		using int_least64_t = long long;
		using uint_least8_t = unsigned char;
		using uint_least16_t = unsigned short;
		using uint_least32_t = unsigned int;
		using uint_least64_t = unsigned long long;

		using int_fast8_t = signed char;
		using int_fast16_t = int;
		using int_fast32_t = int;
		using int_fast64_t = long long;
		using uint_fast8_t = unsigned char;
		using uint_fast16_t = unsigned int;
		using uint_fast32_t = unsigned int;
		using uint_fast64_t = unsigned long long;

		using intmax_t = long long;
		using uintmax_t = unsigned long long;
	}

	namespace image {
		enum class filte_method {
			box = 0,
			median = 1,
			gaussian = 2,
		};

		enum class interp_method {
			linear = 0,
			bilinear = 1,
			bicubic = 2,
		};

		enum class image_info {
			plane_number = 3,
			format_number = 1,
			element_number = 2,
		};

		template<int _arg_1, int _arg_2, int _arg_3>
		struct format_code {
			enum { value = (((_arg_1 << 8) + _arg_2) << 8) + _arg_3 };
		};

		enum class image_format {
			image_format_bgr = format_code<1, 3, 0>::value,
			image_format_rgb = format_code<1, 3, 1>::value,
			image_format_yuv = format_code<1, 3, 2>::value,
			image_format_gray = format_code<1, 1, 0>::value,
			image_format_nv12 = format_code<2, 1, 0>::value,
			image_format_nv21 = format_code<2, 1, 1>::value,
		};
	};

	namespace operator_reload {
		//auto operator<<(std::ostream &, const std::string &) {
		//	return 0;
		//}

		template<typename _function>
		auto operator| (bool condition, _function&& function) {
			if (true == condition) {
				return std::forward<_function>(function)();
			}
		};
		
		template<typename _args, typename _function>
		auto operator| (_args&& args, _function&& function) {
			return std::forward<_function>(function)(std::forward<_args>(args));
		};

		//template<typename... _args, typename _function>
		//auto operator| (_args&&... args, const _function& function) {
		//	return function(std::forward<_args>(args)...);
		//};

		//template<typename _function, typename... _args>
		//auto operator| (_function& function, _args&&... args) {
		//	//return std::forward<_function>(function)(std::forward<_args>(args)...);
		//};

		//template<typename _function, typename... _args>
		//auto operator| (bool condition, _function& function, _args&&... args) {
		//	if (true == condition) {
		//		return std::forward<_function>(function)(std::forward<_args>(args)...);
		//	}
		//};

	};
};