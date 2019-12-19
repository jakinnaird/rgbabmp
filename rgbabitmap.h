/*
	Simple, efficient RGBA bitmap library

	// Inspired by Arash Partow's bitmap_image.hpp
	// http://partow.net/programming/bitmap/index.html

	Copyright (c) 2019 James A. Kinnaird

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to 
	deal in the Software without restriction, including without limitation the 
	rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
	sell copies of the Software, and to permit persons to whom the Software is 
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in 
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
	FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
	IN THE SOFTWARE.
*/

#pragma once

#include <vector>

namespace rgbabitmap
{
	class bitmap
	{
	private:
		unsigned int m_width;
		unsigned int m_height;
		unsigned int m_row_increment;
		unsigned int m_bytes_per_pixel;
		std::vector<unsigned char> m_bitmap;

	public:
		bitmap(void) :
			m_width(0),
			m_height(0),
			m_bytes_per_pixel(4)	// RGBA
		{
		}

		bitmap(unsigned int width, unsigned int height) :
			m_width(width),
			m_height(height),
			m_bytes_per_pixel(4)	// RGBA
		{
			create_bitmap();
		}

		// load an RGBA bitmap
		bitmap(const unsigned char *pixels, unsigned int width,
			unsigned int height) :
			m_width(width),
			m_height(height),
			m_bytes_per_pixel(4)
		{
			create_bitmap();
			unsigned int pitch = m_width * m_bytes_per_pixel;
			unsigned char *data = this->data();
			memcpy(data, pixels, width * height * 4);
		}

		bitmap(const bitmap &that) :
			m_width(that.m_width),
			m_height(that.m_height),
			m_row_increment(0),
			m_bytes_per_pixel(4)	// RGBA
		{
			create_bitmap();
			m_bitmap = that.m_bitmap;
		}

		bitmap& operator= (const bitmap &that)
		{
			if (this != &that)
			{
				m_width = that.m_width;
				m_height = that.m_height;
				m_row_increment = 0;
				m_bytes_per_pixel = that.m_bytes_per_pixel;

				create_bitmap();
				m_bitmap = that.m_bitmap;
			}

			return *this;
		}

		inline void clear(const unsigned char value = 0)
		{
			//std::fill(m_bitmap.begin(), m_bitmap.end(), value);
			memset(m_bitmap.data(), value, m_bitmap.size());
		}

		inline void set_pixel(unsigned int x, unsigned int y,
			unsigned char red, unsigned char green, unsigned char blue,
			unsigned char alpha)
		{
			unsigned int y_offset = y * m_row_increment;
			unsigned int x_offset = x * m_bytes_per_pixel;

			unsigned char *pixel = m_bitmap.data() + (y_offset + x_offset);
			*(pixel + 0) = red;
			*(pixel + 1) = green;
			*(pixel + 2) = blue;
			*(pixel + 3) = alpha;
		}

		inline void get_pixel(unsigned int x, unsigned int y,
			unsigned char &red, unsigned char &green, unsigned char &blue,
			unsigned char &alpha) const
		{
			unsigned int y_offset = y * m_row_increment;
			unsigned int x_offset = x * m_bytes_per_pixel;

			const unsigned char *pixel = m_bitmap.data() + (y_offset + x_offset);
			red = *(pixel + 0);
			green = *(pixel + 1);
			blue = *(pixel + 2);
			alpha = *(pixel + 3);
		}

		inline void set_size(unsigned int width, unsigned int height,
			bool clear = false)
		{
			m_width = width;
			m_height = height;

			create_bitmap();
			if (clear)
				this->clear();
		}

		inline void set_alpha(unsigned char alpha)
		{
			// @TODO: Optimize this loop
			for (unsigned int y = 0; y < m_height; y++)
			{
				for (unsigned int x = 0; x < m_width; x++)
				{
					m_bitmap[y * m_row_increment + x * m_bytes_per_pixel + 3] = alpha;
				}
			}
		}

		inline unsigned int width(void) const
		{
			return m_width;
		}

