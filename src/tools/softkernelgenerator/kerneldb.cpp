#include "kerneldb.h"
#include <wg_enumextras.h>

using namespace wg;
using namespace std;

KernelDB::KernelDB()
{
	reset();
}

KernelDB::~KernelDB()
{

}

KernelDB::CustomBlitSpec::CustomBlitSpec()
{
	for (auto& e : blitTypes)
		e = false;

	for (auto& e : tintModes)
		e = false;

	for (auto& e : blendModes)
		e = false;

	for (auto& e : sourceFormats)
		e = false;

	for (auto& e : destFormats)
		e = false;

}


//____ setTintMode() __________________________________________________________

void KernelDB::setTintMode(TintMode mode, bool bOn)
{
	if (mode >= TintMode_min && mode <= TintMode_max && mode != TintMode::None)
		m_tintModes[int(mode)] = bOn;
}

//____ setBlendMode() _________________________________________________________

void KernelDB::setBlendMode(BlendMode mode, bool bOn)
{
	if (mode >= BlendMode_min && mode <= BlendMode_max && 
		mode != BlendMode::Undefined && mode != BlendMode::Ignore && 
		mode != BlendMode::Blend && mode != BlendMode::Replace )
		m_blendModes[int(mode)] = bOn;
}

//____ setSrcFormat() _________________________________________________________

void KernelDB::setSrcFormat(PixelFormat format, bool bOn)
{
	if (format >= PixelFormat_min && format <= PixelFormat_max &&
		format != PixelFormat::Undefined &&
		format != PixelFormat::BGRA_8 && format != PixelFormat::BGRX_8 &&
		format != PixelFormat::BGR_8 && format != PixelFormat::Index_8 )
		m_srcFormats[int(format)] = bOn;
}

//____ setDestFormat() ________________________________________________________

void KernelDB::setDestFormat(PixelFormat format, bool bOn)
{
	if (format >= PixelFormat_min && format <= PixelFormat_max &&
		format != PixelFormat::Undefined &&
		format != PixelFormat::BGRA_8 && format != PixelFormat::BGRX_8 &&
		format != PixelFormat::BGR_8 && format != PixelFormat::Index_8 &&
		format != PixelFormat::Index_8_linear && format != PixelFormat::Index_8_sRGB )
		m_destFormats[int(format)] = bOn;
}

//____ countKernels() _________________________________________________________

KernelDB::KernelCount KernelDB::countKernels()
{
	int nTintModes = 0;
	int nBlendModes = 0;
	int nSourceFormats = 0;
	int nDestFormats = 0;

	for (auto& e : m_tintModes)
		if (e) nTintModes++;

	for (auto& e : m_blendModes)
		if (e) nBlendModes++;
	
	for (auto& e : m_srcFormats)
		if (e) nSourceFormats++;

	for (auto& e : m_destFormats)
		if (e) nDestFormats++;

	// Remove duplicates that use the same kernel

	if (m_srcFormats[int(PixelFormat::BGR_8_linear)] && m_srcFormats[int(PixelFormat::BGRX_8_linear)])
		nSourceFormats--;

	if (m_srcFormats[int(PixelFormat::BGR_8_sRGB)] && m_srcFormats[int(PixelFormat::BGRX_8_sRGB)])
		nSourceFormats--;

	if (m_destFormats[int(PixelFormat::BGR_8_linear)] && m_destFormats[int(PixelFormat::BGRX_8_linear)])
		nDestFormats--;

	if (m_destFormats[int(PixelFormat::BGR_8_sRGB)] && m_destFormats[int(PixelFormat::BGRX_8_sRGB)])
		nDestFormats--;



	// Fill in values for draw kernels and fundamental blit kernels.

	KernelCount count;

	count.fill = nTintModes * nBlendModes * nDestFormats;
	count.line = nBlendModes * nDestFormats;
	count.clipLine = nBlendModes * nDestFormats;

	int nSegmentKernelTypes = 1;
	if( m_tintModes[int(TintMode::GradientY)])
		nSegmentKernelTypes++;
	if( m_tintModes[int(TintMode::GradientXY)])
		nSegmentKernelTypes++;

	count.segment = nBlendModes * nDestFormats * nSegmentKernelTypes;

	count.pass1blits_straight		= nSourceFormats * 4;
	count.pass1blits_straight_fast8	= nSourceFormats * 2;

	count.pass1blits_transform		= nSourceFormats * 2 * 3;
	count.pass1blits_transform_fast8 = nSourceFormats * 2 * 3;

	count.pass2blits = nTintModes * nBlendModes * nDestFormats;
	count.pass2blits_fast8 = nTintModes * nBlendModes * nDestFormats;		// Room for improvement?

	// Fill in values for transform custom kernels

	for (int i = 0; i < m_customBlits.size() ; i++ )
		count.customBlits += countEntryKernels(i);


	return count;
}


//____ countEntryKernels() ____________________________________________________

