//# ScrollingRasterDD.cc: Base class for scrolling DisplayData objects
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

#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Slicer.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Containers/Record.h>

#include <lattices/Lattices/SubLattice.h>
#include <lattices/Lattices/LatticeConcat.h>
#include <lattices/Lattices/ArrayLattice.h>

#include <display/DisplayDatas/ScrollingRasterDD.h>
#include <display/DisplayDatas/ScrollingRasterDM.h>

#include <display/DisplayCanvas/WCResampleHandler.h>
#include <display/DisplayCanvas/WCSimpleResampleHandler.h>
#include <display/Display/Attribute.h>

#include <display/Display/DisplayCoordinateSystem.h>
#include <coordinates/Coordinates/LinearCoordinate.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//#define CDEBUG 1

	ScrollingRasterDD::ScrollingRasterDD(const uInt nDim,
	                                     const IPosition arrShape,
	                                     const Vector<String> aAxisNames,
	                                     const Vector<String> aAxisUnits,
	                                     uInt mAxis, uInt scanNo):

		PrincipalAxesDD (0, 1, mAxis, False), // mAxis = 3 for PKSMultiBeam
		// False - don't use built-in AxesLabellers
		itsLatticeConcatPtr(0),
		itsLatticeStatisticsPtr(0),
		itsFilledDisplayedLatticePtr(0),

		itsFixedPos(IPosition(nDim, 0, 0, 0, 0)),
		itsFilledCount(0),
		itsNeedResize(True),
		itsShiftAxis(1),
		itsHeaderReceived(False),
		itsHeaderMin(0.),
		itsHeaderMax(1.),
		itsScanNumber(scanNo),
		itsLatticeShape(arrShape),
		itsResampleHandler(0) {
		initSRDD(aAxisNames, aAxisUnits, mAxis);
	}

	void ScrollingRasterDD::initSRDD(const Vector<String> aAxisNames,
	                                 const Vector<String> aAxisUnits,
	                                 uInt mAxis) {
		if (itsLatticeShape == 0) {
			throw AipsError("ScrollingRasterDD constructor - empty shape given.");
		}
		if (itsScanNumber == 0) {
			throw AipsError("scanNo == 0 in ScrollingRasterDD constructor.");
		}
		itsLattices = new MaskedLattice<Float>*[itsScanNumber];
		for (uInt i=0; i<itsScanNumber; i++) {
			itsLattices[i] = 0;
		}
#ifdef CDEBUG
		cerr << "itsScanNumber = " << itsScanNumber
		     << ", itsLattices array created & filled with 0." << endl;
#endif

		recreateEmptyLattices();

		DisplayCoordinateSystem newcsys;

		//LinearCoordinate lc(itsLatticeConcatPtr->ndim());
		LinearCoordinate lc(4);
		newcsys.addCoordinate(lc);

		if (aAxisNames.nelements() > 0) {
			newcsys.setWorldAxisNames(aAxisNames);
		}
		if (aAxisUnits.nelements() > 0) {
			newcsys.setWorldAxisUnits(aAxisUnits);
		}

		setCoordinateSystem(newcsys);

		updateLatticeStatistics();
		getMinAndMax();

		setAxes(0, 1, mAxis, itsFixedPos);

		setup(itsFixedPos);
		setupElements();
		setDefaultOptions();
	}

	void ScrollingRasterDD::recreateEmptyLattices (uInt changedScanNumber) {
		// nShape for PKS: 0:FREQ, 1:TIME(shiftAxis), 2:NIF, 3:NBEAM
#ifdef CDEBUG
		cerr << "ScrollingRasterDD::recreateEmptyLattices : itsLatticeShape = "
		     << itsLatticeShape << endl;
		if (itsLattices && itsLattices[0]) {
			cerr << "    : old Shape = " << itsLattices[0]->shape() << endl;
		} else {
			cerr << "    : old Shape ? - no Lattices allocated yet..." << endl;
		}
#endif
		for (uInt i=0; i<itsScanNumber; i++) {
			delete itsLattices[i];
		}
		delete [] itsLattices;

		if (changedScanNumber) {
			itsScanNumber = changedScanNumber;
		}

		itsLattices = new MaskedLattice<Float>*[itsScanNumber];

		for (uInt i=0; i<itsScanNumber; i++) {
			ArrayLattice<Float> zeroAL(itsLatticeShape);
			zeroAL.set(0.0);
			zeroAL.putAt(itsHeaderMin + 1.0, zeroAL.shape() / 2);

			itsLattices[i] = new SubLattice<Float>(zeroAL);
		}
		updateLatticeConcat();
	}

	void ScrollingRasterDD::initLattice(const Float aHeaderMin,
	                                    const Float aHeaderMax,
	                                    const uInt aScanNumber) {
#ifdef CDEBUG
		cerr << "ScrollingRasterDD::initLattice(aMin, aMax, aScanNo) called" << endl;
#endif
		if (aScanNumber == 0) {
			throw AipsError ("scan number = 0 in ScrollingRasterDD::initLattice.");
		}
		itsHeaderMin = aHeaderMin;
		itsHeaderMax = aHeaderMax;
		itsScanNumber = aScanNumber;

#ifdef CDEBUG
		static uInt oldScanNumber(itsScanNumber);
		if (oldScanNumber!= itsScanNumber) {
			cerr << "WARNING: itsScanNumber changed from "
			     << oldScanNumber << " to " << itsScanNumber << " !!!" << endl;
		}
#endif
		recreateEmptyLattices();
	}

	void ScrollingRasterDD::initLattice(const Record &/*rec*/) {
#ifdef CDEBUG
		cerr << "ScrollingRasterDD::initLattice(Record) called" << endl;
#endif
		if ((!itsHeaderReceived) ||
		        (itsScanNumber == 0) ||
		        (itsHeaderMin == itsHeaderMax)) {
			throw AipsError("Problem in ScrollingRasterDD::initLattice(Record&)");
		}
		recreateEmptyLattices();
		setupElements();
	}

	void ScrollingRasterDD::updateLattice(Array<Float> &arr, DisplayCoordinateSystem &csys) {
#ifdef CDEBUG
		cerr << "ScrollingRasterDD::updateLattice(arr, csys) called." << endl;
#endif
		IPosition arrShape = arr.shape();

		IPosition baseShape = itsLattices[0]->shape();

		setNeedResize(False);

		uInt i = 0;
		while ((i < arr.ndim()) && (!needResize())) {
			if (i != itsShiftAxis) {
				setNeedResize(needResize() || ( arrShape(i) != baseShape(i) ));
			}
			i++;
		}

		updateLatticeConcat(&arr, &csys);
	}

	void ScrollingRasterDD::updateLatticeConcat(Array<Float>* arr,
	        DisplayCoordinateSystem* csys) {
#ifdef CDEBUG
		cerr << "ScrollingRasterDD::updateLatticeConcat(arr = " << arr
		     << ", csys = " << csys << ") called." << endl;
#endif
		if (arr && csys) {
			if (arr->ndim() != 4) {
				throw AipsError("ScrollingRasterDD::updateLatticeConcat(arr, csys) - arr->ndim!=4");
			}

			MaskedLattice<Float>* arrLattice =
			    new SubLattice<Float>(ArrayLattice<Float>(*arr));

			if (needResize()) {
				ArrayLattice<Float> zeroAL(arrLattice->shape());
				zeroAL.set(0.0);
				zeroAL.putAt(1.0, zeroAL.shape() / 2);

				for (uInt i=1; i<itsScanNumber; i++) {
					delete itsLattices[i];
					itsLattices[i] = new SubLattice<Float>(zeroAL);
				}
			}

			delete itsLattices[0];

			for (uInt i=0; i < (itsScanNumber-1); i++) {
				itsLattices[i] = itsLattices[i+1];
			}
			itsLattices[itsScanNumber-1] = arrLattice;

			setCoordinateSystem(*csys);
		}
		delete itsLatticeConcatPtr;
		itsLatticeConcatPtr = new LatticeConcat<Float>(itsShiftAxis);

		for (uInt i=0; i<itsScanNumber; i++) {
			itsLatticeConcatPtr->setLattice(*(itsLattices[itsScanNumber-1 - i]));
		}

		updateLatticeStatistics();
		getMinAndMax();

		if (csys) {
			setup(itsFixedPos);
		}
		//setDefaultOptions(); <- not every time ...
	}

	ScrollingRasterDD::~ScrollingRasterDD() {
		for (uInt i=0; i<nelements(); i++) {
			if(DDelement[i]!=0) {
				delete static_cast<ScrollingRasterDM*>(DDelement[i]);
				DDelement[i] = 0; // always :>
			}
		}

		delete itsResampleHandler;
		itsResampleHandler = 0;
		if (itsLattices) {
			for (uInt i=0; i<itsScanNumber; i++) {
				if (itsLattices[i]) {
					delete itsLattices[i];
					itsLattices[i] = 0;
				}
			}
			delete[] itsLattices;
			itsLattices = 0;
		}
		delete itsLatticeStatisticsPtr;
		itsLatticeStatisticsPtr = 0;
		delete itsFilledDisplayedLatticePtr;
		itsFilledDisplayedLatticePtr = 0;
		delete itsLatticeConcatPtr;
		itsLatticeConcatPtr = 0;
	}

	const IPosition ScrollingRasterDD::dataShape() const {
		return itsLatticeConcatPtr->shape();
	}

	const uInt ScrollingRasterDD::dataDim() const {
		return itsLatticeConcatPtr->ndim();
	}

	const Unit ScrollingRasterDD::dataUnit() const {
		Unit JyPerBeam ("Jy/beam");
		return JyPerBeam;
	}

	Vector<String> ScrollingRasterDD::worldAxisUnits() const {
		Vector<String> tempvec = coordinateSystem().worldAxisUnits();
		return tempvec;
	}

	Vector<String> ScrollingRasterDD::worldAxisNames() const {
		Vector<String> tempvec = coordinateSystem().worldAxisNames();
		return tempvec;
	}

	void ScrollingRasterDD::setupElements() {
		nPixelAxes = coordinateSystem().nPixelAxes();

		Vector<Int> dispAxes = displayAxes();

		IPosition fixedPos = fixedPosition();
#ifdef CDEBUG
		cerr << "ScrollingRasterDD::setupElements() - nPixelAxes = " << nPixelAxes << endl;
		cerr << "... displayAxes = " << dispAxes;
		cerr << ", nelements() = " << nelements() << endl;
		cerr << "... dataShape() = " << dataShape() << endl;
#endif
		if (nPixelAxes > 2) {
			// clean up all old DisplayMethods
			for (uInt i=0; i < nelements(); i++) {
				delete static_cast<ScrollingRasterDM *>(DDelement[i]);
				DDelement[i] = 0;
			}
			setNumImages(dataShape()(dispAxes(2)));
#ifdef CDEBUG
			cerr << "ScrDD::setupElements: setNumImages("
			     << dataShape()(dispAxes(2)) << ") called." << endl;
#endif
			DDelement.resize(nelements(), True);
			for (uInt index = 0; index < nelements(); index++) {
				fixedPos(dispAxes(2)) = index;
				DDelement[index] = (ScrollingRasterDM *)new
				                   ScrollingRasterDM(dispAxes(0), dispAxes(1), dispAxes(2),
				                                     fixedPos, (PrincipalAxesDD *)this);
			}
		} else {
			cerr << "ScrollingRasterDD::setupElements - ONLY "
			     << nPixelAxes << " PixelAxes!!!" << endl;
		}
		PrincipalAxesDD::setupElements();

		iAmRubbish = False;
	}

	void ScrollingRasterDD::getMinAndMax() {
		if (itsHeaderReceived) {
			datamin = itsHeaderMin;
			datamax = itsHeaderMax;
			return;
		}

		if (!itsLatticeConcatPtr || !itsLatticeStatisticsPtr) {
			throw(AipsError("ScrollingRasterDD::getMinAndMax - "
			                "no lattice is available"));
			return;
		}

		Float dMin, dMax;
		if (!itsLatticeStatisticsPtr->getFullMinMax(dMin, dMax)) {
			datamin = -1.0;
			datamax = 1.0;
		} else {
			datamin = dMin;
			datamax = dMax;
		}
	}

	void ScrollingRasterDD::updateLatticeStatistics() {
		if (itsHeaderReceived) {
			return;
		}

		if (itsLatticeStatisticsPtr) {
			delete itsLatticeStatisticsPtr;  // shouldn't we rather call setNewLattice ?
			itsLatticeStatisticsPtr = 0;
		}

		AlwaysAssert(itsFixedPos.nelements() ==
		             itsLatticeConcatPtr->ndim(), AipsError);

		IPosition arrShape = itsLatticeConcatPtr->shape();

		IPosition iStart (4, 0, arrShape(1)-itsFilledCount-1,
		                  itsFixedPos(2), itsFixedPos(3));

		IPosition iEnd (4, arrShape(0), arrShape(1),
		                itsFixedPos(2)+1, itsFixedPos(3)+1);

		IPosition iWidth(4, arrShape(0),
		                 (itsFilledCount ? itsFilledCount : 1),
		                 1, 1);


		//Slicer slicer(iStart, iEnd);
		Slicer slicer(iStart, iWidth);

		if (itsFilledDisplayedLatticePtr) {
			delete itsFilledDisplayedLatticePtr;
			itsFilledDisplayedLatticePtr = 0;
		}

		if (itsLatticeConcatPtr) {
			itsFilledDisplayedLatticePtr = new SubLattice<Float>(
			    *itsLatticeConcatPtr, slicer);
		} else {
			throw AipsError("NULL itsLatticeConcatPtr !");
		}

		if (!itsFilledDisplayedLatticePtr) {
			throw AipsError("Cannot allocate itsFilledDisplayedLatticePtr !!!");
		}

		itsLatticeStatisticsPtr =
		    new LatticeStatistics<Float>(*itsFilledDisplayedLatticePtr, False);

		if (!itsLatticeStatisticsPtr) {
			throw AipsError("Cannot allocate itsLatticeStatisticsPtr");
		}
	}


	String ScrollingRasterDD::showValue(const Vector<Double> &/*world*/) {
		String temp="";
		return temp;
	}


	const Float ScrollingRasterDD::dataValue(IPosition pos) {
		if (!itsLatticeConcatPtr) {
			throw(AipsError("ScrollingRasterDD::dataValue - "
			                "no lattice is available"));
		}
		if (pos.nelements() != itsLatticeConcatPtr->ndim()) {
			throw(AipsError("ScrollingRasterDD::dataValue - "
			                "no such position in lattice"));
		}
		return itsLatticeConcatPtr->operator()(pos);
	}


	const Bool ScrollingRasterDD::maskValue(const IPosition &pos) {
		if (!itsLatticeConcatPtr) {
			throw(AipsError("ScrollingRasterDD::maskValue - "
			                "no lattice available"));
		}
		if (pos.nelements() != itsLatticeConcatPtr->ndim()) {
			throw(AipsError("ScrollingRasterDD::maskValue - "
			                "no such position in lattice"));
		}

// We must use getMaskSlice rather than pixelMask() because
// application of the OTF mask is not reflected by the
// pixelMask() Lattice

		static Array<Bool> tmp;
		static Bool deleteIt;
		{
			itsLatticeConcatPtr->getMaskSlice(tmp,Slicer(pos));
			return *(tmp.getStorage(deleteIt));
		}
	}

	Bool ScrollingRasterDD::sizeControl(WorldCanvasHolder &wcHolder,
	                                    AttributeBuffer &holderBuf) {
		Bool ret = PrincipalAxesDD::sizeControl(wcHolder, holderBuf);
		return ret;
	}


	Bool ScrollingRasterDD::setOptions(Record &rec, Record &recOut) {
		Bool ret = False;

		const String updateStr("update");
		const String valueStr("value");

		if (rec.isDefined(updateStr) &&
		        (rec.dataType(updateStr) == TpRecord)) {
			Record subrec = rec.subRecord(updateStr);
			if (subrec.isDefined(valueStr) &&
			        (subrec.dataType(valueStr) == TpRecord)) {
#ifdef CDEBUG
				cerr << "DD update" << endl;
#endif
				updateLattice(subrec.subRecord(valueStr));
				ret = True;
			}
		}

		const String initStr("init");

		if (rec.isDefined(initStr) &&
		        (rec.dataType(initStr) == TpRecord)) {
			Record subrec = rec.subRecord(initStr);
#ifdef CDEBUG
			cerr << "doing DD Init" << endl;
			cerr << "subrec.description() = " << subrec.description() << endl;
#endif
			if (subrec.isDefined(valueStr) &&
			        (subrec.dataType(valueStr) == TpRecord)) {
#ifdef CDEBUG
				cerr << "OK, got 'init' Record, calling initLattice(Record)..." << endl;
#endif
				initLattice(subrec.subRecord(valueStr));
				ret = True;
			} else {
				throw AipsError("cannot find 'value' subrecord in 'init' record !");
			}

			Record setanimrec;
			if (!recOut.isDefined("setanimator")) {
#ifdef CDEBUG
				cerr << "ScRDD::setOptions - cannot find setanimator subrecord: defining it now." << endl;
#endif
				Record setanimrec;
				recOut.defineRecord("setanimator", setanimrec);
				// This will just poll all conformant DDs for the (maximum)
				// number of animation frames--any DD may request this.
			}

			if(isCSmaster()) {	// (only a CS master DD should set
				// an explicit animation length or index...).

#ifdef CDEBUG
				cerr << "nelements = " <<nelements() << endl;
				cerr << "dataShape() = " << dataShape() << endl;
				cerr << "resetting zindex to 0." << endl;
#endif

				setanimrec.define("zindex",0);
				setanimrec.define("zlength", (Int)dataShape()[displayAxes()[2]]);

#ifdef CDEBUG
				cerr << "setting zlength = dataShape()[" << displayAxes()[2] << "]"
				     << " = " << dataShape()[displayAxes()[2]] << endl;
#endif

				recOut.defineRecord("setanimator",setanimrec);
			}
		}

		if (PrincipalAxesDD::setOptions(rec, recOut)) {
			ret = True;
		}

		Bool error;
		if(readOptionRecord(itsResample, error, rec, "resample")) {
			ret = True;

			delete itsResampleHandler;

			if (itsResample == "bilinear") {
				itsResampleHandler = new WCSimpleResampleHandler(Interpolate2D::LINEAR);
			} else if (itsResample=="bicubic") {
				itsResampleHandler = new WCSimpleResampleHandler(Interpolate2D::CUBIC);
				itsResample = "bicubic";
			} else {
				itsResampleHandler = new WCSimpleResampleHandler(Interpolate2D::NEAREST);
				itsResample = "nearest";
			}
		}
		return ret;
	}

	Record ScrollingRasterDD::getOptions() {
		Record rec = PrincipalAxesDD::getOptions();

		Record resample;
		resample.define("dlformat", "resample");
		resample.define("listname", "resampling mode");
		resample.define("ptype", "choice");
		Vector<String> vresample(3);
		vresample(0) = "nearest";
		vresample(1) = "bilinear";
		vresample(2) = "bicubic";
		resample.define("popt", vresample);
		resample.define("default", "nearest");
		resample.define("value", itsResample);
		resample.define("allowunset", False);
		rec.defineRecord("resample", resample);

		return rec;
	}

	void ScrollingRasterDD::setDefaultOptions() {
		PrincipalAxesDD::setDefaultOptions();

		itsResample = "nearest";
		delete itsResampleHandler;
		itsResampleHandler = new WCSimpleResampleHandler(Interpolate2D::NEAREST);

		Record rec, recout;

		rec.define("aspect", "flexible");
		setOptions(rec, recout);
		//itsOptionsAspect = "flexible";

		rec.define("axislabelswitch", False); // don't want old labellers... :(
		setOptions(rec, recout);
	}

	Bool ScrollingRasterDD::labelAxes(const WCRefreshEvent &/*ev*/) {
		return False;
	}

	ScrollingRasterDD::ScrollingRasterDD(uInt mAxis, uInt scanNo):
		PrincipalAxesDD (0, 1, mAxis, False),   // False - don't use built-in AxesLabellers
		itsLatticeConcatPtr(0),
		itsLatticeStatisticsPtr(0),
		itsFilledDisplayedLatticePtr(0),
		itsFixedPos(IPosition(3, 0, 0, 0)),
		itsFilledCount(0),
		itsNeedResize(True),
		itsShiftAxis(1),
		itsHeaderReceived(False),
		itsHeaderMin(0.),
		itsHeaderMax(1.),
		itsScanNumber(scanNo),
		itsLatticeShape(IPosition(3, 0, 0, 0)),
		itsResampleHandler(0) {
		String empty("");
		const uInt aNDim = itsFixedPos.nelements();
		Vector<String> axisVec(aNDim, empty);
		Vector<String> unitVec(aNDim, empty);

		initSRDD(axisVec, unitVec, mAxis);
	}

// (Required) copy constructor.
	ScrollingRasterDD::ScrollingRasterDD(const ScrollingRasterDD &o) : PrincipalAxesDD(o) {
	}

// (Required) copy assignment.
	void ScrollingRasterDD::operator=(const ScrollingRasterDD &/*other*/) {
	}


} //# NAMESPACE CASA - END

