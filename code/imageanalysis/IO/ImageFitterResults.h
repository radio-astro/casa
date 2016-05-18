//# Copyright (C) 1998,1999,2000,2001,2003
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

#ifndef IMAGEANALYSIS_IMAGEFITTERRESULTS_H
#define IMAGEANALYSIS_IMAGEFITTERRESULTS_H

#include <casa/Quanta/Quantum.h>
#include <components/ComponentModels/ComponentList.h>

#include <imageanalysis/ImageTypedefs.h>

namespace casa {

template<class T> class Vector;

class ImageFitterResults {
	// <summary>
    // Used exclusively by ImageFitter. Unless you are modifying that class,
    // you should have no reason to use this class.
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// </etymology>

	// <synopsis>
	// </synopsis>

	// <example>
	// <srcblock>
	// </srcblock>
	// </example>

public:

	enum CompListWriteControl {
		NO_WRITE,
		WRITE_NO_REPLACE,
		OVERWRITE
	};

	ImageFitterResults() = delete;

	ImageFitterResults(SPCIIF image, SHARED_PTR<LogIO> log);

	~ImageFitterResults();

	void setChannels(Vector<uInt> chans) { _channels = chans; }

	void setConvolvedList(const ComponentList& list) {
		_convolvedList = list;
	}

	void setDeconvolvedList(const ComponentList& list) {
	    _deconvolvedList = list;
	}

	void setPeakIntensities(const Vector<Quantity>& p) {
		_peakIntensities.assign(p);
	}

	void setPeakIntensityErrors(const Vector<Quantity>& m) {
		_peakIntensityErrors.assign(m);
	}
	void setMajorAxes(const Vector<Quantity>& m) {
		_majorAxes = m;
	}

	void setMinorAxes(const Vector<Quantity>& m) {
		_minorAxes = m;
	}

	void setPositionAngles(const Vector<Quantity>& m) {
		_positionAngles = m;
	}

	void setFluxDensities(const Vector<Quantity>& m) {
		_fluxDensities.assign(m);
	}

	void setFluxDensityErrors(const Vector<Quantity>& m) {
		_fluxDensityErrors.assign(m);
	}

	void writeNewEstimatesFile(const String& filename) const;

	void writeCompList(
		ComponentList& list, const String& compListName,
		CompListWriteControl writeControl
	) const;

	String resultsHeader(
		const String& chans, const Vector<uInt>& chanVec,
		const String& region, const String& mask,
		SHARED_PTR<std::pair<Float, Float> > includePixelRange,
		SHARED_PTR<std::pair<Float, Float> > excludePixelRange,
		const String& estimates
	) const;

	static vector<String> unitPrefixes(Bool includeC);

	void setStokes(const String& s) { _stokes = s; }

	String fluxToString(uInt compNumber, Bool hasBeam) const;

	void setFixed(const Vector<String>& s) { _fixed = s; }

	void writeSummaryFile(const String& filename, const CoordinateSystem& csys) const;

private:
	SPCIIF _image;
	SHARED_PTR<LogIO> _log;
	ComponentList _convolvedList{}, _deconvolvedList{};
	Vector<Quantity> _peakIntensities, _peakIntensityErrors,
		_majorAxes, _minorAxes,
		_positionAngles, _fluxDensities, _fluxDensityErrors;
	String _bUnit, _stokes;
	Vector<String> _fixed;
	Vector<uInt> _channels{};
	const static String _class;
	static std::vector<String> _prefixes, _prefixesWithCenti;
};
}

#endif
