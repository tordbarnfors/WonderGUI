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
#include <wg_pixeltools.h>
#include <wg_gfxutil.h>
#include <wg_gfxbase.h>

#include <cstring>
#include <cmath>
#include <algorithm>
#include <vector>
#include <unordered_map>

namespace wg { namespace PixelTools
{

/*
	All conversions go through a buffer of pixels in a common format, a chunk of a line at a time:

		read -> [ color space conversion ] -> write

	Two common formats are used:

		Color8		Channels of 8 bits. Used when no channel of source or destination has more than 8 bits.
					Reading also converts the color space, using tables going straight from the bits of the
					source channel to 8 bits in the destination color space.

		Color16		Channels of 16 bits. Used when the source or destination has more than 8 bits per channel.
					Color space is converted on the whole buffer between reading and writing.

	Color8 has the same layout in memory as ARGB_8 in little endian byte order.
*/

struct Color16
{
	uint16_t	b, g, r, a;
};

static const int c_chunkPixels = 256;				// Pixels converted at a time. Must be a multiple of 16 for bitplanes.

//____ Tables _________________________________________________________________

static bool			s_bTablesInitialized = false;

static uint8_t		s_straightTabs[9][256];			// n bits -> 8 bits, same color space.
static uint8_t		s_toLinearTabs[9][256];			// n bits sRGB -> 8 bits linear.
static uint8_t		s_toSRGBTabs[9][256];			// n bits linear -> 8 bits sRGB.

static uint16_t*	s_pToLinear16 = nullptr;		// 16 bits sRGB -> 16 bits linear. Created when needed.
static uint16_t*	s_pToSRGB16 = nullptr;			// 16 bits linear -> 16 bits sRGB. Created when needed.

static double _sRGBToLinear(double c)
{
	return c <= 0.04045 ? c / 12.92 : pow((c + 0.055) / 1.055, 2.4);
}

static double _linearToSRGB(double l)
{
	return l <= 0.0031308 ? l * 12.92 : 1.055 * pow(l, 1 / 2.4) - 0.055;
}

//____ _initTables() __________________________________________________________

void _initTables()
{
	if (s_bTablesInitialized)
		return;

	for (int bits = 0; bits <= 8; bits++)
	{
		int max = (1 << bits) - 1;

		for (int i = 0; i < 256; i++)
		{
			if (bits == 0)
			{
				s_straightTabs[0][i] = 255;
				s_toLinearTabs[0][i] = 255;
				s_toSRGBTabs[0][i] = 255;
				continue;
			}

			double f = double(std::min(i, max)) / max;

			s_straightTabs[bits][i] = uint8_t(f * 255 + 0.5);
			s_toLinearTabs[bits][i] = uint8_t(_sRGBToLinear(f) * 255 + 0.5);
			s_toSRGBTabs[bits][i] = uint8_t(_linearToSRGB(f) * 255 + 0.5);
		}
	}

	s_bTablesInitialized = true;
}

//____ _releaseTables() _______________________________________________________

void _releaseTables()
{
	delete[] s_pToLinear16;
	delete[] s_pToSRGB16;
	s_pToLinear16 = nullptr;
	s_pToSRGB16 = nullptr;
}

//____ _init16BitTables() ______________________________________________________

static void _init16BitTables()
{
	if (s_pToLinear16)
		return;

	s_pToLinear16 = new uint16_t[65536];
	s_pToSRGB16 = new uint16_t[65536];

	for (int i = 0; i < 65536; i++)
	{
		s_pToLinear16[i] = uint16_t(_sRGBToLinear(i / 65535.0) * 65535 + 0.5);
		s_pToSRGB16[i] = uint16_t(_linearToSRGB(i / 65535.0) * 65535 + 0.5);
	}
}

//____ _convTab() ______________________________________________________________

static const uint8_t* _convTab(int bits, ColorSpace src, ColorSpace dst)
{
	if (src == dst)
		return s_straightTabs[bits];
	else if (dst == ColorSpace::Linear)
		return s_toLinearTabs[bits];
	else
		return s_toSRGBTabs[bits];
}

//____ _convertColor8() ________________________________________________________

static inline Color8 _convertColor8(Color8 col, ColorSpace src, ColorSpace dst)
{
	if (src == dst)
		return col;

	auto pTab = _convTab(8, src, dst);
	return Color8(pTab[col.r], pTab[col.g], pTab[col.b], col.a);
}

//____ Load and store ___________________________________________________________

template<int BYTES, bool BIGENDIAN>
static inline uint64_t _load(const uint8_t* p)
{
	uint64_t v = 0;
	if constexpr (BIGENDIAN)
	{
		for (int i = 0; i < BYTES; i++)
			v = (v << 8) | p[i];
	}
	else
	{
		for (int i = BYTES - 1; i >= 0; i--)
			v = (v << 8) | p[i];
	}
	return v;
}

template<int BYTES, bool BIGENDIAN>
static inline void _store(uint8_t* p, uint64_t v)
{
	if constexpr (BIGENDIAN)
	{
		for (int i = BYTES - 1; i >= 0; i--)
		{
			p[i] = uint8_t(v);
			v >>= 8;
		}
	}
	else
	{
		for (int i = 0; i < BYTES; i++)
		{
			p[i] = uint8_t(v);
			v >>= 8;
		}
	}
}

static inline uint64_t _load(const uint8_t* p, int bytes, bool bBigEndian)
{
	uint64_t v = 0;
	if (bBigEndian)
	{
		for (int i = 0; i < bytes; i++)
			v = (v << 8) | p[i];
	}
	else
	{
		for (int i = bytes - 1; i >= 0; i--)
			v = (v << 8) | p[i];
	}
	return v;
}

static inline void _store(uint8_t* p, uint64_t v, int bytes, bool bBigEndian)
{
	if (bBigEndian)
	{
		for (int i = bytes - 1; i >= 0; i--)
		{
			p[i] = uint8_t(v);
			v >>= 8;
		}
	}
	else
	{
		for (int i = 0; i < bytes; i++)
		{
			p[i] = uint8_t(v);
			v >>= 8;
		}
	}
}

static inline uint16_t _loadWord(const uint8_t* p, bool bBigEndian)
{
	return bBigEndian ? uint16_t((p[0] << 8) | p[1]) : uint16_t(p[0] | (p[1] << 8));
}

static inline void _storeWord(uint8_t* p, uint16_t v, bool bBigEndian)
{
	if (bBigEndian)
	{
		p[0] = uint8_t(v >> 8);
		p[1] = uint8_t(v);
	}
	else
	{
		p[0] = uint8_t(v);
		p[1] = uint8_t(v >> 8);
	}
}

//____ Channel ________________________________________________________________

struct Channel
{
	int			shift = 0;
	int			bits = 0;
	uint64_t	max = 0;			// Mask after shift.
};

static Channel _channel(uint64_t mask)
{
	Channel ch;

	if (mask == 0)
		return ch;

	while ((mask & 1) == 0)
	{
		mask >>= 1;
		ch.shift++;
	}

	ch.max = mask;

	while (mask & 1)
	{
		mask >>= 1;
		ch.bits++;
	}

	return ch;
}

static int _maxChannelBits(const PixelDescription& desc)
{
	if (desc.type != PixelType::Chunky)
		return 8;

	int bits = 0;
	for (uint64_t mask : { desc.B_mask, desc.G_mask, desc.R_mask, desc.A_mask })
		bits = std::max(bits, _channel(mask).bits);

	return bits;
}

//____ bytesPerLine() __________________________________________________________

int bytesPerLine(const PixelDescription& desc, int width)
{
	if (desc.type == PixelType::Bitplanes)
		return ((width + 15) / 16) * desc.bits * 2;
	else
		return width * desc.bits / 8;
}

//____ Reader _________________________________________________________________

struct Reader
{
	// Chunky

