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
				(*m++ > 150) | [&]() {
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
						uchar inv_m = 255 - m[j];
						auto o = out + j * channels;
						for (int k = 0; k < channels; ++k) {
							//o[k] = clamp(dst[1][0] / ori[1][0] * (o[k] - ori[0][0]) + dst[0][0], 0, 255);
							o[k] = (int)(o[k] * inv_m + clamp(dst[1][0] / ori[1][0] * (o[k] - ori[0][0]) + dst[0][0], 0, 255) * m[j]) >> 8;
						}
					}
				}
			}
		);
	}

	enum class threshold_style {
		trunc,
		binary,
	};

	struct threshold_parameter {

	};

	template<typename _type>
	// only for single plane image
	void image_thresdhold_trunc(Mat image, _type thresdhold) {
		parallel_execution(0, image.get_height(), [&](auto begin, auto end) {
			for (int i = begin; i < end; ++i) {
				auto in = image.ptr<uchar>(i);
				for (int j = 0; j < image.get_width(); ++j) {
					for (int k = 0; k < image.get_channels(); ++k) {
						if (*in++ < thresdhold[k]) in[-1] = 0;
					}
				}
			}
		});
	}

	template<typename _type>
	// only for single plane image
	void image_thresdhold_binary(Mat image, _type thresdhold) {
		parallel_execution(0, image.get_height(), [&](auto begin, auto end) {
			for (int i = begin; i < end; ++i) {
				auto in = image.ptr<uchar>(i);
				for (int j = 0; j < image.get_width(); ++j) {
					for (int k = 0; k < image.get_channels(); ++k) {
						if (*in++ < thresdhold[k]) in[-1] = 0;
						else in[-1] = 255;
					}
				}
			}
		});
	}

	// only for single plane image
	void image_thresdhold_otsu(Mat image) {
	//void image_thresdhold(Mat image, threshold_parameter parameter) {
		float delta;
		uchar threshold;
		float mean_back;
		float mean_fore;
		float weight_back;
		float weight_fore;
		float max_delta = 0;
		uchar hist[256] = { 0 };
		float fhist[256] = { 0 };
		auto area = image.area();
		for (int i = 0; i < image.get_height(); ++i) {
			auto in = image.ptr<uchar>(i);
			for (int j = 0; j < image.get_width(); ++j) {
				hist[*in++] += 1;
			}
		}
		for (int i = 0; i < 256; ++i) {
			fhist[i] = hist[i] / area;
		}
		for (int i = 0; i < 256; ++i) {
			mean_back = 0;
			mean_fore = 0;
			weight_back = 0;
			weight_fore = 0;
			for (int j = 0; j < 256; ++j) {
				if (j <= i) {
					weight_back += hist[j];
					mean_back += j * hist[j];
				}
				else {
					weight_fore += hist[i];
					mean_fore += j * hist[j];
				}
			}
			mean_back = mean_back / weight_back;
			mean_fore = mean_fore / weight_fore;
			delta = weight_back * weight_fore * pow(mean_back - mean_fore, 2);
			if (delta > max_delta) {
				max_delta = delta;
				threshold = i;
			}
		}
		image_thresdhold_binary(image, view<uchar>(threshold));
	}
	
	// only for single plane image
	void image_thresdhold_otsu(Mat image, Mat mask) {
	//void image_thresdhold(Mat image, threshold_parameter parameter) {
		float delta;
		uchar threshold;
		float mean_back;
		float mean_fore;
		float weight_back;
		float weight_fore;
		float max_delta = 0;
		uchar hist[256] = { 0 };
		float fhist[256] = { 0 };
		auto area = image.area();
		for (int i = 0; i < image.get_height(); ++i) {
			auto m = mask.ptr<char>(i);
			auto in = image.ptr<uchar>(i);
			for (int j = 0; j < image.get_width(); ++j) {
				if (m[j] <= 0) {
					hist[in[j]] += 1;
				}
			}
		}
		for (int i = 0; i < 256; ++i) {
			fhist[i] = hist[i] / area;
		}
		for (int i = 0; i < 256; ++i) {
			mean_back = 0;
			mean_fore = 0;
			weight_back = 0;
			weight_fore = 0;
			for (int j = 0; j < 256; ++j) {
				if (j <= i) {
					weight_back += hist[j];
					mean_back += j * hist[j];
				}
				else {
					weight_fore += hist[i];
					mean_fore += j * hist[j];
				}
			}
			mean_back = mean_back / weight_back;
			mean_fore = mean_fore / weight_fore;
			delta = weight_back * weight_fore * pow(mean_back - mean_fore, 2);
			if (delta > max_delta) {
				max_delta = delta;
				threshold = i;
			}
		}
		image_thresdhold_binary(image, view<uchar>(threshold));
	}
	
	// only for single plane image (mask)
	void image_dark_mask(Mat image, Mat mask) {
		for (int i = 0; i < image.get_height(); ++i) {
			auto m = mask.ptr<uchar>(i);
			auto in = image.ptr<uchar>(i);
			for (int j = 0; j < image.get_width(); ++j) {
				if (m[j] > 0) {
					in[j] = 0;
				}
			}
		}
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


	// number = box filter number
	decltype(auto) boxes_gauss_para(int sigma, int number) {
		float wIdeal = sqrt((12 * sigma * sigma / number) + 2);
		int wl = fast_floor(wIdeal);
		if (wl % 2 == 0) wl--;
		int wu = wl + 2;

		float mIdeal = (12 * sigma * sigma - number * wl * wl - 4 * number * wl - 3 * number) / (-4 * wl - 4);
		int m = fast_round(mIdeal);

		std::vector<int> sizes;
		for (int i = 0; i < number; i++) {
			sizes.push_back(i < m ? wl : wu);
		}
		return sizes;
	}

	// continus memory can be reduced
	void box_vertical_filter_impl(Mat input, Mat output, int radius) {
		float size = 2 * radius + 1;
		auto channels = input.get_channels();
		for (int i = 0; i < input.get_height(); ++i) {
			auto in = input.ptr<uchar>(i);
			auto out = output.ptr<uchar>(i);
			for (int j = 0; j < input.get_width(); ++j) {
				view<int> sum = 0;
				for (int k = j - radius; k < j + radius + 1; ++k) {
					auto index = clamp(k, 0, input.get_width() - 1);
					for (int c = 0; c < channels; ++c) {
						sum[c] += in[index *channels + c];
					}
				}
				for (int c = 0; c < channels; ++c) {
					out[j * channels + c] = sum[c] / size;
				}
			}
		}
	}

	void box_horizontal_filter_impl(Mat input, Mat output, int radius) {
		float size = 2 * radius + 1;
		auto channels = input.get_channels();
		for (int i = 0; i < input.get_height(); ++i) {
			auto out = output.ptr<uchar>(i);
			for (int j = 0; j < input.get_width(); ++j) {
				view<int> sum = 0;
				auto in_vertical = &input.ptr<uchar>(0)[j * channels];
				for (int k = i - radius; k < i + radius + 1; ++k) {
					auto index = clamp(k, 0, input.get_height() - 1);
					for (int c = 0; c < channels; ++c) {
						sum[c] += (in_vertical + index * input.get_pitch())[c];
					}
				}
				for (int c = 0; c < channels; ++c) {
					out[j * channels + c] = sum[c] / size;
				}
			}
		}
	}

	void box_filter_impl(Mat input, Mat output, int radius) {
		box_vertical_filter_impl(input, output, radius);
		box_horizontal_filter_impl(output, input, radius);
		box_vertical_filter_impl(input, output, radius);
		box_horizontal_filter_impl(output, input, radius);
		box_vertical_filter_impl(input, output, radius);
	}

	// box for gauss, need to be moved to filter directory
	void image_filter(Mat input, Mat output, float sigma, int radius) {
		auto para = boxes_gauss_para(sigma, 3);
		box_filter_impl(input, output, 3);
		//box_filter_impl(input, output, (para[0] - 1) / 2);
		box_filter_impl(output, input, (para[1] - 1) / 2);
		box_filter_impl(input, output, (para[2] - 1) / 2);
	}

	// box for gauss, need to be moved to filter directory
	decltype(auto) image_filter(Mat input, float sigma, int radius) {
		auto para = boxes_gauss_para(sigma, 3);
		Mat output(input.size(), 65792);
		box_filter_impl(input, output, radius);
		return output;
	}
}