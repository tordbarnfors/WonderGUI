/*=========================================================================

						 >>> WonderGUI <<<

  This file is part of Tord Jansson's WonderGUI Graphics Toolkit
  and copyright (c) Tord Jansson, Sweden [tord.jansson@gmail.com].

							-----------

  The WonderGUI Graphics Toolkit is free Ioware; you can redistribute
  this file and/or modify it under the terms of the GNU General Public
  License as published by the Free Ioware Foundation; either
  version 2 of the License, or (at your option) any later version.

							-----------

  The WonderGUI Graphics Toolkit is also available for use in commercial
  closed-source projects under a separate license. Interested parties
  should contact Tord Jansson [tord.jansson@gmail.com] for details.

=========================================================================*/

#ifndef	WG_PLUGINEDGEMAP_DOT_H
#define	WG_PLUGINEDGEMAP_DOT_H
#pragma once


#include <wg_edgemap.h>

typedef void* wg_obj;
typedef struct wg_edgemapBP_struct wg_edgemapBP;

namespace wg
{

	class PluginEdgemap;
	typedef	StrongPtr<PluginEdgemap>	PluginEdgemap_p;
	typedef	WeakPtr<PluginEdgemap>	    PluginEdgemap_wp;

	//____ PluginEdgemap _____________________________________________________________

	class PluginEdgemap : public Edgemap
	{
		friend class PluginEdgemapFactory;

	public:

		//.____ Creation __________________________________________

		static PluginEdgemap_p	create( wg_obj object );

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		//.____ Appearance ____________________________________________________

		virtual bool	setRenderSegments(int nSegments);

		bool	setColors( int begin, int end, const HiColor * pColors ) override;
		bool	setGradients( int begin, int end, const Gradient * pGradients ) override;

		const HiColor * colors() const override;
		const Gradient * gradients() const override;

		HiColor		color(int segment) const override;
		Gradient	gradient(int segment) const override;


		//.____ Content _______________________________________________________

		bool 		importSamples(	SampleOrigo origo, const spx* pSource, int edgeBegin, int edgeEnd,
									int sampleBegin, int sampleEnd, int edgePitch = 0, int samplePitch = 0) override;

		bool 		importSamples(	SampleOrigo origo, const float* pSource, int edgeBegin, int edgeEnd,
									int sampleBegin, int sampleEnd, int edgePitch = 0, int samplePitch = 0) override;

		bool 		exportSamples(	SampleOrigo origo, spx* pDestination, int edgeBegin, int edgeEnd,
									int sampleBegin, int sampleEnd, int edgePitch = 0, int samplePitch = 0) override;

		bool 		exportSamples(	SampleOrigo origo, float* pDestination, int edgeBegin, int edgeEnd,
									int sampleBegin, int sampleEnd, int edgePitch = 0, int samplePitch = 0) override;

		
		//.____ Misc _________________________________________________________
		 
		inline wg_obj		cObject() { return m_cEdgemap; }

	private:
		PluginEdgemap( wg_obj object );
		~PluginEdgemap();

		void	_samplesUpdated(int edgeBegin, int edgeEnd, int sampleBegin, int sampleEnd) override;
		void	_colorsUpdated(int beginColor, int endColor) override;

		wg_obj		m_cEdgemap;
	};
} // namespace wg
#endif //WG_PLUGINEDGEMAP_DOT_H

