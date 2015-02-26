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
		ScrollingRasterDD(const uInt nDim,
		                  const IPosition, const Vector<String>, const Vector<String>,
		                  uInt sAxis = 2, uInt scanNo = 100);
		virtual ~ScrollingRasterDD();

		virtual void updateLattice(const Record &) {};
		virtual void updateLattice(Array<Float> &, DisplayCoordinateSystem &);

		virtual String className() const {
			return String("ScrollingRasterDD");
		}

		virtual Bool setOptions(Record &rec, Record &recOut);
		virtual Record getOptions( bool scrub=false ) const;

		virtual void setDefaultOptions();

		// distribute options to all the axis labellers - empty here, we use WorldAxesDD
		virtual Bool setLabellerOptions(Record &, Record &) {
			return False;
		}

		// retrieve options from the axis labellers - empty here, we use WorldAxesDD
		virtual Record getLabellerOptions( bool /*scrub*/=false ) const {
			Record rec;
			return rec;
		}


		virtual Bool sizeControl(WorldCanvasHolder& wcHolder,
		                         AttributeBuffer& holderBuf);

		virtual WCResampleHandler *resampleHandler() {
			return itsResampleHandler;
		}

	protected:
		friend class ScrollingRasterDM;

		virtual void updateLatticeConcat(Array<Float>* = NULL , DisplayCoordinateSystem* = NULL);
		virtual void initLattice(const Record &);
		virtual void initLattice(const Float, const Float, const uInt);

		virtual void recreateEmptyLattices(uInt changedScanNumber = 0);

		virtual const IPosition dataShape() const;
		virtual uInt dataDim() const;
		virtual const Unit dataUnit() const;
		virtual void setupElements();
		virtual void getMinAndMax();
		virtual void updateLatticeStatistics();

		//virtual void refresh(Bool);

		virtual Bool labelAxes(const WCRefreshEvent &ev);

		virtual MaskedLattice<Float>* maskedLattice() {
			return itsLatticeConcatPtr;
		}

		virtual Display::DisplayDataType classType() {
			return Display::Raster;
		}
		// Pure virtual function from DisplayData...
		String dataType() const {
			return "scrolling";
		}

		virtual String showValue(const Vector<Double> &world);
		virtual Float dataValue(IPosition pos);
		virtual Bool maskValue(const IPosition &pos);

		virtual Vector<String> worldAxisNames() const;
		virtual Vector<String> worldAxisUnits() const;

		// (Required) default constructor.
		ScrollingRasterDD(uInt mAxis=2, uInt scanNo=100);

		// (Required) copy constructor.
		ScrollingRasterDD(const ScrollingRasterDD &other);

		// (Required) copy assignment.
		void operator=(const ScrollingRasterDD &other);

		// Set Spectral preference -> not used here.
		virtual void setSpectralPreference (
		    DisplayCoordinateSystem& /*cSys*/, const String&, const String& ) {}

		void setHeaderMin(Float x) {
			itsHeaderMin = x;
		}
		void setHeaderMax(Float x) {
			itsHeaderMax = x;
		}
		Float headerMin() {
			return itsHeaderMin;
		}
		Float headerMax() {
			return itsHeaderMax;
		}
		void setScanNumber(uInt x) {
			itsScanNumber = x;
		}
		uInt scanNumber() {
			return itsScanNumber;
		}
		Bool headerReceived() {
			return itsHeaderReceived;
		}
		void setHeaderReceived(Bool x) {
			itsHeaderReceived = x;
		}

		IPosition latticesShape() {
			return itsLattices[0]->shape();
		}
		uInt shiftAxis() {
			return itsShiftAxis;
		}

		void setNeedResize(const Bool x) {
			itsNeedResize = x;
		}
		Bool needResize() {
			return itsNeedResize;
		}

		IPosition fixedPos() {
			return itsFixedPos;
		}

		//void setPlaneNumber(const uInt x) { itsPlaneNumber = x; }
		//uInt planeNumber() { return itsPlaneNumber; }

		void setLatticeShape(const IPosition x) {
			itsLatticeShape = x;
		}
		IPosition latticeShape() {
			return itsLatticeShape;
		}

	private:
		// Worker function for c'tors.
		void initSRDD(const Vector<String> aAxisNames, const Vector<String> aAxisUnits,
		              uInt mAxis);

		uInt nDim;

		//ImageInterface<Float> *itsImagePtr;
		////Array<Float>          *itsBaseArrayPtr;
		LatticeConcat<Float>* itsLatticeConcatPtr;
		//MaskedLattice<Float>  *itsMaskedLatticePtr;
		LatticeStatistics<Float> *itsLatticeStatisticsPtr;
		SubLattice<Float> *itsFilledDisplayedLatticePtr;
		//Int itsM2Axis;
		IPosition itsFixedPos;

		Int itsFilledCount;

		MaskedLattice<Float> **itsLattices;

		Bool itsNeedResize;
		uInt itsShiftAxis;

		Bool itsHeaderReceived;
		Float itsHeaderMin;
		Float itsHeaderMax;
		uInt itsScanNumber;
		IPosition itsLatticeShape;
		//IPosition itsScanShape;
		//uInt itsPlaneNumber;

		// storage for the display parameters
		String itsResample;

		// pointer to resampler
		WCResampleHandler *itsResampleHandler;

	};


} //# NAMESPACE CASA - END

#endif

