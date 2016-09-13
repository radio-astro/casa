//# ScrollingRasterDD.h: Base class for scrolling DisplayData objects
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003,2004
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
//
#ifndef TRIALDISPLAY_SCROLLINGRASTERDD_H
#define TRIALDISPLAY_SCROLLINGRASTERDD_H

#include <display/DisplayDatas/PrincipalAxesDD.h>

#include <casa/Arrays/Array.h>
#include <lattices/Lattices/Lattice.h>
#include <casa/Containers/Record.h>

#include <images/Images/ImageInterface.h>
#include <lattices/Lattices/MaskedLattice.h>
#include <lattices/LatticeMath/LatticeStatistics.h>
#include <lattices/Lattices/SubLattice.h>
#include <lattices/Lattices/LatticeConcat.h>

// <summary>
// Base class for scrolling DisplayData objects
// </summary>

namespace casa { //# NAMESPACE CASA - BEGIN

	class WCResampleHandler;

	class ScrollingRasterDM;

	class ScrollingRasterDD : public PrincipalAxesDD {

	public:
		ScrollingRasterDD(const casacore::uInt nDim,
		                  const casacore::IPosition, const casacore::Vector<casacore::String>, const casacore::Vector<casacore::String>,
		                  casacore::uInt sAxis = 2, casacore::uInt scanNo = 100);
		virtual ~ScrollingRasterDD();

		virtual void updateLattice(const casacore::Record &) {};
		virtual void updateLattice(casacore::Array<casacore::Float> &, DisplayCoordinateSystem &);

		virtual casacore::String className() const {
			return casacore::String("ScrollingRasterDD");
		}

		virtual casacore::Bool setOptions(casacore::Record &rec, casacore::Record &recOut);
		virtual casacore::Record getOptions( bool scrub=false ) const;

		virtual void setDefaultOptions();

		// distribute options to all the axis labellers - empty here, we use WorldAxesDD
		virtual casacore::Bool setLabellerOptions(casacore::Record &, casacore::Record &) {
			return false;
		}

		// retrieve options from the axis labellers - empty here, we use WorldAxesDD
		virtual casacore::Record getLabellerOptions( bool /*scrub*/=false ) const {
			casacore::Record rec;
			return rec;
		}


		virtual casacore::Bool sizeControl(WorldCanvasHolder& wcHolder,
		                         AttributeBuffer& holderBuf);

		virtual WCResampleHandler *resampleHandler() {
			return itsResampleHandler;
		}

	protected:
		friend class ScrollingRasterDM;

		virtual void updateLatticeConcat(casacore::Array<casacore::Float>* = NULL , DisplayCoordinateSystem* = NULL);
		virtual void initLattice(const casacore::Record &);
		virtual void initLattice(const casacore::Float, const casacore::Float, const casacore::uInt);

		virtual void recreateEmptyLattices(casacore::uInt changedScanNumber = 0);

		virtual const casacore::IPosition dataShape() const;
		virtual casacore::uInt dataDim() const;
		virtual const casacore::Unit dataUnit() const;
		virtual void setupElements();
		virtual void getMinAndMax();
		virtual void updateLatticeStatistics();

		//virtual void refresh(casacore::Bool);

		virtual casacore::Bool labelAxes(const WCRefreshEvent &ev);

		virtual casacore::MaskedLattice<casacore::Float>* maskedLattice() {
			return itsLatticeConcatPtr;
		}

		virtual Display::DisplayDataType classType() {
			return Display::Raster;
		}
		// Pure virtual function from DisplayData...
		casacore::String dataType() const {
			return "scrolling";
		}

		virtual casacore::String showValue(const casacore::Vector<casacore::Double> &world);
		virtual casacore::Float dataValue(casacore::IPosition pos);
		virtual casacore::Bool maskValue(const casacore::IPosition &pos);

		virtual casacore::Vector<casacore::String> worldAxisNames() const;
		virtual casacore::Vector<casacore::String> worldAxisUnits() const;

		// (Required) default constructor.
		ScrollingRasterDD(casacore::uInt mAxis=2, casacore::uInt scanNo=100);

		// (Required) copy constructor.
		ScrollingRasterDD(const ScrollingRasterDD &other);

		// (Required) copy assignment.
		void operator=(const ScrollingRasterDD &other);

		// Set Spectral preference -> not used here.
		virtual void setSpectralPreference (
		    DisplayCoordinateSystem& /*cSys*/, const casacore::String&, const casacore::String& ) {}

		void setHeaderMin(casacore::Float x) {
			itsHeaderMin = x;
		}
		void setHeaderMax(casacore::Float x) {
			itsHeaderMax = x;
		}
		casacore::Float headerMin() {
			return itsHeaderMin;
		}
		casacore::Float headerMax() {
			return itsHeaderMax;
		}
		void setScanNumber(casacore::uInt x) {
			itsScanNumber = x;
		}
		casacore::uInt scanNumber() {
			return itsScanNumber;
		}
		casacore::Bool headerReceived() {
			return itsHeaderReceived;
		}
		void setHeaderReceived(casacore::Bool x) {
			itsHeaderReceived = x;
		}

		casacore::IPosition latticesShape() {
			return itsLattices[0]->shape();
		}
		casacore::uInt shiftAxis() {
			return itsShiftAxis;
		}

		void setNeedResize(const casacore::Bool x) {
			itsNeedResize = x;
		}
		casacore::Bool needResize() {
			return itsNeedResize;
		}

		casacore::IPosition fixedPos() {
			return itsFixedPos;
		}

		//void setPlaneNumber(const casacore::uInt x) { itsPlaneNumber = x; }
		//casacore::uInt planeNumber() { return itsPlaneNumber; }

		void setLatticeShape(const casacore::IPosition x) {
			itsLatticeShape = x;
		}
		casacore::IPosition latticeShape() {
			return itsLatticeShape;
		}

	private:
		// Worker function for c'tors.
		void initSRDD(const casacore::Vector<casacore::String> aAxisNames, const casacore::Vector<casacore::String> aAxisUnits,
		              casacore::uInt mAxis);

		casacore::uInt nDim;

		//casacore::ImageInterface<casacore::Float> *itsImagePtr;
		////casacore::Array<casacore::Float>          *itsBaseArrayPtr;
		casacore::LatticeConcat<casacore::Float>* itsLatticeConcatPtr;
		//casacore::MaskedLattice<casacore::Float>  *itsMaskedLatticePtr;
		casacore::LatticeStatistics<casacore::Float> *itsLatticeStatisticsPtr;
		casacore::SubLattice<casacore::Float> *itsFilledDisplayedLatticePtr;
		//casacore::Int itsM2Axis;
		casacore::IPosition itsFixedPos;

		casacore::Int itsFilledCount;

		casacore::MaskedLattice<casacore::Float> **itsLattices;

		casacore::Bool itsNeedResize;
		casacore::uInt itsShiftAxis;

		casacore::Bool itsHeaderReceived;
		casacore::Float itsHeaderMin;
		casacore::Float itsHeaderMax;
		casacore::uInt itsScanNumber;
		casacore::IPosition itsLatticeShape;
		//casacore::IPosition itsScanShape;
		//casacore::uInt itsPlaneNumber;

		// storage for the display parameters
		casacore::String itsResample;

		// pointer to resampler
		WCResampleHandler *itsResampleHandler;

	};


} //# NAMESPACE CASA - END

#endif

