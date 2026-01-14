
#include "compressbench.h"

#include <wondergui.h>
#include <wg_freetypefont.h>
#include <string>



using namespace wg;
using namespace wapp;
using namespace std;

int compress(uint8_t* pDest, uint16_t* pBegin, uint16_t* pEnd);
int roundtrip(Blob_p pPixels);
int decompress(uint16_t* pDest, uint8_t* pBegin, uint8_t* pEnd);



//____ create() _______________________________________________________________

WonderApp_p WonderApp::create()
{
	return new MyApp();
}

//____ init() _________________________________________________________________

bool MyApp::init(API * pAPI)
{
	m_pAPI = pAPI;

	std::string selected = pAPI->openFileDialog("File to compress", "c://Workspace//WonderGUI//resources", "", { "" }, "");

	auto pSource = pAPI->loadBlob(selected);

	auto pInspector = SurfaceFileInspector::create(pSource->data());

	auto pPixels = (uint16_t*) (((char*)pSource->data()) + pInspector->pixelDataOffset());
	auto srcSize = pInspector->pixelDataBytes();

	auto pBlob = Blob::create(pPixels, srcSize, nullptr);
	roundtrip(pBlob);



//	assert(pSource->size() % 2 == 0);

	auto pDest = (uint8_t *) Base::memStackAlloc(srcSize * 2);



	int size = compress(pDest, pPixels, pPixels + srcSize / 2);

	snprintf(m_message, 1024, "Uncompressed size: %d\nCompressed size: %d\nRatio: %d%%", srcSize, size, size * 100 / srcSize);


	Base::memStackFree(srcSize * 2);


	if (!_setupGUI(pAPI))
	{
		printf("ERROR: Failed to setup GUI!\n");
		return false;
	}

	return true;
}

//____ update() _______________________________________________________________

bool MyApp::update()
{
	return m_bContinue;
}

//____ exit() _________________________________________________________________

void MyApp::exit()
{

}

//____ closeWindow() __________________________________________________________

void MyApp::closeWindow(Window* pWindow)
{
	m_pWindow = nullptr;
	m_bContinue = false;
}

//____ _setupGUI() ____________________________________________________________

bool MyApp::_setupGUI(API* pAPI)
{
	m_pWindow = Window::create(pAPI, { .size = {800,600}, .title = "Hello World" });

	//

	auto pFontBlob = pAPI->loadBlob("resources/DroidSans.ttf");

	if (!pFontBlob)
		return false;

	auto pFont = FreeTypeFont::create(pFontBlob);

	m_pTextStyle = TextStyle::create({
		.color = Color8::Black,
		.font = pFont, 
		.size = 14,
	});

	m_pTextLayoutCentered = BasicTextLayout::create({ .placement = Placement::Center });

	//

	auto pBgSkin = ColorSkin::create( Color::PapayaWhip );

	auto pTextDisplay = TextDisplay::create({

		.display = {
			.layout = m_pTextLayoutCentered, 
			.style = m_pTextStyle, 
			.text = m_message 
		},

		.skin = pBgSkin
	});

	m_pWindow->mainCapsule()->slot = pTextDisplay;
	return true;
}


/*
	00 0xxxxx		New pixels(1 - 32)
	00 1xxxxx		Repeat previous pixel(1 - 32)
	01 xxxxxx		Pixel from index
	1 rrgggbb		Modif rgb values
*/

int roundtrip( Blob_p pPixels )
{
	assert(pPixels->size() % 2 == 0);

	Blob_p pCompressed = Blob::create(pPixels->size()*2 );

	Blob_p pDecompressed = Blob::create(pPixels->size()+4);

	int compressedSize = compress((uint8_t*)pCompressed->data(), (uint16_t*)pPixels->data(), ((uint16_t*)pPixels->data()) + pPixels->size() / 2);

	int decompressedSize = decompress((uint16_t*)pDecompressed->data(), (uint8_t*)pCompressed->data(), ((uint8_t*)pCompressed->data()) + compressedSize);

	uint16_t* pOriginal = (uint16_t*)pPixels->data();
	uint16_t* pReconstructed = (uint16_t*)pDecompressed->data();
	for (int i = 0; i < pPixels->size() / 2; i++)
	{
		if (pOriginal[i] != pReconstructed[i])
		{
			printf("ERROR: Mismatch at pixel %d: original=%04X, reconstructed=%04X\n", i, pOriginal[i], pReconstructed[i]);
			return -1;
		}
	}

	return compressedSize;
}

