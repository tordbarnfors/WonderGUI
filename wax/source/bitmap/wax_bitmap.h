/*=========================================================================

                             >>> WonderGUI <<<

  This file is part of Tord Bärnfors' WonderGUI UI Toolkit and copyright
  Tord Bärnfors, Sweden [mail: first name AT barnfors DOT c_o_m].

                                -----------

  The WonderGUI UI Toolkit is free software; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

                                -----------

  The WonderGUI UI Toolkit is also available for use in commercial
  closed source projects under a separate license. Interested parties
  should contact Bärnfors Technology AB [www.barnfors.com] for details.

=========================================================================*/
#ifndef WAX_BITMAP_DOT_H
#define	WAX_BITMAP_DOT_H
#pragma once
//==============================================================================

#include <functional>

#include <wax_types.h>
#include <wax_geo.h>
#include <wax_color.h>
#include <wax_pointers.h>
#include <wax_blob.h>
#include <wax_util.h>

namespace wax
{
	class BitmapFactory;

	class Bitmap;
	typedef	StrongPtr<Bitmap>	Bitmap_p;
	typedef	WeakPtr<Bitmap>	Bitmap_wp;


	//____ PixelBuffer ________________________________________________________
	/**
	 * @brief Simple abstract representation of a bitmap image.
	 * 
	 * @author Tord Jansson
	 * @date 26/01/21
	 * 
	 * PixelBuffer is a small, simple class for low-level access to the pixels of a Bitmap in
	 * a way that is independent of the underlying architecture.
	 * 
	 * PixelBuffers can not be created directly and their members are read-only. Only the 
	 * pixels (pointed to by the member pPixels) can be modified.
	 * 
	 * They are created by the Bitmap to which they are connected and either, depending
	 * on type of Bitmap, allocates a buffer for reading/writing pixels from/to the Bitmap
	 * OR provides direct access to the Bitmap pixels.
	 * 
	 * Either way you allocate a PixelBuffer by calling Bitmap::allocPixelBuffer() on the Bitmap
	 * you want to manipulate. This sets up a PixelBuffer that is mapped to a rectangular region
	 * of the Bitmap.
	 * 
	 * If you want to read the Bitmaps' pixels you then call Bitmap::pushPixels() to make the Bitmap 
	 * copy its pixels to the buffer. This step is unnecessary if the underlying architecture 
	 * allows direct access to the pixels of the Bitmap, but harmless 
	 * (just returns immediately) and should always be performed to prepare the buffer for reading.
	 * 
	 * If you will just write pixels, not read, and will fill the buffer with new pixels, you can skip pushPixels()
	 * and possibly gain performance.
	 * 
	 * In order to update the Bitmap with the pixels in the PixelBuffer, call Bitmap::pullPixels() to make
	 * the Bitmap pull in the Pixels from the buffer.
	 * 
	 * Lastly, you need to free the memory reserved by the PixelBuffer by calling Bitmap::freePixelBuffer().
	 * Failing to do so will leak memory since Bitmaps don't keep track of their PixelBuffers and won't delete
	 * them when destroyed.
	 * 
	 * The pixels of a PixelBuffer are not necessarily in the format, color space or byte order of the Bitmap
	 * itself, so always check the format, colorSpace and bigEndian members of the PixelBuffer.
	 *
	 * A PixelBuffer is strongly tied to the Bitmap it was created from and may not be used in any call
	 * to any other Bitmap. So you can not push pixels from one Bitmap into a PixelBuffer and then pull 
	 * them into another as a way of copying content between them. Use Bitmap::copy() for that.
	 */

	struct PixelBuffer
	{
		PixelFormat		format;
		uint8_t*		pixels;
		const Color8*	palette;
		RectI			rect;
		int				pitch;
		ColorSpace		colorSpace;
		bool			bigEndian;
	};

