
#include <wax.h>
#include <wax_softsurface.h>
#include <wax_softsurfacefactory.h>
#include <wax_q565compression.h>
#include <wax_lzcompression.h>
#include <wax_rlecompression.h>
#include <wax_qoicompression.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <utility>
#include <fstream>

using namespace wax;
using namespace std;

bool	bQuit = false;



PixelFormat		g_format = PixelFormat::ARGB_8;
ColorSpace		g_colorSpace = ColorSpace::sRGB;
bool			g_bBigEndian = (WAX_IS_BIG_ENDIAN == 1);

Compressor_p	g_pPixelCompressor;
bool			g_bQOICompression = false;

char *			g_pInputFileName = nullptr;
char *			g_pOutputFileName = nullptr;

//____ hasExtension() _________________________________________________________

bool hasExtension( const char * pFileName, const char * pExtension )
{
	size_t nameLen = strlen(pFileName);
	size_t extLen = strlen(pExtension);

	if( nameLen < extLen )
		return false;

	for( size_t i = 0 ; i < extLen ; i++ )
	{
		if( tolower(pFileName[nameLen - extLen + i]) != tolower(pExtension[i]) )
			return false;
	}
	return true;
}

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
				g_bBigEndian = (WAX_IS_BIG_ENDIAN == 1);
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
				g_pPixelCompressor = RLECompressor::create( WAXBP(RLECompressor, _.primSize = 1 ));
			}
			else if (strcmp(pValue, "RLE2") == 0)
			{
				g_pPixelCompressor = RLECompressor::create(WAXBP(RLECompressor, _.primSize = 2 ));
			}
			else if (strcmp(pValue, "QOI") == 0)
			{
				g_bQOICompression = true;		// Compressor is created once we know format and byte order.
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

	if( g_bQOICompression )
	{
		if( g_format != PixelFormat::ARGB_8 && g_format != PixelFormat::XRGB_8 )
		{
			printf( "ERROR: QOI compression is only supported for formats ARGB_8 and XRGB_8!\n" );
			return -1;
		}

		g_pPixelCompressor = QOICompressor::create( WAXBP(QOICompressor, _.format = g_format, _.bigEndian = g_bBigEndian ) );
	}

	return 0;
}

//____ printUsage() ___________________________________________________________

void printUsage(char** argv)
{
	printf( "%s [param] inputImage outputSurface\n\n", argv[0]);

	printf( "Input image can be in any format supported by stb_image or QOI.\n" );
	printf( "Output surface is saved as QOI image if filename ends with '.qoi'.\n\n" );

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
	printf( "    QOI  - QOI compression for ARGB_8 and XRGB_8, same as in QOI images.\n");
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

	// Init wg

	wax::Core::init();

	// Load image

	unsigned char *pImage = nullptr;
	Surface_p pOrg;

	if( hasExtension(g_pInputFileName, ".qoi") )
	{
		std::ifstream in(g_pInputFileName, std::ios::binary);
		pOrg = SurfaceReader::create({ .factory = SoftSurfaceFactory::create() })->readSurfaceFromStream(in);
	}
	else
	{
		int width, height, channels;
		pImage = stbi_load(g_pInputFileName, &width, &height, &channels, 4);

		// stb_image gives us sRGB bytes in R, G, B, A order, which is ARGB_8 with red and blue
		// swapped, read as little endian.

		if( pImage )
		{
			PixelDescription desc = Util::pixelFormatToDescription(PixelFormat::ARGB_8, false);

			swap( desc.B_mask, desc.R_mask);

			pOrg = SoftSurface::create( { .format = PixelFormat::ARGB_8, .size = {width,height}}, pImage, desc, 0 );
		}
	}

	if( pOrg == nullptr )
	{
		printf( "Error loading/decoding '%s'\n", g_pInputFileName );
		wax::Core::exit();
		return -1;
	}

	// Convert surface

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

		if( hasExtension(g_pOutputFileName, ".qoi") )
			pWriter->writeSurfaceToQOIStream(out, pConverted);
		else
			pWriter->writeSurfaceToStream(out, pConverted);
		out.close();
	}

exit:
	pOrg = nullptr;
	pConverted = nullptr;
	g_pPixelCompressor = nullptr;
	wax::Core::exit();
	if( pImage )
		stbi_image_free(pImage);
    return 0;
}

