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

#include <imageanalysis/Annotations/RegionTextList.h>

#include <casa/OS/File.h>
#include <imageanalysis/Annotations/AnnRegion.h>
#include <images/Regions/WCDifference.h>

namespace casa {


RegionTextList::RegionTextList(
	const Bool deletePointersOnDestruct
) : _lines(Vector<AsciiAnnotationFileLine>(0)),
	_deletePointersOnDestruct(deletePointersOnDestruct),
	_csys(CoordinateSystem()), _shape(IPosition()),
	_canGetRegion(False) {}


RegionTextList::RegionTextList(
	const CoordinateSystem& csys,
	const IPosition shape,
	const Bool deletePointersOnDestruct
) : _lines(Vector<AsciiAnnotationFileLine>(0)),
	_deletePointersOnDestruct(deletePointersOnDestruct),
	_csys(csys),_shape(shape), _canGetRegion(True) {}

RegionTextList::RegionTextList(
	const String& filename, const CoordinateSystem& csys,
	const IPosition shape,
	const Int requireAtLeastThisVersion,
	const Bool deletePointersOnDestruct
) : _lines(Vector<AsciiAnnotationFileLine>(0)),
_deletePointersOnDestruct(deletePointersOnDestruct),
	_csys(csys), _shape(shape), _canGetRegion(True) {
	RegionTextParser parser(filename, csys, shape, requireAtLeastThisVersion);
	Vector<AsciiAnnotationFileLine> lines = parser.getLines();
	for (
		Vector<AsciiAnnotationFileLine>::const_iterator iter=lines.begin();
		iter != lines.end(); iter++
	) {
		addLine(*iter);
	}
}

RegionTextList::RegionTextList(
	const CoordinateSystem& csys, const String& text,
	const IPosition shape,
	const Bool deletePointersOnDestruct
) : _lines(Vector<AsciiAnnotationFileLine>(0)),
_deletePointersOnDestruct(deletePointersOnDestruct),
	_csys(csys), _shape(shape), _canGetRegion(True) {
	RegionTextParser parser(csys, shape, text);
	Vector<AsciiAnnotationFileLine> lines = parser.getLines();
	for (
		Vector<AsciiAnnotationFileLine>::const_iterator iter=lines.begin();
		iter != lines.end(); iter++
	) {
		addLine(*iter);
	}
}

RegionTextList::~RegionTextList() {
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

void RegionTextList::addLine(const AsciiAnnotationFileLine& line) {
	AsciiAnnotationFileLine x = line;
	_lines.resize(_lines.size()+1, True);
	_lines[_lines.size()-1] = x;
	if (x.getType() == AsciiAnnotationFileLine::ANNOTATION && _canGetRegion) {
		const AnnotationBase * const annotation = x.getAnnotationBase();
		if (annotation->isRegion()) {
			const AnnRegion *region = dynamic_cast<const AnnRegion *>(x.getAnnotationBase());
			if (! region->isAnnotationOnly()) {
				_regions.resize(_regions.size() + 1);

				WCRegion *wcregion = region->getRegion();
				if (region->isDifference()) {
					if (_regions.size() == 1) {
						Vector<Double> blc, trc;
						_csys.toWorld(blc, IPosition(_csys.nPixelAxes(), 0));
						_csys.toWorld(trc, _shape);
						Vector<Quantity> qblc(blc.size()), qtrc(trc.size());
						Vector<String> wUnits = _csys.worldAxisUnits();
						Vector<Int> absRel(blc.size(), RegionType::Abs);
						for (uInt i=0; i<qblc.size(); i++) {
							qblc[i] = Quantity(blc[i], wUnits[i]);
							qtrc[i] = Quantity(blc[i], wUnits[i]);
						}
						WCBox x;
						_regions[0] = new WCBox(qblc, qtrc, _csys, absRel);
					}
					else {
						_regions[_regions.size() - 1] = new WCDifference(
							*_regions[_regions.size() - 2],
							*wcregion
						);
					}
				}
				else {
					if (_regions.size() == 1) {
						_regions[0] = wcregion;
					}
					else {
						_regions[_regions.size() - 1] = new WCUnion(
							*_regions[_regions.size() - 2],
							*wcregion
						);
					}
				}
			}
		}
	}
}

Record RegionTextList::regionAsRecord() const {
	if (_regions.size() == 0) {
		throw AipsError("No regions found");
	}
	return getRegion()->toRecord("");
}

WCRegion* RegionTextList::getRegion() const {
	if (! _canGetRegion) {
		throw AipsError(
			"Object constructed with too little information for forming composite region. Use another constructor."
		);
	}
	if (_regions.size() == 0) {
		return 0;
	}
	return _regions[_regions.size() - 1];
}

uInt RegionTextList::nLines() const {
	return _lines.size();
}

AsciiAnnotationFileLine RegionTextList::lineAt(
	const uInt i
) const {
	if (i >= _lines.size()) {
		throw AipsError("Index out of range");
	}
	return _lines[i];
}

ostream& RegionTextList::print(ostream& os) const {
	String vString = String::toString(RegionTextParser::CURRENT_VERSION);
	os << "#CRTFv" + vString
		<< " CASA Region Text Format version "
		<< vString << endl;
	for (
		Vector<AsciiAnnotationFileLine>::const_iterator iter=_lines.begin();
		iter != _lines.end(); iter++
	) {
		if (
			iter == _lines.begin()
			&& iter->getType() == AsciiAnnotationFileLine::COMMENT
			&& iter->getComment().contains(
				Regex(RegionTextParser::MAGIC.regexp() + "v[0-9]+")
			)
		) {
			// skip writing header line if it already exists, we write
			// our own here to avoid clashes with previous spec versions
			continue;
		}
		os << *iter << endl;
	}
	return os;
}


}