	//____ NinePatch ___________________________________________________________
	/**
	 * @class NinePatch
	 * @author Tord Jansson
	 * @date 26/01/21
	 * 
	 * A struct that specifies a block of graphics and how it shall be copied, what
	 * parts of the block that should be stretched (if needed) and what parts should not.
	 * 
	 * A nine-patch specifies a rectangle of a Bitmap divided into 9 different areas:
	 * 
	 * - the four corners (northwest, northeast, southeast and south west.
	 * - the four sides between the corners (north, east, south and west).
	 * - the center.
	 * 
	 * When the block of grahics is copied to an area that is larger than the block itself,
	 * it is stretched in the following way:
	 * 
	 * - the four corners of the block are copied to the four corners of the destination (no stretching).
	 * - the north and south sides are stretched horizontally to fill the space between their corners.
	 * - the west and east sides are stretched vertically to fill the space between the corners.
	 * - the center is stretched to fill the area between the four sides.
	 * 
	 * Additionally, one extra rectangular area can be specified to not be stretched, this is what is called
	 * the 'rigid part' of the NinePatch. This is more seldom used, but is useful if you for example wants to
	 * create a speech bubble like the ones used in comics. You might want the speech bubble to be able to stretch
	 * to nicely fit the text, but it also has a tail or arrow pointing to the speaker, which should not be stretched.
	 * 
	 * @var block				The rectangular area of the block of pixels to copy.
	 * @var frame				Specifies the size of the frame building up the corners and sides.
	 * @var rigidPartXOfs		Start of the rigid part along the X-axis, from the beginning of the block (block.x).
	 * @var rigidPartXLength	Length of the rigid part along the X-axis.
	 * @var rigidPartXSections	Top, center, bottom or any combination of these, OR:ed together. Specifying what
	 * 							sections should be rigid horizontally.
	 * @var rigidPartYOfs		Start of the rigid part along the Y-axis, from the beginning of the block (block.y).
	 * @var rigidPartYLength	Length of the rigid part along the Y-axis.
	 * @var rigidPartYSections	Left, center, right or any combination of these, OR:ed together. Specifying what
	 * 							sections should be rigid horizontally.
	 * 
	 */

	struct NinePatch
	{
		Rect		block;
		Border		frame;

		pts			rigidPartXOfs = 0;
		pts			rigidPartXLength = 0;
		YSections	rigidPartXSections = YSections::None;

		pts			rigidPartYOfs = 0;
		pts			rigidPartYLength = 0;
		XSections	rigidPartYSections = XSections::None;
	};



	//____ Bitmap ______________________________________________________________
	/**
	 * @brief Simple abstract representation of a bitmap image.
	 *
	 * A Bitmap is a simple bitmap image that can hold graphical elements for
	 * rendering of the GUI such as skin blocks, fonts and icons. Bitmaps normally
	 * acts as the source of graphical copy operations but can also be used as destinations
	 * when using offscreen bitmaps as rendering caches.
	 *
	 *
	 *
	 **/


	class Bitmap : public Object
	{
		friend class Device;
		friend class DeviceGen2;
		friend class Backend;

	public:

		// Puts back the content of a bitmap that lost it, see Blueprint::restorer. What it
		// captures has to be there for as long as the bitmap is, or until dropRestorer().

		typedef std::function<bool( Bitmap * pBitmap )>	Restorer;

		//.____ Blueprint _________________________________________

		struct Blueprint
		{
			bool				bigEndian = (WAX_IS_BIG_ENDIAN == 1);	// Byte order of the pixels. Defaults to the byte order of the system.
			bool				canvas = false;
			ColorSpace			colorSpace = ColorSpace::Undefined;		// Color space of the pixels. Defaults to sRGB.
			Color				fillColor = Color::Undefined;			// What a bitmap created without pixels is filled with. Undefined leaves its content unspecified.
			Finalizer_p			finalizer = nullptr;
			const Color8* 		palette = nullptr;
			int					paletteCapacity = 0;		// Default to paletteSize. Must be >= paletteSize if set.
			int					paletteSize = 0;			// Default to max for format when palette != nullptr.
			bool				dynamic = false;
			PixelFormat			format = PixelFormat::Undefined;
			int					identity = 0;
			bool				mipmap = false;

