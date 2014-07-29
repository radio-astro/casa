//# Copyright (C) 2005
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#

#include "ColorHistogramScale.qo.h"
#include <display/Display/ColormapDefinition.h>
#include <QDebug>
#include <QtCore/qmath.h>

namespace casa {

	ColorHistogramScale::ColorHistogramScale(bool invert, int logScale ):QwtLinearColorMap(),
		colorDefinition(NULL) {
		invertMap = invert;
		this->logScale = logScale;
	}

	String ColorHistogramScale::getColorMapName() const {
		return mapName;
	}


	void ColorHistogramScale::setColorMapName( const String& colorMapName ) {
		mapName = colorMapName;
		colorDefinition = new ColormapDefinition( colorMapName );
		const int COLOR_COUNT = 100;

		float divisor = 1;
		if ( logScale > 0 ){
			divisor = (COLOR_COUNT*1.0f) / logScale;
		}
		double maxValue = (COLOR_COUNT - 1) / divisor;
		double MAX_POWER = qPow(2, maxValue);
		for ( int i = 0; i < COLOR_COUNT; i++ ) {
			float percent = (i*1.0f) / COLOR_COUNT;
			float colorIndex = percent;

			if ( logScale > 0 ){
				double power = (i*1.0)/divisor;
				double value = qPow(2, power);
				percent = value / MAX_POWER;
			}

			Float red = 0;
			Float green = 0;
			Float blue = 0;
			colorDefinition->getValue( percent, red, green, blue );
			const int COLOR_MAX = 255;
			int greenValue = static_cast<int>( green * COLOR_MAX );
			int redValue = static_cast<int>( red * COLOR_MAX );
			int blueValue = static_cast<int>( blue * COLOR_MAX );
			if ( invertMap ) {
				greenValue = COLOR_MAX - greenValue;
				redValue   = COLOR_MAX - redValue;
				blueValue  = COLOR_MAX - blueValue;
			}

			QColor colorStop( redValue, greenValue, blueValue );
			addColorStop( colorIndex, colorStop );
		}
	}


	ColorHistogramScale::~ColorHistogramScale() {
		delete colorDefinition;
	}

} /* namespace casa */
