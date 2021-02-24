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

#ifdef _android_platform_
#include <arm_neon.h>
#else
#include <base/neon_windows.h>
#endif

namespace harpocrates {

	using namespace type;
	using namespace engine;
	using namespace operator_reload;

	constexpr static int align_size = 64;

	using filter_signature = meta_hash<type_char<'i', 'm', 'a', 'g', 'e', 'f', 'i', 'l', 't', 'e', 'r'>>;
	
	enum class kernel_type {
		smooth = 0,
		general,
		interger,
		symmeterical,
		asymmeterical,
	};

	enum class filte_method {
		box = 0,
		gabor,
		gauss,
		median,
		bilateral,
		box_gauss,
		adaptive_bilateral,
	};

	// lowcase class only for specific useage
	struct base_filter {
		std::pair<int, int> __size;
		std::pair<int, int> __anchor;
		virtual void apply() = 0;
		virtual void reset() = 0;
		virtual ~base_filter() = 0;
	};

	struct col_filter {
		// __size is the kernel vertical size
		int __size, __anchor;
		virtual void reset() = 0;
		virtual ~col_filter() = 0;
		virtual void apply(const uchar** input, uchar* output, int dst_stride, int dstcount, int width) = 0;
	};

	struct row_filter {
		// __size is the kernel horizontal size
		int __size, __anchor;
		virtual ~row_filter() = 0;
		virtual void apply(const uchar* input, uchar* output, int width, int channels) = 0;
	};

	class symm_filter {
	};

	class linear_filter {
	};

	template<typename _in_type, typename _out_type>
	struct ColSum : public col_filter {
		ColSum(int size, int anchor, float scale) : _size(size), __anchor(anchor), __scale(scale), sum_count(0) {
		}
		void reset() override {
			sum_count = 0;
		}
		void apply(const uchar** input, uchar* output, int dst_stride, int dstcount, int width) override {
			_in_type* sum;
			const bool is_scale = (1 != scale);
			(width != ((int)sum_vec.size())) | [&]() {
				sum_count = 0;
				sum_vec.resize(width);				
			};
			sum = sum_vec[0];
			(0 == sum_count) | [&]() {
				for (int i = 0; i < width; ++i) {
					sum[i] = 0;
				}
				for (sum_count = 0; sum_count < __size - 1; ++sum_count, ++input) {
					int i = 0;
					const _in_type* in = (const _in_type*)input[0];
					for (i = 0; i < width - 2; i += 2) {
						_in_type s_1 = sum[i] + in[i];
						_in_type s_2 = sum[i + 1] + in[i + 1];
						sum[i] = s_1;
						sum[i + 1] = s_2;
					}
					for (; i < width; ++i) {
						sum[i] += in[i];
					}
				}
			};
			(0 != sum_count) | [&]() {
				input += __size - 1;
			};

			for (; count--; input++) {
				const _in_type* in_p = (const _in_type*)input[0];
				const _out_type* out = (const _out_type*)output;
				const _in_type* in_m = (const _in_type*)input[1 - __size];
				(is_scale) | [&]() {
					int i = 0;
					for (i = 0; i <= width - 2; i += 2) {
						_in_type s_1 = sum[i] + in_p[i];
						_in_type s_2 = sum[i + 1] + in_p[i + 1];
						out[i] = (_out_type)s_1 * scale;
						out[i + 1] = (_out_type)s_2 * scale;
						s_1 -= in_m[i];
						s_2 -= in_m[i + 1];
						sum[i] = s_1;
						sum[i + 1] = s_2;
					}
					for (; i < width; ++i) {
						_in_type s = sum[i] + in_p[i];
						out[i] = (_out_type)s_1 * scale;
						sum[i] = s - in_m[i];
					}
				};
				(!is_scale) | [&]() {
					int i = 0;
					for (i = 0; i <= width - 2; i += 2) {
						_in_type s_1 = sum[i] + in_p[i];
						_in_type s_2 = sum[i + 1] + in_p[i + 1];
						out[i] = (_out_type)s_1;
						out[i + 1] = (_out_type)s_2;
						s_1 -= in_m[i];
						s_2 -= in_m[i + 1];
						sum[i] = s_1;
						sum[i + 1] = s_2;
					}
					for (; i < width; ++i) {
						_in_type s = sum[i] + in_p[i];
						out[i] = (_out_type)s_1;
						sum[i] = s - in_m[i];
					}
				};
				out += dst_stride;
			}
		}
		float scale;
		int sum_count;
		std::vector<_in_type> sum_vec;
	};

