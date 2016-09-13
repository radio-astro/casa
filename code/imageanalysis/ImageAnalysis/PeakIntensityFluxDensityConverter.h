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
//# $Id: tSubImage.cc 20567 2009-04-09 23:12:39Z gervandiepen $

#ifndef IMAGEANALYSIS_PEAKINTENSITYFLUXDENSITYCONVERTER_H
#define IMAGEANALYSIS_PEAKINTENSITYFLUXDENSITYCONVERTER_H

#include <imageanalysis/ImageAnalysis/ImageTask.h>

#include <components/ComponentModels/ComponentType.h>
#include <scimath/Mathematics/GaussianBeam.h>

#include <casa/namespace.h>

namespace casa {

class PeakIntensityFluxDensityConverter : public ImageTask<Float> {
	// <summary>
	// Class for converting between peak intensity and flux density of a source
	// of specified shape.
	// </summary>

	// <reviewed reviewer="" date="" tests="" demos="">
	// </reviewed>

	// <prerequisite>
	// </prerequisite>

	// <etymology>
	// Collapses image.
	// </etymology>

	// <synopsis>
	// </synopsis>

	// <example>
	// <srcblock>
	// </srcblock>
	// </example>

public:
    using Angular2DGaussian = GaussianBeam;

    // Specify image to get beam and coordinate info from
	PeakIntensityFluxDensityConverter(
		const SPCIIF image
	);

	// destructor
	~PeakIntensityFluxDensityConverter();

	// set the source size using a GaussianShape object. Note that this in itself,
	// does not constrain the shape to be Gaussian, it is just used to set the size
	// (and puts the burden on the caller to do sanity checking by first constructing
	// a GaussianSource object). The position angle is not used.
	void setSize(const Angular2DGaussian& size) { _size = size; }

	// set the component shape

	void setShape(const String& shape) { _shape = ComponentType::shape(shape); }

	void setShape (ComponentType::Shape shape) { _shape = shape; }

	// Given the channel and polarization, determine which beam in the
	// image to use for the resolution area.
	void setBeam(Int channel, Int polarization);

	Quantity fluxDensityToPeakIntensity(
		Bool& hadToMakeFakeBeam, const Quantity& fluxDensity
	) const;

	Quantity peakIntensityToFluxDensity(
		Bool& hadToMakeFakeBeam, const Quantity& fluxDensity
	) const;

	String getClass() const {
		static const String s = "PeakIntensityFluxDensityConverter";
		return s;
	}

protected:
	inline  CasacRegionManager::StokesControl _getStokesControl() const {
		return CasacRegionManager::USE_ALL_STOKES;
	}

	inline vector<Coordinate::Type> _getNecessaryCoordinates() const {
		return vector<Coordinate::Type>(1, Coordinate::DIRECTION);
	}

private:

	Angular2DGaussian _size;
	ComponentType::Shape _shape;
	GaussianBeam _beam;



};
}

#endif
