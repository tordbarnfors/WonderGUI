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

#include <wg_softedgemap.h>
#include <wg_gradyent.h>

#include <cstring>

namespace wg
{

const TypeInfo SoftEdgemap::TYPEINFO = { "SoftEdgemap", &Edgemap::TYPEINFO };


//____ create() ______________________________________________________________

SoftEdgemap_p	SoftEdgemap::create( const Blueprint& blueprint )
{
	if( !_validateBlueprint(blueprint) )
		return nullptr;
	
	return SoftEdgemap_p( new SoftEdgemap(blueprint) );
}

SoftEdgemap_p SoftEdgemap::create( const Blueprint& blueprint, SampleOrigo origo, const float * pSamples, int edges, int edgePitch, int samplePitch)
{
	if( !_validateBlueprint(blueprint) )
		return nullptr;

	if(edges >= blueprint.segments )
		return nullptr;

	auto p = SoftEdgemap_p( new SoftEdgemap(blueprint) );

	p->_importSamples(origo, pSamples, 0, edges, 0, p->m_size.w+1, edgePitch, samplePitch);
	p->m_nbRenderSegments = edges+1;
	
	return p;
}

SoftEdgemap_p SoftEdgemap::create( const Blueprint& blueprint, SampleOrigo origo, const spx * pSamples, int edges, int edgePitch, int samplePitch)
{
	if( !_validateBlueprint(blueprint) )
		return nullptr;

	if(edges >= blueprint.segments )
		return nullptr;
	
	auto p = SoftEdgemap_p( new SoftEdgemap(blueprint) );

	p->_importSamples(origo, pSamples, 0, edges, 0, p->m_size.w+1, edgePitch, samplePitch);
	p->m_nbRenderSegments = edges+1;
	
	return p;
}

//____ constructor ___________________________________________________________

SoftEdgemap::SoftEdgemap(const Blueprint& bp) : Edgemap(bp)
{
	// Setup buffers
	
	int sampleArraySize = (bp.size.w+1) * bp.segments * sizeof(spx);
	int colorArraySize = (bp.size.w + bp.size.h) * bp.segments * sizeof(HiColor);	// We always reserve memory for a full set of tintmap colors.
	int gradientArraySize = bp.segments*sizeof(Gradient);							// We keep a set of gradients for backward compatiblity

	int bytes = sampleArraySize + colorArraySize + gradientArraySize;
	
	m_pBuffer = new char[bytes];

	m_pSamples = (spx*) m_pBuffer;
	m_pColors = (HiColor*) (m_pBuffer + sampleArraySize);
	m_pGradients = (Gradient*) (m_pColors + colorArraySize);
	
	int colorPitch = bp.size.w + bp.size.h;
	
	for( int seg = 0 ; seg < bp.segments ; seg++ )
	{
		auto pOutput = m_pColors + (seg * colorPitch);
				
		if( bp.tintmaps && bp.tintmaps[seg] )
		{
			auto pTintmap = bp.tintmaps[seg];
			
			pTintmap->exportHorizontalColors(bp.size.w*64, pOutput);
			pOutput += bp.size.w;
			pTintmap->exportVerticalColors(bp.size.h*64, pOutput);

			if( pTintmap->isHorizontal() )
				m_horrTintmaps.set(seg);

			if( pTintmap->isVertical() )
				m_vertTintmaps.set(seg);
			
			if( pTintmap->isOpaque() )
				m_opaqueSegments.set(seg);
			
			m_pGradients[seg] = pTintmap->exportGradient();
		}
		else if( bp.gradients )
		{
			auto& gradient = bp.gradients[seg];
			
			auto pTintmap = Gradyent::create(gradient);
			
			pTintmap->exportHorizontalColors(bp.size.w*64, pOutput);
			pOutput += bp.size.w;
			pTintmap->exportVerticalColors(bp.size.h*64, pOutput);

			if( pTintmap->isHorizontal() )
				m_horrTintmaps.set(seg);

			if( pTintmap->isVertical() )
				m_vertTintmaps.set(seg);
			
			if( pTintmap->isOpaque() )
				m_opaqueSegments.set(seg);
			else if( gradient.topLeft.a == 0 && gradient.topRight.a == 0 && gradient.bottomLeft.a == 0 && gradient.bottomRight.a == 0 )
				m_transparentSegments.set(seg);
			
			m_pGradients[seg] = pTintmap->exportGradient();
		}
		else if( bp.colors )
		{
			for( int i = 0 ; i < bp.size.w ; i++ )
				* pOutput++ = bp.colors[seg];

			for( int i = 0 ; i < bp.size.h ; i++ )
				* pOutput++ = HiColor::White;

			if( bp.colors[seg].a == 0 )
				m_transparentSegments.set(seg);
			else if( bp.colors[seg].a == 4096 )
				m_opaqueSegments.set(seg);
			
			m_pGradients[seg] = Gradient(bp.colors[seg], bp.colors[seg], bp.colors[seg], bp.colors[seg]);

		}
		else
		{
			for( int i = 0 ; i < bp.size.w + bp.size.h ; i++ )
				* pOutput++ = HiColor::Transparent;

			m_transparentSegments.set(seg);
			
			m_pGradients[seg] = Gradient(HiColor::Transparent, HiColor::Transparent, HiColor::Transparent, HiColor::Transparent);
		}
		
	}
	
}

//____ destructor ____________________________________________________________

SoftEdgemap::~SoftEdgemap()
{
	delete [] m_pBuffer;
}

//____ typeInfo() ____________________________________________________________

const TypeInfo& SoftEdgemap::typeInfo(void) const
{
	return TYPEINFO;
}

//____ setColors() ____________________________________________________________

bool SoftEdgemap::setColors( int begin, int end, const HiColor * pColors )
{
	int pitch = m_size.w + m_size.h;
	
	auto pOutput = m_pColors + begin * pitch;
	
	for( int seg = begin ; seg < end ; seg++ )
	{
		for( int i = 0 ; i < m_size.w ; i++ )
			* pOutput++ = * pColors;
		
		for( int i = 0 ; i < m_size.h ; i++ )
			* pOutput++ = HiColor::White;

		m_transparentSegments[seg] = (pColors->a == 0 );
		m_opaqueSegments[seg] = (pColors->a == 4096 );

		m_horrTintmaps.reset(seg);
		m_vertTintmaps.reset(seg);

		pColors++;
	}
	
	return true;
}

//____ setGradients() _________________________________________________________

bool SoftEdgemap::setGradients( int begin, int end, const Gradient * pGradients )
{
	int pitch = m_size.w + m_size.h;
	
	auto pOutput = m_pColors + begin * pitch;
	
	for( int seg = begin ; seg < end ; seg++ )
	{
		auto& gradient = * pGradients++;
		
		if( gradient.topLeft.a > 0 || gradient.topRight.a > 0 || gradient.bottomLeft.a > 0 || gradient.bottomRight.a > 0 )
		{
			auto pTintmap = Gradyent::create( gradient );
			
			pTintmap->exportHorizontalColors(m_size.w*64, pOutput);
			pOutput += m_size.w;
			pTintmap->exportVerticalColors(m_size.h*64, pOutput);
			pOutput += m_size.h;

			m_horrTintmaps[seg] = pTintmap->isHorizontal();
			m_vertTintmaps[seg] = pTintmap->isVertical();

			m_transparentSegments[seg] = false;
			m_opaqueSegments[seg] = pTintmap->isOpaque();

			m_pGradients[seg] = pTintmap->exportGradient();
		}
		else
		{
			for( int i = 0 ; i < m_size.w + m_size.h ; i++ )
				* pOutput++ = HiColor::Transparent;

			m_transparentSegments.set(seg);
			m_opaqueSegments.reset(seg);

			m_pGradients[seg] = Gradient(HiColor::Transparent, HiColor::Transparent, HiColor::Transparent, HiColor::Transparent);
		}
	}
	
	return true;
}

//____ setTintmaps() __________________________________________________________

bool SoftEdgemap::setTintmaps( int begin, int end, const Tintmap_p * pTintmaps )
{
	int pitch = m_size.w + m_size.h;
	
	auto pOutput = m_pColors + begin * pitch;
	
	for( int seg = begin ; seg < end ; seg++ )
	{
		auto pTintmap = * pTintmaps++;
		
		if( pTintmap )
		{
			pTintmap->exportHorizontalColors(m_size.w*64, pOutput);
			pOutput += m_size.w;
			pTintmap->exportVerticalColors(m_size.h*64, pOutput);
			pOutput += m_size.h;

			m_horrTintmaps[seg] = pTintmap->isHorizontal();
			m_vertTintmaps[seg] = pTintmap->isVertical();

			m_transparentSegments[seg] = false;
			m_opaqueSegments[seg] = pTintmap->isOpaque();
			
			m_pGradients[seg] = pTintmap->exportGradient();
		}
		else
		{
			for( int i = 0 ; i < m_size.w + m_size.h ; i++ )
				* pOutput++ = HiColor::Transparent;

			m_transparentSegments.set(seg);
			m_opaqueSegments.reset(seg);

			m_pGradients[seg] = Gradient(HiColor::Transparent, HiColor::Transparent, HiColor::Transparent, HiColor::Transparent);
		}
	}
	
	return true;
}


//____ importSamples() _________________________________________________________

bool SoftEdgemap::importSamples( SampleOrigo origo, const spx * pSource, int edgeBegin, int edgeEnd,
							  int sampleBegin, int sampleEnd, int edgePitch, int samplePitch )
{
	if( pSource == nullptr || edgeBegin < 0 || edgeBegin > edgeEnd || edgeEnd > (m_nbSegments-1) || sampleBegin < 0 || sampleBegin > sampleEnd || sampleEnd > (m_size.w+1) )
		return false;

	_importSamples( origo, pSource, edgeBegin, edgeEnd, sampleBegin, sampleEnd, edgePitch, samplePitch);
	return true;
}

bool SoftEdgemap::importSamples( SampleOrigo origo, const float * pSource, int edgeBegin, int edgeEnd,
							  int sampleBegin, int sampleEnd, int edgePitch, int samplePitch )
{
	if( pSource == nullptr || edgeBegin < 0 || edgeBegin > edgeEnd || edgeEnd > (m_nbSegments-1) || sampleBegin < 0 || sampleBegin > sampleEnd || sampleEnd > (m_size.w+1) )
		return false;

	_importSamples( origo, pSource, edgeBegin, edgeEnd, sampleBegin, sampleEnd, edgePitch, samplePitch);
	return true;
}


//____ exportSamples() _________________________________________________________

bool SoftEdgemap::exportSamples( SampleOrigo origo, spx * pDestination, int edgeBegin, int edgeEnd,
							  int sampleBegin, int sampleEnd, int edgePitch, int samplePitch )
{
	//TODO: Implement!!!
	
	return false;
}

bool  SoftEdgemap::exportSamples( SampleOrigo origo, float * pDestination, int edgeBegin, int edgeEnd,
							  int sampleBegin, int sampleEnd, int edgePitch, int samplePitch )
{
	//TODO: Implement!!!
	
	return false;
}

//____ _importSamples() ________________________________________________________

void SoftEdgemap::_importSamples( SampleOrigo origo, const spx * pSource, int edgeBegin, int edgeEnd,
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

void SoftEdgemap::_importSamples( SampleOrigo origo, const float * pSource, int edgeBegin, int edgeEnd,
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




} // namespace wg

