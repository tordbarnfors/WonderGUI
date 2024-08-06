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

#include <wg_gfxtypes.h>
#include <wg_geo.h>
#include <wg_color.h>
#include <wg_pointers.h>
#include <wg_blob.h>
#include <wg_gfxutil.h>

namespace wg
{
	class SurfaceFactory;

	class Surface;
	typedef	StrongPtr<Surface>	Surface_p;
	typedef	WeakPtr<Surface>	Surface_wp;


	//____ PixelBuffer ________________________________________________________
	/**
	 * @brief Simple abstract representation of a bitmap image.
	 * 
	 * @author Tord Jansson
	 * @date 26/01/21
	 * 
	 * PixelBuffer is a small, simple class for low-level access to the pixels of a Surface in
	 * a way that is independent of the underlying architecture.
	 * 
	 * PixelBuffers can not be created directly and their members are read-only. Only the 
	 * pixels (pointed to by the member pPixels) can be modified.
	 * 
	 * They are created by the Surface to which they are connected and either, depending
	 * on type of Surface, allocates a buffer for reading/writing pixels from/to the Surface
	 * OR provides direct access to the Surface pixels.
	 * 
	 * Either way you allocate a PixelBuffer by calling Surface::allocPixelBuffer() on the Surface
	 * you want to manipulate. This sets up a PixelBuffer that is mapped to a rectangular region
	 * of the Surface.
	 * 
	 * If you want to read the Surfaces' pixels you then call Surface::pushPixels() to make the Surface 
	 * copy its pixels to the buffer. This step is unnecessary if the underlying architecture 
	 * allows direct access to the pixels of the Surface, but harmless 
	 * (just returns immediately) and should always be performed to prepare the buffer for reading.
	 * 
	 * If you will just write pixels, not read, and will fill the buffer with new pixels, you can skip pushPixels()
	 * and possibly gain performance.
	 * 
	 * In order to update the Surface with the pixels in the PixelBuffer, call Surface::pullPixels() to make
	 * the Surface pull in the Pixels from the buffer.
	 * 
	 * Lastly, you need to free the memory reserved by the PixelBuffer by calling Surface::freePixelBuffer().
	 * Failing to do so will leak memory since Surfaces don't keep track of their PixelBuffers and won't delete
	 * them when destroyed.
	 * 
	 * A PixelBuffer is strongly tied to the Surface it was created from and may not be used in any call
	 * to any other Surface. So you can not push pixels from one Surface into a PixelBuffer and then pull 
	 * them into another as a way of copying content between them. Use Surface::copy() for that.
	 */

	struct PixelBuffer
	{
		PixelFormat		format;
		uint8_t*		pixels;
		const Color8*	palette;
		RectI			rect;
		int				pitch;
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
	 * A nine-patch specifies a rectangle of a Surface divided into 9 different areas:
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
	 * create speech bubble like the ones used in comics. You might want the speech bubble to be able to stretch
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
		friend class GfxDeviceGen1;
		friend class GfxDeviceGen2;

	public:

		//.____ Blueprint _________________________________________

		struct Blueprint
		{
			Object_p			baggage;
			bool				buffered = false;
			bool				canvas = false;
			Finalizer_p			finalizer = nullptr;
			const Color8* 		palette = nullptr;
			int					paletteCapacity = 0;		// Default to paletteSize. Must be >= paletteSize if set.
			int					paletteSize = 0;			// Default to max for format when palette != nullptr.
			bool				dynamic = false;
			PixelFormat			format = PixelFormat::Undefined;
			int					identity = 0;
			bool				mipmap = false;
			SampleMethod		sampleMethod = SampleMethod::Undefined;
			int					scale = 0;
			SizeI				size;					// Mandatory.
			bool				tiling = false;
		};

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		inline void			setIdentity(int id);
		inline int			identity() const;

		//.____ Geometry _________________________________________________

		inline SizeI		pixelSize() const;						///< @brief Get the size in pixels of the surface.
		inline	int			pixelWidth() const;						///< @brief Get the width of the surface.
		inline	int			pixelHeight() const;						///< @brief Get the height of the surface.

		inline Size			pointSize() const;
		inline pts			pointWidth() const;
		inline pts			pointHeight() const;