			// A promise to give the bitmap its content back, which lets a bitmap with a texture
			// on a GPU go without pixels of its own in plain memory. It is called when the texture
			// has been lost, between frames, and writes the content the bitmap is to have with
			// allocPixelBuffer() and pullPixels(), fill() or copy(). True says the content is back.
			// False says nothing was done: the bitmap is then transparent black and says
			// isContentLost() until all of it has been written. declineRestore() is one that
			// does just that, for what is written anew all the time.
			//
			// Such a bitmap is read by reading the texture back, which waits for the GPU, so
			// what is read regularly should be without one. Bitmaps that keep their pixels
			// anyway never call it: all of SoftBitmap, and Alpha_8 and palette based ones with
			// a texture.

			Restorer			restorer = nullptr;
			SampleMethod		sampleMethod = SampleMethod::Undefined;
			int					scale = 0;
			SizeI				size;					// Mandatory. Size in pixels.
			bool				tiling = false;
			Object_p			userData;
		};

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		inline void			setIdentity(int id);
		inline int			identity() const;

		//.____ Geometry _________________________________________________

		inline SizeI		pixelSize() const;						///< @brief Get the size in pixels of the bitmap.
		inline	int			pixelWidth() const;						///< @brief Get the width of the bitmap.
		inline	int			pixelHeight() const;						///< @brief Get the height of the bitmap.

		inline Size			pointSize() const;
		inline pts			pointWidth() const;
		inline pts			pointHeight() const;

		virtual int			scale() const;				// Need to be virtual for PluginBitmap as long as we have setScale().


		//.____ Appearance ____________________________________________________

		inline SampleMethod	sampleMethod() const;
		inline bool			isTiling() const;
		inline bool			isMipmapped() const;

		//.____ Content _______________________________________________________

		virtual int			alpha( CoordSPX coord ) = 0;	///< @brief Get Alpha value of subpixel at specified coordinate.

		inline const Color8* palette() const;
		inline int			paletteSize() const;
		inline int			paletteCapacity() const;

		inline const PixelDescription*	pixelDescription() const; ///< @brief Get the pixel description for the bitmap.
		inline PixelFormat	pixelFormat() const;
		inline int			pixelBits() const;
		inline ColorSpace	colorSpace() const;
		inline bool			isBigEndian() const;

		inline bool			isOpaque() const;				///< @brief Check if bitmap is guaranteed to be entirely opaque.
		inline bool			isDynamic() const;
		inline bool			canBeCanvas() const;				///< @brief Check if bitmap can be used as canvas.

		//.____ Control _______________________________________________________

		inline const PixelBuffer allocPixelBuffer();

		virtual const PixelBuffer allocPixelBuffer(const RectI& rect) = 0;

        inline bool			pushPixels(const PixelBuffer& buffer);
		virtual bool		pushPixels(const PixelBuffer& buffer, const RectI& bufferRect) = 0;

        inline void			pullPixels(const PixelBuffer& buffer, bool bAutoNotify = true );
		virtual void		pullPixels(const PixelBuffer& buffer, const RectI& bufferRect, bool bAutoNotify = true);			// Needs to be overridden!

		virtual void		freePixelBuffer(const PixelBuffer& buffer) = 0;


		//.____  Rendering ____________________________________________________

		virtual bool		fill( Color color );								///< @brief Fill bitmap with specified color.
		virtual bool		fill(const RectI& region, Color color );			///< @brief Fill section of bitmap with specified color
		virtual bool		copy( CoordI dest, Bitmap * pSrcBitmap, const RectI& srcRect );	///< @brief Copy block of graphics from other bitmap
		virtual bool		copy( CoordI dest, Bitmap * pSrcBitmap );		///< @brief Copy other bitmap as a block
		virtual Bitmap_p	convert( const Bitmap::Blueprint& bp, BitmapFactory* pFactory = nullptr);

		//.____ Misc _________________________________________________________

		inline void			setUserData(Object * pUserData);
		inline Object_p		userData() const;

		virtual int			addObserver(const std::function<void(int nRects, const RectI* pRects)>& func);
		virtual bool		removeObserver( int observerId );
		inline	bool		hasObservers() const { return (m_pObserver != nullptr); }

		virtual void		notifyObservers(int nRects, const RectI* pRects);

		Blueprint			blueprint() const;