	template<typename _in_type, typename _out_type>
	struct RowSum : public row_filter {
		RowSum(int size, int anchor) : _size(size), __anchor(anchor) {
		}
		void apply(const uchar* input, uchar* output, int width, int channels) override {
			const int stride = width * channels;
			_out_type* out = (_out_type*)output;
			const _in_type* in = (const _in_type*)input;
			for (int i = 0; i < channels; ++i, ++in, ++out) {
				_out_type sum = 0;
				for (int j = 0; j < __size * channels; j += channels) {
					sum += in[i];
				}
				out[0] = sum;
				for (int j = 0; j < (width - 1) * channels; j += channels) {
					sum += in[j + __size * channels] - in[j];
					out[i + j] = sum;
				}
			}
		};
	};

	struct gaussian_parameter {
		int _;
		float h_sigma;
		float v_sigma;
		gaussian_parameter() {
			_ = 0;
			h_sigma = 1;
			v_sigma = 1;
		}
		gaussian_parameter(float sigma) {
			_ = 0;
			h_sigma = v_sigma = sigma;
		}
		gaussian_parameter(float _1_sigma, float _2_sigma) {
			_ = 0;
			h_sigma = _1_sigma;
			v_sigma = _2_sigma;
		}
		gaussian_parameter& operator= (gaussian_parameter&& other) {
			_ = other._;
			h_sigma = other.h_sigma;
			v_sigma = other.v_sigma;
		}
		gaussian_parameter& operator= (const gaussian_parameter& other) {
			_ = other._;
			h_sigma = other.h_sigma;
			v_sigma = other.v_sigma;
		}
	};
	
	struct bilateral_parameter {
		int __radius;
		float color_sigma;
		float space_sigma;
		bilateral_parameter() {
			__radius = 3;
			color_sigma = 1;
			space_sigma = 1;
		}
		bilateral_parameter(int radius, float sigma) {
			__radius = radius;
			color_sigma = space_sigma = sigma;
		}
		bilateral_parameter(int radius, float _1_sigma, float _2_sigma) {
			__radius = radius;
			color_sigma = _1_sigma;
			space_sigma = _2_sigma;
		}
		bilateral_parameter& operator= (bilateral_parameter&& other) {
			__radius = other.__radius;
			color_sigma = other.color_sigma;
			space_sigma = other.space_sigma;
		}
		bilateral_parameter& operator= (const bilateral_parameter& other) {
			__radius = other.__radius;
			color_sigma = other.color_sigma;
			space_sigma = other.space_sigma;
		}
	};

