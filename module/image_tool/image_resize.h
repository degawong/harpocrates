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

	using coef_signature = meta_hash<type_char<'c', 'o', 'e', 'f', 'f', 'i', 'c', 'i', 'e', 'n', 't'>>;
	using resize_signature = meta_hash<type_char<'i', 'm', 'a', 'g', 'e', 'r', 'e', 's', 'i', 'z', 'e'>>;
	
	enum class interp_method {
		area = 0,
		nearest,
		bilinear,
		bicubic,
		lanczos,
		area_fast,
	};

	struct DecimateAlpha {
		int dst_position;
		int src_position;
		float alpha_weight;
	};

	enum class image_convert {
		bgr_rgb,
		rgb_bgr,
		bgr_yuv,
		rgb_yuv,
		yuv_bgr,
		yuv_rgb,
		bgr_nv12,
		rgb_nv12,
		nv12_bgr,
		nv12_rgb,
		bgr_nv21,
		rgb_nv21,
		nv21_bgr,
		nv21_rgb,
		bgr_gray,
		rgb_gray,
		unsupport
	};

	// image resize(only for unsigned char type)
	decltype(auto) get_coef_impl_nearest(int src_width, int dst_width, int channels, AutoBuff<int16_t>& auto_buff) {
		const float scale = src_width / (float)dst_width;
		float offset = scale / 2 - 0.5;
		auto position = auto_buff.get_data(0);
		for (int i = 0; i < dst_width; ++i) {
			const int interger_position = fast_round(max(0.f, i * scale + offset));
			const int dst_position = min(interger_position, src_width - 1) * channels;
			for (int j = 0; j < channels; ++j) {
				*position++ = dst_position + j;
			}
		}
	}
	decltype(auto) get_coef_impl_bicubic(int src_width, int dst_width, int channels, AutoBuff<int16_t>& buff) {
	}
	decltype(auto) get_coef_impl_bilinear(int src_width, int dst_width, int channels, AutoBuff<int16_t>& buff) {
		const float scale = src_width / (float)dst_width;
		float offset = scale / 2 - 0.5;
		auto position = buff.get_data(0);
		auto coef_weight = buff.get_data(1);
		for (int i = 0; i < dst_width; ++i) {
			const float float_position = std::clamp(i * scale, 0.f, (float)src_width - 1);
			const int interger_position = min(fast_floor(float_position), src_width - 2);
			const int channel_position = i * channels;
			const int interger_weight = (float_position - interger_position) * (1 << shift_number<int>);
			const int original_position = interger_position * channels;
			for (int j = 0; j < channels; ++j) {
				*coef_weight++ = interger_weight;
				*position++ = original_position + j;
			}
		}
	}
	
	class CoefEngine final :
		public uncopyable,
		public BaseEngine,
		public SingletonPattern<CoefEngine> {
		using algorithm_handle = std::function<void(Mat, Mat)>;
		using native_handle = decltype(ImplReflection<coef_signature::signature, interp_method, void, int, int, int, AutoBuff<int16_t>>::get_instance());
	private:
		CoefEngine() {
			__handle = ImplReflection<coef_signature::signature, interp_method, void, int, int, int, AutoBuff<int16_t>>::get_instance();
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
			__handle->regist_factory(interp_method::nearest, get_coef_impl_nearest);
			__handle->regist_factory(interp_method::bicubic, get_coef_impl_bicubic);
			__handle->regist_factory(interp_method::bilinear, get_coef_impl_bilinear);
		}
		virtual void __regist_neon_engine() override {
		}
		virtual void __regist_opencl_engine() override {
		}
	private:
		native_handle __handle;
	private:
		friend SingletonPattern<CoefEngine>;
	};

	decltype(auto) get_coef_impl(Mat input, Mat output, AutoBuff<int16_t>& auto_buff, interp_method interpolate_method) {
		auto handle = CoefEngine::get_instance();
		auto ir = handle->get_engine();
		auto op = ir->get_algorithm(interpolate_method);
		op(input.get_width(), output.get_width(), input.get_elements(), auto_buff);
	}
	
	decltype(auto) get_resize_area_tab(int src_width, float offset, int dst_width, int channels, float scale, DecimateAlpha* table) {
		int size = 0;
		for (int i = 0; i < dst_width; ++i) {
			const float float_src_position_left = i * scale + offset;
			const float float_src_position_right = float_src_position_left + scale;
			const float cell_width = min(scale, src_width - float_src_position_left);

			int int_src_position_left_up = fast_ceil(float_src_position_left);
			int int_src_position_right_down = fast_floor(float_src_position_right);

			int_src_position_right_down = min(int_src_position_right_down, src_width - 1);
			int_src_position_left_up = min(int_src_position_left_up, int_src_position_right_down);

			((int_src_position_left_up - float_src_position_left) > 1e-3) | [&]() {
				table[size].dst_position = i * channels;
				table[size].src_position = (int_src_position_left_up - 1) * channels;
				table[size++].alpha_weight = (int_src_position_left_up - float_src_position_left) / cell_width;
			};
			for (int j = int_src_position_left_up; j < int_src_position_right_down; ++j) {
				table[size].dst_position = i * channels;
				table[size].src_position = j * channels;
				table[size++].alpha_weight = 1.0f / cell_width;
			}
			((float_src_position_right - int_src_position_right_down) > 1e-3) | [&]() {
				table[size].dst_position = i * channels;
				table[size].src_position = int_src_position_right_down * channels;
				table[size++].alpha_weight = min(min(float_src_position_right - int_src_position_right_down, 1.0f), cell_width) / cell_width;
			};
		}
		return size;
	}

	decltype(auto) resize_impl_area(Mat input, Mat output) {
		const auto channels = input.get_elements();
		float col_scale = input.get_width() / (float)output.get_width();
		float row_scale = input.get_height() / (float)output.get_height();
		AutoBuff<int32_t> adjust_buff(output.get_height() + 1, 1, 1);
		AutoBuff<DecimateAlpha> alpha_buff(input.get_width() + input.get_height(), 4, 1);
		auto col_table = alpha_buff.get_data(0);
		auto row_table = alpha_buff.get_data(2);
		auto adjust_table = adjust_buff.get_data();
		const int row_table_size = get_resize_area_tab(input.get_height(), 0, output.get_height(), 1, row_scale, row_table);
		const int col_table_size = get_resize_area_tab(input.get_width(), 0, output.get_width(), input.get_elements(), col_scale, col_table);
		for (int i = 0, length = 0; i < row_table_size; ++i) {
			if (i == 0 || (row_table[i].dst_position != row_table[i - 1].dst_position)) {
				adjust_table[length++] = i;
			}
			(row_table_size == i) | [&]() {
				adjust_table[length] = row_table_size;
			};
		}

		parallel_execution(
			0,
			output.get_height(),
			[&](auto begin, auto end) {
				AutoBuff<float> assis_buff(output.get_pitch(), 2, 1);
				auto buf = assis_buff.get_data(0);
				auto sum_buf = assis_buff.get_data(1);

				auto offset_table = adjust_buff.get_data(0);
				std::memset(sum_buf, 0, output.get_pitch() * sizeof(float));
				int pre_dst_position = row_table[adjust_table[begin]].dst_position;

				for (int i = adjust_table[begin]; i < adjust_table[end]; ++i) {
					const float beta_weight = row_table[i].alpha_weight;
					const int row_dst_position = row_table[i].dst_position;
					const int row_src_position = row_table[i].src_position;
					std::memset(buf, 0, output.get_pitch() * sizeof(float));
					auto in = input.ptr<uchar>(row_src_position);
					for (int j = 0; j < col_table_size; ++j) {
						const float alpha_weight = col_table[j].alpha_weight;
						const int col_dst_position = col_table[j].dst_position;
						const int col_src_position = col_table[j].src_position;
						for (int k = 0; k < input.get_elements(); ++k) {
							buf[col_dst_position + k] += in[col_src_position + k] * alpha_weight;
						}
					}
					(row_dst_position == pre_dst_position) | [&]() {
						for (int j = 0; j < output.get_width(); ++j) {
							for (int k = 0; k < channels; ++k) {
								sum_buf[j * channels + k] += beta_weight * buf[j * channels + k];
							}
						}
					};
					(row_dst_position != pre_dst_position) | [&]() {
						auto out = output.ptr<uchar>(pre_dst_position);
						for (int j = 0; j < output.get_width(); ++j) {
							for (int k = 0; k < channels; ++k) {
								*out++ = (uchar)sum_buf[j * input.get_elements() + k];
								sum_buf[j * channels + k] = beta_weight * buf[j * channels + k];
							}
						}
						pre_dst_position = row_dst_position;
					};
				}
				auto out = output.ptr<uchar>(pre_dst_position);
				for (int j = 0; j < output.get_width(); ++j) {
					for (int k = 0; k < channels; ++k) {
						*out++ = (uchar)sum_buf[j * channels + k];
					}
				}
			});
	}
	decltype(auto) resize_impl_nearest(Mat input, Mat output) {
		float col_scale = input.get_width() / (float)output.get_width();
		float row_scale = input.get_height() / (float)output.get_height();
		auto offset = row_scale / 2 - 0.5;
		AutoBuff<int16_t> auto_buff(output.get_width(), 1, output.get_elements());
		get_coef_impl(input, output, auto_buff, interp_method::nearest);
		parallel_execution(0, output.get_height(), [&](auto begin, auto end) {
			for (int i = begin; i < end; ++i) {
				auto out = output.ptr<uchar>(i);
				auto weight = auto_buff.get_data(0);
				auto in = input.ptr<uchar>(min(i * row_scale + offset, input.get_height() - 1));
				for (int j = 0; j < output.get_width(); ++j) {
					for (int k = 0; k < output.get_elements(); ++k) {
						*out++ = in[*weight++];
					}
				}
			}
		});
	}
	decltype(auto) resize_impl_bicubic(Mat input, Mat output) {
	}
	decltype(auto) resize_impl_lanczos(Mat input, Mat output) {
	}
	decltype(auto) resize_impl_bilinear(Mat input, Mat output) {
		float col_scale = input.get_width() / (float)output.get_width();
		float row_scale = input.get_height() / (float)output.get_height();
		auto offset = row_scale / 2 - 0.5;
		AutoBuff<int16_t> auto_buff(output.get_width(), 2, output.get_elements());
		get_coef_impl(input, output, auto_buff, interp_method::bilinear);
		parallel_execution(0, output.get_height(), [&](auto begin, auto end) {
			for (int i = begin; i < end; ++i) {
				auto out = output.ptr<uchar>(i);
				auto weight = auto_buff.get_data(1);
				auto channels = input.get_elements();
				auto position = auto_buff.get_data(0);
				const float float_position = std::clamp(i * row_scale, 0.f, (float)input.get_height() - 1);
				const int interger_position = min(fast_floor(float_position), input.get_height() - 2);
				auto in_up = input.ptr<uchar>(interger_position);
				auto in_down = input.ptr<uchar>(interger_position + 1);
				const int vertical_weight = (float_position - interger_position) * (1 << shift_number<int>);
				for (int j = 0; j < output.get_width(); ++j) {
					for (int k = 0; k < output.get_elements(); ++k) {
						const auto horizontal_weight = *weight++;
						const auto horizontal_position = *position++;
						const auto lt = in_up[horizontal_position];
						const auto lb = in_down[horizontal_position];
						auto tt = lt + (((in_up[horizontal_position + channels] - lt) * horizontal_weight) >> shift_number<int>);
						auto tb = lb + (((in_down[horizontal_position + channels] - lb) * horizontal_weight) >> shift_number<int>);
						*out++ = tt + (((tb - tt) * vertical_weight) >> shift_number<int>);
					}
				}
			}
		});
	}
	decltype(auto) resize_impl_area_fast(Mat input, Mat output) {
		// when the zoom out scale is interger
		return resize_impl_bilinear(input, output);
	}
	
	class ResizeEngine final :
		public uncopyable,
		public BaseEngine,
		public SingletonPattern<ResizeEngine> {
		using algorithm_handle = std::function<void(Mat, Mat)>;
		using native_handle = decltype(ImplReflection<resize_signature::signature, interp_method, void, Mat, Mat>::get_instance());
	private:
		ResizeEngine() {
			__handle = ImplReflection<resize_signature::signature, interp_method, void, Mat, Mat>::get_instance();
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
			__handle->regist_factory(interp_method::area, resize_impl_area);
			__handle->regist_factory(interp_method::nearest, resize_impl_nearest);
			__handle->regist_factory(interp_method::bicubic, resize_impl_bicubic);
			__handle->regist_factory(interp_method::lanczos, resize_impl_lanczos);
			__handle->regist_factory(interp_method::bilinear, resize_impl_bilinear);
			__handle->regist_factory(interp_method::area_fast, resize_impl_area_fast);
		}
		virtual void __regist_neon_engine() override {
			__regist_base_engine();
		}
		virtual void __regist_opencl_engine() override {
		}
	private:
		native_handle __handle;
	private:
		friend SingletonPattern<ResizeEngine>;
	};

	decltype(auto) resize(Mat in, Mat out, interp_method interpolate_method) {
		auto handle = ResizeEngine::get_instance();
		auto ir = handle->get_engine();
		auto op = ir->get_algorithm(interpolate_method);
		op(in, out);
	}

	template<typename _interp = interp_method>
	decltype(auto) imresize(Mat input, Mat output, _interp interpolate_method = interp_method::bilinear) {
		assert(input.get_format() == output.get_format(), "input and output must be the same format");
		(input.size() == output.size()) | [&]() {
			copy(input, output);
		};
		(input.size() != output.size()) | [&]() {
			resize(input, output, interpolate_method);
		};
	}
}