		// True if what the bitmap held was lost together with the device of a
		// GPU backend: something rendered into a canvas, or the content of a
		// bitmap whose restorer did nothing. The bitmap keeps working and is
		// transparent black. Whoever put the content there needs to do it again,
		// and it goes back to false when all of the bitmap has been written.
		// For a canvas the easiest is to replace the bitmap.

		virtual bool		isContentLost() const { return m_bContentLost; }	// Virtual for PluginBitmap.

		// A restorer that does nothing, for a bitmap whose content is replaced
		// all the time, as that of a stream, or by someone who looks at
		// isContentLost() in their own time.

		static bool			declineRestore( Bitmap * pBitmap );

		// Has a bitmap with a restorer decline from now on, for when what the restorer
		// needs is going away ahead of the bitmap. The bitmap stays one with a restorer.

		void				dropRestorer();

	protected:

		template<class BP>
		Bitmap(const BP& bp, PixelFormat defaultPixelFormat, SampleMethod defaultSampleMethod)
		{
			PixelFormat format = bp.format == PixelFormat::Undefined ? defaultPixelFormat : bp.format;
			SampleMethod method = bp.sampleMethod == SampleMethod::Undefined ? defaultSampleMethod : bp.sampleMethod;

			m_pixelFormat = format;
			m_colorSpace = bp.colorSpace == ColorSpace::Undefined ? ColorSpace::sRGB : bp.colorSpace;
			m_bBigEndian = bp.bigEndian;
			m_pixelDescription = Util::pixelFormatToDescription(format, bp.bigEndian);
			m_pPixelDescription = &m_pixelDescription;

			m_size = bp.size;
			m_scale = bp.scale == 0 ? 64 : bp.scale;
			m_sampleMethod = method;
			m_bTiling = bp.tiling;
			m_bCanvas = bp.canvas;
			m_bDynamic = bp.dynamic;
			m_restorer = bp.restorer;
			m_id = bp.identity;
			m_pUserData = bp.userData;

			if (bp.finalizer)
				setFinalizer(bp.finalizer);

			// Caculate and set palette size and capacity.

			if (m_pPixelDescription->type == PixelType::Index || m_pPixelDescription->type == PixelType::Bitplanes)
			{
				int maxPaletteSize = 1 << m_pPixelDescription->bits;

				m_paletteSize = bp.paletteSize != 0 ? bp.paletteSize : (bp.palette ? maxPaletteSize : 0);

				m_paletteCapacity = std::max(bp.paletteCapacity, m_paletteSize);
				if (m_paletteCapacity == 0)
					m_paletteCapacity = maxPaletteSize;
			}
		}
		virtual ~Bitmap();

		struct Observer
		{
			int id = 0;
			std::function<void(int nRects, const RectI* pRects)>	func;
			Observer* pNext = nullptr;
		};

		int					_alpha(CoordSPX coord, const PixelBuffer& buffer);

        static bool         _isBlueprintValid( const Blueprint& bp, SizeI maxSize);
		
		void				_fixSrcParam( PixelFormat& format, const Color8*& pPalette, int& paletteSize );
		void				_fixSrcParam( const PixelDescription& format, const Color8*& pPalette, int& paletteSize );

		int                 m_id = 0;

 		int					m_scale = 64;

		const PixelDescription*	m_pPixelDescription;	// Points at m_pixelDescription.
		PixelDescription	m_pixelDescription;
		PixelFormat			m_pixelFormat;
		ColorSpace			m_colorSpace = ColorSpace::sRGB;
		bool				m_bBigEndian = (WAX_IS_BIG_ENDIAN == 1);
		SizeI				m_size;								// Width and height in pixels.

		SampleMethod		m_sampleMethod = SampleMethod::Nearest;
		bool				m_bMipmapped = false;
		bool				m_bTiling = false;
		bool				m_bCanvas = false;
		bool				m_bDynamic = false;
		bool				m_bContentLost = false;			// Set by backends, see isContentLost().
		Restorer			m_restorer;
//		bool				m_bOpaque = false;

		Color8 *			m_pPalette = nullptr;					// Pointer at color lookup table. Always 256 entries long.
		int					m_paletteSize = 0;
		int					m_paletteCapacity = 0;
		
