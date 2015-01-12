//# Copyright (C) 2000,2001
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

#include <lattices/Lattices/LatticeStatsDataProviderBase.h>

#include <lattices/Lattices/LatticeProgress.h>

namespace casa {

template <class AccumType, class T>
LatticeStatsDataProviderBase<AccumType, T>::LatticeStatsDataProviderBase()
: _hasRanges(False), _isInclude(True), _ranges(),
  _progressMeter(NULL), _minPos(), _maxPos() {}

template <class AccumType, class T>
LatticeStatsDataProviderBase<AccumType, T>::~LatticeStatsDataProviderBase() {}

template <class AccumType, class T>
uInt LatticeStatsDataProviderBase<AccumType, T>::getMaskStride() {
	return 1;
}

template <class AccumType, class T>
void LatticeStatsDataProviderBase<AccumType, T>::finalize() {}

template <class AccumType, class T>
DataRanges LatticeStatsDataProviderBase<AccumType, T>::getRanges() {
	return _ranges;
}

template <class AccumType, class T>
uInt LatticeStatsDataProviderBase<AccumType, T>::getStride() {
	return 1;
}

template <class AccumType, class T>
const T* LatticeStatsDataProviderBase<AccumType, T>::getWeights() {
	return NULL;
}

template <class AccumType, class T>
Bool LatticeStatsDataProviderBase<AccumType, T>::hasRanges() const {
	return _hasRanges;
}

template <class AccumType, class T>
Bool LatticeStatsDataProviderBase<AccumType, T>::hasWeights() const {
	return False;
}

template <class AccumType, class T>
Bool LatticeStatsDataProviderBase<AccumType, T>::isInclude() const {
	return _isInclude;
}

template <class AccumType, class T>
void LatticeStatsDataProviderBase<AccumType, T>::minMaxPos(
	IPosition& minPos, IPosition& maxPos) const {
	minPos = _minPos;
	maxPos = _maxPos;
}

template <class AccumType, class T>
void LatticeStatsDataProviderBase<AccumType, T>::setProgressMeter(
	CountedPtr<LattStatsProgress> pm
) {
	_progressMeter = pm;
}

template <class AccumType, class T>
void LatticeStatsDataProviderBase<AccumType, T>::setRanges(
	const DataRanges& ranges, Bool isInclude
) {
	_hasRanges = ! ranges.empty();
	_ranges = ranges;
	_isInclude = isInclude;
}

template <class AccumType, class T>
void LatticeStatsDataProviderBase<AccumType, T>::_updateProgress() {
	if (! _progressMeter.null()) {
		(*_progressMeter)++;
	}
}

}

