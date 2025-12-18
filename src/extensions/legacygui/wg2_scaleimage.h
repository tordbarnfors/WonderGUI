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
#ifndef WG2_SCALEIMAGE_DOT_H
#define WG2_SCALEIMAGE_DOT_H

#include <assert.h>

#ifndef WG2_WIDGET_DOT_H
#	include <wg2_widget.h>
#endif

#include <wg_surface.h>
#include <wg_surfacefactory.h>


#include <vector>

//____ WgScaleImage __________________________________________________________

class WgScaleImage : public WgWidget
{
public:
	WgScaleImage();
	virtual ~WgScaleImage();

	const char *Type( void ) const override;
	static const char * GetClass();
	WgWidget * NewOfMyType() const override{ return new WgScaleImage(); };

	void			SetSurfaceFactory( wg::SurfaceFactory * pFactory );
	wg::SurfaceFactory_p GetSurfaceFactory() const { return m_pSurfaceFactory; }

	void			SetSource( wg::Surface * pSurf );
	wg::Surface_p   GetSource() const { return m_pOrgSurface; }

	void			SetImageScale( bool bScale );
	inline bool		GetImageScale() const { return m_bScale; }

	void			SetImageOrigo( WgOrigo origo );
	inline WgOrigo	GetImageOrigo() const { return m_imgOrigo; }

	void			SetImageMaxSize( WgSize max );



	WgSize			PreferredPixelSize() const override;

	void resample(wg::Surface* src, wg::Surface* dest)
	{
        auto srcpixbuf = src->allocPixelBuffer();
        auto destpixbuf = dest->allocPixelBuffer();

        src->pushPixels(srcpixbuf);
        
        unsigned char* pSrc = (unsigned char*)srcpixbuf.pixels;
        unsigned char* pDest = (unsigned char*)destpixbuf.pixels;

		bool quit = false;
		Resize_HQ_4ch(pSrc,
                      srcpixbuf.pitch/4,
                      srcpixbuf.rect.h,
                      pDest,
                      destpixbuf.pitch/4,
                      destpixbuf.rect.h,
					  &quit );

        dest->pullPixels(destpixbuf);
        
        src->freePixelBuffer(srcpixbuf);
        dest->freePixelBuffer(destpixbuf);
	}


	int min(int a, int b)
	{
		return (a<b)?a:b;
	}

	/*
	Source Code for Smooth Image Resampling (Resizing) in C/C++ (Visual Studio)
	 by Ryan Geiss - 3 July 2008


	 The code below performs a fairly-well-optimized high-quality resample
	 (smooth resize) of a 3-channel image that is padded to 4 bytes per
	 pixel.  The pixel format is assumed to be ARGB.  If you want to make
	 it handle an alpha channel, the changes should be very straightforward.

	 In general, if the image is being enlarged, bilinear interpolation
	 is used; if the image is being downsized, all input pixels are weighed
	 appropriately to produce the correct result.

	 In order to be efficient, it actually performs 1 of 4 routines.  First,
	 if you are cutting the image size *exactly* in half (common when generating
	 mipmap levels), it will use a specialized routine to do just that.  There
	 are actually two versions of this routine - an MMX one and a non-MMX one.
	 It detects if MMX is present and chooses the right one.

	 If you're not cutting the image perfectly in half, it executes one
	 of two general resize routines.  If upsampling (increasing width and height)
	 on both X and Y, then it executes a faster resize algorithm that just performs
	 a 2x2 bilinear interpolation of the appropriate input pixels, for each output
	 pixel.

	 If downsampling on either X or Y (or both), though, the general-purpose
	 routine gets run.  It iterates over every output pixel, and for each one, it
	 iterates over the input pixels that map to that output pixel [there will
	 usually be more than 1 in this case].  Each input pixel is properly weighted
	 to produce exactly the right image.  There's a little bit of extra bookkeeping,
	 but in general, it's pretty efficient.

	 Note that on extreme downsizing (2,800 x 2,800 -> 1x1 or greater ratio),
	 the colors can overflow.  If you want to fix this lazily, just break
	 your resize into two passes.

	 Also note that when your program exits, or when you are done using this
	 function, you should delete [] g_px1a and g_px1ab if they have been
	 allocated.

	 I posted this here because this is pretty common code that is a bit of
	 a pain to write; I've written it several times over the years, and I really
	 don't feel like writing it again.  So - here it is - for my reference, and
	 for yours.  Enjoy!

	 */

	typedef unsigned int DWORD;

