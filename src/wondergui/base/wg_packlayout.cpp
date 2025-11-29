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
#include <wg_packlayout.h>
#include <wg_base.h>
#include <wg_util.h>

namespace wg
{
	using namespace Util;


	const TypeInfo PackLayout::TYPEINFO = { "PackLayout", &Object::TYPEINFO };


	//____ create() ___________________________________________________________

	PackLayout_p PackLayout::create(const Blueprint& blueprint) 
	{ 
		return PackLayout_p(new PackLayout(blueprint)); 
	}

	//____ constructor ____________________________________________________________

	PackLayout::PackLayout(const Blueprint& bp)
	{
		m_wantedSize	= bp.wantedSize;
		m_expandFactor	= bp.expandFactor;
		m_shrinkFactor	= bp.shrinkFactor;
		m_hideSetting	= bp.hideSetting;

		

		if (!bp.list1.empty())
			_convertList(m_list1, bp.list1);

		if (!bp.list2.empty())
			_convertList(m_list2, bp.list2);

		if (!bp.list3.empty())
			_convertList(m_list3, bp.list3);

		m_minSizeSource = bp.minSizeSource;
		m_defaultSizeSource = bp.defaultSizeSource;
		m_maxSizeSource = bp.maxSizeSource;

		m_expandOrder = bp.expandOrder;
		m_shrinkOrder = bp.shrinkOrder;

		if (m_wantedSize != WantedSize::Default || m_defaultSizeSource != SizeSource::Item)
			m_bCalcWantedLength = true;

		if (m_minSizeSource == SizeSource::Weight || m_defaultSizeSource == SizeSource::Weight || m_maxSizeSource == SizeSource::Weight)
			m_bUseSpxWeight = true;

		if (bp.finalizer)
			setFinalizer(bp.finalizer);
	}

	//____ Destructor _____________________________________________________________

	PackLayout::~PackLayout()
	{
	}

	//____ typeInfo() _________________________________________________________

	const TypeInfo& PackLayout::typeInfo(void) const
	{
		return TYPEINFO;
	}

	//____ getItemSizes() ___________________________________________________

