/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef	WG_GEO_DOT_H
#define	WG_GEO_DOT_H
#pragma once
//=============================================================================

#include <wg_types.h>
#include <wg_mu.h>

#include <utility>
#include <cstdlib>

namespace wg
{
	template<class T> class CoordT;
	template<class T> class BorderT;
	template<class T> class SizeT;
	template<class T> class RectT;

	typedef	CoordT<int>		CoordI;
	typedef CoordT<float>	CoordF;

	typedef BorderT<int>	BorderI;
	typedef BorderT<float>	BorderF;

	typedef	SizeT<int>		SizeI;
	typedef SizeT<float>	SizeF;

	typedef	RectT<int>		RectI;
	typedef RectT<float>	RectF;

	class	Coord;
	class	Border;
	class   Size;
	class	Rect;

	//____ Coord ______________________________________________________________

	class Coord
	{
	public:
		//.____ Creation __________________________________________

		Coord() : x(0), y(0) {}
		Coord(MU x, MU y) : x(x), y(y) {}
		Coord(const Coord& coord) : x(coord.x), y(coord.y) {}
		Coord(const Rect& rect);

		template<typename T>
		explicit Coord(const CoordT<T>& r) : x((MU)r.x), y((MU)r.y) {}

		template<typename T>
		explicit Coord(const RectT<T>& r) : x((MU)r.x), y((MU)r.y) {}


		//.____ Misc ______________________________________________

		inline const Size& toSize() const;
		inline void clear() { x = 0; y = 0; }			///< Sets X and Y to 0.

		inline const CoordI&	qpix() const {	return reinterpret_cast<const CoordI&>(*this); }
		inline const CoordI		px() const;
		inline const CoordF		fpx() const;

		inline Coord aligned() const { Coord c; c.x.qpix = x.qpix & 0xFFFFFFFC; c.y.qpix = y.qpix & 0xFFFFFFFC; return c; }

		template<typename Type>
		inline static Coord fromPX(const CoordT<Type>& c) { Rect c2; c2.x.qpix = int(c.x*4); c2.y.qpix = int(c.y*4); return c2; }

		//.____ Operators ___________________________________________

		// Coord can be IMPLICITLY cast to all CoordT<>

		template<typename T>
		inline operator CoordT<T>() const { return CoordT<T>(x,y); }


		inline Coord operator=(const Coord& k) { x = k.x; y = k.y; return *this; }

		inline bool operator==(const Coord& k) const { if (x == k.x && y == k.y) return true; return false; }
		inline bool operator!=(const Coord& k) const { if (x != k.x || y != k.y) return true; return false; }

