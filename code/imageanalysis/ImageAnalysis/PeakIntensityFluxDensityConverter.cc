//# tSubImage.cc: Test program for class SubImage
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

#include <imageanalysis/ImageAnalysis/PeakIntensityFluxDensityConverter.h>

#include <components/ComponentModels/SkyCompRep.h>
#include <images/Images/ImageUtilities.h>

namespace casa {

PeakIntensityFluxDensityConverter::PeakIntensityFluxDensityConverter(
		const ImageTask::shCImFloat  image
) : ImageTask(
		image, "", 0, "", "", "", "", "", ""
	), _size(Angular2DGaussian::NULL_BEAM), _shape(ComponentType::GAUSSIAN),
	_beam(GaussianBeam::NULL_BEAM) {
	_construct(False);
}

PeakIntensityFluxDensityConverter::~PeakIntensityFluxDensityConverter() {}

void PeakIntensityFluxDensityConverter::setBeam(Int channel, Int polarization) {
	_beam = _getImage()->imageInfo().restoringBeam(channel, polarization);
}

Quantity PeakIntensityFluxDensityConverter::fluxDensityToPeakIntensity(
	Bool& hadToMakeFakeBeam, const Quantity& fluxDensity
) const {

	hadToMakeFakeBeam = False;
	const CoordinateSystem& csys = _getImage()->coordinates();
	const Unit& brightnessUnit = _getImage()->units();
	GaussianBeam beam = _beam;
	if (brightnessUnit.getName().contains("/beam") && beam.isNull()) {
		beam = ImageUtilities::makeFakeBeam(
			*_getLog(), csys, _getVerbosity() >= ImageTask::WHISPER
		);
		hadToMakeFakeBeam = True;
	}
	return SkyCompRep::integralToPeakFlux(
		csys.directionCoordinate(),
		_shape, fluxDensity, brightnessUnit,
		_size.getMajor(), _size.getMinor(), beam
	);
}

Quantity PeakIntensityFluxDensityConverter::peakIntensityToFluxDensity(
	Bool& hadToMakeFakeBeam, const Quantity& peakIntensity
) const {

	hadToMakeFakeBeam = False;
	const Unit& brightnessUnit = _getImage()->units();
	const CoordinateSystem& csys = _getImage()->coordinates();
	GaussianBeam beam = _beam;
	if (brightnessUnit.getName().contains("/beam") && beam.isNull()) {
		beam = ImageUtilities::makeFakeBeam(
			*_getLog(), csys, _getVerbosity() >= ImageTask::WHISPER
		);
		hadToMakeFakeBeam = True;
	}
	return SkyCompRep::peakToIntegralFlux(
		csys.directionCoordinate(),
		_shape, peakIntensity,
		_size.getMajor(), _size.getMinor(), beam
	);
}
}
