/*
 * @autor: degawong
 * @date: Do not edit
 * @lastEditors: degawong
 * @lastEditTime: Do not edit
 * @description: Do not edi
 * @filePath: Do not edit
 */ 

#include <vector>
#include <string>
#include <thread>
#include <iostream>
#include <optional>
#include <execution>

#include <base/base.h>
#include <reflection/reflection.h>
#include <time_clock/time_clock.h>
#include <image_tool/image_tool.h>
#include <path_walker/path_walker.h>
#include <thread_pool/thread_pool.h>
#include <singleton_pattern/singleton_pattern.h>

using namespace harpocrates;
using namespace operator_reload;

std::string path{ "non exist directory" };
std::string image_path{ "f:/image/lena.bmp" };

//#include <vld.h>

int main() {

	Mat image;
	image.read_image(image_path);
	image.write_image(image_path);

	std::for_each(
		//std::execution::par,
		image.begin(),
		image.end(),
		[](auto iter) {
		    *iter = 0;
	    }
	);

	for (auto iter : image) {
		*iter = 255;
	}

	std::cout << pi<float> << std::endl;
	std::cout << radian<float> << std::endl;

	auto path_walker = PathWalker::get_instance();
	auto image_list = path_walker->walk_path(path, ".*\.(bmp|jpg)");
	std::for_each(
		image_list.begin(),
		image_list.end(),
		[&](auto iter) {
		    std::cout << iter << std::endl;
	    }
	);

	auto parallel_1 = [](auto iter) {
		 **iter = 20;
		 return 0;
	};

	parallel_for_each(image.begin(), image.end(), parallel_1);

	auto parallel_2 = [&](auto begin, auto end, auto info) {
		std::for_each(
			image.begin(),
			image.end(),
			[](auto iter) {
			    *iter = 128;
		    }
		);
	};

	parallel_execution(image.begin(), image.end(), parallel_2, std::string("harpocrates..."));

	std::string("operator | reload") | [=](auto iter) {
		std::cout << iter << std::endl;
	};

	return 0;
}