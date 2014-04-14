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

#ifndef COLORHISTOGRAMSCALE_H_
#define COLORHISTOGRAMSCALE_H_

#include <QVector>
#include <qwt_color_map.h>
#include <qwt_double_interval.h>
#include <casa/BasicSL/String.h>

namespace casa {

	class ColormapDefinition;

	/**
	 * Overrides QwtLinearColorMap in order to provide QwtPlot color
	 * lookups based on a casa ColormapDefinition.
	 */

	class ColorHistogramScale : public QwtLinearColorMap {
	public:
		ColorHistogramScale( bool invert, int logarithmic );
		String getColorMapName() const;
		void setColorMapName( const String& colorMapName );

		virtual ~ColorHistogramScale();
	private:
		ColorHistogramScale( ColorHistogramScale& other );
		ColorHistogramScale operator=(ColorHistogramScale& other);
		String mapName;
		bool invertMap;
		int logScale;
		ColormapDefinition* colorDefinition;
	};

} /* namespace casa */
#endif /* COLORHISTOGRAMSCALE_H_ */
