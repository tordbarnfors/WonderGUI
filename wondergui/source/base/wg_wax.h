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
#ifndef WG_WAX_DOT_H
#define WG_WAX_DOT_H
#pragma once

#include <wax.h>

//____ Wax ____________________________________________________________________
//
// WonderGUI is built on top of the Wax graphics library. The Wax classes,
// types and functions that are part of WonderGUI's API are brought into the
// wg namespace here, so they can be used as wg::Surface, wg::GfxDevice etc.
//
// Only what is listed here becomes part of wg. Remove a line to keep that
// name in wax only, or add an alias to give it a different name in wg, e.g.:
//
//     using GfxDevice = wax::Device;
//
// Wax extensions, like SoftBackend or StreamEncoder, are not brought in.
// Refer to them as wax::SoftBackend etc.
//
// Don't forward-declare wax classes inside namespace wg, that conflicts with
// the declarations here. Forward-declare them in namespace wax instead.

namespace wg
{

	//____ wax_backendlogger.h _____________________________________________________

	using wax::BackendLogger_p;
	using wax::BackendLogger_wp;
	using wax::BackendLogger;

	//____ wax_bitmapcache.h _______________________________________________________

	using wax::BitmapCache_p;
	using wax::BitmapCache_wp;
	using wax::BitmapCacheListener;
	using wax::BitmapCache;

	//____ wax_bitmapfont.h ________________________________________________________

	using wax::BitmapFont_p;
	using wax::BitmapFont_wp;
	using wax::BitmapFont;

	//____ wax_blob.h ______________________________________________________________

	using wax::Blob_p;
	using wax::Blob_wp;
	using wax::Blob;

	//____ wax_blurbrush.h _________________________________________________________

	using wax::Blurbrush_p;
	using wax::Blurbrush_wp;
	using wax::Blurbrush;

	//____ wax_canvaslayers.h ______________________________________________________

	using wax::CanvasLayers_p;
	using wax::CanvasLayers_wp;
	using wax::CanvasLayers;

	//____ wax_chain.h _____________________________________________________________

	using wax::ChainImp;
	using wax::Chain;
	using wax::Link;

	//____ wax_circularwarp.h ______________________________________________________

	using wax::CircularWarp_p;
	using wax::CircularWarp_wp;
	using wax::CircularWarp;

	//____ wax_color.h _____________________________________________________________

	using wax::Color8;
	using wax::Color;
	using wax::HiColor;

	//____ wax_componentptr.h ______________________________________________________

	using wax::ComponentPtr;

	//____ wax_compression.h _______________________________________________________

	using wax::Compressor_p;
	using wax::Compressor_wp;
	using wax::Decompressor_p;
	using wax::Decompressor_wp;
	using wax::Compressor;
	using wax::Decompressor;

	//____ wax_core.h ______________________________________________________________

	using wax::SurfaceFactory_p;
	using GfxDeviceFactory_p = wax::DeviceFactory_p;
	using wax::Error;

	//____ wax_dataset.h ___________________________________________________________

	using wax::BasicDataset_p;
	using wax::BasicDataset_wp;
	using wax::BasicDataset;
	using wax::Dataset_p;
	using wax::Dataset_wp;
	using wax::Dataset;

	//____ wax_dummyfont.h _________________________________________________________

	using wax::DummyFont_p;
	using wax::DummyFont_wp;
	using wax::DummyFont;

	//____ wax_dynamicbuffer.h _____________________________________________________

	using wax::DynamicBuffer;

	//____ wax_edgemap.h ___________________________________________________________

	using wax::Edgemap_p;
	using wax::Edgemap_wp;
	using wax::Edgemap;

	//____ wax_edgemapfactory.h ____________________________________________________

	using wax::EdgemapFactory_p;
	using wax::EdgemapFactory_wp;
	using wax::EdgemapFactory;

	//____ wax_edgemaptools.h ______________________________________________________

	namespace EdgemapTools = wax::EdgemapTools;

