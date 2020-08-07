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
		image_io() : data(nullptr) {
		};
	public:
		decltype(auto) read_image(std::string image_path) {
			int format = 65792;
			data = nullptr;
			(std::regex_match(image_path, std::regex(".*\.(bmp|jpg|png)$"))) | [this, &image_path]() {
				data = stbi_load(image_path.c_str(), &width, &height, &channel, 0);
			};
			auto image = (nullptr != data) | [&]() {
				(3 == channel) | [&]() {
					format = 66305;
				};
				auto image = Mat(width, height, format);
				for (int i = 0; i < height; ++i) {
					std::copy_n(&data[i * width * channel], width * channel, image.ptr<uchar>(i));
				}
				return image;
			};
			__dellocate();
			return image;
		}
		[[noreturn]]decltype(auto) write_image(Mat image, std::string image_path) {
			int a = image.get_elements();
			int c = a + 1;
			(std::regex_match(image_path, std::regex(".*\.(bmp)$"))) | [&]() {
				(nullptr != (data = __new_data(image.get_width() * image.get_height() * image.get_elements()))) | [&]() {
					for (int i = 0; i < image.get_height(); ++i) {
						std::copy_n(
							image.ptr<uchar>(i),
							image.get_width() * image.get_elements(),
							&data[i * image.get_width() * image.get_elements()]
						);
					}
					stbi_write_bmp(image_path.c_str(), image.get_width(), image.get_height(), image.get_elements(), data);
				};
			};
			__delete_data();
		}
	private:
		decltype(auto) __new_data(int size) {
			return new unsigned char[size];
		}
		decltype(auto) __delete_data() {
			delete[] data;
			data = nullptr;
		}
	private:
		auto __dellocate() {
			stbi_image_free(data);
			data = nullptr;
		}
	private:
		int width;
		int height;
		int channel;
		alignas(_align_size) unsigned char* data;
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
		any_equel(image.get_format_code(), 66305, 65792) | [&]() {
			return image_io().write_image(image, path);
		};
		every_not_eque(image.get_format_code(), 66305, 65792) | [&]() {
			Mat rgb(image.get_width(), image.get_height(), 66305);
			color_convert(image, rgb);
			return image_io().write_image(rgb, path);
		};
	}
}