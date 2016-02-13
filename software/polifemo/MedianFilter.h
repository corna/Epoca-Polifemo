/*
 * MedianFilter
 * Copyright (C) 2016 Nicola Corna <nicola@corna.info>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef MEDIANFILTER_H
#define MEDIANFILTER_H

#include <stdint.h>

#include "Filter.h"

template <typename T, uint8_t filterSize> class MedianFilter
{
protected:
  const T maxValue;
	T values[filterSize];
	uint8_t lastIndex;

public:
  //Arduino doesn't include numeric_limits<T>::max(), therefore
  //we have to use this hack
	inline MedianFilter (T maxValue, T startValue = T()) :
	    maxValue(maxValue), lastIndex(filterSize - 1)
	{
		for (uint8_t i = 0; i < filterSize; i++)
		    values[i] = startValue;
	}

	inline T filter (T value)
	{
		T valuesCopy[filterSize];
		uint8_t minValueIndex = 0;

		values[lastIndex] = value;
		lastIndex = (lastIndex + filterSize - 1) % filterSize;

		for(uint8_t i = 0; i < filterSize; i++)
		    valuesCopy[i] = values[i];

		for(uint8_t i = 0; i < filterSize / 2 + 1; i++)
		{
			for(uint8_t j = 0; j < filterSize; j++)
			{
				if(valuesCopy[j] < valuesCopy[minValueIndex])
					minValueIndex = j;
			}
    	valuesCopy[minValueIndex] = maxValue;
		}

		return values[minValueIndex];
	}
};

#endif
