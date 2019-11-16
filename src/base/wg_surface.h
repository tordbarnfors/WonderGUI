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

#ifndef WG_SURFACE_DOT_H
#define	WG_SURFACE_DOT_H
#pragma once
//==============================================================================

#include <wg_types.h>
#include <wg_geo.h>
#include <wg_color.h>
#include <wg_pointers.h>
#include <wg_blob.h>

namespace wg
{


	class Surface;
	typedef	StrongPtr<Surface>	Surface_p;
	typedef	WeakPtr<Surface>	Surface_wp;

	//____ Surface ______________________________________________________________
	/**
	 * @brief Simple abstract representation of a bitmap image.
	 *
	 * A Surface is a simple bitmap image that can hold graphical elements for
	 * rendering of the GUI such as skin blocks, fonts and icons. Surfaces normally
	 * acts as the source of graphical copy operations but can also be used as destinations
	 * when using offscreen surfaces as rendering caches.
	 *
	 *
	 *
	 **/


	class Surface : public Object
	{
		friend class GfxDevice;

	public:
		//.____ Identification __________________________________________

		bool				isInstanceOf( const char * pClassName ) const override;
		const char *		className( void ) const override;
		static const char	CLASSNAME[];
		static Surface_p	cast( Object * pObject );

		//.____ Geometry _________________________________________________

		SizeI				size() const { return m_size; }		///< @brief Get the size in pixels of the surface.
																///<
																///< Get the width and height of the surface in a SizeI structure.
																///< @return Size of the suface measured in pixels.
		virtual	int			width() const;						///< @brief Get the width of the surface.
		virtual	int			height() const;						///< @brief Get the height of the surface.

		//.____ Appearance ____________________________________________________

		virtual void		setScaleMode( ScaleMode mode );
		ScaleMode			scaleMode() const { return m_scaleMode; }

		bool				isMipmapped() const { return m_bMipmapped; }

		//.____ Content _______________________________________________________

		virtual uint32_t	pixel( CoordI coord ) const = 0;		///< @brief Get pixel at specified coordinate.
																///<
																///< Get the raw pixel value from the specified coordinate of the surface.
																///<
																///< @param coord Coordinate of pixel to get.
																///<
																///< The coordinate must be within the surface boundaries. A coordinate outside
																///< the surface will result in undefined behavior.
																///<
																///< If pixel is less than
																///< 32 bits in size, it is packed into the lower bits of the return value with
																///< the rest of the bits cleared.
																///<
																///< @return Pixel value in surface's native format.

		inline uint32_t		pixel( int x, int y ) const;		///< @brief Get pixel at specified coordinate.

		virtual uint8_t		alpha( CoordI coord ) const = 0;	///< @brief Get Alpha value of pixel at specified coordinate.
																///<
																///< Get the alpha value from the specified coordinate of the surface.
																///<
																///< @param coord Coordinate of pixel.
																///<
																///< Gets the alpha value of the pixel at the specified coordinate in the range of 0-255.
																///< The alpha value is by default used for opacity level,
																///< where 0 is a fully transparent pixel and 255 is opaque.
																///<
																///< The coordinate specified must be within the surface boundaries. A coordinate outside
																///< the surface will result in undefined behavior.
																///<
																///< @return Alpha value of pixel at coordinate.

		inline uint8_t		alpha( int x, int y ) const;		///< @brief Get Alpha value of pixel at specified coordinate.

		virtual	uint32_t	colorToPixel( const Color& col ) const;		///< @brief Convert specified color to a pixel in surface's native format.
		virtual	Color		pixelToColor( uint32_t pixel ) const;		///< @brief Get the color and alpha values of a pixel.

		inline const Color* clut() const { return m_pClut; }

		const PixelDescription*	pixelDescription() const { return &m_pixelDescription; }///< @brief Get the pixel description for the surface.
		PixelFormat			pixelFormat() const { return m_pixelDescription.format; }

