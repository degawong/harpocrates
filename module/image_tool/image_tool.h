/*
 * @Author: your name
 * @Date: 2020-04-30 10:39:53
 * @LastEditTime: 2020-08-04 16:25:09
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \harpocrates\module\image_tool\image_tool.h
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

#include <image/stb_image.h>
#include <image/stb_image_write.h>

#include <thread_pool/thread_pool.h>
#include <singleton_pattern/singleton_pattern.h>

#ifdef _android_platform_
#include <arm_neon.h>
#else
#include <base/neon_windows.h>
#endif


namespace harpocrates {

	using namespace type;
	using namespace image;
	using namespace operator_reload;

	template<typename _type>
	// downcase word class only for private use
	class iterator : public std::iterator<std::random_access_iterator_tag, typename _type::value_type> {
	public:
		using pointer = typename _type::pointer;
		using reference = typename _type::reference;
		using value_type = typename _type::value_type;
		using difference_type = typename _type::difference_type;
		using iterator_category = typename _type::iterator_category;
	public:
		iterator() : __data(nullptr) {
		};
		~iterator() {
			__data = nullptr;
		};
	public:
		iterator(int verbose, pointer data) {
			__data = data;
			__verbose = verbose;
		}
	public:
		iterator(const iterator& iter) {
			__data = iter.__data;
			__verbose = iter.__verbose;
		}
		iterator(iterator&& iter) {
			__data = iter.__data;
			__verbose = iter.__verbose;
		}
	public:
		auto operator++ () {
			__data += 1;
			return *this;
		}
		auto operator++ (int) {
			auto value = *__data;
			__data += 1;
			return value;
		}
		auto operator-- () {
			__data -= 1;
			return *this;
		}
		auto operator-- (int) {
			auto value = *__data;
			__data -= 1;
			return value;
		}
		template<typename _diff_type>
		auto operator+ (_diff_type step) {
			int multi = 3;
			(65792 == __verbose) | [&]() {
				multi = 1;
			};
			return iterator<_type>(__verbose, __data + multi * step);
		}
		template<typename _diff_type>
		auto operator- (_diff_type step) {
			int multi = 3;
			(65792 == __verbose) | [&]() {
				multi = 1;
			};
			return iterator<_type>(__verbose, __data - multi * step);
		}
		auto operator+ (const iterator& iter) {
			int multi = 3;
			(65792 == __verbose) | [&]() {
				multi = 1;
			};
			return (__data - iter.__data) / (multi * sizeof(value_type));
		}
		auto operator- (const iterator& iter) {
			int multi = 3;
			(65792 == __verbose) | [&]() {
				multi = 1;
			};
			return (__data - iter.__data) / (multi * sizeof(value_type));
		}
		auto operator= (iterator&& iter) {
			__data = iter.__data;
			__verbose = iter.__verbose;
			return *this;
		}
		auto operator= (const iterator& iter) {
			__data = iter.__data;
			__verbose = iter.__verbose;
			return *this;
		}
		auto operator* () {
			return __data;
		}
		auto operator> (const iterator& iter) const {
			return __data > iter.__data;
		}
		auto operator< (const iterator& iter) const {
			return __data < iter.__data;
		}
		auto operator>= (const iterator& iter) const {
			return __data >= iter.__data;
		}
		auto operator<= (const iterator& iter) const {
			return __data <= iter.__data;
		}
		auto operator== (const iterator& iter) const {
			return __data == iter.__data;
		}
		auto operator!= (const iterator& iter) const {
			return __data != iter.__data;
		}
		auto operator>> (std::istream& iter) const {
			return iter >> *__data;
		}
		auto operator<< (std::ostream& iter) const {
			return iter << *__data;
		}
	private:
		int __verbose;
		pointer __data;
	private:
		friend _type;
	};

	template<class _derived>
	class ReferCount {
	protected:
		auto _shallow_clean() {
			__release();
		}
		auto _get_refer() {
			return __refer_count;
		}
		auto _get_refer() const {
			return __refer_count;
		}
		auto _add_ref_count() {
			(nullptr != __refer_count) | [this]() {
				(*__refer_count)++;
			};
		}
		auto _dec_ref_count() {
			(__derived().__shareable) | [this]() {
				(nullptr != __refer_count) | [this]() {
					(1 == ((*__refer_count)--)) | [this]() {
						__uninit();
					};
					__release();
				};
			};
		}
		auto _init(int* ref_count) {
			__refer_count = ref_count;
		}
	private:
		_derived& __derived() { 
			return *static_cast<_derived*>(this); 
		}
		const _derived& __derived() const {
			return *static_cast<const _derived*>(this); 
		}
		auto __release() {
			__refer_count = nullptr;
			__derived().__shallow_clean();
		}
		auto __uninit() {
			delete __refer_count;
			__derived().__dellocator();
		}
	protected:
		int* __refer_count;
	};

	template<typename _data_type = unsigned char, int _align_size = 64>
	class MatData : public ReferCount<MatData<_data_type, _align_size>> {
	public:
		using pointer = _data_type *;
		using value_type = _data_type;
		using reference = _data_type &;
		using difference_type = ptrdiff_t;
		using iterator_category = std::random_access_iterator_tag;
	public:
		MatData(bool shareable = true) : __shareable(shareable) {
			_shallow_clean();
			__shareable | [this]() {
				_init(new int(1));
			};
		}
		MatData(int width, int height, int code_format) noexcept
			: __width(width), __height(height), __pitch{ 0 }, __shareable(true), __chunck_size{ 0 }, __code_format(code_format), __data{ nullptr } {
			_init(new int(1));
			__allocator();
		}
		virtual ~MatData() {
			_dec_ref_count();
		}
	public:
		MatData(MatData&& object) noexcept {
			_shallow_clean();
			_init(object._get_refer());
			__width = object.__width;
			__height = object.__height;
			__shareable = object.__shareable;
			__code_format = object.__code_format;
			for (size_t i = 0; i < 4; ++i) {
				__data[i] = object.__data[i];
				__pitch[i] = object.__pitch[i];
			}
			object._shallow_clean();
		}
		MatData(const MatData& object) noexcept {
			_shallow_clean();
			_init(object._get_refer());
			__width = object.__width;
			__height = object.__height;
			__shareable = object.__shareable;
			__code_format = object.__code_format;
			for (size_t i = 0; i < 4; ++i) {
				__data[i] = object.__data[i];
				__pitch[i] = object.__pitch[i];
			}
			if (__shareable) {
				_add_ref_count();
			}
		}
	public:
		MatData& operator= (MatData&& object) {
			if (&object != this) {
				_dec_ref_count();
				_shallow_clean();
				_init(object._get_refer());
				__width = object.__width;
				__height = object.__height;
				__shareable = object.__shareable;
				__code_format = object.__code_format;
				for (size_t i = 0; i < 4; ++i) {
					__data[i] = object.__data[i];
					__pitch[i] = object.__pitch[i];
				}
				object._shallow_clean();
			}
			return *this;
		}
		MatData& operator= (const MatData& object) {
			if (&object != this) {
				_dec_ref_count();
				_shallow_clean();
				_init(object._get_refer());
				_add_ref_count();
				__width = object.__width;
				__height = object.__height;
				__shareable = object.__shareable;
				__code_format = object.__code_format;
				for (size_t i = 0; i < 4; ++i) {
					__data[i] = object.__data[i];
					__pitch[i] = object.__pitch[i];
				}
			}
			return *this;
		}
	private:
		auto __copy_to_image(int width, int height, int channel, const _data_type* data) {
			auto res = return_code::success;
			return res;
		}
		auto __copy_from_image(int width, int height, int channel, const _data_type* data) {
			auto res = return_code::success;
			(!((width == __width) && (height == __height) && (channel == __parse_format_code<image_info::element_number>()))) | [&]() {
				res = return_code::not_match;
			};
			if (return_code::success != res) {
				return res;
			}
			for (int i = 0; i < __height; ++i) {
				std::copy_n(&data[i * width * channel], width * channel, &__data[0][i * __pitch[0]]);
			}
			return res;
		}
	public:
		template<typename _ptr_type>
		decltype(auto) ptr(int i) {
			return &__data[0][i * __pitch[0]];
		}
		MatData crop(int left, int right, int top, int bottom) {
			return rect(left, right, top, bottom);
		}
		MatData rect(int left, int right, int top, int bottom) {
			MatData region(false);
			region.__width = right - left;
			region.__height = bottom - top;
			region.__code_format = __code_format;
			// to be modification
			region.__pitch[0] = __pitch[0];
			region.__data[0] = &(__data[0][top * __pitch[0] + left * __parse_format_code<image_info::element_number>()]);
			if(any_equel(__code_format, 131328, 131329)) {
				region.__pitch[1] = __pitch[1];
				region.__data[1] = &(__data[1][(top >> 1) * __pitch[0] + ((left >> 1) << 1)]);
			}
			return region;
		}
	public:
		auto begin() {
			return iterator<MatData>(__code_format, __data[0]);
		}
		auto end() {
			return iterator<MatData>(__code_format, &__data[0][__height * __pitch[0]]);
		}
	public:
		_data_type* operator[] (int index) {
			return __data[index];
		}
		_data_type* operator[] (int index) const {
			return __data[index];
		}
		template<typename _out_iterator>
		friend const _out_iterator& operator<< (_out_iterator& os, const MatData& mat) {
			for (int i = 0; i < mat.get_height(); ++i) {
				for (int j = 0; j < mat.get_width(); ++j) {
					for (int k = 0; k < mat.get_elements(); ++k) {
						os << std::right << std::setw(3) << int(mat[0][i * mat.get_pitch() + j * mat.get_elements() + k]) << "  ";
					}
				}
				os << std::endl;
			}
			return os;
		}
	public:
		_data_type* data(int index = 0) {
			return __data[index];
		}
		_data_type* data(int index = 0) const {
			return __data[index];
		}
		MatData& copy() {
			auto ret = MatData(__width, __height, __code_format);
			ret.__copy_data(*this);
			return ret;
		}
		auto set_value(_data_type value) {
			// assert(__code_format != base::image_format::image_format_nv12 || __code_format != base::image_format::image_format_nv21);
			// it is correct only the data type is single type
			// if the data type is int, then the initial value
			// is 0x(value)(value)(value)(value)
			// data is or not continues in the rows's point of view
			if (__pitch[0] == __cacu_pitch(__width, 8 * __parse_format_code<image_info::element_number>())) {
				std::memset(__data[0], value, __chunck_size[0]);
			}
			else {
				for (int i = 0; i < __height; ++i) {
					std::memset(&__data[0][i * __pitch[0]], value, __width * __parse_format_code<image_info::element_number>());
				}
			}
		}
	public:
		std::string get_info(std::string name = "dummy") {
			std::stringstream info;
			info << name << " -- " << "width : " << __width << ", height : " << __height << ", format : " << __code_format << std::endl;
			return info.str();
		}
	public:
		const int get_width() const {
			return __width;
		}
		const int get_height() const {
			return __height;
		}
		const int get_pitch(int query = 0) const {
			return __pitch[query];
		}
		const int get_elements() const {
			return __parse_format_code<image_info::element_number>();
		}
		const int get_format_code() const {
			return __code_format;
		}
		// when the data format is nv12 or nv21, be careful
		_data_type* get_data(int plane_index = 0, int row = 0) {
			return &__data[plane_index][(row) * __pitch[plane_index]];
		}
		const _data_type* get_data(int plane_index = 0, int row = 0) const {
			return &__data[plane_index][row * __pitch[plane_index]];
		}
	private:
		void __copy_data(const MatData& object) {
			if (__pitch[0] == __cacu_pitch(__width, 8 * __parse_format_code<image_info::element_number>())) {
				// continues
				for (size_t i = 0; i < __parse_format_code<image_info::plane_number>(); ++i) {
					std::copy_n(object.__data[i], __chunck_size[i], __data[i]);
				}
			}
			else {
				// not continues(rect  region)
				for (size_t i = 0; i < __parse_format_code<image_info::plane_number>(); ++i) {
					for (int i = 0; i < __height; ++i) {
						std::copy_n(&object.__data[i][i * __pitch[i]], __pitch[i], &__data[i][i * __pitch[i]]);
					}
				}
			}
		}
	private:
		auto __shallow_clean() {
			__width = 0;
			__height = 0;
			__code_format = 0;
			for (size_t i = 0; i < 4; ++i) {
				__pitch[i] = 0;
				__data[i] = nullptr;
				__chunck_size[i] = 0;
			}
		}
		auto __allocator() {
			__get_format_details();
			for (size_t i = 0; i < __parse_format_code<image_info::plane_number>(); ++i) {
				__data[i] = new _data_type[__chunck_size[i]]{ 0 };
			}
		}
		auto __dellocator() {
			for (size_t i = 0; i < 4; ++i) {
				delete[] __data[i];
			}
		}
	private:
		template<image_info _query>
		int __parse_format_code() const {
			return (__code_format >> (8 * (int(_query) - 1))) & 0x00ff;
		}
		int __cacu_pitch(int width, int bit_count) {
			return (((int)(width) * (bit_count)+31) / 32 * 4);
		}
		void __cacu_chunck_size() {
			for (size_t i = 0; i < 4; ++i) {
				__chunck_size[i] = __height * __pitch[i];
			}
		}
		void __adjust_chunck_size() {
			(any_equel(__code_format, 131328, 131329)) | [this]() {
				__chunck_size[1] = __height * __pitch[1] / 2;
			};
		}
		void __get_format_details() {
			for (size_t i = 0; i < __parse_format_code<image_info::plane_number>(); ++i) {
				__pitch[i] = __cacu_pitch(__width, 8 * __parse_format_code<image_info::element_number>());
			}
			__cacu_chunck_size();
			__adjust_chunck_size();
		}
	private:
		decltype(auto) signature() {
		}
    private:
		bool __shareable;
	private:
		int __width;
		int __height;
		int __pitch[4];
		int __code_format;
		int __chunck_size[4];
	private:
		alignas(_align_size) _data_type* __data[4];
		static_assert((_align_size >= 16) && (_align_size % 16 == 0));
	private:
		friend iterator<MatData<_data_type, _align_size>>;
		friend ReferCount<MatData<_data_type, _align_size>>;
	};

	template<typename _data_type, int _align_size>
	class Tensor;

	template<class _derived>
	class TensorRefer {
	protected:
		auto _get_refer() {
			return __refer_count;
		}
		auto _shallow_clean() {
			__release();
		}
		auto _add_ref_count() {
			(nullptr != __refer_count) | [this]() {
				(*__refer_count)++;
			};
		}
		auto _dec_ref_count() {
			(nullptr != __refer_count) | [this]() {
				(1 == ((*__refer_count)--)) | [this]() {
					__uninit();
				};
				__release();
			};
		}
		auto _init(int* ref_count) {
			__refer_count = ref_count;
		}
	private:
		_derived& __derived() { 
			return *static_cast<_derived*>(this); 
		}
		const _derived& __derived() const {
			return *static_cast<const _derived*>(this); 
		}
		auto __release() {
			__refer_count = nullptr;
			__derived().__shallow_clean();
		}
		auto __uninit() {
			delete __refer_count;
			//__derived().__dellocator();
			__derived().__align_dellocator();
		}
	private:
		int* __refer_count;
	};

	template<typename _data_type = unsigned char, int _align_size = 64>
	class Tensor : public TensorRefer<Tensor<_data_type, _align_size>> {
	public:
		Tensor() {
			_shallow_clean();
			_init(new int(1));
		}
		Tensor(int cols, int rows, int channels) : __cols(cols), __rows(rows), __channels(channels), __data(nullptr) {
			_init(new int(1));
			//__allocator();
			__align_allocator();
		}
		~Tensor() {
			_dec_ref_count();
		}
	public:
		Tensor(Tensor&& object) {
			_shallow_clean();
			_init(object._get_refer());
			__cols = object.__cols;
			__rows = object.__rows;
			__data = object.__data;
			__channels = object.__channels;
			object._shallow_clean();
		}
		Tensor(const Tensor& object) {
			_shallow_clean();
			_init(object._get_refer());
			_add_ref_count();
			__cols = object.__cols;
			__rows = object.__rows;
			__data = object.__data;
			__channels = object.__channels;
		}
	public:
		Tensor& operator= (Tensor&& object) {
			if (this != &object) {
				_dec_ref_count();
				_shallow_clean();
				_init(object._get_refer());
				__cols = object.__cols;
				__rows = object.__rows;
				__data = object.__data;
				__channels = object.__channels;
				object._shallow_clean();
			}
			return *this;
		}
		Tensor& operator= (const Tensor& object) {
			if (this != &object) {
				_dec_ref_count();
				_shallow_clean();
				_init(object._get_refer());
				_add_ref_count();
				__cols = object.__cols;
				__rows = object.__rows;
				__data = object.__data;
				__channels = object.__channels;
			}
			return *this;
		}
	public:
		int cols() const {
			return __cols;
		}
		int rows() const {
			return __rows;
		}
		int channels() const {
			return __channels;
		}
	public:
		_data_type* operator[] (int index) {
			return __data[index];
		}
		_data_type* operator[] (int index) const {
			return __data[index];
		}
		_data_type* get_data(int cols, int rows) {
			return __data + rows * __cols * __channels + cols * __channels;
		}
		_data_type* get_data(int cols, int rows) const {
			return __data + rows * __cols * __channels + cols * __channels;
		}
		template<typename _out_iterator>
		friend const _out_iterator& operator<< (_out_iterator& os, const Tensor& tensor) {
			for (int i = 0; i < tensor.rows(); ++i) {
				for (int j = 0; j < tensor.cols(); ++j) {
					for (int k = 0; k < tensor.channels(); ++k) {
						os << std::right << std::setw(3) << int(tensor.get_data(i, j)[k]) << "  ";
					}
				}
				os << std::endl;
			}
			return os;
		}
	private:
		void __shallow_clean() {
			__cols = 0;
			__rows = 0;
			__channels = 0;
			__data = nullptr;
		}
		void __allocator() {
			__data = new _data_type[__cols * __rows * __channels * sizeof(_data_type)]{ 0 };
		}
		void __dellocator() {
			delete[] __data;
		}
		//ʵ���ڼ����ϵͳ�У��������ڴ��ж����Բ������ʽ���д洢��
		//��n����16ʱ����-n��Ϊ-16�� -16��ԭ��Ϊ1000...0010000, �䷴��Ϊ1111...1101111,
		//����Ϊ1111...1110000���൱��ȡ���ǵ�ַ�ĸ�λ����λֱ�ӽضϽ��ж���
		template<typename data_type>
		data_type* __align_pointer(data_type* ptr, int align_size = _align_size) {
			return (data_type*)(((size_t)ptr + align_size - 1) & -align_size);
		}
		void __align_allocator() {
			// sizeof(void*) is the u_ptr address size
			_data_type* u_ptr = (_data_type*)std::malloc(__cols * __rows * __channels * sizeof(_data_type) + sizeof(void*) + _align_size);
			_data_type** a_ptr = __align_pointer<_data_type*>((_data_type**)u_ptr + 1, _align_size);
			a_ptr[-1] = u_ptr;
			__data = (_data_type*)a_ptr;
		}
		void __align_dellocator() {
			auto u_ptr = ((_data_type**)__data)[-1];
			std::free(u_ptr);
		}
	private:
		decltype(auto) signature() {
		}
	private:
		int __cols;
		int __rows;
		int __channels;
		alignas(_align_size) _data_type* __data;
	private:
		friend TensorRefer<Tensor<_data_type, _align_size>>;
	};

	using Mat = MatData<unsigned char, 64>;

	class AutoBuff : public SingletonPattern<AutoBuff> {
	public:
		~AutoBuff() {
			(nullptr != __data) | [this]() {
				delete[] __data;
			};
		}
	private:
		AutoBuff(int cols = 1024, int rows = 1024, int channels = 1) : 
		    __cols(cols), __rows(rows), __channels(channels), __data(nullptr) {
			__data = new uchar[__cols * __rows * __channels]{ 0 };
		}
	public:
		decltype(auto) get_data() {
			return __data;
		}
	private:
		int __cols;
		int __rows;
		int __channels;
		alignas(64) uchar * __data;
	private:
		friend SingletonPattern<AutoBuff>;
	};

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

	namespace neon {

	}

	namespace image {
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
					r = ((y_00) + r_diff + 16) >> 5;
					g = ((y_00) - g_diff + 16) >> 5;
					b = ((y_00) + b_diff + 16) >> 5;
					*(dst_0++) = std::clamp(b, 0, 255);
					*(dst_0++) = std::clamp(g, 0, 255);
					*(dst_0++) = std::clamp(r, 0, 255);
					r = ((y_01) + r_diff + 16) >> 5;
					g = ((y_01) - g_diff + 16) >> 5;
					b = ((y_01) + b_diff + 16) >> 5;
					*(dst_0++) = std::clamp(b, 0, 255);
					*(dst_0++) = std::clamp(g, 0, 255);
					*(dst_0++) = std::clamp(r, 0, 255);
					r = ((y_10) + r_diff + 16) >> 5;
					g = ((y_10) - g_diff + 16) >> 5;
					b = ((y_10) + b_diff + 16) >> 5;
					*(dst_1++) = std::clamp(b, 0, 255);
					*(dst_1++) = std::clamp(g, 0, 255);
					*(dst_1++) = std::clamp(r, 0, 255);
					r = y_11 + r_diff;
					g = y_11 - g_diff;
					b = y_11 + b_diff;
					r = ((y_11) + r_diff + 16) >> 5;
					g = ((y_11) - g_diff + 16) >> 5;
					b = ((y_11) + b_diff + 16) >> 5;
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
					r = ((y_00) + r_diff + 16) >> 5;
					g = ((y_00) - g_diff + 16) >> 5;
					b = ((y_00) + b_diff + 16) >> 5;
					*(dst_0++) = std::clamp(r, 0, 255);
					*(dst_0++) = std::clamp(g, 0, 255);
					*(dst_0++) = std::clamp(b, 0, 255);
					r = ((y_01) + r_diff + 16) >> 5;
					g = ((y_01) - g_diff + 16) >> 5;
					b = ((y_01) + b_diff + 16) >> 5;
					*(dst_0++) = std::clamp(r, 0, 255);
					*(dst_0++) = std::clamp(g, 0, 255);
					*(dst_0++) = std::clamp(b, 0, 255);
					r = ((y_10) + r_diff + 16) >> 5;
					g = ((y_10) - g_diff + 16) >> 5;
					b = ((y_10) + b_diff + 16) >> 5;
					*(dst_1++) = std::clamp(r, 0, 255);
					*(dst_1++) = std::clamp(g, 0, 255);
					*(dst_1++) = std::clamp(b, 0, 255);
					r = y_11 + r_diff;
					g = y_11 - g_diff;
					b = y_11 + b_diff;
					r = ((y_11) + r_diff + 16) >> 5;
					g = ((y_11) - g_diff + 16) >> 5;
					b = ((y_11) + b_diff + 16) >> 5;
					*(dst_1++) = std::clamp(r, 0, 255);
					*(dst_1++) = std::clamp(g, 0, 255);
					*(dst_1++) = std::clamp(b, 0, 255);
				}
			}
		}
	
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

		decltype(auto) color_convert(Mat in, Mat out) {
			auto res = return_code::success;
			if ((in.get_width() != out.get_width()) || (in.get_height() != out.get_height())) {
				res = return_code::unsupport;
			}
			// bgr <==> rgb
			((in.get_format_code() == 66304) && (out.get_format_code() == 66305)) | [&]() {
				return color_convert_impl_bgr_2_rgb(in, out);
			};
			((in.get_format_code() == 66305) && (out.get_format_code() == 66304)) | [&]() {
				return color_convert_impl_bgr_2_rgb(in, out);
			};
			// bgr <==> yuv
			((in.get_format_code() == 66304) && (out.get_format_code() == 66306)) | [&]() {
				return color_convert_impl_bgr_2_yuv(in, out);
			};
			((in.get_format_code() == 66305) && (out.get_format_code() == 66306)) | [&]() {
				return color_convert_impl_rgb_2_yuv(in, out);
			};
			((in.get_format_code() == 66306) && (out.get_format_code() == 66304)) | [&]() {
				return color_convert_impl_yuv_2_bgr(in, out);
			};
			((in.get_format_code() == 66306) && (out.get_format_code() == 66305)) | [&]() {
				return color_convert_impl_yuv_2_rgb(in, out);
			};
			// bgr <==> nv12
			((in.get_format_code() == 66304) && (out.get_format_code() == 131328)) | [&]() {
				return color_convert_impl_bgr_2_nv12(in, out);
			};
			((in.get_format_code() == 66305) && (out.get_format_code() == 131328)) | [&]() {
				return color_convert_impl_rgb_2_nv12(in, out);
			};
			((in.get_format_code() == 131328) && (out.get_format_code() == 66304)) | [&]() {
				return color_convert_impl_nv12_2_bgr(in, out);
			};
			((in.get_format_code() == 131328) && (out.get_format_code() == 66305)) | [&]() {
				return color_convert_impl_nv12_2_rgb(in, out);
			};
			// bgr <==> nv21
			((in.get_format_code() == 66304) && (out.get_format_code() == 131329)) | [&]() {
				return color_convert_impl_bgr_2_nv21(in, out);
			};
			((in.get_format_code() == 66305) && (out.get_format_code() == 131329)) | [&]() {
				return color_convert_impl_rgb_2_nv21(in, out);
			};
			((in.get_format_code() == 131329) && (out.get_format_code() == 66304)) | [&]() {
				return color_convert_impl_nv21_2_bgr(in, out);
			};
			((in.get_format_code() == 131329) && (out.get_format_code() == 66305)) | [&]() {
				return color_convert_impl_nv21_2_rgb(in, out);
			};
			return res;
		}

		template<typename _type, typename _format = image_format>
		decltype(auto) imread(_type path, _format format) {
			auto image = image_io().read_image(path);
			every_not_eque(int(format), 66305, 65792) | [&]() {
				Mat expect(image.get_width(), image.get_height(), int(format));
				auto _ = color_convert(image, expect);
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
				auto _ = color_convert(image, rgb);
				return image_io().write_image(rgb, path);
			};
		}

		template<typename _type, typename _interp = interp_method>
		decltype(auto) imresize(Mat input, Mat output, _interp interpolate_method = interp_method::bilinear) {

		}
	
		template<typename _type, typename _filter = filte_method>
		decltype(auto) imfilter(Mat input, Mat output, Tensor<uchar, 64> filter, _filter filter_method = filte_method::gaussian) {

		}

	}
}

