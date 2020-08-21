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

#include <reflection/reflection.h>
#include <image_tool/image_border.h>
#include <thread_pool/thread_pool.h>
#include <memory_tool/memory_tool.h>
#include <meta_program/meta_program.h>

namespace harpocrates {

	using namespace type;
	using namespace engine;
	using namespace operator_reload;
	
	enum class fill_method {
		_4,
		_8,
	};

	struct fill_segment {
		ushort y;
		ushort l;
		ushort r;
		ushort prev_l;
		ushort prev_r;
		int16_t dir;
	};

	struct flood_parameter {
		// rect is the output of the minium fill region of the result
		flood_parameter(Point location = { 128,128,0,0 }) {
			area = 0;
			seed = location;
			fill_value = 255;
			region = true;
			fill_image = true;
			fixed_range = true;
			up_bound = { 30 };
			low_bound = { 30 };
			connectivity = fill_method::_8;
		}
		Rect rect;
		Point seed;
		bool region;
		long long area;
		bool fill_image;
		bool fixed_range;
		uchar fill_value;
		Scalar up_bound;
		Scalar low_bound;
		fill_method connectivity;
	};

	struct diff_operator {
		diff_operator(Scalar low, Scalar up, int channels) {
			for (int i = 0; i < channels; ++i) {
				low_bound[i] = low[i];
				interval_bound[i] = low[i] + up[i];
			}
		}
		template<typename _1, typename _2>
		inline decltype(auto) operator()(_1 in, _2 base, int channels)const {
			bool res = true;
			for (int i = 0; i < channels; ++i) {
				res &= (((unsigned)(in[i] - base[i] + low_bound[i])) <= interval_bound[i]);
			}
			return res;
		}
		Scalar low_bound;
		Scalar interval_bound;
	};

	template<typename _type>
	decltype(auto) push_data(std::vector<fill_segment>& buffer, _type& head, _type& tail, _type& end, uint16_t y, uint16_t l, uint16_t r, uint16_t prev_l, uint16_t prev_r, int16_t dir) {
		tail->y = (uint16_t)(y);
		tail->l = (uint16_t)(l);
		tail->r = (uint16_t)(r);
		tail->dir = (int16_t)(dir);
		tail->prev_l = (uint16_t)(prev_l);
		tail->prev_r = (uint16_t)(prev_r);
		if (++tail == end) {
			buffer.resize(buffer.size() * 2);
			tail = &buffer.front() + (tail - head);
			head = &buffer.front();
			end = head + buffer.size();
		}
	}

	template<typename _type>
	decltype(auto) pop_data(std::vector<fill_segment>& buffer, _type& head, _type& tail, _type& end, uint16_t& y, uint16_t& l, uint16_t& r, uint16_t& prev_l, uint16_t& prev_r, int16_t& dir) {
		--tail;
		y = tail->y;
		l = tail->l;
		r = tail->r;
		dir = tail->dir;
		prev_l = tail->prev_l;
		prev_r = tail->prev_r;
	}

