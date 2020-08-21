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

	//enum { XY_SHIFT = 16, XY_ONE = 1 << XY_SHIFT, DRAWING_STORAGE_BLOCK = (1 << 12) - 256 };
	constexpr static int shift_count = 16;
	constexpr static int shift_65536 = 1 << shift_count;
	constexpr static int storage_block = (1 << 12) - 256;

	enum class line_type {
		normal,
		anti_alias,
	};

	template<typename _type>
	struct _point {
		_type x;
		_type y;
	};

	using Point = _point<int>;

	struct poly_edge {
		poly_edge() : y0(0), y1(0), x(0), dx(0), next(0) {
		}
		int y0, y1;
		int x, dx;
		poly_edge *next;
	};

	decltype(auto) Line(Mat& img, Point pt1, Point pt2,
		const void* _color, int connectivity = 8)
	{
		if (connectivity == 0)
			connectivity = 8;
		if (connectivity == 1)
			connectivity = 4;

		LineIterator iterator(img, pt1, pt2, connectivity, true);
		int i, count = iterator.count;
		int pix_size = (int)img.elemSize();
		const uchar* color = (const uchar*)_color;

		for (i = 0; i < count; i++, ++iterator)
		{
			uchar* ptr = *iterator;
			if (pix_size == 1)
				ptr[0] = color[0];
			else if (pix_size == 3)
			{
				ptr[0] = color[0];
				ptr[1] = color[1];
				ptr[2] = color[2];
			}
			else
				memcpy(*iterator, color, pix_size);
		}
	}

	template<typename _type>
	decltype(auto) scalar_2_rawdata(_type* buff, const Scalar& scalar, int channels, int unroll_to = 0) {
		for (int i = 0; i < channels; ++i) {
			buff[i] = scalar[i];
		}
		(unroll_to > channels) | [&]() {
			for (int i = channels; i < unroll_to; ++i) {
				buff[i] = buff[i - channels];
			}
		};
	}

	decltype(auto) collect_polyedge(Mat& input, const Point* points, int count, 
		std::vector<poly_edge>& edges, const void* color, int shift, Point offset, line_type line) {
		int delta = offset.y + (shift ? (1 << (shift - 1)) : 0);
		Point pt0 = points[count - 1], pt1;
		pt0.x = (pt0.x + offset.x) << (shift_count - shift);
		pt0.y = (pt0.y + delta) >> shift;

		edges.reserve(edges.size() + count);

		for (int i = 0; i < count; ++i, pt0 = pt1) {
			Point t0, t1;
			poly_edge edge;

			pt1 = points[i];
			pt1.x = (pt1.x + offset.x) << (shift_count - shift);
			pt1.y = (pt1.y + delta) >> shift;

			if (line_type::anti_alias == line) {
				t0.y = pt0.y; t1.y = pt1.y;
				t0.x = (pt0.x + (shift_65536 >> 1)) >> shift_count;
				t1.x = (pt1.x + (shift_65536 >> 1)) >> shift_count;
				//Line(img, t0, t1, color, line_type);
			}
			else {
				t0.x = pt0.x; t1.x = pt1.x;
				t0.y = pt0.y << shift_count;
				t1.y = pt1.y << shift_count;
				//LineAA(img, t0, t1, color);
			}

			if (pt0.y == pt1.y) {
				continue;
			}

			if (pt0.y < pt1.y) {
				edge.y0 = pt0.y;
				edge.y1 = pt1.y;
				edge.x = pt0.x;
			}
			else {
				edge.y0 = pt1.y;
				edge.y1 = pt0.y;
				edge.x = pt1.x;
			}
			edge.dx = (pt1.x - pt0.x) / (pt1.y - pt0.y);
			edges.push_back(edge);
		}
	}

	decltype(auto) fill_polygon(Mat input, const Point* points, const std::vector<int> points_count, const int polygon_count, const Scalar& color, const int shift, const Point& offset, line_type line = line_type::normal) {
		float buff[4] = { 0 };
		std::vector<poly_edge> edges;
		scalar_2_rawdata((float*)buff, color, input.get_channels(), 0);
		edges.resize(std::accumulate(points_count.begin(), points_count.end(), 1));
		for (int i = 0; i < polygon_count; ++i) {

		}
	}
}