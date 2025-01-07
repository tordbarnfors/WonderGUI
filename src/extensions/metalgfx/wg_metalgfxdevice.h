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

#ifndef WG_METALGFXDEVICE_DOT_H
#define WG_METALGFXDEVICE_DOT_H
#pragma once

#import "simd/simd.h"
#import "MetalKit/MetalKit.h"


#include <wg_gfxtypes.h>
#include <wg_gfxdevice_gen1.h>
#include <wg_metalsurface.h>

namespace wg
{
	class Surface;

	class MetalGfxDevice;
	typedef	StrongPtr<MetalGfxDevice>	MetalGfxDevice_p;
	typedef	WeakPtr<MetalGfxDevice>	MetalGfxDevice_wp;

	class MetalGfxDevice : public GfxDeviceGen1
	{
        friend class MetalSurface;
	public:
		//.____ Creation ________________________________________________

        static void setMetalDevice( id<MTLDevice> device );
        static id<MTLDevice> metalDevice() { return s_metalDevice; }
        
        static MetalGfxDevice_p	create();

		//.____ Identification __________________________________________

		const TypeInfo&		typeInfo(void) const override;
		const static TypeInfo	TYPEINFO;

		const TypeInfo&			surfaceType( void ) const override;

		//.____ Misc _______________________________________________________

		SurfaceFactory_p		surfaceFactory() override;
        EdgemapFactory_p       edgemapFactory() override;

        bool    setDefaultCanvas( MTLRenderPassDescriptor* renderPassDesc, SizeI pixelSize, PixelFormat pixelFormat, int scale = 64 );
 
        using GfxDevice::canvas;
        const CanvasInfo canvas(CanvasRef ref) const override;

        //.____ State _________________________________________________

        void    setTintColor(HiColor color) override;
        void    setTintGradient(const RectSPX& rect, const Gradient& gradient) override;
        void    clearTintGradient() override;

        bool    setBlendMode(BlendMode blendMode) override;
        bool    setBlitSource(Surface * pSource) override;
        void    setMorphFactor(float factor) override;
		void	setFixedBlendColor( HiColor color ) override;
		void	setBlurbrush(Blurbrush* pBrush) override;
		
        bool    isCanvasReady() const;
            
        //.____ Rendering ________________________________________________
        
        bool    beginRender() override;
        bool    endRender() override;
        bool    isIdle() override;
        void    flush() override;
        void    flushAndWait();

        void    autopresent( id<MTLDrawable> drawable );           // endRender() will clear this, so needs to be set for each begin/end render cycle.

        using   GfxDevice::fill;
		void	fill(const RectSPX& rect, HiColor col) override;

		void	drawLine(CoordSPX begin, CoordSPX end, HiColor color, spx thickness) override;
        void    drawLine(CoordSPX begin, Direction dir, spx length, HiColor color, spx thickness) override;

		void	drawEdgemap(CoordSPX dest, Edgemap * pEdgemap ) override;
		void	flipDrawEdgemap(CoordSPX dest, Edgemap * pEdgemap, GfxFlip flip) override;


	protected:
		MetalGfxDevice();
		~MetalGfxDevice();

        void    _canvasWasChanged() override;
        void    _renderLayerWasChanged() override;    // Checked for errors before we get here.
        void    _clipListWasChanged() override;            // Called when cliplist has been changed.
        
		void	_transformBlitSimple(const RectSPX& dest, CoordSPX src, const int simpleTransform[2][2], OpType type) override;
		void	_transformBlitComplex(const RectSPX& dest, BinalCoord src, const binalInt complexTransform[2][2], OpType type) override;

		void	_transformDrawSegments(const RectSPX& dest, int nSegments, const HiColor * pSegmentColors, int nEdges, const int * pEdges, int edgeStripPitch, TintMode tintMode, const int simpleTransform[2][2]) override;

        enum class VertexInputIndex
        {
            VertexBuffer = 0,
            ExtrasBuffer = 1,
            Uniform = 2
        };

        enum class FragmentInputIndex
        {
            ExtrasBuffer = 0,
			BlurUniform = 1
        };

        
        enum class TextureIndex
        {
            Texture = 0,
            Palette = 1,
            SegPal = 2
        };
        
        enum class BlitFragShader
        {
            Normal = 0,
            PaletteNearest = 1,
            PaletteInterpolated = 2,
            A8Source = 3,
            RGBXSource = 4
        };
        