	Channel			ch[4];				// b, g, r, a
	const uint8_t*	pTab[4];			// Channel bits -> 8 bits in destination color space. Color8 only.
	uint8_t			fill8[4];			// Value of missing channels.
	uint16_t		fill16[4];

	// Index and bitplanes

	const Color8*	pPalette8 = nullptr;	// In destination color space.
	const Color16*	pPalette16 = nullptr;	// In source color space.
	int				paletteEntries = 0;

	bool			bBigEndian = false;
	int				planes = 0;			// Bitplanes, not counting any alpha plane.
	bool			bAlphaPlane = false;
};

typedef void(*Read8Func)(const uint8_t* pSrc, Color8* pDst, int nPixels, const Reader& rd);
typedef void(*Read16Func)(const uint8_t* pSrc, Color16* pDst, int nPixels, const Reader& rd);

//____ _readChunky8() __________________________________________________________

template<int BYTES, bool BIGENDIAN>
static void _readChunky8(const uint8_t* pSrc, Color8* pDst, int nPixels, const Reader& rd)
{
	for (int i = 0; i < nPixels; i++)
	{
		uint64_t v = _load<BYTES, BIGENDIAN>(pSrc);
		pSrc += BYTES;

		uint8_t c[4];
		for (int j = 0; j < 4; j++)
			c[j] = rd.ch[j].bits ? rd.pTab[j][(v >> rd.ch[j].shift) & rd.ch[j].max] : rd.fill8[j];

		pDst[i] = Color8(c[2], c[1], c[0], c[3]);
	}
}

//____ _readARGB8() ____________________________________________________________

// ARGB_8 and XRGB_8 in little endian byte order, no color space conversion.

static void _readARGB8(const uint8_t* pSrc, Color8* pDst, int nPixels, const Reader& rd)
{
	memcpy(pDst, pSrc, nPixels * 4);
}

static void _readXRGB8(const uint8_t* pSrc, Color8* pDst, int nPixels, const Reader& rd)
{
	memcpy(pDst, pSrc, nPixels * 4);
	for (int i = 0; i < nPixels; i++)
		pDst[i].a = 255;
}

//____ _readChunky16() _________________________________________________________

template<int BYTES, bool BIGENDIAN>
static void _readChunky16(const uint8_t* pSrc, Color16* pDst, int nPixels, const Reader& rd)
{
	for (int i = 0; i < nPixels; i++)
	{
		uint64_t v = _load<BYTES, BIGENDIAN>(pSrc);
		pSrc += BYTES;

		uint16_t c[4];
		for (int j = 0; j < 4; j++)
		{
			auto& ch = rd.ch[j];
			if (ch.bits == 0)
				c[j] = rd.fill16[j];
			else
			{
				uint64_t x = (v >> ch.shift) & ch.max;
				c[j] = ch.bits == 16 ? uint16_t(x) : uint16_t((x * 65535 + ch.max / 2) / ch.max);
			}
		}

		pDst[i] = { c[0], c[1], c[2], c[3] };
	}
}

//____ _readIndex() ____________________________________________________________

template<class COLOR, int BYTES>
static inline void _readIndex(const uint8_t* pSrc, COLOR* pDst, int nPixels, const COLOR* pPalette, int paletteEntries, bool bBigEndian)
{
	for (int i = 0; i < nPixels; i++)
	{
		int index = BYTES == 1 ? pSrc[0] : _loadWord(pSrc, bBigEndian);
		pSrc += BYTES;

		pDst[i] = index < paletteEntries ? pPalette[index] : COLOR();
	}
}

static void _readIndex8_8(const uint8_t* pSrc, Color8* pDst, int nPixels, const Reader& rd)
{
	_readIndex<Color8, 1>(pSrc, pDst, nPixels, rd.pPalette8, rd.paletteEntries, rd.bBigEndian);
}

static void _readIndex16_8(const uint8_t* pSrc, Color8* pDst, int nPixels, const Reader& rd)
{
	_readIndex<Color8, 2>(pSrc, pDst, nPixels, rd.pPalette8, rd.paletteEntries, rd.bBigEndian);
}

static void _readIndex8_16(const uint8_t* pSrc, Color16* pDst, int nPixels, const Reader& rd)
{
	_readIndex<Color16, 1>(pSrc, pDst, nPixels, rd.pPalette16, rd.paletteEntries, rd.bBigEndian);
}

static void _readIndex16_16(const uint8_t* pSrc, Color16* pDst, int nPixels, const Reader& rd)
{
	_readIndex<Color16, 2>(pSrc, pDst, nPixels, rd.pPalette16, rd.paletteEntries, rd.bBigEndian);
}

//____ _readBitplanes() ________________________________________________________

template<class COLOR>
static inline void _readBitplanes(const uint8_t* pSrc, COLOR* pDst, int nPixels, const COLOR* pPalette, int paletteEntries,
								  int planes, bool bAlphaPlane, bool bBigEndian)
{
	int totalPlanes = planes + (bAlphaPlane ? 1 : 0);

	while (nPixels > 0)
	{
		uint16_t words[9];
		for (int p = 0; p < totalPlanes; p++)
			words[p] = _loadWord(pSrc + p * 2, bBigEndian);

		const uint16_t* pPlanes = bAlphaPlane ? words + 1 : words;

		int pixels = std::min(16, nPixels);
		for (int i = 0; i < pixels; i++)
		{
			int bit = 15 - i;

			int index = 0;
			for (int p = 0; p < planes; p++)
				index |= ((pPlanes[p] >> bit) & 1) << p;

			COLOR col = index < paletteEntries ? pPalette[index] : COLOR();

			if (bAlphaPlane && ((words[0] >> bit) & 1) == 0)
				col.a = 0;

			*pDst++ = col;
		}

		pSrc += totalPlanes * 2;
		nPixels -= pixels;
	}
}

static void _readBitplanes8(const uint8_t* pSrc, Color8* pDst, int nPixels, const Reader& rd)
{
	_readBitplanes<Color8>(pSrc, pDst, nPixels, rd.pPalette8, rd.paletteEntries, rd.planes, rd.bAlphaPlane, rd.bBigEndian);
}

static void _readBitplanes16(const uint8_t* pSrc, Color16* pDst, int nPixels, const Reader& rd)
{
	_readBitplanes<Color16>(pSrc, pDst, nPixels, rd.pPalette16, rd.paletteEntries, rd.planes, rd.bAlphaPlane, rd.bBigEndian);
}

//____ _chunkyReadFunc8() ______________________________________________________

template<bool BIGENDIAN>
static Read8Func _chunkyReadFunc8(int bytes)
{
	switch (bytes)
	{
		case 1: return _readChunky8<1, BIGENDIAN>;
		case 2: return _readChunky8<2, BIGENDIAN>;
		case 3: return _readChunky8<3, BIGENDIAN>;
		case 4: return _readChunky8<4, BIGENDIAN>;
		case 8: return _readChunky8<8, BIGENDIAN>;
		default: return nullptr;
	}
}

template<bool BIGENDIAN>
static Read16Func _chunkyReadFunc16(int bytes)
{
	switch (bytes)
	{
		case 1: return _readChunky16<1, BIGENDIAN>;
		case 2: return _readChunky16<2, BIGENDIAN>;
		case 3: return _readChunky16<3, BIGENDIAN>;
		case 4: return _readChunky16<4, BIGENDIAN>;
		case 8: return _readChunky16<8, BIGENDIAN>;
		default: return nullptr;
	}
}

//____ _setupChunkyReader() ____________________________________________________

static void _setupChunkyReader(Reader& rd, const PixelDescription& desc, ColorSpace srcCS, ColorSpace dstCS)
{
	uint64_t masks[4] = { desc.B_mask, desc.G_mask, desc.R_mask, desc.A_mask };

	// Pixels with alpha only are white.

	bool bAlphaOnly = (desc.R_mask | desc.G_mask | desc.B_mask) == 0;

	for (int i = 0; i < 4; i++)
	{
		rd.ch[i] = _channel(masks[i]);

		if (i == 3)
			rd.pTab[i] = rd.ch[i].bits <= 8 ? s_straightTabs[rd.ch[i].bits] : nullptr;
		else
			rd.pTab[i] = rd.ch[i].bits <= 8 ? _convTab(rd.ch[i].bits, srcCS, dstCS) : nullptr;

		bool bFull = (i == 3 || bAlphaOnly);
		rd.fill8[i] = bFull ? 255 : 0;
		rd.fill16[i] = bFull ? 65535 : 0;
	}
}

//____ Writer _________________________________________________________________

class IndexMatcher;

struct Writer
{
	// Chunky

