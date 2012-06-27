/*
 * Util.h
 *
 *  Created on: Apr 12, 2012
 *      Author: slovelan
 */

#ifndef UTIL_H_
#define UTIL_H_

#include <QString>
namespace casa { //# NAMESPACE CASA - BEGIN
	class Util {
		public:
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
