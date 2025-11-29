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
#include <wg2_sizebroker.h>




WgSizeBroker::WgSizeBroker()
{
}

WgSizeBroker::~WgSizeBroker()
{
}

/*
int WgSizeBroker::SetItemLengths( WgSizeBrokerItem * pItems, int nItems, int _totalLength ) const
{
	int totalLength = 0;
	for( int i = 0 ; i < nItems ; i++ )
	{
		totalLength += pItems[i].preferred;
		pItems[i].output = pItems[i].preferred;
	}
	return totalLength;
}

*/

int WgUniformSizeBroker::SetItemLengths( WgSizeBrokerItem * pItems, int nItems, int totalLength ) const
{
	if( nItems == 0 )
		return 0;

	// Gather some data we need

	int staticLength = 0;               // Total preferred length of all static (non-stretching) items
	int nUnifiedItems = 0;                // Number of static items
	float extraWeight = 0;                // Total "extra weight", of all items with weight > 1.

	for( int i = 0 ; i < nItems ; i++ )
	{
		if( pItems[i].weight == 0.f )
			staticLength += pItems[i].preferred;
		else if( pItems[i].weight <= 1.f )
			nUnifiedItems++;
		else
			extraWeight += pItems[i].weight-1.f;
	}

	// Calculate baseLength and extraLengthPerWeightUnit

	int unifiedLength = nUnifiedItems > 0 ? (totalLength - staticLength) / nUnifiedItems : 0;
	int paddingLength = totalLength - staticLength - unifiedLength*nUnifiedItems;
	float paddingPerWeightUnit = extraWeight > 0 ? paddingLength / extraWeight : 0.f;

	if(unifiedLength < 0)
		unifiedLength = 0;

	if(paddingPerWeightUnit < 0.0f)
		paddingPerWeightUnit = 0.0f;

	 // Loop through items and set their length

	float paddingAcc = 0.0001f;
	int total = 0;
	int length;

	for( int i = 0 ; i < nItems ; i++ )
	{
		if( pItems[i].weight == 0 )
		{
			length = pItems[i].preferred;
		}
		else if( pItems[i].weight > 1 )
		{
			paddingAcc += (pItems[i].weight-1) * paddingPerWeightUnit;
			length = (int) paddingAcc;
			paddingAcc -= (int) paddingAcc;
		}
		else
			length = unifiedLength;

		pItems[i].output = length;
		total += length;
	}
	return total;
}



int WgUniformSizeBroker::_findLongestUnified( WgSizeBrokerItem * pItems, int nItems ) const
{
	int longest = 0;
	for( int i = 0 ; i < nItems ; i++ )
		if( pItems[i].preferred > longest && pItems[i].weight == 1.f )
			longest = pItems[i].preferred;
	return longest;
}

int WgOSDTrackMetersSizeBroker::SetItemLengths( WgSizeBrokerItem * pItems, int nItems, int totalLength ) const
{
	if( nItems == 0 )
		return 0;

	// Gather some data we need

	int nNullWeight = 0;               // Number of items with 0.f weight
	float totalWeight = 0;             // Total weight of all items

	for( int i = 0 ; i < nItems ; i++ )
	{
		totalWeight += pItems[i].weight;
		if( pItems[i].weight == 0.f )
			nNullWeight++;
	}

	//

	int totalUsed = 0;

	for( int i = 0 ; i < nItems ; i++ )
	{
		if( pItems[i].weight != 0 )
		{
			int length = pItems[i].weight / totalWeight * totalLength;
			if( length == 0 )
			{
				pItems[i].output = 1;
				totalWeight -= pItems[i].weight;
				totalUsed += 1;
			}
		}
	}

	// Loop through items and set their length

	int lengthLeft = totalLength - totalUsed;

	for( int i = 0 ; i < nItems ; i++ )
	{
		if( pItems[i].weight != 0 )
		{
			int length = pItems[i].weight / totalWeight * lengthLeft;
			if( length != 0 )
			{
				pItems[i].output = length;
				totalUsed += length;
			}
		}
	}

	//

	if (nNullWeight > 0) {
		int leftOver = totalLength - totalUsed;

		int lengthPerNullItem = leftOver/nNullWeight;

		for( int i = 0 ; i < nItems ; i++ )
		{
			if( pItems[i].weight == 0 )
			{
				pItems[i].output = lengthPerNullItem;
				totalUsed += lengthPerNullItem;
			}
		}
	}

	return totalUsed;
}

int WgOSDFaderTrackSizeBroker::SetItemLengths(WgSizeBrokerItem *pItems, int nItems, int totalLength) const
{
    if (nItems == 0)
        return 0;

    // Gather some data we need
     // Total preferred length of all static (non-stretching) items
    float totalWeight = 0; // Total weight of all items.

    for (int i = 0; i < nItems; i++)
    {
            totalWeight += pItems[i].weight;
    }

    // Calculate baseLength and extraLengthPerWeightUnit

    float lengthPerWeightUnit = totalWeight > 0 ? (totalLength) / totalWeight : 0;

    if (lengthPerWeightUnit < 0.f)
        lengthPerWeightUnit = 0;

    // Loop through items and set their length

    int total = 0;
    int length;

    for (int i = 0; i < nItems; i++)
    {
        if (pItems[i].weight <= 0)
            length = pItems[i].preferred;
        else
            length = pItems[i].weight * lengthPerWeightUnit;

        pItems[i].output = length;
        total += length;
    }
    return total;
}

int WgScalePreferredSizeBroker::SetItemLengths( WgSizeBrokerItem * pItems, int nItems, int totalLength ) const
{
	int total = 0;
	for( int i = 0 ; i < nItems ; i++ )
	{
		int def = (int) (pItems[i].preferred * pItems[i].weight);
		pItems[i].output = def;
		total += def;
	}
	return total;
}

int WgScaleWeightSizeBroker::SetItemLengths(WgSizeBrokerItem * pItems, int nItems, int totalLength) const
{
	if (nItems == 0)
		return 0;

	// Gather some data we need

	int staticLength = 0;               // Total preferred length of all static (non-stretching) items
	float totalWeight = 0;                // Total weight of all items.

	for (int i = 0; i < nItems; i++)
	{
		if (pItems[i].weight <= 0.f)
			staticLength += pItems[i].preferred;
		else
			totalWeight += pItems[i].weight;
	}

	// Calculate baseLength and extraLengthPerWeightUnit

	float lengthPerWeightUnit = totalWeight > 0 ? (totalLength - staticLength) / totalWeight : 0;

	if (lengthPerWeightUnit < 0.f)
		lengthPerWeightUnit = 0;


	// Loop through items and set their length

	int total = 0;
	int length;

	for (int i = 0; i < nItems; i++)
	{
		if (pItems[i].weight <= 0)
			length = pItems[i].preferred;
		else
			length = pItems[i].weight * lengthPerWeightUnit;

		pItems[i].output = length;
		total += length;
	}
	return total;
}

float WgScaleWeightSizeBroker::_findNeededLengthPerWeight(WgSizeBrokerItem * pItems, int nItems) const
{
	float largest = 0;
	for (int i = 0; i < nItems; i++)
	{
		if (pItems[i].weight > 0)
		{
			int unifiedLengthNeeded = pItems[i].preferred / pItems[i].weight;
			if (unifiedLengthNeeded  > largest)
				largest = unifiedLengthNeeded;
		}
	}
	return largest;
}