	Channel			ch[4];				// b, g, r, a
	uint8_t			pack8[4][256];		// 8 bits -> channel bits. Color8 only.
	uint64_t		padding = 0;		// Bits not part of any channel, which are set.

	// Index and bitplanes

	IndexMatcher*	pMatcher = nullptr;
	bool			bBigEndian = false;
	int				planes = 0;			// Bitplanes, not counting any alpha plane.
	bool			bAlphaPlane = false;
};

typedef bool(*Write8Func)(const Color8* pSrc, uint8_t* pDst, int nPixels, Writer& wr);
typedef bool(*Write16Func)(const Color16* pSrc, uint8_t* pDst, int nPixels, Writer& wr);

//____ IndexMatcher ____________________________________________________________

// Finds palette entries matching colors exactly, adding colors to the palette as needed.

class IndexMatcher
{
public:
	IndexMatcher(Color8* pPalette, int nEntries, int maxEntries) : m_pPalette(pPalette), m_nEntries(nEntries), m_maxEntries(maxEntries)
	{
		for (int i = 0; i < nEntries; i++)
			m_map.emplace(_key(pPalette[i]), i);			// First occurence is kept.
	}

	inline bool match(Color8 col, int& index)
	{
		uint32_t key = _key(col);

		auto it = m_map.find(key);
		if (it != m_map.end())
		{
			index = it->second;
			return true;
		}

		if (m_nEntries >= m_maxEntries || m_pPalette == nullptr)
			return false;

		m_pPalette[m_nEntries] = col;
		m_map.emplace(key, m_nEntries);
		index = m_nEntries++;
		return true;
	}

	inline int nbEntries() const { return m_nEntries; }

private:
	static inline uint32_t _key(Color8 col) { return (uint32_t(col.a) << 24) | (uint32_t(col.r) << 16) | (uint32_t(col.g) << 8) | col.b; }

	Color8*			m_pPalette;
	int				m_nEntries;
	int				m_maxEntries;