		virtual int			scale() const;				// Need to be virtual for PluginSurface as long as we have setScale().


		//.____ Appearance ____________________________________________________

		inline SampleMethod	sampleMethod() const;
		inline bool			isTiling() const;
		inline bool			isMipmapped() const;

		//.____ Content _______________________________________________________

		virtual int			alpha( CoordSPX coord ) = 0;	///< @brief Get Alpha value of subpixel at specified coordinate.

		inline const Color8* palette() const;
		inline int			paletteSize() const;
		inline int			paletteCapacity() const;

		inline const PixelDescription*	pixelDescription() const; ///< @brief Get the pixel description for the surface.
		inline PixelFormat	pixelFormat() const;
		inline int			pixelBits() const;

		inline bool			isOpaque() const;				///< @brief Check if surface is guaranteed to be entirely opaque.
		inline bool			canBeCanvas() const;				///< @brief Check if surface can be used as canvas.
		
		//.____ Control _______________________________________________________

		inline const PixelBuffer allocPixelBuffer();

		virtual const PixelBuffer allocPixelBuffer(const RectI& rect) = 0;

        inline bool			pushPixels(const PixelBuffer& buffer);
		virtual bool		pushPixels(const PixelBuffer& buffer, const RectI& bufferRect) = 0;

        inline void			pullPixels(const PixelBuffer& buffer);
		virtual void		pullPixels(const PixelBuffer& buffer, const RectI& bufferRect);			// Needs to be overridden!

		virtual void		freePixelBuffer(const PixelBuffer& buffer) = 0;


		//.____  Rendering ____________________________________________________

		virtual bool		fill( HiColor color );								///< @brief Fill surface with specified color.
		virtual bool		fill(const RectI& region, HiColor color );			///< @brief Fill section of surface with specified color
		virtual bool		copy( CoordI dest, Surface * pSrcSurf, const RectI& srcRect );	///< @brief Copy block of graphics from other surface
		virtual bool		copy( CoordI dest, Surface * pSrcSurf );		///< @brief Copy other surface as a block
		virtual Surface_p	convert( const Surface::Blueprint& bp, SurfaceFactory* pFactory = nullptr);

		//.____ Misc _________________________________________________________

		inline void			setBaggage(Object * pBaggage);
		inline Object_p		baggage() const;

		virtual int			addObserver(const std::function<void(int nRects, const RectSPX* pRects)>& func);
		virtual bool		removeObserver( int observerId );

		Blueprint			blueprint() const;

	protected:

		template<class BP>
		Surface(const BP& bp, PixelFormat defaultPixelFormat, SampleMethod defaultSampleMethod)
		{
			PixelFormat format = bp.format == PixelFormat::Undefined ? defaultPixelFormat : bp.format;
			SampleMethod method = bp.sampleMethod == SampleMethod::Undefined ? defaultSampleMethod : bp.sampleMethod;

			format = Util::clarifyPixelFormat(format);

			m_pixelFormat = format;
			m_pPixelDescription = &Util::pixelFormatToDescription(format);

			m_size = bp.size;
			m_scale = bp.scale == 0 ? 64 : bp.scale;
			m_sampleMethod = method;
			m_bTiling = bp.tiling;
			m_bCanvas = bp.canvas;
			m_bBuffered = bp.buffered;
			m_bDynamic = bp.dynamic;
			m_id = bp.identity;
			m_pBaggage = bp.baggage;

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
		virtual ~Surface();

		struct Observer
		{
			int id = 0;
			std::function<void(int nRects, const RectSPX* pRects)>	func;
			Observer* pNext = nullptr;
		};

		static const uint8_t *	s_pixelConvTabs[9];

		void				_notifyObservers(int nRects, const RectSPX* pRects);
		int					_alpha(CoordSPX coord, const PixelBuffer& buffer);

        static bool         _isBlueprintValid( const Blueprint& bp, SizeI maxSize);
		
		void				_fixSrcParam( PixelFormat& format, const Color8*& pPalette, int& paletteSize );
		void				_fixSrcParam( const PixelDescription& format, const Color8*& pPalette, int& paletteSize );

		int                 m_id = 0;

 		int					m_scale = 64;

		const PixelDescription*	m_pPixelDescription;
		PixelFormat			m_pixelFormat;
		SizeI				m_size;								// Width and height in pixels.

		SampleMethod		m_sampleMethod = SampleMethod::Nearest;
		bool				m_bMipmapped = false;
		bool				m_bTiling = false;
        bool                m_bCanvas = false;
		bool				m_bBuffered = false;
		bool				m_bDynamic = false;
//		bool				m_bOpaque = false;

		Color8 *			m_pPalette = nullptr;					// Pointer at color lookup table. Always 256 entries long.
		int					m_paletteSize = 0;
		int					m_paletteCapacity = 0;
		
		Object_p			m_pBaggage;
		Observer *			m_pObserver = nullptr;
	};