	spx PackLayout::getItemSizes(spx* pOutput, spx availableSpace, int scale, int nItems, const Item* pItems)
	{
		// Updated lists cached spx values if scale has changed

		if (scale != m_cacheScale)
			_updateSpxCache(scale);

		// Convert weight to spx if needed below

		int spxWeightAllocAmount = 0;
		auto pWeightSpx = nullptr;

		if (m_bUseSpxWeight)
		{
			spxWeightAllocAmount = sizeof(spx) * nItems;
			auto pWeightSpx = (spx*)Base::memStackAlloc(spxWeightAllocAmount);

			for (int i = 0; i < nItems; i++)
				pWeightSpx[i] = Util::align(spx( ((pItems[i].weight>>8) * scale)>>8));
		}

		spx totalLength = 0;


		// Calc and store the start length for all items

		{

			// Calculate start lengths

			switch (m_wantedSize)
			{
			case WantedSize::Default:
			{
				const spx* pSource;
				int	 pitch;
				std::tie(pSource,pitch) = _defaultSizes(pItems, pWeightSpx);

				for (int i = 0; i < nItems; i++)
				{

					totalLength += *pSource;
					pOutput[i] = *pSource;
					pSource = (spx*)(((char*)pSource) + pitch);
				}
				break;
			}
			case WantedSize::MaxDefault:
			{
				const spx* pSource;
				int	 pitch;
				std::tie(pSource, pitch) = _defaultSizes(pItems, pWeightSpx);

				spx max = 0;

				for (int i = 0; i < nItems; i++)
				{
					if (*pSource > max)
						max = *pSource;
					pSource = (spx*)(((char*)pSource) + pitch);
				}

				for (int i = 0; i < nItems; i++)
					pOutput[i] = max;

				totalLength = max * nItems;
				break;
			}
			case WantedSize::DefaultTimesWeight:
			{
				const spx* pSource;
				int	 pitch;
				std::tie(pSource, pitch) = _defaultSizes(pItems, pWeightSpx);

				for (int i = 0; i < nItems; i++)
				{
					spx val = Util::align(spx((int64_t(*pSource) * pItems[i].weight) / 65536));
					totalLength += val;
					pOutput[i] = val;
					pSource = (spx*)(((char*)pSource) + pitch);
				}
				break;
			}
			case WantedSize::MaxDefaultTimesWeight:
			{
				const spx* pSource;
				int	 pitch;
				std::tie(pSource, pitch) = _defaultSizes(pItems, pWeightSpx);
 
				spx max = 0;

				for (int i = 0; i < nItems; i++)
				{
					if (*pSource > max)
						max = *pSource;
					pSource = (spx*)(((char*)pSource) + pitch);
				}

				for (int i = 0; i < nItems; i++)
				{
					spx x = Util::align(spx((int64_t(max) * pItems[i].weight) / 65536));
					totalLength += x;
					pOutput[i] = x;
					pSource = (spx*)(((char*)pSource) + pitch);
				}

				break;
			}

			default:			// Should never get here.
				break;
			}

		}


		// Get our min/max sizes.

		const spx* pMin;
		int	 pitchMin;
		std::tie(pMin, pitchMin) = _minSizes(pItems, pWeightSpx);

		const spx* pMax;
		int	 pitchMax;
		std::tie(pMax, pitchMax) = _maxSizes(pItems, pWeightSpx);

		// Limit the start sizes by min and max values

		//TODO: Optimize by skipping when not needed (like when all sources are set to default and func is Source or MaxSource).

		const spx* pMin2 = pMin;
		const spx* pMax2 = pMax;

		for (int i = 0; i < nItems; i++)
		{
			if (pOutput[i] < *pMin2)
			{
				totalLength += *pMin2 - pOutput[i];
				pOutput[i] = *pMin2;
			}

			if (pOutput[i] > *pMax2)
			{
				totalLength -= pOutput[i] - *pMax2;
				pOutput[i] = *pMax2;
			}

			pMin2 = (spx*)(((char*)pMin2) + pitchMin);
			pMax2 = (spx*)(((char*)pMax2) + pitchMax);
		}

		// Figure out which items to hide and hide them.

		if (totalLength > availableSpace)
		{
			// Hide items if we are in restained hide mode and they won't fit even at minSize.

			if (m_hideSetting == HideSetting::Restrained)
			{
				// Calculate total min size

				const spx* pMin2 = pMin;
				spx minTotal = 0;
				for (int i = 0; i < nItems; i++)
				{
					minTotal += *pMin2;
					pMin2 = (spx*)(((char*)pMin) + pitchMin);
				}

				// Remove until minTotal <= availableSpace

				int nRemoved = 0;
				while (minTotal > availableSpace && nRemoved < m_shrinkOrder.size())
				{
					int i = m_shrinkOrder[nRemoved++];
					totalLength -= pOutput[i];
					minTotal -= (spx) * (((char*)pMin) + pitchMin * i);
					pOutput[i] = -1;
				}
			}
			else if (m_hideSetting == HideSetting::Aggressive)
			{					
				// Get our item factors

				int allocBytes = sizeof(int64_t) * nItems;
				auto pFactors = (uint64_t*)Base::memStackAlloc(allocBytes);
				uint64_t factorTotal = _getItemFactors(pFactors, m_shrinkFactor, nItems, pItems, pOutput, pWeightSpx);

				// Loop through our items and calculate the sizes they must have to fit available space.
				// Check which ones goes below their minSize and find the one that first goes below its
				// minSize and remove that one.
				// Repeat until no item has to shrink below its minSize for us to fit available space.

				while (totalLength > availableSpace)
				{
					int itemToRemove = -1;
					float removalFactor = 2;		// 

					uint64_t acc = 0;
					const spx* pMin2 = pMin;

					for (int i = 0; i < nItems; i++)
					{
						uint64_t factor = pFactors[i];

						if (factor > 0)
						{
							uint64_t valToSub = factor * (totalLength - availableSpace) + acc;

							acc = valToSub % factorTotal;
							spx spxToSub = (spx)(valToSub / factorTotal);

							if (pOutput[i] - spxToSub < *pMin2)
							{
								float newFactor = (pOutput[i] - *pMin2) / (float)spxToSub;
								if (newFactor < removalFactor)
								{
									itemToRemove = i;
									removalFactor = newFactor;
								}
							}
						}
					}

					if (itemToRemove >= 0)
					{
						totalLength -= pOutput[itemToRemove];
						pOutput[itemToRemove] = -1;
						factorTotal -= pFactors[itemToRemove];
						pFactors[itemToRemove] = 0;
					}
					else
						break;
				}

				// Clean up

				Base::memStackFree(allocBytes);
			}
		}

		// Use shrink function if we got too long.

		if (totalLength > availableSpace && m_shrinkFactor != Factor::Zero)
		{
			spx excessLength = totalLength - availableSpace;

			switch (m_shrinkFactor)
			{
				case Factor::Ordered:
				{

					break;
				}

				default:
				{
					// Get our item factors

					int allocBytes = sizeof(int64_t) * nItems;
					auto pFactors = (uint64_t*)Base::memStackAlloc(allocBytes);
					uint64_t factorTotal = _getItemFactors(pFactors, m_shrinkFactor, nItems, pItems, pOutput, pWeightSpx);

	
					// Allocate spaceLeft in possibly several passes until all has been allocated

					while (factorTotal > 0.f && excessLength > 0)
					{
						uint64_t acc = 0;
						const spx* pMin2 = pMin;

						uint64_t currentTotal = factorTotal;
						spx currentExcessLength = excessLength;

						for (int i = 0; i < nItems; i++)
						{
							uint64_t factor = pFactors[i];

							if (factor > 0)
							{
								uint64_t valToSub = factor * currentExcessLength + acc;

								acc = valToSub % currentTotal;
								spx spxToSub = (spx)(valToSub / currentTotal);

								spx maxSub = pOutput[i] - *pMin2;
								if (spxToSub > maxSub)
								{
									spxToSub = maxSub;
									factorTotal -= factor;
									pFactors[i] = 0;
								}
								pOutput[i] -= spxToSub;
								excessLength -= spxToSub;
							}

							pMin2 = (spx*)(((char*)pMin2) + pitchMin);
						}
					}

					// Clean up

					Base::memStackFree(allocBytes);
					break;
				}
			}

			totalLength = availableSpace + excessLength;
		}

		// Use expand function if we have space left.

		if (totalLength < availableSpace && m_expandFactor != Factor::Zero )
		{
			spx spaceLeft = availableSpace - totalLength;

			switch (m_expandFactor)
			{
				case Factor::Ordered:
				{
					for (int i = 0; i < m_expandOrder.size(); i++)
					{
						int index = m_expandOrder[i];

						spx itemMax = * (spx*)(((char*)pMax) + pitchMax*index);
						spx itemAdd = itemMax - pOutput[index];

						if (itemAdd > 0)
						{
							itemAdd = std::max(itemAdd, spaceLeft);
							pOutput[index] += itemAdd;
							spaceLeft -= itemAdd;
							if (spaceLeft == 0)
								break;
						}
					}
					break;
				}
/*
				case Factor::One:
				{
					// Check how many items can still expand

					const spx* pMax2 = pMax;
					int nExpandable = 0;
					for (int i = 0; i < nItems; i++)
					{
						if (pOutput[i] < *pMax2)
							nExpandable++;
						pMax2 = (spx*)(((char*)pMax2) + pitchMax);

					}

					// Allocate spaceLeft in possibly several passes until all has been allocated.

					while (nExpandable > 0 && spaceLeft > 0)
					{
						pMax2 = pMax;

						int64_t		spaceToAddPerPiece = int64_t(spaceLeft) * 65536 * 65536 / nExpandable + 1;
						int64_t		spaceToAddAcc = spaceToAddPerPiece;

						// Loop through all items and expand those that can be expanded

						for (int i = 0; i < nItems; i++)
						{
							spx itemLen = pOutput[i];
							spx itemMax = *pMax2;
							if (itemLen < itemMax)
							{
								int64_t addToThisItem = spaceToAddAcc & 0xFFFFFFC000000000;
								spaceToAddAcc += spaceToAddPerPiece - addToThisItem;
								int add32 = addToThisItem >> 32;
								if (itemLen + add32 >= itemMax)				// We use >= since we need to decrease nExpandable.
								{
									add32 = itemMax - itemLen;
									nExpandable--;
								}

								pOutput[i] += add32;
								spaceLeft -= add32;
							}
							pMax2 = (spx*)(((char*)pMax2) + pitchMax);
						}
					}

					break;
				}
*/

				default:
				{
					// Get our item factors

					int allocBytes = sizeof(int64_t) * nItems;
					auto pFactors = (uint64_t*) Base::memStackAlloc(allocBytes);
					uint64_t factorTotal = _getItemFactors(pFactors, m_expandFactor, nItems, pItems, pOutput, pWeightSpx);

					// Allocate spaceLeft in possibly several passes until all has been allocated


					while(factorTotal > 0.f && spaceLeft > 0)
					{
						uint64_t	acc = 0;
						const spx*	pMax2 = pMax;

						uint64_t	currentTotal = factorTotal;
						spx			currentSpaceLeft = spaceLeft;

						for (int i = 0; i < nItems; i++)
						{
							uint64_t factor = pFactors[i];

							if (factor > 0)
							{
								uint64_t valToAdd = factor * currentSpaceLeft + acc;

								acc = valToAdd % currentTotal;
								spx spxToAdd = (spx)(valToAdd / currentTotal);

								spx maxAdd = *pMax2 - pOutput[i];
								if (spxToAdd > maxAdd)
								{
									spxToAdd = maxAdd;
									factorTotal -= factor;
									pFactors[i] = 0;
								}
								pOutput[i] += spxToAdd;
								spaceLeft -= spxToAdd;
							}

							pMax2 = (spx*)(((char*)pMax2) + pitchMax);
						}
					}

					// Clean up

					Base::memStackFree(allocBytes);
					break;
				}

			}

			totalLength = availableSpace - spaceLeft;
		}


		// Align our output

		{
			spx acc = 32;
			for (int i = 0; i < nItems; i++)
			{
				acc += pOutput[i];
				spx aligned = acc & 0xFFFFFFC0;
				acc -= aligned;
				pOutput[i] = aligned;
			}
		}

		// Cleanup

		if (spxWeightAllocAmount > 0)
			Base::memStackFree(spxWeightAllocAmount);

		return totalLength;
	}

