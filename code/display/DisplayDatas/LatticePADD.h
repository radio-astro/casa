//# LatticePADD.h: Class for displaying lattices along principal axes
//# Copyright (C) 1998,1999,2000,2001,2002
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

#ifndef TRIALDISPLAY_LATTICEPADD_H
#define TRIALDISPLAY_LATTICEPADD_H

#include <casa/aips.h>
#include <casa/Quanta/Unit.h>
#include <images/Images/ImageInterface.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>
#include <casa/Containers/Record.h>
#include <display/Display/DParameterChoice.h>
#include <display/Display/DParameterRange.h>

namespace casacore{

	class IPosition;
	class ImageRegion;
	class WCLELMask;
	template <class T> class Array;
	template <class T> class Lattice;
	template <class T> class MaskedLattice;
	template <class T> class LatticeStatistics;
}

namespace casa { //# NAMESPACE CASA - BEGIN

	class WCResampleHandler;
	class WorldCanvas;

// <summary>
// Partial implementation of PrincipalAxesDD for casacore::Lattice-based data.
// </summary>
//
// <synopsis>
// This class is a partial (ie. base) implementation of PrincipalAxesDD
// which adds methods particular to handling casacore::Lattice-based data.
// </synopsis>

	template <class T> class LatticePADisplayData : public PrincipalAxesDD {

	public:
		// Constructors (no default)
		//LatticePADisplayData();

		// casacore::Array-based constructors: >2d and 2d
		// <group>
		LatticePADisplayData(casacore::Array<T> *array, const casacore::uInt xAxis,
		                     const casacore::uInt yAxis, const casacore::uInt mAxis,
		                     const casacore::IPosition fixedPos);
		LatticePADisplayData(casacore::Array<T> *array, const casacore::uInt xAxis,
		                     const casacore::uInt yAxis);
		// </group>

		// Image-based constructors: >2d and 2d
		// <group>
		LatticePADisplayData( SHARED_PTR<casacore::ImageInterface<T> > image, const casacore::uInt xAxis, const casacore::uInt yAxis, const casacore::uInt mAxis, const casacore::IPosition fixedPos, viewer::StatusSink *sink=0 );
		LatticePADisplayData(SHARED_PTR<casacore::ImageInterface<T> > image, const casacore::uInt xAxis,
		                     const casacore::uInt yAxis);
		// </group>

		// Destructor
		virtual ~LatticePADisplayData();

		// casacore::Format a string containing value information at the
		// given world coordinate
		virtual casacore::String showValue(const casacore::Vector<casacore::Double> &world);

		// required functions to help inherited "setup" amongst other things
		virtual const casacore::IPosition dataShape() const;
		virtual casacore::uInt dataDim() const;
		virtual /*const*/ T dataValue(casacore::IPosition pos);
		virtual const casacore::Unit dataUnit() const;

		// Pure virtual function from DisplayData...
		casacore::String dataType() const {
			return "image";
		}
		// Get image analyis about object...
		virtual ImageAnalysis *imageanalysis( ) const;
		SHARED_PTR<casacore::ImageInterface<casacore::Float> > imageinterface( );


		// left as pure virtual for implementation in concrete class
		virtual void setupElements() = 0;
		virtual void getMinAndMax();

		// return mask value at given position
		virtual casacore::Bool maskValue(const casacore::IPosition &pos);

		// install the default options for this DisplayData
		virtual void setDefaultOptions();

		// apply options stored in val to the DisplayData; return value
		// true means a refresh is needed...
		virtual casacore::Bool setOptions(casacore::Record &rec, casacore::Record &recOut);



		// retrieve the current and default options and parameter types.
		virtual casacore::Record getOptions( bool scrub=false ) const;

		// Return the class name of this DisplayData; useful mostly for
		// debugging purposes, and perhaps future use in the glish widget
		// interface.
		virtual casacore::String className() const {
			return casacore::String("LatticePADisplayData");
		}
		casacore::String description( ) const {
			return itsBaseImagePtr ? itsBaseImagePtr->name( ) : "none available";
		}

		virtual WCResampleHandler *resampleHandler() {
			return itsResampleHandler;
		}



		virtual Display::ComplexToRealMethod complexMode() {
			return itsComplexToRealMethod;
		}
		virtual void setComplexMode(Display::ComplexToRealMethod method) {
			itsComplexToRealMethod = method;
		}

		virtual SHARED_PTR<casacore::MaskedLattice<T> > maskedLattice() {
			return itsMaskedLatticePtr;
		}


		// Insert an array into a Record. The array is insert into a "value" field, eg
		// somerecord.fieldname.value
		virtual casacore::Bool insertArray(casacore::Record& toGoInto, casacore::Vector<casacore::Float> toInsert, const casacore::String fieldname);
		virtual casacore::Bool insertFloat(casacore::Record& toGoInto, casacore::Float toInsert, const casacore::String fieldname);

		// Return the last calculated histogram
		virtual casacore::Record getHist() const;

		// Return the brightness unit as a string
		virtual casacore::String getBrightnessUnits() const;

		// Aids updateHistogram() by computing a stride to use for efficiency
		// when computing histograms (could be used elsewhere too).
		// casacore::Input parameter 'shape' is the shape of the original lattice or array.
		// Return value indicates whether striding should be used; if so, the
		// recommended stride is returned in the 'stride' parameter.
		// maxPixels is the desired maximum number of elements in the sub-lattice
		// that would result from using the returned stride (may be exceeded
		// because of minPerAxis requirements, or in any case by a few percent).
		// A stride greater than 1 will not be returned for an axis if it
		// would make the length of that axis in the strided sub-lattice
		// less than minPerAxis.
		static casacore::Bool useStriding(const casacore::IPosition& shape, casacore::IPosition& stride,
		                        casacore::uInt maxPixels=1000000u, casacore::uInt minPerAxis=20u);


	protected:

		// Called by constructors: set up data for beam drawing, if applicable.
		virtual void SetUpBeamData_();


		// Will draw the beam ellipse if applicable (i.e., the LatticePADD
		// has an image with beam data, beam drawing is turned on, and the
		// WC's casacore::CoordinateSystem is set to sky coordinates).
		virtual void drawBeamEllipse_(WorldCanvas* wc);



	private:

		// The base image cloned at construction.
		SHARED_PTR<casacore::ImageInterface<T> > itsBaseImagePtr;

		// The base array cloned at construction.
		casacore::Array<T>* itsBaseArrayPtr;

		// The image histogram
		casacore::Record imageHistogram;

		// Whether to always calculate the histogram or not
		casacore::Bool calcHist;

		// The masked lattice, effectively referencing one of itsBaseImagePtr
		// or itsBaseArray, or some sub-region of said.
		SHARED_PTR<casacore::MaskedLattice<T> > itsMaskedLatticePtr;

		// Says whether the destructor should delete itsMaskedLattice or not
		casacore::Bool itsDeleteMLPointer;

		// Object to use for calculating statistics.
		casacore::LatticeStatistics<casacore::Float>* itsLatticeStatisticsPtr;

		// Is itsLattice a casacore::SubImage?
		casacore::ImageRegion* itsRegionPtr;

		// OTF mask
		casacore::WCLELMask* itsMaskPtr;

		// The data unit
		casacore::Unit itsDataUnit;

		// the complex to real method
		Display::ComplexToRealMethod itsComplexToRealMethod;

		// storage for the display parameters
		casacore::String itsResample;
		casacore::String itsComplexMode;

		// beam-drawing state
		// </group>

		std::vector<std::vector<double> > beams_;
		casacore::String majorunit_;	//# units of above (should be angular
		casacore::String minorunit_;	//# (relative world sky coordinates).
		casacore::String paunit_;	//# angular units of posangle_.

		DParameterChoice* beamOnOff_;		//# User-selectable parameters.
		DParameterChoice* beamStyle_;		//# "Outline", "Filled"
		DParameterChoice* beamColor_;
		DParameterRange<casacore::Int>* beamLineWidth_;
		DParameterRange<casacore::Float>* beamXCenter_;		//# 0=left edge, 1=right edge
		DParameterRange<casacore::Float>* beamYCenter_;		//# 0=bottom edge, 1=top edge
		// </group>



		// pointer to resampler
		WCResampleHandler *itsResampleHandler;

		// update itsLatticeStatistics
		void updateLatticeStatistics();

		// Update the histogram, and attach it to the supplied record
		casacore::Bool updateHistogram(casacore::Record &rec, casacore::MaskedLattice<casacore::Complex> &pImage);
		casacore::Bool updateHistogram(casacore::Record &rec, casacore::ImageInterface<casacore::Float> &pImage);
		casacore::Bool updateHistogram(casacore::Record &rec, const casacore::Array<casacore::Complex>*);
		casacore::Bool updateHistogram(casacore::Record &rec, const casacore::Array<casacore::Float>*);

		casacore::WCLELMask* makeMask (const casacore::RecordInterface& mask);
		casacore::ImageRegion* makeRegion (const casacore::RecordInterface& region);
		casacore::Bool isMaskDifferent (casacore::WCLELMask*& pMask);
		casacore::Bool isRegionDifferent (casacore::ImageRegion*& pRegion);

		// Transfer preferences between CoordinateSystems
		casacore::Bool transferPreferences (DisplayCoordinateSystem& cSysInOut,
		                          const DisplayCoordinateSystem& cSysIn) const;

		// Delete temporary image data
		void delTmpImage();
	};


} //# NAMESPACE CASA - END

#ifndef AIPS_NO_TEMPLATE_SRC
#include <display/DisplayDatas/LatticePADD.tcc>
//#include <display/DisplayDatas/LatticePADD2.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC
#endif