	decltype(auto) flood_fill_impl_grad(Mat image, Mat mask, flood_parameter& parameter) {
		bool region = parameter.region;
		std::vector<fill_segment> buffer;
		auto channels = image.get_channels();
		bool fill_image = parameter.fill_image;
		uchar fill_value = parameter.fill_value;
		buffer.resize(max(image.get_width(), image.get_height()) * 2);
		int _8_connectivity = (parameter.connectivity == fill_method::_8);
		diff_operator diff(parameter.low_bound, parameter.up_bound, channels);

		auto end = &buffer.front() + buffer.size();
		auto head = &buffer.front();
		auto tail = &buffer.front();

		long long area = 0;
		Scalar sum = 0;
		Scalar image_value;
		Scalar const_image_value;
		auto seed_x = parameter.seed.axis.x;
		auto seed_y = parameter.seed.axis.y;
		auto image_ptr = image.ptr<uchar>(parameter.seed.axis.y);
		auto mask_ptr = mask.ptr<uchar>(parameter.seed.axis.y + 1) + 1;
		int16_t dir;
		uint16_t y, l, r, prev_l, prev_r;
		uint16_t x_max, x_min, y_min, y_max;

		l = r = seed_x;
		y_min = y_max = seed_y;

		if (mask_ptr[l]) {
			return;
		}
		mask_ptr[l] = fill_value;

		for (int i = 0; i < channels; ++i) {
			const_image_value[i] = image_ptr[l * channels + i];
		}

		if (parameter.fixed_range) {
			while (!mask_ptr[r + 1] && diff(image_ptr + (r + 1) * channels, const_image_value, channels)) {
				mask_ptr[++r] = fill_value;
			}
			while (!mask_ptr[l - 1] && diff(image_ptr + (l - 1) * channels, const_image_value, channels)) {
				mask_ptr[--l] = fill_value;
			}
		}
		else {
			while (!mask_ptr[r + 1] && diff(image_ptr + (r + 1) * channels, image_ptr + (r) * channels, channels)) {
				mask_ptr[++r] = fill_value;
			}
			while (!mask_ptr[l - 1] && diff(image_ptr + (l - 1) * channels, image_ptr + (l) * channels, channels)) {
				mask_ptr[--l] = fill_value;
			}
		}

		x_min = l;
		x_max = r;

		push_data(buffer, head, tail, end, seed_y, l, r, r + 1, r, 1);

		while (head != tail) {
			pop_data(buffer, head, tail, end, y, l, r, prev_l, prev_r, dir);
			int data[][3] = {
				{-dir, l - _8_connectivity, r + _8_connectivity},
				{dir, l - _8_connectivity, prev_l - 1},
				{dir, prev_r + 1, r + _8_connectivity}
			};
			unsigned int length = unsigned int(r - l);
			if (region) {
				area += (int)length + 1;
				if (x_max < r) x_max = r;
				if (x_min > l) x_min = l;
				if (y_max < y) y_max = y;
				if (y_min > y) y_min = y;
			}
			for (int index = 0; index < 3; ++index) {
				dir = data[index][0];
				int left = data[index][1];
				int right = data[index][2];
				image_ptr = image.ptr<uchar>(y + dir);
				auto image_1 = image.ptr<uchar>(y);
				mask_ptr = mask.ptr<uchar>(y + dir + 1) + 1;
				if (parameter.fixed_range) {
					for (int i = left; i <= right; ++i) {
						if (!mask_ptr[i] && diff(image_ptr + i * channels, const_image_value, channels)) {
							int j = i;
							mask_ptr[i] = fill_value;
							while (!mask_ptr[--j] && diff(image_ptr + j * channels, const_image_value, channels)) {
								mask_ptr[j] = fill_value;
							}
							while (!mask_ptr[++i] && diff(image_ptr + i * channels, const_image_value, channels)) {
								mask_ptr[i] = fill_value;
							}
							push_data(buffer, head, tail, end, y + dir, j + 1, i - 1, l, r, -dir);
						}
					}
				}
				else if (!_8_connectivity) {
					for (int i = left; i <= right; ++i) {
						if (!mask_ptr[i] && diff(image_ptr + i * channels, image_1 + i * channels, channels)) {
							int j = i;
							mask_ptr[i] = fill_value;
							while (!mask_ptr[--j] && diff(image_ptr + j * channels, image_ptr + (j + 1) * channels, channels)) {
								mask_ptr[j] = fill_value;
							}
							while (!mask_ptr[++i] && (diff(image_ptr + i * channels, image_ptr + (i - 1) * channels, channels) || ((i <= r) && diff(image_ptr + i * channels, image_1 + i * channels, channels)))) {
								mask_ptr[i] = fill_value;
							}
							push_data(buffer, head, tail, end, y + dir, j + 1, i - 1, l, r, -dir);
						}
					}
				}
				else {
					for (int i = left; i <= right; ++i) {
						int idx;
						auto get_value = [&]() {
							for (int c = 0; c < channels; ++c) {
								image_value[c] = image_ptr[i * channels + c];
							}
						};
						if (!mask_ptr[i] &&
							(((get_value(),
							(unsigned)(idx = i - l - 1) <= length) &&
							diff(image_value, image_1 + (i - 1) * channels, channels)) ||
							((unsigned)(++idx) <= length &&
							diff(image_value, image_1 + i * channels, channels)) ||
							((unsigned)(++idx) <= length &&
							diff(image_value, image_1 + (i + 1) * channels, channels)))) {
							int j = i;
							mask_ptr[i] = fill_value;

							while (!mask_ptr[--j] && diff(image_ptr + j * channels, image_ptr + (j + 1) * channels, channels)) {
								mask_ptr[j] = fill_value;
							}
							while (
								!mask[++i] &&
								((get_value(), 
								diff(image_value, image_ptr + (i - 1) * channels, channels)) ||
								(((unsigned)(idx = i - l - 1) <= length &&
								diff(image_value, image_1 + (i - 1) * channels, channels))) ||
								((unsigned)(++idx) <= length &&
								diff(image_value, image_1 + i * channels, channels)) ||
								((unsigned)(++idx) <= length &&
								diff(image_value, image_1 + (i + 1) * channels, channels)))) {
								mask_ptr[i] = fill_value;
							}
							push_data(buffer, head, tail, end, y + dir, j + 1, i - 1, l, r, -dir);
						}
					}
				}
			}
			image_value = 0;
			image_ptr = image.ptr<uchar>(y);
			for (int i = l; i <= (int)r; ++i) {
				for (int j = 0; j < channels; ++j) {
					if (fill_image) {
						image_ptr[i * channels + j] = image_value[j];
					}
					else if (region) {
						sum[j] += image_ptr[i * channels + j];
					}
				}
			}
		}
		if (region) {
			parameter.area = area;
			parameter.rect[0] = x_min;
			parameter.rect[1] = y_min;
			parameter.rect[2] = x_max - x_min + 1;
			parameter.rect[3] = y_max - y_min + 1;
		}
	}

	// mask must height > image height + 2 width > image width + 9 & -8
	decltype(auto) flood_fill(Mat image, Mat mask, flood_parameter& parameter) {
		(!mask()) | [&]() {
			mask = Mat((image.get_width() + 9) & -8, image.get_height() + 2, int(image_format::image_format_gray));
			mask = 0;
			std::memset(mask.ptr<uchar>(0), 1, mask.get_pitch());
			std::memset(mask.ptr<uchar>(mask.get_height() - 1), 1, mask.get_pitch());
			for (int i = 1; i < mask.get_height() - 1; ++i) {
				mask.ptr<uchar>(i)[0] = 1;
				mask.ptr<uchar>(i)[mask.get_width() - 1] = 1;
			}
		};
		return flood_fill_impl_grad(image, mask, parameter);
	}
}