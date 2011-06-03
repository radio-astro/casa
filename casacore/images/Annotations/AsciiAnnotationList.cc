//# AsciiRegionFile.cc
//# Copyright (C) 1998,1999,2000,2001
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

#include <images/Annotations/AsciiAnnotationList.h>

#include <casa/OS/File.h>
#include <images/IO/AsciiAnnotationFileParser.h>

namespace casa {

AsciiAnnotationList::AsciiAnnotationList(
	const Bool deletePointersOnDestruct
) : _lines(Vector<AsciiAnnotationFileLine>(0)),
_deletePointersOnDestruct(deletePointersOnDestruct) {}

AsciiAnnotationList::AsciiAnnotationList(
	const String& filename, const CoordinateSystem& csys,
	const Bool deletePointersOnDestruct
) : _lines(Vector<AsciiAnnotationFileLine>(0)),
_deletePointersOnDestruct(deletePointersOnDestruct) {
	AsciiAnnotationFileParser parser(filename, csys);
	_lines = parser.getLines();
}

AsciiAnnotationList::~AsciiAnnotationList() {
	if (_deletePointersOnDestruct) {
		for (
			Vector<AsciiAnnotationFileLine>::const_iterator iter = _lines.begin();
			iter != _lines.end(); iter++
		) {
			if (iter->getType() == AsciiAnnotationFileLine::ANNOTATION) {
				delete (iter->getAnnotationBase());
			}
		}
	}
}

void AsciiAnnotationList::addLine(const AsciiAnnotationFileLine& line) {
	AsciiAnnotationFileLine x = line;
	_lines.resize(_lines.size()+1, True);
	_lines[_lines.size()-1] = x;
}

uInt AsciiAnnotationList::nLines() const {
	return _lines.size();
}

ostream& AsciiAnnotationList::print(ostream& os) const {
	for (
		Vector<AsciiAnnotationFileLine>::const_iterator iter=_lines.begin();
		iter != _lines.end(); iter++
	) {
		os << *iter << endl;
	}
	return os;
}


}