	std::unordered_map<uint32_t, int>	m_map;
};

//____ _writeChunky8() _________________________________________________________

template<int BYTES, bool BIGENDIAN>
static bool _writeChunky8(const Color8* pSrc, uint8_t* pDst, int nPixels, Writer& wr)
{
	for (int i = 0; i < nPixels; i++)
	{
		Color8 col = pSrc[i];
		uint8_t c[4] = { col.b, col.g, col.r, col.a };

		uint64_t v = wr.padding;
		for (int j = 0; j < 4; j++)
		{
			if (wr.ch[j].bits)
				v |= uint64_t(wr.pack8[j][c[j]]) << wr.ch[j].shift;
		}

		_store<BYTES, BIGENDIAN>(pDst, v);
		pDst += BYTES;
	}
	return true;
}

//____ _writeARGB8() ___________________________________________________________

// ARGB_8 and XRGB_8 in little endian byte order.

static bool _writeARGB8(const Color8* pSrc, uint8_t* pDst, int nPixels, Writer& wr)
{
	memcpy(pDst, pSrc, nPixels * 4);
	return true;
}

static bool _writeXRGB8(const Color8* pSrc, uint8_t* pDst, int nPixels, Writer& wr)
{
	memcpy(pDst, pSrc, nPixels * 4);
	for (int i = 0; i < nPixels; i++)
		pDst[i * 4 + 3] = 255;
	return true;
}

//____ _writeChunky16() ________________________________________________________

template<int BYTES, bool BIGENDIAN>
static bool _writeChunky16(const Color16* pSrc, uint8_t* pDst, int nPixels, Writer& wr)
{
	for (int i = 0; i < nPixels; i++)
	{
		Color16 col = pSrc[i];
		uint16_t c[4] = { col.b, col.g, col.r, col.a };

		uint64_t v = wr.padding;
		for (int j = 0; j < 4; j++)
		{
			auto& ch = wr.ch[j];
			if (ch.bits)
			{
				uint64_t x = ch.bits == 16 ? c[j] : (uint64_t(c[j]) * ch.max + 32767) / 65535;
				v |= x << ch.shift;
			}
		}

		_store<BYTES, BIGENDIAN>(pDst, v);
		pDst += BYTES;
	}
	return true;
}

//____ _writeIndex() ___________________________________________________________

template<int BYTES>
static bool _writeIndex(const Color8* pSrc, uint8_t* pDst, int nPixels, Writer& wr)
{
	for (int i = 0; i < nPixels; i++)
	{
		int index;
		if (!wr.pMatcher->match(pSrc[i], index))
			return false;

		if (BYTES == 1)
			*pDst++ = uint8_t(index);
		else
		{
			_storeWord(pDst, uint16_t(index), wr.bBigEndian);
			pDst += 2;
		}
	}
	return true;
}

//____ _writeBitplanes() _______________________________________________________

// Pixels of a last, partial word are merged with what is already in the destination.

static bool _writeBitplanes(const Color8* pSrc, uint8_t* pDst, int nPixels, Writer& wr)
{
	int totalPlanes = wr.planes + (wr.bAlphaPlane ? 1 : 0);

	while (nPixels > 0)
	{
		uint16_t words[9] = { 0,0,0,0,0,0,0,0,0 };
		uint16_t* pPlanes = wr.bAlphaPlane ? words + 1 : words;

		int pixels = std::min(16, nPixels);
		for (int i = 0; i < pixels; i++)
		{
			Color8 col = *pSrc++;
			int bit = 15 - i;

			if (wr.bAlphaPlane)
			{
				if (col.a < 128)
					continue;							// Transparent, all bits left cleared.

				words[0] |= 1 << bit;
				col.a = 255;
			}

			int index;
			if (!wr.pMatcher->match(col, index))
				return false;

			for (int p = 0; p < wr.planes; p++)
				pPlanes[p] |= ((index >> p) & 1) << bit;
		}

		uint16_t mask = uint16_t(0xFFFF << (16 - pixels));

		for (int p = 0; p < totalPlanes; p++)
		{
			uint16_t word = words[p];
			if (pixels < 16)
				word = (_loadWord(pDst + p * 2, wr.bBigEndian) & ~mask) | word;
			_storeWord(pDst + p * 2, word, wr.bBigEndian);
		}

		pDst += totalPlanes * 2;
		nPixels -= pixels;
	}
	return true;
}

//____ _chunkyWriteFunc8() ______________________________________________________

template<bool BIGENDIAN>
static Write8Func _chunkyWriteFunc8(int bytes)
{
	switch (bytes)
	{
		case 1: return _writeChunky8<1, BIGENDIAN>;
		case 2: return _writeChunky8<2, BIGENDIAN>;
		case 4: return _writeChunky8<4, BIGENDIAN>;
		case 8: return _writeChunky8<8, BIGENDIAN>;
		default: return nullptr;
	}
}

template<bool BIGENDIAN>
static Write16Func _chunkyWriteFunc16(int bytes)
{
	switch (bytes)
	{
		case 1: return _writeChunky16<1, BIGENDIAN>;
		case 2: return _writeChunky16<2, BIGENDIAN>;
		case 4: return _writeChunky16<4, BIGENDIAN>;
		case 8: return _writeChunky16<8, BIGENDIAN>;
		default: return nullptr;
	}
}

//____ _setupChunkyWriter() ____________________________________________________

static void _setupChunkyWriter(Writer& wr, const PixelDescription& desc)
{
	uint64_t masks[4] = { desc.B_mask, desc.G_mask, desc.R_mask, desc.A_mask };

	for (int i = 0; i < 4; i++)
	{
		wr.ch[i] = _channel(masks[i]);

		if (wr.ch[i].bits <= 8)
		{
			int max = int(wr.ch[i].max);
			for (int v = 0; v < 256; v++)
				wr.pack8[i][v] = uint8_t((v * max + 127) / 255);
		}
	}

	uint64_t allBits = desc.bits >= 64 ? ~uint64_t(0) : (uint64_t(1) << desc.bits) - 1;
	wr.padding = allBits & ~(desc.B_mask | desc.G_mask | desc.R_mask | desc.A_mask);
}

//____ _convertColorSpace16() __________________________________________________

static void _convertColorSpace16(Color16* pPixels, int nPixels, ColorSpace dstCS)
{
	_init16BitTables();
	const uint16_t* pTab = dstCS == ColorSpace::Linear ? s_pToLinear16 : s_pToSRGB16;

	for (int i = 0; i < nPixels; i++)
	{
		pPixels[i].b = pTab[pPixels[i].b];
		pPixels[i].g = pTab[pPixels[i].g];
		pPixels[i].r = pTab[pPixels[i].r];
	}
}

//____ _narrow() _______________________________________________________________

static inline uint8_t _narrow(uint16_t v)
{
	return uint8_t((uint32_t(v) * 255 + 32767) / 65535);
}

static void _narrow(const Color16* pSrc, Color8* pDst, int nPixels)
{
	for (int i = 0; i < nPixels; i++)
		pDst[i] = Color8(_narrow(pSrc[i].r), _narrow(pSrc[i].g), _narrow(pSrc[i].b), _narrow(pSrc[i].a));
}

//____ _identicalPalettes() ____________________________________________________

static bool _identicalPalettes(const Color8* pPalette1, const Color8* pPalette2, int nEntries)
{
	for (int i = 0; i < nEntries; i++)
		if (pPalette1[i] != pPalette2[i])
			return false;

	return true;
}

//____ _copyRows() _____________________________________________________________

static void _copyRows(int height, int lineBytes, const uint8_t* pSrc, int srcPitchAdd, uint8_t* pDst, int dstPitchAdd)
{
	for (int y = 0; y < height; y++)
	{
		memcpy(pDst, pSrc, lineBytes);
		pSrc += lineBytes + srcPitchAdd;
		pDst += lineBytes + dstPitchAdd;
	}
}

//____ _remapIndexes() __________________________________________________________

// Copies indexed pixels to indexed pixels with another palette.

static bool _remapIndexes(int width, int height, const uint8_t* pSrc, const PixelDescription& srcDesc, ColorSpace srcCS, int srcPitchAdd,
						  const Color8* pSrcPalette, int srcPaletteEntries,
						  uint8_t* pDst, const PixelDescription& dstDesc, ColorSpace dstCS, int dstPitchAdd,
						  Color8* pDstPalette, int& dstPaletteEntries, int maxDstPaletteEntries)
{
	int srcBytes = srcDesc.bits / 8;
	int dstBytes = dstDesc.bits / 8;

	// Find out which colors are used.

	std::vector<bool>	used(srcPaletteEntries, false);

	const uint8_t* p = pSrc;
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int index = srcBytes == 1 ? p[0] : _loadWord(p, srcDesc.bigEndian);
			if (index < srcPaletteEntries)
				used[index] = true;
			p += srcBytes;
		}
		p += srcPitchAdd;
	}