int compress(uint8_t* pDest, uint16_t* pBegin, uint16_t* pEnd)
{
	uint16_t palette[64];

	uint8_t	pixelToIndex[65536];


	for (int i = 0; i < 64; i++)
		palette[i] = 0;

	for (int i = 0; i < 65536; i++)
	{
		int r = i & 0x001F;
		int g = (i >> 5) & 0x003F;
		int b = (i >> 11) & 0x001F;

		pixelToIndex[i] = (r * 3 + g * 5 + b * 7) % 64;
	}


	uint16_t prevValue = 0;

	uint16_t* pRead = pBegin;
	uint8_t* pWrite = pDest;
	while (pRead < pEnd)
	{		
		uint16_t value = *pRead++;

		if (value == prevValue)
		{
			uint16_t count = 1;
			while (pRead < pEnd && *pRead == value && count < 32)
			{
				count++;
				pRead++;
			}

			*pWrite++ = 0x20 | (count - 1);						// Store as repeat of previous value
		}
		else
		{
			uint8_t index = pixelToIndex[value];

			if (palette[index] == value)
				*pWrite++ = 0x40 | index;						// Store as index lookup
			else
			{
				uint16_t prevR = prevValue & 0x001F;
				uint16_t prevG = (prevValue >> 5) & 0x003F;
				uint16_t prevB = (prevValue >> 11) & 0x001F;

				uint16_t r = value & 0x001F;
				uint16_t g = (value >> 5) & 0x003F;
				uint16_t b = (value >> 11) & 0x001F;

				uint16_t diffR = r - prevR + 2;
				uint16_t diffG = g - prevG + 4;
				uint16_t diffB = b - prevB + 2;


				if (diffR < 4 && diffG < 8 && diffB < 4)
				{
					*pWrite++ = 0x80 | (diffR << 5) | (diffG << 2) | diffB;		// Store as RGB-delta
					palette[index] = value;
				}
				else
				{
					auto pCounter = pWrite;

					*pWrite++ = 1;
					*pWrite++ = (uint8_t)value;
					*pWrite++ = (uint8_t)(value >> 8);

					palette[index] = value;

					int count = 1;
					while (pRead < pEnd && count < 32)
					{
						uint16_t futureValue = *pRead;

						if (futureValue == value)
							break;				// Next pixel is start of repetive section

						uint8_t futureIndex = pixelToIndex[futureValue];
						if (palette[futureIndex] == futureValue)
							break;				// Next pixel can be taken from index;

						uint16_t futureR = futureValue & 0x001F;
						uint16_t futureG = (futureValue >> 5) & 0x003F;
						uint16_t futureB = (futureValue >> 11) & 0x001F;

						uint16_t diffR = futureR - r + 2;
						uint16_t diffG = futureG - g + 4;
						uint16_t diffB = futureB - b + 2;

						if (diffR < 4 && diffG < 8 && diffB < 4)
							break;				// Next pixel can be stored as RGB-delta.

						palette[futureIndex] = futureValue;

						value = futureValue;

						r = futureR;
						g = futureG;
						b = futureB;

						*pWrite++ = (uint8_t)value;
						*pWrite++ = (uint8_t)(value >> 8);

						pRead++;
						count++;
					}

					*pCounter = count-1;
				}

			}
		}

		prevValue = value;
	}
	return int(pWrite - pDest);
}


int decompress(uint16_t* pDest, uint8_t* pBegin, uint8_t* pEnd)
{
	uint16_t* pOriginalDest = pDest;

	uint16_t palette[64];

	uint8_t	pixelToIndex[65536];

	for (int i = 0; i < 64; i++)
		palette[i] = 0;

	for (int i = 0; i < 65536; i++)
	{
		int r = i & 0x001F;
		int g = (i >> 5) & 0x003F;
		int b = (i >> 11) & 0x001F;

		pixelToIndex[i] = (r * 3 + g * 5 + b * 7) % 64;
	}

	uint16_t	lastPixel = 0;

	auto pRead = pBegin;
	while (pRead < pEnd)
	{
		uint8_t v = *pRead++;

		if (v < 0x20)
		{
			int nbPixels = v + 1;
			for (int i = 0; i < nbPixels; i++)
			{
				lastPixel = *pRead++;
				lastPixel |= (*pRead++) << 8;
				*pDest++ = lastPixel;

				palette[pixelToIndex[lastPixel]] = lastPixel;
			}
		}
		else if (v < 0x40)
		{
			int nbPixels = (v & 0x1F) + 1;
			for (int i = 0; i < nbPixels; i++)
				*pDest++ = lastPixel;
		}
		else if (v < 0x80)
		{
			int index = v & 0x3F;
			lastPixel = palette[index];
			*pDest++ = lastPixel;
		}
		else
		{
			uint8_t r = uint8_t(lastPixel & 0x001F);
			uint8_t g = uint8_t((lastPixel >> 5) & 0x003F);
			uint8_t b = uint8_t((lastPixel >> 11) & 0x001F);

			r += ((v >> 5) & 0x3) - 2;
			g += ((v >> 2) & 0x7) - 4;
			b += (v & 0x3) - 2;

			lastPixel = (b << 11) | (g << 5) | r;
			*pDest++ = lastPixel;

			palette[pixelToIndex[lastPixel]] = lastPixel;
		}
	}

	return int(pDest - pOriginalDest);
}