	bool CheckForMMX()
	{
		return false;
		/*
		DWORD bMMX = 0;
		DWORD *pbMMX = &bMMX;
		__try {
			__asm {
				mov eax, 1
				cpuid
				mov edi, pbMMX
				mov DWORD ptr [edi], edx
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER) {
			bMMX = 0;
		}
		return ((bMMX & 0x00800000) != 0);  // check bit 23
		 */
	}


	void Resize_HQ_4ch( unsigned char* src, int w1, int h1,
					   unsigned char* dest, int w2, int h2,
					   bool* pQuitFlag)
	{

		int* g_px1a    = NULL;
		int  g_px1a_w  = 0;
		int* g_px1ab   = NULL;
		int  g_px1ab_w = 0;
		bool g_bMMX = false;
		bool g_bMMX_known = false;

		// Both buffers must be in ARGB format, and a scanline should be w*4 bytes.

		// If pQuitFlag is non-NULL, then at the end of each scanline, it will check
		//    the value at *pQuitFlag; if it's set to 'true', this function will abort.
		// (This is handy if you're background-loading an image, and decide to cancel.)

		// NOTE: THIS WILL OVERFLOW for really major downsizing (2800x2800 to 1x1 or more)
		// (2800 ~ sqrt(2^23)) - for a lazy fix, just call this in two passes.

		assert(src);
		assert(dest);
		assert(w1 >= 1);
		assert(h1 >= 1);
		assert(w2 >= 1);
		assert(h2 >= 1);

		// check for MMX (one time only)
		if (!g_bMMX_known)
		{
			g_bMMX = CheckForMMX();
			g_bMMX_known = true;
		}


		// NO: This code has NOT been tested with WonderGUI, so let's not use it.

	 /*   if (w2*2==w1 && h2*2==h1)
		{
			// perfect 2x2:1 case - faster code
			// (especially important because this is common for generating low (large) mip levels!)
			DWORD *dsrc  = (DWORD*)src;
			DWORD *ddest = (DWORD*)dest;

			if (g_bMMX==1)
			{
				// MMX LOOP - about 32% faster
				// PREFETCH: no (...tests determined it was actually SLOWER)
				// MASS PRESERVING (~remainders): *NO*

				__m64 zero;
				zero.m64_i32[0] = 0;
				zero.m64_i32[1] = 0;

				int i = 0;
				for (int y2=0; y2<h2; y2++)
				{
					int y1 = y2*2;
					DWORD* temp_src = &dsrc[y1*w1];
					for (int x2=0; x2<w2; x2++)
					{
						__m64 a = *(__m64*)(&temp_src[ 0]);  // ABCDEFGH
						__m64 b = *(__m64*)(&temp_src[w1]);  // IJKLMNOP
						register __m64 c = _mm_unpacklo_pi8(a, zero); // 0E0F0G0H      // PUNPCKLBW
						register __m64 d = _mm_unpacklo_pi8(b, zero); // 0M0N0O0P      // PUNPCKLBW
						c = _mm_add_pi16(c, d);
						d = _mm_unpackhi_pi8(a, zero);
						c = _mm_add_pi16(c, d);
						d = _mm_unpackhi_pi8(b, zero);
						c = _mm_add_pi16(c, d);
						c = _mm_srli_pi16(c, 2);
						c = _mm_packs_pu16(c, c);

						ddest[i++] = c.m64_u32[0];
						temp_src += 2;
					}

					if (pQuitFlag && *pQuitFlag)
						break;
				}
				_mm_empty();    // do this always - just that __m64's existence, above, will tamper w/float stuff.
			}
			else
			{
				// NON-MMX LOOP
				// PREFETCH: no (...tests determined it was actually SLOWER)
				// MASS PRESERVING (~remainders): YES
				DWORD remainder = 0;
				int i = 0;
				for (int y2=0; y2<h2; y2++)
				{
					int y1 = y2*2;
					DWORD* temp_src = &dsrc[y1*w1];
					for (int x2=0; x2<w2; x2++)
					{
						DWORD xUL = temp_src[0];
						DWORD xUR = temp_src[1];
						DWORD xLL = temp_src[w1];
						DWORD xLR = temp_src[w1 + 1];
						// note: DWORD packing is 0xAARRGGBB

						DWORD redblue = (xUL & 0x00FF00FF) + (xUR & 0x00FF00FF) + (xLL & 0x00FF00FF) + (xLR & 0x00FF00FF) + (remainder & 0x00FF00FF);
						DWORD green   = (xUL & 0x0000FF00) + (xUR & 0x0000FF00) + (xLL & 0x0000FF00) + (xLR & 0x0000FF00) + (remainder & 0x0000FF00);
						// redblue = 000000rr rrrrrrrr 000000bb bbbbbbbb
						// green   = xxxxxx00 000000gg gggggggg 00000000
						remainder =  (redblue & 0x00030003) | (green & 0x00000300);
						ddest[i++]   = ((redblue & 0x03FC03FC) | (green & 0x0003FC00)) >> 2;

						temp_src += 2;
					}

					if (pQuitFlag && *pQuitFlag)
						break;
				}
			//}
		}
		else */

		// This has been tested with WonderGUI2 and works with alpha channel
		{
			// arbitrary resize.
			unsigned int *dsrc  = (unsigned int *)src;
			unsigned int *ddest = (unsigned int *)dest;

			bool bUpsampleX = (w1 < w2);
			bool bUpsampleY = (h1 < h2);

			// If too many input pixels map to one output pixel, our 32-bit accumulation values
			// could overflow - so, if we have huge mappings like that, cut down the weights:
			//    256 max color value
			//   *256 weight_x
			//   *256 weight_y
			//   *256 (16*16) maximum # of input pixels (x,y) - unless we cut the weights down...
			int weight_shift = 0;
			float source_texels_per_out_pixel = (   (w1/(float)w2 + 1)
												 * (h1/(float)h2 + 1)
												 );
			float weight_per_pixel = source_texels_per_out_pixel * 256 * 256;  //weight_x * weight_y
			float accum_per_pixel = weight_per_pixel*256; //color value is 0-255
			float weight_div = accum_per_pixel / 4294967000.0f;
			if (weight_div > 1)
				weight_shift = (int)ceilf( logf((float)weight_div)/logf(2.0f) );
			weight_shift = min(15, weight_shift);  // this could go to 15 and still be ok.

			float fh = 256*h1/(float)h2;
			float fw = 256*w1/(float)w2;

			if (bUpsampleX && bUpsampleY)
			{
				// faster to just do 2x2 bilinear interp here

				// cache x1a, x1b for all the columns:
				// ...and your OS better have garbage collection on process exit :)
				if (g_px1a_w < w2)
				{
					if (g_px1a) delete [] g_px1a;
					g_px1a = new int[w2*2 * 1];
					g_px1a_w = w2*2;
				}
				for (int x2=0; x2<w2; x2++)
				{
					// find the x-range of input pixels that will contribute:
					int x1a = (int)(x2*fw);
					x1a = min(x1a, 256*(w1-1) - 1);
					g_px1a[x2] = x1a;
				}

				// FOR EVERY OUTPUT PIXEL
				for (int y2=0; y2<h2; y2++)
				{
					// find the y-range of input pixels that will contribute:
					int y1a = (int)(y2*fh);
					y1a = min(y1a, 256*(h1-1) - 1);
					int y1c = y1a >> 8;

					unsigned int *ddest = &((unsigned int *)dest)[y2*w2 + 0];

					for (int x2=0; x2<w2; x2++)
					{
						// find the x-range of input pixels that will contribute:
						int x1a = g_px1a[x2];//(int)(x2*fw);
						int x1c = x1a >> 8;
						unsigned int *dsrc2 = &dsrc[y1c*w1 + x1c];

						// PERFORM BILINEAR INTERPOLATION on 2x2 pixels
						unsigned int r=0, g=0, b=0, a=0, ww=0;
						unsigned int weight_x = 256 - (x1a & 0xFF);
						unsigned int weight_y = 256 - (y1a & 0xFF);
						for (int y=0; y<2; y++)
						{
							for (int x=0; x<2; x++)
							{
								unsigned int c = dsrc2[x + y*w1];
								unsigned int r_src = (c    ) & 0xFF;
								unsigned int g_src = (c>> 8) & 0xFF;
								unsigned int b_src = (c>>16) & 0xFF;
								unsigned int a_src = (c>>24) & 0xFF;
								unsigned int w = (weight_x * weight_y) >> weight_shift;
								r += r_src * w;
								g += g_src * w;
								b += b_src * w;
								a += a_src * w;
								ww += w;
								weight_x = 256 - weight_x;
							}
							weight_y = 256 - weight_y;
						}

						// write results
						// NO:
						unsigned int c = ((r/ww)) | ((g/ww)<<8) | ((b/ww)<<16) | ((a/ww)<<24);
						// Was: unsigned int c = ((r>>16)) | ((g>>8) & 0xFF00) | (b & 0xFF0000);
						*ddest++ = c;//ddest[y2*w2 + x2] = c;
					}
				}
			}
			else
			{
				// cache x1a, x1b for all the columns:
				// ...and your OS better have garbage collection on process exit :)
				if (g_px1ab_w < w2)
				{
					if (g_px1ab) delete [] g_px1ab;
					g_px1ab = new int[w2*2 * 2];
					g_px1ab_w = w2*2;
				}
				for (int x2=0; x2<w2; x2++)
				{
					// find the x-range of input pixels that will contribute:
					int x1a = (int)((x2  )*fw);
					int x1b = (int)((x2+1)*fw);
					if (bUpsampleX) // map to same pixel -> we want to interpolate between two pixels!
						x1b = x1a + 256;
					x1b = min(x1b, 256*w1 - 1);
					g_px1ab[x2*2+0] = x1a;
					g_px1ab[x2*2+1] = x1b;
				}

				// FOR EVERY OUTPUT PIXEL
				for (int y2=0; y2<h2; y2++)
				{
					// find the y-range of input pixels that will contribute:
					int y1a = (int)((y2  )*fh);
					int y1b = (int)((y2+1)*fh);
					if (bUpsampleY) // map to same pixel -> we want to interpolate between two pixels!
						y1b = y1a + 256;
					y1b = min(y1b, 256*h1 - 1);
					int y1c = y1a >> 8;
					int y1d = y1b >> 8;

					for (int x2=0; x2<w2; x2++)
					{
						// find the x-range of input pixels that will contribute:
						int x1a = g_px1ab[x2*2+0];    // (computed earlier)
						int x1b = g_px1ab[x2*2+1];    // (computed earlier)
						int x1c = x1a >> 8;
						int x1d = x1b >> 8;

						// ADD UP ALL INPUT PIXELS CONTRIBUTING TO THIS OUTPUT PIXEL:
						unsigned int r=0, g=0, b=0, a=0, ww=0;
						for (int y=y1c; y<=y1d; y++)
						{
							unsigned int weight_y = 256;
							if (y1c != y1d)
							{
								if (y==y1c)
									weight_y = 256 - (y1a & 0xFF);
								else if (y==y1d)
									weight_y = (y1b & 0xFF);
							}

							unsigned int *dsrc2 = &dsrc[y*w1 + x1c];
//                            DBGM(DBG_GUI, ("(x,y) = (%d,%d), (w1,h1)=(%d,%d)", x1c, x, w1, h1));

							for (int x=x1c; x<=x1d; x++)
							{
								unsigned int weight_x = 256;
								if (x1c != x1d)
								{
									if (x==x1c)
										weight_x = 256 - (x1a & 0xFF);
									else if (x==x1d)
										weight_x = (x1b & 0xFF);
								}

								unsigned int c = *dsrc2++;//dsrc[y*w1 + x];

								//NO: Check if we're in the padded area:
								if(x >= m_pOrgSurface->pixelSize().w)
									c = 0;

								unsigned int r_src = (c    ) & 0xFF;
								unsigned int g_src = (c>> 8) & 0xFF;
								unsigned int b_src = (c>>16) & 0xFF;
								unsigned int a_src = (c>>24) & 0xFF;
								unsigned int w = (weight_x * weight_y) >> weight_shift;
								r += r_src * w;
								g += g_src * w;
								b += b_src * w;
								a += a_src * w;
								ww += w;
							}
						}

						// write results
						unsigned int c = ((r/ww)) | ((g/ww)<<8) | ((b/ww)<<16) | ((a/ww)<<24);
						*ddest++ = c;//ddest[y2*w2 + x2] = c;
					}

					if (pQuitFlag && *pQuitFlag)
						break;
				}
			}
		}

		if (g_px1ab) delete [] g_px1ab;
		if (g_px1a) delete [] g_px1a;
	}

protected:

	void	_onNewSize( const WgSize& size ) override;
	void	_onCloneContent( const WgWidget * _pOrg ) override;
	void	_onRender( wg::GfxDevice * pDevice, const WgRect& _canvas, const WgRect& _window) override;
	bool	_onAlphaTest( const WgCoord& ofs ) override;

	void	_regenerateSurface();
	void	_recalcImageRect( WgSize widgetSize, bool bRegenerate );

	void    _preRender() override;

	
private:

	wg::Surface_p   m_pOrgSurface;				// Original surface
	wg::Surface_p   m_pGenSurface;				// Generated surface
	bool			m_bScale;					// Set if image should scale, not stretch
	WgSize			m_maxImgSize;
	WgRect			m_imgRect;					// Output rectangle for image within widget.
	WgOrigo			m_imgOrigo;
	wg::SurfaceFactory_p m_pSurfaceFactory;
};


#endif //WG2_SCALEIMAGE_DOT_H