	//____ wax_enumextras.h ________________________________________________________

	using wax::ErrorLevel_min;
	using wax::ErrorCode_min;
	using wax::TintMode_min;
	using wax::TintShape_min;
	using wax::TintSpread_min;
	using wax::TintRadius_min;
	using wax::BlendMode_min;
	using wax::Alignment_min;
	using wax::Placement_min;
	using wax::Direction_min;
	using wax::Axis_min;
	using wax::SampleMethod_min;
	using wax::PixelFormat_min;
	using wax::PixelType_min;
	using wax::ColorSpace_min;
	using wax::GfxFlip_min;
	using wax::TransitionCurve_min;
	using wax::CanvasRef_min;
	using wax::SampleOrigo_min;
	using wax::Compression_min;
	using wax::LineCapShape_min;
	using wax::ErrorLevel_max;
	using wax::ErrorCode_max;
	using wax::TintMode_max;
	using wax::TintShape_max;
	using wax::TintSpread_max;
	using wax::TintRadius_max;
	using wax::BlendMode_max;
	using wax::Alignment_max;
	using wax::Placement_max;
	using wax::Direction_max;
	using wax::Axis_max;
	using wax::SampleMethod_max;
	using wax::PixelFormat_max;
	using wax::PixelType_max;
	using wax::ColorSpace_max;
	using wax::GfxFlip_max;
	using wax::TransitionCurve_max;
	using wax::CanvasRef_max;
	using wax::SampleOrigo_max;
	using wax::Compression_max;
	using wax::LineCapShape_max;
	using wax::ErrorLevel_size;
	using wax::ErrorCode_size;
	using wax::TintMode_size;
	using wax::TintShape_size;
	using wax::TintSpread_size;
	using wax::TintRadius_size;
	using wax::BlendMode_size;
	using wax::Alignment_size;
	using wax::Placement_size;
	using wax::Direction_size;
	using wax::Axis_size;
	using wax::SampleMethod_size;
	using wax::PixelFormat_size;
	using wax::PixelType_size;
	using wax::ColorSpace_size;
	using wax::GfxFlip_size;
	using wax::TransitionCurve_size;
	using wax::CanvasRef_size;
	using wax::SampleOrigo_size;
	using wax::Compression_size;
	using wax::LineCapShape_size;
	using wax::toString;

	//____ wax_font.h ______________________________________________________________

	using wax::Glyph;
	using wax::Font_p;
	using wax::Font_wp;
	using wax::Font;

	//____ wax_geo.h _______________________________________________________________

	using wax::Coord;
	using wax::CoordSPX;
	using wax::CoordS;
	using wax::CoordI;
	using wax::CoordF;
	using wax::Border;
	using wax::BorderSPX;
	using wax::BorderS;
	using wax::BorderI;
	using wax::BorderF;
	using wax::Corners;
	using wax::CornersSPX;
	using wax::Size;
	using wax::SizeSPX;
	using wax::SizeS;
	using wax::SizeI;
	using wax::SizeF;
	using wax::Rect;
	using wax::RectSPX;
	using wax::RectS;
	using wax::RectI;
	using wax::RectF;
	using wax::BinalCoord;
	using wax::BinalSize;
	using wax::BinalRect;
	using wax::CoordT;
	using wax::BorderT;
	using wax::CornersT;
	using wax::SizeT;
	using wax::RectT;

	//____ wax_backend.h ________________________________________________________

	using GfxBackend_p = wax::Backend_p;
	using GfxBackend_wp = wax::Backend_wp;
	using GfxBackend = wax::Backend;

	//____ wax_device.h _________________________________________________________

	using GfxDevice_p = wax::Device_p;
	using GfxDevice_wp = wax::Device_wp;
	using wax::WaveLine;
	using wax::RoundedBoxSpec;
	using wax::RingSlice;
	using wax::RingSpec;
	using wax::LineCap;
	using wax::LineCaps;
	using wax::CanvasInfo;
	using GfxDevice = wax::Device;