	//____ getWantedSizes() _________________________________________________

	spx PackLayout::getWantedSizes(spx* pOutput, int scale, int nItems, const Item* pItems)
	{
		// This can be optimized when using lists as source and function Source or MaxSource
		// by generating a vector of return values for each number of items.

		if (scale != m_cacheScale)
			_updateSpxCache(scale);

		spx result = 0;
		int allocAmount = 0;

		const spx* pSource;
		int	 pitch;

		// Get size source

		switch (m_defaultSizeSource)
		{
			default:
			case SizeSource::Weight:
			{
				allocAmount = sizeof(spx) * nItems;
				auto pWeightSpx = (spx*)Base::memStackAlloc(allocAmount);

				for (int i = 0; i < nItems; i++)
					pWeightSpx[i] = Util::align(spx(((pItems[i].weight>>8) * scale)>>8));

				pSource = pWeightSpx;
				pitch = sizeof(spx);
				break;
			}

			case SizeSource::Item:
				pSource = &pItems->def;
				pitch = sizeof(Item);
				break;

			case SizeSource::List1:
				pSource =  m_list1_spxCache.data();
				pitch = sizeof(spx);
				break;

			case SizeSource::List2:
				pSource = m_list2_spxCache.data();
				pitch = sizeof(spx);
				break;

			case SizeSource::List3:
				pSource = m_list3_spxCache.data();
				pitch = sizeof(spx);
				break;
		}

		// Calculate wanted length

		switch (m_wantedSize)
		{
			case WantedSize::Default:
			{
				for (int i = 0; i < nItems; i++)
				{
					pOutput[i] = *pSource;
					result += *pSource;
					pSource = (spx*)(((char*)pSource) + pitch);
				}
				break;
			}
			case WantedSize::MaxDefault:
			{
				spx max = 0;

				for (int i = 0; i < nItems; i++)
				{
					if( *pSource > max )
						max = *pSource;
					pSource = (spx*)(((char*)pSource) + pitch);
				}

				for (int i = 0; i < nItems; i++)
					pOutput[i] = max;

				result = max * nItems;
				break;
			}
			case WantedSize::DefaultTimesWeight:
				for (int i = 0; i < nItems; i++)
				{
					spx x = Util::align(spx((int64_t(*pSource) * pItems[i].weight) / 65536));
					pOutput[i] = x;
					result += x; 
					pSource = (spx*)(((char*)pSource) + pitch);
				}
				break;
			case WantedSize::MaxDefaultTimesWeight:
			{
				spx max = 0;

				for (int i = 0; i < nItems; i++)
				{
					if (*pSource > max)
						max = *pSource;
					pSource = (spx*)(((char*)pSource) + pitch);
				}

				for (int i = 0; i < nItems; i++)
				{
					spx x = Util::align(spx((int64_t(max) * pItems[i].weight)/65536));
					pOutput[i] = x;
					result += x;
					pSource = (spx*)(((char*)pSource) + pitch);
				}

				break;
			}

			default:			// Should never get here.
				break;
		}

		if( allocAmount > 0 )
			Base::memStackFree(allocAmount);

		return result;
	}