        enum class DestFormat
        {
            BGRX8_linear,
            BGRA8_linear,
            BGRX8_sRGB,
            BGRA8_sRGB,
            Alpha_8
        };
        
        
        enum Command
        {
            None,
            SetCanvas,
//            SetClip,
            SetBlendMode,
            SetMorphFactor,
            SetTintColor,
            SetTintGradient,
            ClearTintGradient,
			SetFixedBlendColor,
			SetBlurMatrices,
            SetBlitSource,
            Fill,
            FillSubPixel,                // Includes start/direction lines.
            Plot,
            LineFromTo,
            Blit,
			Blur,
            Segments,

        };

        id<MTLRenderCommandEncoder> _setCanvas( MetalSurface * pCanvas, int width, int height, CanvasInit initOperation, Color clearColor );
        void            _setBlendMode( id<MTLRenderCommandEncoder>, BlendMode mode);
        void            _setMorphFactor( id<MTLRenderCommandEncoder>, float morphFactor);
		void            _setFixedBlendColor( id<MTLRenderCommandEncoder>, HiColor color);
		void 			_setBlurMatrices( id<MTLRenderCommandEncoder> renderEncoder, spx radius, const float red[9], const float green[9], const float blue[9] );
        void            _setBlitSource( id<MTLRenderCommandEncoder>, MetalSurface * pSurf);
        void            _setTintColor( id<MTLRenderCommandEncoder>, HiColor color);
        void            _setTintGradient( id<MTLRenderCommandEncoder>, const RectI& rect, const Gradient& gradient);
        void            _clearTintGradient( id<MTLRenderCommandEncoder> renderEncoder );

        inline void    _beginDrawCommand(Command cmd);
        inline void    _beginDrawCommandWithSource(Command cmd);
        inline void    _beginDrawCommandWithInt(Command cmd, int data);
        inline void    _beginClippedDrawCommand(Command cmd );
        inline void    _beginStateCommand(Command cmd, int dataSize);
		inline void    _beginStateCommandWithAlignedData(Command cmd, int dataSize);
        inline void    _endCommand();


        typedef void(MetalGfxDevice::*CmdFinalizer_p)();

        void    _dummyFinalizer();
        void    _drawCmdFinalizer();
        
        void    _initTables();
        float    _scaleThickness(float thickeness, float slope);
        id<MTLRenderPipelineState> _compileRenderPipeline( NSString* label, NSString* vertexShader,
                                    NSString* fragmentShader, BlendMode blendMode, PixelFormat destFormat );
        
		void    _resizeBuffers(int commandNeeded, int vertexNeeded, int clipListNeeded, int extrasNeeded, int segEdgeNeeded);
        void    _executeBuffer();
        void    _resetBuffers();

        SurfaceFactory_p    m_pSurfaceFactory = nullptr;
        EdgemapFactory_p   m_pEdgemapFactory = nullptr;

        float    m_lineThicknessTable[17];

        float    m_sRGBtoLinearTable[256];
        float    m_linearToLinearTable[256];
        
        
        //
        
        Command         m_cmd;
        CmdFinalizer_p  m_pCmdFinalizer;
        int             m_cmdBeginVertexOfs;                        // Saved for CmdFinalizer

        int             m_nSegments;                                // Number of segments for current segment command.

        int             m_canvasYstart;

        bool            m_bFullyInitialized = false;
        CanvasInit      m_beginRenderOp = CanvasInit::Keep;
        
        struct Uniform
        {
            float      canvasDimX;
            float      canvasDimY;
            int        canvasYOfs;
            int        canvasYMul;

            simd_float4 flatTint;

            RectI    tintRect;

            simd_float4   topLeftTint;
            simd_float4   topRightTint;
            simd_float4   bottomRightTint;
            simd_float4   bottomLeftTint;

            SizeI    textureSize;
        };

		struct BlurUniform
		{
			simd_float4   	colorMtx[9];
			simd_float2		offset[9];
		};
		
        struct Vertex
        {
            CoordI          coord;
            int             extrasOfs;                        // Offset into extras buffer.
            simd_float2   uv;
        };
           
        // Buffers

        Uniform         m_uniform;
		BlurUniform		m_blurUniform;

        int             m_commandBufferSize = 4096;
        int *           m_pCommandBuffer = nullptr;         // Queue of commands to execute when flushing buffer
        int             m_commandOfs = 0;                   // Write offset in m_commandBuffer
        
        id<MTLBuffer>   m_vertexBufferId = nil;
        int             m_vertexBufferSize = 4096;
        Vertex *        m_pVertexBuffer = nullptr;          // Pointer to content of vertex buffer
        int             m_vertexFlushPoint = 0;
        int             m_vertexOfs = 0;                    // Write offset in m_pVertexData

        id<MTLBuffer>   m_extrasBufferId = nil;
        int             m_extrasBufferSize = 4096;
        float *         m_pExtrasBuffer = nullptr;          // Pointer to content of extras buffer
        int             m_extrasOfs = 0;                    // Write offset in m_pExtrasData

