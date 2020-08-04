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
#include <xutility>
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

int main() {

	//path = "f:/image/yuv";
	auto path_walker = PathWalker::get_instance();
	auto image_list = path_walker->walk_path(path, ".*\.(bmp|jpg)");

	for (auto ref : image_list) {

		auto image = imread(ref, image_format::image_format_yuv);

		//std::for_each(
		//	//std::execution::par,
		//	image.begin(),
		//	image.end(),
		//	[](auto iter) {
		//	    *iter = 0;
		//    }
		//);

		//for (auto iter : image) {
		//	*iter = 50;
		//}

		auto parallel_1 = [](auto iter) {
			**iter = 100;
			//std::cout << std::this_thread::get_id() << std::endl;
		};

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

		//parallel_execution(image.begin(), image.end(), parallel_2, std::string("harpocrates..."));

		//for (int i = 0; i < 256; ++i) {
		//	auto yuv = image.ptr<uchar>(i);
		//	std::memset(
		//		yuv,
		//		i / 20 * 20,
		//		image.get_pitch(0)
		//	);
		//}

		//for (int i = 0; i < 256; ++i) {
		//	auto yuv = image.ptr<uchar>(i);
		//	for (int j = 0; j < image.get_width(); ++j) {
		//		*yuv = 128;
		//		yuv += 3;
		//	}			
		//}

		imwrite(image, "f:/image/yuv.bmp");
	}

	return 0;
}