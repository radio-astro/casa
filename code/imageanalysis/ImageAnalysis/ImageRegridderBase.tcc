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
//# $Id: $

#include <imageanalysis/ImageAnalysis/ImageRegridderBase.h>

namespace casa {

template <class T> ImageRegridderBase<T>::ImageRegridderBase(
	const SPCIIT image, const Record *const regionRec,
	const String& maskInp, const String& outname, Bool overwrite,
	const CoordinateSystem& csys, const IPosition& axes,
	const IPosition& shape
) : ImageTask<T>(
		image, "", regionRec, "", "", "", maskInp, outname, overwrite
	), _csysTo(csys), _axes(axes), _shape(shape),
	_specAsVelocity(False), _doRefChange(False),
	_replicate(False), _forceRegrid(False), _decimate(10),
	_method(Interpolate2D::LINEAR), _outputStokes(), _nReplicatedChans(0) {
	this->_construct();
	_finishConstruction();
}

template <class T> ImageRegridderBase<T>::~ImageRegridderBase() {}

template <class T> Bool ImageRegridderBase<T>::_regriddingDirectionAxes() const {
	Vector<Int> dirAxesNumbers = _csysTo.directionAxesNumbers();
	if (! dirAxesNumbers.empty()) {
		vector<Int> v = dirAxesNumbers.tovector();
		for (Int i=0; i<(Int)_axes.size(); i++) {
			if (std::find(v.begin(), v.end(), _axes[i]) != v.end()) {
				return True;
			}
		}
	}
	return False;
}

template <class T> void ImageRegridderBase<T>::setDecimate(Int d) {
	if (d > 1 && _regriddingDirectionAxes()) {
		Vector<Int> dirAxesNumbers = _csysTo.directionAxesNumbers();
		vector<Int> v = dirAxesNumbers.tovector();
		for (Int i=0; i<(Int)_axes.size(); i++) {
			Int axis = _axes[i];
			ThrowIf(
				(Int)_shape[axis] < 3*d
				&& std::find(v.begin(), v.end(), axis) != v.end(),
				"The output image has only "
				+ String::toString(_shape[axis])
				+ " pixels along axis " + String::toString(axis)
				+ ", so the maximum value of decimate should "
				"be " + String::toString(_shape[axis]/3)
			);
		}
	}
	_decimate = d;
}

template <class T> void ImageRegridderBase<T>::_finishConstruction() {
	Bool shapeSpecified = ! _shape.empty() && _shape[0] >= 0;
	if (! shapeSpecified) {
		IPosition imShape = this->_getImage()->shape();
		_shape.resize(imShape.size());
		_shape = imShape;
		if (this->_getDropDegen()) {
			IPosition tmp = _shape.nonDegenerate();
			_shape.resize(tmp.size());
			_shape = tmp;
		}
	}
	_kludgedShape = _shape;
	const CoordinateSystem csysFrom = this->_getImage()->coordinates();
	// enforce stokes rules CAS-4960
	if (csysFrom.hasPolarizationCoordinate() && _csysTo.hasPolarizationCoordinate()) {
		Vector<Int> templateStokes = _csysTo.stokesCoordinate().stokes();
		Vector<Int> inputStokes = csysFrom.stokesCoordinate().stokes();
		Int inputPolAxisNumber = csysFrom.polarizationAxisNumber();
		if (
			(
				_axes.empty()
				|| inputPolAxisNumber < (Int)_axes.size()
			) && templateStokes.size() > 1
		) {
			if (
				(
					_axes.empty() && inputStokes.size() > 1
				)
				|| _axes[inputPolAxisNumber] > 0
			) {
				StokesCoordinate stokesFrom = csysFrom.stokesCoordinate();
				StokesCoordinate stokesTo = _csysTo.stokesCoordinate();
				Stokes::StokesTypes valFrom, valTo;
				for (uInt i=0; i<inputStokes.size(); i++) {
					stokesFrom.toWorld(valFrom, i);
					for (uInt j=0; j<templateStokes.size(); j++) {
						stokesTo.toWorld(valTo, j);
						if (valFrom == valTo) {
							_outputStokes.push_back(Stokes::name(valFrom));
							break;
						}
					}
				}
				ThrowIf(
					_outputStokes.empty(),
					"Input image and template coordinate system have no common stokes."
				);
				ThrowIf(
					shapeSpecified && ((Int)_outputStokes.size() != _shape[inputPolAxisNumber]),
					"Specified output stokes axis length (" + String::toString(_shape[inputPolAxisNumber])
					+ ") does not match the number of common stokes ("
					+ String::toString(_outputStokes.size())
					+ ") in the input image and template coordinate system."
				);
				// This is a kludge to fool the underlying ImageRegrid constructor that the shape
				// is acceptable to it. We copy just the stokes we from the output of ImageRegrid.
				ImageMetaData md(this->_getImage());
				_kludgedShape[csysFrom.polarizationAxisNumber(False)] = md.nStokes();
			}
		}
	}
	Int spectralAxisNumber = csysFrom.spectralAxisNumber(False);
	if (
		csysFrom.hasSpectralAxis() && _csysTo.hasSpectralAxis()
		&& this->_getImage()->shape()[spectralAxisNumber] == 1
		&& ! _axes.empty()
	) {
		uInt count = 0;
		foreach_(Int axis, _axes) {
			if (axis == spectralAxisNumber) {
				*this->_getLog() << LogIO::NORMAL << "You've specified "
					<< "explicitly that the spectral axis should be "
					<< "regridded. However, the input image has a "
					<< "degenerate spectral axis and so it cannot be "
					<< "regridded. Instead, the resulting single output "
					<< "channel will be replicated " << _shape[axis]
					<< " times in the output image." << LogIO::POST;
				IPosition newAxes(_axes.size() - 1, 0);
				IPosition toRemove(1, count);
				newAxes = _axes.removeAxes(toRemove);
				_axes.resize(newAxes.size());
				_axes = newAxes;
				_nReplicatedChans = _shape[axis];
				_kludgedShape[axis] = 1;

				break;
			}
			count++;
		}
	}
}

template <class T> template <class U> void ImageRegridderBase<T>::setConfiguration(
	const ImageRegridderBase<U>& that
) {
	_method = that._method;
	_decimate = that._getDecimate();
	_replicate = that._getReplicate();
	_doRefChange = that._getDoRefChange();
	_forceRegrid = that._getForceRegrid();
	this->setStretch(that._getStretch());
	_specAsVelocity = that._specAsVelocity;
	this->setDropDegen(that._getDropDegen());
}

}