		inline unsigned int height(void) const
		{
			return m_height;
		}

		inline unsigned int bytes_per_pixel(void) const
		{
			return m_bytes_per_pixel;
		}

		inline unsigned int pixel_count() const
		{
			return m_width * m_height;
		}

		inline const unsigned char* data(void) const
		{
			return m_bitmap.data();
		}

		inline unsigned char* data(void)
		{
			return const_cast<unsigned char*>(m_bitmap.data());
		}

	private:
		void create_bitmap(void)
		{
			m_row_increment = m_width * m_bytes_per_pixel;
			m_bitmap.resize(m_height * m_row_increment);
		}
	};

	class canvas
	{
	private:
		bitmap &m_bitmap;
		unsigned int m_pen_width;
		unsigned char m_pen_red;
		unsigned char m_pen_green;
		unsigned char m_pen_blue;
		unsigned char m_pen_alpha;

		unsigned int m_clip_minx;
		unsigned int m_clip_miny;
		unsigned int m_clip_maxx;
		unsigned int m_clip_maxy;
		bool m_clipped;

	public:
		canvas(bitmap &bitmap)
			: m_bitmap(bitmap),
			m_pen_width(1),
			m_pen_red(255),
			m_pen_green(255),
			m_pen_blue(255),
			m_pen_alpha(255),
			m_clip_minx(0),
			m_clip_miny(0),
			m_clip_maxx(bitmap.width()),
			m_clip_maxy(bitmap.height()),
			m_clipped(false)
		{
		}

		int width(void) const { return m_bitmap.width(); }
		int height(void) const { return m_bitmap.height(); }

		void setclip(int minx, int miny, int maxx, int maxy)
		{
			m_clip_minx = minx;
			m_clip_miny = miny;
			m_clip_maxx = maxx;
			m_clip_maxy = maxy;
		}

		void rectangle(int x1, int y1, int x2, int y2)
		{
			line_segment(x1, y1, x2, y1);
			line_segment(x2, y1, x2, y2);
			line_segment(x2, y2, x1, y2);
			line_segment(x1, y2, x1, y1);
		}

		void triangle(int x1, int y1, int x2, int y2, int x3, int y3)
		{
			line_segment(x1, y1, x2, y2);
			line_segment(x2, y2, x3, y3);
			line_segment(x3, y3, x1, y1);
		}

