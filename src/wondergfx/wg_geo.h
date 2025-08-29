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

#include <wg_gfxtypes.h>

#include <utility>
#include <cstdlib>
#include <algorithm>

namespace wg
{
	template<class T> class CoordT;
	template<class T> class BorderT;
	template<class T> class SizeT;
	template<class T> class RectT;

	typedef CoordT<pts>		Coord;
	typedef CoordT<spx>		CoordSPX;
	typedef	CoordT<short>	CoordS;
	typedef	CoordT<int>		CoordI;
	typedef CoordT<float>	CoordF;

	typedef BorderT<pts>	Border;
	typedef BorderT<spx>	BorderSPX;
	typedef BorderT<short>	BorderS;
	typedef BorderT<int>	BorderI;
	typedef BorderT<float>	BorderF;

	typedef	SizeT<pts>		Size;
	typedef	SizeT<spx>		SizeSPX;
	typedef	SizeT<short>	SizeS;
	typedef	SizeT<int>		SizeI;
	typedef SizeT<float>	SizeF;

	typedef	RectT<pts>		Rect;
	typedef	RectT<spx>		RectSPX;
	typedef	RectT<short>	RectS;
	typedef	RectT<int>		RectI;
	typedef RectT<float>	RectF;

	typedef CoordT<binalInt>	BinalCoord;
	typedef SizeT<binalInt>		BinalSize;
	typedef	RectT<binalInt>		BinalRect;