int KernelDB::countEntryKernels( int entry )
{
	auto& spec = m_customBlits[entry];

	int blitAndTile = 0;
	int nBlitTypes = 0;
	int nTintModes = 0;
	int nBlendModes = 0;
	int nSourceFormats = 0;
	int nDestFormats = 0;


	for (int i = 0; i < TintMode_size; i++)
		if (spec.tintModes[i] && m_tintModes[i])
			nTintModes++;

	for (int i = 0; i < BlendMode_size; i++)
		if (spec.blendModes[i] && m_blendModes[i])
			nBlendModes++;

	for (int i = 0; i < PixelFormat_size; i++)
		if (spec.sourceFormats[i] && m_srcFormats[i])
			nSourceFormats++;

	for (int i = 0; i < PixelFormat_size; i++)
		if (spec.destFormats[i] && m_destFormats[i])
			nDestFormats++;

	for (int i = 0; i < BlitType_size; i++)
		if (spec.blitTypes[i])
			nBlitTypes++;

	// Remove duplicates that use the same kernel

	if (m_srcFormats[int(PixelFormat::BGR_8_linear)] && m_srcFormats[int(PixelFormat::BGRX_8_linear)] && spec.sourceFormats[int(PixelFormat::BGR_8_linear)] && spec.sourceFormats[int(PixelFormat::BGRX_8_linear)])
		nSourceFormats--;

	if (m_srcFormats[int(PixelFormat::BGR_8_sRGB)] && m_srcFormats[int(PixelFormat::BGRX_8_sRGB)] && spec.sourceFormats[int(PixelFormat::BGR_8_sRGB)] && spec.sourceFormats[int(PixelFormat::BGRX_8_sRGB)])
		nSourceFormats--;

	if (m_destFormats[int(PixelFormat::BGR_8_linear)] && m_destFormats[int(PixelFormat::BGRX_8_linear)] && spec.destFormats[int(PixelFormat::BGR_8_linear)] && spec.destFormats[int(PixelFormat::BGRX_8_linear)])
		nDestFormats--;

	if (m_destFormats[int(PixelFormat::BGR_8_sRGB)] && m_destFormats[int(PixelFormat::BGRX_8_sRGB)] && spec.destFormats[int(PixelFormat::BGR_8_sRGB)] && spec.destFormats[int(PixelFormat::BGRX_8_sRGB)])
		nDestFormats--;


	return  nBlitTypes * nTintModes * nBlendModes * nSourceFormats * nDestFormats;
}

//____ toString() _____________________________________________________________

const char * toString(KernelDB::BlitType i)
{
	static const char * names[] = {
		"StraightBlit",
		"StraightTile",
		"StraightBlur",
		"TransformBlitNearest",
		"TransformClipBlitNearest",
		"TransformTileNearest",
		"TransformBlurNearest",
		"TransformBlitBilinear",
		"TransformClipBlitBilinear",
		"TransformTileBilinear",
		"TransformBlurBilinear" };

	return names[(int)i];
}

//____ generateSource() _______________________________________________________

