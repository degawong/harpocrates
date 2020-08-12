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
	using namespace engine;
	using namespace operator_reload;
	
	using border_signature = meta_hash<type_char<'i', 'm', 'a', 'g', 'e', 'b', 'o', 'r', 'd', 'e', 'r'>>;
	/*
	 Various border types, image boundaries are denoted with '|'
	 * replicate:     aaaaaa|abcdefgh|hhhhhhh
	 * reflect:       fedcba|abcdefgh|hgfedcb
	 * reflect_101:   gfedcb|abcdefgh|gfedcba
	 * wrap:          cdefgh|abcdefgh|abcdefg
	 * constant:      iiiiii|abcdefgh|iiiiiii  with some specified 'i'
	 */
	enum class border_type {
		wrap = 0,
		zero,
		reflect,
		constant,
		replicate,
		reflect_101,
		transparent,
	};
	/**
	 * @author: degawong
	 * @param {p : may <0 || >len }
	 * @param {p : the bordered image location}
	 * @param {len : this specific axis length}
	 * @return {dst position in src position, it is a index}
	 * @description:
	 */
	decltype(auto) border_interpolate(int p, int len, border_type border) {
		(border_type::wrap == border) | [&]() {
			(0 > p) | [&]() {
				p -= ((p - len + 1) / len) * len;
			};
			(len <= p) | [&]() {
				p %= len;
			};
		};
		(border_type::constant == border) | [&]() {
			p = -1;
		};
		(border_type::replicate == border) | [&]() {
			p = p < 0 ? 0 : len - 1;
		};
		any_equel(border, border_type::reflect, border_type::reflect_101) | [&]() {
			int delta = (border_type::reflect_101 == border);
			if (1 == len) {
				p = 0;
			}
			else {
				do {
					(0 > p) | [&]() {
						p = -p - 1 + delta;
					};
					(0 <= p) | [&]() {
						p = len - 1 - (p - len) - delta;
					};
				} while ((unsigned)p >= (unsigned)len);
			}
		};
		return p;
	}

	decltype(auto) zero_border(Mat input, int top, int bottom, int left, int right, const Scalar& bgr, border_type border) {
		const int stride = input.get_pitch(0);
		const int offset = stride - right * input.get_elements();
		((0 != left) || (0 != right)) | [&]() {
			for (int i = top; i < input.get_height() - bottom; ++i) {
				auto in = input.ptr<uchar>(i);
				(0 != left) | [&]() {
					std::memset(in, 0, left * input.get_elements());
				};
				(0 != right) | [&]() {
					std::memset(in + offset, 0, right * input.get_elements());
				};
			}
		};
		for (int i = 0; i < top; ++i) {
			std::memset(input.ptr<uchar>(i), 0, stride);
		}
		for (int i = 0; i < bottom; ++i) {
			std::memset(input.ptr<uchar>(input.get_height() - bottom + i), 0, stride);
		}
	}
	decltype(auto) common_border(Mat input, int top, int bottom, int left, int right, const Scalar& bgr, border_type border) {
		const int roi_width = input.get_width() - left - right;
		const int roi_height = input.get_height() - top - bottom;

		AutoBuff<uchar> auto_buff((left + right) * input.get_elements(), 1, 1); 

		auto left_buff = auto_buff.get_data(0);
		auto right_buff = left_buff + left * input.get_elements();

		for (int i = 0; i < left; ++i) {
			auto index = (border_interpolate(i - left, roi_width, border) + left) * input.get_elements();
			for (int j = 0; j < input.get_elements(); ++j) {
				*left_buff++ = index + j;
			}
		}

		for (int i = 0; i < left; ++i) {
			auto index = (border_interpolate(i + left, roi_width, border) + left) * input.get_elements();
			for (int j = 0; j < input.get_elements(); ++j) {
				*right_buff++ = index + j;
			}
		}
		
		const int stride = input.get_pitch(0);
		const int offset = stride - right * input.get_elements();

		for (int i = top; i < input.get_height() - bottom; ++i) {
			auto in = input.ptr<uchar>(i);
			for (int j = 0; j < left * input.get_elements(); ++j) {
				in[j] = in[left_buff[j]];
			}
			for (int j = 0; j < right * input.get_elements(); ++j) {
				in[offset + j] = in[right_buff[j]];
			}
		}

		for (int i = 0; i < top; ++i) {
			auto index = border_interpolate(i - top, roi_height, border) + top;
			std::memcpy(input.ptr<uchar>(i), input.ptr<uchar>(index), stride);
		}

		for (int i = 0; i < bottom; ++i) {
			auto index = border_interpolate(i + roi_height, roi_height, border) + top;
			std::memcpy(input.ptr<uchar>(input.get_height() - bottom - i), input.ptr<uchar>(index), stride);
		}
	}
	decltype(auto) constant_border(Mat input, int top, int bottom, int left, int right, const Scalar& bgr, border_type border) {
		AutoBuff<uchar> auto_buff(input.get_pitch(), 1, 1);
		auto buff = auto_buff.get_data();
		((0 != top) || (0 != bottom)) | [&]() {
			for (int i = 0; i < input.get_width(); ++i) {
				for (int j = 0; j < input.get_elements(); ++j) {
					*buff++ = bgr[j];
				}
			}
		};
		const int stride = input.get_pitch(0);
		const int offset = stride - right * input.get_elements();
		((0 != left) || (0 != right)) | [&]() {
			for (int i = top; i < input.get_height() - bottom; ++i) {
				auto in = input.ptr<uchar>(i);
				(0 != left) | [&]() {
					std::memcpy(in, buff, left * input.get_elements());
				};
				(0 != right) | [&]() {
					std::memcpy(in + offset, buff, right * input.get_elements());
				};
			}
		};
		for (int i = 0; i < top; ++i) {
			std::memcpy(input.ptr<uchar>(i), buff, stride);
		}
		for (int i = 0; i < bottom; ++i) {
			std::memcpy(input.ptr<uchar>(input.get_height() - bottom + i), buff, stride);
		}
	}

	class BorderEngine final :
		public BaseEngine,
		public uncopyable,
		public SingletonPattern<BorderEngine> {
		using algorithm_handle = std::function<void(Mat, int, int, int, int, const Scalar&, border_type)>;
		using native_handle = decltype(ImplReflection<border_signature::signature, border_type, void, Mat, int, int, int, int, const Scalar&, border_type>::get_instance());
	private:
		BorderEngine() {
			__handle = ImplReflection<border_signature::signature, border_type, void, Mat, int, int, int, int, const Scalar&, border_type>::get_instance();
			__regist_engine();
		}
	public:
		decltype(auto) get_engine() {
			return __handle;
		}
	private:
		virtual void __regist_engine() override {
			__regist_base_engine();
		}
	private:
		virtual void __regist_sse_engine() override {
		}
		virtual void __regist_base_engine() override {
			__handle->regist_factory(border_type::zero, zero_border);
			__handle->regist_factory(border_type::wrap, common_border);
			__handle->regist_factory(border_type::reflect, common_border);
			__handle->regist_factory(border_type::replicate, common_border);
			__handle->regist_factory(border_type::constant, constant_border);
			__handle->regist_factory(border_type::reflect_101, common_border);
		}
		virtual void __regist_neon_engine() override {
		}
		virtual void __regist_opencl_engine() override {
		}
	private:
		native_handle __handle;
	private:
		friend SingletonPattern<BorderEngine>;
	};

	decltype(auto) make_border(Mat input, int top, int bottom, int left, int right, const Scalar& bgr, border_type border = border_type::reflect) {

	}

	decltype(auto) copy_make_border() {

	}

}