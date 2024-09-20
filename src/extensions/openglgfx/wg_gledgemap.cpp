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

#include <wg_gledgemap.h>

#include <cstring>

namespace wg
{

const TypeInfo GlEdgemap::TYPEINFO = { "GlEdgemap", &Edgemap::TYPEINFO };


//____ create() ______________________________________________________________

GlEdgemap_p	GlEdgemap::create( const Blueprint& blueprint )
{
	if( !_validateBlueprint(blueprint) )
		return nullptr;
	
	return GlEdgemap_p( new GlEdgemap(blueprint) );
}

GlEdgemap_p GlEdgemap::create( const Blueprint& blueprint, SampleOrigo origo, const float * pSamples, int edges, int edgePitch, int samplePitch)
{
	if( !_validateBlueprint(blueprint) )
		return nullptr;

	if(edges >= blueprint.segments )
		return nullptr;

	auto p = GlEdgemap_p( new GlEdgemap(blueprint) );

	p->_importSamples(origo, pSamples, 0, edges, 0, p->m_size.w+1, edgePitch, samplePitch);
	p->m_nbRenderSegments = edges+1;
	
	return p;
}

GlEdgemap_p GlEdgemap::create( const Blueprint& blueprint, SampleOrigo origo, const spx * pSamples, int edges, int edgePitch, int samplePitch)
{
	if( !_validateBlueprint(blueprint) )
		return nullptr;

	if(edges >= blueprint.segments )
		return nullptr;
	
	auto p = GlEdgemap_p( new GlEdgemap(blueprint) );

	p->_importSamples(origo, pSamples, 0, edges, 0, p->m_size.w+1, edgePitch, samplePitch);
	p->m_nbRenderSegments = edges+1;
	
	return p;
}

//____ constructor ___________________________________________________________

GlEdgemap::GlEdgemap(const Blueprint& bp) : Edgemap(bp)
{
	// Setup buffers
	
	int sampleArraySize = (bp.size.w+1) * bp.segments * sizeof(spx);
	int colorArraySize = bp.colors ? bp.segments*sizeof(HiColor) : 0;
	int gradientArraySize = bp.gradients ? bp.segments*sizeof(Gradient) : 0;
	int tintmapArraySize = bp.tintmaps ? bp.segments*sizeof(Tintmap*) : 0;
	int renderColorsBytes = (bp.gradients || bp.tintmaps) ? bp.segments * sizeof(HiColor) * 4 : 0;		// Reserve space for XY-gradient colors.

	int bytes = sampleArraySize + colorArraySize + gradientArraySize + tintmapArraySize + renderColorsBytes;
	
	m_pBuffer = new char[bytes];
	
	auto pBuffer = m_pBuffer;
	
	if( gradientArraySize > 0 )
	{
		m_pGradients = (Gradient*) pBuffer;
		memcpy(m_pGradients, bp.gradients, gradientArraySize);
		pBuffer += gradientArraySize;

		m_pRenderColors = (HiColor*) pBuffer;
		pBuffer += renderColorsBytes;
	}
	else if( tintmapArraySize > 0 )
	{
		m_pTintmaps = (Tintmap**) pBuffer;
		
		for( int i = 0 ; i < bp.segments ; i++ )
		{
			m_pTintmaps[i] = bp.tintmaps[i];
			m_pTintmaps[i]->retain();
		}
			
		pBuffer += tintmapArraySize;
		m_pRenderColors = (HiColor*) pBuffer;
		pBuffer += renderColorsBytes;
	}
	else if( colorArraySize > 0 )
	{
		m_pColors = (HiColor*) pBuffer;
		memcpy(m_pColors, bp.colors, colorArraySize);
		pBuffer += colorArraySize;

		m_pRenderColors = m_pColors;
	}

	_updateRenderColors();
	
	m_pSamples = (spx*) pBuffer;
}

//____ destructor ____________________________________________________________

GlEdgemap::~GlEdgemap()
{
	// Decrease reference count of tintmaps.

	if( m_pTintmaps )
	{
		for( int i = 0 ; i < m_nbSegments ; i++ )
			m_pTintmaps[i]->release();
	}

	// Delete our buffer
	
	delete [] m_pBuffer;
}

//____ typeInfo() ____________________________________________________________

const TypeInfo& GlEdgemap::typeInfo(void) const
{
	return TYPEINFO;
}

//____ setColors() ____________________________________________________________

bool GlEdgemap::setColors( int begin, int end, const HiColor * pColors )
{
	if( !Edgemap::setColors(begin,end,pColors))
		return false;
	
	_updateRenderColors();
	return true;
}

//____ setGradients() _________________________________________________________

bool GlEdgemap::setGradients( int begin, int end, const Gradient * pGradients )
{
	if( !Edgemap::setGradients(begin,end,pGradients))
		return false;
	
	_updateRenderColors();
	return true;
}

//____ importSamples() _________________________________________________________

bool GlEdgemap::importSamples( SampleOrigo origo, const spx * pSource, int edgeBegin, int edgeEnd,
							  int sampleBegin, int sampleEnd, int edgePitch, int samplePitch )
{
	if( pSource == nullptr || edgeBegin < 0 || edgeBegin > edgeEnd || edgeEnd > (m_nbSegments-1) || sampleBegin < 0 || sampleBegin > sampleEnd || sampleEnd > (m_size.w+1) )
		return false;

	_importSamples( origo, pSource, edgeBegin, edgeEnd, sampleBegin, sampleEnd, edgePitch, samplePitch);
	return true;
}

bool GlEdgemap::importSamples( SampleOrigo origo, const float * pSource, int edgeBegin, int edgeEnd,
							  int sampleBegin, int sampleEnd, int edgePitch, int samplePitch )
{
	if( pSource == nullptr || edgeBegin < 0 || edgeBegin > edgeEnd || edgeEnd > (m_nbSegments-1) || sampleBegin < 0 || sampleBegin > sampleEnd || sampleEnd > (m_size.w+1) )
		return false;

	_importSamples( origo, pSource, edgeBegin, edgeEnd, sampleBegin, sampleEnd, edgePitch, samplePitch);
	return true;
}


//____ exportSamples() _________________________________________________________

bool GlEdgemap::exportSamples( SampleOrigo origo, spx * pDestination, int edgeBegin, int edgeEnd,
							  int sampleBegin, int sampleEnd, int edgePitch, int samplePitch )
{
	//TODO: Implement!!!
	
	return false;
}

bool  GlEdgemap::exportSamples( SampleOrigo origo, float * pDestination, int edgeBegin, int edgeEnd,
							  int sampleBegin, int sampleEnd, int edgePitch, int samplePitch )
{
	//TODO: Implement!!!
	
	return false;
}

//____ _importSamples() ________________________________________________________

void GlEdgemap::_importSamples( SampleOrigo origo, const spx * pSource, int edgeBegin, int edgeEnd,
							  int sampleBegin, int sampleEnd, int edgePitch, int samplePitch )
{
	if( samplePitch == 0 )
		samplePitch = 1;
	
	
	if( edgePitch == 0 )
		edgePitch = samplePitch * (sampleEnd - sampleBegin);
	
	int destSamplePitch = m_nbSegments-1;
	int destEdgePitch = 1;


	spx mul = (origo == SampleOrigo::Top || origo == SampleOrigo::MiddleDown) ? 1 : -1;
	spx offset = 0;
	
	if( origo == SampleOrigo::Bottom )
		offset = m_size.h*64;
	else if( origo == SampleOrigo::MiddleDown || origo == SampleOrigo::MiddleUp )
		offset = m_size.h*32;

	for( int edge = edgeBegin ; edge < edgeEnd ; edge++ )
	{
		const spx * pSrc = pSource + edgePitch * (edge-edgeBegin);
		spx * pDst = m_pSamples + edge + sampleBegin*(m_nbSegments-1);

		for( int sample = sampleBegin ; sample < sampleEnd ; sample++ )
		{
			* pDst = (* pSrc * mul) + offset;
			pDst += destSamplePitch;
			pSrc += samplePitch;
		}
	}
}

void GlEdgemap::_importSamples( SampleOrigo origo, const float * pSource, int edgeBegin, int edgeEnd,
							  int sampleBegin, int sampleEnd, int edgePitch, int samplePitch )
{
	if( samplePitch == 0 )
		samplePitch = 1;
	
	
	if( edgePitch == 0 )
		edgePitch = samplePitch * (sampleEnd - sampleBegin);
	
	int destSamplePitch = m_nbSegments-1;
	int destEdgePitch = 1;


	spx mul = (origo == SampleOrigo::Top || origo == SampleOrigo::MiddleDown) ? 1 : -1;
	spx offset = 0;

	if( origo == SampleOrigo::Bottom )
		offset = m_size.h*64;
	else if( origo == SampleOrigo::MiddleDown || origo == SampleOrigo::MiddleUp )
		offset = m_size.h*32;

	if( origo == SampleOrigo::MiddleDown || origo == SampleOrigo::MiddleUp )
		mul *= m_size.h*32;
	else
		mul *= m_size.h*64;
	
	for( int edge = edgeBegin ; edge < edgeEnd ; edge++ )
	{
		const float * pSrc = pSource + edgePitch * edge + samplePitch * sampleBegin;
		spx * pDst = m_pSamples + edge + sampleBegin*(m_nbSegments-1);

		for( int sample = sampleBegin ; sample < sampleEnd ; sample++ )
		{
			* pDst = (* pSrc * mul) + offset;
			pDst += destSamplePitch;
			pSrc += samplePitch;
		}
	}
}


//____ _updateRenderColors() __________________________________________________

void GlEdgemap::_updateRenderColors()
{
	// Analyze gradients to figure out our tint mode and update our render colors.
		
	if( m_pGradients )
	{
		// Figure out and set optimal TintMode.
		
		bool bHorizontal = false;
		bool bVertical = false;
		bool bFlat = true;
		
		for( int i = 0 ; i < m_nbSegments ; i++ )
		{
			const Gradient& grad = m_pGradients[i];
			
			if(grad.topLeft != grad.topRight)
				bHorizontal = true;

			if(grad.topLeft != grad.bottomLeft)
				bVertical = true;
			
			if(grad.topLeft != grad.topRight || grad.topLeft != grad.bottomRight || grad.topLeft != grad.bottomLeft )
				bFlat = false;
		}
		
		if( bFlat )
			m_tintMode = TintMode::Flat;
		else if( bHorizontal && bVertical )
			m_tintMode = TintMode::GradientXY;
		else if( bVertical )
			m_tintMode = TintMode::GradientY;
		else
			m_tintMode = TintMode::GradientX;

		// Copy gradient colors to render colors.
		
		switch( m_tintMode )
		{
			case TintMode::None:
			case TintMode::Flat:
			{
				for( int i = 0 ; i < m_nbSegments ; i++ )
					m_pRenderColors[i] = m_pGradients[i].topLeft;

				break;
			}

			case TintMode::GradientX:
			{
				for( int i = 0 ; i < m_nbSegments ; i++ )
				{
					const Gradient& grad = m_pGradients[i];

					m_pRenderColors[i*2] = grad.topLeft;
					m_pRenderColors[i*2+1] = grad.topRight;
				}
				break;
			}
			case TintMode::GradientY:
			{
				for( int i = 0 ; i < m_nbSegments ; i++ )
				{
					const Gradient& grad = m_pGradients[i];

					m_pRenderColors[i*2] = grad.topLeft;
					m_pRenderColors[i*2+1] = grad.bottomLeft;
				}
				break;
			}
			case TintMode::GradientXY:
			{
				for( int i = 0 ; i < m_nbSegments ; i++ )
				{
					const Gradient& grad = m_pGradients[i];

					m_pRenderColors[i*4]   = grad.topLeft;
					m_pRenderColors[i*4+1] = grad.topRight;
					m_pRenderColors[i*4+2] = grad.bottomRight;
					m_pRenderColors[i*4+3] = grad.bottomLeft;
				}
				break;
			}
		}
	}
	else
	{
		m_tintMode = TintMode::Flat;
	}
	
	// Generate render colors from Tintmaps as well as we can to support legacy implementations.
	
	if( m_pTintmaps )
	{
		m_tintMode = TintMode::GradientXY;
		
		for( int i = 0 ; i < m_nbSegments ; i++ )
		{
			Gradient grad = m_pTintmaps[i]->exportGradient();
			
			m_pRenderColors[i*4]   = grad.topLeft;
			m_pRenderColors[i*4+1] = grad.topRight;
			m_pRenderColors[i*4+2] = grad.bottomRight;
			m_pRenderColors[i*4+3] = grad.bottomLeft;
		}
	}
}

} // namespace wg

