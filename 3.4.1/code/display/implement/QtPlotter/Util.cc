/*
 * Util.cpp
 *
 *  Created on: Apr 12, 2012
 *      Author: slovelan
 */

#include <display/QtPlotter/Util.h>
#include <assert.h>
#include <cmath>
#include <iostream>
using namespace std;
namespace casa {

	Util::Util() {
	// TODO Auto-generated constructor stub
	}

	Util::~Util() {
	// TODO Auto-generated destructor stub
	}

	const double Util::PI = std::atan(1.0) * 4.0;
	const double Util::TIME_CONV = 60.0;

	double Util::degMinSecToRadians( int degrees, int mins, float secs ){
		assert( -90 <= degrees && degrees <=90 );
		assert( 0 <= mins && mins < TIME_CONV );
		assert( 0 <= secs && secs < TIME_CONV );
        double decimalDegrees = toDecimalDegrees( degrees, mins, secs );
		return toRadians( decimalDegrees );
	}

	double Util::hrMinSecToRadians( int hours, int mins, float secs ){
		assert( 0 <= hours && hours <= 24 );
		assert( 0 <= mins && mins < TIME_CONV );
		assert( 0 <= secs && secs < TIME_CONV );

		double decimalHours = toDecimalDegrees( hours, mins, secs );
		double degrees = decimalHours * 15;
		return toRadians( degrees );
	}

	QString Util::toHTML( const QString& baseStr ){
		QString htmlStr( "<font color='black'>");
		htmlStr.append( baseStr );
		htmlStr.append( "</font>");
		return htmlStr;
	}

	double Util::toRadians( double degrees ){
		double rads = degrees *  PI / 180;
		return rads;
	}

	double Util::toDecimalDegrees( int degrees, int mins, float seconds ){
		double deg = degrees + mins / TIME_CONV + seconds / (TIME_CONV * TIME_CONV );
		return deg;
	}
}

int main(){
	cout << "Hi" << endl;
}
