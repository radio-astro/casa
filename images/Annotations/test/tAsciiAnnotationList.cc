//# Copyright (C) 2000,2001,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#

#include <casa/aips.h>
#include <images/Annotations/AsciiAnnotationList.h>
#include <images/Annotations/AnnRegion.h>

#include <coordinates/Coordinates/CoordinateUtil.h>

#include <casa/namespace.h>

#include <iomanip>

int main () {
	LogIO log;
	try {
		CoordinateSystem csys = CoordinateUtil::defaultCoords4D();
		AnnotationBase::unitInit();
		AsciiAnnotationList list(
			"fixtures/goodAsciiRegionFile.txt", csys,
			IPosition(0, csys.nPixelAxes())
		);
		cout << std::setprecision(9) << list << endl;
		AlwaysAssert(list.nLines() == 31, AipsError);
		AsciiAnnotationFileLine line30 = list.lineAt(30);
		AlwaysAssert(
			line30.getType() == AsciiAnnotationFileLine::ANNOTATION,
			AipsError
		);
		AlwaysAssert(
			line30.getAnnotationBase()->getLineWidth() == 9,
			AipsError
		);
		AlwaysAssert(
			line30.getAnnotationBase()->isRegion(),
			AipsError
		);
		AlwaysAssert(dynamic_cast<const AnnRegion *>(
			line30.getAnnotationBase())->isAnnotationOnly(),
			AipsError
		);

		AsciiAnnotationFileLine line23 = list.lineAt(22);
		AlwaysAssert(
			line23.getType() == AsciiAnnotationFileLine::ANNOTATION,
			AipsError
		);
		AlwaysAssert(
			line23.getAnnotationBase()->getLineWidth() == 22,
			AipsError
		);
		AlwaysAssert(
			! line23.getAnnotationBase()->isRegion(),
			AipsError
		);
		Quantity k(4.1122334455667778, "km");
		cout << std::setprecision(10) << k << endl;

	} catch (AipsError x) {
		log << LogIO::SEVERE
			<< "Caught exception: " << x.getMesg()
			<< LogIO::POST;
		return 1;
	}

	cout << "OK" << endl;
	return 0;
}
