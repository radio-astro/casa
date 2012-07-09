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

class QWidget;

namespace casa { //# NAMESPACE CASA - BEGIN
	class Util {
		public:
			static const QString ORGANIZATION;
			static const QString APPLICATION;
			static void showUserMessage( QString& msg, QWidget* parent);
			static double degMinSecToRadians( int degrees, int mins, float secs );
			static double hrMinSecToRadians( int hours, int mins, float secs );
			static QString toHTML( const QString& baseStr );
		private:
			Util();
			virtual ~Util();
			static double toRadians( double degrees );
			static double toDecimalDegrees( int hrs, int mins, float seconds );
			static const double PI;
			static const double TIME_CONV;

	};
}
#endif /* UTIL_H_ */
