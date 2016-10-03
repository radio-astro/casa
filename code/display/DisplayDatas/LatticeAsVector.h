//# LatticeAsVector.h: Class to display data as a vector map
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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
//# $Id$

#ifndef TRIALDISPLAY_LATTICEASVECTOR_H
#define TRIALDISPLAY_LATTICEASVECTOR_H

//# aips includes:
#include <casa/aips.h>
#include <casa/Quanta/Unit.h>

//# trial includes:
#include <images/Images/ImageInterface.h>

//# display library includes:
#include <display/DisplayDatas/LatticePADD.h>

namespace casacore{

	template <class T> class Array;
	class IPosition;
	class Record;
}

namespace casa { //# NAMESPACE CASA - BEGIN

//# forwards:
	template <class T> class LatticePADMVector;

// <summary>Class to manage the drawing of vector maps of slices from AIPS++ Lattices</summary>
//
// <use visibility=export>
//
// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>
//
// <prerequisite>
// <li> <linkto class="LatticePADisplayData">LatticePADisplayData</linkto>
// <li> <linkto class="DisplayData">DisplayData</linkto>
// </prerequisite>
//
// <etymology>
// The purpose of this class is to draw vectors maps of data that
// is lattice-based.
// </etymology>
//
// <synopsis>
// This class should be used to display vector maps---i.e.
// lines with an amplitude and position angle
// for two-dimensional slices of data extracted from AIPS++ Lattices or
// Arrays having two or more dimensions.  The data source can be Complex
// or Float.  If casacore::Complex, then both the amplitude and position angle
// can be extracted.  If casacore::Float, the data represents the position angle
// (degrees) and an amplitude of unity is assumed.
// I
//
// At construction, any axes in the data can be mapped to the X and Y
// axes of the display device (see the <linkto
// class="PrincipalAxesDD">PrincipalAxesDD</linkto> class).  For data
// with more than two dimensions, a third axis in the dataset can be
// selected for generating a sequence of maps along: this is known
// as the "movie" axis.  Animation (see the <linkto
// class="Animator">Animator</linkto> class) will cause different
// slices of the data to be selected from along this axis.  After
// construction, the axis settings of a LatticeAsVector object can be
// modified at a later stage.
//
//# </synopsis>
//
// <example>
// A LatticeAsVector object could be constructed and used as follows:
// <srcblock>
// casacore::PagedImage<casacore::Complex> *pimage = new casacore::PagedImage<casacore::Complex>(casacore::String("test.im"));
// DisplayData *dd;
// casacore::uInt ndim = pimage->ndim();
// if (ndim < 2) {
//   throw(casacore::AipsError(casacore::String("Image has less than two dimensions")));
// } else if (ndim == 2) {
//   dd = (DisplayData *)(new LatticeAsVector<casacore::Complex>(pimage, 0, 1));
// } else {
//   casacore::IPosition fixedPos(ndim);
//   fixedPos = 0;
//   dd = (DisplayData *)(new LatticeAsVector<casacore::Complex>(pimage, 0, 1, 2,fixedPos));
// }
// // wcHolder is an existing WorldCanvasHolder *...
// wcHolder->addDisplayData(ddata);
// wcHolder->refresh();
// </srcblock>
// </example>
//
// <motivation>
// Displaying 2-dimensional slices of a lattice-based data volume is
// a standard display requirement for astronomical data visualization
// and presentation.
// </motivation>
//
// <templating arg=T>
// </templating>
//
// <thrown>
// </thrown>
//
// <todo asof="2000/12/16">
// </todo>