	//____ setIdentity() ____________________________________________________________
/**
 * @brief Set the ID of this Surface
 *
 * @param id	An integer to be saved in the Surface as its ID.
 *
 * The ID is not used internally by WonderGUI and doesn't affect anything. It just
 * provides a simple way for you to tag a Surface for later identification.
 */

	void Surface::setIdentity(int id)
	{
		m_id = id;
	}

	//____ identity() _______________________________________________________________
	/**
	 * @brief Get the ID of this Surface
	 *
	 * @return ID of this Surface. Default ID for all surfaces is 0.
	 *
	 * The ID is not used internally by WonderGUI and doesn't affect anything. It just
	 * provides a simple way for you to tag a Surface for later identification.
	 */

	int Surface::identity() const
	{
		return m_id;
	}

	//____ pixelSize() _____________________________________________________________
	/**
	 * Get the width and height of the surface in a SizeI structure.
	 *
	 * @return Size of the suface measured in pixels.
	 */
	SizeI Surface::pixelSize() const
	{
		return m_size;
	}

	//____ pixelWidth() ________________________________________________________________
	/**
	 * Get the width of the surface.
	 *
	 * @return The width of the surface, measured in pixels.
	 **/
	int Surface::pixelWidth() const
	{
		return m_size.w;
	}

	//____ pixelHeight() _______________________________________________________________
	/**
	 * Get the height of the surface.
	 *
	 * @return The height of the surface, measured in pixels.
	 **/
	int Surface::pixelHeight() const
	{
		return m_size.h;
	}

	//____ pointSize() _____________________________________________________________
	/**
	 * Get the width and height of the surface in a Size structure.
	 *
	 * @return Size of the suface measured in points.
	 */
	Size Surface::pointSize() const
	{
		return Size(m_size*64)/m_scale;
	}

	//____ pointWidth() ________________________________________________________________
	/**
	 * Get the width of the surface.
	 *
	 * @return The width of the surface, measured in points.
	 **/
	pts Surface::pointWidth() const
	{
		return pts(m_size.w*64)/m_scale;
	}

	//____ pointHeight() _______________________________________________________________
	/**
	 * Get the height of the surface.
	 *
	 * @return The height of the surface, measured in points.
	 **/
	pts Surface::pointHeight() const
	{
		return pts(m_size.h*64)/m_scale;
	}

	//____ sampleMethod() ________________________________________________________

	SampleMethod Surface::sampleMethod() const
	{
		return m_sampleMethod;
	}

	//____ isTiling() _________________________________________________________

	bool Surface::isTiling() const 
	{ 
		return m_bTiling; 
	}

	//____ isMipmapped() ______________________________________________________

	bool Surface::isMipmapped() const
	{
		return m_bMipmapped;
	}

	//____ isOpaque() _________________________________________________________

	bool Surface::isOpaque() const
	{
		//TODO: Indexed can also be opaque. Check their alpha on init instead?

		return 	m_pPixelDescription->A_mask == 0 && (m_pPixelDescription->type == PixelType::Chunky ||
		m_pPixelDescription->type == PixelType::Chunky_BE );
	}

	//____ canBeCanvas() ______________________________________________________

	bool Surface::canBeCanvas() const
	{
		return m_bCanvas;
	}

	//____ palette() _____________________________________________________________

	const Color8* Surface::palette() const
	{
		return m_pPalette;
	}

	//____ paletteSize() _____________________________________________________________

