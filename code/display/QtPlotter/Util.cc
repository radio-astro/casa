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
#include <display/QtPlotter/Util.h>
#include <assert.h>
#include <cmath>
#include <iostream>
#include <QMessageBox>
#include <QWidget>

namespace casa {

	const QString Util::ORGANIZATION = "NRAO/CASA";
	const QString Util::APPLICATION = "Spectral Profiler";

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

	void Util::showUserMessage( QString& msg, QWidget* parent ){
		QMessageBox msgBox( parent );
		msgBox.setText( msg );
		msgBox.exec();
	}
}