	template <class T> class LatticeAsVector : public LatticePADisplayData<T> {

	public:
		// casacore::Array-based constructors: >2d and 2d.  xAxis and yAxis specify
		// which axis in the array (0-based) should be mapped to X and Y
		// on the display device: ie. 2-d slices of the data to be displayed
		// have these axes.  mAxis specifies the "movie" axis, which is the axis
		// along which different slices are taken.  fixedPos is an IPosition
		// having the same length as the number of dimensions in the array,
		// and indicate the fixed axis values for axes in the data that are
		// not specified as xAxis, yAxis or mAxis.
		// <group>
		LatticeAsVector(casacore::Array<T>* array, const casacore::uInt xAxis,
		                const casacore::uInt yAxis, const casacore::uInt mAxis,
		                const casacore::IPosition fixedPos);
		LatticeAsVector(casacore::Array<T>* array, const casacore::uInt xAxis,
		                const casacore::uInt yAxis);
		// </group>

		// Image-based constructors: >2d and 2d.  xAxis and yAxis specify
		// which axis in the image (0-based) should be mapped to X and Y
		// on the display device: ie. 2-d slices of the data to be displayed
		// have these axes.  mAxis specifies the "movie" axis, which is the axis
		// along which different slices are taken.  fixedPos is an IPosition
		// having the same length as the number of dimensions in the image,
		// and indicate the fixed axis values for axes in the data that are
		// not specified as xAxis, yAxis or mAxis.
		// <group>
		LatticeAsVector(SHARED_PTR<casacore::ImageInterface<T> > image, const casacore::uInt xAxis,
		                const casacore::uInt yAxis, const casacore::uInt mAxis,
		                const casacore::IPosition fixedPos);
		LatticeAsVector(SHARED_PTR<casacore::ImageInterface<T> > image, const casacore::uInt xAxis,
		                const casacore::uInt yAxis);
		// </group>

		// Destructor
		virtual ~LatticeAsVector();

		// Create the various elements in the sequence of displayable
		// maps.  This is called upon construction as well as whenever
		// the display and/or movie axes are changed via a call to
		// PrincipalAxesDD::setAxes.
		//virtual void setupElements(casacore::IPosition fixedPos = casacore::IPosition(2));
		virtual void setupElements();

		// install the default options for display
		virtual void setDefaultOptions();

		// Apply the options stored in the provided casacore::Record to the
		// LatticeAsVector object.  If the return value is true, then
		// some options have changed, and a refresh is needed to update
		// the display.
		virtual casacore::Bool setOptions(casacore::Record& rec, casacore::Record& recOut);

		// Retrieve the currently set options, and their types, default
		// values, and any help text associated with each parameter.  This
		// information can be used to generate form-type graphical user
		// interfaces or command-line interfaces to set the options with
		// prompts.
		virtual casacore::Record getOptions( bool scrub=false ) const;

		// Return the DisplayData type; used by the WorldCanvasHolder to
		// determine the order of drawing.
		virtual Display::DisplayDataType classType() {
			return Display::Vector;
		}

		// class name
		virtual casacore::String className() const {
			return casacore::String("LatticeAsVector");
		}

		// Value of the casacore::Lattice at a position--used by showValue().  Overrides
		// base version to take user-controlled addition to phase angle into account.
		virtual /*const*/ T dataValue(casacore::IPosition pos);

	private:

		casacore::Float getVariance();
//
		casacore::Float itsScale;          // Amplitude scale factor
		casacore::Float itsLineWidth;      // Line width of vectors
		casacore::Int itsIncX;             // Increment in X (pixels)
		casacore::Int itsIncY;             // Increment in Y (pixels)
		casacore::Bool itsArrow;           // Do vectors have arrow head ?
		casacore::Float itsBarb;           // Arrow head shape
		casacore::String itsColor;         // Color of vectors
		casacore::Float itsRotation;       // Add extra rotation
		casacore::String itsPhaseType;     // phase handling type (normal/polarimetric)
		casacore::Bool itsDebias;          // Debias amplitude ?
		casacore::Float itsVar;            // Variance of noise for debiasing
		casacore::Unit itsUnits;           // Image brightness units
		casacore::Bool itsConstAmp;        // SHow as constant amplitude
//
		friend class LatticePADMVector<T>;

		//# Make parent members known.
	public:
		using LatticePADisplayData<T>::nelements;
		using LatticePADisplayData<T>::nPixelAxes;
		using LatticePADisplayData<T>::fixedPosition;
		using LatticePADisplayData<T>::displayAxes;
		using LatticePADisplayData<T>::dataShape;
		using LatticePADisplayData<T>::getMinAndMax;
		using LatticePADisplayData<T>::readOptionRecord;
	protected:
		using LatticePADisplayData<T>::setNumImages;
		using LatticePADisplayData<T>::DDelement;
		using LatticePADisplayData<T>::datamax;
	};


//# Define template specializations.
	template <> casacore::Float LatticeAsVector<casacore::Complex>::getVariance();
	template <> casacore::Float LatticeAsVector<casacore::Float>::getVariance();



} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <display/DisplayDatas/LatticeAsVector.tcc>
//#include <display/DisplayDatas/LatticeAsVector2.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC
#endif
