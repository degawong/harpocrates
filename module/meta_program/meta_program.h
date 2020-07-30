
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
		decltype(std::declval<_type&>().end()),
		decltype(std::declval<_type&>().begin())
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

	template<typename _diff_type>
	constexpr _diff_type max_possible_v = static_cast<_diff_type>(static_cast<std::make_unsigned_t<_diff_type>>(-1)>> 1);
	
	template<typename _diff_type>
	constexpr _diff_type min_possible_v = -max_possible_v<_diff_type> -1;

}