	// Map them to entries in destination palette.

	std::vector<int>	remap(srcPaletteEntries, 0);

	IndexMatcher matcher(pDstPalette, dstPaletteEntries, maxDstPaletteEntries);

	bool bOk = true;
	for (int i = 0; i < srcPaletteEntries && bOk; i++)
	{
		if (used[i])
			bOk = matcher.match(_convertColor8(pSrcPalette[i], srcCS, dstCS), remap[i]);
	}

	dstPaletteEntries = matcher.nbEntries();

	if (!bOk)
		return false;

	// Copy and remap

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int index = srcBytes == 1 ? pSrc[0] : _loadWord(pSrc, srcDesc.bigEndian);
			pSrc += srcBytes;

			index = index < srcPaletteEntries ? remap[index] : 0;

			if (dstBytes == 1)
				*pDst++ = uint8_t(index);
			else
			{
				_storeWord(pDst, uint16_t(index), dstDesc.bigEndian);
				pDst += 2;
			}
		}
		pSrc += srcPitchAdd;
		pDst += dstPitchAdd;
	}

	return true;
}

//____ copyPixels() [PixelFormat] _____________________________________________

bool copyPixels(int width, int height,
				const uint8_t* pSrc, PixelFormat srcFormat, ColorSpace srcColorSpace, bool srcBigEndian, int srcPitchAdd,
				const Color8* pSrcPalette, int srcPaletteEntries,
				uint8_t* pDst, PixelFormat dstFormat, ColorSpace dstColorSpace, bool dstBigEndian, int dstPitchAdd,
				Color8* pDstPalette, int& dstPaletteEntries, int maxDstPaletteEntries)
{
	if (srcFormat == PixelFormat::Undefined)
		return false;

	return copyPixels(width, height, pSrc, Util::pixelFormatToDescription(srcFormat, srcBigEndian), srcColorSpace, srcPitchAdd,
					  pSrcPalette, srcPaletteEntries, pDst, dstFormat, dstColorSpace, dstBigEndian, dstPitchAdd,
					  pDstPalette, dstPaletteEntries, maxDstPaletteEntries);
}

//____ copyPixels() [PixelDescription] ________________________________________

