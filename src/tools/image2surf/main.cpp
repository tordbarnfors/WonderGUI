
#include <wondergfx.h>
#include <wg_softsurface.h>
#include <wg_softsurfacefactory.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <utility>
#include <fstream>

using namespace wg;
using namespace std;

bool	bQuit = false;



PixelFormat		g_format = PixelFormat::BGRA_8;

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
	printf( "--format=[format]   - Set format of output surface.\n" );
	
	
	printf( "\nOutput formats:\n" );
	for( int format = int(PixelFormat_min) ; format < int(PixelFormat_size) ; format++ )
	{
		if( PixelFormat(format) != PixelFormat::Undefined )
			printf( "%s\n", toString((PixelFormat)format) );
	}
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

	PixelDescription desc = Util::pixelFormatToDescription(PixelFormat::BGRA_8_sRGB);
	
	swap( desc.B_mask, desc.R_mask);

	auto pOrg = SoftSurface::create( { .format = PixelFormat::BGRA_8, .size = {width,height}}, pImage, desc, 0 );
	
	auto pConverted = pOrg->convert({ .format = g_format }, SoftSurfaceFactory::create());

	if( pConverted == nullptr )
	{
		printf( "Failed to convert image. Likely reasons:\n - Destination format not supported\n - Too many colors to fit in destination palette (if destination is indexed)\n");
		goto exit;
	}

	// Write surface

	{
		std::ofstream out(g_pOutputFileName, std::ios::binary);
		auto pWriter = SurfaceWriter::create({});
		pWriter->writeSurfaceToStream(out, pConverted);
		out.close();
	}

exit:
	GfxBase::exit();
	stbi_image_free(pImage);
    return 0;
}

