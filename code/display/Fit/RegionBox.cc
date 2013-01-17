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

#include "RegionBox.h"
#include <assert.h>
#include <QDebug>

namespace casa {

RegionBox::RegionBox(const QList<int> &pixel_x, const QList<int> &pixel_y) {

	initBox( pixel_x, pixel_y );
}

void RegionBox::update( const QList<int> & pixelX, const QList<int> & pixelY ){
	clear();
	initBox( pixelX, pixelY );
}

void RegionBox::initBox( const QList<int> & pixelX, const QList<int>& pixelY ){
	regionBoxBLC.append(pixelX[0]);
	regionBoxBLC.append(pixelY[0]);
	regionBoxTRC.append(pixelX[1]);
	regionBoxTRC.append(pixelY[1]);
}

void RegionBox::clear(){
	regionBoxBLC.clear();
	regionBoxTRC.clear();
}

bool RegionBox::isInBox( double valueX, double valueY ) const {
	bool valueInBox = false;
	if ( regionBoxBLC[0]<= valueX && valueX <= regionBoxTRC[0] ){
		if ( regionBoxBLC[1] <= valueY && valueY <= regionBoxTRC[1] ){
			valueInBox = true;
		}
	}
	return valueInBox;
}

QString RegionBox::toString() const {
	QString pixelStr( "");
	const QString COMMA_STR( ",");
	pixelStr.append( QString::number(regionBoxBLC[0]) + COMMA_STR );
	pixelStr.append( QString::number(regionBoxBLC[1]) + COMMA_STR );

	pixelStr.append( QString::number(regionBoxTRC[0]) + COMMA_STR );
	pixelStr.append( QString::number(regionBoxTRC[1]));
	return pixelStr;
}



QString RegionBox::toStringLabelled() const {
	QString pixelStr( "");
	const QString COMMA_STR( ",");
	pixelStr.append( "Bottom Lower Corner=" );
	pixelStr.append( "("+QString::number(regionBoxBLC[0]) + COMMA_STR );
	pixelStr.append( QString::number(regionBoxBLC[1]) + "),  " );

	pixelStr.append( "Top Right Corner=" );
	pixelStr.append( "(" + QString::number(regionBoxTRC[0]) + COMMA_STR );
	pixelStr.append( QString::number(regionBoxTRC[1]) + ")");
	return pixelStr;
}

RegionBox::~RegionBox() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