	struct filter_package {
		Scalar __scalar;
		border_type __col_border;
		border_type __row_border;
		std::pair<int, int> __size;
		std::pair<int, int> __anchor;
		union {
			gaussian_parameter __gaussian;
			bilateral_parameter __bilateral;
		};
		filter_package() {
			__scalar = { 0 };
			__size = { 0, 0 };
			__anchor = { 0, 0 };
			__bilateral = bilateral_parameter();
			__col_border = border_type::reflect_101;
			__row_border = border_type::reflect_101;
		}
		filter_package(
			gaussian_parameter parameter,
			std::pair<int, int> size,
			std::pair<int, int> anchor,
			const border_type& col_border = border_type::reflect_101,
			const border_type& row_border = border_type::reflect_101,
			const Scalar& scalar = { 0 }) {
			__size = size;
			__anchor = anchor;
			__scalar = scalar;
			__gaussian = parameter;
			__col_border = col_border;
			__row_border = row_border;
		}
		filter_package(
			bilateral_parameter parameter,
			std::pair<int, int> size,
			std::pair<int, int> anchor,
			const border_type& col_border = border_type::reflect_101,
			const border_type& row_border = border_type::reflect_101,
			const Scalar& scalar = { 0 }) {
			__size = size;
			__anchor = anchor;
			__scalar = scalar;
			__bilateral = parameter;
			__col_border = col_border;
			__row_border = row_border;
		}
		filter_package(filter_package&& other) {
			__size = other.__size;
			__anchor = other.__anchor;
			__scalar = other.__scalar;
			__bilateral = other.__bilateral;
			__col_border = other.__col_border;
			__row_border = other.__row_border;
		}
		filter_package(const filter_package&& other) {
			__size = other.__size;
			__anchor = other.__anchor;
			__scalar = other.__scalar;
			__bilateral = other.__bilateral;
			__col_border = other.__col_border;
			__row_border = other.__row_border;
		}
		filter_package& operator= (filter_package&& other) {
			__size = other.__size;
			__anchor = other.__anchor;
			__scalar = other.__scalar;
			__bilateral = other.__bilateral;
			__col_border = other.__col_border;
			__row_border = other.__row_border;
		}
		filter_package& operator= (const filter_package& other) {
			__size = other.__size;
			__anchor = other.__anchor;
			__scalar = other.__scalar;
			__bilateral = other.__bilateral;
			__col_border = other.__col_border;
			__row_border = other.__row_border;
		}
	};