bool KernelDB::generateSource(std::ostream& out, const std::string& kernelLabel )
{
	char temp[4096];


	bool bHasLinearSource = m_srcFormats[(int)PixelFormat::BGRA_4_linear] ||
							m_srcFormats[(int)PixelFormat::BGRA_8_linear] ||
							m_srcFormats[(int)PixelFormat::BGRX_8_linear] ||
							m_srcFormats[(int)PixelFormat::BGR_565_linear] ||
							m_srcFormats[(int)PixelFormat::BGR_8_linear] ||
							m_srcFormats[(int)PixelFormat::Index_8_linear] ||
							m_srcFormats[(int)PixelFormat::RGB_565_bigendian] ||
							m_srcFormats[(int)PixelFormat::RGB_555_bigendian] ||
							m_srcFormats[(int)PixelFormat::Alpha_8];

	bool bHasLinearDest =   m_destFormats[(int)PixelFormat::BGRA_4_linear] ||
							m_destFormats[(int)PixelFormat::BGRA_8_linear] ||
							m_destFormats[(int)PixelFormat::BGRX_8_linear] ||
							m_destFormats[(int)PixelFormat::BGR_565_linear] ||
							m_destFormats[(int)PixelFormat::BGR_8_linear] ||
							m_destFormats[(int)PixelFormat::RGB_565_bigendian] ||
							m_destFormats[(int)PixelFormat::RGB_555_bigendian] ||
							m_destFormats[(int)PixelFormat::Alpha_8];

	bool bHasSRGBSource =	m_srcFormats[(int)PixelFormat::BGRA_8_sRGB] ||
							m_srcFormats[(int)PixelFormat::BGRX_8_sRGB] ||
							m_srcFormats[(int)PixelFormat::BGR_8_sRGB] ||
							m_srcFormats[(int)PixelFormat::BGR_565_sRGB] ||
							m_srcFormats[(int)PixelFormat::Index_8_sRGB];

	bool bHasSRGBDest =		m_destFormats[(int)PixelFormat::BGRA_8_sRGB] ||
							m_destFormats[(int)PixelFormat::BGRX_8_sRGB] ||
							m_destFormats[(int)PixelFormat::BGR_565_sRGB] ||
							m_destFormats[(int)PixelFormat::BGR_8_sRGB];

	bool bUseFast8Blits = bHasLinearSource && bHasLinearDest;

	bool bUseSRGBBlits = bHasSRGBSource || bHasSRGBDest;

	// Print out some info
	
	out << "/*******************************************************************************" << endl << endl;
	out << "This file has been generated by SoftKernelGenerator and controls what" << endl;
	out << "TintModes, BlendModes and PixelFormats are supported in software rendering as" << endl;
	out << "well as what blit operations have custom/optimized implementations." << endl;
	out << endl;
	out << "The more supported, the more code is generated, resulting in a larger binary." << endl;
	out << endl;
	out << "Generating optimized blits for all possible combinations can result in tens of" << endl;
	out << "thousands of optimized render loops, growing the size of WonderGUI by many" << endl;
	out << "megabytes." << endl;
	out << endl;
	out << "For modern desktop applications wg_softkernels_default.h/cpp provides a good" << endl;
	out << "balance between size and performance, supporting everyting but only optimizing" << endl;
	out << "most important blit operations." << endl;
	out << endl;
	out << "********************************************************************************" << endl << endl;
	out << "SUPPORTED MODES AND FORMATS " << endl << endl;

		// TintModes
	{
		bool bAllSupported = true;
		out << "Tint modes:            ";
		for (int i = 0; i < TintMode_size; i++)
		{
			if( m_tintModes[i] )
				out << wg::toString( (TintMode) i ) << endl << "                       ";
			else
				bAllSupported = false;
		}

		out << "(unsupported:";
		if( bAllSupported )
			out << " none";
		else
		{
			for (int i = 0; i < TintMode_size; i++)
			{
				if( !m_tintModes[i] )
					out << " " << wg::toString( (TintMode) i );
			}
		}
		out << ")" << endl << endl;
	}


		// BlendModes
	{
		bool bAllSupported = true;
		out << "Blend modes:           ";
		for (int i = 0; i < BlendMode_size; i++)
		{
			if( m_blendModes[i] )
				out << wg::toString( (BlendMode) i ) << endl << "                       ";
			else if( i != int(BlendMode::Undefined) && i != int(BlendMode::Ignore) )
				bAllSupported = false;
		}

		out << "(unsupported:";
		if( bAllSupported )
			out << " none";
		else
		{
			for (int i = 0; i < BlendMode_size; i++)
			{
				if( !m_blendModes[i] && i != int(BlendMode::Undefined) && i != int(BlendMode::Ignore) )
					out << " " << wg::toString( (BlendMode) i );
			}
		}
		out << ")" << endl << endl;
	}
	
		// SourceFormats
	{
		bool bAllSupported = true;
		out << "Source formats:        ";
		for (int i = 0; i < PixelFormat_size; i++)
		{
			if( m_srcFormats[i] )
				out << wg::toString( (PixelFormat) i ) << endl << "                       ";
			else if( i != int(PixelFormat::Undefined) && i != int(PixelFormat::BGR_8) && i != int(PixelFormat::BGRX_8) && i != int(PixelFormat::BGRA_8) && i != int(PixelFormat::BGR_565) && i != int(PixelFormat::Index_8) )
				bAllSupported = false;
		}

		out << "(unsupported:";
		if( bAllSupported )
			out << " none";
		else
		{
			for (int i = 0; i < PixelFormat_size; i++)
			{
				if( !m_srcFormats[i] &&  i != int(PixelFormat::Undefined) && i != int(PixelFormat::BGR_8) && i != int(PixelFormat::BGRX_8) && i != int(PixelFormat::BGRA_8) && i != int(PixelFormat::BGR_565) && i != int(PixelFormat::Index_8)  )
					out << " " << wg::toString( (PixelFormat) i );
			}
		}
		out << ")" << endl << endl;
	}

		// DestinationFormats
	{
		bool bAllSupported = true;
		out << "Destination formats:   ";
		for (int i = 0; i < PixelFormat_size; i++)
		{
			if( m_destFormats[i] )
				out << wg::toString( (PixelFormat) i ) << endl << "                       ";
			else if( i != int(PixelFormat::Undefined) && i != int(PixelFormat::BGR_8) && i != int(PixelFormat::BGRX_8) && i != int(PixelFormat::BGRA_8) && i != int(PixelFormat::BGR_565) && i != int(PixelFormat::Index_8) && i != int(PixelFormat::Index_8_sRGB) && i != int(PixelFormat::Index_8_linear) )
				bAllSupported = false;
		}

		out << "(unsupported:";
		if( bAllSupported )
			out << " none";
		else
		{
			for (int i = 0; i < PixelFormat_size; i++)
			{
				if( !m_destFormats[i] &&  i != int(PixelFormat::Undefined) && i != int(PixelFormat::BGR_8) && i != int(PixelFormat::BGRX_8) && i != int(PixelFormat::BGRA_8) && i != int(PixelFormat::BGR_565) && i != int(PixelFormat::Index_8) && i != int(PixelFormat::Index_8_sRGB) && i != int(PixelFormat::Index_8_linear) )
					out << " " << wg::toString( (PixelFormat) i );
			}
		}
		out << ")" << endl << endl;
	}

	int customCounter = 1;
	for( auto& blitSpec : m_customBlits )
	{
		out << endl << "CUSTOM BLIT KERNELS DEFINITION #" << customCounter << endl << endl;

		out << "Operations:         ";
		for( int i = 0 ; i < BlitType_size ; i++ )
		{
			if( blitSpec.blitTypes[i] )
				out << toString( (BlitType)i ) << endl << "                    ";
		}
		out << endl;

		out << "Tint Modes:         ";
		for( int i = 0 ; i < TintMode_size ; i++ )
		{
			if( blitSpec.tintModes[i] )
				out << toString( (TintMode)i ) << endl << "                    ";
		}
		out << endl;

		out << "Blend Modes:        ";
		for( int i = 0 ; i < BlendMode_size ; i++ )
		{
			if( blitSpec.blendModes[i] )
				out << toString( (BlendMode)i ) << endl << "                    ";
		}
		out << endl;

		out << "Source Formats:     ";
		for( int i = 0 ; i < PixelFormat_size ; i++ )
		{
			if( blitSpec.sourceFormats[i] && m_srcFormats[i] )
				out << toString( (PixelFormat)i ) << endl << "                    ";
		}
		out << endl;

		out << "Destination Formats:";
		for( int i = 0 ; i < PixelFormat_size ; i++ )
		{
			if( blitSpec.sourceFormats[i] && m_destFormats[i] )
				out << toString( (PixelFormat)i ) << endl << "                    ";
		}
		out << endl;
	}
	
/*
	KernelCount kernels = countKernels();
	
	out << endl << "    Custom blit kernels: " << kernels.customBlits << endl;
 */

	
	out << endl << "*******************************************************************************/" << endl << endl;

	
	

	// Print out initial stanza

	out << "#include <wg_softbackend.h>" << endl;
	out << "#include <wg_softkernelstanza.impl.h>" << endl;
	out << "#include <wg_c_object.h>" << endl;

	out << endl;
	out << "using namespace wg;" << endl;
	out << endl;

	out << "namespace wg" << endl;
	out << "{" << endl;
	out << "	bool " << kernelLabel << "( SoftBackend * pDevice );" << endl;
	out << "};" << endl << endl;
	
	out << "#ifdef __cplusplus" << endl;
	out << "extern \"C\" {" << endl;
	out << "#endif" << endl;
	out << "int	wg_" << kernelLabel << "( wg_obj backend )" << endl;
	out << "{" << endl;
	out << "	auto pBackend = static_cast<SoftBackend*>(reinterpret_cast<Object*>(backend));" << endl;
	out << endl;
	out << "	return " << kernelLabel << "(pBackend);" << endl;
	out << "}" << endl << endl;
	out << "#ifdef __cplusplus" << endl;
	out << "}" << endl;
	out << "#endif" << endl;


	out << endl;
	out << "bool wg::" << kernelLabel << "( SoftBackend * pBackend )" << endl;
	out << "{" << endl;

	// Print out the fill kernels.

	for (int tintMode = 0; tintMode < TintMode_size; tintMode++)
	{
		if (m_tintModes[tintMode])
		{
			for (int blendMode = 0; blendMode < BlendMode_size; blendMode++)
			{
				if (m_blendModes[blendMode])
				{
					for (int destFormat = 0; destFormat < PixelFormat_size; destFormat++)
					{
						if (m_destFormats[destFormat])
						{
							auto pBlend = toString((BlendMode)blendMode);
							auto pFormat = toString((PixelFormat)destFormat);
							auto pTint = toString((TintMode)tintMode);

							snprintf(temp, 4096, "pBackend->setFillKernel( TintMode::%s, BlendMode::%s, PixelFormat::%s, _fill<TintMode::%s,BlendMode::%s, PixelFormat::%s> );\n",
								pTint, pBlend, pFormat, pTint, pBlend, pFormat);

							out << temp;
						}
					}
				}
			}
			out << endl;
		}
	}

	// Print out the line kernels.

	for (int blendMode = 0; blendMode < BlendMode_size; blendMode++)
	{
		if (m_blendModes[blendMode])
		{
			for (int destFormat = 0; destFormat < PixelFormat_size; destFormat++)
			{
				if (m_destFormats[destFormat])
				{
					auto pBlend = toString((BlendMode)blendMode);
					auto pFormat = toString((PixelFormat)destFormat);

					snprintf(temp, 4096, "pBackend->setLineKernel( BlendMode::%s, PixelFormat::%s, _draw_line<BlendMode::%s, TintMode::None, PixelFormat::%s> );\n",
						pBlend, pFormat, pBlend, pFormat);

					out << temp;
				}
			}
			out << endl;
		}
	}

	// Print out the clip line kernels.

	for (int blendMode = 0; blendMode < BlendMode_size; blendMode++)
	{
		if (m_blendModes[blendMode])
		{
			for (int destFormat = 0; destFormat < PixelFormat_size; destFormat++)
			{
				if (m_destFormats[destFormat])
				{
					auto pBlend = toString((BlendMode)blendMode);
					auto pFormat = toString((PixelFormat)destFormat);

					snprintf(temp, 4096, "pBackend->setClipLineKernel( BlendMode::%s, PixelFormat::%s, _clip_draw_line<BlendMode::%s, TintMode::None, PixelFormat::%s> );\n",
						pBlend, pFormat, pBlend, pFormat);

					out << temp;
				}
			}
			out << endl;
		}
	}

	// Print out the segment kernels.

	for (int blendMode = 0; blendMode < BlendMode_size; blendMode++)
	{
		if (m_blendModes[blendMode])
		{
			for (int destFormat = 0; destFormat < PixelFormat_size; destFormat++)
			{
				if (m_destFormats[destFormat])
				{
					auto pBlend = toString((BlendMode)blendMode);
					auto pFormat = toString((PixelFormat)destFormat);

					if (m_tintModes[(int)TintMode::None] || m_tintModes[(int)TintMode::Flat] || m_tintModes[(int)TintMode::GradientX])
					{
						snprintf(temp, 4096, "pBackend->setSegmentStripKernel( SoftBackend::StripSource::Colors, BlendMode::%s, PixelFormat::%s,  _draw_segment_strip<SoftBackend::StripSource::Colors, BlendMode::%s, PixelFormat::%s> );\n",
										pBlend, pFormat, pBlend, pFormat);
					}

					if (m_tintModes[(int)TintMode::GradientY])
					{
						snprintf(temp, 4096, "pBackend->setSegmentStripKernel( SoftBackend::StripSource::Tintmaps, BlendMode::%s, PixelFormat::%s, _draw_segment_strip<SoftBackend::StripSource::Tintmaps, BlendMode::%s, PixelFormat::%s> );\n",
							pBlend, pFormat, pBlend, pFormat);
					}

					if (m_tintModes[(int)TintMode::GradientXY])
					{
						snprintf(temp, 4096, "pBackend->setSegmentStripKernel( SoftBackend::StripSource::ColorsAndTintmaps, BlendMode::%s, PixelFormat::%s, _draw_segment_strip<SoftBackend::StripSource::ColorsAndTintmaps, BlendMode::%s, PixelFormat::%s> );\n",
							pBlend, pFormat, pBlend, pFormat);
					}


					out << temp;
				}
			}
			out << endl;
		}
	}

	// Print out the blit pass 2 kernels.
	// We only need these if we support SRGB since we otherwise always take the fast8-route.

	if (bUseSRGBBlits)
	{
		for (int tintMode = 0; tintMode < TintMode_size; tintMode++)
		{
			if (m_tintModes[tintMode])
			{
				for (int blendMode = 0; blendMode < BlendMode_size; blendMode++)
				{
					if (m_blendModes[blendMode])
					{
						for (int destFormat = 0; destFormat < PixelFormat_size; destFormat++)
						{
							if (m_destFormats[destFormat])
							{
								auto pBlend = toString((BlendMode)blendMode);
								auto pFormat = toString((PixelFormat)destFormat);
								auto pTint = toString((TintMode)tintMode);

								snprintf(temp, 4096, "pBackend->setStraightBlitKernel( PixelFormat::Undefined, SoftBackend::ReadOp::Normal, TintMode::%s, BlendMode::%s, PixelFormat::%s, _straight_blit<PixelFormat::Undefined, TintMode::%s, BlendMode::%s, PixelFormat::%s, SoftBackend::ReadOp::Normal> );\n",
									pTint, pBlend, pFormat, pTint, pBlend, pFormat);

								out << temp;
							}
						}
					}
				}
				out << endl;
			}
		}
	}

	// Print out the blit pass 2 kernels - fast 8 version.

	if (bUseFast8Blits)
	{
		for (int tintMode = 0; tintMode < TintMode_size; tintMode++)
		{
			if (m_tintModes[tintMode])
			{
				for (int blendMode = 0; blendMode < BlendMode_size; blendMode++)
				{
					if (m_blendModes[blendMode])
					{
						for (int destFormat = 0; destFormat < PixelFormat_size; destFormat++)
						{
							if (m_destFormats[destFormat])
							{
								auto pBlend = toString((BlendMode)blendMode);
								auto pFormat = toString((PixelFormat)destFormat);
								auto pTint = toString((TintMode)tintMode);

								snprintf(temp, 4096, "pBackend->setStraightBlitKernel( PixelFormat::BGRA_8_linear, SoftBackend::ReadOp::Normal, TintMode::%s, BlendMode::%s, PixelFormat::%s, _straight_blit<PixelFormat::BGRA_8_linear, TintMode::%s, BlendMode::%s, PixelFormat::%s, SoftBackend::ReadOp::Normal> );\n",
									pTint, pBlend, pFormat, pTint, pBlend, pFormat);

								out << temp;
							}
						}
					}
				}
				out << endl;
			}
		}
	}
	else
	{
		// We still need to support 8 bit straight blits (no tint, no blend).

		for (int destFormat = 0; destFormat < PixelFormat_size; destFormat++)
		{
			if (m_destFormats[destFormat])
			{
				auto pFormat = toString((PixelFormat)destFormat);
	
				snprintf(temp, 4096, "pBackend->setStraightBlitKernel( PixelFormat::BGRA_8_linear, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Replace, PixelFormat::%s, _straight_blit<PixelFormat::BGRA_8_linear, TintMode::None, BlendMode::Replace, PixelFormat::%s, SoftBackend::ReadOp::Normal> );\n",
					pFormat, pFormat);

				out << temp;
			}
		}
		out << endl;
	}

	// print out the simple blit pass 1 kernels

	for (int srcFormat = 0; srcFormat < PixelFormat_size; srcFormat++)
	{
		if (m_srcFormats[srcFormat] )
		{
			auto pFormat = toString((PixelFormat)srcFormat);

			snprintf(temp, 4096, "pBackend->setStraightBlitKernel( PixelFormat::%s, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, _straight_blit<PixelFormat::%s, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, SoftBackend::ReadOp::Normal> );\n",
				pFormat, pFormat);
			out << temp;

			snprintf(temp, 4096, "pBackend->setStraightBlitKernel( PixelFormat::%s, SoftBackend::ReadOp::Tile, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, _straight_blit<PixelFormat::%s, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, SoftBackend::ReadOp::Tile> );\n",
				pFormat, pFormat);
			out << temp;

			snprintf(temp, 4096, "pBackend->setStraightBlitKernel( PixelFormat::%s, SoftBackend::ReadOp::Blur, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, _straight_blit<PixelFormat::%s, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, SoftBackend::ReadOp::Blur> );\n",
				pFormat, pFormat);
			out << temp;
		}
	}
	out << endl;

	// print out fast8 version of simple blit pass 1 kernels. Blur never uses fastblit.

	if (bUseFast8Blits)
	{
		for (int srcFormat = 0; srcFormat < PixelFormat_size; srcFormat++)
		{
			if (m_srcFormats[srcFormat])
			{
				auto pFormat = toString((PixelFormat)srcFormat);

				snprintf(temp, 4096, "pBackend->setStraightBlitKernel( PixelFormat::%s, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, _straight_blit<PixelFormat::%s, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, SoftBackend::ReadOp::Normal> );\n",
					pFormat, pFormat);
				out << temp;

				snprintf(temp, 4096, "pBackend->setStraightBlitKernel( PixelFormat::%s, SoftBackend::ReadOp::Tile, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, _straight_blit<PixelFormat::%s, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, SoftBackend::ReadOp::Tile> );\n",
					pFormat, pFormat);
				out << temp;

				snprintf(temp, 4096, "pBackend->setStraightBlitKernel( PixelFormat::%s, SoftBackend::ReadOp::Blur, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, _straight_blit<PixelFormat::%s, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, SoftBackend::ReadOp::Blur> );\n",
					pFormat, pFormat);
				out << temp;
			}
		}
		out << endl;
	}


	// print out the complex blit pass 1 kernels

	for (int srcFormat = 0; srcFormat < PixelFormat_size; srcFormat++)
	{
		if (m_srcFormats[srcFormat])
		{
			auto pFormat = toString((PixelFormat)srcFormat);

			snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Nearest, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Replace, PixelFormat::Undefined,  _transform_blit<PixelFormat::%s, SampleMethod::Nearest, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, SoftBackend::ReadOp::Normal> );\n",
				pFormat, pFormat);
			out << temp;

			snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Nearest, SoftBackend::ReadOp::Clip, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, _transform_blit<PixelFormat::%s, SampleMethod::Nearest, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, SoftBackend::ReadOp::Clip> );\n",
				pFormat, pFormat);
			out << temp;

			snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Nearest, SoftBackend::ReadOp::Tile, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, _transform_blit<PixelFormat::%s, SampleMethod::Nearest, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, SoftBackend::ReadOp::Tile> );\n",
				pFormat, pFormat);
			out << temp;

			snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Nearest, SoftBackend::ReadOp::Blur, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, _transform_blit<PixelFormat::%s, SampleMethod::Nearest, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, SoftBackend::ReadOp::Blur> );\n",
				pFormat, pFormat);
			out << temp;

			
			
			snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Bilinear, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Replace, PixelFormat::Undefined,  _transform_blit<PixelFormat::%s, SampleMethod::Bilinear, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, SoftBackend::ReadOp::Normal> );\n",
				pFormat, pFormat);
			out << temp;

			snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Bilinear, SoftBackend::ReadOp::Clip, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, _transform_blit<PixelFormat::%s, SampleMethod::Bilinear, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, SoftBackend::ReadOp::Clip> );\n",
				pFormat, pFormat);
			out << temp;

			snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Bilinear, SoftBackend::ReadOp::Tile, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, _transform_blit<PixelFormat::%s, SampleMethod::Bilinear, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, SoftBackend::ReadOp::Tile> );\n",
				pFormat, pFormat);
			out << temp;

			snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Bilinear, SoftBackend::ReadOp::Blur, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, _transform_blit<PixelFormat::%s, SampleMethod::Bilinear, TintMode::None, BlendMode::Replace, PixelFormat::Undefined, SoftBackend::ReadOp::Blur> );\n",
				pFormat, pFormat);
			out << temp;

		}
	}
	out << endl;

	// print out fast8 version of the complex blit pass 1 kernels

	if (bUseFast8Blits)
	{
		for (int srcFormat = 0; srcFormat < PixelFormat_size; srcFormat++)
		{
			if (m_srcFormats[srcFormat])
			{
				auto pFormat = toString((PixelFormat)srcFormat);

				snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Nearest, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear,  _transform_blit<PixelFormat::%s, SampleMethod::Nearest, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, SoftBackend::ReadOp::Normal> );\n",
					pFormat, pFormat);
				out << temp;

				snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Nearest, SoftBackend::ReadOp::Clip, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, _transform_blit<PixelFormat::%s, SampleMethod::Nearest, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, SoftBackend::ReadOp::Clip> );\n",
					pFormat, pFormat);
				out << temp;

				snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Nearest, SoftBackend::ReadOp::Tile, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, _transform_blit<PixelFormat::%s, SampleMethod::Nearest, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, SoftBackend::ReadOp::Tile> );\n",
					pFormat, pFormat);
				out << temp;

				snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Nearest, SoftBackend::ReadOp::Blur, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, _transform_blit<PixelFormat::%s, SampleMethod::Nearest, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, SoftBackend::ReadOp::Blur> );\n",
					pFormat, pFormat);
				out << temp;

				
				snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Bilinear, SoftBackend::ReadOp::Normal, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear,  _transform_blit<PixelFormat::%s, SampleMethod::Bilinear, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, SoftBackend::ReadOp::Normal> );\n",
					pFormat, pFormat);
				out << temp;

				snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Bilinear, SoftBackend::ReadOp::Clip, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, _transform_blit<PixelFormat::%s, SampleMethod::Bilinear, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, SoftBackend::ReadOp::Clip> );\n",
					pFormat, pFormat);
				out << temp;

				snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Bilinear, SoftBackend::ReadOp::Tile, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, _transform_blit<PixelFormat::%s, SampleMethod::Bilinear, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, SoftBackend::ReadOp::Tile> );\n",
					pFormat, pFormat);
				out << temp;

				snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Bilinear, SoftBackend::ReadOp::Blur, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, _transform_blit<PixelFormat::%s, SampleMethod::Bilinear, TintMode::None, BlendMode::Replace, PixelFormat::BGRA_8_linear, SoftBackend::ReadOp::Blur> );\n",
					pFormat, pFormat);
				out << temp;

			}
		}
		out << endl;

	}

	// Print out one pass straight blit kernels

	for (auto& entry : m_customBlits)
	{
		for (int tintMode = 0; tintMode < TintMode_size; tintMode++)
		{
			for (int blendMode = 0; blendMode < BlendMode_size; blendMode++)
			{
				for (int srcFmt = 0; srcFmt < PixelFormat_size; srcFmt++)
				{
					for (int dstFmt = 0; dstFmt < PixelFormat_size; dstFmt++)
					{
						if (entry.tintModes[tintMode] && entry.blendModes[blendMode] && entry.sourceFormats[srcFmt] && m_srcFormats[srcFmt] && entry.destFormats[dstFmt] && m_destFormats[dstFmt])
						{
							if (entry.blitTypes[(int)BlitType::StraightBlit])
							{
								snprintf(temp, 4096, "pBackend->setStraightBlitKernel( PixelFormat::%s, SoftBackend::ReadOp::Normal, TintMode::%s, BlendMode::%s, PixelFormat::%s, _straight_blit<PixelFormat::%s, TintMode::%s, BlendMode::%s, PixelFormat::%s, SoftBackend::ReadOp::Normal> );\n",
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt),
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt));
								
								out << temp;
							}

							if (entry.blitTypes[(int)BlitType::StraightTile])
							{
								snprintf(temp, 4096, "pBackend->setStraightBlitKernel( PixelFormat::%s, SoftBackend::ReadOp::Tile, TintMode::%s, BlendMode::%s, PixelFormat::%s, _straight_blit<PixelFormat::%s, TintMode::%s, BlendMode::%s, PixelFormat::%s, SoftBackend::ReadOp::Tile> );\n",
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt),
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt));

								out << temp;
							}

							if (entry.blitTypes[(int)BlitType::StraightBlur])
							{
								snprintf(temp, 4096, "pBackend->setStraightBlitKernel( PixelFormat::%s, SoftBackend::ReadOp::Blur, TintMode::%s, BlendMode::%s, PixelFormat::%s, _straight_blit<PixelFormat::%s, TintMode::%s, BlendMode::%s, PixelFormat::%s, SoftBackend::ReadOp::Blur> );\n",
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt),
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt));

								out << temp;
							}

						}

					}
				}
			}
		}
	}
	out << endl;

	// Print out one pass transform blit kernels

	for (auto& entry : m_customBlits)
	{
		for (int tintMode = 0; tintMode < TintMode_size; tintMode++)
		{
			for (int blendMode = 0; blendMode < BlendMode_size; blendMode++)
			{
				for (int srcFmt = 0; srcFmt < PixelFormat_size; srcFmt++)
				{
					for (int dstFmt = 0; dstFmt < PixelFormat_size; dstFmt++)
					{
						if (entry.tintModes[tintMode] && entry.blendModes[blendMode] && entry.sourceFormats[srcFmt] && m_srcFormats[srcFmt] && entry.destFormats[dstFmt] && m_destFormats[dstFmt])
						{
							if (entry.blitTypes[(int)BlitType::TransformBlitNearest])
							{
								snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Nearest, SoftBackend::ReadOp::Normal, TintMode::%s, BlendMode::%s, PixelFormat::%s, _transform_blit<PixelFormat::%s, SampleMethod::Nearest, TintMode::%s, BlendMode::%s, PixelFormat::%s, SoftBackend::ReadOp::Normal> );\n",
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt),
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt));
								
								out << temp;
							}

							if (entry.blitTypes[(int)BlitType::TransformBlitBilinear])
							{
								snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Bilinear, SoftBackend::ReadOp::Normal, TintMode::%s, BlendMode::%s, PixelFormat::%s, _transform_blit<PixelFormat::%s, SampleMethod::Bilinear, TintMode::%s, BlendMode::%s, PixelFormat::%s, SoftBackend::ReadOp::Normal> );\n",
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt),
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt));
								
								out << temp;
							}


							if (entry.blitTypes[(int)BlitType::TransformClipBlitNearest])
							{
								snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Nearest, SoftBackend::ReadOp::Clip, TintMode::%s, BlendMode::%s, PixelFormat::%s, _transform_blit<PixelFormat::%s, SampleMethod::Nearest, TintMode::%s, BlendMode::%s, PixelFormat::%s, SoftBackend::ReadOp::Clip> );\n",
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt),
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt));
								
								out << temp;
							}

							if (entry.blitTypes[(int)BlitType::TransformClipBlitBilinear])
							{
								snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Bilinear, SoftBackend::ReadOp::Clip, TintMode::%s, BlendMode::%s, PixelFormat::%s, _transform_blit<PixelFormat::%s, SampleMethod::Bilinear, TintMode::%s, BlendMode::%s, PixelFormat::%s, SoftBackend::ReadOp::Clip> );\n",
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt),
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt));
								
								out << temp;
							}

							if (entry.blitTypes[(int)BlitType::TransformTileNearest])
							{
								snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Nearest, SoftBackend::ReadOp::Tile, TintMode::%s, BlendMode::%s, PixelFormat::%s, _transform_blit<PixelFormat::%s, SampleMethod::Nearest, TintMode::%s, BlendMode::%s, PixelFormat::%s, SoftBackend::ReadOp::Tile> );\n",
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt),
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt));
								
								out << temp;
							}

							if (entry.blitTypes[(int)BlitType::TransformTileBilinear])
							{
								snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Bilinear, SoftBackend::ReadOp::Tile, TintMode::%s, BlendMode::%s, PixelFormat::%s, _transform_blit<PixelFormat::%s, SampleMethod::Bilinear, TintMode::%s, BlendMode::%s, PixelFormat::%s, SoftBackend::ReadOp::Tile> );\n",
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt),
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt));
								
								out << temp;
							}
							
							if (entry.blitTypes[(int)BlitType::TransformBlurNearest])
							{
								snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Nearest, SoftBackend::ReadOp::Blur, TintMode::%s, BlendMode::%s, PixelFormat::%s, _transform_blit<PixelFormat::%s, SampleMethod::Nearest, TintMode::%s, BlendMode::%s, PixelFormat::%s, SoftBackend::ReadOp::Blur> );\n",
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt),
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt));
								
								out << temp;
							}

							if (entry.blitTypes[(int)BlitType::TransformBlurBilinear])
							{
								snprintf(temp, 4096, "pBackend->setTransformBlitKernel( PixelFormat::%s, SampleMethod::Bilinear, SoftBackend::ReadOp::Blur, TintMode::%s, BlendMode::%s, PixelFormat::%s, _transform_blit<PixelFormat::%s, SampleMethod::Bilinear, TintMode::%s, BlendMode::%s, PixelFormat::%s, SoftBackend::ReadOp::Blur> );\n",
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt),
									toString((PixelFormat)srcFmt),
									toString((TintMode)tintMode),
									toString((BlendMode)blendMode),
									toString((PixelFormat)dstFmt));
								
								out << temp;
							}

						}

					}
				}
			}
		}
	}



	// Print end of method

	out << endl << "    return true;" << endl;
	out << "}" << endl;


	return true;
}


