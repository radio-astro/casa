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

	void RegionBox::update( const QList<int> & pixelX, const QList<int> & pixelY ) {
		clear();
		initBox( pixelX, pixelY );
	}

	void RegionBox::initBox( const QList<int> & pixelX, const QList<int>& pixelY ) {
		regionBoxBLC.append(pixelX[0]);
		regionBoxBLC.append(pixelY[0]);
		regionBoxTRC.append(pixelX[1]);
		regionBoxTRC.append(pixelY[1]);
	}

	void RegionBox::clear() {
		regionBoxBLC.clear();
		regionBoxTRC.clear();
	}

	bool RegionBox::isInBox( double valueX, double valueY ) const {
		bool valueInBox = false;
		if ( regionBoxBLC[0]<= valueX && valueX <= regionBoxTRC[0] ) {
			if ( regionBoxBLC[1] <= valueY && valueY <= regionBoxTRC[1] ) {
				valueInBox = true;
			}
		}
		return valueInBox;
	}

	QVector<int> RegionBox::getActualBLC( const Vector<int>& imageBLC, const Vector<int>& imageTRC, bool* valid ) const {
		//For the bottom left, we take the maximum of the image and region bottom left.
		QVector<int> actualBLC(2);
		actualBLC[0] = qMax( imageBLC[0], regionBoxBLC[0] );
		actualBLC[1] = qMax( imageBLC[1], regionBoxBLC[1] );
		*valid = false;
		if ( isInImage( actualBLC, imageBLC, imageTRC)) {
			*valid = true;
		}
		return actualBLC;
	}

	bool RegionBox::isInImage( const QVector<int>& coords, const Vector<int>& imageBLC,
	                           const Vector<int>& imageTRC ) const {
		bool interior = false;
		if ( imageBLC[0]<= coords[0] && coords[0]<=imageTRC[0]) {
			if ( imageBLC[1]<= coords[1] && coords[1]<=imageTRC[1]) {
				interior = true;
			}
		}
		return interior;
	}

	QVector<int> RegionBox::getActualTRC( const Vector<int>& imageBLC, const Vector<int>& imageTRC, bool* valid ) const {
		//For the top right, we take the minimum of the image and region trc.
		QVector<int> actualTRC(2);
		*valid = false;
		if ( imageTRC.size() >= 2 && regionBoxTRC.size() >= 2 ){
			actualTRC[0] = qMin( imageTRC[0], regionBoxTRC[0] );
			actualTRC[1] = qMin( imageTRC[1], regionBoxTRC[1] );
			if ( isInImage( actualTRC, imageBLC, imageTRC ) ) {
				*valid = true;
			}
		}
		return actualTRC;
	}


	QString RegionBox::toString(const Vector<int>& imageBLC, const Vector<int>& imageTRC ) const {
		QString pixelStr( "");
		const QString COMMA_STR( ",");
		bool valid = false;
		QVector<int> actualTRC = getActualTRC( imageBLC, imageTRC, &valid );
		if ( valid ) {
			QVector<int> actualBLC = getActualBLC( imageBLC, imageTRC, &valid );
			if ( valid ) {
				pixelStr.append( QString::number(actualBLC[0]) + COMMA_STR );
				pixelStr.append( QString::number(actualBLC[1]) + COMMA_STR );

				pixelStr.append( QString::number(actualTRC[0]) + COMMA_STR );
				pixelStr.append( QString::number(actualTRC[1]));
			}
		}
		return pixelStr;
	}



	QString RegionBox::toStringLabelled(const Vector<int>& imageBLC, const Vector<int>& imageTRC) const {
		QString pixelStr( "");
		const QString COMMA_STR( ",");
		bool valid = false;
		QVector<int> actualTRC = getActualTRC( imageBLC, imageTRC, &valid );
		if ( valid ) {
			QVector<int> actualBLC = getActualBLC( imageBLC, imageTRC, &valid );
			if ( valid ) {
				pixelStr.append( "Bottom Lower Corner=" );
				pixelStr.append( "("+QString::number(actualBLC[0]) + COMMA_STR );
				pixelStr.append( QString::number(actualBLC[1]) + "),  " );

				pixelStr.append( "Top Right Corner=" );
				pixelStr.append( "(" + QString::number(actualTRC[0]) + COMMA_STR );
				pixelStr.append( QString::number(actualTRC[1]) + ")");
			}
		}
		return pixelStr;
	}

	RegionBox::~RegionBox() {
		// TODO Auto-generated destructor stub
	}

} /* namespace casa */