	//____ doesCalcWantedLength() _____________________________________________

	bool PackLayout::doesCalcWantedLength()
	{
		return m_bCalcWantedLength;
	}

	//____ _defaultSizes() ______________________________________________________

	std::tuple<const spx*, int> PackLayout::_defaultSizes( const Item* pItems, const spx * pWeightSPX )
	{
		switch (m_defaultSizeSource)
		{
			case SizeSource::Item:
				return std::make_tuple(&pItems->def, (int) sizeof(Item));

			case SizeSource::Weight:
				return std::make_tuple(pWeightSPX, (int) sizeof(spx));

			case SizeSource::List1:
				return std::make_tuple(m_list1_spxCache.data(), (int) sizeof(spx));

			case SizeSource::List2:
				return std::make_tuple(m_list2_spxCache.data(), (int) sizeof(spx));

			case SizeSource::List3:
			default:
				return std::make_tuple(m_list3_spxCache.data(), (int) sizeof(spx));
		}
	}

	//____ _minSizes() ______________________________________________________

	std::tuple<const spx*, int> PackLayout::_minSizes( const Item* pItems, const spx* pWeightSPX )
	{
		switch (m_minSizeSource)
		{
		case SizeSource::Item:
			return std::make_tuple(&pItems->min, (int) sizeof(Item));

		case SizeSource::Weight:
			return std::make_tuple(pWeightSPX, (int) sizeof(spx));

		case SizeSource::List1:
			return std::make_tuple(m_list1_spxCache.data(), (int) sizeof(spx));

		case SizeSource::List2:
			return std::make_tuple(m_list2_spxCache.data(), (int) sizeof(spx));

		case SizeSource::List3:
		default:
			return std::make_tuple(m_list3_spxCache.data(), (int) sizeof(spx));
		}
	}