	//____ CoordT<> ___________________________________________________________
	/**
		@brief Class template for coordinate classes containing X and Y values.

		Class template for coordinate classes containing X and Y values.

		Specializations include:

		- __Coord__		- coordinate specified in pts (points)
		- __CoordSPX__	- coordinate specified in spx (subpixels, e.g 1/64 pixel)
		- __CoordI__	- coordinate in integer format
		- __CoordF__	- coordinate in float format

	**/
	template<class Type> class CoordT
	{
	public:
		//.____ Creation __________________________________________

		constexpr CoordT() : x(0), y(0) {}
		constexpr CoordT(Type x, Type y) : x(x), y(y) {}
//		CoordT(const CoordT<Type>& coord) : x(coord.x), y(coord.y) {}
		explicit constexpr CoordT(const SizeT<Type>& size);

		template<typename T>
		explicit constexpr CoordT(const CoordT<T>& r) : x((Type)r.x), y((Type)r.y) {}

		template<typename T>
		explicit constexpr CoordT(const RectT<T>& r) : x((Type)r.x), y((Type)r.y) {}


		//.____ Misc ______________________________________________

		inline void clear() { x = 0; y = 0; }			///< Sets X and Y to 0.

		//.____ Operators ___________________________________________

//		inline CoordT<Type> operator=(const CoordT<Type>& k) { x = k.x; y = k.y; return *this; }

		inline bool operator==(const CoordT<Type>& k) const { if (x == k.x && y == k.y) return true; return false; }
		inline bool operator!=(const CoordT<Type>& k) const { if (x != k.x || y != k.y) return true; return false; }

		inline CoordT<Type>& operator+=(const CoordT<Type>& k) { x += k.x; y += k.y; return *this; }
		inline CoordT<Type>& operator-=(const CoordT<Type>& k) { x -= k.x; y -= k.y; return *this; }
		inline CoordT<Type> operator+(const CoordT<Type>& k) const { return { x + k.x, y + k.y }; }
		inline CoordT<Type> operator-(const CoordT<Type>& k) const { return { x - k.x, y - k.y }; }
        inline CoordT<Type> operator-() const { return { -x, -y }; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline CoordT<Type>& operator*=(Type2 v) { x = (Type)(x*v); y = (Type)(y*v); return *this; }

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
	inline constexpr CoordT<Type>::CoordT(const SizeT<Type>& size)
	: x(size.w), y(size.h)
	{
	}

	//____ BorderT ______________________________________________________________
	/**
		@brief Specifies the thickness of the border of a rectangular area.

		Class Template for specifying the thickness of the border of a rectangular area.

		Specializations include:

		- __Border__	- border specified in pts (points)
		- __BorderSPX__	- border specified in spx (subpixels, e.g 1/64 pixel)
		- __BorderI__	- border in integer format
		- __BorderF__	- border in float format

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

		inline bool	contains(const BorderT<Type>& border) const;			///< @brief Check if border is smaller in all directions.

		void growToContain( const BorderT<Type>& border );


		static inline BorderT<Type> min( const BorderT<Type>& r1, const BorderT<Type>& r2 );
		static inline BorderT<Type> max( const BorderT<Type>& r1, const BorderT<Type>& r2 );
		static inline BorderT<Type> diff( const RectT<Type>& inner, const RectT<Type>& outer );

		//.____ Operators ___________________________________________

		inline operator SizeT<Type>() const { return SizeT<Type>(left + right, top + bottom); }

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


	//____ min() ________________________________________________________________
	/**
	 * @brief Get the smallest values of the two borders.
	 *
	 * Compares each side of the two borders and returns a border with the smallest values.
	 *
	 * @return	A border with the smallest values.
	 **/

	template<typename Type>
	BorderT<Type> BorderT<Type>::min( const BorderT<Type>& b1, const BorderT<Type>& b2 )
	{
		return { std::min( b1.top, b2.top ), std::min( b1.right, b2.right ), std::min( b1.bottom, b2.bottom ), std::min( b1.left, b2.left ) };
	}

	//____ max() ________________________________________________________________
	/**
	 * @brief Get the largest values of the two borders.
	 *
	 * Compares each side of the two borders and returns a border with the largest values.
	 *
	 * @return	A border with the largest values.
	 **/

	template<typename Type>
	BorderT<Type> BorderT<Type>::max( const BorderT<Type>& b1, const BorderT<Type>& b2 )
	{
		return { std::max( b1.top, b2.top ), std::max( b1.right, b2.right ), std::max( b1.bottom, b2.bottom ), std::max( b1.left, b2.left ) };
	}

	//____ diff() ________________________________________________________________

	template<typename Type>
	BorderT<Type> BorderT<Type>::diff( const RectT<Type>& inner, const RectT<Type>& outer )
	{
		return {inner.y - outer.y, (outer.x + outer.w) - (inner.x + inner.w),
				(outer.y + outer.h) - (inner.y + inner.h), inner.x - outer.x };
	}

	//____ contains() ________________________________________________________________

	template<typename Type>
	bool BorderT<Type>::contains(const BorderT<Type>& border) const
	{
		return (top >= border.top && right >= border.right && bottom >= border.bottom && left >= border.left);
	}

	//____ growToContain() ________________________________________________________________

	template<typename Type>
	void BorderT<Type>::growToContain( const BorderT<Type>& border )
	{
		top		= std::max( top, border.top );
		right	= std::max( right, border.right );
		bottom	= std::max( bottom, border.bottom );
		left	= std::max( left, border.left );
	}

	//____ SizeT<T> ________________________________________________________

	/**
		@brief Specifies the size of a rectangular area.

		Class Template for specifying the size of a rectangular area.

		Specializations include:

		- __Size__		- size specified in pts (points)
		- __SizeSPX__	- size specified in spx (subpixels, e.g 1/64 pixel)
		- __SizeI__		- size in integer format
		- __SizeF__		- size in float format


		Specifies the size of a rectangular area.
	**/

	template<class Type> class SizeT
	{
	public:
		//.____ Creation __________________________________________

		SizeT() : w(0), h(0) {}
		SizeT( Type width, Type height ) : w(width), h(height) {}
		explicit SizeT( const CoordT<Type>& coord ) : w(coord.x), h(coord.y) {}
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
		inline bool	isValid() const { return (w >= 0 && h >= 0); }

		//.____ Operators ___________________________________________

		inline bool operator==(const SizeT<Type>& k) const	{ if( w == k.w && h == k.h ) return true; return false; }
		inline bool operator!=(const SizeT<Type>& k) const	{ if( w != k.w || h != k.h ) return true; return false; }

		inline SizeT<Type>& operator+=(const SizeT<Type>& k)		{ w += k.w; h += k.h; return *this;}
		inline SizeT<Type>& operator-=(const SizeT<Type>& k)		{ w -= k.w; h -= k.h; return *this;}
		inline SizeT<Type> operator+(const SizeT<Type>& k) const	{ return { w + k.w, h + k.h }; }
		inline SizeT<Type> operator-(const SizeT<Type>& k) const	{ return { w - k.w, h - k.h }; }
        inline SizeT<Type> operator-() const                        { return { -w, -h }; }

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

		Class Template for specifying a rectangular area of a 2D surface through position and size.

		Specializations include:

		- __Rect__		- rectangle specified in pts (points)
		- __RectSPX__	- rectangle specified in spx (subpixels, e.g 1/64 pixel)
		- __RectI__		- rectangle in integer format
		- __RectF__		- rectangle in float format


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

		// All versions of RectI, CoordI and SizeI can be cast to any RectI EXPLICITLY.

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


		static RectT<Type> bounds( const RectT<Type>& r1, const RectT<Type>& r2 );
		static RectT<Type> boundsExcludingEmpty( const RectT<Type>& r1, const RectT<Type>& r2 );

		static RectT<Type> overlap(const RectT<Type>& r1, const RectT<Type>& r2);

		void growToContain( const RectT<Type>& rect );
		void growToContain( const CoordT<Type>& coord );

		inline bool	contains( const CoordT<Type>& coord ) const;		///< @brief Check if coordinate is within rectangle.
		inline bool contains( const RectT<Type>& rect ) const;			///< @brief Check if rectangle is fully within our rectangle.

		inline CoordT<Type> limit( const CoordT<Type>& coord ) const;			///< @brief Limit given coordinate to stay within rectangle.
		inline RectT<Type> limit( const RectT<Type>& rect ) const;				///< @brief Limit given rectangle to stay fully within our rectangle.
		inline CoordT<Type> center() const { return {x+w/2,y+h/2}; }
		inline RectT<Type> center(SizeT<Type> sz) const { return { x + (w-sz.w) / 2,y + (h-sz.h) / 2, sz }; }

		inline bool isOverlapping( const RectT<Type>& rect ) const;	///< @brief Check for overlap with specified rectangle.

		bool		intersectsWithOrContains(CoordT<Type> p1, CoordT<Type> p2, int precision = 14) const;
		bool		clipLine(CoordT<Type> * p1, CoordT<Type> * p2, int precision = 14) const;

		inline SizeT<Type> size() const;						///< @brief Get the size of the rectangle.
		inline CoordT<Type> pos() const;						///< @brief Get the position of the rectangle.

		inline Type left() const;								///< @brief Get X coordinate of left border.
		inline Type top() const;									///< @brief Get Y cordinate of top border.
		inline Type right() const;								///< @brief Get X coordinate of right border.
		inline Type bottom() const;								///< @brief Get Y coordinate of bottom border.

		inline bool	isEmpty() const;							///< @brief Check if rectangle has no area.
		inline bool	isValid() const;							///< @brief Check that width and height are not negtive.
		inline void clear();									///< @brief Sets all values to zero.

		inline CoordT<Type> distance( CoordT<Type> coord ) const;   ////< @brief Get distance (signed) between coordinate and rectangle. 0 if inside.

        inline CoordT<Type> topLeft() const { return {x,y}; }
        inline CoordT<Type> topRight() const { return {x+w,y}; }
        inline CoordT<Type> bottomRight() const { return {x+w,y+h}; }
        inline CoordT<Type> bottomLeft() const { return {x,y+h}; }

		//.____ Operators ___________________________________________

		// All versions of RectT can be IMPLICITLY cast to their subtypes SizeT and CoordT

		inline operator SizeT<Type>() const { return SizeT<Type>(w, h); }
		inline operator CoordT<Type>() const { return CoordT<Type>(x, y); }

		//

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

		template<typename Type2>
		inline RectT<Type>& operator*=(SizeT<Type2> v) { x *= v.w; y *= v.h; w *= v.w; h *= v.h; return *this; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline RectT<Type>& operator*=(Type2 v) { x *= v; y *= v; w *= v; h *= v; return *this; }

		template<typename Type2>
		inline RectT<Type>& operator/=(SizeT<Type2> v) { x /= v.w; y /= v.h; w /= v.w; h /= v.h; return *this; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline RectT<Type>& operator/=(Type2 v) { x /= v; y /= v; w /= v; h /= v; return *this; }

		template<typename Type2>
		inline RectT<Type> operator*(SizeT<Type2> v) const { return { x * v.w, y * v.h, w * v.w, h * v.h }; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline RectT<Type> operator*(Type2 v) const { return { Type(x * v), Type(y * v), Type(w * v), Type(h * v) }; }

		template<typename Type2>
		inline RectT<Type> operator/(SizeT<Type2> v) const { return { x / v.w, y / v.h, w / v.w, h / v.h }; }

		template<typename Type2, class = typename std::enable_if<std::is_arithmetic<Type2>::value>::type>
		inline RectT<Type> operator/(Type2 v) const { return { Type(x / v), Type(y / v), Type(w / v), Type(h / v) }; }


		//.____ Properties __________________________________________

		Type	x;		///< Start position along the X axis.
		Type	y;		///< Start position along the Y axis.
		Type	w;		///< Width, i.e. length of rectangle along the X axis.
		Type	h;		///< Height, i.e. length of rectangle along the Y axis.
	};

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
	 * Limit given rect to stay within this rectangle.
	 *
	 * @param rect Rect to limit to the bounds of.
	 *
	 * @return Rect, limited to the geometry of this rectangle.
	 **/
	template<typename Type>
	inline RectT<Type> RectT<Type>::limit( const RectT<Type>& rect ) const
	{
		RectT<Type> out = rect;
		if(out.w > w)
			out.w = w;

		if(out.h > h)
			out.h = h;

		if(out.right() > right())
			out.x = x + w - out.w;
		else if(out.left() < left())
			out.x = x;

		if(out.top() < top())
			out.y = y;
		else if(out.bottom() > bottom())
			out.y = y + h - out.h;

		return out;
	}

	//_____________________________________________________________________________
	/**
	 * Check for overlap (partial or full) with specified rectangle.
	 *
	 * @param rect Rectangle to check against.
	 *
	 * @return True if rectangles overlap.
	 **/
	 template<typename Type>
	 inline bool RectT<Type>::isOverlapping( const RectT<Type>& rect ) const
	{
		if(left() >= rect.right()) return false;
		if(right() <= rect.left()) return false;
		if(top() >= rect.bottom()) return false;
		if(bottom() <= rect.top()) return false;

		return true;
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
	  * 	Check that rect doesn't have negative width or height.
	  **/
	 template<typename Type>
	 inline bool	RectT<Type>::isValid() const
	 {
		 return (w >= 0 && h >= 0) ? true : false;
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

	 //____ overlap() _________________________________________________
	 /**
	  * @brief Get the overlap of specified rectangles.
	  *
	  * Get the overlap of specified rectangles.
	  *
	  * @param r1	First rectangle.
	  * @param r2	Second rectangle.
	  *
	  * @return	A rectangle that is the overlap between the specified rectangles or an empty
	  *         rectangle if rectangles don't overlap.
	  **/

	template<typename Type>
	RectT<Type> RectT<Type>::overlap(const RectT<Type>& r1, const RectT<Type>& r2)
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

		if(x2 - x1 > 0 && y2 - y1 > 0)
			return	{ x1, y1, x2 - x1, y2 - y1 };
		else
			return { 0, 0, 0, 0 };
	}


	 //____ bounds() ________________________________________________________________
	 /**
	  * @brief Get the bounds of the specified rectangles.
	  *
	  * Get the bounds of the specified rectangles.
	  *
	  * @param r1	First rectangle.
	  * @param r2	Second rectangle.
	  *
	  * The bounds of the rectangles is the smallest rectangle that fully contains both.
	  *
	  * @return	A rectangle that is the bounds of the specified rectangles.
	  **/

	 template<typename Type>
	 RectT<Type> RectT<Type>::bounds(const RectT<Type>& r1, const RectT<Type>& r2)
	 {
		 RectT<Type> out;
		 out.x = r1.x < r2.x ? r1.x : r2.x;
		 out.y = r1.y < r2.y ? r1.y : r2.y;
		 out.w = (r1.x + r1.w > r2.x + r2.w ? r1.x + r1.w : r2.x + r2.w) - out.x;
		 out.h = (r1.y + r1.h > r2.y + r2.h ? r1.y + r1.h : r2.y + r2.h) - out.y;
		 return out;
	 }

	//____ boundsExcludingEmpty() ________________________________________________________________
	/**
	 * @brief Get the bounds of the non-empty specified rectangles.
	 *
	 * Get the bounds of the non-empty specified rectangles.
	 *
	 * @param r1	First rectangle.
	 * @param r2	Second rectangle.
	 *
	 * The bounds of the rectangles is the smallest rectangle that fully contains both. If one of the rectangles are empty
	 *  the other rectangle is returned without modification. If both are emtpy, an empty rectangle is returned.
	 *
	 * @return	A rectangle that is the bounds of the non-empty specified rectangles.
	 **/

	template<typename Type>
	RectT<Type> RectT<Type>::boundsExcludingEmpty(const RectT<Type>& r1, const RectT<Type>& r2)
	{
		if( r1.isEmpty() )
			return r2;

		if( r2.isEmpty() )
			return r1;

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


} // namespace wg
#endif	//WG_GEO_DOT_H