	//____ wax_device_gen2.h ____________________________________________________

	using GfxDeviceGen2_p = wax::DeviceGen2_p;
	using GfxDeviceGen2_wp = wax::DeviceGen2_wp;
	using GfxDeviceGen2 = wax::DeviceGen2;

	//____ wax_devicefactory.h __________________________________________________

	using GfxDeviceFactory_wp = wax::DeviceFactory_wp;
	using GfxDeviceFactory = wax::DeviceFactory;

	//____ wax_gpushapes.h _________________________________________________________

	namespace GpuShapes = wax::GpuShapes;

	//____ wax_gridwalker.h ________________________________________________________

	using wax::GridWalker;

	//____ wax_lzcompression.h _____________________________________________________

	using wax::LZCompressor_p;
	using wax::LZCompressor_wp;
	using wax::LZDecompressor_p;
	using wax::LZDecompressor_wp;
	using wax::LZCompressor;
	using wax::LZDecompressor;

	//____ wax_memheap.h ___________________________________________________________

	using wax::MemHeap;

	//____ wax_mempool.h ___________________________________________________________

	using wax::MemPool;

	//____ wax_memstack.h __________________________________________________________

	using wax::MemStack;

	//____ wax_object.h ____________________________________________________________

	using wax::Object_p;
	using wax::Object;

	//____ wax_patches.h ___________________________________________________________

	using wax::PatchesT;
	using wax::PatchesPTS;
	using wax::PatchesSPX;
	using wax::PatchesI;
	using wax::PatchesF;

	//____ wax_pixeltools.h ________________________________________________________

	namespace PixelTools = wax::PixelTools;

	//____ wax_printer.h ___________________________________________________________

	using wax::Printer_p;
	using wax::Printer_wp;
	using wax::Printer;

	//____ wax_q565compression.h ___________________________________________________

	using wax::Q565Compressor_p;
	using wax::Q565Compressor_wp;
	using wax::Q565Decompressor_p;
	using wax::Q565Decompressor_wp;
	using wax::Q565Compressor;
	using wax::Q565Decompressor;

	//____ wax_qoicompression.h ____________________________________________________

	using wax::QOICompressor_p;
	using wax::QOICompressor_wp;
	using wax::QOIDecompressor_p;
	using wax::QOIDecompressor_wp;
	using wax::QOICompressor;
	using wax::QOIDecompressor;

	//____ wax_rlecompression.h ____________________________________________________

	using wax::RLECompressor_p;
	using wax::RLECompressor_wp;
	using wax::RLEDecompressor_p;
	using wax::RLEDecompressor_wp;
	using wax::RLECompressor;
	using wax::RLEDecompressor;

	//____ wax_spans.h _____________________________________________________________

	using wax::Spans;

	//____ wax_spxcompression.h ____________________________________________________

	using wax::SPXCompressor_p;
	using wax::SPXCompressor_wp;
	using wax::SPXDecompressor_p;
	using wax::SPXDecompressor_wp;
	using wax::SPXCompressor;
	using wax::SPXDecompressor;

	//____ wax_strongptr.h _________________________________________________________

	using wax::StrongPtr;

	//____ wax_surface.h ___________________________________________________________

	using wax::Surface_p;
	using wax::Surface_wp;
	using wax::PixelBuffer;
	using wax::NinePatch;
	using wax::Surface;

	//____ wax_surfacefactory.h ____________________________________________________

	using wax::SurfaceFactory_wp;
	using wax::SurfaceFactory;

	//____ wax_surfacefileheader.h _________________________________________________

	using wax::SurfaceFileHeader;
	using wax::SurfaceFileLayout;
	using wax::surfaceFileLayout;

	//____ wax_surfacefileinspector.h ______________________________________________

	using wax::SurfaceFileInspector_p;
	using wax::SurfaceFileInspector_wp;
	using wax::SurfaceFileInspector;

	//____ wax_surfacereader.h _____________________________________________________