        id<MTLBuffer>   m_clipListBufferId = nil;
        int             m_clipListBufferSize = 256;
        RectI *         m_pClipListBuffer = nullptr;
        int             m_clipWriteOfs = 0;                 // Write offset in m_clipListBuffer
        int             m_clipCurrOfs = -1;                 // Offset to where current clipList is written to in clipListBuffer, or -1 if not written.

        id<MTLBuffer>   m_segEdgeBufferId = nil;
        int             m_segEdgeBufferSize = 16384;
        float *         m_pSegEdgeBuffer = nullptr;
        int             m_segEdgeFlushPoint = 0;
        int             m_segEdgeOfs = 0;
        
        const int       c_segPalEntrySize = 2*4*4*c_maxSegments;  // Bytes per palette (4 pixels of 4 uint16_t per segment).

        id<MTLTexture>  m_segPalTextureId = nil;
        id<MTLBuffer>   m_segPalBufferId = nil;
        int             m_segPalBufferSize = 64;            // Number of complete palettes, not number of uint16_t.
        uint16_t *      m_pSegPalBuffer = nullptr;
        int             m_segPalOfs = 0;
                
        
        // Active state data

        MetalSurface_p  m_pActiveBlitSource = nullptr;                  // Currently active blit source during buffer execution, not to confuse with m_pBlitSource which might not be active yet.
        MetalSurface_p  m_pActiveCanvas     = nullptr;                  // Currently active canvas during buffer execution, not to confuse with m_pCanvas which might not be active yet.
        SizeI           m_activeCanvasSize;
        DestFormat      m_activeCanvasFormat;
        bool            m_bGradientActive   = false;
        BlendMode       m_activeBlendMode   = BlendMode::Blend;
        float           m_activeMorphFactor = 0.5f;
		HiColor         m_activeFixedBlendColor = HiColor::White;
		spx				m_activeBlurRadius = 64;

		float           m_morphFactorInUse = -1;
		HiColor         m_fixedBlendColorInUse = HiColor::Undefined;

		
		
        
        id<MTLLibrary>              m_library = nil;
        id<MTLDrawable>             m_drawableToAutoPresent = nil;
        MTLRenderPassDescriptor*    m_defaultCanvasRenderPassDesc = nil;
        PixelFormat                 m_defaultCanvasPixelFormat;
		CanvasInfo                  m_defaultCanvas;

        id<MTLCommandBuffer>        m_metalCommandBuffer = nil;

        bool                        m_bRendering = false;               // Set to true while between beginRender() and endRender() calls.
        
        std::atomic<int>            m_flushesInProgress;                // Number of buffer flushes to complete before metal is idle.
                
        id<MTLRenderPipelineState>  m_plotPipelines[BlendMode_size][5];          // [BlendMode][DestFormat]
        id<MTLRenderPipelineState>  m_lineFromToPipelines[BlendMode_size][5];    // [BlendMode][DestFormat]
        id<MTLRenderPipelineState>  m_fillPipelines[2][BlendMode_size][5];       // [bGradient][BlendMode][DestFormat]
        id<MTLRenderPipelineState>  m_fillAAPipelines[2][BlendMode_size][5];     // [bGradient][BlendMode][DestFormat]
        
        id<MTLRenderPipelineState>  m_blitPipelines[5][2][BlendMode_size][5];   // [BlitFragShader][bGradient][BlendMode][DestFormat]

		id<MTLRenderPipelineState>  m_blurPipelines[2][BlendMode_size][5];   	// [bGradient][BlendMode][DestFormat]

        id<MTLRenderPipelineState>  m_segmentsPipelines[c_maxSegments][2][BlendMode_size][5];   // [nbEdges][bGradient][BlendMode][DestFormat]
        
        id<MTLSamplerState>         m_samplers[2][2][2];                        // [bMipMap][bInterpolate][bTile]
        
        static const char shaders[];

        static id<MTLDevice>        s_metalDevice;
        static id<MTLCommandQueue>  s_metalCommandQueue;



        MetalGfxDevice *            m_pPrevActiveDevice; // Storage for previous active device when we become active.
        static MetalGfxDevice *    s_pActiveDevice;    // Pointer at Metal device currently in rendering state.
    };

    //____ _beginDrawCommand() ________________________________________________

    inline void MetalGfxDevice::_beginDrawCommand(Command cmd )
    {
        if (m_commandOfs > m_commandBufferSize - 2 )
            _resizeBuffers(2,0,0,0,0);

        m_cmd = cmd;
        m_pCmdFinalizer = &MetalGfxDevice::_drawCmdFinalizer;
        m_cmdBeginVertexOfs = m_vertexOfs;
        m_pCommandBuffer[m_commandOfs++] = cmd;
        
        if (m_canvas.pSurface)
            static_cast<MetalSurface*>(m_canvas.pSurface.rawPtr())->m_bBufferNeedsSync = true;
    }

