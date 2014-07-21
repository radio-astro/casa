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

#ifndef IMAGEANALYSIS_IMAGEREGRIDDERBASE_H
#define IMAGEANALYSIS_IMAGEREGRIDDERBASE_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>
//#include <casa/Arrays/IPosition.h>
#include <scimath/Mathematics/Interpolate2D.h>
#include <casa/namespace.h>

namespace casa {

template <class T> class ImageRegridderBase : public ImageTask<T> {
	// <summary>
	// Data store of ImageRegridder and ComplexImageRegridder
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Data for image regridder.
	// </etymology>

	// <synopsis>
	// </synopsis>

	template<typename U> friend class ImageRegridderBase;

public:

	// destructor
	~ImageRegridderBase();

	// regrid the spectral axis in velocity space rather than frequency space?
	void setSpecAsVelocity(Bool v) { _specAsVelocity = v; }

	// Set interpolation method.
	void setMethod(const String& method) { _method = Interpolate2D::stringToMethod(method); }
	void setMethod(Interpolate2D::Method method) { _method = method; }

	void setDoRefChange(Bool d) { _doRefChange = d; }

	void setReplicate(Bool r) { _replicate = r; }

	// throws exception if 3*decimate > length of an axis that will be regridded
	void setDecimate(Int d);

	void setForceRegrid(Bool f) { _forceRegrid = f; }

	void setShape(const IPosition s) { _shape = s; }

	virtual SPIIT regrid() const = 0;

	template <class U> void setConfiguration(const ImageRegridderBase<U>& that);

protected:

	ImageRegridderBase(
		const SPCIIT image, const Record *const regionRec,
		const String& maskInp, const String& outname, Bool overwrite,
		const CoordinateSystem& csys, const IPosition& axes,
		const IPosition& shape
	);

	Interpolate2D::Method _getMethod() const { return _method; }

	Bool _getDoRefChange() const { return _doRefChange; }

	Bool _getReplicate() const { return _replicate; }

	Int _getDecimate() const { return _decimate;}

	Bool _getForceRegrid() const { return _forceRegrid; }

	inline  CasacRegionManager::StokesControl _getStokesControl() const {
		return CasacRegionManager::USE_ALL_STOKES;
	}

	inline vector<Coordinate::Type> _getNecessaryCoordinates() const {
		return vector<Coordinate::Type>(0);
	}

	Bool _getSpecAsVelocity() const { return _specAsVelocity; }


	IPosition _getShape() const {return _shape;}

	const CoordinateSystem& _getTemplateCoords() const { return _csysTo; }

	IPosition _getAxes() const { return _axes; }

	IPosition _getKludgedShape() const { return _kludgedShape; }

	vector<String> _getOutputStokes() const { return _outputStokes; }

	uInt _getNReplicatedChans() const { return _nReplicatedChans; }

	Bool _regriddingDirectionAxes() const;


private:
	const CoordinateSystem _csysTo;
	IPosition _axes, _shape, _kludgedShape;
	Bool _specAsVelocity, _doRefChange, _replicate, _forceRegrid;
	Int _decimate;
	Interpolate2D::Method _method;
	vector<String> _outputStokes;
	uInt _nReplicatedChans;

	void _finishConstruction();

	ImageRegridderBase() {};

};
}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <imageanalysis/ImageAnalysis/ImageRegridderBase.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC

#endif