		virtual bool		isOpaque() const = 0;				///< @brief Check if surface is entirely opaque.
																///<
																///< Check if surface is entirely opaque.
																///<
																///< A surface is opaque if it contains no transparent or semi-transparent
																///< pixels at all and therefore is guaranteed to completely cover any graphics
																///< behind it. This method is called regularly in time-critical sections of
																///< the code to optimize rendering operations and therefore should not do any
																///< time consuming operations.
																///<
																///< @return True if surface is guaranteed to only contain completely opaque pixels. False if
																///< if it does contain (semi)transparent pixels or we simply just don't know.


		//.____ Control _______________________________________________________

		virtual uint8_t *	lock( AccessMode mode ) = 0;		///< @brief Lock the surface for direct access to pixel data.
																///<
																///< Locks the surface to gain direct access to the pixel data.
																///<
																///< @param mode	AccessMode for the locked surface, can be AccessMode::ReadOnly, AccessMode::WriteOnly
																///<				or AccessMode::ReadWrite.
																///<
																///< Locking a surface gives the user direct access to the raw pixel data of the surface.
																///< The mode of access can be limited to read-only or write-only, which might yield
																///< performance gains on some platforms as compared to read and write access.
																///<
																///< The first pixel of the first line of raw pixel data starts at the address returned
																///< by this method. The rest of the pixels for the same line follows. There might be some
																///< extra bytes between the end of one line and the beginning of the next which should
																///< neither be read or written. Use the value returned by pitch() to properly calculate
																///< the start adress of each line.
																///<
																///< Once you are done with a surface you should release it by calling unlock() after
																///< which no access of the raw pixel data is allowed until the surface has been locked again.
																///< Be aware that the address and pitch of the raw pixel data might change between calls to
																///< lock().
																///<
																///< You should not use the surface as source or destination in any GfxDevice or
																///< Surface::copyTo() calls while it
																///< is locked as that might result in undefined behavior on certain Surface implementations
																///< and hardware architectures.
																///<
																///< @return Pointer to start of pixel data or null if failed.

		virtual uint8_t *	lockRegion( AccessMode mode, const RectI& region ) = 0; ///< @brief Lock a limited region of the surface for direct access to pixel data.
																///<
																///< Locks a limited region of the surface to gain direct access to its pixel data.
																///<
																///< @param mode	AccessMode for the locked surface, can be AccessMode::ReadOnly, AccessMode::WriteOnly
																///<				or AccessMode::ReadWrite.
																///< @param region	The rectangular region of the surface to lock.
																///<
																///< Identical to lock() except that just a specific region of the surface may be accessed
																///< and that the pointer returned is to the first pixel of that region, not the entire surface.
																///<
																///< Use pitch() to get the distance from the start of one pixel row of the region to the next.
																///< Use unlock() to release the surface when you are done with it.
																///<
																///< Just locking a specific region of a surface as opposed to the whole of it might yield
																///< performance gains depending on Surface implementation and hardware architecture in use.
																///<
																///< @return Pointer to start of pixel data for specified region or null if failed.

		virtual void		unlock() = 0;						///< @brief Unlock a locked surface.
																///<
																///< Unlocks a surface that has previously been locked by a call to lock() or lockRegion().

		inline 	bool		isLocked() const { return (m_accessMode != AccessMode::None); }	///< @brief Check if surface is locked.
																							///<
																							///< Check if surface is locked.
																							///< @return True if surface is locked, otherwise false.

		inline	AccessMode 	lockStatus() const { return m_accessMode; }						///< @brief Check if surface is locked and in what way.
																							///<
																							///< Check if surface is locked and if so, what kind of access is allowed.
																							///< @return AccessMode::ReadOnly, AccessMode::WriteOnly or AccessMode::ReadWrite if the surface is locked,
																							///< otherwise AccessMode::None.

		inline  RectI		regionLocked() const;											///< @brief Get the locked region of the surface.
		inline  int			pitch() const;													///< @brief Get the pitch of the locked region.
		inline uint8_t *	pixels() const { return m_pPixels; }			///< @brief Get a pointer to the raw pixels of the locked region.
																			///< Get a pointer to the first line of raw pixels of the locked region.
																			///<
																			///< The pointer returned is identical to the one previously returned
																			///< by the call to lock() or lockRegion().
																			///< @return Pointer to the raw pixels of the locked region or
																			///<		 null if surface is not locked.

		//.____  Rendering ____________________________________________________

