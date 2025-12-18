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
#include <wg_edgemaptools.h>
#include <wg_gfxbase.h>
#include <wg_gfxutil.h>

using namespace std;

namespace wg {
namespace EdgemapTools {

//____ createDonut() ______________________________________________________

Edgemap_p createDonut(SizeSPX size, spx thickness, HiColor fillColor, spx outlineThickness, HiColor outlineColor, EdgemapFactory * pFactory )
{
	auto pCurveTab = GfxBase::curveTab();
	int curveTabSize = GfxBase::curveTabSize();

	// Center in 26.6 format.

	CoordI center = { size.w / 2, size.h / 2 };

	// Outer rect of elipse rounded to full pixels.

	RectI outerRect;
	outerRect.x = 0;
	outerRect.y = 0;
	outerRect.w = ((size.w + 63) >> 6) - outerRect.x;
	outerRect.h = ((size.h + 63) >> 6) - outerRect.y;

	// Adjusted clip

	RectI clip = outerRect;
	if (clip.w == 0 || clip.h == 0)
		return nullptr;

	int clipLeft = clip.x - outerRect.x;

	// Calculate maximum width and height from center for the 4 edges of the elipse.

	int radiusY[4];
	radiusY[0] = center.y;
	radiusY[1] = (int)(radiusY[0] - outlineThickness);
	radiusY[2] = (int)(radiusY[1] - thickness);
	radiusY[3] = (int)(radiusY[2] - outlineThickness);

	int radiusX[4];
	radiusX[0] = center.x;
	radiusX[1] = (int)(radiusX[0] - outlineThickness);
	radiusX[2] = (int)(radiusX[1] - thickness);
	radiusX[3] = (int)(radiusX[2] - outlineThickness);

	// Reserve buffer for our line traces

	int samplePoints = clip.w + 1;

	int bufferSize = samplePoints * sizeof(int) * 4 * 2;		// length+1 * sizeof(int) * 4 separate traces * 2 halves.
	int * pBuffer = (int*)GfxBase::memStackAlloc(bufferSize);

	// Do line traces.

	int yMid = (center.y & 0xFFFFFFC0) - outerRect.y * 64;
	int yAdjust = center.y & 0x3F;						// Compensate for center not being on pixel boundary.
	int centerOfs = center.x - (outerRect.x << 6);
	int samplePitch = 4;

	for (int edge = 0; edge < 4; edge++)
	{
		int * pOutUpper = pBuffer + edge;
		int * pOutLower = pBuffer + 3 - edge + samplePoints * 4;

		if (radiusX[edge] <= 0 || radiusY[edge] <= 0)
		{
			for (int sample = 0; sample < samplePoints; sample++)
			{
				pOutUpper[sample*samplePitch] = yMid;
				pOutLower[sample*samplePitch] = yMid;
			}
		}
		else
		{
			//TODO: We need to handle subpixels better here.
			// We should increase curvePos with the real xStart offset (could be as much as 63/64 of a pixel)
			// and handle xMid similarly.
			
			int xStart = (centerOfs - radiusX[edge] + 63) >> 6;		// First pixel-edge inside curve.
			int xMid = (centerOfs) >> 6;							// Pixel edge on or right before center.
			int xEnd = (centerOfs + radiusX[edge]) >> 6;			// Last pixel-edge inside curve.


			int curveInc = int(int64_t((65536 * 64)-1) * (curveTabSize - 1) / radiusX[edge]); // Keep as many decimals as possible, this is important!
			int curvePos = int((((radiusX[edge] - centerOfs) & 0x3F) * ((int64_t)curveInc)) >> 6);

			if (clipLeft > 0)
			{
				xStart -= clipLeft;
				xMid -= clipLeft;
				xEnd -= clipLeft;

				if (xStart < 0)
					curvePos += (-xStart) * curveInc;
			}

			// Clip xStart, xMid and xEnd

			if (xEnd >= samplePoints)
			{
				xEnd = samplePoints - 1;

				xStart = min(xStart, xEnd);
				xMid = min(xMid, xEnd);
			}

			//

			int sample = 0;
			while (sample < xStart)
			{
				pOutUpper[sample*samplePitch] = yMid;
				pOutLower[sample*samplePitch] = yMid;
				sample++;
			}

			while (sample <= xMid)
			{
				int i = curvePos >> 16;
				uint32_t f = curvePos & 0xFFFF;

				uint32_t heightFactor = (pCurveTab[i] * (65535 - f) + pCurveTab[i + 1] * f) >> 16;
				int height = ((radiusY[edge] >> 16) * heightFactor) + ((radiusY[edge] & 0xFFFF) * heightFactor >> 16);  // = (radiusY[edge] * heightFactor) / 65536, but without overflow.

				pOutUpper[sample*samplePitch] = yMid + yAdjust - height;
				pOutLower[sample*samplePitch] = yMid + yAdjust + height;
				sample++;
				curvePos += curveInc;
			}

			curvePos = (curveTabSize - 1) * 65536 * 2 - curvePos;

			while (sample <= xEnd)
			{
				int i = curvePos >> 16;
				uint32_t f = curvePos & 0xFFFF;

				uint32_t heightFactor = (pCurveTab[i] * (65535 - f) + pCurveTab[i + 1] * f) >> 16;
				int height = ((radiusY[edge] >> 16) * heightFactor) + ((radiusY[edge] & 0xFFFF) * heightFactor >> 16); // = (radiusY[edge] * heightFactor) / 65536, but without overflow.

				pOutUpper[sample*samplePitch] = yMid + yAdjust - height;
				pOutLower[sample*samplePitch] = yMid + yAdjust + height;
				sample++;
				curvePos -= curveInc;
			}

			while (sample < samplePoints)
			{
				pOutUpper[sample*samplePitch] = yMid;
				pOutLower[sample*samplePitch] = yMid;
				sample++;
			}

			// Take care of left and right edges that needs more calculations to get the angle right.

			int pixFracLeft = (xStart << 6) - (centerOfs - radiusX[edge]);
			int pixFracRight = (centerOfs + radiusX[edge]) & 0x3F;

			if (pixFracLeft > 0 && xStart > 0)
			{
				pOutUpper[(xStart - 1)*samplePitch] = pOutUpper[xStart*samplePitch] + (yMid + yAdjust - pOutUpper[xStart*samplePitch]) * 64 / pixFracLeft;
				pOutLower[(xStart - 1)*samplePitch] = pOutLower[xStart*samplePitch] + (yMid + yAdjust - pOutLower[xStart*samplePitch]) * 64 / pixFracLeft;

			}
			if (pixFracRight > 0 && xEnd < samplePoints - 1)
			{
				pOutUpper[(xEnd + 1)*samplePitch] = pOutUpper[xEnd*samplePitch] + (yMid + yAdjust - pOutUpper[xEnd*samplePitch]) * 64 / pixFracRight;
				pOutLower[(xEnd + 1)*samplePitch] = pOutLower[xEnd*samplePitch] + (yMid + yAdjust - pOutLower[xEnd*samplePitch]) * 64 / pixFracRight;
			}

		}
	}

	// Render columns

	HiColor	col[9];
	col[0] = HiColor::Transparent;
	col[1] = outlineColor;
	col[2] = fillColor;
	col[3] = outlineColor;
	col[4] = HiColor::Transparent;
	col[5] = outlineColor;
	col[6] = fillColor;
	col[7] = outlineColor;
	col[8] = HiColor::Transparent;

	if( !pFactory )
		pFactory = GfxBase::defaultEdgemapFactory();
		
	auto pEdgemap = pFactory->createEdgemap( WGBP(Edgemap,
													_.size = outerRect.size(),
													_.segments = 9,
													_.colors = col) );
	

	pEdgemap->importSamples(SampleOrigo::Top, pBuffer, 0, 4, 0, samplePoints, 1, 4);
	pEdgemap->importSamples(SampleOrigo::Top, pBuffer+samplePoints * 4, 4, 8, 0, samplePoints, 1, 4);

	pEdgemap->setRenderSegments(9);
	
	// Free temporary work memory

	GfxBase::memStackFree(bufferSize);

	return pEdgemap;
}



//____ drawOutlinedDonut() ______________________________________________________

bool drawOutlinedDonut(Edgemap * pEdgemap, spx thickness, spx outlineThickness )
{
	if( pEdgemap->segments() < 9 )
		return false;

	auto pCurveTab = GfxBase::curveTab();
	int curveTabSize = GfxBase::curveTabSize();

	
	// Center and corners in 26.6 format.

	SizeI size = pEdgemap->pixelSize();
	
	CoordI center = {  (size.w * 64) / 2, (size.h * 64) / 2 };

	// Calculate maximum width and height from center for the 4 edges of the elipse.

	int radiusY[4];
	radiusY[0] = center.y;
	radiusY[1] = (int)(radiusY[0] - outlineThickness);
	radiusY[2] = (int)(radiusY[1] - thickness);
	radiusY[3] = (int)(radiusY[2] - outlineThickness);

	int radiusX[4];
	radiusX[0] = center.x;
	radiusX[1] = (int)(radiusX[0] - outlineThickness);
	radiusX[2] = (int)(radiusX[1] - thickness);
	radiusX[3] = (int)(radiusX[2] - outlineThickness);

	// Reserve buffer for our line traces

	int samplePoints = size.w + 1;

	int bufferSize = samplePoints * sizeof(int) * 4 * 2;		// length+1 * sizeof(int) * 4 separate traces * 2 halves.
	int * pBuffer = (int*)GfxBase::memStackAlloc(bufferSize);

	// Do line traces.

	int yMid = (center.y & 0xFFFFFFC0);
	int yAdjust = center.y & 0x3F;						// Compensate for center not being on pixel boundary.
	int centerOfs = center.x;
	int samplePitch = 4;

	for (int edge = 0; edge < 4; edge++)
	{
		int * pOutUpper = pBuffer + edge;
		int * pOutLower = pBuffer + 3 - edge + samplePoints * 4;

		if (radiusX[edge] <= 0 || radiusY[edge] <= 0)
		{
			for (int sample = 0; sample < samplePoints; sample++)
			{
				pOutUpper[sample*samplePitch] = yMid;
				pOutLower[sample*samplePitch] = yMid;
			}
		}
		else
		{
			//TODO: We need to handle subpixels better here.
			// We should increase curvePos with the real xStart offset (could be as much as 63/64 of a pixel)
			// and handle xMid similarly.
			
			int xStart = (centerOfs - radiusX[edge] + 63) >> 6;		// First pixel-edge inside curve.
			int xMid = (centerOfs) >> 6;							// Pixel edge on or right before center.
			int xEnd = (centerOfs + radiusX[edge]) >> 6;			// Last pixel-edge inside curve.


			int curveInc = int(int64_t((65536 * 64)-1) * (curveTabSize - 1) / radiusX[edge]); // Keep as many decimals as possible, this is important!
			int curvePos = int((((radiusX[edge] - centerOfs) & 0x3F) * ((int64_t)curveInc)) >> 6);

			// Clip xStart, xMid and xEnd

			if (xEnd >= samplePoints)
			{
				xEnd = samplePoints - 1;

				xStart = min(xStart, xEnd);
				xMid = min(xMid, xEnd);
			}

			//

			int sample = 0;
			while (sample < xStart)
			{
				pOutUpper[sample*samplePitch] = yMid;
				pOutLower[sample*samplePitch] = yMid;
				sample++;
			}

			while (sample <= xMid)
			{
				int i = curvePos >> 16;
				uint32_t f = curvePos & 0xFFFF;

				uint32_t heightFactor = (pCurveTab[i] * (65535 - f) + pCurveTab[i + 1] * f) >> 16;
				int height = ((radiusY[edge] >> 16) * heightFactor) + ((radiusY[edge] & 0xFFFF) * heightFactor >> 16);  // = (radiusY[edge] * heightFactor) / 65536, but without overflow.

				pOutUpper[sample*samplePitch] = yMid + yAdjust - height;
				pOutLower[sample*samplePitch] = yMid + yAdjust + height;
				sample++;
				curvePos += curveInc;
			}

			curvePos = (curveTabSize - 1) * 65536 * 2 - curvePos;

			while (sample <= xEnd)
			{
				int i = curvePos >> 16;
				uint32_t f = curvePos & 0xFFFF;

				uint32_t heightFactor = (pCurveTab[i] * (65535 - f) + pCurveTab[i + 1] * f) >> 16;
				int height = ((radiusY[edge] >> 16) * heightFactor) + ((radiusY[edge] & 0xFFFF) * heightFactor >> 16); // = (radiusY[edge] * heightFactor) / 65536, but without overflow.

				pOutUpper[sample*samplePitch] = yMid + yAdjust - height;
				pOutLower[sample*samplePitch] = yMid + yAdjust + height;
				sample++;
				curvePos -= curveInc;
			}

			while (sample < samplePoints)
			{
				pOutUpper[sample*samplePitch] = yMid;
				pOutLower[sample*samplePitch] = yMid;
				sample++;
			}

			// Take care of left and right edges that needs more calculations to get the angle right.

			int pixFracLeft = (xStart << 6) - (centerOfs - radiusX[edge]);
			int pixFracRight = (centerOfs + radiusX[edge]) & 0x3F;

			if (pixFracLeft > 0 && xStart > 0)
			{
				pOutUpper[(xStart - 1)*samplePitch] = pOutUpper[xStart*samplePitch] + (yMid + yAdjust - pOutUpper[xStart*samplePitch]) * 64 / pixFracLeft;
				pOutLower[(xStart - 1)*samplePitch] = pOutLower[xStart*samplePitch] + (yMid + yAdjust - pOutLower[xStart*samplePitch]) * 64 / pixFracLeft;

			}
			if (pixFracRight > 0 && xEnd < samplePoints - 1)
			{
				pOutUpper[(xEnd + 1)*samplePitch] = pOutUpper[xEnd*samplePitch] + (yMid + yAdjust - pOutUpper[xEnd*samplePitch]) * 64 / pixFracRight;
				pOutLower[(xEnd + 1)*samplePitch] = pOutLower[xEnd*samplePitch] + (yMid + yAdjust - pOutLower[xEnd*samplePitch]) * 64 / pixFracRight;
			}

		}
	}

	// Import generated samples to edgemap

	pEdgemap->importSamples(SampleOrigo::Top, pBuffer, 0, 4, 0, samplePoints, 1, 4);
	pEdgemap->importSamples(SampleOrigo::Top, pBuffer+samplePoints * 4, 4, 8, 0, samplePoints, 1, 4);

	// Free temporary work memory

	GfxBase::memStackFree(bufferSize);

	pEdgemap->setRenderSegments(9);
	return true;
}


//____ convertSamples() _____________________________________________________

void convertSamples( spx * pDest, const spx * pSource, SampleOrigo origo, spx height, int nEdges, int nSamples, int srcEdgePitch, int srcSamplePitch, int destEdgePitch, int destSamplePitch )
{
	spx mul = (origo == SampleOrigo::Top || origo == SampleOrigo::MiddleDown) ? 1 : -1;
	spx offset = 0;
	
	if( origo == SampleOrigo::Bottom )
		offset = height;
	else if( origo == SampleOrigo::MiddleDown || origo == SampleOrigo::MiddleUp )
		offset = height/2;

	for( int edge = 0 ; edge < nEdges ; edge++ )
	{
		const spx * pSrc = pSource + srcEdgePitch * edge;
		spx * pDst = pDest + destEdgePitch * edge;

		for( int sample = 0 ; sample < nSamples ; sample++ )
		{
			* pDst = (* pSrc * mul) + offset;
			pDst += destSamplePitch;
			pSrc += srcSamplePitch;
		}
	}
}

void convertSamples( spx * pDest, const float * pSource, SampleOrigo origo, spx height, int nEdges, int nSamples, int srcEdgePitch, int srcSamplePitch, int destEdgePitch, int destSamplePitch )
{
	spx mul = (origo == SampleOrigo::Top || origo == SampleOrigo::MiddleDown) ? 1 : -1;
	spx offset = 0;
	
	if( origo == SampleOrigo::Bottom )
		offset = height;
	else if( origo == SampleOrigo::MiddleDown || origo == SampleOrigo::MiddleUp )
		offset = height/2;

	if( origo == SampleOrigo::MiddleDown || origo == SampleOrigo::MiddleUp )
		mul *= height/2;
	else
		mul *= height;
	
	for( int edge = 0 ; edge < nEdges ; edge++ )
	{
		const float * pSrc = pSource + srcEdgePitch * edge;
		spx * pDst = pDest + destEdgePitch * edge;

		for( int sample = 0 ; sample < nSamples ; sample++ )
		{
			* pDst = (* pSrc * mul) + offset;
			pDst += destSamplePitch;
			pSrc += srcSamplePitch;
		}
	}
}

//____ convertSample() _____________________________________________________

spx convertSample( spx sample, SampleOrigo origo, spx height )
{
	spx mul = (origo == SampleOrigo::Top || origo == SampleOrigo::MiddleDown) ? 1 : -1;
	spx offset = 0;
	
	if( origo == SampleOrigo::Bottom )
		offset = height;
	else if( origo == SampleOrigo::MiddleDown || origo == SampleOrigo::MiddleUp )
		offset = height/2;

	return (sample * mul) + offset;
}

spx convertSample( float sample, SampleOrigo origo, spx height )
{
	spx mul = (origo == SampleOrigo::Top || origo == SampleOrigo::MiddleDown) ? 1 : -1;
	spx offset = 0;
	
	if( origo == SampleOrigo::Bottom )
		offset = height;
	else if( origo == SampleOrigo::MiddleDown || origo == SampleOrigo::MiddleUp )
		offset = height/2;

	return (sample * mul) + offset;
}

}	// namespace EdgemapTools
}	// namespace wg