		Object_p			m_pUserData;
		Observer *			m_pObserver = nullptr;
	};

	//____ setIdentity() ____________________________________________________________
/**
 * @brief Set the ID of this Bitmap
 *
 * @param id	An integer to be saved in the Bitmap as its ID.
 *
 * The ID is not used internally by Wax and doesn't affect anything. It just
 * provides a simple way for you to tag a Bitmap for later identification.
 */

	void Bitmap::setIdentity(int id)
	{
		m_id = id;
	}

	//____ identity() _______________________________________________________________
	/**
	 * @brief Get the ID of this Bitmap
	 *
	 * @return ID of this Bitmap. Default ID for all bitmaps is 0.
	 *
	 * The ID is not used internally by Wax and doesn't affect anything. It just
	 * provides a simple way for you to tag a Bitmap for later identification.
	 */

	int Bitmap::identity() const
	{
		return m_id;
	}

	//____ pixelSize() _____________________________________________________________
	/**
	 * Get the width and height of the bitmap in a SizeI structure.
	 *
	 * @return Size of the suface measured in pixels.
	 */
	SizeI Bitmap::pixelSize() const
	{
		return m_size;
	}

	//____ pixelWidth() ________________________________________________________________
	/**
	 * Get the width of the bitmap.
	 *
	 * @return The width of the bitmap, measured in pixels.
	 **/
	int Bitmap::pixelWidth() const
	{
		return m_size.w;
	}

	//____ pixelHeight() _______________________________________________________________
	/**
	 * Get the height of the bitmap.
	 *
	 * @return The height of the bitmap, measured in pixels.
	 **/
	int Bitmap::pixelHeight() const
	{
		return m_size.h;
	}

	//____ pointSize() _____________________________________________________________
	/**
	 * Get the width and height of the bitmap in a Size structure.
	 *
	 * @return Size of the suface measured in points.
	 */
	Size Bitmap::pointSize() const
	{
		return Size(m_size*64)/m_scale;
	}

	//____ pointWidth() ________________________________________________________________
	/**
	 * Get the width of the bitmap.
	 *
	 * @return The width of the bitmap, measured in points.
	 **/
	pts Bitmap::pointWidth() const
	{
		return pts(m_size.w*64)/m_scale;
	}

	//____ pointHeight() _______________________________________________________________
	/**
	 * Get the height of the bitmap.
	 *
	 * @return The height of the bitmap, measured in points.
	 **/
	pts Bitmap::pointHeight() const
	{
		return pts(m_size.h*64)/m_scale;
	}

	//____ sampleMethod() ________________________________________________________

	SampleMethod Bitmap::sampleMethod() const
	{
		return m_sampleMethod;
	}

	//____ isTiling() _________________________________________________________

	bool Bitmap::isTiling() const 
	{ 
		return m_bTiling; 
	}

	//____ isMipmapped() ______________________________________________________

	bool Bitmap::isMipmapped() const
	{
		return m_bMipmapped;
	}

	//____ isOpaque() _________________________________________________________

	bool Bitmap::isOpaque() const
	{
		//TODO: Indexed can also be opaque. Check their alpha on init instead?

		return 	m_pPixelDescription->A_mask == 0 && m_pPixelDescription->type == PixelType::Chunky;
	}

	//____ isDynamic() ___________________________________________________________

	bool Bitmap::isDynamic() const
	{
		return m_bDynamic;
	}

	//____ canBeCanvas() ______________________________________________________

	bool Bitmap::canBeCanvas() const
	{
		return m_bCanvas;
	}

	//____ palette() _____________________________________________________________

	const Color8* Bitmap::palette() const
	{
		return m_pPalette;
	}

	//____ paletteSize() _____________________________________________________________

	int Bitmap::paletteSize() const
	{
		return m_paletteSize;
	}

	//____ paletteCapacity() _____________________________________________________________

	int Bitmap::paletteCapacity() const
	{
		return m_paletteCapacity;
	}

	//____ pixelFormat() ______________________________________________________

	PixelFormat Bitmap::pixelFormat() const
	{
		return m_pixelFormat;
	}

	//____ pixelBits() ______________________________________________________