    //____ _beginDrawCommandWithSource() ________________________________________________

    inline void MetalGfxDevice::_beginDrawCommandWithSource(Command cmd)
    {
        if (m_commandOfs > m_commandBufferSize - 2)
			_resizeBuffers(2,0,0,0,0);

        m_cmd = cmd;
        m_pCmdFinalizer = &MetalGfxDevice::_drawCmdFinalizer;
        m_cmdBeginVertexOfs = m_vertexOfs;
        m_pCommandBuffer[m_commandOfs++] = cmd;

        if( m_pBlitSource )
            static_cast<MetalSurface*>(m_pBlitSource.rawPtr())->m_bPendingReads = true;

        if( m_canvas.pSurface)
            static_cast<MetalSurface*>(m_canvas.pSurface.rawPtr())->m_bBufferNeedsSync = true;
    }

    //____ _beginDrawCommandWithInt() ________________________________________________

    inline void MetalGfxDevice::_beginDrawCommandWithInt(Command cmd, int data)
    {
        if (m_commandOfs > m_commandBufferSize - 3)
			_resizeBuffers(3,0,0,0,0);

        m_cmd = cmd;
        m_pCmdFinalizer = &MetalGfxDevice::_drawCmdFinalizer;
        m_cmdBeginVertexOfs = m_vertexOfs;
        m_pCommandBuffer[m_commandOfs++] = cmd;
        m_pCommandBuffer[m_commandOfs++] = data;

        if (m_canvas.pSurface)
            static_cast<MetalSurface*>(m_canvas.pSurface.rawPtr())->m_bBufferNeedsSync = true;
    }

    //____ _beginClippedDrawCommand() ________________________________________________

    inline void MetalGfxDevice::_beginClippedDrawCommand(Command cmd)
    {
        if (m_commandOfs > m_commandBufferSize - 4 || (m_clipCurrOfs == -1 && m_clipWriteOfs > m_clipListBufferSize - m_nClipRects) )
            _resizeBuffers(4,0,m_nClipRects,0,0);

        if (m_clipCurrOfs == -1)
        {
            m_clipCurrOfs = m_clipWriteOfs;

            for (int i = 0; i < m_nClipRects; i++)
            {
                RectI clip = m_pClipRects[i];
//                clip.y = m_canvasSize.h - (clip.y + clip.h);

                m_pClipListBuffer[m_clipWriteOfs++] = clip;
            }
        }

        m_cmd = cmd;
        m_pCmdFinalizer = &MetalGfxDevice::_drawCmdFinalizer;
        m_cmdBeginVertexOfs = m_vertexOfs;
        m_pCommandBuffer[m_commandOfs++] = cmd;
        m_pCommandBuffer[m_commandOfs++] = m_clipCurrOfs;
        m_pCommandBuffer[m_commandOfs++] = m_nClipRects;

        if (m_canvas.pSurface)
            static_cast<MetalSurface*>(m_canvas.pSurface.rawPtr())->m_bBufferNeedsSync = true;
    }


    //____ _beginStateCommand() ________________________________________________

    inline void MetalGfxDevice::_beginStateCommand(Command cmd, int dataSize)
    {
        if (m_commandOfs > m_commandBufferSize - dataSize - 1 )
            _resizeBuffers(dataSize+1,0,0,0,0);

        m_cmd = cmd;
        m_pCmdFinalizer = &MetalGfxDevice::_dummyFinalizer;

        m_pCommandBuffer[m_commandOfs++] = cmd;
    }

//____ _beginStateCommandWithAlignedData() _______________________________________

inline void MetalGfxDevice::_beginStateCommandWithAlignedData(Command cmd, int dataSize)
{
	if (m_commandOfs > m_commandBufferSize - dataSize - 1 - 1 )
		_resizeBuffers(dataSize+2,0,0,0,0);

	m_cmd = cmd;
	m_pCmdFinalizer = &MetalGfxDevice::_dummyFinalizer;

	m_pCommandBuffer[m_commandOfs] = Command::None;
	m_commandOfs = (m_commandOfs & 0xFFFFFFFE) + 1;

	m_pCommandBuffer[m_commandOfs++] = cmd;
}


    //____ _endCommand() ______________________________________________________

    inline void MetalGfxDevice::_endCommand()
    {
        (this->*m_pCmdFinalizer)();
        m_pCmdFinalizer = &MetalGfxDevice::_dummyFinalizer;
    }



} // namespace wg
#endif //WG_METALGFXDEVICE_DOT_H