//____ clear() _____________________________________________________________

void KernelDB::clear()
{
	// Set all BlitTypes, TintModes and BlendModes

	for (bool& b : m_tintModes)
		b = false;

	for (bool& b : m_blendModes)
		b = false;

	m_blendModes[int(BlendMode::Replace)] = true;
	m_blendModes[int(BlendMode::Blend)] = true;

	m_tintModes[int(TintMode::None)] = true;

	// Clear all Source and Dest formats (specifics will be set later).

	for (bool& b : m_srcFormats)
		b = false;

	for (bool& b : m_destFormats)
		b = false;

	// Clear all custom blits.

	m_customBlits.clear();
}


//____ _reset() _____________________________________________________________

void KernelDB::reset()
{
	// Set all TintModes and BlendModes

	for (bool& b : m_tintModes)
		b = true;

	for (bool& b : m_blendModes)
		b = true;

	m_blendModes[int(BlendMode::Undefined)] = false;
	m_blendModes[int(BlendMode::Ignore)] = false;


	// Clear all Source and Dest formats (specifics will be set later).

	for (bool& b : m_srcFormats)
		b = false;

	for (bool& b : m_destFormats)
		b = false;

	// Set standard source formats

	m_srcFormats[int(PixelFormat::BGR_8_sRGB)] = true;
	m_srcFormats[int(PixelFormat::BGR_8_linear)] = true;
	m_srcFormats[int(PixelFormat::BGRX_8_sRGB)] = true;
	m_srcFormats[int(PixelFormat::BGRX_8_linear)] = true;
	m_srcFormats[int(PixelFormat::BGRA_8_sRGB)] = true;
	m_srcFormats[int(PixelFormat::BGRA_8_linear)] = true;
	m_srcFormats[int(PixelFormat::BGRA_4_linear)] = true;
	m_srcFormats[int(PixelFormat::BGR_565_linear)] = true;
	m_srcFormats[int(PixelFormat::BGR_565_sRGB)] = true;
	m_srcFormats[int(PixelFormat::Index_8_sRGB)] = true;
	m_srcFormats[int(PixelFormat::Index_8_linear)] = true;
	m_srcFormats[int(PixelFormat::RGB_565_bigendian)] = true;
	m_srcFormats[int(PixelFormat::RGB_555_bigendian)] = true;
	m_srcFormats[int(PixelFormat::Alpha_8)] = true;

	// Set standard destination formats

	m_destFormats[int(PixelFormat::BGR_8_sRGB)] = true;
	m_destFormats[int(PixelFormat::BGR_8_linear)] = true;
	m_destFormats[int(PixelFormat::BGRX_8_sRGB)] = true;
	m_destFormats[int(PixelFormat::BGRX_8_linear)] = true;
	m_destFormats[int(PixelFormat::BGRA_8_sRGB)] = true;
	m_destFormats[int(PixelFormat::BGRA_8_linear)] = true;
	m_destFormats[int(PixelFormat::BGRA_4_linear)] = true;
	m_destFormats[int(PixelFormat::BGR_565_linear)] = true;
	m_destFormats[int(PixelFormat::BGR_565_sRGB)] = true;
	m_destFormats[int(PixelFormat::RGB_565_bigendian)] = true;
	m_destFormats[int(PixelFormat::RGB_555_bigendian)] = true;
	m_destFormats[int(PixelFormat::Alpha_8)] = true;

	// Set custom blit methods
	{
		m_customBlits.clear();

		CustomBlitSpec spec;

		for (bool& b : spec.blitTypes)
			b = true;

		spec.tintModes[int(TintMode::None)] = true;

		spec.blendModes[int(BlendMode::Blend)] = true;
		spec.blendModes[int(BlendMode::Replace)] = true;

		spec.destFormats[int(PixelFormat::BGRA_8_linear)] = true;
		spec.destFormats[int(PixelFormat::BGRA_8_sRGB)] = true;

		spec.destFormats[int(PixelFormat::BGRX_8_linear)] = true;
		spec.destFormats[int(PixelFormat::BGRX_8_sRGB)] = true;

		spec.destFormats[int(PixelFormat::BGR_8_linear)] = true;
		spec.destFormats[int(PixelFormat::BGR_8_sRGB)] = true;

		for (int srcFormat = 0; srcFormat < PixelFormat_size; srcFormat++)
		{
			if (m_srcFormats[srcFormat])
				spec.sourceFormats[srcFormat] = true;
		}

		m_customBlits.push_back(spec);
	}

}
