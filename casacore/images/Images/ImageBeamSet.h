//# Copyright (C) 1996,1997,1998,1999,2000,2001,2003
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

#ifndef IMAGES_IMAGEBEAMSET_H
#define IMAGES_IMAGEBEAMSET_H

#include <casa/aips.h>
#include <components/ComponentModels/GaussianBeam.h>

namespace casa {

// <summary>
// Represents a set of restoring beams associated with an image.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
// </prerequisite>

// <etymology>
// A Set of Beams associated with an Image.
// </etymology>

// <synopsis>
// This class represents a set of restoring beams associated with
// a deconvolved image.
// </synopsis>
//
// <example>

// </example>


// <motivation>
// Restoring beams are used many places in image analysis tasks.
// </motivation>

// <todo>
// </todo>

class ImageBeamSet {
public:

	enum AxisType {
		SPECTRAL,
		POLARIZATION
	};

	// Construct an empty beam set.
	ImageBeamSet();

	// Construct a beam set from an array of beams with the
	// representing the specified image axes. The number of
	// dimensions in the <src>beams</src> Array must match
	// the number of elements in the <src>axes</src> Vector.
	ImageBeamSet(
		const Array<GaussianBeam>& beams,
		const Vector<AxisType>& axes
	);

	// construct an ImageBeamSet representing a single beam
	ImageBeamSet(const GaussianBeam& beam);

	//create an ImageBeamSet of the specified shape with all
	// GaussianBeams initialized to null beams.
	ImageBeamSet(const IPosition& shape, const Vector<AxisType>& axes);

	ImageBeamSet(const ImageBeamSet& other);

	~ImageBeamSet();

	ImageBeamSet& operator=(const ImageBeamSet& other);

	GaussianBeam &operator()(const IPosition &);

	const GaussianBeam &operator()(const IPosition &) const;

    Array<GaussianBeam> operator[] (uInt i) const;

    Array<GaussianBeam> operator()(
    	const IPosition &start,
        const IPosition &end
    );

    const Array<GaussianBeam> operator()(
    	const IPosition &start,
    	const IPosition &end
    ) const;

    Bool operator== (const ImageBeamSet& other) const;

    Bool operator!= (const ImageBeamSet& other) const;

	const Vector<AxisType>& getAxes() const;

	// get the single global beam. If there are multiple beams,
	// an exception is thrown.
	const GaussianBeam& getBeam() const;

	// get the beam at the specified location
	const GaussianBeam& getBeam(
		const IPosition& position,
		const Vector<AxisType>& axes=Vector<AxisType>(0)
	) const;

	// set the beam at a given location. Optionally, the axis types can
	// be supplied so one can specify the position using a different axis
	// ordering than the current object has. If specified, <src>axes</src>
	// must have the same number of elements as the dimensionality of the
	// current beam set and all axes of the current beam set must be
	// elements.
	void setBeam(
		const GaussianBeam& beam, const IPosition& position,
		const Vector<AxisType>& axes=Vector<AxisType>(0)
	);

	// does this beam set contain only a single beam?
	Bool hasSingleBeam() const;

	// does this beam set contain multiple beams?
	Bool hasMultiBeam() const;

	// is this beam set empty (ie does it not have any beams)?
	//Bool isEmpty() const;

	static const String& className();

	size_t size() const;

	void resize(const IPosition& pos);

	const Array<GaussianBeam>& getBeams() const;

	void setBeams(const Array<GaussianBeam>& beams);

	size_t nelements() const;

	Bool empty() const;

	IPosition shape() const;

	size_t ndim() const;

	void set(const GaussianBeam& beam);

private:
	Array<GaussianBeam> _beams;
	Vector<AxisType> _axes;

	IPosition _truePosition(
		const IPosition& position, const Vector<AxisType>& axes
	) const;

	static void _checkForDups(const Vector<AxisType>& axes);

	void _checkAxisTypeSize(const Vector<AxisType>& axes) const;

};

}

#endif

