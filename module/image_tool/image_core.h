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

	void mean_variance_impl_nomask(const Mat image, view<float>& mean, view<float>& variance) {
		mean = 0;
		variance = 0;
		for (int i = 0; i < image.get_height(); ++i) {
			auto in = image.ptr<uchar>(i);
			for (int j = 0; j < image.get_width(); ++j) {
				for (int k = 0; k < image.get_channels(); ++k) {
					uchar v = *in++;
					mean[k] += v;
					variance[k] += v * v;
				}
			}
		}
		auto area = image.get_width() * image.get_height();
		for (int i = 0; i < image.get_channels(); ++i) {
			mean[i] /= (area + 1e-8);
			variance[i] = std::sqrt((variance[i] - area * mean[i] * mean[i]) / area);
		}
	}

	void mean_variance_impl_mask(const Mat image, const Mat mask, view<float>& mean, view<float>& variance) {
		mean = 0;
		variance = 0;
		int area = 0;
		for (int i = 0; i < image.get_height(); ++i) {
			auto m = mask.ptr<uchar>(i);
			auto in = image.ptr<uchar>(i);
			for (int j = 0; j < image.get_width(); ++j) {
				(*m++ > 0) | [&]() {
					auto v = in + j * image.get_channels();
					for (int k = 0; k < image.get_channels(); ++k) {
						mean[k] += v[k];
						variance[k] += v[k] * v[k];
					}
					++area;
				};
			}
		}
		for (int i = 0; i < image.get_channels(); ++i) {
			mean[i] /= (area + 1e-8);
			variance[i] = std::sqrt((variance[i] - area * mean[i] * mean[i]) / area);
		}
	}

	void mean_variance(const Mat image, const Mat mask, view<float>& mean, view<float>& variance) {
		(mask()) |[&]() {
			mean_variance_impl_mask(image, mask, mean, variance);
		};
		(!mask()) |[&]() {
			mean_variance_impl_nomask(image, mean, variance);
		};
	}

	void mean_variance_transform(Mat image, Mat mask, std::vector<view<float>>& ori, std::vector<view<float>>& dst) {
		auto channels = image.get_channels();
		parallel_execution(
			0,
			image.get_height(),
			[&](auto begin, auto end) {
				for (int i = begin; i < end; ++i) {
					auto m = mask.ptr<uchar>(i);
					auto out = image.ptr<uchar>(i);
					for (int j = 0; j < mask.get_width(); ++j) {
						(m[j] > 0) | [&]() {
							auto o = out + j * channels;
							for (int k = 0; k < channels; ++k) {
								o[k] = clamp(dst[1][0] / ori[1][0] * (o[k] - ori[0][0]) + dst[0][0], 0, 255);
							}
						};
					}
				}
			}
		);
	}

	void alpha_blend(const Mat input, Mat output, const Mat mask) {
		auto channels = input.get_channels();
		parallel_execution(
			0,
			input.get_height(),
			[&](auto begin, auto end) {
				for (int i = begin; i < end; ++i) {
					auto m = mask.ptr<uchar>(i);
					auto in = input.ptr<uchar>(i);
					auto out = output.ptr<uchar>(i);
					for (int j = 0; j < mask.get_width(); ++j) {
						(m[j] > 0) | [&]() {
							auto pos_m = m[j];
							auto neg_m = 255 - pos_m;
							auto o = out + j * channels;
							auto i_0 = in + j * channels;
							for (int k = 0; k < channels; ++k) {
								o[k] = (i_0[k] * pos_m + o[k] * neg_m) >> 8;
							}
						};
					}
				}
			}
		);
	}

	template<typename _type = void>
	Mat alpha_blend(const Mat _1, const Mat _2, const Mat mask) {
		auto channels = _1.get_channels();
		Mat output(_1.get_width(), _1.get_height(), _1.get_format());
		parallel_execution(
			0,
			mask.get_height(),
			[&](auto begin, auto end) {
				for (int i = begin; i < end; ++i) {
					auto m = mask.ptr<uchar>(i);
					auto in_1 = _1.ptr<uchar>(i);
					auto in_2 = _2.ptr<uchar>(i);
					auto out = output.ptr<uchar>(i);
					for (int j = 0; j < mask.get_width(); ++j) {
						(m[j] > 0) | [&]() {
							auto pos_m = m[j];
							auto neg_m = 255 - pos_m;
							auto o = out + j * channels;
							auto i_1 = in_1 + j * channels;
							auto i_2 = in_2 + j * channels;
							for (int k = 0; k < channels; ++k) {
								o[k] = (i_1[k] * pos_m + i_2[k] * neg_m) >> 8;
							}
						};
					}
				}
			}
		);
		return output;
	}
}