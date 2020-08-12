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

	using color_signature = meta_hash<type_char<'c', 'o', 'l', 'r', 'c', 'o', 'n', 'v', 'e', 'r', 't'>>;
	// color convert
	decltype(auto) color_convert_bgr_2_yuv_chunk(int begin, int end, Mat input, Mat output) {
		for (int i = begin; i < end; ++i) {
			auto in = input.ptr<uchar>(i);
			auto out = output.ptr<uchar>(i);
			for (int j = 0; j < input.get_width(); ++j) {
				uchar b = *in++;
				uchar g = *in++;
				uchar r = *in++;
				*out++ = std::clamp(int(0.299 * r + 0.587 * g + 0.114 * b), 0, 255);
				*out++ = std::clamp(int(-0.169 * r - 0.331 * g + 0.5 * b + 128), 0, 255);
				*out++ = std::clamp(int(0.5 * r - 0.419 * g - 0.081 * b + 128), 0, 255);
			}
		}
	}
	decltype(auto) color_convert_rgb_2_yuv_chunk(int begin, int end, Mat input, Mat output) {
		for (int i = begin; i < end; ++i) {
			auto in = input.ptr<uchar>(i);
			auto out = output.ptr<uchar>(i);
			for (int j = 0; j < input.get_width(); ++j) {
				uchar r = *in++;
				uchar g = *in++;
				uchar b = *in++;
				*out++ = std::clamp(int(0.299 * r + 0.587 * g + 0.114 * b), 0, 255);
				*out++ = std::clamp(int(-0.169 * r - 0.331 * g + 0.5 * b + 128), 0, 255);
				*out++ = std::clamp(int(0.5 * r - 0.419 * g - 0.081 * b + 128), 0, 255);
			}
		}
	}
	decltype(auto) color_convert_yuv_2_bgr_chunk(int begin, int end, Mat input, Mat output) {
		for (int i = begin; i < end; ++i) {
			auto in = input.ptr<uchar>(i);
			auto out = output.ptr<uchar>(i);
			for (int j = 0; j < input.get_width(); ++j) {
				uchar y = *in++;
				uchar u = *in++;
				uchar v = *in++;
				*out++ = std::clamp(int(y + 1.779 * (u - 128)), 0, 255);
				*out++ = std::clamp(int(y - 0.3455 * (u - 128) - 0.7169 * (v - 128)), 0, 255);
				*out++ = std::clamp(int(y + 1.4075 * (v - 128)), 0, 255);
			}
		}
	}
	decltype(auto) color_convert_yuv_2_rgb_chunk(int begin, int end, Mat input, Mat output) {
		for (int i = begin; i < end; ++i) {
			auto in = input.ptr<uchar>(i);
			auto out = output.ptr<uchar>(i);
			for (int j = 0; j < input.get_width(); ++j) {
				uchar y = *in++;
				uchar u = *in++;
				uchar v = *in++;
				*out++ = std::clamp(int(y + 1.4075 * (v - 128)), 0, 255);
				*out++ = std::clamp(int(y - 0.3455 * (u - 128) - 0.7169 * (v - 128)), 0, 255);
				*out++ = std::clamp(int(y + 1.779 * (u - 128)), 0, 255);
			}
		}
	}

	decltype(auto) color_convert_bgr_2_nv12_chunk(int begin, int end, Mat input, Mat output) {
		int u, v;
		uchar b, g, r;
		for (int i = (begin >> 1); i < (end >> 1); ++i) {
			auto uv_plane = output.get_data(1, i);
			auto y_pre = output.ptr<uchar>(2 * i + 0);
			auto y_post = output.ptr<uchar>(2 * i + 1);
			auto bgr_pre = input.ptr<uchar>(2 * i + 0);
			auto bgr_post = input.ptr<uchar>(2 * i + 1);
			for (int j = 0; j < input.get_width() >> 1; ++j) {
				b = *bgr_pre++;
				g = *bgr_pre++;
				r = *bgr_pre++;
				*y_pre++ = std::clamp(int(0.299 * r + 0.587 * g + 0.114 * b), 0, 255);
				u = std::clamp(int(-0.169 * r - 0.331 * g + 0.5 * b + 128), 0, 255);
				v = std::clamp(int(0.5 * r - 0.419 * g - 0.081 * b + 128), 0, 255);
				b = *bgr_pre++;
				g = *bgr_pre++;
				r = *bgr_pre++;
				*y_pre++ = std::clamp(int(0.299 * r + 0.587 * g + 0.114 * b), 0, 255);
				u += std::clamp(int(-0.169 * r - 0.331 * g + 0.5 * b + 128), 0, 255);
				v += std::clamp(int(0.5 * r - 0.419 * g - 0.081 * b + 128), 0, 255);
				b = *bgr_post++;
				g = *bgr_post++;
				r = *bgr_post++;
				*y_post++ = std::clamp(int(0.299 * r + 0.587 * g + 0.114 * b), 0, 255);
				u += std::clamp(int(-0.169 * r - 0.331 * g + 0.5 * b + 128), 0, 255);
				v += std::clamp(int(0.5 * r - 0.419 * g - 0.081 * b + 128), 0, 255);
				b = *bgr_post++;
				g = *bgr_post++;
				r = *bgr_post++;
				*y_post++ = std::clamp(int(0.299 * r + 0.587 * g + 0.114 * b), 0, 255);
				u += std::clamp(int(-0.169 * r - 0.331 * g + 0.5 * b + 128), 0, 255);
				v += std::clamp(int(0.5 * r - 0.419 * g - 0.081 * b + 128), 0, 255);
				*uv_plane++ = u >> 2;
				*uv_plane++ = v >> 2;
			}
		}
	}
	decltype(auto) color_convert_rgb_2_nv12_chunk(int begin, int end, Mat input, Mat output) {
		int u, v;
		uchar b, g, r;
		for (int i = (begin >> 1); i < (end >> 1); ++i) {
			auto uv_plane = output.get_data(1, i);
			auto y_pre = output.ptr<uchar>(2 * i + 0);
			auto y_post = output.ptr<uchar>(2 * i + 1);
			auto bgr_pre = input.ptr<uchar>(2 * i + 0);
			auto bgr_post = input.ptr<uchar>(2 * i + 1);
			for (int j = 0; j < input.get_width() >> 1; ++j) {
				r = *bgr_pre++;
				g = *bgr_pre++;
				b = *bgr_pre++;
				*y_pre++ = std::clamp(int(0.299 * r + 0.587 * g + 0.114 * b), 0, 255);
				u = std::clamp(int(-0.169 * r - 0.331 * g + 0.5 * b + 128), 0, 255);
				v = std::clamp(int(0.5 * r - 0.419 * g - 0.081 * b + 128), 0, 255);
				r = *bgr_pre++;
				g = *bgr_pre++;
				b = *bgr_pre++;
				*y_pre++ = std::clamp(int(0.299 * r + 0.587 * g + 0.114 * b), 0, 255);
				u += std::clamp(int(-0.169 * r - 0.331 * g + 0.5 * b + 128), 0, 255);
				v += std::clamp(int(0.5 * r - 0.419 * g - 0.081 * b + 128), 0, 255);
				r = *bgr_post++;
				g = *bgr_post++;
				b = *bgr_post++;
				*y_post++ = std::clamp(int(0.299 * r + 0.587 * g + 0.114 * b), 0, 255);
				u += std::clamp(int(-0.169 * r - 0.331 * g + 0.5 * b + 128), 0, 255);
				v += std::clamp(int(0.5 * r - 0.419 * g - 0.081 * b + 128), 0, 255);
				r = *bgr_post++;
				g = *bgr_post++;
				b = *bgr_post++;
				*y_post++ = std::clamp(int(0.299 * r + 0.587 * g + 0.114 * b), 0, 255);
				u += std::clamp(int(-0.169 * r - 0.331 * g + 0.5 * b + 128), 0, 255);
				v += std::clamp(int(0.5 * r - 0.419 * g - 0.081 * b + 128), 0, 255);
				*uv_plane++ = u >> 2;
				*uv_plane++ = v >> 2;
			}
		}
	}
	decltype(auto) color_convert_nv12_2_bgr_chunk(int begin, int end, Mat input, Mat output) {
		uchar y, u, v;
		for (int i = (begin >> 1); i < (end >> 1); ++i) {
			auto uv_plane = input.get_data(1, i);
			auto y_pre = input.ptr<uchar>(2 * i + 0);
			auto y_post = input.ptr<uchar>(2 * i + 1);
			auto bgr_pre = output.ptr<uchar>(2 * i + 0);
			auto bgr_post = output.ptr<uchar>(2 * i + 1);
			for (int j = 0; j < input.get_width() >> 1; ++j) {
				y = *y_pre++;
				u = *uv_plane++;
				v = *uv_plane++;
				*bgr_pre++ = std::clamp(int(y + 1.779 * (u - 128)), 0, 255);
				*bgr_pre++ = std::clamp(int(y - 0.3455 * (u - 128) - 0.7169 * (v - 128)), 0, 255);
				*bgr_pre++ = std::clamp(int(y + 1.4075 * (v - 128)), 0, 255);
				y = *y_pre++;
				*bgr_pre++ = std::clamp(int(y + 1.779 * (u - 128)), 0, 255);
				*bgr_pre++ = std::clamp(int(y - 0.3455 * (u - 128) - 0.7169 * (v - 128)), 0, 255);
				*bgr_pre++ = std::clamp(int(y + 1.4075 * (v - 128)), 0, 255);
				y = *y_post++;
				*bgr_post++ = std::clamp(int(y + 1.779 * (u - 128)), 0, 255);
				*bgr_post++ = std::clamp(int(y - 0.3455 * (u - 128) - 0.7169 * (v - 128)), 0, 255);
				*bgr_post++ = std::clamp(int(y + 1.4075 * (v - 128)), 0, 255);
				y = *y_post++;
				*bgr_post++ = std::clamp(int(y + 1.779 * (u - 128)), 0, 255);
				*bgr_post++ = std::clamp(int(y - 0.3455 * (u - 128) - 0.7169 * (v - 128)), 0, 255);
				*bgr_post++ = std::clamp(int(y + 1.4075 * (v - 128)), 0, 255);
			}
		}
	}
	decltype(auto) color_convert_nv12_2_rgb_chunk(int begin, int end, Mat input, Mat output) {
		uchar y, u, v;
		for (int i = (begin >> 1); i < (end >> 1); ++i) {
			auto uv_plane = input.get_data(1, i);
			auto y_pre = input.ptr<uchar>(2 * i + 0);
			auto y_post = input.ptr<uchar>(2 * i + 1);
			auto bgr_pre = output.ptr<uchar>(2 * i + 0);
			auto bgr_post = output.ptr<uchar>(2 * i + 1);
			for (int j = 0; j < input.get_width() >> 1; ++j) {
				y = *y_pre++;
				u = *uv_plane++;
				v = *uv_plane++;
				*bgr_pre++ = std::clamp(int(y + 1.4075 * (v - 128)), 0, 255);
				*bgr_pre++ = std::clamp(int(y - 0.3455 * (u - 128) - 0.7169 * (v - 128)), 0, 255);
				*bgr_pre++ = std::clamp(int(y + 1.779 * (u - 128)), 0, 255);
				y = *y_pre++;
				*bgr_pre++ = std::clamp(int(y + 1.4075 * (v - 128)), 0, 255);
				*bgr_pre++ = std::clamp(int(y - 0.3455 * (u - 128) - 0.7169 * (v - 128)), 0, 255);
				*bgr_pre++ = std::clamp(int(y + 1.779 * (u - 128)), 0, 255);
				y = *y_post++;
				*bgr_post++ = std::clamp(int(y + 1.4075 * (v - 128)), 0, 255);
				*bgr_post++ = std::clamp(int(y - 0.3455 * (u - 128) - 0.7169 * (v - 128)), 0, 255);
				*bgr_post++ = std::clamp(int(y + 1.779 * (u - 128)), 0, 255);
				y = *y_post++;
				*bgr_post++ = std::clamp(int(y + 1.4075 * (v - 128)), 0, 255);
				*bgr_post++ = std::clamp(int(y - 0.3455 * (u - 128) - 0.7169 * (v - 128)), 0, 255);
				*bgr_post++ = std::clamp(int(y + 1.779 * (u - 128)), 0, 255);
			}
		}
	}

	decltype(auto) color_convert_bgr_2_nv21_chunk(int begin, int end, Mat input, Mat output) {
		int u, v;
		uchar b, g, r;
		for (int i = (begin >> 1); i < (end >> 1); ++i) {
			auto uv_plane = output.get_data(1, i);
			auto y_pre = output.ptr<uchar>(2 * i + 0);
			auto y_post = output.ptr<uchar>(2 * i + 1);
			auto bgr_pre = input.ptr<uchar>(2 * i + 0);
			auto bgr_post = input.ptr<uchar>(2 * i + 1);
			for (int j = 0; j < input.get_width() >> 1; ++j) {
				b = *bgr_pre++;
				g = *bgr_pre++;
				r = *bgr_pre++;
				*y_pre++ = std::clamp(int(0.299 * r + 0.587 * g + 0.114 * b), 0, 255);
				u = std::clamp(int(-0.169 * r - 0.331 * g + 0.5 * b + 128), 0, 255);
				v = std::clamp(int(0.5 * r - 0.419 * g - 0.081 * b + 128), 0, 255);
				b = *bgr_pre++;
				g = *bgr_pre++;
				r = *bgr_pre++;
				*y_pre++ = std::clamp(int(0.299 * r + 0.587 * g + 0.114 * b), 0, 255);
				u += std::clamp(int(-0.169 * r - 0.331 * g + 0.5 * b + 128), 0, 255);
				v += std::clamp(int(0.5 * r - 0.419 * g - 0.081 * b + 128), 0, 255);
				b = *bgr_post++;
				g = *bgr_post++;
				r = *bgr_post++;
				*y_post++ = std::clamp(int(0.299 * r + 0.587 * g + 0.114 * b), 0, 255);
				u += std::clamp(int(-0.169 * r - 0.331 * g + 0.5 * b + 128), 0, 255);
				v += std::clamp(int(0.5 * r - 0.419 * g - 0.081 * b + 128), 0, 255);
				b = *bgr_post++;
				g = *bgr_post++;
				r = *bgr_post++;
				*y_post++ = std::clamp(int(0.299 * r + 0.587 * g + 0.114 * b), 0, 255);
				u += std::clamp(int(-0.169 * r - 0.331 * g + 0.5 * b + 128), 0, 255);
				v += std::clamp(int(0.5 * r - 0.419 * g - 0.081 * b + 128), 0, 255);
				*uv_plane++ = v >> 2;
				*uv_plane++ = u >> 2;
			}
		}
	}
	decltype(auto) color_convert_rgb_2_nv21_chunk(int begin, int end, Mat input, Mat output) {
		int u, v;
		uchar b, g, r;
		for (int i = (begin >> 1); i < (end >> 1); ++i) {
			auto uv_plane = output.get_data(1, i);
			auto y_pre = output.ptr<uchar>(2 * i + 0);
			auto y_post = output.ptr<uchar>(2 * i + 1);
			auto bgr_pre = input.ptr<uchar>(2 * i + 0);
			auto bgr_post = input.ptr<uchar>(2 * i + 1);
			for (int j = 0; j < input.get_width() >> 1; ++j) {
				r = *bgr_pre++;
				g = *bgr_pre++;
				b = *bgr_pre++;
				*y_pre++ = std::clamp(int(0.299 * r + 0.587 * g + 0.114 * b), 0, 255);
				u = std::clamp(int(-0.169 * r - 0.331 * g + 0.5 * b + 128), 0, 255);
				v = std::clamp(int(0.5 * r - 0.419 * g - 0.081 * b + 128), 0, 255);
				r = *bgr_pre++;
				g = *bgr_pre++;
				b = *bgr_pre++;
				*y_pre++ = std::clamp(int(0.299 * r + 0.587 * g + 0.114 * b), 0, 255);
				u += std::clamp(int(-0.169 * r - 0.331 * g + 0.5 * b + 128), 0, 255);
				v += std::clamp(int(0.5 * r - 0.419 * g - 0.081 * b + 128), 0, 255);
				r = *bgr_post++;
				g = *bgr_post++;
				b = *bgr_post++;
				*y_post++ = std::clamp(int(0.299 * r + 0.587 * g + 0.114 * b), 0, 255);
				u += std::clamp(int(-0.169 * r - 0.331 * g + 0.5 * b + 128), 0, 255);
				v += std::clamp(int(0.5 * r - 0.419 * g - 0.081 * b + 128), 0, 255);
				r = *bgr_post++;
				g = *bgr_post++;
				b = *bgr_post++;
				*y_post++ = std::clamp(int(0.299 * r + 0.587 * g + 0.114 * b), 0, 255);
				u += std::clamp(int(-0.169 * r - 0.331 * g + 0.5 * b + 128), 0, 255);
				v += std::clamp(int(0.5 * r - 0.419 * g - 0.081 * b + 128), 0, 255);
				*uv_plane++ = v >> 2;
				*uv_plane++ = u >> 2;
			}
		}
	}
	decltype(auto) color_convert_nv21_2_bgr_chunk(int begin, int end, Mat input, Mat output) {
		uchar y, u, v;
		for (int i = (begin >> 1); i < (end >> 1); ++i) {
			auto uv_plane = input.get_data(1, i);
			auto y_pre = input.ptr<uchar>(2 * i + 0);
			auto y_post = input.ptr<uchar>(2 * i + 1);
			auto bgr_pre = output.ptr<uchar>(2 * i + 0);
			auto bgr_post = output.ptr<uchar>(2 * i + 1);
			for (int j = 0; j < input.get_width() >> 1; ++j) {
				y = *y_pre++;
				v = *uv_plane++;
				u = *uv_plane++;
				*bgr_pre++ = std::clamp(int(y + 1.779 * (u - 128)), 0, 255);
				*bgr_pre++ = std::clamp(int(y - 0.3455 * (u - 128) - 0.7169 * (v - 128)), 0, 255);
				*bgr_pre++ = std::clamp(int(y + 1.4075 * (v - 128)), 0, 255);
				y = *y_pre++;
				*bgr_pre++ = std::clamp(int(y + 1.779 * (u - 128)), 0, 255);
				*bgr_pre++ = std::clamp(int(y - 0.3455 * (u - 128) - 0.7169 * (v - 128)), 0, 255);
				*bgr_pre++ = std::clamp(int(y + 1.4075 * (v - 128)), 0, 255);
				y = *y_post++;
				*bgr_post++ = std::clamp(int(y + 1.779 * (u - 128)), 0, 255);
				*bgr_post++ = std::clamp(int(y - 0.3455 * (u - 128) - 0.7169 * (v - 128)), 0, 255);
				*bgr_post++ = std::clamp(int(y + 1.4075 * (v - 128)), 0, 255);
				y = *y_post++;
				*bgr_post++ = std::clamp(int(y + 1.779 * (u - 128)), 0, 255);
				*bgr_post++ = std::clamp(int(y - 0.3455 * (u - 128) - 0.7169 * (v - 128)), 0, 255);
				*bgr_post++ = std::clamp(int(y + 1.4075 * (v - 128)), 0, 255);
			}
		}
	}
	decltype(auto) color_convert_nv21_2_rgb_chunk(int begin, int end, Mat input, Mat output) {
		uchar y, u, v;
		for (int i = (begin >> 1); i < (end >> 1); ++i) {
			auto uv_plane = input.get_data(1, i);
			auto y_pre = input.ptr<uchar>(2 * i + 0);
			auto y_post = input.ptr<uchar>(2 * i + 1);
			auto bgr_pre = output.ptr<uchar>(2 * i + 0);
			auto bgr_post = output.ptr<uchar>(2 * i + 1);
			for (int j = 0; j < input.get_width() >> 1; ++j) {
				y = *y_pre++;
				v = *uv_plane++;
				u = *uv_plane++;
				*bgr_pre++ = std::clamp(int(y + 1.4075 * (v - 128)), 0, 255);
				*bgr_pre++ = std::clamp(int(y - 0.3455 * (u - 128) - 0.7169 * (v - 128)), 0, 255);
				*bgr_pre++ = std::clamp(int(y + 1.779 * (u - 128)), 0, 255);
				y = *y_pre++;
				*bgr_pre++ = std::clamp(int(y + 1.4075 * (v - 128)), 0, 255);
				*bgr_pre++ = std::clamp(int(y - 0.3455 * (u - 128) - 0.7169 * (v - 128)), 0, 255);
				*bgr_pre++ = std::clamp(int(y + 1.779 * (u - 128)), 0, 255);
				y = *y_post++;
				*bgr_post++ = std::clamp(int(y + 1.4075 * (v - 128)), 0, 255);
				*bgr_post++ = std::clamp(int(y - 0.3455 * (u - 128) - 0.7169 * (v - 128)), 0, 255);
				*bgr_post++ = std::clamp(int(y + 1.779 * (u - 128)), 0, 255);
				y = *y_post++;
				*bgr_post++ = std::clamp(int(y + 1.4075 * (v - 128)), 0, 255);
				*bgr_post++ = std::clamp(int(y - 0.3455 * (u - 128) - 0.7169 * (v - 128)), 0, 255);
				*bgr_post++ = std::clamp(int(y + 1.779 * (u - 128)), 0, 255);
			}
		}
	}

	decltype(auto) color_convert_bgr_2_gray_neon_chunk(int begin, int end, Mat input, Mat output) {
		int gray;
		int j = 0;
		int b, g, r;
		uint8x8x3_t bgr;
		int16x8_t v_b, v_g, v_r, v_gray;

		int16x8_t v_29, v_149;

		v_29 = vdupq_n_s16(29);
		v_149 = vdupq_n_s16(149);

		for (int i = begin; i < end; ++i) {
			auto bgr_plane = input.ptr<uchar>(i);
			auto gray_plane = output.ptr<uchar>(i);
			for (j = 0; j < input.get_width() - 8; j += 8) {
				bgr = vld3_u8(bgr_plane);

				v_b = vreinterpretq_s16_u16(vmovl_u8(bgr.val[0]));
				v_g = vreinterpretq_s16_u16(vmovl_u8(bgr.val[1]));
				v_r = vreinterpretq_s16_u16(vmovl_u8(bgr.val[2]));

				v_gray = vmlaq_s16(vmulq_n_s16(v_r, 76), v_g, v_149);
				v_gray = vshrq_n_s16(vmlaq_s16(v_gray, v_b, v_29), 8);

				vst1_u8(gray_plane, vqmovun_s16(v_gray));

				bgr_plane += 24;
				gray_plane += 8;
			}
			for (; j < input.get_width(); ++j)
			{
				b = *bgr_plane++;
				g = *bgr_plane++;
				r = *bgr_plane++;

				gray = ((76 * r) + (149 * g) + (29 * b)) >> 8;
				*(gray_plane++) = std::clamp(gray, 0, 255);
			}
		}
	}
	decltype(auto) color_convert_rgb_2_gray_neon_chunk(int begin, int end, Mat input, Mat output) {
		int gray;
		int j = 0;
		int b, g, r;
		uint8x8x3_t bgr;
		int16x8_t v_b, v_g, v_r, v_gray;

		int16x8_t v_29, v_149;

		v_29 = vdupq_n_s16(29);
		v_149 = vdupq_n_s16(149);

		for (int i = begin; i < end; ++i) {
			auto bgr_plane = input.ptr<uchar>(i);
			auto gray_plane = output.ptr<uchar>(i);
			for (j = 0; j < input.get_width() - 8; j += 8) {
				bgr = vld3_u8(bgr_plane);

				v_r = vreinterpretq_s16_u16(vmovl_u8(bgr.val[0]));
				v_g = vreinterpretq_s16_u16(vmovl_u8(bgr.val[1]));
				v_b = vreinterpretq_s16_u16(vmovl_u8(bgr.val[2]));

				v_gray = vmlaq_s16(vmulq_n_s16(v_r, 76), v_g, v_149);
				v_gray = vshrq_n_s16(vmlaq_s16(v_gray, v_b, v_29), 8);

				vst1_u8(gray_plane, vqmovun_s16(v_gray));

				bgr_plane += 24;
				gray_plane += 8;
			}
			for (; j < input.get_width(); ++j)
			{
				r = *bgr_plane++;
				g = *bgr_plane++;
				b = *bgr_plane++;

				gray = ((76 * r) + (149 * g) + (29 * b)) >> 8;
				*(gray_plane++) = std::clamp(gray, 0, 255);
			}
		}
	}

	decltype(auto) color_convert_bgr_2_yuv_neon_chunk(int begin, int end, Mat input, Mat output) {
		int j = 0;
		int b, g, r;
		int y, u, v;
		int16x8_t v_b, v_g, v_r;
		int16x8_t v_y, v_u, v_v;
		uint8x8x3_t bgr, yuv;

		int16x8_t v_38, v_7, v_9, v_91, v128x4u, v128x7v;

		v_7 = vdupq_n_s16(7);
		v_9 = vdupq_n_s16(9);
		v_38 = vdupq_n_s16(38);
		v_91 = vdupq_n_s16(91);
		v128x4u = vdupq_n_s16(128 * 16);
		v128x7v = vdupq_n_s16(128 * 128);

		for (int i = begin; i < end; ++i) {
			auto bgr_plane = input.ptr<uchar>(i);
			auto yuv_plane = output.ptr<uchar>(i);
			for (j = 0; j < input.get_width() - 8; j += 8) {
				bgr = vld3_u8(bgr_plane);

				v_b = vreinterpretq_s16_u16(vmovl_u8(bgr.val[0]));
				v_g = vreinterpretq_s16_u16(vmovl_u8(bgr.val[1]));
				v_r = vreinterpretq_s16_u16(vmovl_u8(bgr.val[2]));

				v_y = vmlaq_s16(vmulq_n_s16(v_r, 19), v_g, v_38);
				v_y = vshrq_n_s16(vmlaq_s16(v_y, v_b, v_7), 6);

				v_u = vmlaq_s16(v128x4u, vsubq_s16(v_b, v_y), v_9);
				v_u = vshrq_n_s16(v_u, 4);

				v_v = vmlaq_s16(v128x7v, vsubq_s16(v_r, v_y), v_91);
				v_v = vshrq_n_s16(v_v, 7);

				yuv.val[0] = vqmovun_s16(v_y);
				yuv.val[1] = vqmovun_s16(v_u);
				yuv.val[2] = vqmovun_s16(v_v);

				vst3_u8((uint8_t *)yuv_plane, yuv);

				bgr_plane += 24;
				yuv_plane += 24;
			}
			for (; j < input.get_width(); ++j)
			{
				b = *bgr_plane++;
				g = *bgr_plane++;
				r = *bgr_plane++;

				y = (19 * r + 38 * g + 7 * b) >> 6;
				*(yuv_plane++) = std::clamp(y, 0, 255);
				u = ((9 * (b - y)) >> 4) + 128;
				*(yuv_plane++) = std::clamp(u, 0, 255);
				v = ((91 * (r - y)) >> 7) + 128;
				*(yuv_plane++) = std::clamp(v, 0, 255);
			}
		}
	}
	decltype(auto) color_convert_rgb_2_yuv_neon_chunk(int begin, int end, Mat input, Mat output) {
		int j = 0;
		int b, g, r;
		int y, u, v;
		int16x8_t v_b, v_g, v_r;
		int16x8_t v_y, v_u, v_v;
		uint8x8x3_t bgr, yuv;

		int16x8_t v_38, v_7, v_9, v_91, v128x4u, v128x7v;

		v_7 = vdupq_n_s16(7);
		v_9 = vdupq_n_s16(9);
		v_38 = vdupq_n_s16(38);
		v_91 = vdupq_n_s16(91);
		v128x4u = vdupq_n_s16(128 * 16);
		v128x7v = vdupq_n_s16(128 * 128);

		for (int i = begin; i < end; ++i) {
			auto bgr_plane = input.ptr<uchar>(i);
			auto yuv_plane = output.ptr<uchar>(i);
			for (j = 0; j < input.get_width() - 8; j += 8) {
				bgr = vld3_u8(bgr_plane);

				v_r = vreinterpretq_s16_u16(vmovl_u8(bgr.val[0]));
				v_g = vreinterpretq_s16_u16(vmovl_u8(bgr.val[1]));
				v_b = vreinterpretq_s16_u16(vmovl_u8(bgr.val[2]));

				v_y = vmlaq_s16(vmulq_n_s16(v_r, 19), v_g, v_38);
				v_y = vshrq_n_s16(vmlaq_s16(v_y, v_b, v_7), 6);

				v_u = vmlaq_s16(v128x4u, vsubq_s16(v_b, v_y), v_9);
				v_u = vshrq_n_s16(v_u, 4);

				v_v = vmlaq_s16(v128x7v, vsubq_s16(v_r, v_y), v_91);
				v_v = vshrq_n_s16(v_v, 7);

				yuv.val[0] = vqmovun_s16(v_y);
				yuv.val[1] = vqmovun_s16(v_u);
				yuv.val[2] = vqmovun_s16(v_v);

				vst3_u8((uint8_t *)yuv_plane, yuv);

				bgr_plane += 24;
				yuv_plane += 24;
			}
			for (; j < input.get_width(); ++j)
			{
				r = *bgr_plane++;
				g = *bgr_plane++;
				b = *bgr_plane++;

				y = (19 * r + 38 * g + 7 * b) >> 6;
				*(yuv_plane++) = std::clamp(y, 0, 255);
				u = ((9 * (b - y)) >> 4) + 128;
				*(yuv_plane++) = std::clamp(u, 0, 255);
				v = ((91 * (r - y)) >> 7) + 128;
				*(yuv_plane++) = std::clamp(v, 0, 255);
			}
		}
	}
	decltype(auto) color_convert_yuv_2_bgr_neon_chunk(int begin, int end, Mat input, Mat output) {
		int j = 0;
		int b, g, r;
		int y, u, v;
		int16x8_t v_b, v_g, v_r;
		int16x8_t v_y, v_u, v_v;
		uint8x8x3_t bgr, yuv;
		int16x8_t v_128, v_16;

		int16x8_t v_38, v_7, v_9, v_91, v128x4u, v128x7v;

		v_7 = vdupq_n_s16(7);
		v_9 = vdupq_n_s16(9);
		v_38 = vdupq_n_s16(38);
		v_91 = vdupq_n_s16(91);
		v_16 = vdupq_n_s16(16);
		v_128 = vdupq_n_s16(128);
		v128x4u = vdupq_n_s16(128 * 16);
		v128x7v = vdupq_n_s16(128 * 128);

		for (int i = begin; i < end; ++i) {
			auto yuv_plane = input.ptr<uchar>(i);
			auto bgr_plane = output.ptr<uchar>(i);
			for (j = 0; j < input.get_width() - 8; j += 8) {
				yuv = vld3_u8(yuv_plane);

				v_y = vreinterpretq_s16_u16(vmovl_u8(yuv.val[0]));
				v_u = vreinterpretq_s16_u16(vmovl_u8(yuv.val[1]));
				v_v = vreinterpretq_s16_u16(vmovl_u8(yuv.val[2]));

				v_y = vshlq_n_s16(v_y, 5);
				v_u = vsubq_s16(v_u, v_128);
				v_v = vsubq_s16(v_v, v_128);

				v_r = vmulq_n_s16(v_v, 45);
				v_b = vmulq_n_s16(v_u, 57);
				v_g = vaddq_s16(vmulq_n_s16(v_u, 11), vmulq_n_s16(v_v, 23));

				v_r = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y, v_r), v_16), 5);
				v_g = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y, v_g), v_16), 5);
				v_b = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y, v_b), v_16), 5);

				//auto v_r_r = vzipq_s16(v_r, v_r);
				//auto v_g_g = vzipq_s16(v_g, v_g);
				//auto v_b_b = vzipq_s16(v_b, v_b);
				//v_r = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y, v_r_r.val[0]), v_16), 5);
				//v_g = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y, v_g_g.val[0]), v_16), 5);
				//v_b = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y, v_b_b.val[0]), v_16), 5);

				bgr.val[0] = vqmovun_s16(v_b);
				bgr.val[1] = vqmovun_s16(v_g);
				bgr.val[2] = vqmovun_s16(v_r);

				vst3_u8((uint8_t *)bgr_plane, bgr);

				bgr_plane += 24;
				yuv_plane += 24;
			}
			for (; j < input.get_width(); ++j)
			{
				y = *yuv_plane++;
				u = *yuv_plane++;
				v = *yuv_plane++;

				y = y << 5;
				u = u - 128;
				v = v - 128;

				r = (y + (v * 45) + 16) >> 5;
				g = (y - (u * 11) - (v * 23) + 16) >> 5;
				b = (y + (u * 57) + 16) >> 5;

				*(bgr_plane++) = std::clamp(b, 0, 255);
				*(bgr_plane++) = std::clamp(g, 0, 255);
				*(bgr_plane++) = std::clamp(r, 0, 255);
			}
		}
	}
	decltype(auto) color_convert_yuv_2_rgb_neon_chunk(int begin, int end, Mat input, Mat output) {
		int j = 0;
		int b, g, r;
		int y, u, v;
		int16x8_t v_b, v_g, v_r;
		int16x8_t v_y, v_u, v_v;
		uint8x8x3_t bgr, yuv;
		int16x8_t v_128, v_16;

		int16x8_t v_38, v_7, v_9, v_91, v128x4u, v128x7v;

		v_7 = vdupq_n_s16(7);
		v_9 = vdupq_n_s16(9);
		v_38 = vdupq_n_s16(38);
		v_91 = vdupq_n_s16(91);
		v_16 = vdupq_n_s16(16);
		v_128 = vdupq_n_s16(128);
		v128x4u = vdupq_n_s16(128 * 16);
		v128x7v = vdupq_n_s16(128 * 128);

		for (int i = begin; i < end; ++i) {
			auto yuv_plane = input.ptr<uchar>(i);
			auto bgr_plane = output.ptr<uchar>(i);
			for (j = 0; j < input.get_width() - 8; j += 8) {
				yuv = vld3_u8(yuv_plane);

				v_y = vreinterpretq_s16_u16(vmovl_u8(yuv.val[0]));
				v_u = vreinterpretq_s16_u16(vmovl_u8(yuv.val[1]));
				v_v = vreinterpretq_s16_u16(vmovl_u8(yuv.val[2]));

				v_y = vshlq_n_s16(v_y, 5);
				v_u = vsubq_s16(v_u, v_128);
				v_v = vsubq_s16(v_v, v_128);

				v_r = vmulq_n_s16(v_v, 45);
				v_b = vmulq_n_s16(v_u, 57);
				v_g = vaddq_s16(vmulq_n_s16(v_u, 11), vmulq_n_s16(v_v, 23));

				v_r = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y, v_r), v_16), 5);
				v_g = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y, v_g), v_16), 5);
				v_b = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y, v_b), v_16), 5);

				//auto v_r_r = vzipq_s16(v_r, v_r);
				//auto v_g_g = vzipq_s16(v_g, v_g);
				//auto v_b_b = vzipq_s16(v_b, v_b);
				//v_r = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y, v_r_r.val[0]), v_16), 5);
				//v_g = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y, v_g_g.val[0]), v_16), 5);
				//v_b = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y, v_b_b.val[0]), v_16), 5);

				bgr.val[0] = vqmovun_s16(v_r);
				bgr.val[1] = vqmovun_s16(v_g);
				bgr.val[2] = vqmovun_s16(v_b);

				vst3_u8((uint8_t *)bgr_plane, bgr);

				bgr_plane += 24;
				yuv_plane += 24;
			}
			for (; j < input.get_width(); ++j)
			{
				y = *yuv_plane++;
				u = *yuv_plane++;
				v = *yuv_plane++;

				y = y << 5;
				u = u - 128;
				v = v - 128;

				r = (y + (v * 45) + 16) >> 5;
				g = (y - (u * 11) - (v * 23) + 16) >> 5;
				b = (y + (u * 57) + 16) >> 5;

				*(bgr_plane++) = std::clamp(r, 0, 255);
				*(bgr_plane++) = std::clamp(g, 0, 255);
				*(bgr_plane++) = std::clamp(b, 0, 255);
			}
		}
	}

	decltype(auto) color_convert_bgr_2_nv12_neon_chunk(int begin, int end, Mat input, Mat output) {
		int j = 0;
		int b, g, r, y;
		int16x8x2_t tmp;
		uint8x8x2_t u_tmp;
		uint8x8x3_t bgr_1, bgr_2;
		int16x8_t y_1, y_2, u_1, v_1;
		int16x8_t r_1, g_1, b_1, r_2, g_2, b_2;
		int16x8_t y_tmp, v_38, v_7, v_9, v_91, v128x4u, v128x7v;

		v_7 = vdupq_n_s16(7);
		v_9 = vdupq_n_s16(9);
		v_38 = vdupq_n_s16(38);
		v_91 = vdupq_n_s16(91);
		v128x4u = vdupq_n_s16(128 * 16);
		v128x7v = vdupq_n_s16(128 * 128);

		for (int i = begin; i < end; ++i) {
			auto y_plane = output.ptr<uchar>(i);
			auto bgr_plane = input.ptr<uchar>(i);
			auto uv_plane = output.get_data(1, i >> 1);
			for (j = 0; j < input.get_width() - 16; j += 16) {
				bgr_1 = vld3_u8(bgr_plane);
				bgr_plane += 24;
				bgr_2 = vld3_u8(bgr_plane);
				bgr_plane += 24;

				b_1 = vreinterpretq_s16_u16(vmovl_u8(bgr_1.val[0]));
				g_1 = vreinterpretq_s16_u16(vmovl_u8(bgr_1.val[1]));
				r_1 = vreinterpretq_s16_u16(vmovl_u8(bgr_1.val[2]));

				b_2 = vreinterpretq_s16_u16(vmovl_u8(bgr_2.val[0]));
				g_2 = vreinterpretq_s16_u16(vmovl_u8(bgr_2.val[1]));
				r_2 = vreinterpretq_s16_u16(vmovl_u8(bgr_2.val[2]));

				y_1 = vmlaq_s16(vmulq_n_s16(r_1, 19), g_1, v_38);
				y_1 = vshrq_n_s16(vmlaq_s16(y_1, b_1, v_7), 6);
				y_2 = vmlaq_s16(vmulq_n_s16(r_2, 19), g_2, v_38);
				y_2 = vshrq_n_s16(vmlaq_s16(y_2, b_2, v_7), 6);

				vst1_u8(y_plane, vqmovun_s16(y_1));
				y_plane += 8;
				vst1_u8(y_plane, vqmovun_s16(y_2));
				y_plane += 8;

				tmp = vuzpq_s16(b_1, b_2);
				b_1 = tmp.val[0];
				tmp = vuzpq_s16(r_1, r_2);
				r_1 = tmp.val[0];
				tmp = vuzpq_s16(y_1, y_2);
				y_1 = tmp.val[0];

				if (0 == (i % 2)) {
					u_1 = vmlaq_s16(v128x4u, vsubq_s16(b_1, y_1), v_9);
					v_1 = vmlaq_s16(v128x7v, vsubq_s16(r_1, y_1), v_91);

					u_tmp = vzip_u8(vqshrun_n_s16(u_1, 4), vqshrun_n_s16(v_1, 7));
					vst1_u8(uv_plane, u_tmp.val[0]);
					uv_plane += 8;
					vst1_u8(uv_plane, u_tmp.val[1]);
					uv_plane += 8;
				}
			}
			for (; j < input.get_width(); ++j)
			{
				b = *bgr_plane++;
				g = *bgr_plane++;
				r = *bgr_plane++;

				y = (19 * r + 38 * g + 7 * b) >> 6;
				*(y_plane++) = std::clamp(y, 0, 255);

				if ((0 == (i % 2)) && (0 == (j % 2))) {
					*(uv_plane++) = std::clamp(((9 * (b - y)) >> 4) + 128, 0, 255);
					*(uv_plane++) = std::clamp(((91 * (r - y)) >> 7) + 128, 0, 255);
				}
			}
		}
	}
	decltype(auto) color_convert_rgb_2_nv12_neon_chunk(int begin, int end, Mat input, Mat output) {
		int j = 0;
		int b, g, r, y;
		int16x8x2_t tmp;
		uint8x8x2_t u_tmp;
		uint8x8x3_t bgr_1, bgr_2;
		int16x8_t y_1, y_2, u_1, v_1;
		int16x8_t r_1, g_1, b_1, r_2, g_2, b_2;
		int16x8_t y_tmp, v_38, v_7, v_9, v_91, v128x4u, v128x7v;

		v_7 = vdupq_n_s16(7);
		v_9 = vdupq_n_s16(9);
		v_38 = vdupq_n_s16(38);
		v_91 = vdupq_n_s16(91);
		v128x4u = vdupq_n_s16(128 * 16);
		v128x7v = vdupq_n_s16(128 * 128);

		for (int i = begin; i < end; ++i) {
			auto y_plane = output.ptr<uchar>(i);
			auto bgr_plane = input.ptr<uchar>(i);
			auto uv_plane = output.get_data(1, i >> 1);
			for (j = 0; j < input.get_width() - 16; j += 16) {
				bgr_1 = vld3_u8(bgr_plane);
				bgr_plane += 24;
				bgr_2 = vld3_u8(bgr_plane);
				bgr_plane += 24;

				r_1 = vreinterpretq_s16_u16(vmovl_u8(bgr_1.val[0]));
				g_1 = vreinterpretq_s16_u16(vmovl_u8(bgr_1.val[1]));
				b_1 = vreinterpretq_s16_u16(vmovl_u8(bgr_1.val[2]));

				r_2 = vreinterpretq_s16_u16(vmovl_u8(bgr_2.val[0]));
				g_2 = vreinterpretq_s16_u16(vmovl_u8(bgr_2.val[1]));
				b_2 = vreinterpretq_s16_u16(vmovl_u8(bgr_2.val[2]));

				y_1 = vmlaq_s16(vmulq_n_s16(r_1, 19), g_1, v_38);
				y_1 = vshrq_n_s16(vmlaq_s16(y_1, b_1, v_7), 6);
				y_2 = vmlaq_s16(vmulq_n_s16(r_2, 19), g_2, v_38);
				y_2 = vshrq_n_s16(vmlaq_s16(y_2, b_2, v_7), 6);

				vst1_u8(y_plane, vqmovun_s16(y_1));
				y_plane += 8;
				vst1_u8(y_plane, vqmovun_s16(y_2));
				y_plane += 8;

				tmp = vuzpq_s16(b_1, b_2);
				b_1 = tmp.val[0];
				tmp = vuzpq_s16(r_1, r_2);
				r_1 = tmp.val[0];
				tmp = vuzpq_s16(y_1, y_2);
				y_1 = tmp.val[0];

				if (0 == (i % 2)) {
					u_1 = vmlaq_s16(v128x4u, vsubq_s16(b_1, y_1), v_9);
					v_1 = vmlaq_s16(v128x7v, vsubq_s16(r_1, y_1), v_91);

					u_tmp = vzip_u8(vqshrun_n_s16(u_1, 4), vqshrun_n_s16(v_1, 7));
					vst1_u8(uv_plane, u_tmp.val[0]);
					uv_plane += 8;
					vst1_u8(uv_plane, u_tmp.val[1]);
					uv_plane += 8;
				}
			}
			for (; j < input.get_width(); ++j)
			{
				r = *bgr_plane++;
				g = *bgr_plane++;
				b = *bgr_plane++;

				y = (19 * r + 38 * g + 7 * b) >> 6;
				*(y_plane++) = std::clamp(y, 0, 255);

				if ((0 == (i % 2)) && (0 == (j % 2))) {
					*(uv_plane++) = std::clamp(((9 * (b - y)) >> 4) + 128, 0, 255);
					*(uv_plane++) = std::clamp(((91 * (r - y)) >> 7) + 128, 0, 255);
				}
			}
		}
	}
	decltype(auto) color_convert_nv12_2_bgr_neon_chunk(int begin, int end, Mat input, Mat output) {
		int j = 0;
		int u, v, r, g, b;
		int y_00, y_01, y_10, y_11;
		int r_diff, g_diff, b_diff;

		uint8x8x3_t bgr;
		int16x8x2_t uv_tmp;
		int16x8_t v_128, v_16;
		int16x8_t r_1, g_1, b_1;
		int16x8x2_t r_diff_1, g_diff_1, b_diff_1;
		int16x8_t v_y_00, v_y_01, v_y_10, v_y_11, u_0, v_0;

		v_16 = vdupq_n_s16(16);
		v_128 = vdupq_n_s16(128);
		for (int i = begin; i <= end - 2; i += 2) {
			auto dst_0 = output.ptr<uchar>(i);
			auto dst_1 = dst_0 + output.get_pitch(0);
			auto y_plane = input.ptr<uchar>(i);
			auto uv_plane = input.get_data(1, i >> 1);
			for (j = 0; j < input.get_width() - 16; j += 16) {
				v_y_00 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8((uint8_t *)y_plane)));
				v_y_10 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8((uint8_t *)(y_plane + input.get_pitch(0)))));
				y_plane += 8;
				v_y_01 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8((uint8_t *)y_plane)));
				v_y_11 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8((uint8_t *)(y_plane + input.get_pitch(0)))));
				y_plane += 8;

				v_y_00 = vshlq_n_s16(v_y_00, 5);
				v_y_10 = vshlq_n_s16(v_y_10, 5);
				v_y_01 = vshlq_n_s16(v_y_01, 5);
				v_y_11 = vshlq_n_s16(v_y_11, 5);

				u_0 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(uv_plane)));
				uv_plane += 8;
				v_0 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(uv_plane)));
				uv_plane += 8;
				uv_tmp = vuzpq_s16(u_0, v_0);
				v_0 = vsubq_s16(uv_tmp.val[1], v_128);
				u_0 = vsubq_s16(uv_tmp.val[0], v_128);

				r_1 = vmulq_n_s16(v_0, 45);
				b_1 = vmulq_n_s16(u_0, 57);
				g_1 = vaddq_s16(vmulq_n_s16(u_0, 11), vmulq_n_s16(v_0, 23));

				r_diff_1 = vzipq_s16(r_1, r_1);
				g_diff_1 = vzipq_s16(g_1, g_1);
				b_diff_1 = vzipq_s16(b_1, b_1);

				r_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_00, r_diff_1.val[0]), v_16), 5);
				g_1 = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y_00, g_diff_1.val[0]), v_16), 5);
				b_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_00, b_diff_1.val[0]), v_16), 5);
				bgr.val[0] = vqmovun_s16(b_1);
				bgr.val[1] = vqmovun_s16(g_1);
				bgr.val[2] = vqmovun_s16(r_1);
				vst3_u8((uint8_t *)dst_0, bgr);
				dst_0 += 24;
				r_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_10, r_diff_1.val[0]), v_16), 5);
				g_1 = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y_10, g_diff_1.val[0]), v_16), 5);
				b_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_10, b_diff_1.val[0]), v_16), 5);
				bgr.val[0] = vqmovun_s16(b_1);
				bgr.val[1] = vqmovun_s16(g_1);
				bgr.val[2] = vqmovun_s16(r_1);
				vst3_u8((uint8_t *)dst_1, bgr);
				dst_1 += 24;
				r_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_01, r_diff_1.val[1]), v_16), 5);
				g_1 = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y_01, g_diff_1.val[1]), v_16), 5);
				b_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_01, b_diff_1.val[1]), v_16), 5);
				bgr.val[0] = vqmovun_s16(b_1);
				bgr.val[1] = vqmovun_s16(g_1);
				bgr.val[2] = vqmovun_s16(r_1);
				vst3_u8((uint8_t *)dst_0, bgr);
				dst_0 += 24;
				r_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_11, r_diff_1.val[1]), v_16), 5);
				g_1 = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y_11, g_diff_1.val[1]), v_16), 5);
				b_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_11, b_diff_1.val[1]), v_16), 5);
				bgr.val[0] = vqmovun_s16(b_1);
				bgr.val[1] = vqmovun_s16(g_1);
				bgr.val[2] = vqmovun_s16(r_1);
				vst3_u8((uint8_t *)dst_1, bgr);
				dst_1 += 24;
			}
			for (; j <= input.get_width() - 2; j += 2) {
				y_10 = *(y_plane + input.get_pitch(0));
				y_00 = *(y_plane++);
				y_11 = *(y_plane + input.get_pitch(0));
				y_01 = *(y_plane++);
				u = *(uv_plane++) - 128;
				v = *(uv_plane++) - 128;
				y_10 = y_10 << 5;
				y_00 = y_00 << 5;
				y_11 = y_11 << 5;
				y_01 = y_01 << 5;
				r_diff = ((v * 45));
				g_diff = ((u * 11)) + ((v * 23));
				b_diff = ((u * 57));
				r = ((y_00)+r_diff + 16) >> 5;
				g = ((y_00)-g_diff + 16) >> 5;
				b = ((y_00)+b_diff + 16) >> 5;
				*(dst_0++) = std::clamp(b, 0, 255);
				*(dst_0++) = std::clamp(g, 0, 255);
				*(dst_0++) = std::clamp(r, 0, 255);
				r = ((y_01)+r_diff + 16) >> 5;
				g = ((y_01)-g_diff + 16) >> 5;
				b = ((y_01)+b_diff + 16) >> 5;
				*(dst_0++) = std::clamp(b, 0, 255);
				*(dst_0++) = std::clamp(g, 0, 255);
				*(dst_0++) = std::clamp(r, 0, 255);
				r = ((y_10)+r_diff + 16) >> 5;
				g = ((y_10)-g_diff + 16) >> 5;
				b = ((y_10)+b_diff + 16) >> 5;
				*(dst_1++) = std::clamp(b, 0, 255);
				*(dst_1++) = std::clamp(g, 0, 255);
				*(dst_1++) = std::clamp(r, 0, 255);
				r = y_11 + r_diff;
				g = y_11 - g_diff;
				b = y_11 + b_diff;
				r = ((y_11)+r_diff + 16) >> 5;
				g = ((y_11)-g_diff + 16) >> 5;
				b = ((y_11)+b_diff + 16) >> 5;
				*(dst_1++) = std::clamp(b, 0, 255);
				*(dst_1++) = std::clamp(g, 0, 255);
				*(dst_1++) = std::clamp(r, 0, 255);
			}
		}
	}
	decltype(auto) color_convert_nv12_2_rgb_neon_chunk(int begin, int end, Mat input, Mat output) {
		int j = 0;
		int u, v, r, g, b;
		int y_00, y_01, y_10, y_11;
		int r_diff, g_diff, b_diff;

		uint8x8x3_t bgr;
		int16x8x2_t uv_tmp;
		int16x8_t v_128, v_16;
		int16x8_t r_1, g_1, b_1;
		int16x8x2_t r_diff_1, g_diff_1, b_diff_1;
		int16x8_t v_y_00, v_y_01, v_y_10, v_y_11, u_0, v_0;

		v_16 = vdupq_n_s16(16);
		v_128 = vdupq_n_s16(128);
		for (int i = begin; i <= end - 2; i += 2) {
			auto dst_0 = output.ptr<uchar>(i);
			auto dst_1 = dst_0 + output.get_pitch(0);
			auto y_plane = input.ptr<uchar>(i);
			auto uv_plane = input.get_data(1, i >> 1);
			for (j = 0; j < input.get_width() - 16; j += 16) {
				v_y_00 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8((uint8_t *)y_plane)));
				v_y_10 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8((uint8_t *)(y_plane + input.get_pitch(0)))));
				y_plane += 8;
				v_y_01 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8((uint8_t *)y_plane)));
				v_y_11 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8((uint8_t *)(y_plane + input.get_pitch(0)))));
				y_plane += 8;

				v_y_00 = vshlq_n_s16(v_y_00, 5);
				v_y_10 = vshlq_n_s16(v_y_10, 5);
				v_y_01 = vshlq_n_s16(v_y_01, 5);
				v_y_11 = vshlq_n_s16(v_y_11, 5);

				u_0 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(uv_plane)));
				uv_plane += 8;
				v_0 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(uv_plane)));
				uv_plane += 8;
				uv_tmp = vuzpq_s16(u_0, v_0);
				v_0 = vsubq_s16(uv_tmp.val[1], v_128);
				u_0 = vsubq_s16(uv_tmp.val[0], v_128);

				r_1 = vmulq_n_s16(v_0, 45);
				b_1 = vmulq_n_s16(u_0, 57);
				g_1 = vaddq_s16(vmulq_n_s16(u_0, 11), vmulq_n_s16(v_0, 23));

				r_diff_1 = vzipq_s16(r_1, r_1);
				g_diff_1 = vzipq_s16(g_1, g_1);
				b_diff_1 = vzipq_s16(b_1, b_1);

				r_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_00, r_diff_1.val[0]), v_16), 5);
				g_1 = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y_00, g_diff_1.val[0]), v_16), 5);
				b_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_00, b_diff_1.val[0]), v_16), 5);
				bgr.val[0] = vqmovun_s16(r_1);
				bgr.val[1] = vqmovun_s16(g_1);
				bgr.val[2] = vqmovun_s16(b_1);
				vst3_u8((uint8_t *)dst_0, bgr);
				dst_0 += 24;
				r_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_10, r_diff_1.val[0]), v_16), 5);
				g_1 = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y_10, g_diff_1.val[0]), v_16), 5);
				b_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_10, b_diff_1.val[0]), v_16), 5);
				bgr.val[0] = vqmovun_s16(r_1);
				bgr.val[1] = vqmovun_s16(g_1);
				bgr.val[2] = vqmovun_s16(b_1);
				vst3_u8((uint8_t *)dst_1, bgr);
				dst_1 += 24;
				r_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_01, r_diff_1.val[1]), v_16), 5);
				g_1 = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y_01, g_diff_1.val[1]), v_16), 5);
				b_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_01, b_diff_1.val[1]), v_16), 5);
				bgr.val[0] = vqmovun_s16(r_1);
				bgr.val[1] = vqmovun_s16(g_1);
				bgr.val[2] = vqmovun_s16(b_1);
				vst3_u8((uint8_t *)dst_0, bgr);
				dst_0 += 24;
				r_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_11, r_diff_1.val[1]), v_16), 5);
				g_1 = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y_11, g_diff_1.val[1]), v_16), 5);
				b_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_11, b_diff_1.val[1]), v_16), 5);
				bgr.val[0] = vqmovun_s16(r_1);
				bgr.val[1] = vqmovun_s16(g_1);
				bgr.val[2] = vqmovun_s16(b_1);
				vst3_u8((uint8_t *)dst_1, bgr);
				dst_1 += 24;
			}
			for (; j <= input.get_width() - 2; j += 2) {
				y_10 = *(y_plane + input.get_pitch(0));
				y_00 = *(y_plane++);
				y_11 = *(y_plane + input.get_pitch(0));
				y_01 = *(y_plane++);
				u = *(uv_plane++) - 128;
				v = *(uv_plane++) - 128;
				y_10 = y_10 << 5;
				y_00 = y_00 << 5;
				y_11 = y_11 << 5;
				y_01 = y_01 << 5;
				r_diff = ((v * 45));
				g_diff = ((u * 11)) + ((v * 23));
				b_diff = ((u * 57));
				r = ((y_00)+r_diff + 16) >> 5;
				g = ((y_00)-g_diff + 16) >> 5;
				b = ((y_00)+b_diff + 16) >> 5;
				*(dst_0++) = std::clamp(r, 0, 255);
				*(dst_0++) = std::clamp(g, 0, 255);
				*(dst_0++) = std::clamp(b, 0, 255);
				r = ((y_01)+r_diff + 16) >> 5;
				g = ((y_01)-g_diff + 16) >> 5;
				b = ((y_01)+b_diff + 16) >> 5;
				*(dst_0++) = std::clamp(r, 0, 255);
				*(dst_0++) = std::clamp(g, 0, 255);
				*(dst_0++) = std::clamp(b, 0, 255);
				r = ((y_10)+r_diff + 16) >> 5;
				g = ((y_10)-g_diff + 16) >> 5;
				b = ((y_10)+b_diff + 16) >> 5;
				*(dst_1++) = std::clamp(r, 0, 255);
				*(dst_1++) = std::clamp(g, 0, 255);
				*(dst_1++) = std::clamp(b, 0, 255);
				r = y_11 + r_diff;
				g = y_11 - g_diff;
				b = y_11 + b_diff;
				r = ((y_11)+r_diff + 16) >> 5;
				g = ((y_11)-g_diff + 16) >> 5;
				b = ((y_11)+b_diff + 16) >> 5;
				*(dst_1++) = std::clamp(r, 0, 255);
				*(dst_1++) = std::clamp(g, 0, 255);
				*(dst_1++) = std::clamp(b, 0, 255);
			}
		}
	}

	decltype(auto) color_convert_bgr_2_nv21_neon_chunk(int begin, int end, Mat input, Mat output) {
		int j = 0;
		int b, g, r, y;
		int16x8x2_t tmp;
		uint8x8x2_t u_tmp;
		uint8x8x3_t bgr_1, bgr_2;
		int16x8_t y_1, y_2, u_1, v_1;
		int16x8_t r_1, g_1, b_1, r_2, g_2, b_2;
		int16x8_t y_tmp, v_38, v_7, v_9, v_91, v128x4u, v128x7v;

		v_7 = vdupq_n_s16(7);
		v_9 = vdupq_n_s16(9);
		v_38 = vdupq_n_s16(38);
		v_91 = vdupq_n_s16(91);
		v128x4u = vdupq_n_s16(128 * 16);
		v128x7v = vdupq_n_s16(128 * 128);

		for (int i = begin; i < end; ++i) {
			auto y_plane = output.ptr<uchar>(i);
			auto bgr_plane = input.ptr<uchar>(i);
			auto uv_plane = output.get_data(1, i >> 1);
			for (j = 0; j < input.get_width() - 16; j += 16) {
				bgr_1 = vld3_u8(bgr_plane);
				bgr_plane += 24;
				bgr_2 = vld3_u8(bgr_plane);
				bgr_plane += 24;

				b_1 = vreinterpretq_s16_u16(vmovl_u8(bgr_1.val[0]));
				g_1 = vreinterpretq_s16_u16(vmovl_u8(bgr_1.val[1]));
				r_1 = vreinterpretq_s16_u16(vmovl_u8(bgr_1.val[2]));

				b_2 = vreinterpretq_s16_u16(vmovl_u8(bgr_2.val[0]));
				g_2 = vreinterpretq_s16_u16(vmovl_u8(bgr_2.val[1]));
				r_2 = vreinterpretq_s16_u16(vmovl_u8(bgr_2.val[2]));

				y_1 = vmlaq_s16(vmulq_n_s16(r_1, 19), g_1, v_38);
				y_1 = vshrq_n_s16(vmlaq_s16(y_1, b_1, v_7), 6);
				y_2 = vmlaq_s16(vmulq_n_s16(r_2, 19), g_2, v_38);
				y_2 = vshrq_n_s16(vmlaq_s16(y_2, b_2, v_7), 6);

				vst1_u8(y_plane, vqmovun_s16(y_1));
				y_plane += 8;
				vst1_u8(y_plane, vqmovun_s16(y_2));
				y_plane += 8;

				tmp = vuzpq_s16(b_1, b_2);
				b_1 = tmp.val[0];
				tmp = vuzpq_s16(r_1, r_2);
				r_1 = tmp.val[0];
				tmp = vuzpq_s16(y_1, y_2);
				y_1 = tmp.val[0];

				if (0 == (i % 2)) {
					u_1 = vmlaq_s16(v128x4u, vsubq_s16(b_1, y_1), v_9);
					v_1 = vmlaq_s16(v128x7v, vsubq_s16(r_1, y_1), v_91);

					u_tmp = vzip_u8(vqshrun_n_s16(v_1, 7), vqshrun_n_s16(u_1, 4));
					vst1_u8(uv_plane, u_tmp.val[0]);
					uv_plane += 8;
					vst1_u8(uv_plane, u_tmp.val[1]);
					uv_plane += 8;
				}
			}
			for (; j < input.get_width(); ++j)
			{
				b = *bgr_plane++;
				g = *bgr_plane++;
				r = *bgr_plane++;

				y = (19 * r + 38 * g + 7 * b) >> 6;
				*(y_plane++) = std::clamp(y, 0, 255);

				if ((0 == (i % 2)) && (0 == (j % 2))) {
					*(uv_plane++) = std::clamp(((91 * (r - y)) >> 7) + 128, 0, 255);
					*(uv_plane++) = std::clamp(((9 * (b - y)) >> 4) + 128, 0, 255);
				}
			}
		}
	}
	decltype(auto) color_convert_rgb_2_nv21_neon_chunk(int begin, int end, Mat input, Mat output) {
		int j = 0;
		int b, g, r, y;
		int16x8x2_t tmp;
		uint8x8x2_t u_tmp;
		uint8x8x3_t bgr_1, bgr_2;
		int16x8_t y_1, y_2, u_1, v_1;
		int16x8_t r_1, g_1, b_1, r_2, g_2, b_2;
		int16x8_t y_tmp, v_38, v_7, v_9, v_91, v128x4u, v128x7v;

		v_7 = vdupq_n_s16(7);
		v_9 = vdupq_n_s16(9);
		v_38 = vdupq_n_s16(38);
		v_91 = vdupq_n_s16(91);
		v128x4u = vdupq_n_s16(128 * 16);
		v128x7v = vdupq_n_s16(128 * 128);

		for (int i = begin; i < end; ++i) {
			auto y_plane = output.ptr<uchar>(i);
			auto bgr_plane = input.ptr<uchar>(i);
			auto uv_plane = output.get_data(1, i >> 1);
			for (j = 0; j < input.get_width() - 16; j += 16) {
				bgr_1 = vld3_u8(bgr_plane);
				bgr_plane += 24;
				bgr_2 = vld3_u8(bgr_plane);
				bgr_plane += 24;

				r_1 = vreinterpretq_s16_u16(vmovl_u8(bgr_1.val[0]));
				g_1 = vreinterpretq_s16_u16(vmovl_u8(bgr_1.val[1]));
				b_1 = vreinterpretq_s16_u16(vmovl_u8(bgr_1.val[2]));

				r_2 = vreinterpretq_s16_u16(vmovl_u8(bgr_2.val[0]));
				g_2 = vreinterpretq_s16_u16(vmovl_u8(bgr_2.val[1]));
				b_2 = vreinterpretq_s16_u16(vmovl_u8(bgr_2.val[2]));

				y_1 = vmlaq_s16(vmulq_n_s16(r_1, 19), g_1, v_38);
				y_1 = vshrq_n_s16(vmlaq_s16(y_1, b_1, v_7), 6);
				y_2 = vmlaq_s16(vmulq_n_s16(r_2, 19), g_2, v_38);
				y_2 = vshrq_n_s16(vmlaq_s16(y_2, b_2, v_7), 6);

				vst1_u8(y_plane, vqmovun_s16(y_1));
				y_plane += 8;
				vst1_u8(y_plane, vqmovun_s16(y_2));
				y_plane += 8;

				tmp = vuzpq_s16(b_1, b_2);
				b_1 = tmp.val[0];
				tmp = vuzpq_s16(r_1, r_2);
				r_1 = tmp.val[0];
				tmp = vuzpq_s16(y_1, y_2);
				y_1 = tmp.val[0];

				if (0 == (i % 2)) {
					u_1 = vmlaq_s16(v128x4u, vsubq_s16(b_1, y_1), v_9);
					v_1 = vmlaq_s16(v128x7v, vsubq_s16(r_1, y_1), v_91);

					u_tmp = vzip_u8(vqshrun_n_s16(v_1, 7), vqshrun_n_s16(u_1, 4));
					vst1_u8(uv_plane, u_tmp.val[0]);
					uv_plane += 8;
					vst1_u8(uv_plane, u_tmp.val[1]);
					uv_plane += 8;
				}
			}
			for (; j < input.get_width(); ++j)
			{
				r = *bgr_plane++;
				g = *bgr_plane++;
				b = *bgr_plane++;

				y = (19 * r + 38 * g + 7 * b) >> 6;
				*(y_plane++) = std::clamp(y, 0, 255);

				if ((0 == (i % 2)) && (0 == (j % 2))) {
					*(uv_plane++) = std::clamp(((91 * (r - y)) >> 7) + 128, 0, 255);
					*(uv_plane++) = std::clamp(((9 * (b - y)) >> 4) + 128, 0, 255);
				}
			}
		}
	}
	decltype(auto) color_convert_nv21_2_bgr_neon_chunk(int begin, int end, Mat input, Mat output) {
		int j = 0;
		int u, v, r, g, b;
		int y_00, y_01, y_10, y_11;
		int r_diff, g_diff, b_diff;

		uint8x8x3_t bgr;
		int16x8x2_t uv_tmp;
		int16x8_t v_128, v_16;
		int16x8_t r_1, g_1, b_1;
		int16x8x2_t r_diff_1, g_diff_1, b_diff_1;
		int16x8_t v_y_00, v_y_01, v_y_10, v_y_11, u_0, v_0;

		v_16 = vdupq_n_s16(16);
		v_128 = vdupq_n_s16(128);
		for (int i = begin; i <= end - 2; i += 2) {
			auto dst_0 = output.ptr<uchar>(i);
			auto dst_1 = dst_0 + output.get_pitch(0);
			auto y_plane = input.ptr<uchar>(i);
			auto uv_plane = input.get_data(1, i >> 1);
			for (j = 0; j < input.get_width() - 16; j += 16) {
				v_y_00 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8((uint8_t *)y_plane)));
				v_y_10 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8((uint8_t *)(y_plane + input.get_pitch(0)))));
				y_plane += 8;
				v_y_01 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8((uint8_t *)y_plane)));
				v_y_11 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8((uint8_t *)(y_plane + input.get_pitch(0)))));
				y_plane += 8;

				v_y_00 = vshlq_n_s16(v_y_00, 5);
				v_y_10 = vshlq_n_s16(v_y_10, 5);
				v_y_01 = vshlq_n_s16(v_y_01, 5);
				v_y_11 = vshlq_n_s16(v_y_11, 5);

				u_0 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(uv_plane)));
				uv_plane += 8;
				v_0 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(uv_plane)));
				uv_plane += 8;
				uv_tmp = vuzpq_s16(u_0, v_0);
				v_0 = vsubq_s16(uv_tmp.val[0], v_128);
				u_0 = vsubq_s16(uv_tmp.val[1], v_128);

				r_1 = vmulq_n_s16(v_0, 45);
				b_1 = vmulq_n_s16(u_0, 57);
				g_1 = vaddq_s16(vmulq_n_s16(u_0, 11), vmulq_n_s16(v_0, 23));

				r_diff_1 = vzipq_s16(r_1, r_1);
				g_diff_1 = vzipq_s16(g_1, g_1);
				b_diff_1 = vzipq_s16(b_1, b_1);

				r_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_00, r_diff_1.val[0]), v_16), 5);
				g_1 = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y_00, g_diff_1.val[0]), v_16), 5);
				b_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_00, b_diff_1.val[0]), v_16), 5);
				bgr.val[0] = vqmovun_s16(b_1);
				bgr.val[1] = vqmovun_s16(g_1);
				bgr.val[2] = vqmovun_s16(r_1);
				vst3_u8((uint8_t *)dst_0, bgr);
				dst_0 += 24;
				r_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_10, r_diff_1.val[0]), v_16), 5);
				g_1 = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y_10, g_diff_1.val[0]), v_16), 5);
				b_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_10, b_diff_1.val[0]), v_16), 5);
				bgr.val[0] = vqmovun_s16(b_1);
				bgr.val[1] = vqmovun_s16(g_1);
				bgr.val[2] = vqmovun_s16(r_1);
				vst3_u8((uint8_t *)dst_1, bgr);
				dst_1 += 24;
				r_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_01, r_diff_1.val[1]), v_16), 5);
				g_1 = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y_01, g_diff_1.val[1]), v_16), 5);
				b_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_01, b_diff_1.val[1]), v_16), 5);
				bgr.val[0] = vqmovun_s16(b_1);
				bgr.val[1] = vqmovun_s16(g_1);
				bgr.val[2] = vqmovun_s16(r_1);
				vst3_u8((uint8_t *)dst_0, bgr);
				dst_0 += 24;
				r_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_11, r_diff_1.val[1]), v_16), 5);
				g_1 = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y_11, g_diff_1.val[1]), v_16), 5);
				b_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_11, b_diff_1.val[1]), v_16), 5);
				bgr.val[0] = vqmovun_s16(b_1);
				bgr.val[1] = vqmovun_s16(g_1);
				bgr.val[2] = vqmovun_s16(r_1);
				vst3_u8((uint8_t *)dst_1, bgr);
				dst_1 += 24;
			}
			for (; j <= input.get_width() - 2; j += 2) {
				y_10 = *(y_plane + input.get_pitch(0));
				y_00 = *(y_plane++);
				y_11 = *(y_plane + input.get_pitch(0));
				y_01 = *(y_plane++);
				v = *(uv_plane++) - 128;
				u = *(uv_plane++) - 128;
				y_10 = y_10 << 5;
				y_00 = y_00 << 5;
				y_11 = y_11 << 5;
				y_01 = y_01 << 5;
				r_diff = ((v * 45));
				g_diff = ((u * 11)) + ((v * 23));
				b_diff = ((u * 57));
				r = ((y_00)+r_diff + 16) >> 5;
				g = ((y_00)-g_diff + 16) >> 5;
				b = ((y_00)+b_diff + 16) >> 5;
				*(dst_0++) = std::clamp(b, 0, 255);
				*(dst_0++) = std::clamp(g, 0, 255);
				*(dst_0++) = std::clamp(r, 0, 255);
				r = ((y_01)+r_diff + 16) >> 5;
				g = ((y_01)-g_diff + 16) >> 5;
				b = ((y_01)+b_diff + 16) >> 5;
				*(dst_0++) = std::clamp(b, 0, 255);
				*(dst_0++) = std::clamp(g, 0, 255);
				*(dst_0++) = std::clamp(r, 0, 255);
				r = ((y_10)+r_diff + 16) >> 5;
				g = ((y_10)-g_diff + 16) >> 5;
				b = ((y_10)+b_diff + 16) >> 5;
				*(dst_1++) = std::clamp(b, 0, 255);
				*(dst_1++) = std::clamp(g, 0, 255);
				*(dst_1++) = std::clamp(r, 0, 255);
				r = y_11 + r_diff;
				g = y_11 - g_diff;
				b = y_11 + b_diff;
				r = ((y_11)+r_diff + 16) >> 5;
				g = ((y_11)-g_diff + 16) >> 5;
				b = ((y_11)+b_diff + 16) >> 5;
				*(dst_1++) = std::clamp(b, 0, 255);
				*(dst_1++) = std::clamp(g, 0, 255);
				*(dst_1++) = std::clamp(r, 0, 255);
			}
		}
	}
	decltype(auto) color_convert_nv21_2_rgb_neon_chunk(int begin, int end, Mat input, Mat output) {
		int j = 0;
		int u, v, r, g, b;
		int y_00, y_01, y_10, y_11;
		int r_diff, g_diff, b_diff;

		uint8x8x3_t bgr;
		int16x8x2_t uv_tmp;
		int16x8_t v_128, v_16;
		int16x8_t r_1, g_1, b_1;
		int16x8x2_t r_diff_1, g_diff_1, b_diff_1;
		int16x8_t v_y_00, v_y_01, v_y_10, v_y_11, u_0, v_0;

		v_16 = vdupq_n_s16(16);
		v_128 = vdupq_n_s16(128);
		for (int i = begin; i <= end - 2; i += 2) {
			auto dst_0 = output.ptr<uchar>(i);
			auto dst_1 = dst_0 + output.get_pitch(0);
			auto y_plane = input.ptr<uchar>(i);
			auto uv_plane = input.get_data(1, i >> 1);
			for (j = 0; j < input.get_width() - 16; j += 16) {
				v_y_00 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8((uint8_t *)y_plane)));
				v_y_10 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8((uint8_t *)(y_plane + input.get_pitch(0)))));
				y_plane += 8;
				v_y_01 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8((uint8_t *)y_plane)));
				v_y_11 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8((uint8_t *)(y_plane + input.get_pitch(0)))));
				y_plane += 8;

				v_y_00 = vshlq_n_s16(v_y_00, 5);
				v_y_10 = vshlq_n_s16(v_y_10, 5);
				v_y_01 = vshlq_n_s16(v_y_01, 5);
				v_y_11 = vshlq_n_s16(v_y_11, 5);

				u_0 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(uv_plane)));
				uv_plane += 8;
				v_0 = vreinterpretq_s16_u16(vmovl_u8(vld1_u8(uv_plane)));
				uv_plane += 8;
				uv_tmp = vuzpq_s16(u_0, v_0);
				v_0 = vsubq_s16(uv_tmp.val[0], v_128);
				u_0 = vsubq_s16(uv_tmp.val[1], v_128);

				r_1 = vmulq_n_s16(v_0, 45);
				b_1 = vmulq_n_s16(u_0, 57);
				g_1 = vaddq_s16(vmulq_n_s16(u_0, 11), vmulq_n_s16(v_0, 23));

				r_diff_1 = vzipq_s16(r_1, r_1);
				g_diff_1 = vzipq_s16(g_1, g_1);
				b_diff_1 = vzipq_s16(b_1, b_1);

				r_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_00, r_diff_1.val[0]), v_16), 5);
				g_1 = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y_00, g_diff_1.val[0]), v_16), 5);
				b_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_00, b_diff_1.val[0]), v_16), 5);
				bgr.val[0] = vqmovun_s16(r_1);
				bgr.val[1] = vqmovun_s16(g_1);
				bgr.val[2] = vqmovun_s16(b_1);
				vst3_u8((uint8_t *)dst_0, bgr);
				dst_0 += 24;
				r_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_10, r_diff_1.val[0]), v_16), 5);
				g_1 = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y_10, g_diff_1.val[0]), v_16), 5);
				b_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_10, b_diff_1.val[0]), v_16), 5);
				bgr.val[0] = vqmovun_s16(r_1);
				bgr.val[1] = vqmovun_s16(g_1);
				bgr.val[2] = vqmovun_s16(b_1);
				vst3_u8((uint8_t *)dst_1, bgr);
				dst_1 += 24;
				r_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_01, r_diff_1.val[1]), v_16), 5);
				g_1 = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y_01, g_diff_1.val[1]), v_16), 5);
				b_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_01, b_diff_1.val[1]), v_16), 5);
				bgr.val[0] = vqmovun_s16(r_1);
				bgr.val[1] = vqmovun_s16(g_1);
				bgr.val[2] = vqmovun_s16(b_1);
				vst3_u8((uint8_t *)dst_0, bgr);
				dst_0 += 24;
				r_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_11, r_diff_1.val[1]), v_16), 5);
				g_1 = vshrq_n_s16(vaddq_s16(vsubq_s16(v_y_11, g_diff_1.val[1]), v_16), 5);
				b_1 = vshrq_n_s16(vaddq_s16(vaddq_s16(v_y_11, b_diff_1.val[1]), v_16), 5);
				bgr.val[0] = vqmovun_s16(r_1);
				bgr.val[1] = vqmovun_s16(g_1);
				bgr.val[2] = vqmovun_s16(b_1);
				vst3_u8((uint8_t *)dst_1, bgr);
				dst_1 += 24;
			}
			for (; j <= input.get_width() - 2; j += 2) {
				y_10 = *(y_plane + input.get_pitch(0));
				y_00 = *(y_plane++);
				y_11 = *(y_plane + input.get_pitch(0));
				y_01 = *(y_plane++);
				v = *(uv_plane++) - 128;
				u = *(uv_plane++) - 128;
				y_10 = y_10 << 5;
				y_00 = y_00 << 5;
				y_11 = y_11 << 5;
				y_01 = y_01 << 5;
				r_diff = ((v * 45));
				g_diff = ((u * 11)) + ((v * 23));
				b_diff = ((u * 57));
				r = ((y_00)+r_diff + 16) >> 5;
				g = ((y_00)-g_diff + 16) >> 5;
				b = ((y_00)+b_diff + 16) >> 5;
				*(dst_0++) = std::clamp(r, 0, 255);
				*(dst_0++) = std::clamp(g, 0, 255);
				*(dst_0++) = std::clamp(b, 0, 255);
				r = ((y_01)+r_diff + 16) >> 5;
				g = ((y_01)-g_diff + 16) >> 5;
				b = ((y_01)+b_diff + 16) >> 5;
				*(dst_0++) = std::clamp(r, 0, 255);
				*(dst_0++) = std::clamp(g, 0, 255);
				*(dst_0++) = std::clamp(b, 0, 255);
				r = ((y_10)+r_diff + 16) >> 5;
				g = ((y_10)-g_diff + 16) >> 5;
				b = ((y_10)+b_diff + 16) >> 5;
				*(dst_1++) = std::clamp(r, 0, 255);
				*(dst_1++) = std::clamp(g, 0, 255);
				*(dst_1++) = std::clamp(b, 0, 255);
				r = y_11 + r_diff;
				g = y_11 - g_diff;
				b = y_11 + b_diff;
				r = ((y_11)+r_diff + 16) >> 5;
				g = ((y_11)-g_diff + 16) >> 5;
				b = ((y_11)+b_diff + 16) >> 5;
				*(dst_1++) = std::clamp(r, 0, 255);
				*(dst_1++) = std::clamp(g, 0, 255);
				*(dst_1++) = std::clamp(b, 0, 255);
			}
		}
	}

	// bgr <==> rgb
	decltype(auto) color_convert_impl_bgr_2_rgb(Mat& in, Mat& out) {
		parallel_execution(
			0,
			in.get_height(),
			[&](auto begin, auto end) {
			for (auto i = begin; i < end; ++i) {
				auto src = in.ptr<uchar>(i);
				auto dst = out.ptr<uchar>(i);
				for (int j = 0; j < in.get_width(); ++j) {
					dst[0] = src[2];
					dst[1] = src[1];
					dst[2] = src[0];
					dst += 3;
					src += 3;
				}
			}
		}
		);
		return return_code::success;
	}
	decltype(auto) color_convert_impl_rgb_2_bgr(Mat& in, Mat& out) {
		return color_convert_impl_bgr_2_rgb(in, out);
	}
	// bgr <==> gray
	decltype(auto) color_convert_impl_bgr_2_gray(Mat& in, Mat& out) {
		parallel_execution(
			0,
			in.get_height(),
			[&](auto begin, auto end) {
			color_convert_bgr_2_gray_neon_chunk(begin, end, in, out);
		}
		);
		return return_code::success;
	}
	decltype(auto) color_convert_impl_rgb_2_gray(Mat& in, Mat& out) {
		parallel_execution(
			0,
			in.get_height(),
			[&](auto begin, auto end) {
			color_convert_rgb_2_gray_neon_chunk(begin, end, in, out);
		}
		);
		return return_code::success;
	}
	// bgr <==> yuv
	decltype(auto) color_convert_impl_bgr_2_yuv(Mat& in, Mat& out) {
		parallel_execution(
			0,
			in.get_height(),
			[&](auto begin, auto end) {
			color_convert_bgr_2_yuv_neon_chunk(begin, end, in, out);
		}
		);
		return return_code::success;
	}
	decltype(auto) color_convert_impl_rgb_2_yuv(Mat& in, Mat& out) {
		parallel_execution(
			0,
			in.get_height(),
			[&](auto begin, auto end) {
			color_convert_rgb_2_yuv_neon_chunk(begin, end, in, out);
		}
		);
		return return_code::success;
	}
	decltype(auto) color_convert_impl_yuv_2_bgr(Mat& in, Mat& out) {
		parallel_execution(
			0,
			in.get_height(),
			[&](auto begin, auto end) {
			color_convert_yuv_2_bgr_neon_chunk(begin, end, in, out);
		}
		);
		return return_code::success;
	}
	decltype(auto) color_convert_impl_yuv_2_rgb(Mat& in, Mat& out) {
		parallel_execution(
			0,
			in.get_height(),
			[&](auto begin, auto end) {
			color_convert_yuv_2_rgb_neon_chunk(begin, end, in, out);
		}
		);
		return return_code::success;
	}
	// bgr <==> nv12
	decltype(auto) color_convert_impl_bgr_2_nv12(Mat& in, Mat& out) {
		parallel_execution(
			0,
			in.get_height(),
			[&](auto begin, auto end) {
			color_convert_bgr_2_nv12_neon_chunk(begin, end, in, out);
		}
		);
		return return_code::success;
	}
	decltype(auto) color_convert_impl_rgb_2_nv12(Mat& in, Mat& out) {
		parallel_execution(
			0,
			in.get_height(),
			[&](auto begin, auto end) {
			color_convert_rgb_2_nv12_neon_chunk(begin, end, in, out);
		}
		);
		return return_code::success;
	}
	decltype(auto) color_convert_impl_nv12_2_bgr(Mat& in, Mat& out) {
		parallel_execution(
			0,
			in.get_height(),
			[&](auto begin, auto end) {
			color_convert_nv12_2_bgr_neon_chunk(begin, end, in, out);
		}
		);
		return return_code::success;
	}
	decltype(auto) color_convert_impl_nv12_2_rgb(Mat& in, Mat& out) {
		parallel_execution(
			0,
			in.get_height(),
			[&](auto begin, auto end) {
			color_convert_nv12_2_rgb_neon_chunk(begin, end, in, out);
		}
		);
		return return_code::success;
	}
	// bgr <==> nv21
	decltype(auto) color_convert_impl_bgr_2_nv21(Mat& in, Mat& out) {
		parallel_execution(
			0,
			in.get_height(),
			[&](auto begin, auto end) {
			color_convert_bgr_2_nv21_neon_chunk(begin, end, in, out);
		}
		);
		return return_code::success;
	}
	decltype(auto) color_convert_impl_rgb_2_nv21(Mat& in, Mat& out) {
		parallel_execution(
			0,
			in.get_height(),
			[&](auto begin, auto end) {
			color_convert_rgb_2_nv21_neon_chunk(begin, end, in, out);
		}
		);
		return return_code::success;
	}
	decltype(auto) color_convert_impl_nv21_2_bgr(Mat& in, Mat& out) {
		parallel_execution(
			0,
			in.get_height(),
			[&](auto begin, auto end) {
			color_convert_nv21_2_bgr_neon_chunk(begin, end, in, out);
		}
		);
		return return_code::success;
	}
	decltype(auto) color_convert_impl_nv21_2_rgb(Mat& in, Mat& out) {
		parallel_execution(
			0,
			in.get_height(),
			[&](auto begin, auto end) {
			color_convert_nv21_2_rgb_neon_chunk(begin, end, in, out);
		}
		);
		return return_code::success;
	}

	class ColorEngine final : 
		public BaseEngine, 
		public uncopyable, 
		public SingletonPattern<ColorEngine> {
		using algorithm_handle = std::function<void(Mat, Mat)>;
		using native_handle = decltype(ImplReflection<color_signature::signature, image_convert, void, Mat, Mat>::get_instance());
	private:
		ColorEngine() {
			__handle = ImplReflection<color_signature::signature, image_convert, void, Mat, Mat>::get_instance();
			__regist_engine();			
		}
	public:
		decltype(auto) get_engine() {
			return __handle;
		}
	private:
		virtual void __regist_engine() override {
#if SSE_OPTIMIZE_LEVEL
			__regist_sse_engine();
#elif BASE_OPTIMIZE_LEVEL
			__regist_base_engine();
#elif NEON_OPTIMIZE_LEVEL
			__regist_neon_engine();
#else OPENCL_OPTIMIZE_LEVEL
			__regist_opencl_engine();
#endif
		}
	private:
		virtual void __regist_sse_engine() override {
		}
		virtual void __regist_base_engine() override {
			__handle->regist_factory(image_convert::bgr_rgb, color_convert_impl_bgr_2_rgb);
			__handle->regist_factory(image_convert::rgb_bgr, color_convert_impl_rgb_2_bgr);
			__handle->regist_factory(image_convert::bgr_gray, color_convert_impl_bgr_2_gray);
			__handle->regist_factory(image_convert::rgb_gray, color_convert_impl_rgb_2_gray);
			__handle->regist_factory(image_convert::bgr_yuv, color_convert_impl_bgr_2_yuv);
			__handle->regist_factory(image_convert::rgb_yuv, color_convert_impl_rgb_2_yuv);
			__handle->regist_factory(image_convert::yuv_bgr, color_convert_impl_yuv_2_bgr);
			__handle->regist_factory(image_convert::yuv_rgb, color_convert_impl_yuv_2_rgb);
			__handle->regist_factory(image_convert::bgr_nv12, color_convert_impl_bgr_2_nv12);
			__handle->regist_factory(image_convert::rgb_nv12, color_convert_impl_rgb_2_nv12);
			__handle->regist_factory(image_convert::nv12_bgr, color_convert_impl_nv12_2_bgr);
			__handle->regist_factory(image_convert::nv12_rgb, color_convert_impl_nv12_2_rgb);
			__handle->regist_factory(image_convert::bgr_nv21, color_convert_impl_bgr_2_nv21);
			__handle->regist_factory(image_convert::rgb_nv21, color_convert_impl_rgb_2_nv21);
			__handle->regist_factory(image_convert::nv21_bgr, color_convert_impl_nv21_2_bgr);
			__handle->regist_factory(image_convert::nv21_rgb, color_convert_impl_nv21_2_rgb);
		}
		virtual void __regist_neon_engine() override {
			__handle->regist_factory(image_convert::bgr_rgb, color_convert_impl_bgr_2_rgb);
			__handle->regist_factory(image_convert::rgb_bgr, color_convert_impl_rgb_2_bgr);
			__handle->regist_factory(image_convert::bgr_gray, color_convert_impl_bgr_2_gray);
			__handle->regist_factory(image_convert::rgb_gray, color_convert_impl_rgb_2_gray);
			__handle->regist_factory(image_convert::bgr_yuv, color_convert_impl_bgr_2_yuv);
			__handle->regist_factory(image_convert::rgb_yuv, color_convert_impl_rgb_2_yuv);
			__handle->regist_factory(image_convert::yuv_bgr, color_convert_impl_yuv_2_bgr);
			__handle->regist_factory(image_convert::yuv_rgb, color_convert_impl_yuv_2_rgb);
			__handle->regist_factory(image_convert::bgr_nv12, color_convert_impl_bgr_2_nv12);
			__handle->regist_factory(image_convert::rgb_nv12, color_convert_impl_rgb_2_nv12);
			__handle->regist_factory(image_convert::nv12_bgr, color_convert_impl_nv12_2_bgr);
			__handle->regist_factory(image_convert::nv12_rgb, color_convert_impl_nv12_2_rgb);
			__handle->regist_factory(image_convert::bgr_nv21, color_convert_impl_bgr_2_nv21);
			__handle->regist_factory(image_convert::rgb_nv21, color_convert_impl_rgb_2_nv21);
			__handle->regist_factory(image_convert::nv21_bgr, color_convert_impl_nv21_2_bgr);
			__handle->regist_factory(image_convert::nv21_rgb, color_convert_impl_nv21_2_rgb);
		}
		virtual void __regist_opencl_engine() override {
		}
	private:
		native_handle __handle;
	private:
		friend SingletonPattern<ColorEngine>;
	};

	class ConvertCode {
	public:
		ConvertCode(image_convert code) : __code(code) {
		}
		ConvertCode(int input_code, int output_code) {
			((input_code == 66304) && (output_code == 66305)) | [&]() {
				__code = image_convert::bgr_rgb;
			};
			((input_code == 66305) && (output_code == 66304)) | [&]() {
				__code = image_convert::bgr_rgb;
			};
			((input_code == 66304) && (output_code == 66306)) | [&]() {
				__code = image_convert::bgr_yuv;
			};
			((input_code == 66305) && (output_code == 66306)) | [&]() {
				__code = image_convert::rgb_yuv;
			};
			((input_code == 66306) && (output_code == 66304)) | [&]() {
				__code = image_convert::yuv_bgr;
			};
			((input_code == 66306) && (output_code == 66305)) | [&]() {
				__code = image_convert::yuv_rgb;
			};
			((input_code == 66304) && (output_code == 65792)) | [&]() {
				__code = image_convert::bgr_gray;
			};
			((input_code == 66305) && (output_code == 65792)) | [&]() {
				__code = image_convert::rgb_gray;
			};
			((input_code == 66304) && (output_code == 131328)) | [&]() {
				__code = image_convert::bgr_nv12;
			};
			((input_code == 66305) && (output_code == 131328)) | [&]() {
				__code = image_convert::rgb_nv12;
			};
			((input_code == 131328) && (output_code == 66304)) | [&]() {
				__code = image_convert::nv12_bgr;
			};
			((input_code == 131328) && (output_code == 66305)) | [&]() {
				__code = image_convert::nv12_rgb;
			};
			((input_code == 66304) && (output_code == 131329)) | [&]() {
				__code = image_convert::bgr_nv21;
			};
			((input_code == 66305) && (output_code == 131329)) | [&]() {
				__code = image_convert::rgb_nv21;
			};
			((input_code == 131329) && (output_code == 66304)) | [&]() {
				__code = image_convert::nv21_bgr;
			};
			((input_code == 131329) && (output_code == 66305)) | [&]() {
				__code = image_convert::nv21_rgb;
			};
		}
	public:
		image_convert operator()() {
			return __code;
		}
	private:
		image_convert __code;
	};

	decltype(auto) color_convert(Mat input, Mat output) {
		(input.get_format() != output.get_format()) | [&]() {
			auto handle = ColorEngine::get_instance();
			auto ir = handle->get_engine();
			auto op = ir->get_algorithm(ConvertCode(input.get_format(), output.get_format())());
			op(input, output); 
		};
		(input.get_format() == output.get_format()) | [&]() {
			copy(input, output);
		};
	}
}