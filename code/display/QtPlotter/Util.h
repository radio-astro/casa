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
#ifndef UTIL_H_
#define UTIL_H_

#include <QString>
#include <casa/Arrays/Vector.h>
#include <display/Display/DisplayCoordinateSystem.h>
#include <imageanalysis/ImageAnalysis/PixelValueManipulatorData.h>
#include <imageanalysis/ImageAnalysis/ImageCollapserData.h>

class QWidget;

namespace casacore{

	template <class T> class ImageInterface;
	class ImageRegion;
}

namespace casa { //# NAMESPACE CASA - BEGIN


	class Util {


	public:
		static const QString ORGANIZATION;
		static const QString APPLICATION;
		static void showUserMessage( QString& msg, QWidget* parent);
		static double degMinSecToRadians( int degrees, int mins, float secs );
		static double hrMinSecToRadians( int hours, int mins, float secs );
		static QString toHTML( const QString& baseStr );
		static int getCenter( const casacore::Vector<casacore::Double>& values, casacore::Double& mean );
		static void getRa(double radians, int& raHour, int& raMin, double& raSec);
		static void getDec(double radians, int& decDeg, int& decMin, double& decSec);
		static void minMax( double& min, double& max, const casacore::Vector<casacore::Double>& values );
		static QString toDegreeString( int hrs, int mins, double secs );
		static QString toDecString( int hrs, int mins, double secs );
		static void appendSign( double rad, QString& degreeString );
		static QList<QString> getTitleCaseVariations( QString source );
		static QString toTitleCase( QString word );
		static casacore::String mainImageName( casacore::String path );

		/**
		 * Designed to take an input string of the format '[km/s]' and return 'km/s'.
		 */
		static QString stripBrackets( QString unitStr );

		/**
		 * Designed to take an input string of the format <font color='black'>km/s</font>'
		 * and return 'km/s'.
		 */
		static QString stripFont( QString unitStr );

		//Support for tabular axes that are frequency.  Returns -1 if there
		//is no tabular index in the image in frequency units.
		static int getTabularFrequencyAxisIndex(SHARED_PTR<const casacore::ImageInterface<casacore::Float> > img);

		static casacore::Record getRegionRecord( casacore::String shape, const DisplayCoordinateSystem& cSys,
					const casacore::Vector<casacore::Double>& x, const casacore::Vector<casacore::Double>& y);

		static std::pair<casacore::Vector<casacore::Float>,casacore::Vector<casacore::Float> > getProfile(SHARED_PTR<const casacore::ImageInterface<casacore::Float> >& imagePtr,
				const casacore::Vector<casacore::Double>& x, const casacore::Vector<casacore::Double>& y, const casacore::String& shape,
				int tabularAxis, ImageCollapserData::AggregateType, casacore::String unit,
				const casacore::String& coordinateType,
				const casacore::Quantity *const restFreq=0, const casacore::String& frame="");
		/**
		 * Returns the record of a 3-dimension region with the base in the shape of
		 * an ellipse and height given by the channel range.
		 * @param cSys the image coordinate system.
		 * @param x the x-coordinates of the ellipse bounding box.
		 * @param y the y-coordinates of the ellipse bounding box.
		 * @return a casacore::Record describing cylindrical volume with a elliptical base.
		 */
		static casacore::Record getEllipticalRegion3D( const DisplayCoordinateSystem& cSys,
						const casacore::Vector<casacore::Double>& x, const casacore::Vector<casacore::Double>& y,
						int channelMin, int channelMax, int spectralAxisNumber);

		/**
		 * Returns the record of a 3-dimension region with the base in the shape of
		 * a polygon and height given by the channel range.
		 * @param cSys the image coordinate system.
		 * @param x the x-coordinates of the polygon corner points.
		 * @param y the y-coordinates of the polygon corner points.
		 * @return a casacore::Record describing cylindrical volume with a polygonal base.
		 */

		static casacore::Record getPolygonalRegion3D( const DisplayCoordinateSystem& cSys,
						const casacore::Vector<casacore::Double>& x, const casacore::Vector<casacore::Double>& y,
						int channelMin, int channelMax, int spectralAxisNumber);

		/**
		 * Return a 2D region in the shape of an ellipse.
		 * @param cSys the image coordinate system.
		 * @param x the x-coordinates of the ellipse bounding box.
		 * @param y the y-coordinates of the ellipse bounding box.
		 * @return the 2D image region of the ellipse.
		 */
		static casacore::ImageRegion* getEllipsoid(const DisplayCoordinateSystem& cSys,
							const casacore::Vector<casacore::Double>& x, const casacore::Vector<casacore::Double>& y);
		/**
		 * Return a 2D region in the shape of a polygon.
		 * @param cSys the image coordinate system.
		 * @param x the x-coordinates of the polygon corner points.
		 * @param y the y-coordinates of the polygon corner points.
		 * @return the 2D image region of the polygon.
		 */
		static casacore::ImageRegion* getPolygon(const DisplayCoordinateSystem& cSys,
					const casacore::Vector<casacore::Double>& x, const casacore::Vector<casacore::Double>& y);

		/**
		 * Tests whether the contents of the two arrays are the same.
		 * Returns true if the arrays have the same size and contents; false otherwise.
		 */
		static bool arrayEquals( const casacore::Vector<casacore::Double>& a, const casacore::Vector<casacore::Double>& b );
	private:
		Util();
		virtual ~Util();
		static double toRadians( double degrees );
		static double toDecimalDegrees( int hrs, int mins, float seconds );
		static casacore::Record make3DRegion( const DisplayCoordinateSystem& cSys, casacore::ImageRegion* shape3D,
					int channelMin, int channelMax, int spectralAxisNumber );

		static const double PI;
		static const double TIME_CONV;
		static const double RAD_DEGREE_CONVERSION;

	};
}
#endif /* UTIL_H_ */
