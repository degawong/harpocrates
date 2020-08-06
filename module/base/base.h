/*
 * @Description: 
 * @Autor: degawong
 * @Date: 2020-04-15 17:24:12
 * @LastEditors: Please set LastEditors
 * @LastEditTime: 2020-07-23 18:55:55
 */
#pragma once

namespace harpocrates {

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

	template<typename _type>
	inline int fast_round(_type x) {
		static_assert(!std::numeric_limits<_type>::is_integer, "fast_round can only handle float point number");
		return x > 0 ? static_cast<int>(x + 0.5) : 0;
	}

	template<typename _type>
	inline int fast_floor(_type x) {
		static_assert(!std::numeric_limits<_type>::is_integer, "fast_floor can only handle float point number");
		return x > 0 ? static_cast<int>(x) : 0;
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

	template<typename _type>
	// downcase word class only for private use
	struct base_type {
		using value_type = _type;
		using size_type = size_t;
		using pointer = value_type *;
		using difference_type = ptrdiff_t;
		using const_pointer = const value_type *;
		using reference = value_type &;
		using const_reference = const value_type &;
	};

	template<>
	// downcase word class only for private use
	struct base_type<void> {
		using value_type = void;
		using size_type = size_t;
		using pointer = value_type *;
		using difference_type = ptrdiff_t;
		using const_pointer = const value_type *;
	};

	namespace type {
		using handle = void *;

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