	class filter_engine {
	public:
		filter_engine() {
		}
		filter_engine(
			const std::shared_ptr<col_filter>& col_filter,
			const std::shared_ptr<row_filter>& row_filter,
			const std::shared_ptr<base_filter>& base_filter,
			const filter_package& parameter) {
			__initialize(col_filter, row_filter, base_filter, parameter);
		}
		virtual ~filter_engine() {
		}
	public:
		decltype(auto) apply(Mat input, Mat output, const Rect& region = { 0, 0, -1, -1 }, const Point& offset = { 0 }, bool isolated = false) {
			// full image region
			auto region = Rect{ 0, 0, input.get_width(), input.get_height() };
			auto begin = start(input, region, isolated);
		}
		// to be done
		decltype(auto) __locate_roi(std::pair<int, int>& size, std::pair<int, int>&offset) {

		}
		decltype(auto) start(const Mat& input, const Rect& region, bool isolated, int buff_rows = -1) {
			std::pair<int, int> offset;
			std::pair<int, int> size(input.get_width(), input.get_height());
			(!isolated) | [&]() {
				__locate_roi(size, offset);
			};
			start(size, region + offset, buff_rows);
			return __start_y - offset.second;
		}
		decltype(auto) start(std::pair<int, int> whole_size, Rect region, int buff_rows) {
			int i, j;
			const uchar* constVal = !__const_border_value.empty() ? &__const_border_value[0] : nullptr;
			if (buff_rows < 0) {
				buff_rows = __size.second + 3;
			}

			buff_rows = std::max(buff_rows, std::max(__anchor.second, __size.second - __anchor.second - 1) * 2 + 1);

			if (buff_rows < region.region.width || buff_rows != (int)__rows.size()) {
				__rows.resize(buff_rows);
				__max_width = std::max(__max_width, region.region.width);
				__src_row.resize(___ * (__max_width + __size.first - 1));
				if (border_type::constant == __col_border) {
					__const_border_row.resize(__ * (__max_width + __size.first - 1 + align_size));
					uchar *dst = __align_pointer(&__const_border_row[0], align_size), *tdst;
					int n = (int)__const_border_value.size(), N;
					N = (__max_width + __size.first - 1) * esz;
					tdst = is_separable() ? &__src_row[0] : dst;
					for (i = 0; i < N; i += n) {
						n = std::min(n, N - i);
						for (j = 0; j < n; j++) {
							tdst[i + j] = constVal[j];
						}
					}

					if (is_separable()) {
						__row_filter->apply(&__src_row[0], dst, __max_width, channels);
					}
				}

				int __max_buff_step = __ * (int)__align_size(__max_width + (!is_separable() ? __size.first - 1 : 0), align_size);
				__ring_buff.resize(__max_buff_step * __rows.size() + align_size);
			}

			// adjust buff_step so that the used part of the ring buffer stays compact in memory
			__buff_step = __ * (int)__align_size(region.region.width + (!is_separable() ? __size.first - 1 : 0), align_size);

			dx1 = std::max(__anchor.first - roi.x, 0);
			dx2 = std::max(__size.first - __anchor.first - 1 + region.region.x + region.region.width - whole_size.first, 0);

			// recompute border tables
			if (dx1 > 0 || dx2 > 0) {
				if (border_type::constant == __col_border) {
					int nr = is_separable() ? 1 : (int)__rows.size();
					for (i = 0; i < nr; i++) {
						uchar* dst = is_separable() ? &__src_row[0] : __align_pointer(&__ring_buff[0], align_size) + __buff_step * i;
						memcpy(dst, constVal, dx1*esz);
						memcpy(dst + (roi.width + __size.first - 1 - dx2) * esz, constVal, dx2*esz);
					}
				}
				else {
					int xofs1 = std::min(roi.x, __anchor.first) - roi.x;

					int btab_esz = borderElemSize, wholeWidth = wholeSize.width;
					int* btab = (int*)&borderTab[0];

					for (i = 0; i < dx1; i++)
					{
						int p0 = (borderInterpolate(i - dx1, wholeWidth, rowBorderType) + xofs1)*btab_esz;
						for (j = 0; j < btab_esz; j++)
							btab[i*btab_esz + j] = p0 + j;
					}

					for (i = 0; i < dx2; i++)
					{
						int p0 = (borderInterpolate(wholeWidth + i, wholeWidth, rowBorderType) + xofs1)*btab_esz;
						for (j = 0; j < btab_esz; j++)
							btab[(i + dx1)*btab_esz + j] = p0 + j;
					}
				}
			}

			__row_count = dstY = 0;
			__start_y = startY0 = std::max(roi.y - anchor.y, 0);
			__end_y = std::min(roi.y + roi.height + ksize.height - anchor.y - 1, wholeSize.height);
			if (!__col_filter) {
				__col_filter->reset();
			}
			if (!__2d_filter) {
				__2d_filter->reset();
			}
			return __start_y;
		}
	public:
		auto is_separable() -> bool {
			return (nullptr == __2d_filter);
		}
	private:
		[[noreturn]]void __initialize(
			const std::shared_ptr<col_filter>& col_filter,
			const std::shared_ptr<row_filter>& row_filter,
			const std::shared_ptr<base_filter>& base_filter,
			const filter_package& parameter) {
			__col_filter = col_filter;
			__row_filter = row_filter;
			__2d_filter = base_filter;
			(is_separable()) | [&]() {
				__size.first = __col_filter->__size;
				__size.second = __row_filter->__size;
				__anchor.first = __col_filter->__anchor;
				__anchor.second = __row_filter->__anchor;
			};
			(!is_separable()) | [&]() {
				__size = __2d_filter->__size;
				__anchor = __2d_filter->__anchor;
			};
		}
	private:
		int __start_y;
		bool isolated;
		int __max_width;
		Scalar __scalar;
		border_type __col_border;
		border_type __row_border;
		std::vector<uchar*> __rows;
		std::pair<int, int> __size;
		std::pair<int, int> __anchor;
		std::vector<uchar> __src_row;
		std::vector<uchar> __ring_buff;
		std::vector<uchar> __const_border_value;
		std::vector<uchar> __const_border_row;
	private:
		std::shared_ptr<col_filter> __col_filter;
		std::shared_ptr<row_filter> __row_filter;
		std::shared_ptr<base_filter> __2d_filter;
	};