	int Bitmap::pixelBits() const
	{
		return m_pPixelDescription->bits;
	}

	//____ colorSpace() ______________________________________________________

	ColorSpace Bitmap::colorSpace() const
	{
		return m_colorSpace;
	}

	//____ isBigEndian() ______________________________________________________

	bool Bitmap::isBigEndian() const
	{
		return m_bBigEndian;
	}


	//____ pixelDescription() _________________________________________________

	const PixelDescription* Bitmap::pixelDescription() const
	{
		return m_pPixelDescription;
	}

	//____ allocPixelBuffer() __________________________________________________
	/**
	 * @brief 	Allocate a buffer to access the pixels of the full bitmap
	 * @return 	A PixelBuffer providing info about the buffer, as well as working as a handle for
	 * 			future references.
	 * 
	 * Allocates a buffer that provides acces to the pixels of the full bitmap.
	 * 
	 * The buffer should be expected to be uninitialized, but might already contain the
	 * content of the Bitmap depending on Bitmap implementation. For example is the
	 * buffer of a SoftBitmap just a pointer into the pixels of the bitmap, providing
	 * direct access, while the OpenGL and Metal bitmaps are uninitialized since we don't
	 * have direct access to the texture pixels.
	 * 
	 * To fill the buffer with the content of the bitmap, call pushPixels(). To copy the
	 * content of the buffer to the bitmap, use pullPixels(). These calls will do the actual
	 * pixel copying for an OpenGL and Metal bitmap, while just immediately return for a SoftBitmap.
	 * 
	 * Once you are done with your buffer, call freePixelBuffer() to return the memory allocated.
	 * 
	 */
	
	const PixelBuffer Bitmap::allocPixelBuffer() 
	{ 
		return allocPixelBuffer(m_size); 
	}

	//____ pushPixels() _____________________________________________________
	/**
	 * @brief Copy pixels from Bitmap to PixelBuffer.
	 * 
	 * Copy pixels from Bitmap to buffer.
	 * 
	 * @param buffer		PixelBuffer struct describing the buffer to copy pixels to.
	 * @return 				True if operation could be performed.
	 * 
	 * Fills the buffer with the pixels from the bitmap for the area the buffer maps.
	 */

    bool Bitmap::pushPixels(const PixelBuffer& buffer)
	{ 
		return pushPixels(buffer, buffer.rect.size()); 
	}

	//____ pullPixels() ______________________________________________________
	/**
	 * @brief Copy pixels from PixelBuffer to Bitmap.
	 * 
	 * Copy pixels from buffer to Bitmap.
	 * 
	 * @param buffer		PixelBuffer struct describing the buffer containg the pixels to copy.
	 * @return 				True if operation could be performed.
	 * 
	 * Updates the area of the Bitmap that the PixelBuffer maps to with the pixels from the buffer.
	 */

    void Bitmap::pullPixels(const PixelBuffer& buffer, bool bAutoNotify)
	{
		 pullPixels(buffer, buffer.rect.size(), bAutoNotify);
	}

	//____ setUserData() _______________________________________________________
	/**
	 * @brief Attach an Object as user data.
	 * 
	 * @param pUserData Pointer at the object to become the user data.
	 * 
	 * A Bitmap can "carry around" one Object as its user data. Typical user data would
	 * be a DataSet or Blob containing meta data about the Bitmap that might be
	 * relevant to your application.
	 * 
	 * The user data is irrelevant to and ignored by Wax.
	 */

	void Bitmap::setUserData(Object * pUserData)
	{
		m_pUserData = pUserData;
	}


	//_____ userData() _________________________________________________________
	/**
	 * @brief Get the user data for this Bitmap.
	 * 
	 * @return The Object assigned as user data or nullptr if none.
	 * 
	 * A Bitmap can "carry around" one Object as its user data. Typical user data would
	 * be a DataSet or Blob containing meta data about the Bitmap that might be
	 * relevant to your application.
	 * 
	 * The user data is irrelevant to and ignored by Wax.
	 * 
	 */

	Object_p Bitmap::userData() const
	{
		return m_pUserData;
	}



	//==============================================================================

} // namespace wax
#endif // WAX_BITMAP_DOT_H
