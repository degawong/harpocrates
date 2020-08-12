/*
 * @autor: degawong
 * @date: Do not edit
 * @lastEditors: degawong
 * @lastEditTime: Do not edit
 * @description: Do not edi
 * @filePath: Do not edit
 */

#pragma once
#include <deque>
#include <queue>
#include <regex>
#include <mutex>
#include <chrono>
#include <future>
#include <vector>
#include <thread>
#include <atomic>
#include <string>
#include <cstdlib>
#include <numeric>
#include <iostream>
#include <algorithm>
#include <functional>
#include <condition_variable>

#include <base/base.h>

#include <image_tool/image_mat.h>
#include <reflection/reflection.h>
#include <thread_pool/thread_pool.h>
#include <memory_tool/memory_tool.h>
#include <meta_program/meta_program.h>

#ifdef _android_platform_
#include <arm_neon.h>
#else
#include <base/neon_windows.h>
#endif

namespace harpocrates {
	using namespace type;
	using namespace operator_reload;

	template<int _align_size = 64>
	// downcase word class only for private use
	class image_io {
	public:
		~image_io() = default;
		image_io() : __data(nullptr) {
		};
	public:
		decltype(auto) read_image(std::string image_path) {
			int format = 65792;
			__data = nullptr;
			(std::regex_match(image_path, std::regex(".*\.(bmp|jpg|png)$"))) | [&]() {
				__data = stbi_load(image_path.c_str(), &__width, &__height, &__channel, 0);
			};
			auto image = (nullptr != __data) | [&]() {
				(3 == __channel) | [&]() {
					format = 66305;
				};
				auto image = Mat(__width, __height, format);
				for (int i = 0; i < __height; ++i) {
					std::copy_n(&__data[i * __width * __channel], __width * __channel, image.ptr<uchar>(i));
				}
				return image;
			};
			__dellocate();
			return image;
		}
		[[noreturn]]decltype(auto) write_image(Mat image, std::string image_path) {
			(std::regex_match(image_path, std::regex(".*\.(bmp)$"))) | [&]() {
				(nullptr != (__data = __new_data(image.get_width() * image.get_height() * image.get_elements()))) | [&]() {
					for (int i = 0; i < image.get_height(); ++i) {
						std::copy_n(
							image.ptr<uchar>(i),
							image.get_width() * image.get_elements(),
							&__data[i * image.get_width() * image.get_elements()]
						);
					}
					stbi_write_bmp(image_path.c_str(), image.get_width(), image.get_height(), image.get_elements(), __data);
				};
			};
			__delete_data();
		}
	private:
		decltype(auto) __new_data(int size) {
			return new unsigned char[size];
		}
		decltype(auto) __delete_data() {
			delete[] __data;
			__data = nullptr;
		}
	private:
		auto __dellocate() {
			stbi_image_free(__data);
			__data = nullptr;
		}
	private:
		int __width;
		int __height;
		int __channel;
		alignas(_align_size) unsigned char* __data;
	};

	template<typename _type, typename _format = image_format>
	decltype(auto) imread(_type path, _format format) {
		auto image = image_io().read_image(path);
		every_not_eque(int(format), 66305) | [&]() {
			Mat expect(image.get_width(), image.get_height(), int(format));
			color_convert(image, expect);
			image = expect;
		};
		return image;
	}

	template<typename _type>
	decltype(auto) imwrite(Mat image, _type path) {
		any_equel(image.get_format(), 66305, 65792) | [&]() {
			return image_io().write_image(image, path);
		};
		every_not_eque(image.get_format(), 66305, 65792) | [&]() {
			Mat rgb(image.get_width(), image.get_height(), 66305);
			color_convert(image, rgb);
			return image_io().write_image(rgb, path);
		};
	}
}