	decltype(auto) image_filter_impl_box_filter(Mat input, Mat output, const filter_package& parameter) {
		auto area = parameter.__size.first * parameter.__size.second;
		auto _row_filter = std::shared_ptr<row_filter>(new RowSum<uchar, int>(parameter.__size.first, parameter.__anchor.first));
		auto _col_filter = std::shared_ptr<col_filter>(new ColSum<uchar, int>(parameter.__size.first, parameter.__anchor.first, 1.0f / area));
		auto _filter_engine = std::shared_ptr<filter_engine>(new filter_engine(_col_filter, _row_filter, std::shared_ptr<base_filter>(), parameter));
		_filter_engine->apply(input, output);
	}

	decltype(auto) image_filter_impl_gabor_filter(Mat input, Mat output, const filter_package& parameter) {
	}
	
	decltype(auto) image_filter_impl_gauss_filter(Mat input, Mat output, const filter_package& parameter) {
	}
	
	decltype(auto) image_filter_impl_median_filter(Mat input, Mat output, const filter_package& parameter) {
	}
	
	decltype(auto) image_filter_impl_bilateral_filter(Mat input, Mat output, const filter_package& parameter) {
	}
	
	decltype(auto) image_filter_impl_adative_bilateral(Mat input, Mat output, const filter_package& parameter) {
	}

	class FilterEngine final :
		public uncopyable,
		public BaseEngine,
		public SingletonPattern<FilterEngine> {
		using algorithm_handle = std::function<void(Mat, Mat, const filter_package&)>;
		using native_handle = decltype(ImplReflection<filter_signature::signature, filte_method, void, Mat, Mat, const filter_package&>::get_instance());
	private:
		FilterEngine() {
			__handle = ImplReflection<filter_signature::signature, filte_method, void, Mat, Mat, const filter_package&>::get_instance();
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
			__handle->regist_factory(filte_method::box, image_filter_impl_box_filter);
			__handle->regist_factory(filte_method::gabor, image_filter_impl_gabor_filter);
			__handle->regist_factory(filte_method::gauss, image_filter_impl_gauss_filter);
			__handle->regist_factory(filte_method::median, image_filter_impl_median_filter);
			__handle->regist_factory(filte_method::bilateral, image_filter_impl_bilateral_filter);
			__handle->regist_factory(filte_method::adaptive_bilateral, image_filter_impl_adative_bilateral);
		}
		virtual void __regist_neon_engine() override {
		}
		virtual void __regist_opencl_engine() override {
		}
	private:
		native_handle __handle;
	private:
		friend SingletonPattern<FilterEngine>;
	};

	//! lut only for gray scale image
	template<typename _type>
	decltype(auto) lut(Mat input, _type table) {
		static_assert(((std::is_same_v<uchar*, std::decay_t<_type>>) || (std::is_same_v<AutoBuff<uchar>, std::decay_t<_type>>)), "wrong table type");
		parallel_execution(
			0,
			input.get_height(),
			[&](auto begin, auto end) {
				for (int i = begin; i < end; ++i) {
					auto in = input.ptr<uchar>(i);
					for (int j = 0; j < input.get_width(), ++j) {
						*in++ = table[*in];
					}
				}
		    }
		);
	}

	template<typename _type>
	decltype(auto) lut(Mat input, Mat output, _type table) {
		static_assert(((std::is_same_v<uchar*, std::decay_t<_type>>) || (std::is_same_v<AutoBuff<uchar>, std::decay_t<_type>>)), "wrong table type");
		parallel_execution(
			0,
			input.get_height(),
			[&](auto begin, auto end) {
				for (int i = begin; i < end; ++i) {
					auto in = input.ptr<uchar>(i);
					auto out = output.ptr<uchar>(i);
					for (int j = 0; j < input.get_width(), ++j) {
						*out++ = table[*in++];
					}
				}
			}
		);
	}

	template<typename _type, typename _filter = filte_method>
	decltype(auto) imfilter(Mat input, Mat output, _filter filter_method, const filter_package& parameter) {
		assert(input.size() == output.size(), "input and output must be the same format");

	}

}