bool copyPixels(int width, int height,
				const uint8_t* pSrc, const PixelDescription& srcDesc, ColorSpace srcCS, int srcPitchAdd,
				const Color8* pSrcPalette, int srcPaletteEntries,
				uint8_t* pDst, PixelFormat dstFormat, ColorSpace dstCS, bool dstBigEndian, int dstPitchAdd,
				Color8* pDstPalette, int& dstPaletteEntries, int maxDstPaletteEntries)
{
	_initTables();

	if (width <= 0 || height <= 0)
		return true;

	if (dstFormat == PixelFormat::Undefined || srcDesc.bits == 0)
		return false;

	if (srcCS == ColorSpace::Undefined)
		srcCS = ColorSpace::sRGB;
	if (dstCS == ColorSpace::Undefined)
		dstCS = ColorSpace::sRGB;

	PixelDescription dstDesc = Util::pixelFormatToDescription(dstFormat, dstBigEndian);

	bool bSrcHasPalette = (srcDesc.type == PixelType::Index || srcDesc.type == PixelType::Bitplanes);
	bool bDstHasPalette = (dstDesc.type == PixelType::Index || dstDesc.type == PixelType::Bitplanes);

	if (bSrcHasPalette && pSrcPalette == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Source pixels need a palette.", nullptr, nullptr, __func__, __FILE__, __LINE__);
		return false;
	}

	if (bDstHasPalette && pDstPalette == nullptr)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Destination pixels need a palette.", nullptr, nullptr, __func__, __FILE__, __LINE__);
		return false;
	}

	if (srcDesc.type == PixelType::Chunky && srcDesc.bits != 8 && srcDesc.bits != 16 && srcDesc.bits != 24 && srcDesc.bits != 32 && srcDesc.bits != 64)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::InvalidParam, "Chunky pixels need to be 8, 16, 24, 32 or 64 bits.", nullptr, nullptr, __func__, __FILE__, __LINE__);
		return false;
	}

	int srcLineBytes = bytesPerLine(srcDesc, width);
	int dstLineBytes = bytesPerLine(dstDesc, width);

	// Straight copy if layouts are identical.

	bool bSameByteOrder = (srcDesc.bigEndian == dstDesc.bigEndian) || (srcDesc.bits == 8 && srcDesc.type != PixelType::Bitplanes);

	if (srcDesc == PixelDescription(dstDesc.bits, dstDesc.type, dstDesc.R_mask, dstDesc.G_mask, dstDesc.B_mask, dstDesc.A_mask, srcDesc.bigEndian) &&
		bSameByteOrder && srcCS == dstCS)
	{
		if (!bSrcHasPalette)
		{
			_copyRows(height, srcLineBytes, pSrc, srcPitchAdd, pDst, dstPitchAdd);
			return true;
		}

		if (srcPaletteEntries <= dstPaletteEntries && _identicalPalettes(pSrcPalette, pDstPalette, srcPaletteEntries))
		{
			_copyRows(height, srcLineBytes, pSrc, srcPitchAdd, pDst, dstPitchAdd);
			return true;
		}
	}

	// Remap indexes between palettes.

	if (srcDesc.type == PixelType::Index && dstDesc.type == PixelType::Index)
		return _remapIndexes(width, height, pSrc, srcDesc, srcCS, srcPitchAdd, pSrcPalette, srcPaletteEntries,
							 pDst, dstDesc, dstCS, dstPitchAdd, pDstPalette, dstPaletteEntries, maxDstPaletteEntries);

	// General conversion. Decide on 8 or 16 bit channels.

	bool b16Bit = _maxChannelBits(srcDesc) > 8 || _maxChannelBits(dstDesc) > 8;

	// Setup reader

	Reader		rd;
	Read8Func	pRead8 = nullptr;
	Read16Func	pRead16 = nullptr;

	std::vector<Color8>		palette8;
	std::vector<Color16>	palette16;

	rd.bBigEndian = srcDesc.bigEndian;

	if (srcDesc.type == PixelType::Chunky)
	{
		_setupChunkyReader(rd, srcDesc, srcCS, dstCS);

		int bytes = srcDesc.bits / 8;

		if (b16Bit)
			pRead16 = srcDesc.bigEndian ? _chunkyReadFunc16<true>(bytes) : _chunkyReadFunc16<false>(bytes);
		else if (srcCS == dstCS && !srcDesc.bigEndian && bytes == 4 && srcDesc.R_mask == 0xFF0000 && srcDesc.G_mask == 0xFF00 && srcDesc.B_mask == 0xFF &&
				 (srcDesc.A_mask == 0xFF000000 || srcDesc.A_mask == 0))
			pRead8 = srcDesc.A_mask ? _readARGB8 : _readXRGB8;
		else
			pRead8 = srcDesc.bigEndian ? _chunkyReadFunc8<true>(bytes) : _chunkyReadFunc8<false>(bytes);
	}
	else
	{
		rd.paletteEntries = srcPaletteEntries;

		if (b16Bit)
		{
			palette16.resize(srcPaletteEntries);
			for (int i = 0; i < srcPaletteEntries; i++)
			{
				Color8 c = pSrcPalette[i];
				palette16[i] = { uint16_t(c.b * 257), uint16_t(c.g * 257), uint16_t(c.r * 257), uint16_t(c.a * 257) };
			}
			rd.pPalette16 = palette16.data();
		}
		else
		{
			palette8.resize(srcPaletteEntries);
			for (int i = 0; i < srcPaletteEntries; i++)
				palette8[i] = _convertColor8(pSrcPalette[i], srcCS, dstCS);
			rd.pPalette8 = palette8.data();
		}

		if (srcDesc.type == PixelType::Index)
		{
			if (b16Bit)
				pRead16 = srcDesc.bits == 8 ? _readIndex8_16 : _readIndex16_16;
			else
				pRead8 = srcDesc.bits == 8 ? _readIndex8_8 : _readIndex16_8;
		}
		else
		{
			rd.bAlphaPlane = srcDesc.A_mask != 0;
			rd.planes = srcDesc.bits - (rd.bAlphaPlane ? 1 : 0);

			if (b16Bit)
				pRead16 = _readBitplanes16;
			else
				pRead8 = _readBitplanes8;
		}
	}

	if (pRead8 == nullptr && pRead16 == nullptr)
		return false;

	// Setup writer

	Writer		wr;
	Write8Func	pWrite8 = nullptr;
	Write16Func	pWrite16 = nullptr;

	IndexMatcher matcher(pDstPalette, dstPaletteEntries, maxDstPaletteEntries);

	wr.bBigEndian = dstDesc.bigEndian;
	wr.pMatcher = &matcher;

	if (dstDesc.type == PixelType::Chunky)
	{
		_setupChunkyWriter(wr, dstDesc);

		int bytes = dstDesc.bits / 8;

		if (_maxChannelBits(dstDesc) > 8)
			pWrite16 = dstDesc.bigEndian ? _chunkyWriteFunc16<true>(bytes) : _chunkyWriteFunc16<false>(bytes);
		else if (dstFormat == PixelFormat::ARGB_8 && !dstDesc.bigEndian)
			pWrite8 = _writeARGB8;
		else if (dstFormat == PixelFormat::XRGB_8 && !dstDesc.bigEndian)
			pWrite8 = _writeXRGB8;
		else
			pWrite8 = dstDesc.bigEndian ? _chunkyWriteFunc8<true>(bytes) : _chunkyWriteFunc8<false>(bytes);
	}
	else if (dstDesc.type == PixelType::Index)
	{
		pWrite8 = dstDesc.bits == 8 ? _writeIndex<1> : _writeIndex<2>;
	}
	else
	{
		wr.bAlphaPlane = dstDesc.A_mask != 0;
		wr.planes = dstDesc.bits - (wr.bAlphaPlane ? 1 : 0);
		pWrite8 = _writeBitplanes;
	}

	if (pWrite8 == nullptr && pWrite16 == nullptr)
		return false;

	// Convert

	Color8	buffer8[c_chunkPixels];
	Color16	buffer16[c_chunkPixels];

	// Bitplanes have as many bytes per 16 pixels as chunky pixels of the same number of bits,
	// so we step through both with bits / 8 bytes per pixel, since chunks are multiples of 16 pixels.

	bool bOk = true;

	for (int y = 0; y < height && bOk; y++)
	{
		const uint8_t* pS = pSrc;
		uint8_t* pD = pDst;

		for (int x = 0; x < width && bOk; x += c_chunkPixels)
		{
			int pixels = std::min(c_chunkPixels, width - x);

			if (pRead16)
			{
				pRead16(pS, buffer16, pixels, rd);

				if (srcCS != dstCS)
					_convertColorSpace16(buffer16, pixels, dstCS);

				if (pWrite16)
					bOk = pWrite16(buffer16, pD, pixels, wr);
				else
				{
					_narrow(buffer16, buffer8, pixels);
					bOk = pWrite8(buffer8, pD, pixels, wr);
				}
			}
			else
			{
				pRead8(pS, buffer8, pixels, rd);

				if (pWrite8)
					bOk = pWrite8(buffer8, pD, pixels, wr);
				else
				{
					for (int i = 0; i < pixels; i++)
					{
						Color8 c = buffer8[i];
						buffer16[i] = { uint16_t(c.b * 257), uint16_t(c.g * 257), uint16_t(c.r * 257), uint16_t(c.a * 257) };
					}
					bOk = pWrite16(buffer16, pD, pixels, wr);
				}
			}

			pS += pixels * srcDesc.bits / 8;
			pD += pixels * dstDesc.bits / 8;
		}

		pSrc += srcLineBytes + srcPitchAdd;
		pDst += dstLineBytes + dstPitchAdd;
	}

	if (bDstHasPalette)
		dstPaletteEntries = matcher.nbEntries();

	if (!bOk)
	{
		GfxBase::throwError(ErrorLevel::Error, ErrorCode::FailedPrerequisite, "Out of entries in destination palette. All colors can not be represented. Some pixels copied to destination, but operation aborted.", nullptr, nullptr, __func__, __FILE__, __LINE__);
		return false;
	}

	return true;
}