		virtual bool		fill( Color col );						///< @brief Fill surface with specified color.
		virtual bool		fill( Color col, const RectI& region );	///< @brief Fill section of surface with specified color
		virtual bool		copyFrom( Surface * pSrcSurf, const RectI& srcRect, CoordI dst );	///< @brief Copy block of graphics from other surface
		virtual bool		copyFrom( Surface * pSrcSurf, CoordI dst );	///< @brief Copy other surface as a block


	protected:
		Surface();
		virtual ~Surface();

		static const uint8_t *	s_pixelConvTabs[9];

		RectI				_lockAndAdjustRegion( AccessMode modeNeeded, const RectI& region );
		bool 				_copyFrom( const PixelDescription * pSrcFormat, uint8_t * pSrcPixels, int srcPitch, const RectI& srcRect, const RectI& dstRect, const Color * pCLUT = nullptr );

		PixelDescription	m_pixelDescription;
		int					m_pitch;
		SizeI				m_size;				// Width and height in pixels.

		ScaleMode			m_scaleMode;
		bool				m_bMipmapped;

		AccessMode			m_accessMode;
		Color *				m_pClut;			// Pointer at color lookup table. Always 256 entries long.
		uint8_t *			m_pPixels;			// Pointer at pixels when surface locked.
		RectI				m_lockRegion;		// Region of surface that is locked. Width/Height should be set to 0 when not locked.

	};

	//____ Surface::pitch() _______________________________________________
	/**
	 * Get the pitch of the locked region.
	 *
	 * The pitch is the distance in bytes from the start of one line of raw pixels to the start of the next one.
	 * This value must per definition be at least (bytes per raw pixel * width of locked region), but might be
	 * higher if not the whole surface is locked or if the pixel lines are padded.
	 *
	 * Please note that pitch can be negative. This allows us to handle surfaces that for technical reasons are
	 * upside-down internally in a way that is transparent to the user.
	 *
	 * @return Pitch of a pixel line of the locked region, measured in bytes, or 0 if the surface is not locked.
	 *
	 **/
	int Surface::pitch() const
	{
		if( m_accessMode == AccessMode::None )
			return 0;

		return m_pitch;
	}

	//____ pixel() ______________________________________________________________
	/**
	 * Get the raw pixel value from the specified coordinate of the surface.
	 *
	 * @param x X-coordinate of pixel to get.
	 * @param y Y-coordinate of pixel to get.
	 *
	 * The coordinate specified must be within the surface boundaries. A coordinate outside
	 * the surface will result in undefined behavior.
	 *
	 * If pixel is less than
	 * 32 bits in size, it is packed into the lower bits of the return value with
	 * the rest of the bits cleared.
	 *
	 * @return Pixel value in surface's native format.
	 **/
	uint32_t Surface::pixel( int x, int y ) const
	{
		return pixel( CoordI(x,y) );
	}

	//____ alpha() ____________________________________________________________
	/**
	 * Get the alpha value from the specified coordinate of the surface.
	 *
	 * @param x X-coordinate of pixel.
	 * @param y Y-coordinate of pixel.
	 *
	 * Gets the alpha value of the pixel at the specified coordinate in the range of 0-255.
	 * The alpha value is by default used for opacity level,
	 * where 0 is a fully transparent pixel and 255 is opaque.
	 * The coordinate specified must be within the surface boundaries. A coordinate outside
	 * the surface will result in undefined behavior.
	 *
	 * @return Alpha value of pixel at coordinate.
	 **/
	uint8_t Surface::alpha( int x, int y ) const
	{
		return alpha( CoordI(x,y) );
	}

	//____ regionLocked() _________________________________________________________
	/**
	 * Get the region that is locked of a locked surface.
	 *
	 * If the surface has been locked using lock(), this corresponds to the whole surface.
	 *
	 * @return The region of the surface that is locked or an empty rectangle
	 * (0,0,0,0) if surface isn't locked.
	 **/
	RectI Surface::regionLocked() const
	{
		if( m_accessMode==AccessMode::None )
			return RectI();
		else
			return m_lockRegion;
	}


	//==============================================================================

} // namespace wg
#endif // WG_SURFACE_DOT_H