	//____ _maxSizes() ______________________________________________________

	std::tuple<const spx*, int> PackLayout::_maxSizes( const Item* pItems, const spx* pWeightSPX)
	{
		switch (m_maxSizeSource)
		{
		case SizeSource::Item:
			return std::make_tuple(&pItems->max, (int) sizeof(Item));

		case SizeSource::Weight:
			return std::make_tuple(pWeightSPX, (int) sizeof(spx));

		case SizeSource::List1:
			return std::make_tuple(m_list1_spxCache.data(), (int) sizeof(spx));

		case SizeSource::List2:
			return std::make_tuple(m_list2_spxCache.data(), (int) sizeof(spx));

		case SizeSource::List3:
		default:
			return std::make_tuple(m_list3_spxCache.data(), (int) sizeof(spx));
		}
	}

	//____ _updateSpxCache() __________________________________________________

	void PackLayout::_updateSpxCache(int scale)
	{
		m_cacheScale = scale;

		if (!m_list1_spxCache.empty())
			for (int i = 0; i < m_list1.size(); i++)
				m_list1_spxCache[i] = align( spx( int64_t(m_list1[i]) * scale / 65536 ) );

		if (!m_list2_spxCache.empty())
			for (int i = 0; i < m_list2.size(); i++)
				m_list2_spxCache[i] = align( spx( int64_t(m_list2[i]) * scale / 65536) );

		if (!m_list3_spxCache.empty())
			for (int i = 0; i < m_list3.size(); i++)
				m_list1_spxCache[i] = align( spx( int64_t(m_list3[i]) * scale / 65536) );
	}

	//____ _convertList() _____________________________________________________

	void PackLayout::_convertList(std::vector<int>& to, const std::vector<float>& from)
	{
		to.resize(from.size());

		for (int i = 0; i < from.size(); i++)
			to[i] = int(from[i] * 65536);
	}

	//____ _getItemFactors()___________________________________________________