//____ colorToPixelBytes() ____________________________________________________

int colorToPixelBytes(HiColor color, PixelFormat format, ColorSpace colorSpace, bool bigEndian, uint8_t pixelArea[18], const Color8* pPalette, int paletteSize)
{
	_initTables();

	auto desc = Util::pixelFormatToDescription(format, bigEndian);

	if (colorSpace == ColorSpace::Undefined)
		colorSpace = ColorSpace::sRGB;

	auto clamp = [](int v) { return v < 0 ? 0 : v > 4096 ? 4096 : v; };

	if (desc.type == PixelType::Chunky)
	{
		// Channels with 16 bits precision in the color space of the pixels.

		auto toChannel = [colorSpace, clamp](int v)
		{
			double f = clamp(v) / 4096.0;
			if (colorSpace == ColorSpace::Linear)
				f = _sRGBToLinear(f);
			return uint64_t(f * 65535 + 0.5);
		};

		uint64_t c[4] = { toChannel(color.b), toChannel(color.g), toChannel(color.r), uint64_t(clamp(color.a) * 65535 / 4096) };
		uint64_t masks[4] = { desc.B_mask, desc.G_mask, desc.R_mask, desc.A_mask };

		uint64_t allBits = desc.bits >= 64 ? ~uint64_t(0) : (uint64_t(1) << desc.bits) - 1;
		uint64_t v = allBits & ~(desc.B_mask | desc.G_mask | desc.R_mask | desc.A_mask);

		for (int i = 0; i < 4; i++)
		{
			Channel ch = _channel(masks[i]);
			if (ch.bits)
				v |= ((c[i] * ch.max + 32767) / 65535) << ch.shift;
		}

		int bytes = desc.bits / 8;
		_store(pixelArea, v, bytes, desc.bigEndian);
		return bytes;
	}
	else if (desc.type == PixelType::Index)
	{
		int index = findBestMatchInPalette(color, colorSpace, pPalette, paletteSize);

		if (desc.bits == 8)
		{
			pixelArea[0] = uint8_t(index);
			return 1;
		}

		_storeWord(pixelArea, uint16_t(index), desc.bigEndian);
		return 2;
	}
	else
	{
		bool bAlphaPlane = desc.A_mask != 0;
		int planes = desc.bits - (bAlphaPlane ? 1 : 0);

		uint8_t* p = pixelArea;

		if (bAlphaPlane)
		{
			_storeWord(p, color.a >= 2048 ? 0xFFFF : 0, desc.bigEndian);
			p += 2;
			color.a = 4096;
		}

		int index = findBestMatchInPalette(color, colorSpace, pPalette, paletteSize);

		for (int i = 0; i < planes; i++)
		{
			_storeWord(p, ((index >> i) & 1) ? 0xFFFF : 0, desc.bigEndian);
			p += 2;
		}

		return desc.bits * 2;
	}
}

//____ fillBitmap() ___________________________________________________________

