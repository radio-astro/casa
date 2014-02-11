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

#include <tr1/memory>

namespace casa { //# NAMESPACE CASA - BEGIN

	class IPosition;
	class WCResampleHandler;
	class ImageRegion;
	class WCLELMask;
	class WorldCanvas;
	template <class T> class Array;
	template <class T> class Lattice;
	template <class T> class MaskedLattice;
	template <class T> class LatticeStatistics;

// <summary>
// Partial implementation of PrincipalAxesDD for Lattice-based data.
// </summary>
//
// <synopsis>
// This class is a partial (ie. base) implementation of PrincipalAxesDD
// which adds methods particular to handling Lattice-based data.
// </synopsis>

	template <class T> class LatticePADisplayData : public PrincipalAxesDD {

	public:
		// Constructors (no default)
		//LatticePADisplayData();

		// Array-based constructors: >2d and 2d
		// <group>
		LatticePADisplayData(Array<T> *array, const uInt xAxis,
		                     const uInt yAxis, const uInt mAxis,
		                     const IPosition fixedPos);
		LatticePADisplayData(Array<T> *array, const uInt xAxis,
		                     const uInt yAxis);
		// </group>

		// Image-based constructors: >2d and 2d
		// <group>
		LatticePADisplayData( std::tr1::shared_ptr<ImageInterface<T> > image, const uInt xAxis, const uInt yAxis, const uInt mAxis, const IPosition fixedPos, viewer::StatusSink *sink=0 );
		LatticePADisplayData(std::tr1::shared_ptr<ImageInterface<T> > image, const uInt xAxis,
		                     const uInt yAxis);
		// </group>

		// Destructor
		virtual ~LatticePADisplayData();

		// Format a string containing value information at the
		// given world coordinate
		virtual String showValue(const Vector<Double> &world);

		// required functions to help inherited "setup" amongst other things
		virtual const IPosition dataShape() const;
		virtual const uInt dataDim() const;
		virtual const T dataValue(IPosition pos);
		virtual const Unit dataUnit() const;

		// Pure virtual function from DisplayData...
		String dataType() const {
			return "image";
		}
		// Get image analyis about object...
		virtual ImageAnalysis *imageanalysis( ) const;
		std::tr1::shared_ptr<ImageInterface<Float> > imageinterface( );


		// left as pure virtual for implementation in concrete class
		virtual void setupElements() = 0;
		virtual void getMinAndMax();

		// return mask value at given position
		virtual const Bool maskValue(const IPosition &pos);

		// install the default options for this DisplayData
		virtual void setDefaultOptions();

		// apply options stored in val to the DisplayData; return value
		// True means a refresh is needed...
		virtual Bool setOptions(Record &rec, Record &recOut);



		// retrieve the current and default options and parameter types.
		virtual Record getOptions( bool scrub=false ) const;

		// Return the class name of this DisplayData; useful mostly for
		// debugging purposes, and perhaps future use in the glish widget
		// interface.
		virtual String className() const {
			return String("LatticePADisplayData");
		}
		String description( ) const {
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

		virtual std::tr1::shared_ptr<MaskedLattice<T> > maskedLattice() {
			return itsMaskedLatticePtr;
		}


		// Insert an array into a Record. The array is insert into a "value" field, eg
		// somerecord.fieldname.value
		virtual Bool insertArray(Record& toGoInto, Vector<Float> toInsert, const String fieldname);
		virtual Bool insertFloat(Record& toGoInto, Float toInsert, const String fieldname);

		// Return the last calculated histogram
		virtual Record getHist() const;

		// Return the brightness unit as a string
		virtual String getBrightnessUnits() const;

		// Aids updateHistogram() by computing a stride to use for efficiency
		// when computing histograms (could be used elsewhere too).
		// Input parameter 'shape' is the shape of the original lattice or array.
		// Return value indicates whether striding should be used; if so, the
		// recommended stride is returned in the 'stride' parameter.
		// maxPixels is the desired maximum number of elements in the sub-lattice
		// that would result from using the returned stride (may be exceeded
		// because of minPerAxis requirements, or in any case by a few percent).
		// A stride greater than 1 will not be returned for an axis if it
		// would make the length of that axis in the strided sub-lattice
		// less than minPerAxis.
		static Bool useStriding(const IPosition& shape, IPosition& stride,
		                        uInt maxPixels=1000000u, uInt minPerAxis=20u);


	protected:

		// Called by constructors: set up data for beam drawing, if applicable.
		virtual void SetUpBeamData_();


		// Will draw the beam ellipse if applicable (i.e., the LatticePADD
		// has an image with beam data, beam drawing is turned on, and the
		// WC's CoordinateSystem is set to sky coordinates).
		virtual void drawBeamEllipse_(WorldCanvas* wc);



	private:

		// The base image cloned at construction.
		std::tr1::shared_ptr<ImageInterface<T> > itsBaseImagePtr;

		// The base array cloned at construction.
		Array<T>* itsBaseArrayPtr;

		// The image histogram
		Record imageHistogram;

		// Whether to always calculate the histogram or not
		Bool calcHist;

		// The masked lattice, effectively referencing one of itsBaseImagePtr
		// or itsBaseArray, or some sub-region of said.
		std::tr1::shared_ptr<MaskedLattice<T> > itsMaskedLatticePtr;

		// Says whether the destructor should delete itsMaskedLattice or not
		Bool itsDeleteMLPointer;

		// Object to use for calculating statistics.
		LatticeStatistics<Float>* itsLatticeStatisticsPtr;

		// Is itsLattice a SubImage?
		ImageRegion* itsRegionPtr;

		// OTF mask
		WCLELMask* itsMaskPtr;

		// The data unit
		Unit itsDataUnit;

		// the complex to real method
		Display::ComplexToRealMethod itsComplexToRealMethod;

		// storage for the display parameters
		String itsResample;
		String itsComplexMode;

		// beam-drawing state
		// </group>

		std::vector<std::vector<double> > beams_;
		String majorunit_;	//# units of above (should be angular
		String minorunit_;	//# (relative world sky coordinates).
		String paunit_;	//# angular units of posangle_.

		DParameterChoice* beamOnOff_;		//# User-selectable parameters.
		DParameterChoice* beamStyle_;		//# "Outline", "Filled"
		DParameterChoice* beamColor_;
		DParameterRange<Int>* beamLineWidth_;
		DParameterRange<Float>* beamXCenter_;		//# 0=left edge, 1=right edge
		DParameterRange<Float>* beamYCenter_;		//# 0=bottom edge, 1=top edge
		// </group>



		// pointer to resampler
		WCResampleHandler *itsResampleHandler;

		// update itsLatticeStatistics
		void updateLatticeStatistics();

		// Update the histogram, and attach it to the supplied record
		Bool updateHistogram(Record &rec, MaskedLattice<Complex> &pImage);
		Bool updateHistogram(Record &rec, ImageInterface<Float> &pImage);
		Bool updateHistogram(Record &rec, const Array<Complex>*);
		Bool updateHistogram(Record &rec, const Array<Float>*);

		WCLELMask* makeMask (const RecordInterface& mask);
		ImageRegion* makeRegion (const RecordInterface& region);
		Bool isMaskDifferent (WCLELMask*& pMask);
		Bool isRegionDifferent (ImageRegion*& pRegion);

		// Transfer preferences between CoordinateSystems
		Bool transferPreferences (DisplayCoordinateSystem& cSysInOut,
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
