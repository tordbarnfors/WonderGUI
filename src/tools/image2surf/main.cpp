
#include <wondergfx.h>
#include <wg_softsurface.h>
#include <wg_softsurfacefactory.h>
#include <wg_q565compression.h>
#include <wg_lzcompression.h>
#include <wg_rlecompression.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <utility>
#include <fstream>

using namespace wg;
using namespace std;

bool	bQuit = false;



PixelFormat		g_format = PixelFormat::ARGB_8;
ColorSpace		g_colorSpace = ColorSpace::sRGB;
bool			g_bBigEndian = (WG_IS_BIG_ENDIAN == 1);

Compressor_p	g_pPixelCompressor;

char *			g_pInputFileName = nullptr;
char *			g_pOutputFileName = nullptr;

//____ parseComandLine() ______________________________________________________

int parseCommandLine( int argc, char** argv )
{
	for( int i = 1 ; i < argc ; i++ )
	{
		char * pArg = argv[i];
		
		if( strstr(pArg, "--format=" ) == pArg )
		{
			char * pValue = pArg + 9;
			
			g_format = PixelFormat::Undefined;
			for( int format = int(PixelFormat_min) ; format < int(PixelFormat_size) ; format++ )
			{
				if( strcmp( toString((PixelFormat)format), pValue ) == 0 )
				{
					g_format = (PixelFormat) format;
					break;
				}
			}
			
			if( g_format == PixelFormat::Undefined )
			{
				printf( "ERROR: '%s' is not a valid pixelformat!\n", pValue );
				return -1;
			}
		}
		else if( strstr(pArg, "--colorspace=" ) == pArg )
		{
			char * pValue = pArg + 13;

			if( strcmp(pValue, "sRGB") == 0 )
				g_colorSpace = ColorSpace::sRGB;
			else if( strcmp(pValue, "Linear") == 0 || strcmp(pValue, "linear") == 0 )
				g_colorSpace = ColorSpace::Linear;
			else
			{
				printf( "ERROR: '%s' is not a valid color space!\n", pValue );
				return -1;
			}
		}
		else if( strstr(pArg, "--byteorder=" ) == pArg )
		{
			char * pValue = pArg + 12;

			if( strcmp(pValue, "native") == 0 )
				g_bBigEndian = (WG_IS_BIG_ENDIAN == 1);
			else if( strcmp(pValue, "little") == 0 )
				g_bBigEndian = false;
			else if( strcmp(pValue, "big") == 0 )
				g_bBigEndian = true;
			else
			{
				printf( "ERROR: '%s' is not a valid byte order!\n", pValue );
				return -1;
			}
		}
		else if( strstr(pArg, "--pixelcomp=") == pArg )
		{
			char * pValue = pArg + 12;

			if( strcmp(pValue, "NONE" ) == 0)
			{
				// No compression, do nothing.
			}
			else if( strcmp(pValue, "Q565" ) == 0)
			{
				g_pPixelCompressor = Q565Compressor::create();
			}
			else if( strcmp(pValue, "LZWG" ) == 0)
			{
				g_pPixelCompressor = LZCompressor::create();
			}
			else if (strcmp(pValue, "RLE1") == 0)
			{
				g_pPixelCompressor = RLECompressor::create( WGBP(RLECompressor, _.primSize = 1 ));
			}
			else if (strcmp(pValue, "RLE2") == 0)
			{
				g_pPixelCompressor = RLECompressor::create(WGBP(RLECompressor, _.primSize = 2 ));
			}
			else
			{
				printf( "ERROR: '%s' is not a recognized compressor format.\n", pValue );
				return -1;
			}
		}
		else if( strstr(pArg, "--" ) == pArg )
		{
			printf( "ERROR: '%s' is not a recognized parameter!\n", pArg );
			return -1;
		}
		else if( g_pInputFileName == nullptr )
			g_pInputFileName = pArg;
		else if( g_pOutputFileName == nullptr )
			g_pOutputFileName = pArg;
		else
		{
			printf( "ERROR: Don't know what to do with extra command line argument '%s'!\n", pArg );
			return -1;
		}
	}
	
	if( g_pInputFileName == nullptr || g_pOutputFileName == nullptr )
	{
		printf( "ERROR: Missing input and/or output filename!\n" );
		return -1;
	}

	return 0;
}

//____ printUsage() ___________________________________________________________

void printUsage(char** argv)
{
	printf( "%s [param] inputImage outputSurface\n\n", argv[0]);

	printf( "Parameters:\n\n" );
	printf( "--format=[format]          - Set format of output surface. Default is ARGB_8.\n" );
	printf( "--colorspace=[colorspace]  - sRGB (default) or Linear. Colors are converted to it.\n" );
	printf( "--byteorder=[byteorder]    - native (default), little or big.\n" );
	printf( "--pixelcomp=[compression]  - Set compression for pixel data.\n" );

	
	printf( "\nOutput formats:\n" );
	for( int format = int(PixelFormat_min) ; format < int(PixelFormat_size) ; format++ )
	{
		if( PixelFormat(format) != PixelFormat::Undefined )
			printf( "    %s\n", toString((PixelFormat)format) );
	}

	printf( "\nCompression formats:\n" );
	printf( "    Q565 - QOI inspired compression for 16-bit pixels.\n");
	printf( "    LZWG - Lempel-Ziw derivative for compression of any data.\n");
	printf("     RLE1 - Simple RLE-compression, with one byte prim size.\n");
	printf("     RLE2 - Simple RLE-compression, with two byte prim size.\n");
}

//____ main() _________________________________________________________________

int main ( int argc, char** argv )
{ 
	if( argc < 2 )
	{
		printUsage(argv);
		return -1;
	}

	if( parseCommandLine(argc,argv) != 0 )
		return -1;

	// Load image

	int width, height, channels;
	unsigned char *pImage = stbi_load(g_pInputFileName, &width, &height, &channels, 4);
	
	if( pImage == NULL )
	{
		printf( "Error loading/decoding '%s'\n", g_pInputFileName );
		return -1;
	}

	// Init wg

	GfxBase::init();

	// Convert surface

	// stb_image gives us sRGB bytes in R, G, B, A order, which is ARGB_8 with red and blue
	// swapped, read as little endian.

	PixelDescription desc = Util::pixelFormatToDescription(PixelFormat::ARGB_8, false);
	
	swap( desc.B_mask, desc.R_mask);

	auto pOrg = SoftSurface::create( { .format = PixelFormat::ARGB_8, .size = {width,height}}, pImage, desc, 0 );
	
	auto pConverted = pOrg->convert({ .bigEndian = g_bBigEndian, .colorSpace = g_colorSpace, .format = g_format }, SoftSurfaceFactory::create());

	if( pConverted == nullptr )
	{
		printf( "Failed to convert image. Likely reasons:\n - Destination format not supported\n - Too many colors to fit in destination palette (if destination is indexed)\n");
		goto exit;
	}

	// Write surface

	{
		std::ofstream out(g_pOutputFileName, std::ios::binary);
		auto pWriter = SurfaceWriter::create({ .pixelCompressor = g_pPixelCompressor });
		pWriter->writeSurfaceToStream(out, pConverted);
		out.close();
	}

exit:
	GfxBase::exit();
	stbi_image_free(pImage);
    return 0;
}