void fillBitmap(uint8_t* pBitmap, PixelFormat format, ColorSpace colorSpace, bool bigEndian, int pitch, const RectI& fillRect,
				HiColor color, const Color8* pPalette, int paletteSize)
{
	auto desc = Util::pixelFormatToDescription(format, bigEndian);

	uint8_t pixelArea[18];
	int pixelBytes = colorToPixelBytes(color, format, colorSpace, bigEndian, pixelArea, pPalette, paletteSize);

	if (pixelBytes == 0 || fillRect.w <= 0 || fillRect.h <= 0)
		return;

	if (desc.type == PixelType::Bitplanes)
	{
		int planes = desc.bits;
		int groupBytes = planes * 2;

		int firstGroup = fillRect.x / 16;
		int lastGroup = (fillRect.x + fillRect.w - 1) / 16;

		for (int y = fillRect.y; y < fillRect.y + fillRect.h; y++)
		{
			uint8_t* pLine = pBitmap + y * pitch;

			for (int group = firstGroup; group <= lastGroup; group++)
			{
				int beg = std::max(fillRect.x, group * 16) - group * 16;
				int end = std::min(fillRect.x + fillRect.w, group * 16 + 16) - group * 16;

				uint16_t mask = uint16_t((0xFFFF >> beg) & (0xFFFF << (16 - end)));

				uint8_t* pGroup = pLine + group * groupBytes;

				for (int p = 0; p < planes; p++)
				{
					uint16_t fill = _loadWord(pixelArea + p * 2, desc.bigEndian);
					uint16_t word = _loadWord(pGroup + p * 2, desc.bigEndian);
					_storeWord(pGroup + p * 2, uint16_t((word & ~mask) | (fill & mask)), desc.bigEndian);
				}
			}
		}
	}
	else
	{
		for (int y = fillRect.y; y < fillRect.y + fillRect.h; y++)
		{
			uint8_t* pDest = pBitmap + y * pitch + fillRect.x * pixelBytes;

			switch (pixelBytes)
			{
				case 1:
					memset(pDest, pixelArea[0], fillRect.w);
					break;

				default:
					for (int x = 0; x < fillRect.w; x++)
					{
						memcpy(pDest, pixelArea, pixelBytes);
						pDest += pixelBytes;
					}
					break;
			}
		}
	}
}

//____ findBestMatchInPalette() ____________________________________________

int findBestMatchInPalette(HiColor color, ColorSpace paletteColorSpace, const Color8* pPalette, int paletteSize)
{
	if (pPalette == nullptr || paletteSize <= 0)
		return 0;

	if (paletteColorSpace == ColorSpace::Linear)
		color = color.toLinear();

	Color8 col = color;

	// First we make a quick check for exact match

	for (int i = 0; i < paletteSize; i++)
		if (col == pPalette[i])
			return i;

	// Find best match

	int bestMatchIndex = 0;
	int bestMatchDiff = 256 * 256 * 4;

	for (int i = 0; i < paletteSize; i++)
	{
		const Color8& palCol = pPalette[i];

		int diffB = col.b - palCol.b;
		int diffG = col.g - palCol.g;
		int diffR = col.r - palCol.r;
		int diffA = col.a - palCol.a;

		int combDiff = diffB * diffB + diffG * diffG + diffR * diffR + diffA * diffA;

		if (combDiff < bestMatchDiff)
		{
			bestMatchDiff = combDiff;
			bestMatchIndex = i;
		}
	}

	return bestMatchIndex;
}

//____ extractAlphaChannel() __________________________________________________

bool extractAlphaChannel(PixelFormat format, bool bigEndian, const uint8_t* pSrc, int srcPitch, const RectI& srcRect, uint8_t* pDst, int dstPitch, const Color8* pPalette)
{
	_initTables();

	auto desc = Util::pixelFormatToDescription(format, bigEndian);

	switch (desc.type)
	{
		case PixelType::Index:
		{
			if (!pPalette)
				return false;

			int bytes = desc.bits / 8;

			for (int y = 0; y < srcRect.h; y++)
			{
				const uint8_t* pS = pSrc + (srcRect.y + y) * srcPitch + srcRect.x * bytes;
				uint8_t* pD = pDst + y * dstPitch;

				for (int x = 0; x < srcRect.w; x++)
				{
					int index = bytes == 1 ? pS[0] : _loadWord(pS, desc.bigEndian);
					pS += bytes;
					*pD++ = pPalette[index].a;
				}
			}
			return true;
		}

		case PixelType::Chunky:
		{
			if (desc.A_mask == 0)
				return false;

			Channel ch = _channel(desc.A_mask);
			int bytes = desc.bits / 8;

			for (int y = 0; y < srcRect.h; y++)
			{
				const uint8_t* pS = pSrc + (srcRect.y + y) * srcPitch + srcRect.x * bytes;
				uint8_t* pD = pDst + y * dstPitch;

				for (int x = 0; x < srcRect.w; x++)
				{
					uint64_t a = (_load(pS, bytes, desc.bigEndian) >> ch.shift) & ch.max;
					pS += bytes;

					*pD++ = ch.bits <= 8 ? s_straightTabs[ch.bits][a] : uint8_t((a * 255 + ch.max / 2) / ch.max);
				}
			}
			return true;
		}

		case PixelType::Bitplanes:
		{
			bool bAlphaPlane = desc.A_mask != 0;

			if (!bAlphaPlane && !pPalette)
				return false;

			int planes = desc.bits - (bAlphaPlane ? 1 : 0);
			int groupBytes = desc.bits * 2;

			for (int y = 0; y < srcRect.h; y++)
			{
				const uint8_t* pLine = pSrc + (srcRect.y + y) * srcPitch;
				uint8_t* pD = pDst + y * dstPitch;

				for (int x = srcRect.x; x < srcRect.x + srcRect.w; x++)
				{
					const uint8_t* pGroup = pLine + (x / 16) * groupBytes;
					int bit = 15 - (x % 16);

					if (bAlphaPlane)
					{
						*pD++ = ((_loadWord(pGroup, desc.bigEndian) >> bit) & 1) ? 255 : 0;
					}
					else
					{
						int index = 0;
						for (int p = 0; p < planes; p++)
							index |= ((_loadWord(pGroup + p * 2, desc.bigEndian) >> bit) & 1) << p;

						*pD++ = pPalette[index].a;
					}
				}
			}
			return true;
		}
	}

	return false;
}

} } // namespace wg::PixelTools
