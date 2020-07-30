
#pragma once

#include <utility>
#include <iterator>
#include <type_traits>

namespace harpocrates {

	template<typename _iter,
		typename = void>
		struct unwrappable
		: std::false_type {
	};

	template<typename _iter>
	struct unwrappable<
		_iter, 
		std::void_t<decltype(std::declval<_iter&>().seek_to(std::declval<const _iter&>().unwrapped()))>>
		: std::true_type{
	};

	template<typename _iter>
	constexpr bool unwrappable_v = unwrappable<_iter>::value;

	template<typename _iter, class = bool>
	struct do_unwrap_when_unverified
		: std::false_type {
	};

	template<typename _iter>
	struct do_unwrap_when_unverified<_iter, decltype(static_cast<bool>(_iter::unwrap_when_unverified))>
		: std::bool_constant<static_cast<bool>(_iter::unwrap_when_unverified)> {
	};

	template<typename _iter>
	constexpr bool do_unwrap_when_unverified_v = do_unwrap_when_unverified<_iter>::value;

	template<typename _iter,
		typename _UIter,
		typename = void>
		struct wrapped_seekable
		: std::false_type {
	};

	template<typename _iter, typename _UIter>
	struct wrapped_seekable<_iter, _UIter, std::void_t<decltype(std::declval<_iter&>().seek_to(std::declval<const _UIter&>()))>>
		: std::true_type{
	};

	template<typename _iter, typename _UIter>
	constexpr bool wrapped_seekable_v = wrapped_seekable<_iter, _UIter>::value;

	template<typename _iter>
	constexpr _iter operator_assert(_iter iter, std::true_type) {
		return (iter);
	};

	template<typename _iter, typename = void>
	struct offset_verifiable
		: std::false_type {
	};

	template<typename _iter>
	struct offset_verifiable<
		_iter,
		std::void_t<decltype(std::declval<const _iter&>().verify_offset(typename _iter::difference_type{})) >>
		: std::true_type {
	};

	template<class _iter>
	constexpr bool offset_verifiable_v = offset_verifiable<iter>::value;

	template<typename _data_type, typename _iter_tag>
	struct iter_base {
		using pointer = _data_type * ;
		using value_type = _data_type;
		using reference = _data_type & ;
		using difference_type = ptrdiff_t;
		using iterator_category = _iter_tag;
		using const_pointer = const _data_type *;
		using const_reference = const _data_type &;
	};

	template<typename _data_type, typename _iter_tag = std::random_access_iterator_tag>
	struct iter : public iter_base<_data_type, _iter_tag> {
		using _base = iter_base<_data_type, _iter_tag>;
		using pointer = typename _base::pointer;
		using reference = typename _base::reference;
		using value_type = typename _base::value_type;
		using difference_type = typename _base::difference_type;
		using iterator_category = typename _base::iterator_category;
	};

	template<typename _data_type, typename _iter_tag = std::random_access_iterator_tag>
	struct const_iter : public iter_base<_data_type, _iter_tag> {
		using _base = iter_base<_data_type, _iter_tag>;
		using value_type = typename _base::value_type;
		using pointer = typename _base::const_pointer;
		using reference = typename _base::const_reference;
		using difference_type = typename _base::difference_type;
		using iterator_category = typename _base::iterator_category;
	};

	template<typename _iter_type>
	class reverse_iterator {
	public:
		using iterator_type = _iter_type;
		using pointer = typename _iter_type::pointer;
		using reference = typename _iter_type::reference;
		using value_type = typename _iter_type::value_type;
		using difference_type = typename _iter_type::difference_type;
		using iterator_category = typename _iter_type::iterator_category;

		reverse_iterator() 
			: __verbose() {
		}

		explicit reverse_iterator(_iter_type _right) 
			: __verbose(_right) {
		}

		template<class _other>
		reverse_iterator(const reverse_iterator<_other>& right)
			: __verbose(right.base()) {
		}

		template<class _other>
		reverse_iterator& operator=(const reverse_iterator<_other>& right) {
			__verbose = right.base();
			return (*this);
		}

		_iter_type base() const {
			return (__verbose);
		}

		reference operator*() const {
			_iter_type temp = __verbose;
			return (*--temp);
		}

		pointer operator->() const {
			_iter_type temp = __verbose;
			--temp;
			return (operator_assert(temp, std::is_pointer<_iter_type>()));
		}

