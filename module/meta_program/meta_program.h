/*
 * @autor: degawong
 * @date: Do not edit
 * @lastEditors: degawong
 * @lastEditTime: Do not edit
 * @description: Do not edi
 * @filePath: Do not edit
 */

#pragma once

#include <utility>
#include <iterator>
#include <type_traits>

namespace harpocrates {
	// type void_t can be used to be template partial specialization implement
	template <typename _type, typename = void>
	struct is_iterator : std::false_type {};

	// is_iterator concept
	template <typename _type>
	struct is_iterator <
		_type,
		std::void_t<
		// detect the object has the specific function or not whithout calling the constructor of the class
		decltype(std::declval<_type&>().end()),
		decltype(std::declval<_type&>().begin()),
		decltype(std::declval<_type&>().signature())
		>
	> : std::true_type {
	};

	template<typename _type>
	constexpr bool is_iterator_v = is_iterator<_type>::value;

	template<typename _type>
	void iterator_concept(_type iter) {
		static_assert(is_iterator_v<_type>, "iterator user should have member function begin() and end()");
	}

	template<typename _type>
	constexpr _type pi{
		3.141592653589793
	};

	template<typename _type>
	constexpr _type radian{
		57.29577951307855
	};

	template<typename _type = int>
	constexpr _type shift_number{
		11
	};

	template<typename _diff_type>
	constexpr _diff_type max_possible_v = static_cast<_diff_type>(static_cast<std::make_unsigned_t<_diff_type>>(-1)>> 1);
	
	template<typename _diff_type>
	constexpr _diff_type min_possible_v = -max_possible_v<_diff_type> -1;

	template<typename _function, typename... _args>
	struct invoke_type {
		using type = typename std::result_of<_function(_args...)>::type;
	};

	template<typename _function, typename... _args>
	using invoke_type_t = typename invoke_type<_function, _args...>::type;

	template<char...>
	struct type_char {
	};

	template<typename>
	struct meta_hash {
	};

	template<>
	struct meta_hash<type_char<>> {
		enum { signature = 0 };
	};

	template<char _>
	struct meta_hash<type_char<_>> {
		enum { signature = int64_t(_) };
	};

	template<char _1, char _2>
	struct meta_hash<type_char<_1, _2>> {
		enum { signature = int64_t(_1 << 4) + _2 };
	};

	template<char _1, char _2, char... _>
	struct meta_hash<type_char<_1, _2, _...>> {
		enum { signature = meta_hash<type_char<_1, _2>>::signature + meta_hash<type_char<_...>>::signature };
	};
}