	using wax::SurfaceReader_p;
	using wax::SurfaceReader_wp;
	using wax::SurfaceReader;

	//____ wax_surfacewriter.h _____________________________________________________

	using wax::SurfaceWriter_p;
	using wax::SurfaceWriter_wp;
	using wax::SurfaceWriter;

	//____ wax_sysfont.h ___________________________________________________________

	using wax::SysFont_p;
	using wax::SysFont_wp;
	using wax::SysFont;

	//____ wax_tint.h ______________________________________________________________

	using wax::ColorStop;
	using wax::Tint_p;
	using wax::Tint_wp;
	using wax::Tint;

	//____ wax_tinttools.h _________________________________________________________

	namespace TintTools = wax::TintTools;

	//____ wax_transitions.h _______________________________________________________

	using wax::Transition_p;
	using wax::Transition_wp;
	using wax::Transition;
	using wax::ColorTransition_p;
	using wax::ColorTransition_wp;
	using wax::ColorTransition;
	using wax::CoordTransition_p;
	using wax::CoordTransition_wp;
	using wax::CoordTransition;
	using wax::ValueTransition_p;
	using wax::ValueTransition_wp;
	using wax::ValueTransition;

	//____ wax_types.h _____________________________________________________________

	using wax::limit;
	using wax::ErrorLevel;
	using wax::ErrorCode;
	using wax::Bitmask;
	using wax::TypeInfo;
	using wax::binalInt;
	using wax::BINAL_MUL;
	using wax::BINAL_SHIFT;
	using wax::BINAL_MASK;
	using wax::pts;
	using wax::spx;
	using wax::spx_max;
	using wax::TintMode;
	using wax::TintShape;
	using wax::TintSpread;
	using wax::TintRadius;
	using wax::BlendMode;
	using wax::Alignment;
	using wax::Placement;
	using wax::Direction;
	using wax::Axis;
	using wax::XSections;
	using wax::YSections;
	using wax::SampleMethod;
	using wax::PixelFormat;
	using wax::PixelType;
	using wax::ColorSpace;
	using wax::PixelDescription;
	using wax::GfxFlip;
	using wax::Transform;
	using wax::TransitionCurve;
	using wax::CanvasRef;
	using wax::SampleOrigo;
	using wax::Compression;
	using wax::LineCapShape;

	//____ wax_warpgenerator.h _____________________________________________________

	using wax::WarpGenerator_p;
	using wax::WarpGenerator_wp;
	using wax::WarpGenerator;

	//____ wax_warpmap.h ___________________________________________________________

	using wax::Warpmap_p;
	using wax::Warpmap_wp;
	using wax::Warpmap;

	//____ wax_warpmapfactory.h ____________________________________________________

	using wax::WarpmapFactory_p;
	using wax::WarpmapFactory_wp;
	using wax::WarpmapFactory;

	//____ wax_waveform.h __________________________________________________________

	using wax::Waveform_p;
	using wax::Waveform_wp;
	using wax::Waveform;

	//____ wax_weakptr.h ___________________________________________________________

	using wax::Finalizer_p;
	using wax::WeakPtrHub;
	using wax::WeakPtr;

	//____ Not brought in _______________________________________________________
	//
	// Core       WonderGUI has its own Base, which inherits wax::Core.
	// Context    WonderGUI has its own GUIContext, which holds a wax::Context.
	// Util       wg::Util extends it, see wg_util.h.
}

//____ Macros _________________________________________________________________

#define WGBP(T, ...)\
	([&]{ wg::T::Blueprint _{}; __VA_ARGS__; return _; }())

#define WGOVR(T, ...)\
([&]{ auto _ = T; __VA_ARGS__; return _; }())

#define WGCREATE(T, ...)\
	wg::T::create([&]{ wg::T::Blueprint _{}; __VA_ARGS__; return _; }())

#define WGTRACK(pObj) wg::Base::_trackObj(pObj, __FILE__, __LINE__ )

#endif //WG_WAX_DOT_H