	uint64_t PackLayout::_getItemFactors(uint64_t* pOutput, Factor factor, int nItems, const Item* pItems, const spx * pSizes, const spx * pWeightSpx )
	{
		switch (factor)
		{
			case Factor::Zero:
			{
				for (int i = 0; i < nItems; i++)
					pOutput[i] = 0;
				return 0;
			}

			case Factor::One:
			{
				for (int i = 0; i < nItems; i++)
					pOutput[i] = 1;
				return nItems;
			}

			case Factor::Margin:
			{

				const spx* pMin;
				int	 minPitch;
				std::tie(pMin, minPitch) = _minSizes(pItems, pWeightSpx);

				int total = 0;
				for (int i = 0; i < nItems; i++)
				{
					int margin = pSizes[i] - * pMin;
					pOutput[i] = margin;
					total += margin;
					pMin = (spx*)(((char*)pMin) + minPitch);
				}
				return total;
			}

			case Factor::Size:
			{
				int total = 0;
				for (int i = 0; i < nItems; i++)
				{
					int size = pSizes[i];
					pOutput[i] = size;
					total += size;
				}
				return total;
			}

			case Factor::Weight:
			{
				int total = 0;
				for (int i = 0; i < nItems; i++)
				{
					int size = pItems[i].weight;
					pOutput[i] = size;
					total += size;
				}
				return total;
			}

			case Factor::InverseWeight:
			{
				int largestWeight = 0;
				for (int i = 0; i < nItems; i++)
					largestWeight = std::max(largestWeight, pItems[i].weight);


				int total = 0;
				for (int i = 0; i < nItems; i++)
				{
					int size = largestWeight - pItems[i].weight;
					pOutput[i] = size;
					total += size;
				}
				return total;
			}

			case Factor::SizeTimesWeight:
			{
				int total = 0;
				for (int i = 0; i < nItems; i++)
				{
					int size = int((int64_t(pSizes[i]) * pItems[i].weight) / 65536);
					pOutput[i] = size;
					total += size;
				}
				return total;
			}

			case Factor::SizeTimesInverseWeight:
			{
				int largestWeight = 0;
				for (int i = 0; i < nItems; i++)
					largestWeight = std::max(largestWeight, pItems[i].weight);

				int total = 0;
				for (int i = 0; i < nItems; i++)
				{
					int size = int((int64_t(pSizes[i]) * (largestWeight - pItems[i].weight)) / 65536);
					pOutput[i] = size;
					total += size;
				}
				return total;
			}

			case Factor::MarginTimesWeight:
			{
				const spx* pMin;
				int	 minPitch;
				std::tie(pMin, minPitch) = _minSizes(pItems, pWeightSpx);

				int total = 0;
				for (int i = 0; i < nItems; i++)
				{
					int margin = int((int64_t(pSizes[i] - *pMin) * pItems[i].weight) / 65536);
					pOutput[i] = margin;
					total += margin;
					pMin = (spx*)(((char*)pMin) + minPitch);
				}
				return total;
			}

			case Factor::MarginTimesInverseWeight:
			{
				int largestWeight = 0;
				for (int i = 0; i < nItems; i++)
					largestWeight = std::max(largestWeight, pItems[i].weight);

				const spx* pMin;
				int	 minPitch;
				std::tie(pMin, minPitch) = _minSizes(pItems, pWeightSpx);

				int total = 0;
				for (int i = 0; i < nItems; i++)
				{
					int margin = int((int64_t(pSizes[i] - * pMin) * (largestWeight - pItems[i].weight)) / 65536);
					pOutput[i] = margin;
					total += margin;
					pMin = (spx*)(((char*)pMin) + minPitch);
				}
				return total;
			}

			case Factor::List1:
			{
				int total = 0;
				for (int i = 0; i < nItems; i++)
				{
					int size = m_list1[i];
					pOutput[i] = size;
					total += size;
				}
				return total;
			}

			case Factor::List2:
			{
				int total = 0;
				for (int i = 0; i < nItems; i++)
				{
					int size = m_list2[i];
					pOutput[i] = size;
					total += size;
				}
				return total;
			}

			case Factor::List3:
			{
				int total = 0;
				for (int i = 0; i < nItems; i++)
				{
					int size = m_list3[i];
					pOutput[i] = size;
					total += size;
				}
				return total;
			}
			default:
				return 0; // Should never get here!
		}
	}



} // namespace wg
