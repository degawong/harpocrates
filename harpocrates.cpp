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
#include <utility>
#include <iterator>
#include <iostream>
#include <optional>
#include <execution>

#include <base/base.h>
#include <reflection/reflection.h>
#include <time_clock/time_clock.h>
#include <image_tool/image_tool.h>
#include <path_walker/path_walker.h>
#include <thread_pool/thread_pool.h>
#include <meta_program/meta_program.h>
#include <singleton_pattern/singleton_pattern.h>

using namespace harpocrates;
using namespace image;
using namespace operator_reload;

std::string path{ "non exist directory" };

//#include <vld.h>

int main() {

	auto path_walker = PathWalker::get_instance();
	auto image_list = path_walker->walk_path(path, ".*\.(bmp|jpg)");

	for (auto ref : image_list) {

		auto image = imread(ref, image_format::image_format_nv12);

		std::for_each(
			//std::execution::par,
			image.begin(),
			image.end(),
			[](auto iter) {
			    *iter = 0;
		    }
		);

		for (auto iter : image) {
			*iter = 50;
		}

		//auto parallel_1 = [](auto iter) {
		//	**iter = 100;
		//	//std::cout << std::this_thread::get_id() << std::endl;
		//};

		//parallel_for_each(image.begin(), image.end(), parallel_1);

		auto parallel_2 = [&](auto begin, auto end, auto info) {
			std::for_each(
				begin,
				end,
				[](auto iter) {
				    *iter = 150;
			    }
			);
		};

		parallel_execution(image.begin(), image.end(), parallel_2, std::string("harpocrates..."));

		//imwrite(image, image_path);
	}

	std::string("operator | reload") | [&](auto iter) {
		std::cout << iter << std::endl;
	};

	return 0;
}