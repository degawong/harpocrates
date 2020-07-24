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

	template<typename _type>
	constexpr _type pi { 
		3.141592653589793 
	};

	template<typename _type>
	constexpr _type radian { 
		57.29577951307855
	};

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
	_type_1 clamp(_type_1 min, _type_1 max, _type_2 src) {
		return min(max(min, src), max);
	};

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

	namespace image {
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