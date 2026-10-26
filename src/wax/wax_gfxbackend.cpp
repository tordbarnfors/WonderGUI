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
#include <wg_gfxbackend.h>

#include <cstring>
#include <algorithm>

namespace wg
{
	const TypeInfo GfxBackend::TYPEINFO = { "GfxBackend", &Object::TYPEINFO };

	// Transforms for flipping movement over SOURCE when blitting

const Transform			GfxBackend::s_standardTransforms[NbStandardTransforms] = {

	{ 1,0,0,1 },			// Normal

	{ -1,0,0,1 },			// FlipX
	{ 1,0,0,-1 },			// FlipY
	{ 0,-1,1,0 },			// Rot90
	{ 0,1,1,0 },			// Rot90FlipX
	{ 0,-1,-1,0 },			// Rot90FlipY
	{ -1,0,0,-1 },			// Rot180
	{ 1,0,0,-1 },			// Rot180FlipX
	{ -1,0,0,1 },			// Rot180FlipY
	{ 0,1,-1,0 },			// Rot270
	{ 0,-1,-1,0 },			// Rot270FlipX
	{ 0,1,1,0 },			// Rot270FlipY

	{ 0,0,0,1 },			// FloodX
	{ 1,0,0,0 },			// FloodY
	{ 0,0,0,0 },			// FloodXY
};

const int GfxBackend::s_defaultBlur[9] = { 6553, 6553, 6553, 6553, 6553, 6553, 6553, 6553, 6553 };
const spx GfxBackend::s_defaultBlurRadius = 4 * 64;


//____ typeInfo() _________________________________________________________

const TypeInfo& GfxBackend::typeInfo(void) const
{
	return TYPEINFO;
}

//____ decodeRoundedBox() _________________________________________________

GfxBackend::RoundedBoxPayload GfxBackend::decodeRoundedBox(const uint16_t* pPayload)
{
	spx values[13];
	std::memcpy(values, pPayload, sizeof(values));

	RoundedBoxPayload box;
	box.rect = { values[0], values[1], values[2], values[3] };

	for (int i = 0; i < 4; i++)
		box.radius[i] = values[4 + i];

	box.outline = { values[8], values[9], values[10], values[11] };
	box.softness = values[12];
	box.flags = pPayload[26];
	box.pTints = pPayload + c_roundedBoxPayloadWords;
	return box;
}

//____ decodeRing() _______________________________________________________

GfxBackend::RingPayload GfxBackend::decodeRing(const uint16_t* pPayload)
{
	spx values[7];
	std::memcpy(values, pPayload, sizeof(values));

	RingPayload ring;
	ring.rect = { values[0], values[1], values[2], values[3] };
	ring.thickness = values[4];
	ring.outline = values[5];
	ring.softness = values[6];
	ring.flags = pPayload[14];
	ring.nSlices = pPayload[15];
	std::memcpy(&ring.start, pPayload + 16, 4);
	ring.pSlices = pPayload + c_ringPayloadWords;
	ring.pTints = ring.pSlices + ring.nSlices * c_ringSliceWords;
	return ring;
}

//____ RingPayload::sliceSize() ___________________________________________

uint32_t GfxBackend::RingPayload::sliceSize(int slice) const
{
	uint32_t size;
	std::memcpy(&size, pSlices + slice * c_ringSliceWords, 4);
	return size;
}

//____ RingPayload::sliceFlags() __________________________________________

uint16_t GfxBackend::RingPayload::sliceFlags(int slice) const
{
	return pSlices[slice * c_ringSliceWords + 2];
}

//____ decodeWarp() _______________________________________________________

const uint16_t* GfxBackend::decodeWarp(Command cmd, const uint16_t* p, WarpParams& params)
{
	params.nRects = *p++;

	params.warpmap = readObjectIndex(p);
	params.edgemap = (cmd == Command::WarpDrawEdgemap) ? readObjectIndex(p) : 0;

	spx values[4];

	std::memcpy(values, p, 16);
	p += 8;

	params.dest = { values[0], values[1] };
	params.sourceOffset = { values[2], values[3] };

	if (cmd == Command::WarpBlit)
	{
		std::memcpy(values, p, 16);
		p += 8;

		params.source = { values[0], values[1], values[2], values[3] };
	}
	else
		params.source = { 0,0,0,0 };

	return p;
}

//____ encodeLineCaps() ___________________________________________________

uint16_t GfxBackend::encodeLineCaps(LineCaps caps)
{
	auto encodeCap = [](LineCap cap) -> uint16_t
	{
		if (cap.shape > LineCapShape::Spear)
			return 0;

		uint16_t word = uint16_t(cap.shape);

		if (cap.shape == LineCapShape::Spear)
		{
			int steps = std::min(std::max((cap.spearLength + 12) / 25, 1), 32);
			word |= uint16_t(steps - 1) << 3;
		}
		return word;
	};

	return encodeCap(caps.begin) | (encodeCap(caps.end) << 8);
}

//____ decodeLineCaps() ___________________________________________________

LineCaps GfxBackend::decodeLineCaps(uint16_t word)
{
	auto decodeCap = [](uint16_t bits) -> LineCap
	{
		auto shape = LineCapShape(bits & 0x7);
		if (shape > LineCapShape::Spear)
			return LineCap();

		uint16_t spearLength = shape == LineCapShape::Spear ? uint16_t(((bits >> 3) + 1) * 25) : 200;
		return LineCap(shape, spearLength);
	};

	return LineCaps(decodeCap(word & 0xFF), decodeCap(word >> 8));
}

} // namespace wg