		reverse_iterator& operator++() {
			--__verbose;
			return (*this);
		}

		reverse_iterator operator++(int) {
			reverse_iterator temp = *this;
			--__verbose;
			return (temp);
		}

		reverse_iterator& operator--() {
			++__verbose;
			return (*this);
		}

		reverse_iterator operator--(int) {
			reverse_iterator temp = *this;
			++__verbose;
			return (temp);
		}

		_reverse_iterator& operator+=(const difference_type offset) {
			__verbose -= offset;
			return (*this);
		}

		reverse_iterator operator+(const difference_type offset) const {
			return (reverse_iterator(__verbose - offset));
		}

		reverse_iterator& operator-=(const difference_type offset) {
			__verbose += offset;
			return (*this);
		}

		reverse_iterator operator-(const difference_type offset) const {
			return (reverse_iterator(__verbose + offset));
		}

		reference operator[](const difference_type offset) const {
			return (*(*this + offset));
		}

		template<typename _other_type = _iter_type, std::enable_if_t<offset_verifiable_v<_other_type>, int> = 0>
		constexpr void verify_offset(const difference_type offset) const {
			__verbose.verify_offset(-offset);
		}
		//_Unwrappable函数的作用是检查T是否同时具有_Unwrapped和_Seek_to函数, 
		//以及确保_Unwrapped可以作为参数传递给_Seek_to. 如果两个条件都满足, 
		//那么它就是一个 _Unwrappable 对象, 即: 是一个迭代器.
		template<typename _other_type = _iter_type, std::enable_if_t<unwrappable_v<_other_type>, int> = 0>
		constexpr reverse_iterator<unwrapped_t<_other_type>> unwrapped() const {
			return (static_cast<reverse_iterator<unwrapped_t<_other_type>>>(__verbose.unwrapped()));
		}

		static constexpr bool unwrap_when_unverified = do_unwrap_when_unverified_v<_iter_type>;

		template<
			typename _iter,
			std::enable_if_t<wrapped_seekable_v<_iter_type, _iter>, int> = 0>
			constexpr void seek_to(const reverse_iterator<_iter>& iter) {
			__verbose.seek_to(iter.base());
		}
	protected:
		_iter_type __verbose;	// the wrapped iterator
	};

	namespace image_iterator {
		template<typename _image>
		class iterator : public const_iter<_image> {
		public:
			using _base = const_iter<_image>;
			using pointer = typename _image::pointer;
			using reference = typename _image::value_type&;
			using value_type = typename _image::value_type;
			using difference_type = typename _image::difference_type;
			using iterator_category = typename _image::iterator_category;
		public:
			iterator() = default;

			pointer operator->() const {
				return (const_cast<pointer>(_base::operator->()));
			}

			reference operator*() const {
				return (const_cast<reference>(_base::operator*()));
			}

			iterator& operator++() {
				++*(_base *)this;
				return (*this);
			}

			iterator operator++(int)
			{	// postincrement
				iterator _Tmp = *this;
				++*this;
				return (_Tmp);
			}

			iterator& operator--()
			{	// predecrement
				--*(_Mybase *)this;
				return (*this);
			}

			iterator operator--(int)
			{	// postdecrement
				iterator _Tmp = *this;
				--*this;
				return (_Tmp);
			}

			iterator& operator+=(const difference_type _Off)
			{	// increment by integer
				*(_Mybase *)this += _Off;
				return (*this);
			}

			_NODISCARD iterator operator+(const difference_type _Off) const
			{	// return this + integer
				iterator _Tmp = *this;
				return (_Tmp += _Off);
			}

			iterator& operator-=(const difference_type _Off)
			{	// decrement by integer
				return (*this += -_Off);
			}

			_NODISCARD iterator operator-(const difference_type _Off) const
			{	// return this - integer
				iterator _Tmp = *this;
				return (_Tmp -= _Off);
			}

			_NODISCARD difference_type operator-(const _Mybase& _Right) const
			{	// return difference of iterators
				return (*(_Mybase *)this - _Right);
			}

			_NODISCARD reference operator[](const difference_type _Off) const
			{	// subscript
				return (*(*this + _Off));
			}

			_NODISCARD pointer _Unwrapped() const
			{
				return (this->_Ptr);
			}
		};
	}

}
