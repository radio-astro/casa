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


#include <display/QtPlotter/QtDrawSettings.h>

namespace casa {

	QtDrawSettings::QtDrawSettings() {
		minX = 0.0;
		maxX = 10.0;
		numXTicks = 5;

		minY = 0.0;
		maxY = 10.0;
		numYTicks = 5;
	}

	void QtDrawSettings::scroll(int dx, int dy) {
		double stepX = spanX() / numXTicks;
		minX += dx * stepX;
		maxX += dx * stepX;

		double stepY = spanY() / numYTicks;
		minY += dy * stepY;
		maxY += dy * stepY;
	}

	void QtDrawSettings::adjust() {
		adjustAxis(minX, maxX, numXTicks);
		adjustAxis(minY, maxY, numYTicks);
	}

	void QtDrawSettings::adjustAxis(double &min, double &max,
	                                int &numTicks) {
		const int MinTicks = 4;
		double grossStep = (max - min) / MinTicks;
		double step = std::pow(10, floor(log10(grossStep)));

		if (5 * step < grossStep)
			step *= 5;
		else if (2 * step < grossStep)
			step *= 2;

		numTicks = (int)(ceil(max / step) - floor(min / step));
		min = floor(min / step) * step;
		max = ceil(max / step) * step;
	}



}

