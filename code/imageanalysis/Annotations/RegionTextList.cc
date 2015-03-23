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

RegionTextList::RegionTextList()
	: _lines(),
	_csys(CoordinateSystem()), _shape(IPosition()),
	_canGetRegion(False), _union(), _composite() {}


RegionTextList::RegionTextList(
	const CoordinateSystem& csys,
	const IPosition shape
) : _lines(),
	_csys(csys),_shape(shape), _canGetRegion(True), _union(), _composite() {}

RegionTextList::RegionTextList(
	const String& filename, const CoordinateSystem& csys,
	const IPosition shape,
	const Int requireAtLeastThisVersion
) : _lines(),
	_csys(csys), _shape(shape), _canGetRegion(True), _union(), _composite() {
	RegionTextParser parser(filename, csys, shape, requireAtLeastThisVersion);
	vector<AsciiAnnotationFileLine> lines = parser.getLines();
	vector<AsciiAnnotationFileLine>::const_iterator iter = lines.begin();
	vector<AsciiAnnotationFileLine>::const_iterator end = lines.end();
	while (iter != end) {
		addLine(*iter);
		++iter;
	}
}

RegionTextList::RegionTextList(
	const CoordinateSystem& csys, const String& text,
	const IPosition shape) : _lines(),
	_csys(csys), _shape(shape), _canGetRegion(True), _union(), _composite() {
	RegionTextParser parser(csys, shape, text);
	Vector<AsciiAnnotationFileLine> lines = parser.getLines();
	for (
		Vector<AsciiAnnotationFileLine>::const_iterator iter=lines.begin();
		iter != lines.end(); iter++
	) {
		addLine(*iter);
	}
}

RegionTextList::~RegionTextList() {}

void RegionTextList::addLine(const AsciiAnnotationFileLine& line) {
	AsciiAnnotationFileLine x = line;
	_lines.resize(_lines.size()+1, True);
	_lines[_lines.size()-1] = x;
	if (x.getType() == AsciiAnnotationFileLine::ANNOTATION && _canGetRegion) {
		CountedPtr<const AnnotationBase> annotation = x.getAnnotationBase();
		if (annotation->isRegion()) {
			const AnnRegion *region = dynamic_cast<const AnnRegion *>(annotation.get());
			if (! region->isAnnotationOnly()) {
				CountedPtr<const WCRegion> wcregion = region->getRegion();
				if (region->isDifference() && _regions.size() == 0) {
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
					_regions.push_back(new WCBox(qblc, qtrc, _csys, absRel));
					_union.push_back(True);
				}
				_regions.push_back(wcregion);
				_union.push_back(! region->isDifference());
				_composite = NULL;
			}
		}
	}
}

Record RegionTextList::regionAsRecord() const {
	ThrowIf(_regions.size() == 0, "No regions found");
	return getRegion()->toRecord("");
}

CountedPtr<const WCRegion> RegionTextList::getRegion() const {
	if (_composite) {
		return _composite;
	}
	ThrowIf(
		! _canGetRegion,
		"Object constructed with too little information "
		"for forming composite region. Use another constructor."
	);
	if (_regions.size() == 0) {
		return 0;
	}
	if (_regions.size() == 1) {
		_composite = _regions[0];
		return _composite;
	}
	vector<Bool>::const_iterator iter = _union.begin();
	vector<Bool>::const_iterator end = _union.end();
	vector<Bool>::const_iterator foundDifference = std::find(iter, end, False);
	PtrBlock<const WCRegion *> unionRegions;
	if (foundDifference == end) {
		// no complementary regions, just union the whole lot
		unionRegions.resize(_regions.size());
		for (uInt i=0; i<_regions.size(); ++i) {
			unionRegions[i] = _regions[i].get();
		}
		_composite = new WCUnion(False, unionRegions);
		return _composite;
	}
	uInt count = 0;
	while(iter != end) {
		if (*iter) {
			unionRegions.resize(unionRegions.size() + 1);
			unionRegions[_regions.size() - 1] = _regions[count].get();
		}
		else {
			WCUnion myUnion(False, unionRegions);
			const WCDifference *myDiff = new WCDifference(myUnion, *_regions[count]);
			_myDiff.push_back(CountedPtr<const WCDifference>(myDiff));
			unionRegions.resize(1);
			unionRegions[0] = myDiff;
		}
		++count;
		++iter;
	}
	_composite = (unionRegions.size() == 1)
		? unionRegions[0]
		: new WCUnion(False, unionRegions);
	return _composite;
}

uInt RegionTextList::nLines() const {
	return _lines.size();
}

AsciiAnnotationFileLine RegionTextList::lineAt(
	const uInt i
) const {
	ThrowIf(i >= _lines.size(), "Index out of range");
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
