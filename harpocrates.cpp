/*
 * @autor: degawong
 * @date: Do not edit
 * @lastEditors: degawong
 * @lastEditTime: Do not edit
 * @description: Do not edi
 * @filePath: Do not edit
 */ 
#include <map>
#include <vector>
#include <string>
#include <thread>
#include <utility>
#include <iterator>
#include <iostream>
#include <optional>
#include <xutility>
#include <execution>
#include <unordered_map>

#include <iostream>
#include <type_traits>
#include <utility>

template <typename T>
struct BoxedType {};

// an instance of BoxedType<T>
template<typename T>
constexpr auto BoxedInstance = BoxedType<T>{};

// function without function body
template<typename T>
constexpr T StripBoxedType(BoxedType<T>);

// SFINAE out using this function without function body
template<typename LambdaType, typename... ArgTypes,
	typename = decltype(std::declval<LambdaType>()(std::declval<ArgTypes&&>()...))>
	std::true_type is_implemented(void*);

// catch all function without function body
template<typename LambdaType, typename... ArgTypes>
std::false_type is_implemented(...);

constexpr auto is_implementation_valid = [](auto lambda_instance)
{
	return [](auto&&... lambda_args)
	{
		return decltype(is_implemented<decltype(lambda_instance), decltype(lambda_args)&&...>(nullptr)){};
	};
};

constexpr auto is_default_constructible_lambda = [](auto boxed_instance)
-> decltype( decltype(StripBoxedType(boxed_instance))()) {};

constexpr auto is_default_constructible_helper = 
	is_implementation_valid(is_default_constructible_lambda);

template<typename T>
constexpr bool is_default_constructible = is_default_constructible_helper(BoxedInstance<T>);

class Constructible
{
public:
	Constructible() = default;
};

class NotConstructible
{
private:
	NotConstructible() = default;
	
public:
	//NotConstructible() = delete;

};

int main()
{
	std::cout << "Is [int] default constructible ? "
		<<std::boolalpha << is_default_constructible_helper(BoxedInstance<int>) << std::endl;

	std::cout << "Is [int&] default constructible ? "
		<< std::boolalpha << is_default_constructible_helper(BoxedInstance<int&>) << std::endl;

	std::cout << "Is [Constructible] default constructible ? "
		<< std::boolalpha << is_default_constructible_helper(BoxedInstance<Constructible>) << std::endl;

	std::cout << "Is [NotConstructible] default constructible ? "
		<< std::boolalpha << is_default_constructible_helper(BoxedInstance<NotConstructible>) << std::endl;

}

