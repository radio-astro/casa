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
#include <tr1/memory>

class QWidget;

namespace casa { //# NAMESPACE CASA - BEGIN

	template <class T> class ImageInterface;

	class Util {


	public:
		static const QString ORGANIZATION;
		static const QString APPLICATION;
		static void showUserMessage( QString& msg, QWidget* parent);
		static double degMinSecToRadians( int degrees, int mins, float secs );
		static double hrMinSecToRadians( int hours, int mins, float secs );
		static QString toHTML( const QString& baseStr );
		static int getCenter( const Vector<Double>& values, Double& mean );
		static void getRa(double radians, int& raHour, int& raMin, double& raSec);
		static void getDec(double radians, int& decDeg, int& decMin, double& decSec);
		static void minMax( double& min, double& max, const Vector<Double>& values );
		static QString toDegreeString( int hrs, int mins, double secs );
		static QString toDecString( int hrs, int mins, double secs );
		static void appendSign( double rad, QString& degreeString );
		static QList<QString> getTitleCaseVariations( QString source );
		static QString toTitleCase( QString word );
		static String mainImageName( String path );

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
		static int getTabularFrequencyAxisIndex(std::tr1::shared_ptr<const ImageInterface<Float> > img);


	private:
		Util();
		virtual ~Util();
		static double toRadians( double degrees );
		static double toDecimalDegrees( int hrs, int mins, float seconds );
		static const double PI;
		static const double TIME_CONV;
		static const double RAD_DEGREE_CONVERSION;

	};
}
#endif /* UTIL_H_ */
