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


#include "FeatheredData.h"

namespace casa {

FeatheredData::FeatheredData() {
	// TODO Auto-generated constructor stub

}

bool FeatheredData::isEmpty() const {
	bool empty = true;
	if ( ux.size() > 0 || uy.size() > 0 || vx.size() > 0 || vy.size() > 0 ){
		empty = false;
	}
	return empty;
}

void FeatheredData::setU( const Vector<Float>& xVal, const Vector<Float>& yVal ){
	int xCount = xVal.size();
	ux.resize( xCount );
	for ( int i = 0; i < xCount; i++ ){
		ux[i] = xVal[i];
	}
	int yCount = yVal.size();
	uy.resize( yCount );
	for ( int i = 0; i < yCount; i++ ){
		uy[i] = yVal[i];
	}
}

void FeatheredData::setV( const Vector<Float>& xVal, const Vector<Float>& yVal ){
	int xCount = xVal.size();
	vx.resize( xCount );
	for ( int i = 0; i < xCount; i++ ){
		vx[i] = xVal[i];
	}
	int yCount = yVal.size();
	vy.resize( yCount );
	for ( int i = 0; i < yCount; i++ ){
		vy[i] = yVal[i];
	}
}

Vector<Float> FeatheredData::getUX() const {
	return ux;
}

Vector<Float> FeatheredData::getUY() const {
	return uy;
}

Vector<Float> FeatheredData::getVX() const {
	return vx;
}

Vector<Float> FeatheredData::getVY() const {
	return vy;
}

FeatheredData::~FeatheredData() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