		inline Coord& operator+=(const Coord& k) { x += k.x; y += k.y; return *this; }
		inline Coord& operator-=(const Coord& k) { x -= k.x; y -= k.y; return *this; }
		inline Coord operator+(const Coord& k) const { return { x + k.x, y + k.y }; }
		inline Coord operator-(const Coord& k) const { return { x - k.x, y - k.y }; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline Coord& operator*=(Type2 v) { x = (MU)(x*v); y = (MU)(x*v); return *this; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline Coord& operator/=(Type2 v) { x = (MU)(x / v); y = (MU)(y / v); return *this; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline Coord operator*(Type2 v) const { return { (MU)(x*v), (MU)(y*v) }; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline Coord operator/(Type2 v) const { return { (MU)(x / v), (MU)(y / v) }; }


		//.____ Properties __________________________________________

		MU		x, y;
	};

	//____ Border _____________________________________________________________

	class Border
	{
	public:
	
		//.____ Creation __________________________________________

		Border() : top(0), right(0), bottom(0), left(0) {}
		Border(MU topBottom, MU leftRight) : top(topBottom), right(leftRight), bottom(topBottom), left(leftRight) {}
		Border(MU top, MU leftRight, MU bottom) : top(top), right(leftRight), bottom(bottom), left(leftRight) {}
		Border(MU top, MU right, MU bottom, MU left) : top(top), right(right), bottom(bottom), left(left) {}
		Border(MU all) : top(all), right(all), bottom(all), left(all) {}

		template<typename T>
		explicit Border(const BorderT<T>& b) : top((MU)b.top), right((MU)b.right), bottom((MU)b.bottom), left((MU)b.left) {}


		//.____ Misc ______________________________________________

		inline void		set(MU all) { left = right = top = bottom = all; }
		inline void		set(MU topBottom, MU leftRight) { top = bottom = topBottom; left = right = leftRight; }
		inline void		set(MU top, MU leftRight, MU bottom) { this->top = top; this->bottom = bottom; this->left = this->right = leftRight; }
		inline void		set(MU top, MU right, MU bottom, MU left) { this->top = top; this->bottom = bottom; this->left = left; this->right = right; }

		inline Size		size() const;
		inline MU		width() const { return left + right; }
		inline MU		height() const { return top + bottom; }
		inline void		clear() { left.qpix = 0; right.qpix = 0; top.qpix = 0; bottom.qpix = 0; }		///< @brief Sets the thickness of all sides to 0.
		inline bool		isEmpty() const { return (left.qpix | top.qpix | right.qpix | bottom.qpix) == 0; }

		inline Border	scale(int scale) const { return Border(top*scale / 4096, right*scale / 4096, bottom*scale / 4096, left*scale / 4096); } // Only for WG2 compatibility!

		inline const BorderI&	qpix() const { return reinterpret_cast<const BorderI&>(*this); }
		inline const BorderI	px() const;
		inline const BorderF	fpx() const;

		inline Border aligned() const { Border b;  b.top.qpix = top.qpix & 0xFFFFFFFC; b.right.qpix = right.qpix & 0xFFFFFFFC; b.bottom.qpix = bottom.qpix & 0xFFFFFFFC; b.left.qpix = left.qpix & 0xFFFFFFFC; return b; }

		template<typename Type>
		inline static Border fromPX(const BorderT<Type>& b) { Border b2; b2.top.qpix = int(b.top*4); b2.right.qpix = int(b.right*4); b2.bottom.qpix = int(b.bottom*4); b2.left.qpix = int(b.left*4); return b2; }


		//.____ Operators ___________________________________________

		inline operator Size() const;

		// Border can be IMPLICITLY cast to all BorderT<>

		template<typename T>
		inline operator BorderT<T>() const { return BorderT<T>(top, right, bottom, left); }

		//


		inline Border& operator+=(const Border& k) { top += k.top; right += k.right; bottom += k.bottom; left += k.left; return *this; }
		inline Border& operator-=(const Border& k) { top -= k.top; right -= k.right; bottom -= k.bottom; left -= k.left; return *this; }
		inline Border operator+(const Border& k) const { return { top + k.top, right + k.right, bottom + k.bottom, left + k.left }; }
		inline Border operator-(const Border& k) const { return { top - k.top, right - k.right, bottom - k.bottom, left - k.left }; }


		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline Border& operator*=(Type2 v) { top = (MU)(top*v); right = (MU)(right*v); bottom = (MU)(bottom*v); left = (MU)(left*v); return *this; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline Border& operator/=(Type2 v) { top = (MU)(top / v); right = (MU)(right / v); bottom = (MU)(bottom / v); left = (MU)(left / v); return *this; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline Border operator*(Type2 v) const { return { (MU)(top*v), (MU)(right*v), (MU)(bottom*v), (MU)(left*v) }; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline Border operator/(Type2 v) const { return { (MU)(top / v), (MU)(right / v), (MU)(bottom / v), (MU)(left / v) }; }


		bool			operator==(const Border& borders) const {
			return ((left - borders.left) | (right - borders.right) | (top - borders.top) | (bottom - borders.bottom)) == 0;
		}
		bool			operator!=(const Border& borders) const { return !(*this == borders); }

		//.____ Properties __________________________________________

		MU		top, right, bottom, left;
	};


	//____ Size ________________________________________________________

	/**
		@brief Specifies the size of a rectangular area.

		Specifies the size of a rectangular area.

		Internal resolution is quarterpixels.
	**/

	class Size
	{
	public:
		//.____ Creation __________________________________________

		Size() : w(0), h(0) {}
		Size(MU width, MU height) : w(width), h(height) {}
		Size(const Size& size) : w(size.w), h(size.h) {}
		inline Size(const Rect& rect);
		Size(const Coord& c1, const Coord& c2) { w = c2.x - c1.x; h = c2.y - c1.y; }

		// These explicit constructors enables us to cast from RectI and SizeI of different kind.

		template<typename T>
		explicit Size(const SizeT<T>& r) : w((MU)r.w), h((MU)r.h) {}

		template<typename T>
		explicit Size(const RectT<T>& r) : w((MU)r.w), h((MU)r.h) {}


		//.____ Misc ______________________________________________

		inline void			limit(const Size& min, const Size& max);
		inline void			clear() { w = 0; h = 0; }
		inline bool			isEmpty() const { return (w == 0 && h == 0); }

		inline const Coord& toCoord() const { return reinterpret_cast<const Coord&>(*this); }

		inline const SizeI&	qpix() const { return reinterpret_cast<const SizeI&>(*this); }
		inline const SizeI	px() const;
		inline const SizeF	fpx() const;
		inline Size	aligned() const { Size sz2;  sz2.w.qpix = w.qpix & 0xFFFFFFFC; sz2.h.qpix = h.qpix & 0xFFFFFFFC; return sz2; }

		template<typename Type>
		inline static Size fromPX(const SizeT<Type>& sz) { Size sz2; sz2.w.qpix = int(sz.w*4); sz2.h.qpix = int(sz.h*4); return sz2; }

		static inline Size min(Size sz1, Size sz2) { return Size(sz1.w < sz2.w ? sz1.w : sz2.w, sz1.h < sz2.h ? sz1.h : sz2.h); }
		static inline Size max(Size sz1, Size sz2) { return Size(sz1.w > sz2.w ? sz1.w : sz2.w, sz1.h > sz2.h ? sz1.h : sz2.h); }

	
		//.____ Operators ___________________________________________

		// Size can be IMPLICITLY cast to all SizeT<> and RectT<>

		template<typename T>
		inline operator SizeT<T>() const { return SizeT<T>(w, h); }

		template<typename T>
		inline operator RectT<T>() const { return RectT<T>(0, 0, w, h); }

		//

		inline Size operator=(const Size& k) { w = k.w; h = k.h; return *this; }

		inline bool operator==(const Size& k) const { if (w == k.w && h == k.h) return true; return false; }
		inline bool operator!=(const Size& k) const { if (w != k.w || h != k.h) return true; return false; }

		inline Size& operator+=(const Size& k) { w += k.w; h += k.h; return *this; }
		inline Size& operator-=(const Size& k) { w -= k.w; h -= k.h; return *this; }
		inline Size operator+(const Size& k) const { return { w + k.w, h + k.h }; }
		inline Size operator-(const Size& k) const { return { w - k.w, h - k.h }; }

		inline Size& operator+=(const Border& k) { w += k.left + k.right; h += k.top + k.bottom; return *this; }
		inline Size& operator-=(const Border& k) { w -= k.left + k.right; h -= k.top + k.bottom; return *this; }
		inline Size operator+(const Border& k) const { return { w + k.left + k.right, h + k.top + k.bottom }; }
		inline Size operator-(const Border& k) const { return { w - k.left - k.right, h - k.top - k.bottom }; }


		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline Size& operator*=(Type2 x) { w = (MU)(w*x); h = (MU)(h*x); return *this; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline Size& operator/=(Type2 x) { w = (MU)(w / x); h = (MU)(h / x); return *this; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline Size operator*(Type2 x) const { return { (MU)(w*x), (MU)(h*x) }; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline Size operator/(Type2 x) const { return { (MU)(w / x), (MU)(h / x) }; }

		//.____ Properties __________________________________________

		MU		w;		///< Width of the rectangular area in pixels.
		MU		h;		///< Height of the rectangular area in pixels.

	};



	//____ Rect ___________________________________________________________

	class Rect
	{
	public:
		//.____ Creation __________________________________________

		Rect() : x(0), y(0), w(0), h(0) {}									///< @brief Create rectangle with x, y, w and h set to 0.
																			///<
																			///< Create rectangle with x, y, w and h set to 0.
		Rect(MU x, MU y, MU w, MU h) : x(x), y(y), w(w), h(h) {}			///< @brief Create rectangle with the given values.
																			///<
																			///< Create rectangle with the given values.
		Rect(const Rect& r) : x(r.x), y(r.y), w(r.w), h(r.h) {}				///< @brief Create a copy of specified rectangle.
																			///<
																			///< Create a copy of specified rectangle.



		Rect(const Rect& r1, const Rect& r2);						///< @brief Create rectangle from intersection of specified rectangles.
		Rect(const Coord& p1, const Coord& p2);						///< @brief	Create rectangle to cover the area between the specified coordinates.
		Rect(const Coord& p, const Size& sz) : x(p.x), y(p.y), w(sz.w), h(sz.h) {} 	///< @brief Create rectangle of specified position and size.
																							///<
																							///< Create rectangle of specified position and size.
																							///< @param p 	Coordinate containing position for rectangle.
																							///< @param sz	Coordinate containing size for rectangle.
		Rect(const Coord& p, MU w, MU h) : x(p.x), y(p.y), w(w), h(h) {}			///< @brief Create rectangle of specified position and size.
																							///<
																							///< Create rectangle of specified position and size.
																							///< @param p		Coordinate containing position for rectangle.
																							///< @param w		Width for rectangle.
																							///< @param h		Height for rectangle.
		Rect(MU x, MU y, const Size& sz) : x(x), y(y), w(sz.w), h(sz.h) {}			///< @brief Create rectangle of specified position and size.
																							///<
																							///< Create rectangle of specified position and size.
																							///< @param x		Horizontal position for rectangle.
																							///< @param y		Vertical position for rectangle.
																							///< @param sz	Width and height for rectangle.
		Rect(const Coord& p) : x(p.x), y(p.y), w(0), h(0) {}			///< @brief Create rectangle of specified position and size of (0,0).
																			///<
																			///< Create rectangle of specified position and size of (0,0).
																			///< @param p		Coordinate containing position for rectangle.
		Rect(const Size& sz) : x(0), y(0), w(sz.w), h(sz.h) {}		///< @brief Create rectangle of specified size and position (0,0).
																			///<
																			///< Create rectangle of specified size and position (0,0).
																			///< @param sz	Width and height for rectangle.

		// All versions of Rect, CoordI and SizeI can be cast to any RectI EXPLICITLY.

		template<typename T>
		explicit Rect(const RectT<T>& r) : x((MU)r.x), y((MU)r.y), w((MU)r.w), h((MU)r.h) {}

		template<typename T>
		explicit Rect(const CoordT<T>& p) : x((MU)p.x), y((MU)p.y), w(0), h(0) {}

		template<typename T>
		explicit Rect(const SizeT<T>& sz) : x(0), y(0), w((MU)sz.w), h((MU)sz.h) {}


		//.____ Misc ______________________________________________

		inline void setPos(const Coord& pos);							///< @brief Set position of rectangle.
		inline void setSize(const Size& size);					///< @brief Set size of rectangle.

		void grow(const Border& borders);
		void shrink(const Border& borders);

		bool intersection(const Rect& r1, const Rect& r2);
		static Rect getUnion(const Rect& r1, const Rect& r2);

		void growToContain(const Rect& rect);
		void growToContain(const Coord& coord);

		bool	contains(const Coord& coord) const;		///< @brief Check if coordinate is within rectangle.
		bool	contains(const Rect& rect) const;			///< @brief Check if rectangle is fully within our rectangle.

		Coord limit(const Coord& coord) const;			///< @brief Limit given coordinate to stay within rectangle.
		inline Coord center() const { return { x + w / 2,y + h / 2 }; }
		inline Rect center(Size sz) const { return { x + (w - sz.w) / 2,y + (h - sz.h) / 2, sz }; }

		bool intersectsWith(const Rect& rect) const;	///< @brief Check for intersection (partial or full overlap) with specified rectangle.

		bool		intersectsWithOrContains(Coord p1, Coord p2, int precision = 14) const;
		bool		clipLine(Coord * p1, Coord * p2, int precision = 14) const;


		inline MU	width() const;								///< @brief Get the width of the rectangle.
		inline MU	height() const;								///< @brief Get the height of the rectangle.
		inline Size size() const;								///< @brief Get the size of the rectangle.
		inline Coord pos() const;								///< @brief Get the position of the rectangle.

		inline MU left() const;									///< @brief Get X coordinate of left border.
		inline MU top() const;									///< @brief Get Y cordinate of top border.
		inline MU right() const;								///< @brief Get X coordinate of right border.
		inline MU bottom() const;								///< @brief Get Y coordinate of bottom border.

		inline bool	isEmpty() const;							///< @brief Check if rectangle has no area.
		inline void clear();									///< @brief Sets all values to zero.

		inline Coord distance(Coord coord) const;				////< @brief Get distance (signed) between coordinate and rectangle. 0 if inside.

		inline const RectI&	qpix() const { return reinterpret_cast<const RectI&>(*this); }
		inline const RectI	px() const;
		inline const RectF	fpx() const;

		inline Rect		aligned() const { Rect r2;  r2.x.qpix = x.qpix & 0xFFFFFFFC; r2.y.qpix = y.qpix & 0xFFFFFFFC; r2.w.qpix = (w.qpix + (x.qpix & 0x3)) & 0xFFFFFFFC; r2.h.qpix = (h.qpix + (y.qpix & 0x3)) & 0xFFFFFFFC; return r2; }

		template<typename Type>
		inline static Rect fromPX(const RectT<Type>& r) { Rect r2; r2.x.qpix = int(r.x*4); r2.y.qpix = int(r.y*4); r2.w.qpix = int(r.w*4); r2.h.qpix = int(r.h*4); return r2; }

		//.____ Operators ___________________________________________

		// Rect can be IMPLICITLY cast to all RectT<>, CoordT<> and SizeT<>

		template<typename T>
		inline operator RectT<T>() const { return RectT<T>(x, y, w, h); }

		template<typename T>
		inline operator CoordT<T>() const { return CoordT<T>(x, y); }

		template<typename T>
		inline operator SizeT<T>() const { return SizeT<T>(w, h); }

		//

		inline Rect& operator=(const Rect& rect);				///< @brief Normal assignment operator.
		inline bool operator==(const Rect& rect) const;
		inline bool operator!=(const Rect& rect) const;

		inline Rect& operator+=(const Size& k);					///< @brief Increase size of rectangle.
		inline Rect& operator-=(const Size& k);					///< @brief Decrease size of rectangle.
		inline Rect operator+(const Size& k) const; 			///< @brief Returns rectangle with size increased by Size.
		inline Rect operator-(const Size& k) const; 			///< @brief Returns rectangle with size decreased by Size.

		inline Rect& operator+=(const Coord& k); 				///< @brief Increase position of rectangle.
		inline Rect& operator-=(const Coord& k);				///< @brief Decrease position of rectangle.
		inline Rect operator+(const Coord& k) const; 			///< @brief Returns rectangle with position increased by Coord.
		inline Rect operator-(const Coord& k) const; 			///< @brief Returns rectangle with position decreased by Coord.

		inline Rect& operator+=(const Border& k);				///< @brief Grow rectangle by the specified border.
		inline Rect& operator-=(const Border& k);
		inline Rect operator+(const Border& k) const;
		inline Rect operator-(const Border& k) const;

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline Rect& operator*=(Type2 v) { x *= v; y *= v; w *= v; h *= v; return *this; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline Rect& operator/=(Type2 v) { x /= v; y /= v; w /= v; h /= v; return *this; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline Rect operator*(Type2 v) const { return { x * v, y * v, w * v, h * v }; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline Rect operator/(Type2 v) const { return { x / v, y / v, w / v, h / v }; }


		//.____ Properties __________________________________________

		MU		x;		///< Start position along the X axis.
		MU		y;		///< Start position along the Y axis.
		MU		w;		///< Width, i.e. length of rectangle along the X axis.
		MU		h;		///< Height, i.e. length of rectangle along the Y axis.
	};

	//____ CoordT<> ___________________________________________________________
	/**
		@brief Class template for coordinate classes containing X and Y values.

		Class template for coordinate classes containing X and Y values.

		Specializations include:

		- __Coord__		- coordinate with MU values
		- __CoordI__	- coordinate with integer values
		- __CoordF__	- coordinate with float values

		A Coord always holds a coordinate in WonderGUI's internal MU format, allowing it to be converted to points
		or pixels without losing precision, while CoordI and CoordF just holds a coordinate value which could be either points or pixels.

		Casting between Coord and any of the other two is done implicitly and as points. So if a CoordI is assigned the value
		of a Coord it receives the coordinate in points (possibly truncated). Likewise will an asignment
		from a CoordI to a Coord be treated as an assignment of points.

		Casting between CoordI and CoordF can be done explicitly.

		For a better explanation of the WonderGUI coordinate system and how MU, points and pixels relate to each other, you should
		read the coordinate system primer of the tutorial.

	**/
	template<class Type> class CoordT
	{
	public:
		//.____ Creation __________________________________________

		CoordT() : x(0), y(0) {}
		CoordT(Type x, Type y) : x(x), y(y) {}
		CoordT(const CoordT<Type>& coord) : x(coord.x), y(coord.y) {}
		CoordT(const RectT<Type>& rect);

		template<typename T>
		explicit CoordT(const CoordT<T>& r) : x((Type)r.x), y((Type)r.y) {}

		template<typename T>
		explicit CoordT(const RectT<T>& r) : x((Type)r.x), y((Type)r.y) {}


		//.____ Misc ______________________________________________

		inline SizeT<Type> toSize();
		inline void clear() { x = 0; y = 0; }			///< Sets X and Y to 0.

		//.____ Operators ___________________________________________

		inline operator Coord() const { return Coord(x, y); }
		inline operator Rect() const;

		//

		inline CoordT<Type> operator=(const CoordT<Type>& k) { x = k.x; y = k.y; return *this; }

		inline bool operator==(const CoordT<Type>& k) const { if (x == k.x && y == k.y) return true; return false; }
		inline bool operator!=(const CoordT<Type>& k) const { if (x != k.x || y != k.y) return true; return false; }

		inline CoordT<Type>& operator+=(const CoordT<Type>& k) { x += k.x; y += k.y; return *this; }
		inline CoordT<Type>& operator-=(const CoordT<Type>& k) { x -= k.x; y -= k.y; return *this; }
		inline CoordT<Type> operator+(const CoordT<Type>& k) const { return { x + k.x, y + k.y }; }
		inline CoordT<Type> operator-(const CoordT<Type>& k) const { return { x - k.x, y - k.y }; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline CoordT<Type>& operator*=(Type2 v) { x = (Type)(x*v); y = (Type)(x*v); return *this; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline CoordT<Type>& operator/=(Type2 v) { x = (Type)(x / v); y = (Type)(y / v); return *this; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline CoordT<Type> operator*(Type2 v) const { return { (Type)(x*v), (Type)(y*v) }; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline CoordT<Type> operator/(Type2 v) const { return { (Type)(x / v), (Type)(y / v) }; }


		//.____ Properties __________________________________________

		Type	x, y;
	};

	//_____________________________________________________________________________
	template<typename Type>
	inline CoordT<Type>::CoordT(const RectT<Type>& rect)
	{
		x = rect.x;
		y = rect.y;
	}

	//_____________________________________________________________________________
	template<typename Type>
	inline SizeT<Type> CoordT<Type>::toSize()
	{
		return SizeT<Type>(x, y);
	}

	//____ BorderI ______________________________________________________________
	/**
		@brief Specifies the thickness of the border of a rectangular area, measured in pixels.

		Specifies the thickness of the border of a rectangular area, measured in pixels.

		The thickness of the four sides of the border can be set individually.
		The sides of the border always comes in the order: top, right, bottom, left. I.e.
		you start with the top border and go clockwise.

		Borders are in most cases internal to the rectangular areas geometry, so the combined thickness of left and right
		sides should not surpass the rectangles width and the combined thickness of top and bottom sides should not surpass
		the rectangles height.

		Negative values are in most cases not accepted and result in unspecified behavior.
	**/

	template<class Type> class BorderT
	{
	public:
		//.____ Creation __________________________________________

		BorderT() : top(0), right(0), bottom(0), left(0) {}
		BorderT( Type topBottom, Type leftRight ) : top(topBottom), right(leftRight), bottom(topBottom), left(leftRight) {}
		BorderT( Type top, Type leftRight, Type bottom ) : top(top), right(leftRight), bottom(bottom), left(leftRight) {}
		BorderT( Type top, Type right, Type bottom, Type left ) : top(top), right(right), bottom(bottom), left(left) {}
		BorderT( Type all ) : top(all), right(all), bottom(all), left(all) {}

		template<typename T>
		explicit BorderT(const BorderT<T>& b) : top((Type)b.top), right((Type)b.right), bottom((Type)b.bottom), left((Type)b.left) {}


		//.____ Misc ______________________________________________

		inline void		set( Type all ) { left = right = top = bottom = all; }
		inline void		set( Type topBottom, Type leftRight ) { top = bottom = topBottom; left = right = leftRight; }
		inline void		set( Type top, Type leftRight, Type bottom ) { this->top = top; this->bottom = bottom; this->left = this->right = leftRight; }
		inline void		set( Type top, Type right, Type bottom, Type left ) { this->top = top; this->bottom = bottom; this->left = left; this->right = right; }

		inline SizeT<Type>	size() const;
		inline Type		width() const { return left+right; }
		inline Type		height() const { return top+bottom; }
		inline void		clear()			{ left = 0; right = 0; top = 0; bottom = 0; }		///< @brief Sets the thickness of all sides to 0.
		inline bool		isEmpty() const { return (left + top + right + bottom) == 0; }

		inline BorderT<Type>	scale(int scale) const { return BorderT<Type>( top*scale / 4096,right*scale / 4096,bottom*scale / 4096,left*scale / 4096); } // Only for WG2 compatibility!

		//.____ Operators ___________________________________________

		inline operator SizeT<Type>() const { return SizeT<Type>(left + right, top + bottom); }

		// All arithmetic versions of BorderI can be IMPLICITLY cast to Border

//		template<typename TP = Type, typename std::enable_if< std::is_arithmetic<TP>::value, int >::type = 0>
		inline operator Border() const { return Border(top, right, bottom, left); }

		// Border can be IMPLICITLY cast to all BorderT<>

		template<typename T, typename TP = Type, typename std::enable_if< std::is_class<TP>::value, int>::type = 0>
		inline operator BorderT<T>() const { return BorderT<T>(top, right, bottom, left); }

		//


		inline BorderT<Type>& operator+=(const BorderT<Type>& k)        { top += k.top; right += k.right; bottom += k.bottom; left += k.left; return *this; }
		inline BorderT<Type>& operator-=(const BorderT<Type>& k)        { top -= k.top; right -= k.right; bottom -= k.bottom; left -= k.left; return *this;}
		inline BorderT<Type> operator+(const BorderT<Type>& k) const    { return {top+k.top, right+k.right, bottom+k.bottom, left+k.left}; }
		inline BorderT<Type> operator-(const BorderT<Type>& k) const    { return {top-k.top, right-k.right, bottom-k.bottom, left-k.left}; }


		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline BorderT<Type>& operator*=(Type2 v) { top = (Type)(top*v); right = (Type)(right*v); bottom = (Type)(bottom*v); left = (Type)(left*v); return *this; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline BorderT<Type>& operator/=(Type2 v) { top = (Type)(top/v); right = (Type)(right/v); bottom = (Type)(bottom/v); left = (Type)(left/v); return *this; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline BorderT<Type> operator*(Type2 v) const { return { (Type)(top*v), (Type)(right*v), (Type)(bottom*v), (Type)(left*v) }; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline BorderT<Type> operator/(Type2 v) const { return { (Type)(top/v), (Type)(right/v), (Type)(bottom/v), (Type)(left/v) }; }


		bool			operator==(const BorderT<Type>& borders) const { return left == borders.left &&
																			right == borders.right &&
																			top == borders.top &&
																			bottom == borders.bottom; }
		bool			operator!=(const BorderT<Type>& borders) const { return !(*this == borders); }

		//.____ Properties __________________________________________

		Type		top, right, bottom, left;
	};


	//____ SizeT<T> ________________________________________________________

	/**
		@brief Specifies the size of a rectangular area, measured in pixels.

		Specifies the size of a rectangular area, measured in pixels.
		Members are integer precision and can be negative.
	**/

	template<class Type> class SizeT
	{
	public:
		//.____ Creation __________________________________________

		SizeT() : w(0), h(0) {}
		SizeT( Type width, Type height ) : w(width), h(height) {}
		SizeT( const SizeT<Type>& size ) : w(size.w), h(size.h) {}
		SizeT( const RectT<Type>& rect ) : w(rect.w), h(rect.h) {}
		SizeT( const CoordT<Type>& c1, const CoordT<Type>& c2 ) { w = c2.x - c1.x; h = c2.y - c1.y; }

		// These explicit constructors enables us to cast from RectI and SizeI of different kind.

		template<typename T>
		explicit SizeT(const SizeT<T>& r) : w((Type)r.w), h((Type)r.h) {}

		template<typename T>
		explicit SizeT(const RectT<T>& r) : w((Type)r.w), h((Type)r.h) {}


		//.____ Misc ______________________________________________

		inline void limit( const SizeT<Type>& min, const SizeT<Type>& max );
		inline void clear()		{ w = 0; h = 0; }
		inline bool	isEmpty() const { return (w == 0 && h == 0); }

		inline CoordT<Type> toCoord() { return CoordT<Type>(w,h); }

		//.____ Operators ___________________________________________

		// All arithmetic versions of SizeT can be IMPLICITLY cast to Size and Rect

//		template<typename TP = Type, typename std::enable_if< std::is_arithmetic<TP>::value, int>::type = 0>
		inline operator Size() const { return Size(w, h); }

//		template<typename TP = Type, typename std::enable_if< std::is_arithmetic<TP>::value, int>::type = 0>
		inline operator Rect() const;


		// Size can be IMPLICITLY cast to all SizeT<> and RectT<>

		template<typename T, typename TP = Type, typename std::enable_if< !std::is_arithmetic<TP>::value, int>::type = 0>
		inline operator SizeT<T>() const { return SizeT<T>(w, h); }

		template<typename T, typename TP = Type, typename std::enable_if< !std::is_arithmetic<TP>::value, int>::type = 0>
		inline operator RectT<T>() const { return RectT<T>(0, 0, w, h); }

		//

		inline SizeT<Type> operator=(const SizeT<Type>& k)	{ w = k.w; h = k.h; return *this; }

		inline bool operator==(const SizeT<Type>& k) const	{ if( w == k.w && h == k.h ) return true; return false; }
		inline bool operator!=(const SizeT<Type>& k) const	{ if( w != k.w || h != k.h ) return true; return false; }

		inline SizeT<Type>& operator+=(const SizeT<Type>& k)		{ w += k.w; h += k.h; return *this;}
		inline SizeT<Type>& operator-=(const SizeT<Type>& k)		{ w -= k.w; h -= k.h; return *this;}
		inline SizeT<Type> operator+(const SizeT<Type>& k) const	{ return { w + k.w, h + k.h }; }
		inline SizeT<Type> operator-(const SizeT<Type>& k) const	{ return { w - k.w, h - k.h }; }

		inline SizeT<Type>& operator+=(const BorderT<Type>& k)		{ w += k.left + k.right; h += k.top + k.bottom; return *this; }
		inline SizeT<Type>& operator-=(const BorderT<Type>& k)		{ w -= k.left + k.right; h -= k.top + k.bottom; return *this;}
		inline SizeT<Type> operator+(const BorderT<Type>& k) const	{ return { w + k.left + k.right, h + k.top + k.bottom }; }
		inline SizeT<Type> operator-(const BorderT<Type>& k) const	{ return { w - k.left - k.right, h - k.top - k.bottom }; }


		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline SizeT<Type>& operator*=(Type2 x)		{ w = (Type)(w*x); h = (Type)(h*x); return *this; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline SizeT<Type>& operator/=(Type2 x)		{ w = (Type)(w/x); h = (Type)(h/x); return *this; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline SizeT<Type> operator*(Type2 x) const	{ return { (Type)(w*x), (Type)(h*x) }; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline SizeT<Type> operator/(Type2 x) const	{ return { (Type)(w/x), (Type)(h/x) }; }


		static inline SizeT<Type> min( SizeT<Type> sz1, SizeT<Type> sz2 ) { return SizeT<Type>( sz1.w<sz2.w?sz1.w:sz2.w, sz1.h<sz2.h?sz1.h:sz2.h ); }
		static inline SizeT<Type> max( SizeT<Type> sz1, SizeT<Type> sz2 ) { return SizeT<Type>( sz1.w>sz2.w?sz1.w:sz2.w, sz1.h>sz2.h?sz1.h:sz2.h ); }

		//.____ Properties __________________________________________

		Type	w;		///< Width of the rectangular area in pixels.
		Type	h;		///< Height of the rectangular area in pixels.

	};

	//_____________________________________________________________________________
	template<typename Type>
	inline void SizeT<Type>::limit(const SizeT<Type>& min, const SizeT<Type>& max)
	{
		if (w < min.w)
			w = min.w;
		if (h < min.h)
			h = min.h;

		if (w > max.w)
			w = max.w;
		if (h > max.h)
			h = max.h;
	}




	//____ Rect<T> ___________________________________________________________

	/**
		@brief Specifies a rectangular area of a 2D surface through position and size.

		Specifies a rectangular area of a 2D surface through position and size.

		Position is specified through the members x and y, containing the horizontal
		and vertical offset from an origo position while members w and h specifies
		the width and height.

		Origo is normally the top-left corner of a parent rectangle, screen surface or
		bitmap and position normally specifies the position of the top-left corner of the rectangle, although
		this might differ depending on context.

		Increasing x and y will always move the rectangle to the right and down respectively.

		Members are not checked or truncated so width and height are allowed to contain negative values
		but various WonderGUI methods accepting a Rect as parameter might not be able to handle it.

	**/

	template<class Type> class RectT
	{
	public:
		//.____ Creation __________________________________________

		RectT() : x(0), y(0), w(0), h(0) {}								///< @brief Create rectangle with x, y, w and h set to 0.
																			///<
																			///< Create rectangle with x, y, w and h set to 0.
		RectT( Type x, Type y, Type w, Type h ) : x(x), y(y), w(w), h(h) {}	///< @brief Create rectangle with the given values.
																			///<
																			///< Create rectangle with the given values.
		RectT( const RectT<Type>& r ) : x(r.x), y(r.y), w(r.w), h(r.h) {}		///< @brief Create a copy of specified rectangle.
																			///<
																			///< Create a copy of specified rectangle.



		RectT( const RectT<Type>& r1, const RectT<Type>& r2 );						///< @brief Create rectangle from intersection of specified rectangles.
		RectT( const CoordT<Type>& p1, const CoordT<Type>& p2 );						///< @brief	Create rectangle to cover the area between the specified coordinates.
		RectT( const CoordT<Type>& p, const SizeT<Type>& sz ) : x(p.x), y(p.y), w(sz.w), h(sz.h) {} 	///< @brief Create rectangle of specified position and size.
																							///<
																							///< Create rectangle of specified position and size.
																							///< @param p 	Coordinate containing position for rectangle.
																							///< @param sz	Coordinate containing size for rectangle.
		RectT( const CoordT<Type>& p, Type w, Type h ) : x(p.x), y(p.y), w(w), h(h) {}			///< @brief Create rectangle of specified position and size.
																							///<
																							///< Create rectangle of specified position and size.
																							///< @param p		Coordinate containing position for rectangle.
																							///< @param w		Width for rectangle.
																							///< @param h		Height for rectangle.
		RectT( Type x, Type y, const SizeT<Type>& sz ) : x(x), y(y), w(sz.w), h(sz.h) {}			///< @brief Create rectangle of specified position and size.
																							///<
																							///< Create rectangle of specified position and size.
																							///< @param x		Horizontal position for rectangle.
																							///< @param y		Vertical position for rectangle.
																							///< @param sz	Width and height for rectangle.
		RectT( const CoordT<Type>& p ) : x(p.x), y(p.y), w(0), h(0) {}			///< @brief Create rectangle of specified position and size of (0,0).
																			///<
																			///< Create rectangle of specified position and size of (0,0).
																			///< @param p		Coordinate containing position for rectangle.
		RectT( const SizeT<Type>& sz ) : x(0), y(0), w(sz.w), h(sz.h) {}		///< @brief Create rectangle of specified size and position (0,0).
																			///<
																			///< Create rectangle of specified size and position (0,0).
																			///< @param sz	Width and height for rectangle.

		// All versions of Rect, CoordI and SizeI can be cast to any RectI EXPLICITLY.

		template<typename T>
		explicit RectT(const RectT<T>& r) : x((Type)r.x), y((Type)r.y), w((Type)r.w), h((Type)r.h) {}

		template<typename T>
		explicit RectT(const CoordT<T>& p) : x((Type)p.x), y((Type)p.y), w(0), h(0) {}

		template<typename T>
		explicit RectT(const SizeT<T>& sz) : x(0), y(0), w((Type)sz.w), h((Type)sz.h) {}


		//.____ Misc ______________________________________________

		inline void setPos( const CoordT<Type>& p );					///< @brief Set position of rectangle.
		inline void setSize( const SizeT<Type>& sz );					///< @brief Set size of rectangle.

		void grow( const BorderT<Type> &borders );
		void shrink( const BorderT<Type> &borders );

		bool intersection( const RectT<Type>& r1, const RectT<Type>& r2 );
		static RectT<Type> getUnion( const RectT<Type>& r1, const RectT<Type>& r2 );

		void growToContain( const RectT<Type>& rect );
		void growToContain( const CoordT<Type>& coord );

		inline bool	contains( const CoordT<Type>& coord ) const;		///< @brief Check if coordinate is within rectangle.
		inline bool contains( const RectT<Type>& rect ) const;			///< @brief Check if rectangle is fully within our rectangle.

		inline CoordT<Type> limit( const CoordT<Type>& coord ) const;			///< @brief Limit given coordinate to stay within rectangle.
		inline CoordT<Type> center() const { return {x+w/2,y+h/2}; }
		inline RectT<Type> center(SizeT<Type> sz) const { return { x + (w-sz.w) / 2,y + (h-sz.h) / 2, sz }; }

		inline bool intersectsWith( const RectT<Type>& rect ) const;	///< @brief Check for intersection (partial or full overlap) with specified rectangle.

		bool		intersectsWithOrContains(CoordT<Type> p1, CoordT<Type> p2, int precision = 14) const;
		bool		clipLine(CoordT<Type> * p1, CoordT<Type> * p2, int precision = 14) const;


		inline Type width() const;								///< @brief Get the width of the rectangle.
		inline Type height() const;								///< @brief Get the height of the rectangle.
		inline SizeT<Type> size() const;								///< @brief Get the size of the rectangle.
		inline CoordT<Type> pos() const;								///< @brief Get the position of the rectangle.

		inline Type left() const;								///< @brief Get X coordinate of left border.
		inline Type top() const;									///< @brief Get Y cordinate of top border.
		inline Type right() const;								///< @brief Get X coordinate of right border.
		inline Type bottom() const;								///< @brief Get Y coordinate of bottom border.

		inline bool	isEmpty() const;							///< @brief Check if rectangle has no area.
		inline void clear();									///< @brief Sets all values to zero.

		inline CoordT<Type> distance( CoordT<Type> coord ) const;   ////< @brief Get distance (signed) between coordinate and rectangle. 0 if inside.

		//.____ Operators ___________________________________________

		// All arithmetic versions of RectI can be IMPLICITLY cast to Rect, Coord and Size

//		template<typename TP = Type, typename std::enable_if< std::is_arithmetic<TP>::value, int>::type = 0>
		inline operator Rect() const;

//		template<typename TP = Type, typename std::enable_if< std::is_arithmetic<TP>::value, int>::type = 0>
		inline operator Coord() const { return Coord(x, y); }

//		template<typename TP = Type, typename std::enable_if< std::is_arithmetic<TP>::value, int>::type = 0>
		inline operator Size() const { return Size(w, h); }

		// Rect can be IMPLICITLY cast to all RectT<>, CoordT<> and SizeT<>

		template<typename T, typename TP = Type, typename std::enable_if< !std::is_arithmetic<TP>::value, int>::type = 0>
		inline operator RectT<T>() const { return RectT<T>(x, y, w, h); }

		template<typename T, typename TP = Type, typename std::enable_if< !std::is_arithmetic<TP>::value, int>::type = 0>
		inline operator CoordT<T>() const { return CoordT<T>(x, y); }

		template<typename T, typename TP = Type, typename std::enable_if< !std::is_arithmetic<TP>::value, int>::type = 0>
		inline operator SizeT<T>() const { return SizeT<T>(w, h); }

		//

		inline RectT<Type>& operator=(const RectT<Type>&);					///< @brief Normal assignment operator.
		inline bool operator==(const RectT<Type>& rect) const;
		inline bool operator!=(const RectT<Type>& rect) const;

		inline RectT<Type>& operator+=(const SizeT<Type>& k);				///< @brief Increase size of rectangle.
		inline RectT<Type>& operator-=(const SizeT<Type>& k);				///< @brief Decrease size of rectangle.
		inline RectT<Type> operator+(const SizeT<Type>& k) const; 		///< @brief Returns rectangle with size increased by Size.
		inline RectT<Type> operator-(const SizeT<Type>& k) const; 		///< @brief Returns rectangle with size decreased by Size.

		inline RectT<Type>& operator+=(const CoordT<Type>& k); 				///< @brief Increase position of rectangle.
		inline RectT<Type>& operator-=(const CoordT<Type>& k);				///< @brief Decrease position of rectangle.
		inline RectT<Type> operator+(const CoordT<Type>& k) const; 		///< @brief Returns rectangle with position increased by Coord.
		inline RectT<Type> operator-(const CoordT<Type>& k) const; 		///< @brief Returns rectangle with position decreased by Coord.

		inline RectT<Type>& operator+=(const BorderT<Type>& k);				///< @brief Grow rectangle by the specified border.
		inline RectT<Type>& operator-=(const BorderT<Type>& k);
		inline RectT<Type> operator+(const BorderT<Type>& k) const;
		inline RectT<Type> operator-(const BorderT<Type>& k) const;

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline RectT<Type>& operator*=(Type2 v) { x *= v; y *= v; w *= v; h *= v; return *this; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline RectT<Type>& operator/=(Type2 v) { x /= v; y /= v; w /= v; h /= v; return *this; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline RectT<Type> operator*(Type2 v) const { return { x * v, y * v, w * v, h * v }; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline RectT<Type> operator/(Type2 v) const { return { x / v, y / v, w / v, h / v }; }


		//.____ Properties __________________________________________

		Type	x;		///< Start position in pixels along the X axis.
		Type	y;		///< Start position in pixels along the Y axis.
		Type	w;		///< Width, i.e. length of rectangle along the X axis.
		Type	h;		///< Height, i.e. length of rectangle along the Y axis.
	};


	//_____________________________________________________________________________
	inline Coord::Coord(const Rect& rect)
	{
		x = rect.x;
		y = rect.y;
	}

	//_____________________________________________________________________________
	inline const Size& Coord::toSize() const
	{
		return reinterpret_cast<const Size&>(*this);
	}

	//_____________________________________________________________________________

	inline const CoordI	Coord::px() const
	{
		return { x.qpix >> 2, y.qpix >> 2 };
	}

	//_____________________________________________________________________________

	inline const CoordF	Coord::fpx() const
	{
		return { float(x.qpix)/4, float(y.qpix)/4 };
	}

	//_____________________________________________________________________________

	inline Size Border::size() const
	{
		return Size(left + right, top + bottom);
	}

	//_____________________________________________________________________________

	const BorderI Border::px() const 
	{ 
		return { top.qpix >> 2, right.qpix >> 2, bottom.qpix >> 2, left.qpix >> 2 }; 
	}

	//_____________________________________________________________________________

	const BorderF Border::fpx() const
	{
		return { float(top.qpix)/4, float(right.qpix)/4, float(bottom.qpix)/4, float(left.qpix)/4 };
	}

	//_____________________________________________________________________________

	inline Border::operator Size() const
	{
		return Size(left + right, top + bottom);
	}

	//_____________________________________________________________________________

	inline Size::Size(const Rect& rect) : w(rect.w), h(rect.h)
	{
	}

	//_____________________________________________________________________________

	inline void Size::limit(const Size& min, const Size& max)
	{
		if (w < min.w)
			w = min.w;
		if (h < min.h)
			h = min.h;

		if (w > max.w)
			w = max.w;
		if (h > max.h)
			h = max.h;
	}

	//_____________________________________________________________________________

	const SizeI Size::px() const 
	{ 
		return { w.qpix >> 2, h.qpix >> 2 }; 
	}

	//_____________________________________________________________________________

	const SizeF Size::fpx() const
	{
		return { float(w.qpix)/4, float(h.qpix)/4 };
	}

	//_____________________________________________________________________________

	const RectI Rect::px() const 
	{ 
		return { x.qpix >> 2, y.qpix >> 2, w.qpix >> 2, h.qpix >> 2 }; 
	}

	//_____________________________________________________________________________

	const RectF Rect::fpx() const
	{
		return { float(x.qpix)/4, float(y.qpix)/4, float(w.qpix)/4, float(h.qpix)/4 };
	}

	//_____________________________________________________________________________
	/**
	 * Normal assignment operator.
	 **/
	inline Rect& Rect::operator=(const Rect& r2)
	{
		x = r2.x;
		y = r2.y;
		w = r2.w;
		h = r2.h;
		return *this;
	}

	//_____________________________________________________________________________
	/**
	 * Standard comparison operator
	 **/
	inline bool Rect::operator==(const Rect& rect) const
	{
		return x == rect.x && y == rect.y && w == rect.w && h == rect.h;
	}

	//_____________________________________________________________________________
	/**
	 * Standard comparison operator
	 **/
	inline bool Rect::operator!=(const Rect& rect) const
	{
		return !(*this == rect);
	}

	//_____________________________________________________________________________
	/**
	 * Increase size of rectangle.
	 **/
	inline Rect& Rect::operator+=(const Size& k)
	{
		w += k.w;
		h += k.h;
		return *this;
	}

	//_____________________________________________________________________________
	/**
	 * Decrease size of rectangle.
	 **/
	inline Rect& Rect::operator-=(const Size& k)
	{
		w -= k.w;
		h -= k.h;
		return *this;
	}
	//_____________________________________________________________________________
	/**
	 * Get rectangle with size increased by Size.
	 **/

	inline Rect Rect::operator+(const Size& k) const
	{
		Rect res;
		res.x = x;
		res.y = y;
		res.w = w + k.w;
		res.h = h + k.h;
		return res;
	}

	//_____________________________________________________________________________
	/**
	 * Get rectangle with size decreased with Size.
	 **/

	inline Rect Rect::operator-(const Size& k) const
	{
		Rect res;
		res.x = x;
		res.y = y;
		res.w = w - k.w;
		res.h = h - k.h;
		return res;
	}


	//_____________________________________________________________________________
	/**
	 * Increase position of rectangle.
	 **/

	inline Rect& Rect::operator+=(const Coord& k)
	{
		x += k.x;
		y += k.y;
		return *this;
	}

	//_____________________________________________________________________________
	/**
	 * Decrease position of rectangle.
	 **/

	inline Rect& Rect::operator-=(const Coord& k)
	{
		x -= k.x;
		y -= k.y;
		return *this;
	}

	//_____________________________________________________________________________
	/**
	 * Get rectangle with position increased by Coord.
	 **/

	inline Rect Rect::operator+(const Coord& k) const
	{
		Rect res;
		res.x = x + k.x;
		res.y = y + k.y;
		res.w = w;
		res.h = h;
		return res;
	}

	//_____________________________________________________________________________
	/**
	 * Get rectangle with position decreased by Coord.
	 **/
	inline Rect Rect::operator-(const Coord& k) const
	{
		Rect res;
		res.x = x - k.x;
		res.y = y - k.y;
		res.w = w;
		res.h = h;
		return res;
	}


	//_____________________________________________________________________________
	/**
	 * Grow rectangle by the specified border.
	 *
	 * The position of the rectangle is affected by the top and left borders.
	 **/
	inline Rect& Rect::operator+=(const Border& k)
	{
		x -= k.left;
		y -= k.top;
		w += k.left + k.right;
		h += k.top + k.bottom;
		return *this;
	}

	//_____________________________________________________________________________
	/**
	 * @brief Shrink rectangle by the specified border.
	 *
	 * Shrink rectangle by the specified border.
	 * The position of the rectangle is affected by the top and left borders.
	 **/
	inline Rect& Rect::operator-=(const Border& k)
	{
		x += k.left;
		y += k.top;
		w -= k.left + k.right;
		h -= k.top + k.bottom;
		return *this;
	}


	//_____________________________________________________________________________
	/**
	 * @brief Returns a rectangle grown by the border.
	 *
	 * Returns a rectangle grown by the border.
	 * The position of the rectangle is affected by the top and left borders.
	 **/
	inline Rect Rect::operator+(const Border& k) const
	{
		Rect res;
		res.x = x - k.left;
		res.y = y - k.top;
		res.w = w + k.left + k.right;
		res.h = h + k.top + k.bottom;
		return res;
	}

	//_____________________________________________________________________________
	/**
	 * @brief Returns a rectangle shrunk by the border.
	 *
	 * Returns a rectangle shrunk by the border.
	 * The position of the rectangle is affected by the top and left borders.
	 **/
	inline Rect Rect::operator-(const Border& k) const
	{
		Rect	res;
		res.x = x + k.left;
		res.y = y + k.top;
		res.w = w - (k.left + k.right);
		res.h = h - (k.top + k.bottom);
		return res;
	}


	//_____________________________________________________________________________
	/**
	 * Set position (members x and y) of the rectangle.
	 **/
	inline void Rect::setPos(const Coord& p)
	{
		x = p.x;
		y = p.y;
	}

	//_____________________________________________________________________________
	/**
	 * Set the size (members w and h) of the rectangle.
	 **/
	inline void Rect::setSize(const Size& sz)
	{
		w = sz.w;
		h = sz.h;
	}

	//_____________________________________________________________________________
	/**
	 * Get width of the rectangle (member variable w).
	 **/
	inline MU Rect::width() const
	{
		return w;
	}

	//_____________________________________________________________________________
	/**
	 * Get height of the rectangle (member variable h).
	 **/
	inline MU Rect::height() const
	{
		return h;
	}

	//_____________________________________________________________________________
	/**
	 * Get size (width and height) of the rectangle.
	 **/
	inline Size Rect::size() const
	{
		return Size(w, h);
	}

	//_____________________________________________________________________________
	/**
	 * Get position (members x and y) of the rectangle.
	 **/
	inline Coord Rect::pos() const
	{
		return Coord(x, y);
	}

	//_____________________________________________________________________________
	/**
	 * Get X coordinate of left edge. This is equal to member variable x.
	 **/
	inline MU Rect::left() const
	{
		return x;
	}

	//_____________________________________________________________________________
	/**
	 * Get Y coordinate of top edge. This is equal to member variable y.
	 **/
	inline MU Rect::top() const
	{
		return y;
	}

	//_____________________________________________________________________________
	/**
	 * Get X coordinate of right edge. This is equal to member variables x + w.
	 **/
	inline MU Rect::right() const
	{
		return x + w;
	}

	//_____________________________________________________________________________
	/**
	 * Get Y coordinate of bottom edge. This is equal to member variables y + h.
	 **/
	inline MU Rect::bottom() const
	{
		return y + h;
	}
	//_____________________________________________________________________________
	/**
	 * 	Check if rectangle has no area (width or height is zero).
	 **/
	inline bool	Rect::isEmpty() const
	{
		return (w == 0 || h == 0) ? true : false;
	}

	//_____________________________________________________________________________
	/**
	 * Sets all values to zero.
	 **/
	inline void Rect::clear()
	{
		x = 0;
		y = 0;
		w = 0;
		h = 0;
	}


	//____ RectT() _______________________________________________________________
	/**
	 * Create rectangle from intersection of specified rectangles.
	 *
	 * @param r1	First intersecting rectangle.
	 * @param r2	Second intersecting rectangle.
	 *
	 * If specified rectangles don't intersect, width and height will be set to zero.
	 *
	 **/
	template<typename Type>
	RectT<Type>::RectT(const RectT<Type>& r1, const RectT<Type>& r2)
	{
		intersection(r1, r2);
	}

	/**
	 * Create rectangle to cover the area between the specified coordinates.
	 *
	 * @param p1	Coordinate specifying any corner of the rectangle.
	 * @param p2	Coordinate specifying the opposite corner of the rectangle,
	 * 				diagonally across the rectangle from p1.
	 *
	 * The order of the parameters are irrelevant.
	 *
	 **/

	template<typename Type>
	RectT<Type>::RectT(const CoordT<Type>& p1, const CoordT<Type>& p2)
	{
		if (p1.x < p2.x)
		{
			x = p1.x;
			w = p2.x - p1.x;
		}
		else
		{
			x = p2.x;
			w = p1.x - p2.x;
		}

		if (p1.y < p2.y)
		{
			y = p1.y;
			h = p2.y - p1.y;
		}
		else
		{
			y = p2.y;
			h = p1.y - p2.y;
		}
	}



	//_____________________________________________________________________________
	/**
	 * Check if given coordinate is within rectangle.
	 *
	 * @param coord Coordinate to check.
	 *
	 * @return True if coordinate is within the bounds of the rectangle.
	 **/
	template<typename Type>
	inline bool RectT<Type>::contains( const CoordT<Type>& coord ) const
	{
//		int a = coord.x - x;
//		int b = coord.y - y;
//		int c = (x + w) - (coord.x);
//		int d = (y + h) - (coord.y);

//		return (a | b | c | d) >= 0;

		if( coord.x >= x && coord.x < x + w && coord.y >= y && coord.y < y + h )
			return true;
		return false;
	}

	//_____________________________________________________________________________
	/**
	 * Check if parameter rectangle is fully within our rectangle.
	 *
	 * @param rect Rectangle to check.
	 *
	 * @return True if the parameter rectangle is fully within the bounds of our rectangle.
	 **/
	template<typename Type>
	inline bool RectT<Type>::contains( const RectT<Type>& rect ) const
	{
//		int a = rect.x - x;
//		int b = rect.y - y;
//		int c = (x + w) - (rect.x + rect.w);
//		int d = (y + h) - (rect.y + rect.h);

//		return (a | b | c | d) >= 0;

		if( rect.x >= x && rect.y >= y && rect.x + rect.w <= x + w &&  rect.y + rect.h <= y + h )
			return true;
		return false;
	}

	//_____________________________________________________________________________
	/**
	 * Limit given coordinate to stay within rectangle.
	 *
	 * @param coord Coordinate to limit to the bounds of the rectangle.
	 *
	 * @return Coordinate limited to the rectangles geometry.
	 **/
	template<typename Type>
	 inline CoordT<Type> RectT<Type>::limit( const CoordT<Type>& coord ) const
	{
		CoordT<Type> out = coord;
		if( out.x < x )
			out.x = x;
		if( out.y < y )
			out.y = y;
		if( out.x > x + w )
			out.x = x + w;
		if( out.y > y + h )
			out.y = y + h;

		return out;
	}


	//_____________________________________________________________________________
	/**
	 * Check for intersection (partial or full overlap) with specified rectangle.
	 *
	 * @param rect Rectangle to check against.
	 *
	 * @return True if rectangles intersects.
	 **/
	 template<typename Type>
	 inline bool RectT<Type>::intersectsWith( const RectT<Type>& rect ) const
	{
		if(left() >= rect.right()) return false;
		if(right() <= rect.left()) return false;
		if(top() >= rect.bottom()) return false;
		if(bottom() <= rect.top()) return false;

		return true;
	}

	//_____________________________________________________________________________
	/**
	 * Normal assignment operator.
	 **/
	 template<typename Type>
	 inline RectT<Type>& RectT<Type>::operator=( const RectT<Type>& r2 )
	{
		x = r2.x;
		y = r2.y;
		w = r2.w;
		h = r2.h;
		return *this;
	}

	//_____________________________________________________________________________
	/**
	 * Standard comparison operator
	 **/
	 template<typename Type>
	 inline bool RectT<Type>::operator==( const RectT<Type>& rect) const
	{
		return x == rect.x && y == rect.y && w == rect.w && h == rect.h;
	}

	//_____________________________________________________________________________
	/**
	 * Standard comparison operator
	 **/
	 template<typename Type>
	 inline bool RectT<Type>::operator!=( const RectT<Type>& rect) const
	{
		return !(*this == rect);
	}

	//_____________________________________________________________________________
	/**
	 * Increase size of rectangle.
	 **/
	 template<typename Type>
	 inline RectT<Type>& RectT<Type>::operator+=(const SizeT<Type>& k)
	{
		w += k.w;
		h += k.h;
		return *this;
	}

	//_____________________________________________________________________________
	/**
	 * Decrease size of rectangle.
	 **/
	 template<typename Type>
	 inline RectT<Type>& RectT<Type>::operator-=(const SizeT<Type>& k)
	{
		w -= k.w;
		h -= k.h;
		return *this;
	}
	//_____________________________________________________________________________
	/**
	 * Get rectangle with size increased by Size.
	 **/

	 template<typename Type>
	 inline RectT<Type> RectT<Type>::operator+(const SizeT<Type>& k) const
	{
		RectT<Type> res;
		res.x = x;
		res.y = y;
		res.w = w + k.w;
		res.h = h + k.h;
		return res;
	}

	//_____________________________________________________________________________
	/**
	 * Get rectangle with size decreased with Size.
	 **/

	 template<typename Type>
	 inline RectT<Type> RectT<Type>::operator-(const SizeT<Type>& k) const
	{
		RectT<Type> res;
		res.x = x;
		res.y = y;
		res.w = w - k.w;
		res.h = h - k.h;
		return res;
	}


	//_____________________________________________________________________________
	/**
	 * Increase position of rectangle.
	 **/

	 template<typename Type>
	 inline RectT<Type>& RectT<Type>::operator+=(const CoordT<Type>& k)
	{
		x += k.x;
		y += k.y;
		return *this;
	}

	//_____________________________________________________________________________
	/**
	 * Decrease position of rectangle.
	 **/

	 template<typename Type>
	 inline RectT<Type>& RectT<Type>::operator-=(const CoordT<Type>& k)
	{
		x -= k.x;
		y -= k.y;
		return *this;
	}

	//_____________________________________________________________________________
	/**
	 * Get rectangle with position increased with Coord.
	 **/

	 template<typename Type>
	 inline RectT<Type> RectT<Type>::operator+(const CoordT<Type>& k) const
	{
		RectT<Type> res;
		res.x = x + k.x;
		res.y = y + k.y;
		res.w = w;
		res.h = h;
		return res;
	}

	//_____________________________________________________________________________
	/**
	 * Get rectangle with position decreased with Coord.
	 **/
	 template<typename Type>
	 inline RectT<Type> RectT<Type>::operator-(const CoordT<Type>& k) const
	{
		RectT<Type> res;
		res.x = x - k.x;
		res.y = y - k.y;
		res.w = w;
		res.h = h;
		return res;
	}


	//_____________________________________________________________________________
	/**
	 * Grow rectangle by the specified border.
	 *
	 * The position of the rectangle is affected by the top and left borders.
	 **/
	 template<typename Type>
	 inline RectT<Type>& RectT<Type>::operator+=(const BorderT<Type>& k)
	{
		x -= k.left;
		y -= k.top;
		w += k.left + k.right;
		h += k.top + k.bottom;
		return *this;
	}

	//_____________________________________________________________________________
	/**
	 * @brief Shrink rectangle by the specified border.
	 *
	 * Shrink rectangle by the specified border.
	 * The position of the rectangle is affected by the top and left borders.
	 **/
	 template<typename Type>
	 inline RectT<Type>& RectT<Type>::operator-=(const BorderT<Type>& k)
	{
		x += k.left;
		y += k.top;
		w -= k.left + k.right;
		h -= k.top + k.bottom;
		return *this;
	}


	//_____________________________________________________________________________
	/**
	 * @brief Returns a rectangle grown by the border.
	 *
	 * Returns a rectangle grown by the border.
	 * The position of the rectangle is affected by the top and left borders.
	 **/
	 template<typename Type>
	 inline RectT<Type> RectT<Type>::operator+(const BorderT<Type>& k) const
	{
		RectT<Type> res;
		res.x = x - k.left;
		res.y = y- k.top;
		res.w = w + k.left + k.right;
		res.h = h + k.top + k.bottom;
		return res;
	}

	//_____________________________________________________________________________
	/**
	 * @brief Returns a rectangle shrunk by the border.
	 *
	 * Returns a rectangle shrunk by the border.
	 * The position of the rectangle is affected by the top and left borders.
	 **/
	 template<typename Type>
	 inline RectT<Type> RectT<Type>::operator-(const BorderT<Type>& k) const
	{
		RectT<Type> res;
		res.x = x + k.left;
		res.y = y + k.top;
		res.w = w - (k.left + k.right);
		res.h = h - (k.top + k.bottom);
		return res;
	}


	//_____________________________________________________________________________
	/**
	 * Set position (members x and y) of the rectangle.
	 **/
	 template<typename Type>
	 inline void RectT<Type>::setPos( const CoordT<Type>& p )
	{
		x = p.x;
		y = p.y;
	}

	//_____________________________________________________________________________
	/**
	 * Set the size (members w and h) of the rectangle.
	 **/
	 template<typename Type>
	 inline void RectT<Type>::setSize( const SizeT<Type>& sz )
	{
		w = sz.w;
		h = sz.h;
	}

	//_____________________________________________________________________________
	/**
	 * Get width of the rectangle (member variable w).
	 **/
	 template<typename Type>
	 inline Type RectT<Type>::width() const
	{
		return w;
	}

	//_____________________________________________________________________________
	/**
	 * Get height of the rectangle (member variable h).
	 **/
	 template<typename Type>
	 inline Type RectT<Type>::height() const
	{
		return h;
	}

	//_____________________________________________________________________________
	/**
	 * Get size (width and height) of the rectangle.
	 **/
	 template<typename Type>
	 inline SizeT<Type> RectT<Type>::size() const
	{
		return SizeT<Type>(w,h);
	}

	//_____________________________________________________________________________
	/**
	 * Get position (members x and y) of the rectangle.
	 **/
	 template<typename Type>
	 inline CoordT<Type> RectT<Type>::pos() const
	{
		return CoordT<Type>(x,y);
	}

	//_____________________________________________________________________________
	/**
	 * Get X coordinate of left edge. This is equal to member variable x.
	 **/
	 template<typename Type>
	 inline Type RectT<Type>::left() const
	{
		return x;
	}

	//_____________________________________________________________________________
	/**
	 * Get Y coordinate of top edge. This is equal to member variable y.
	 **/
	 template<typename Type>
	 inline Type RectT<Type>::top() const
	{
		return y;
	}

	//_____________________________________________________________________________
	/**
	 * Get X coordinate of right edge. This is equal to member variables x + w.
	 **/
	 template<typename Type>
	 inline Type RectT<Type>::right() const
	{
		return x + w;
	}

	//_____________________________________________________________________________
	/**
	 * Get Y coordinate of bottom edge. This is equal to member variables y + h.
	 **/
	 template<typename Type>
	 inline Type RectT<Type>::bottom() const
	{
		return y + h;
	}
	//_____________________________________________________________________________
	/**
	 * 	Check if rectangle has no area (width or height is zero).
	 **/
	 template<typename Type>
	 inline bool	RectT<Type>::isEmpty() const
	{
		return (w==0||h==0)?true:false;
	}

	//_____________________________________________________________________________
	/**
	 * Sets all values to zero.
	 **/
	 template<typename Type>
	 inline void RectT<Type>::clear()
	{
		x = 0;
		y = 0;
		w = 0;
		h = 0;
	}

	 //____ intersection() ________________________________________________________________
	 /**
	  * @brief Set rectangle to intersection of specified rectangles.
	  *
	  * Set rectangle to intersection of specified rectangles.
	  *
	  * @param r1	First intersecting rectangle.
	  * @param r2	Second intersecting rectangle.
	  *
	  * If specified rectangles don't intersect, width and height will be set to zero.
	  *
	  * @return	True if the specified rectangles intersected.
	  **/
	 template<typename Type>
	 bool RectT<Type>::intersection(const RectT<Type>& r1, const RectT<Type>& r2)
	 {
		 Type		x1, y1;						// So we can use ourself as inparameter.
		 Type		x2, y2;

		 if (r1.x > r2.x)
			 x1 = r1.x;
		 else
			 x1 = r2.x;

		 if (r1.y > r2.y)
			 y1 = r1.y;
		 else
			 y1 = r2.y;

		 if (r1.x + r1.w < r2.x + r2.w)
			 x2 = r1.x + r1.w;
		 else
			 x2 = r2.x + r2.w;

		 if (r1.y + r1.h < r2.y + r2.h)
			 y2 = r1.y + r1.h;
		 else
			 y2 = r2.y + r2.h;

		 x = x1;
		 y = y1;
		 w = x2 - x;
		 h = y2 - y;

		 if (w <= 0 || h <= 0)
		 {
			 w = 0;
			 h = 0;
			 return	false;
		 }

		 return	true;
	 }

	 //____ getUnion() ________________________________________________________________
	 /**
	  * @brief Get the union of specified rectangles.
	  *
	  * Get the union of specified rectangles.
	  *
	  * @param r1	First rectangle.
	  * @param r2	Second rectangle.
	  *
	  * The union of the rectangles is the smallest rectangle that contains both.
	  *
	  * @return	A rectangle that is the union of the specified rectangles.
	  **/

	  //TODO: Inconsistency between how Union and Intersection methods work. Both should be members of static.

	 template<typename Type>
	 RectT<Type> RectT<Type>::getUnion(const RectT<Type>& r1, const RectT<Type>& r2)
	 {
		 RectT<Type> out;
		 out.x = r1.x < r2.x ? r1.x : r2.x;
		 out.y = r1.y < r2.y ? r1.y : r2.y;
		 out.w = (r1.x + r1.w > r2.x + r2.w ? r1.x + r1.w : r2.x + r2.w) - out.x;
		 out.h = (r1.y + r1.h > r2.y + r2.h ? r1.y + r1.h : r2.y + r2.h) - out.y;
		 return out;
	 }


	 //____ shrink() _____________________________________________________________
	 /**
	  * @brief Shrink the rectangle by the specified border.
	  *
	  * Shrink the rectangle by the specified border.
	  *
	  * @param border	Border by which to shrink the rectangle.
	  *
	  * Width and height of rectangle is shrunk by the thickness of the borders.
	  * Top and left borders also affects the position of the rectangle.
	  *
	  * The rectangle is constrained to a minimum width and height of 0.
	  **/

	 template<typename Type>
	 void RectT<Type>::shrink(const BorderT<Type> &border)
	 {
		 x += border.left;
		 y += border.top;
		 w -= border.width();
		 h -= border.height();

		 if (w < 0)
			 w = 0;

		 if (h < 0)
			 h = 0;
	 }

	 //____ grow() _____________________________________________________________
	 /**
	  * @brief Grow the rectangle by the specified border.
	  *
	  * Grow the rectangle by the specified border.
	  *
	  * @param border	BorderI by which to grow the rectangle.
	  *
	  * Width and height of rectangle is increased by the thickness of the borders.
	  * Top and left borders also affects the position of the rectangle.
	  **/
	 template<typename Type>
	 void RectT<Type>::grow(const BorderT<Type> &border)
	 {
		 x -= border.left;
		 y -= border.top;
		 w += border.width();
		 h += border.height();
	 }

	 /**
	  * @brief	Grow the reactangle to contain the specified coordinate.
	  *
	  * Grow the reactangle to contain the specified coordinate.
	  *
	  * @param	coord	Coordinate the rectangle is grown to contain.
	  *
	  * Position of rectangle is affected if coordinate is above or left of coordinate.
	  **/
	 template<typename Type>
	 void RectT<Type>::growToContain(const CoordT<Type>& coord)
	 {
		 if (coord.x < x)
		 {
			 w += x - coord.x;
			 x = coord.x;
		 }
		 else if (coord.x > x + w)
		 {
			 w = coord.x - x;
		 }

		 if (coord.y < y)
		 {
			 h += y - coord.y;
			 y = coord.y;
		 }
		 else if (coord.y > y + h)
		 {
			 h = coord.y - y;
		 }
	 }

	 /**
	  * @brief	Grow the reactangle to contain the specified rectangle.
	  *
	  * Grow the reactangle to contain the specified rectangle.
	  *
	  * @param	r	Rectangle that shall be contained.
	  *
	  * Position of our rectangle is affected if parameter rectangle is (partly or fully) above or left of coordinate.
	  **/

	 template<typename Type>
	 void RectT<Type>::growToContain(const RectT<Type>& r)
	 {
		 if (r.x < x)
		 {
			 w += x - r.x;
			 x = r.x;
		 }

		 if (r.y < y)
		 {
			 h += y - r.y;
			 y = r.y;
		 }

		 if (r.x + r.w > x + w)
		 {
			 w = r.x + r.w - x;
		 }

		 if (r.y + r.h > y + h)
		 {
			 h = r.y + r.h - y;
		 }
	 }



	 //____ intersectsWithOrContains() _______________________________________________________

	 template<typename Type>
	 bool RectT<Type>::intersectsWithOrContains(CoordT<Type> p1, CoordT<Type> p2, int precision) const
	 {
		 // NOTE: This needs to be kept in sync with ClipLine() so they always return same result for same input.

		 Type x1 = x;
		 Type x2 = x + w;

		 // We go from left to right when working with the X coordinates

		 if (p2.x < p1.x)
			 std::swap(p1, p2);

		 // Verify that x-coordinates are within range

		 if (p1.x >= x2 || p2.x <= x1)
			 return false;

		 // Calculate angles now before we destroy any values

		 Type angleX;
		 if( std::is_integral<Type>::value )
			angleX = ((p2.y - p1.y) << precision) / (1 + p2.x - p1.x);		// Change in Y for each increase of X.
		 else
			angleX = (p2.y - p1.y) / (1 + p2.x - p1.x);					// Change in Y for each increase of X.


		 // Clip line so no coord is outside rectangles x-dimensions

		 if (std::is_integral<Type>::value)
		 {
			 if (p1.x < x1)
				 p1.y = p1.y + ((x1 - p1.x)*angleX >> precision);

			 if (p2.x > x2)
				 p2.y = p2.y - ((p2.x - x2)*angleX >> precision);
		 }
		 else
		 {
			 if (p1.x < x1)
				 p1.y = p1.y + (x1 - p1.x)*angleX;

			 if (p2.x > x2)
				 p2.y = p2.y - (p2.x - x2)*angleX;
		 }

		 // We go from top to bottom when working with the Y coordinates

		 if (p2.y < p1.y)
			 std::swap(p1.y, p2.y);

		 // Verify that clipped line intersects in y-dimension.

		 if (p1.y >= y + h || p2.y <= y)
			 return false;

		 return true;
	 }

	 //____ clipLine() _____________________________________________________________

	template<typename Type>
	bool RectT<Type>::clipLine(CoordT<Type>* p1, CoordT<Type>* p2, int precision) const
	{
		// NOTE: This needs to be kept in sync with IntersectsOrContains() so they always return same result for same input.

		Type x1 = x;
		Type x2 = x + w;
		Type y1 = y;
		Type y2 = y + h;

		// We go from left to right when working with the X coordinates

		if (p2->x < p1->x)
			std::swap(p1, p2);

		// Verify that x-coordinates are within range

		if (p1->x >= x2 || p2->x <= x1)
			return false;

		// Calculate both angles now before we destroy precision
		Type angleX, angleY;
		if (std::is_integral<Type>::value)
		{
			angleX = ((p2->y - p1->y) << precision) / (1 + p2->x - p1->x);		// Change in Y for each increase of X.
			angleY = ((p2->x - p1->x) << precision) / (1 + std::abs(p2->y - p1->y));		// Change in X for each increase of Y.
		}
		else
		{
			angleX = (p2->y - p1->y) / (1 + p2->x - p1->x);					// Change in Y for each increase of X.
			angleY = (p2->x - p1->x) / (1 + std::abs(p2->y - p1->y));		// Change in X for each increase of Y.
		}

		// Clip line so no coord is outside rectangles x-dimensions

		if (p1->x < x1)
		{
			if (std::is_integral<Type>::value)
				p1->y = p1->y + ((x1 - p1->x)*angleX >> precision);
			else
				p1->y = p1->y + (x1 - p1->x)*angleX;
			p1->x = x2;
		}

		if (p2->x > x2)
		{
			if (std::is_integral<Type>::value)
				p2->y = p2->y - ((p2->x - x2)*angleX >> precision);
			else
				p2->y = p2->y - (p2->x - x2)*angleX;

			p2->x = x2;
		}

		 // We go from top to bottom when working with the Y coordinates

		 if (p2->y < p1->y)
		 {
			 std::swap(p1, p2);
			 angleY = -angleY;
		 }

		 // Verify that clipped line intersects in y-dimension.

		 if (p1->y >= y2 || p2->y <= y1)
			 return false;

		 // Clip line so no coord is outside rectangles y-dimensions

		 if (p1->y < y1)
		 {
			 if (std::is_integral<Type>::value)
				 p1->x = p1->x + ((y1 - p1->y)*angleY >> precision);
			 else
				 p1->x = p1->x + (y1 - p1->y)*angleY;

			 p1->y = y1;
		 }

		 if (p2->y > y2)
		 {
			 if (std::is_integral<Type>::value)
				 p2->x = p2->x - ((p2->y - y2)*angleY >> precision);
			 else
				p2->x = p2->x - (p2->y - y2)*angleY;
			 p2->y = y2;
		 }

		 return true;
	 }

	//_____________________________________________________________________________

	template<typename Type>
	CoordT<Type> RectT<Type>::distance( CoordT<Type> coord ) const
	{
		coord.x -= x;
		if( coord.x >= 0 )
		{
			coord.x -= w;
			if( coord.x < 0 )
				coord.x = 0;
		}

		coord.y -= y;
		if( coord.y >= 0 )
		{
			coord.y -= h;
			if( coord.y < 0 )
				coord.y = 0;
		}
		return coord;
	}

	 //_____________________________________________________________________________

	template<typename Type>
	SizeT<Type> BorderT<Type>::size() const
	 {
		 return SizeT<Type>(left + right, top + bottom);
	 }


	//_____________________________________________________________________________


	template<typename Type>
	CoordT<Type>::operator Rect() const
	{
		return Rect(x, y, 0, 0);
	}

	template<typename Type>
	SizeT<Type>::operator Rect() const
	{
		return Rect(0, 0, w, h);

	}

	template<typename Type>
	RectT<Type>::operator Rect() const
	{
		return Rect(x, y, w, h);
	}

	//=======================================================================================



} // namespace wg
#endif	//WG_GEO_DOT_H
