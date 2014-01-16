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

#ifndef QTPLOTSETTINGS_H
#define QTPLOTSETTINGS_H

#include <cmath>
#include <QString>
#include <utility>		/***for std::pair***/

using namespace std;

namespace casa {

	class QtPlotSettings {
	public:
		QtPlotSettings();
		void adjust( const QString& topUnits, const QString& bottomUnits,
				bool autoScaleX, bool autoScaleY, bool zoom = false );
		void zoomOut( double zoomFactor, const QString& topUnits, const QString& bottomUnits,
		              bool autoScaleX, bool autoScaleY );
		void zoomIn( double zoomFactor, const QString& topUnits, const QString& bottomUnits,
		             bool autoScaleX, bool autoScaleY );
		void zoomY( double minY, double maxY, bool autoScaleY );
		pair<double,double> getZoomInY( double zoomFactor ) const;
		pair<double,double> getZoomOutY( double zoomFactor ) const;

		void scroll(int dx, int dy);
		enum AxisIndex {xBottom, xTop, END_AXIS_INDEX };
		double spanX( AxisIndex index ) const {
			return maxX[static_cast<int>(index)] - minX[static_cast<int>(index)];
		}
		double spanY() const {
			return maxY - minY;
		}
		double getMinX( AxisIndex index ) const {
			return minX[static_cast<int>(index)];
		}
		void setMinX( AxisIndex index, double value );
		double getMaxX( AxisIndex index ) const {
			return maxX[static_cast<int>(index)];
		}
		void setMaxX( AxisIndex index, double value );
		double getMinY() const {
			return minY;
		}

		void setMinY( double value );
		double getMaxY() const {
			return maxY;
		}

		void setMaxY( double value );
		int getNumTicksX( ) const {
			return numXTicks;
		}
		int getNumTicksY() const {
			return numYTicks;
		}



	private:

		pair<double,double> adjustAxis(double &min, double &max, int &numTicks);
		void adjustAxisTop( double &min, double &max);
		double minX[END_AXIS_INDEX];
		double maxX[END_AXIS_INDEX];
		int numXTicks;
		double minY;
		double maxY;
		double minPercentage;
		double maxPercentage;
		//double originalMinX;
		//double originalMaxX;
		int numYTicks;
	};


}
#endif