		void quadix(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
		{
			line_segment(x1, y1, x2, y2);
			line_segment(x2, y2, x3, y3);
			line_segment(x3, y3, x4, y4);
			line_segment(x4, y4, x1, y1);
		}

		void ellipse(int centerx, int centery, int a, int b)
		{
			int t1 = a * a;
			int t2 = t1 << 1;
			int t3 = t2 << 1;
			int t4 = b * b;
			int t5 = t4 << 1;
			int t6 = t5 << 1;
			int t7 = a * t5;
			int t8 = t7 << 1;
			int t9 = 0;

			int d1 = t2 - t7 + (t4 >> 1);
			int d2 = (t1 >> 1) - t8 + t5;
			int x = a;
			int y = 0;

			int negative_tx = centerx - x;
			int positive_tx = centerx + x;
			int negative_ty = centery - y;
			int positive_ty = centery + y;

			while (d2 < 0)
			{
				plot_pen_pixel(positive_tx, positive_ty);
				plot_pen_pixel(positive_tx, negative_ty);
				plot_pen_pixel(negative_tx, positive_ty);
				plot_pen_pixel(negative_tx, negative_ty);

				++y;

				t9 = t9 + t3;

				if (d1 < 0)
				{
					d1 = d1 + t9 + t2;
					d2 = d2 + t9;
				}
				else
				{
					x--;
					t8 = t8 - t6;
					d1 = d1 + (t9 + t2 - t8);
					d2 = d2 + (t9 + t5 - t8);
					negative_tx = centerx - x;
					positive_tx = centerx + x;
				}

				negative_ty = centery - y;
				positive_ty = centery + y;
			}

			do
			{
				plot_pen_pixel(positive_tx, positive_ty);
				plot_pen_pixel(positive_tx, negative_ty);
				plot_pen_pixel(negative_tx, positive_ty);
				plot_pen_pixel(negative_tx, negative_ty);

				x--;
				t8 = t8 - t6;

				if (d2 < 0)
				{
					++y;
					t9 = t9 + t3;
					d2 = d2 + (t9 + t5 - t8);
					negative_ty = centery - y;
					positive_ty = centery + y;
				}
				else
					d2 = d2 + (t5 - t8);

				negative_tx = centerx - x;
				positive_tx = centerx + x;
			} while (x >= 0);
		}

		void circle(int centerx, int centery, int radius)
		{
			int x = 0;
			int d = (1 - radius) << 1;

			while (radius >= 0)
			{
				plot_pen_pixel(centerx + x, centery + radius);
				plot_pen_pixel(centerx + x, centery - radius);
				plot_pen_pixel(centerx - x, centery + radius);
				plot_pen_pixel(centerx - x, centery - radius);

				if ((d + radius) > 0)
					d -= ((--radius) << 1) - 1;
				if (x > d)
					d += ((++x) << 1) + 1;
			}
		}

		void arc(int centerx, int centery, int startx, int starty, float angle)
		{
			// based on http://stackoverflow.com/a/8889666

			const int N = 130 * (angle / 1.57f);	// *magic*
			float dx = startx - centerx;
			float dy = starty - centery;
			float ctheta = cos(angle / (N - 1));
			float stheta = sin(angle / (N - 1));

			for (int i = 1; i != N; ++i)
			{
				float dxtemp = ctheta * dx - stheta * dy;
				dy = stheta * dx + ctheta * dy;
				dx = dxtemp;
				plot_pen_pixel(centerx + dx, centery + dy);
			}
		}

		void rounded_rectangle(int x1, int y1, int x2, int y2, int radius)
		{
			// draw the line segments
			line_segment(x1 + radius, y1, x2 - radius, y1);	// top
			line_segment(x2 - 1, y1 + radius, x2 - 1, y2 - radius);	// right
			line_segment(x1 + radius, y2 - 1, x2 - radius, y2 - 1);	// bottom
			line_segment(x1, y1 + radius, x1, y2 - radius);	// left

			// draw the arcs
			arc(x1 + radius, y1 + radius, x1, y1 + radius, 1.57f);	// top-left
			arc(x2 - radius, y1 + radius, x2 - radius, y1, 1.57f);	// top-right
			arc(x2 - radius, y2 - radius, x2, y2 - radius, 1.57f);	// bottom-right
			arc(x1 + radius, y2 - radius, x1 + radius, y2, 1.57f);	// bottom-left
		}

		void fill_rectangle(int x1, int y1, int x2, int y2)
		{
			for (int y = y1; y < y2; y++)
				line_segment(x1, y, x2, y);
		}

		void fill_circle(int centerx, int centery, int radius)
		{
			// based on http://stackoverflow.com/questions/1201200/fast-algorithm-for-drawing-filled-circles

			int r2 = radius * radius;
			float r2_delta = radius * 0.8f;
			for (int y = -radius; y <= radius; y++)
			{
				for (int x = -radius; x <= radius; x++)
				{
					if (x*x + y*y <= r2 + r2_delta)
						plot_pen_pixel(centerx + x, centery + y);
				}
			}
		}

		void fill_arc(int centerx, int centery, int startx, int starty, int angle)
		{
			// based on http://stackoverflow.com/a/8889666

			const int N = 130 * (angle / 1.57f);	// *magic*
			float dx = startx - centerx;
			float dy = starty - centery;
			float ctheta = cos(angle / (N - 1));
			float stheta = sin(angle / (N - 1));

			for (int i = 1; i != N; ++i)
			{
				float dxtemp = ctheta * dx - stheta * dy;
				dy = stheta * dx + ctheta * dy;
				dx = dxtemp;
				line_segment(centerx, centery, centerx + dx, centery + dy);
			}
		}

		void fill_rounded_rectangle(int x1, int y1, int x2, int y2, int radius)
		{
			// fill the rectangles
			fill_rectangle(x1 + radius, y1, x2 - radius, y1 + radius);		// top
			fill_rectangle(x1, y1 + radius, x2, y2 - radius);			// center
			fill_rectangle(x1 + radius, y2 - radius, x2 - radius, y2 + 1);		// bottom

			// draw the arcs
			fill_arc(x1 + radius, y1 + radius, x1, y1 + radius, 1.57f);	// top-left
			fill_arc(x2 - radius, y1 + radius, x2 - radius, y1, 1.57f);	// top-right
			fill_arc(x2 - radius, y2 - radius, x2, y2 - radius, 1.57f);	// bottom-right
			fill_arc(x1 + radius, y2 - radius, x1 + radius, y2, 1.57f);	// bottom-left
		}

		void blit(const bitmap &src, int srcx, int srcy, 
			int dstx, int dsty, int width, int height)
		{
			// clip the bounds as needed
			if (srcx + width > src.width())
				width = src.width() - srcx;
			else if (dstx + width > m_bitmap.width())
				width = m_bitmap.width() - dstx;
			if (srcy + height > src.height())
				height = src.height() - srcy;
			else if (dsty + height > m_bitmap.height())
				height = m_bitmap.height() - dsty;

			// save the old pen
			unsigned char old_pen_red = m_pen_red;
			unsigned char old_pen_green = m_pen_green;
			unsigned char old_pen_blue = m_pen_blue;
			unsigned char old_pen_alpha = m_pen_alpha;

			// blit
			for (int y = srcy; y < srcy + height; y++)
			{
				for (int x = srcx; x < srcx + width; x++)
				{
					src.get_pixel(x, y, m_pen_red, m_pen_green,
						m_pen_blue, m_pen_alpha);
					plot_pixel(dstx + x, dsty + y);
				}
			}

			// restore the original pen
			m_pen_red = old_pen_red;
			m_pen_green = old_pen_green;
			m_pen_blue = old_pen_blue;
			m_pen_alpha = old_pen_alpha;
		}

		void line_segment(int x1, int y1, int x2, int y2)
		{
			// do clipping
			if (!m_clipped && !(m_clipped = ClipLine(x1, y1, x2, y2)))
				return;

			int steep = 0;
			int sx = ((x2 - x1) > 0) ? 1 : -1;
			int sy = ((y2 - y1) > 0) ? 1 : -1;
			int dx = abs(x2 - x1);
			int dy = abs(y2 - y1);

			if (dy > dx)
			{
				std::swap(x1, y1);
				std::swap(dx, dy);
				std::swap(sx, sy);

				steep = 1;
			}

			int e = 2 * dy - dx;

			for (int i = 0; i < dx; ++i)
			{
				if (steep)
					plot_pen_pixel(y1, x1);
				else
					plot_pen_pixel(x1, y1);

				while (e >= 0)
				{
					y1 += sy;
					e -= (dx << 1);
				}

				x1 += sx;
				e += (dy << 1);
			}

			plot_pen_pixel(x2, y2);
			m_clipped = false;
		}

		void plot_pixel(unsigned int x, unsigned int y)
		{
			// do clipping if needed
			if (!m_clipped &&
				(x < m_clip_minx ||
				x > m_clip_maxx ||
				y < m_clip_miny ||
				y > m_clip_maxy))
				return;

			// grab the current color here
			unsigned char sr, sg, sb, sa;
			m_bitmap.get_pixel(x, y, sr, sg, sb, sa);

			// alpha blend
			// http://www.codeguru.com/cpp/cpp/algorithms/general/article.php/c15989/Tip-An-Optimized-Formula-for-Alpha-Blending-Pixels.htm
			unsigned char red, green, blue, alpha;
			red = ((m_pen_red * m_pen_alpha) + (sr * (255 - m_pen_alpha))) >> 8;
			green = ((m_pen_green * m_pen_alpha) + (sg * (255 - m_pen_alpha))) >> 8;
			blue = ((m_pen_blue * m_pen_alpha) + (sb * (255 - m_pen_alpha))) >> 8;
			alpha = m_pen_alpha;

			m_bitmap.set_pixel(x, y, red, green,
				blue, alpha);
		}

		void plot_pen_pixel(unsigned int x, unsigned int y)
		{
			switch (m_pen_width)
			{
			case 1:
				plot_pixel(x, y);
				break;
			case 2:
				plot_pixel(x, y);
				plot_pixel(x + 1, y);
				plot_pixel(x + 1, y + 1);
				plot_pixel(x, y + 1);
				break;
			case 3:
				plot_pixel(x, y - 1);
				plot_pixel(x - 1, y - 1);
				plot_pixel(x + 1, y - 1);

				plot_pixel(x, y);
				plot_pixel(x - 1, y);
				plot_pixel(x + 1, y);

				plot_pixel(x, y + 1);
				plot_pixel(x - 1, y + 1);
				plot_pixel(x + 1, y + 1);
				break;
			default:
				plot_pixel(x, y);
				break;
			}
		}

		void pen_width(const unsigned int &width)
		{
			if (width > 0 && width < 4)
				m_pen_width = width;
		}

		int pen_width(void) const { return m_pen_width; }

		void pen_color(const unsigned char &red,
			const unsigned char &green,
			const unsigned char &blue,
			const unsigned char &alpha)
		{
			m_pen_red = red;
			m_pen_green = green;
			m_pen_blue = blue;
			m_pen_alpha = alpha;
		}

	private:
		typedef int OutCode;
		
		const int INSIDE = 0;
		const int LEFT = 1;
		const int RIGHT = 2;
		const int BOTTOM = 4;
		const int TOP = 8;

		inline OutCode ComputeOutCode(int x, int y)
		{
			OutCode code = INSIDE;

			if (x < m_clip_minx)
				code |= LEFT;
			else if (x > m_clip_maxx)
				code |= RIGHT;

			if (y < m_clip_miny)
				code |= BOTTOM;
			else if (y > m_clip_maxy)
				code |= TOP;
			
			return code;
		}

		// if true, we draw the line using the updated coords
		bool ClipLine(int &x0, int &y0, int &x1, int &y1)
		{
			// Cohen-Sutherland clipping algorithm
			OutCode outcode0 = ComputeOutCode(x0, y0);
			OutCode outcode1 = ComputeOutCode(x1, y1);

			bool accept = false;
			while (true)
			{
				if (!(outcode0 | outcode1))
				{
					// trivial accept - all points inside
					accept = true;
					break;
				}
				else if (outcode0 & outcode1)
				{
					// trivial fail - all points outside
					break;
				}
				else
				{
					// compute adjusted line segment
					int x, y;

					// choose the correct outside point
					OutCode outcode = outcode0 ? outcode0 : outcode1;

					if (outcode & TOP)
					{
						x = x0 + (x1 - x0) * (m_clip_maxy - y0) / (float)(y1 - y0);
						y = m_clip_maxy;
					}
					else if (outcode & BOTTOM)
					{
						x = x0 + (x1 - x0) * (m_clip_miny - y0) / (float)(y1 - y0);
						y = m_clip_miny;
					}
					else if (outcode & RIGHT)
					{
						x = m_clip_maxx;
						y = y0 + (y1 - y0) * (m_clip_maxx - x0) / (float)(x1 - x0);
					}
					else if (outcode & LEFT)
					{
						x = m_clip_minx;
						y = y0 + (y1 - y0) * (m_clip_minx - x0) / (float)(x1 - x0);
					}

					// update the points
					if (outcode == outcode0)
					{
						x0 = x;
						y0 = y;
						outcode0 = ComputeOutCode(x0, y0);
					}
					else
					{
						x1 = x;
						y1 = y;
						outcode1 = ComputeOutCode(x1, y1);
					}
				}
			}

			return accept;
		}
	};
}