	int Surface::paletteSize() const
	{
		return m_paletteSize;
	}

	//____ paletteCapacity() _____________________________________________________________

	int Surface::paletteCapacity() const
	{
		return m_paletteCapacity;
	}

	//____ pixelFormat() ______________________________________________________

	PixelFormat Surface::pixelFormat() const
	{
		return m_pixelFormat;
	}

	//____ pixelBits() ______________________________________________________

	int Surface::pixelBits() const
	{
		return m_pPixelDescription->bits;
	}


	//____ pixelDescription() _________________________________________________

	const PixelDescription* Surface::pixelDescription() const
	{
		return m_pPixelDescription;
	}

	//____ allocPixelBuffer() __________________________________________________
	/**
	 * @brief 	Allocate a buffer to access the pixels of the full surface
	 * @return 	A PixelBuffer providing info about the buffer, as well as working as a handle for
	 * 			future references.
	 * 
	 * Allocates a buffer that provides acces to the pixels of the full surface.
	 * 
	 * The buffer should be expected to be uninitialized, but might already contain the
	 * content of the Surface depending on Surface implementation. For example is the
	 * buffer of a SoftSurface just a pointer into the pixels of the surface, providing
	 * direct access, while the OpenGL and Metal surfaces are uninitialized since we don't
	 * have direct access to the texture pixels.
	 * 
	 * To fill the buffer with the content of the surface, call pushPixels(). To copy the
	 * content of the buffer to the surface, use pullPixels(). These calls will do the actual
	 * pixel copying for an OpenGL and Metal surface, while just immediately return for a SoftSurface.
	 * 
	 * Once you are done with your buffer, call freePixelBuffer() to return the memory allocated.
	 * 
	 */
	
	const PixelBuffer Surface::allocPixelBuffer() 
	{ 
		return allocPixelBuffer(m_size); 
	}

	//____ pushPixels() _____________________________________________________
	/**
	 * @brief Copy pixels from Surface to PixelBuffer.
	 * 
	 * Copy pixels from Surface to buffer.
	 * 
	 * @param buffer		PixelBuffer struct describing the buffer to copy pixels to.
	 * @return 				True if operation could be performed.
	 * 
	 * Fills the buffer with the pixels from the surface for the area the buffer maps.
	 */

    bool Surface::pushPixels(const PixelBuffer& buffer)
	{ 
		return pushPixels(buffer, buffer.rect.size()); 
	}

	//____ pullPixels() ______________________________________________________
	/**
	 * @brief Copy pixels from PixelBuffer to Surface.
	 * 
	 * Copy pixels from buffer to Surface.
	 * 
	 * @param buffer		PixelBuffer struct describing the buffer containg the pixels to copy.
	 * @return 				True if operation could be performed.
	 * 
	 * Updates the area of the Surface that the PixelBuffer maps to with the pixels from the buffer.
	 */

    void Surface::pullPixels(const PixelBuffer& buffer)
	{ 
		 pullPixels(buffer, buffer.rect.size()); 
	}

	//____ setBaggage() _______________________________________________________
	/**
	 * @brief Attach an Object as baggage.
	 * 
	 * @param pBaggage Pointer at the object to become the baggage.
	 * 
	 * A Surface can "carry around" one Object as its baggage. Typical baggage would
	 * be a DataSet or Blob containing meta data about the Surface that might be
	 * relevant to your application.
	 * 
	 * The baggage is irrelevant to and ignored by WonderGUI.
	 */

	void Surface::setBaggage(Object * pBaggage)
	{
		m_pBaggage = pBaggage;
	}


	//_____ baggage() _________________________________________________________
	/**
	 * @brief Get the baggage for this Surface.
	 * 
	 * @return The Object assigned as baggage or nullptr if none.
	 * 
	 * A Surface can "carry around" one Object as its baggage. Typical baggage would
	 * be a DataSet or Blob containing meta data about the Surface that might be
	 * relevant to your application.
	 * 
	 * The baggage is irrelevant to and ignored by WonderGUI.
	 * 
	 */

	Object_p Surface::baggage() const
	{
		return m_pBaggage;
	}



	//==============================================================================

} // namespace wg
#endif // WG_SURFACE_DOT_H
