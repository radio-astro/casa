//# LatticePADD.cc: Class for displaying lattices along principal axes
//# Copyright (C) 1998,1999,2000,2001,2002,2004
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

#include <casa/aips.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/IPosition.h>
#include <casa/Containers/Record.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <display/Display/Attribute.h>
#include <display/DisplayDatas/DisplayMethod.h>
#include <display/DisplayDatas/LatticePADM.h>
#include <display/DisplayCanvas/WCResampleHandler.h>
#include <display/DisplayCanvas/WCSimpleResampleHandler.h>
#include <display/DisplayDatas/LatticePADD.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/Lattice.h>
#include <lattices/LatticeMath/LatticeStatistics.h>
#include <lattices/Lattices/MaskedLattice.h>
#include <lattices/Lattices/LatticeLocker.h>
#include <lattices/Lattices/SubLattice.h>
#include <images/Images/ImageInterface.h>
#include <display/Utilities/ImageProperties.h>
#include <imageanalysis/ImageAnalysis/SubImageFactory.h>
#include <images/Images/SubImage.h>
#include <images/Regions/ImageRegion.h>
#include <images/Regions/WCLELMask.h>
#include <scimath/Mathematics/Interpolate2D.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <tables/Tables/TableRecord.h>
#include <casa/Quanta/Unit.h>
#include <casa/OS/RegularFile.h>
#include <casa/OS/Directory.h>
#include <display/Display/WorldCanvas.h>


namespace casa { //# NAMESPACE CASA - BEGIN



// >2d array-based ctor
	template <class T>
	LatticePADisplayData<T>::LatticePADisplayData(casacore::Array<T> *array,
	        const casacore::uInt xAxis,
	        const casacore::uInt yAxis,
	        const casacore::uInt mAxis,
	        const casacore::IPosition fixedPos) :
		PrincipalAxesDD(xAxis, yAxis, mAxis),
		itsBaseImagePtr(0),
		itsBaseArrayPtr(0),
		itsMaskedLatticePtr(0),
		itsDeleteMLPointer(false),
		itsLatticeStatisticsPtr(0),
		itsRegionPtr(0),
		itsMaskPtr(0),
		itsDataUnit("_"),
		itsComplexToRealMethod(Display::Magnitude) {

		itsBaseArrayPtr = new casacore::Array<T>;
		*itsBaseArrayPtr = array->copy();
		itsMaskedLatticePtr = new casacore::SubLattice<T>(casacore::ArrayLattice<T>(*itsBaseArrayPtr));
		itsDeleteMLPointer = true;
		updateLatticeStatistics();

		iAmRubbish = false;
		/*
		casacore::Vector<casacore::Int> axes(3);
		axes(0) = xAxis;
		axes(1) = yAxis;
		axes(2) = mAxis;
		*/

		// setup a coordinate system
		casacore::CoordinateSystem newcsys;
		casacore::LinearCoordinate lc(itsMaskedLatticePtr->ndim());
		newcsys.addCoordinate(lc);
		casacore::Vector<casacore::Double> tmp = newcsys.referencePixel();
		tmp = tmp - (casacore::Double)1.0;
		newcsys.setReferencePixel(tmp);
		setCoordinateSystem(newcsys);

		// call base class setup:
		setup(fixedPos);
		getMinAndMax();


	}

// 2d array-based ctor
	template <class T>
	LatticePADisplayData<T>::LatticePADisplayData(casacore::Array<T> *array,
	        const casacore::uInt xAxis,
	        const casacore::uInt yAxis) :
		PrincipalAxesDD(xAxis, yAxis),
		itsBaseImagePtr(0),
		itsBaseArrayPtr(0),
		itsMaskedLatticePtr(0),
		itsDeleteMLPointer(false),
		itsLatticeStatisticsPtr(0),
		itsRegionPtr(0),
		itsMaskPtr(0),
		itsDataUnit("_"),
		itsComplexToRealMethod(Display::Magnitude) {

		itsBaseArrayPtr = new casacore::Array<T>;
		*itsBaseArrayPtr = array->copy();
		itsMaskedLatticePtr = new casacore::SubLattice<T>(casacore::ArrayLattice<T>(*itsBaseArrayPtr));
		itsDeleteMLPointer = true;
		updateLatticeStatistics();

		iAmRubbish = false;
		/*
		casacore::Vector<casacore::Int> axes(2);
		axes(0) = xAxis;
		axes(1) = yAxis;
		*/

		// setup a coordinate system
		casacore::CoordinateSystem newcsys;
		casacore::LinearCoordinate lc(2);
		newcsys.addCoordinate(lc);
		casacore::Vector<casacore::Double> tmp = newcsys.referencePixel().copy();
		tmp = tmp - (casacore::Double)1.0;
		newcsys.setReferencePixel(tmp);
		setCoordinateSystem(newcsys);

		casacore::IPosition fixedPos(2);
		fixedPos = 0;

		// call base class setup
		setup(fixedPos);
		getMinAndMax();
	}


// >2d image-based ctor
	template <class T>
	LatticePADisplayData<T>::LatticePADisplayData(SHARED_PTR<casacore::ImageInterface<T> > image,
	        const casacore::uInt xAxis,
	        const casacore::uInt yAxis,
	        const casacore::uInt mAxis,
	        const casacore::IPosition fixedPos, viewer::StatusSink *sink ) :
		PrincipalAxesDD(xAxis, yAxis, mAxis, true, sink),
		itsBaseImagePtr(),
		itsBaseArrayPtr(0),
		itsMaskedLatticePtr(),
		itsDeleteMLPointer(false),
		itsLatticeStatisticsPtr(0),
		itsRegionPtr(0),
		itsMaskPtr(0),
		itsDataUnit(image->units()),
		itsComplexToRealMethod(Display::Magnitude) {

		itsBaseImagePtr.reset(image->cloneII());
		itsMaskedLatticePtr = itsBaseImagePtr;
		updateLatticeStatistics();

		iAmRubbish = false;

		setCoordinateSystem(itsBaseImagePtr->coordinates());

		// call base class setup:
		setup(fixedPos);
		getMinAndMax();

		SetUpBeamData_();
	}


// 2d image-based ctor
	template <class T>
	LatticePADisplayData<T>::LatticePADisplayData(SHARED_PTR<casacore::ImageInterface<T> > image,
	        const casacore::uInt xAxis,
	        const casacore::uInt yAxis) :
		PrincipalAxesDD(xAxis, yAxis),
		itsBaseImagePtr(),
		itsBaseArrayPtr(0),
		itsMaskedLatticePtr(),
		itsDeleteMLPointer(false),
		itsLatticeStatisticsPtr(0),
		itsRegionPtr(0),
		itsMaskPtr(0),
		itsDataUnit(image->units()),
		itsComplexToRealMethod(Display::Magnitude) {
		itsBaseImagePtr.reset(image->cloneII());
		itsMaskedLatticePtr = itsBaseImagePtr;
		updateLatticeStatistics();

		iAmRubbish = false;
		setCoordinateSystem(image->coordinates());
		casacore::IPosition fixedPos(2);
		fixedPos = 0;

		// call base class setup
		setup(fixedPos);
		getMinAndMax();

		SetUpBeamData_();
	}



// Destructor
	template <class T>
	LatticePADisplayData<T>::~LatticePADisplayData() {

		if (delTmpData_)
			delTmpImage();

		if (itsLatticeStatisticsPtr) {
			delete itsLatticeStatisticsPtr;
		}
		if (itsBaseArrayPtr) {
			delete itsBaseArrayPtr;
		}
		if (itsResampleHandler) {
			delete itsResampleHandler;
		}

		if( beams_.size( ) > 0 ) {
			delete beamOnOff_;
			delete beamStyle_;
			delete beamColor_;
			delete beamLineWidth_;
			delete beamXCenter_;
			delete beamYCenter_;
		}

	}



// Query the shape of the lattice
	template <class T>
	const casacore::IPosition LatticePADisplayData<T>::dataShape() const {
		if (!itsMaskedLatticePtr) {
			throw(casacore::AipsError("LatticePADisplayData<T>::dataShape - "
			                "no lattice is available"));
		}
		return itsMaskedLatticePtr->shape();
	}

// Query the dimension of the lattice
	template <class T>
	casacore::uInt LatticePADisplayData<T>::dataDim() const {
		if (!itsMaskedLatticePtr) {
			throw(casacore::AipsError("LatticePADisplayData<T>::dataDim - "
			                "no lattice is available"));
		}
		return itsMaskedLatticePtr->ndim();
	}

// Query the value of the lattice at a particular position:
	template <class T>
	/*const*/ T LatticePADisplayData<T>::dataValue(casacore::IPosition pos) {
		if (!itsMaskedLatticePtr) {
			throw(casacore::AipsError("LatticePADisplayData<T>::dataValue - "
			                "no lattice is available"));
		}
		if (pos.nelements() != itsMaskedLatticePtr->ndim()) {
			throw(casacore::AipsError("LatticePADisplayData<T>::dataValue - "
			                "no such position in lattice"));
		}
		return itsMaskedLatticePtr->operator()(pos);

	}

	template <class T>
	casacore::Bool LatticePADisplayData<T>::maskValue(const casacore::IPosition &pos) {
		if (!itsMaskedLatticePtr) {
			throw(casacore::AipsError("LatticePADisplayData<T>::maskValue - "
			                "no lattice available"));
		}
		if (pos.nelements() != itsMaskedLatticePtr->ndim()) {
			throw(casacore::AipsError("LatticePADisplayData<T>::maskValue - "
			                "no such position in lattice"));
		}

// We must use getMaskSlice rather than pixelMask() because
// application of the OTF mask is not reflected by the
// pixelMask() Lattice

		static casacore::Array<casacore::Bool> tmp;
		static casacore::Bool deleteIt;
		{
			itsMaskedLatticePtr->getMaskSlice(tmp,casacore::Slicer(pos));
			return *(tmp.getStorage(deleteIt));
		}
	}

// Query the units of the lattice values
	template <class T>
	const casacore::Unit LatticePADisplayData<T>::dataUnit() const {
		if (!itsMaskedLatticePtr) return casacore::Unit("");
		//    throw(casacore::AipsError("LatticePADisplayData<T>::dataUnit - "
		//	(dumb)	    "no lattice is available"));

		if(itsDataUnit==casacore::Unit("_")) return casacore::Unit("");

		return itsDataUnit;
	}

	template <class T>
	casacore::String LatticePADisplayData<T>::getBrightnessUnits() const {
		try {
			return itsDataUnit.getName();
		} catch(...) {
			throw(casacore::AipsError("LatticePADisplayData<T>::getBrightnessUnit - "
			                "couldn't get brightness unit"));
		}

	}

	template <class T>
	void LatticePADisplayData<T>::setDefaultOptions() {

		calcHist = false;
		PrincipalAxesDD::setDefaultOptions();
		itsResample = "nearest";
		itsResampleHandler = new WCSimpleResampleHandler(casacore::Interpolate2D::NEAREST);
		itsComplexMode = "magnitude";
		setComplexMode(Display::Magnitude);
	}

	template <class T>
	casacore::Bool LatticePADisplayData<T>::setOptions(casacore::Record &rec, casacore::Record &recOut) {
		casacore::Bool ret = PrincipalAxesDD::setOptions(rec, recOut);
		casacore::Bool newHistNeeded = false;    // lei050

		casacore::ImageInterface<T>* pImage = 0;
		casacore::DataType dtype;

		casacore::Bool error;
		if(readOptionRecord(itsResample, error, rec, "resample")) {
			ret = true;

			//newHistNeeded = true;
			//#dk -- commented out 12/05
			// This is unnecessary, I believe: display resampling
			// mode should have no effect on the data histogram,
			// and recalculating it can be expensive!

			if (itsResampleHandler) {
				delete itsResampleHandler;
			}
			if (itsResample == "bilinear") {
				itsResampleHandler = new WCSimpleResampleHandler(casacore::Interpolate2D::LINEAR);
			} else if (itsResample=="bicubic") {
				itsResampleHandler = new WCSimpleResampleHandler(casacore::Interpolate2D::CUBIC);
				itsResample = "bicubic";
			} else {
				itsResampleHandler = new WCSimpleResampleHandler(casacore::Interpolate2D::NEAREST);
				itsResample = "nearest";
			}
		}
		//
		casacore::Bool fillRecOut = false;
		T typetester;
		dtype = casacore::whatType(&typetester);

		if ((dtype == casacore::TpComplex) || (dtype == casacore::TpDComplex)) {
			if (readOptionRecord(itsComplexMode, error, rec, "complexmode")) {
				ret = true;
				newHistNeeded = true;

				if (itsComplexMode == "phase") {
					setComplexMode(Display::Phase);
				} else if (itsComplexMode == "real") {
					setComplexMode(Display::Real);
				} else if (itsComplexMode == "imaginary") {
					setComplexMode(Display::Imaginary);
				} else {
					setComplexMode(Display::Magnitude);
					itsComplexMode = "magnitude";
				}
				cleanup();
				updateLatticeStatistics();
				getMinAndMax();
				fillRecOut = true;
			}
		}

		casacore::Bool reread = false;
		casacore::Bool forceRegionParse = false;
		if (readOptionRecord(reread, error, rec, "reread")) {
			ret = true;
			cleanup();
			getMinAndMax();
			fillRecOut = true;
			forceRegionParse = true;
			if (itsBaseImagePtr) {   //reset to default mask
				casacore::LatticeLocker lock(*itsBaseImagePtr, casacore::FileLocker::Write);
				if (lock.hasLock(casacore::FileLocker::Write)) {
					itsBaseImagePtr->setDefaultMask(itsBaseImagePtr->getDefaultMask());
					newHistNeeded = true;
				} else {
					casacore::LogIO os(casacore::LogOrigin("LatticePADisplayData", "setOptions", WHERE));
					os << casacore::LogIO::SEVERE << "Couldn't lock image." << casacore::LogIO::POST;
				}
			}
		}

		if (rec.isDefined("newdata")) {
			newHistNeeded = true;
			casacore::DataType indtype = rec.dataType("newdata");
			if ((indtype == casacore::TpString) && itsBaseImagePtr) {
				// we were built from an image, and we've been given a string -
				// assume it's an image name...

			} else if (!itsBaseImagePtr &&
			           (((indtype == casacore::TpArrayFloat) && (dtype == casacore::TpFloat)) ||
			            ((indtype == casacore::TpArrayComplex) && (dtype == casacore::TpComplex)))) {
				// we were built from an array of casacore::Float/casacore::Complex, and we've been given
				// the same...
				casacore::Array<T> array;
				rec.get("newdata", array);
				if (array.shape().nelements() != itsBaseArrayPtr->shape().nelements()) {
					throw(casacore::AipsError("dimensionality of new data is invalid"));
				}
				cleanup();

// When the data source is an casacore::Array, itsDeleteMLPointer must always
// be true

				AlwaysAssert(itsDeleteMLPointer, casacore::AipsError);
				itsMaskedLatticePtr.reset(new casacore::SubLattice<T>(casacore::ArrayLattice<T>(array)));
				updateLatticeStatistics();
				setAxes(displayAxes()[0], displayAxes()[1],
				        displayAxes()[2], fixedPosition());
				getMinAndMax();
				fillRecOut = true;
				ret = true;
			} else {
				throw(casacore::AipsError("Invalid use of 'newdata' option"));
			}
		}

// The image region and OTF mask both involve the use of SubImage
// We must handle them together, and the mask expression must
// be applied first, followed by application of the region

		if (itsBaseImagePtr && (rec.isDefined("region") || rec.isDefined("mask"))) {
			casacore::String resetString("resetCoordinates");
			Attribute resetAtt(resetString, true);

// If the region is unset, the returned pointer is null

			casacore::ImageRegion* pRegion = 0;
			casacore::Bool regionChanged = false;
			if (rec.isDefined("region")) {
				pRegion = makeRegion (rec);

// Update private region pointer and see if region changed

				regionChanged = isRegionDifferent (pRegion);
				if (regionChanged) {
					newHistNeeded = true;
				}

			}

// If the mask is unset, the returned pointer is null

			casacore::WCLELMask* pMask = 0;
			casacore::Bool maskChanged = false;
			if (rec.isDefined("mask")) {
				pMask = makeMask (rec);

// Update private mask pointer and see if mask changed

				maskChanged = isMaskDifferent (pMask);
				if (maskChanged) {
					newHistNeeded = true;
				}
			}
//
			if (forceRegionParse || regionChanged || maskChanged) {
				cleanup();
//
				if (itsMaskPtr) {
					casacore::ImageRegion maskRegion(*itsMaskPtr);
					if (itsRegionPtr) {
						casacore::SubImage<T> subIm(*itsBaseImagePtr, maskRegion, false);
						pImage = new casacore::SubImage<T>(subIm, *itsRegionPtr, false);
					} else {
						pImage = new casacore::SubImage<T>(*itsBaseImagePtr, maskRegion, false);
					}
				} else {
					if (itsRegionPtr) {
						pImage = new casacore::SubImage<T>(*itsBaseImagePtr, *itsRegionPtr, false);
					}
				}
//
				if (itsDeleteMLPointer && itsMaskedLatticePtr) {
					itsMaskedLatticePtr.reset();
				}
				itsMaskedLatticePtr.reset();

// If pImage is now null, it means both region and mask are now unset
// so we return to the base image

				DisplayCoordinateSystem cSysOld = originalCoordinateSystem();
				if (pImage) {
					itsMaskedLatticePtr.reset(pImage);
					itsDeleteMLPointer = true;
					newHistNeeded = true;

// Transfer over the axis unit and velocity choices that
// might have been set by PADD

					DisplayCoordinateSystem cSys = pImage->coordinates();
					transferPreferences(cSys, cSysOld);
//
					setCoordinateSystem(cSys);
				} else {
					itsMaskedLatticePtr = itsBaseImagePtr;
					itsDeleteMLPointer = false;

// Transfer over the axis unit and velocity choices that
// might have been set by PADD

					DisplayCoordinateSystem cSys = itsBaseImagePtr->coordinates();
					transferPreferences(cSys, cSysOld);
//
					setCoordinateSystem(cSys);
				}

// Update other things

				updateLatticeStatistics();
				setAxes(displayAxes()[0], displayAxes()[1],
				        displayAxes()[2], fixedPosition());
				getMinAndMax();
				fillRecOut = true;

				// Request an update to the number of frames on the animator(s) where
				// this DD is registered.  (Note: this is _not_ a change to an 'Adjust'
				// gui, unlike most other uses of recOut).  The animator's current
				// frame number will remain unchanged if it is still within range;
				// otherwise it will be set to the first frame.

				if(!recOut.isDefined("setanimator")) {
					casacore::Record setanimrec;
					recOut.defineRecord("setanimator",setanimrec);
				}

// Set this so that coordinates are reset...

				setAttributeOnPrimaryWCHs(resetAtt);
				ret = true;
			}
		}
//

		if (fillRecOut) {

			casacore::Record trec = getOptions();

			// We change datamin/datamax (contained in "minmaxhist")  in "rec"
			// only if they do not pre-exist (if they do the user has specified
			// these values) If not, we stick in the updated min and max.

			if (!rec.isDefined(PrincipalAxesDD::HISTOGRAM_RANGE) &&
			        trec.isDefined(PrincipalAxesDD::HISTOGRAM_RANGE)) {

				casacore::Vector<casacore::Float> tempinsert(2);
				tempinsert(0) = datamin;
				tempinsert(1) = datamax;

				insertArray(rec, tempinsert, PrincipalAxesDD::HISTOGRAM_RANGE);
				insertArray(recOut, tempinsert, PrincipalAxesDD::HISTOGRAM_RANGE);

			}
		}

		//Check whether or not the flag telling us whether to calculate a
		//histogram has gone from true to false (eg window opened)


		if(rec.isDefined("alwaysupdate")) {
			casacore::Bool optCalc;

			rec.get("alwaysupdate", optCalc);

			if (optCalc && !calcHist) {
				calcHist = optCalc;

				if(!(pImage)) {
					if((itsRegionPtr) || (itsMaskPtr)) {
						if ((itsRegionPtr) && (itsMaskPtr)) {
							//Region and Mask set
							casacore::ImageRegion maskRegion(*itsMaskPtr);
							casacore::SubImage<T> subIm(*itsBaseImagePtr, maskRegion, false);
							pImage = new casacore::SubImage<T>(subIm, *itsRegionPtr, false);
						} else if (itsRegionPtr) {
							//Region only
							pImage = new casacore::SubImage<T>(*itsBaseImagePtr, *itsRegionPtr, false);
						} else {
							//Mask only
							casacore::ImageRegion maskRegion(*itsMaskPtr);
							pImage = new casacore::SubImage<T>(*itsBaseImagePtr, maskRegion, false);
						}
					}
				}
				newHistNeeded = true;

			} else {
				calcHist = optCalc;
			}
		}

		// New histogram needed?
		if(getOptions().isDefined(PrincipalAxesDD::HISTOGRAM_RANGE) && newHistNeeded && calcHist) {
			if (pImage) {
				if (updateHistogram(recOut, *pImage)) {
					newHistNeeded = false;
				} else {
					throw(casacore::AipsError("LatticePADD.cc - Error making new histogram data"
					                " - (from pImage)"));
				}
			} else if (itsBaseImagePtr) {
				if (updateHistogram(recOut, *itsBaseImagePtr)) {
					newHistNeeded = false;
				} else {
					throw(casacore::AipsError("LatticePADD.cc - Error making new histogram data"
					                " - (from baseImage)"));
				}

			} else if (itsBaseArrayPtr) {
				if (updateHistogram(recOut, itsBaseArrayPtr)) {
					newHistNeeded = false;
				} else {
					throw(casacore::AipsError("LatticePADD.cc - Error making new histogram data"
					                " - (from baseArray)"));
				}

			} else {
				throw(casacore::AipsError("LatticePADD.cc - Error making new histogram data"
				                " - (couldn't find anything to use!)"));
			}

		} else {
			if (recOut.isDefined(PrincipalAxesDD::HISTOGRAM_RANGE) &&
			        recOut.subRecord(PrincipalAxesDD::HISTOGRAM_RANGE).isDefined("newdata")) {
				casacore::Record tmphist = recOut.subRecord(PrincipalAxesDD::HISTOGRAM_RANGE);
				tmphist.define("newdata", false);
				tmphist.define("histarray", "unset");
				recOut.defineRecord(PrincipalAxesDD::HISTOGRAM_RANGE, tmphist);
			}

		}

		//After all that, check whether histogramgui window needs new statistices
		if (rec.isDefined("imagestats") && getOptions().isDefined(PrincipalAxesDD::HISTOGRAM_RANGE)) {
			casacore::Vector<casacore::String> whatToGet;
			rec.get("imagestats", whatToGet);

			casacore::Record addStats;
			if (recOut.isDefined(PrincipalAxesDD::HISTOGRAM_RANGE)) {
				addStats = recOut.subRecord(PrincipalAxesDD::HISTOGRAM_RANGE);
			} else {
				addStats = getOptions().subRecord(PrincipalAxesDD::HISTOGRAM_RANGE);
			}
//
			casacore::Record theStats;
			casacore::Array<casacore::Double> tempStats;
			for (casacore::uInt i=0; i<whatToGet.nelements(); i++) {
				if (whatToGet(i) == "mean") {
					itsLatticeStatisticsPtr->getStatistic(tempStats,casacore::LatticeStatsBase::MEAN);
					theStats.define("mean", tempStats);
				} else if (whatToGet(i) == "median") {
					itsLatticeStatisticsPtr->getStatistic(tempStats,casacore::LatticeStatsBase::MEDIAN);
					theStats.define("median", tempStats);
				} else if (whatToGet(i) == "stddev") {
					itsLatticeStatisticsPtr->getStatistic(tempStats,casacore::LatticeStatsBase::SIGMA);
					theStats.define("stddev", tempStats);
				}
			}
			theStats.define("new", true);
			addStats.defineRecord("stats", theStats);
			recOut.defineRecord(PrincipalAxesDD::HISTOGRAM_RANGE, addStats);
		} else {
			if (recOut.isDefined(PrincipalAxesDD::HISTOGRAM_RANGE)) {
				casacore::Record temp = recOut.subRecord(PrincipalAxesDD::HISTOGRAM_RANGE);
				if (temp.isDefined("stats")) {
					casacore::Record clear = temp.subRecord("stats");
					clear.define("new", false);
					temp.defineRecord("stats", clear);
					recOut.defineRecord(PrincipalAxesDD::HISTOGRAM_RANGE, temp);
				}
			}
		}


		// Set Beam ellipse user interface parameters, if applicable.

		if( beams_.size( ) > 0 ) {
			casacore::Bool  needsRefresh = beamOnOff_->fromRecord(rec);
			needsRefresh = beamStyle_->fromRecord(rec)     || needsRefresh;
			needsRefresh = beamColor_->fromRecord(rec)     || needsRefresh;
			needsRefresh = beamLineWidth_->fromRecord(rec) || needsRefresh;
			needsRefresh = beamXCenter_->fromRecord(rec)   || needsRefresh;
			needsRefresh = beamYCenter_->fromRecord(rec)   || needsRefresh;
			ret = ret                                      || needsRefresh;
		}


		return ret;
	}



	template <class T>
	casacore::Record LatticePADisplayData<T>::getHist() const {
		if(imageHistogram.isDefined("values")) {
			return imageHistogram;
		} else {
			casacore::Record unset;
			unset.define("unset", "unset");
			return unset;
		}
	}

	template <class T>
	casacore::Record LatticePADisplayData<T>::getOptions( bool scrub ) const {

		casacore::Record rec = PrincipalAxesDD::getOptions(scrub);

// Some of these widgets are not appropriate to the
// LatticeAsVector DD (which has a casacore::Complex data source)

		if (className() != casacore::String("LatticeAsVector")) {
			casacore::Record resample;
			resample.define("dlformat", "resample");
			resample.define("listname", "resampling mode");
			resample.define("ptype", "choice");
			casacore::Vector<casacore::String> vresample(3);
			vresample(0) = "nearest";
			vresample(1) = "bilinear";
			vresample(2) = "bicubic";
			resample.define("popt", vresample);
			resample.define("default", "nearest");
			resample.define("value", itsResample);
			resample.define("allowunset", false);
			rec.defineRecord("resample", resample);
//
			T typetester;
			casacore::DataType dtype = casacore::whatType(&typetester);
			if ((dtype == casacore::TpComplex) || (dtype == casacore::TpDComplex)) {
				casacore::Record complexmode;
				complexmode.define("dlformat", "complexmode");
				complexmode.define("listname", "Complex mode");
				complexmode.define("ptype", "choice");
				casacore::Vector<casacore::String> vcomplexmode(4);
				vcomplexmode(0) = "magnitude";
				vcomplexmode(1) = "phase";
				vcomplexmode(2) = "real";
				vcomplexmode(3) = "imaginary";
				complexmode.define("popt", vcomplexmode);
				complexmode.define("default", "magnitude");
				complexmode.define("value", itsComplexMode);
				complexmode.define("allowunset", false);
				rec.defineRecord("complexmode", complexmode);
			}
		}
//
		if (itsBaseImagePtr && ! scrub) {
			casacore::Record region;
			region.define("dlformat", "region");
			region.define("listname", "Image region");
			region.define("ptype", "region");
			casacore::Record unset;
			unset.define("i_am_unset", "i_am_unset");
			region.defineRecord("default", unset);
			region.defineRecord("value", unset);
			region.define("allowunset", true);
			rec.defineRecord("region", region);
//
			casacore::Record mask;
			mask.define("dlformat", "mask");
			mask.define("listname", "Mask expression");
			mask.define("ptype", "string");
			mask.defineRecord("default", unset);
			mask.defineRecord("value", unset);
			mask.define("allowunset", true);
			rec.defineRecord("mask", mask);
		}


		// Send beam ellipse user interface parameters, if applicable.

		if( beams_.size( ) > 0 ) {
			beamOnOff_->toRecord(rec);
			beamStyle_->toRecord(rec);
			beamColor_->toRecord(rec);
			beamLineWidth_->toRecord(rec);
			beamXCenter_->toRecord(rec);
			beamYCenter_->toRecord(rec);
		}


		return rec;
	}

// update the stored minimum and maximum data values (casacore::Float version)
	template<class T>
	void LatticePADisplayData<T>::getMinAndMax() {
		// sanity check
		if (!itsMaskedLatticePtr || !itsLatticeStatisticsPtr) {
			throw(casacore::AipsError("LatticePADisplayData<T>::getMinAndMax - "
			                "no lattice is available"));
			return;
		}
//
		casacore::Float dMin, dMax;
		if (!itsLatticeStatisticsPtr->getFullMinMax(dMin, dMax)) {
			datamin = -1.0;
			datamax = 1.0;
		} else {
			datamin = dMin;
			datamax = dMax;
		}
	}


	template<class T>
	casacore::WCLELMask* LatticePADisplayData<T>::makeMask (const casacore::RecordInterface& mask) {
		casacore::LogIO os(casacore::LogOrigin("LatticePADisplayData", "makeRegion", WHERE));
		casacore::WCLELMask* maskPtr = 0;
		if (mask.dataType("mask") == casacore::TpRecord) {
			casacore::Record rec = mask.asRecord("mask");
			if (rec.isDefined("i_am_unset")) {
			} else {
				os << casacore::LogIO::SEVERE << "Mask is illegal record" << casacore::LogIO::POST;
			}
		} else if (mask.dataType("mask") == casacore::TpString) {
			maskPtr = new casacore::WCLELMask(mask.asString("mask"));
			if (!maskPtr) {
				os << "Failed to create WCLELMask from mask String" << casacore::LogIO::POST;
			}
		} else {
			os << casacore::LogIO::SEVERE << "Mask is illegal record type" << casacore::LogIO::POST;
		}
//
		return maskPtr;
	}


	template<class T>
	casacore::ImageRegion* LatticePADisplayData<T>::makeRegion (const casacore::RecordInterface& region) {
		casacore::LogIO os(casacore::LogOrigin("LatticePADisplayData", "makeRegion", WHERE));
		casacore::ImageRegion* regionPtr = 0;
		if (region.dataType("region") == casacore::TpRecord) {
			casacore::Record rec = region.asRecord("region");
			if (rec.isDefined("i_am_unset")) {
//
			} else {
				regionPtr = casacore::ImageRegion::fromRecord(rec, casacore::String(""));
				if (!regionPtr) {
					os << casacore::LogIO::NORMAL << "Failed to create ImageRegion from region record" << casacore::LogIO::POST;
				}
			}
		} else {
			os << casacore::LogIO::SEVERE << "Region is illegal record type" << casacore::LogIO::POST;
		}
//
		return regionPtr;
	}



	template<class T>
	casacore::Bool LatticePADisplayData<T>::isRegionDifferent (casacore::ImageRegion*& pRegion) {
		casacore::Bool same = false;
		if (!pRegion) {
			if (!itsRegionPtr) same = true;
		} else {
			if (itsRegionPtr) {
				if (*itsRegionPtr==*pRegion) same = true;
			}
		}
//
		if (same) {
			delete pRegion;
			pRegion = 0;
		} else {
			if (itsRegionPtr) delete itsRegionPtr;
			itsRegionPtr = pRegion;
		}
//
		return !same;
	}

	template<class T>
	casacore::Bool LatticePADisplayData<T>::isMaskDifferent (casacore::WCLELMask*& pMask) {
		casacore::Bool same = false;
		if (!pMask) {
			if (!itsMaskPtr) same = true;
		} else {
			if (itsMaskPtr) {
				if (*itsMaskPtr==*pMask) same = true;
			}
		}
//
		if (same) {
			delete pMask;
			pMask = 0;
		} else {
			if (itsMaskPtr) delete itsMaskPtr;
			itsMaskPtr = pMask;
		}
//
		return !same;
	}



	template<class T>
	casacore::Bool LatticePADisplayData<T>::insertFloat(casacore::Record& into, casacore::Float from, const casacore::String field) {
		casacore::Record tempSub;
		if (!into.isDefined(field)) {
			tempSub = getOptions().subRecord(field);
			tempSub.define("value", from);
			into.defineRecord(field, tempSub);
		} else {
			if (into.dataType(field) == casacore::TpRecord) {
				tempSub = into.subRecord(field);
				tempSub.define("value", from);
				into.defineRecord(field, tempSub);
			} else {
				into.removeField(field);
				tempSub = into.subRecord(field);
				tempSub.define("value", from);
				into.defineRecord(field, tempSub);
			}
		}

		return true;
	}

	template<class T>
	casacore::Bool LatticePADisplayData<T>::insertArray(casacore::Record& into, casacore::Vector<casacore::Float> from, const casacore::String field) {
		casacore::Record tempSub;
		if (!into.isDefined(field)) {
			tempSub = getOptions().subRecord(field);
			tempSub.define("value", from);
			into.defineRecord(field, tempSub);
		} else {
			if (into.dataType(field) == casacore::TpRecord) {
				tempSub = into.subRecord(field);
				tempSub.define("value", from);
				into.defineRecord(field, tempSub);
			} else {
				into.removeField(field);
				tempSub = into.subRecord(field);
				tempSub.define("value", from);
				into.defineRecord(field, tempSub);
			}
		}
		return true;

	}

	template<class T>
	casacore::Bool LatticePADisplayData<T>::transferPreferences (DisplayCoordinateSystem& cSysInOut,
	        const DisplayCoordinateSystem& cSysIn) const {
		if (cSysIn.nCoordinates()!=cSysInOut.nCoordinates()) return false;
		if (cSysIn.nWorldAxes()!=cSysInOut.nWorldAxes()) return false;
		if (cSysIn.nPixelAxes()!=cSysInOut.nPixelAxes()) return false;
//
		casacore::Int after = -1;
		casacore::Int cIn = cSysIn.findCoordinate (casacore::Coordinate::SPECTRAL, after);
		after = -1;
		casacore::Int cInOut = cSysInOut.findCoordinate (casacore::Coordinate::SPECTRAL, after);
//
		if (cIn!=-1 && cInOut!=-1 && cIn==cInOut) {
			const casacore::SpectralCoordinate scIn = cSysIn.spectralCoordinate(cIn);
			const casacore::SpectralCoordinate scInOut = cSysInOut.spectralCoordinate(cInOut);
			casacore::SpectralCoordinate scInOut2(scInOut);
//
			casacore::MDoppler::Types velDoppler = scIn.velocityDoppler ();
			casacore::String velUnit = scIn.velocityUnit();
			scInOut2.setVelocity (velUnit, velDoppler);
//
			casacore::String formatUnit = scIn.formatUnit();
			scInOut2.setFormatUnit(formatUnit);
//
			cSysInOut.replaceCoordinate(scInOut2, cInOut);
		}
		return true;
	}

	template<class T>
	void LatticePADisplayData<T>::delTmpImage() {
		if (itsBaseImagePtr) {
			casacore::String tmpImage(itsBaseImagePtr->name(false));
			delTmpData(tmpImage);
		}
	}

	template<class T>
	casacore::Bool LatticePADisplayData<T>::useStriding(
	    const casacore::IPosition& shape, casacore::IPosition& stride,
	    casacore::uInt maxPixels, casacore::uInt minPerAxis) {
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
		//
		// The idea is to sample using no more than maxPixels elements from the
		// original casacore::Lattice or Array.  Histograms needn't be more accurate for
		// their purpose (which is to set color scaling).


		maxPixels = max(1u, maxPixels);
		minPerAxis = max(1u, minPerAxis);
		// (safety: insure against improper input parameter use).

		casacore::uInt nAxes = shape.nelements();

		stride.resize(nAxes);
		stride=1;	// Initial stride on all axes.

		casacore::uInt latticeSize = 1;
		for(casacore::uInt axis=0; axis<nAxes; axis++) latticeSize *= shape[axis];

		if(latticeSize <= maxPixels) return false;
		// No striding needed.


		casacore::Double reduceFctr = casacore::Double(latticeSize)/maxPixels;
		// We want striding to reduce the data examined by at least
		// this factor.  (reduceFctr > 1).

		// Strided sampling would be poor on the casacore::Stokes axis, or even
		// on a frequency axis.  This code makes a lame attempt at
		// avoiding this, by assuming that sky coordinates are on the
		// first two axes, doing strided sampling there only, if possible.
		// In no case, however, is a stride greater than 1 set for an axis
		// which would cause fewer than minPerAxis elements to be used on
		// that axis.

		// After determining a stride for the shorter of the first two axes,
		// other axes will be given strides (up to their maximum) in axisOrder,
		// until reduceFctr (or maximum striding on all axes) is reached.
		// The longer of the first two axes will be the next one strided,
		// after the shortest; then the rest, as necessary.

		casacore::Int shortAxis=0, longAxis=1;
		casacore::IPosition axisOrder(nAxes);
		for (casacore::uInt i=1; i<nAxes; i++) axisOrder[i]=i;

		if(nAxes>1 && shape[1]<shape[0])  {
			shortAxis = 1;
			axisOrder[1] = longAxis = 0;
		}

		casacore::Int shortMaxStride = max(1u, shape[shortAxis]/minPerAxis);
		casacore::Int longMaxStride  = max(1u, shape[longAxis]/minPerAxis);
		// maximum usable stride on short, long axes.

		casacore::Int sqrtStride = casacore::Int(ceil(sqrt(reduceFctr)));
		// This stride on first two axes, if usable, would achieve
		// the needed reduceFctr...

		// ...We may even get away with one less on the short axis...
		casacore::Int shortStride = sqrtStride-1;
		if(shortStride*min(sqrtStride,longMaxStride) < reduceFctr) shortStride++;
		// (No, not enough: use full sqrtStride, if possible...).

		stride[shortAxis] = min(shortMaxStride, shortStride);
		// ...but no more than shortMaxStride, in any case.


		reduceFctr /= stride[shortAxis];
		// remaining reduction factor to be achieved.
		// (slightly inaccurate, but not enough to matter...).

		// Compute stride on remaining axes (starting with the
		// longest of the first two).

		for (casacore::uInt i=1; i<nAxes; i++) {

			if(reduceFctr<=1.) break;	// reduceFctr achieved -- done.

			casacore::Int strideAxis = axisOrder[i];
			// Next axis -- the one to stride now.
			casacore::Int maxStride = max(1u, shape[strideAxis]/minPerAxis);
			// Its maximum stride.
			stride[strideAxis] = min(maxStride, casacore::Int(ceil(reduceFctr)));
			// stride to use on this axis.
			reduceFctr /= stride[strideAxis];
		}
		// reduction factor still to be achieved on remaining
		// axes, in possible.

		return true;
	}



	template<class T>
	void LatticePADisplayData<T>::SetUpBeamData_() {
		// Called by constructors: set up data for beam drawing, if applicable.

		if(! itsBaseImagePtr) return;

		viewer::ImageProperties info;
		try {
			// implicit constructor
			info = viewer::ImageProperties(itsBaseImagePtr);
		} catch(...) { }
		beams_ = info.restoringBeams( );


		if ( beams_.size( ) > 0 ) {

			majorunit_ = "arcsec";
			minorunit_ = "arcsec";
			paunit_    = "deg";


			// Set up parameters for user interface.

			casacore::Vector<casacore::String> yesNo(2);
			yesNo[0]="Yes";
			yesNo[1]="No";

			casacore::Vector<casacore::String> outlnFill(2);
			outlnFill[0]="Outline";
			outlnFill[1]="Filled";
			casacore::Vector<casacore::String> bmClr(11);
			bmClr(0) = "foreground";
			bmClr(1) = "background";
			bmClr(2) = "black";
			bmClr(3) = "white";
			bmClr(4) = "red";
			bmClr(5) = "green";
			bmClr(6) = "blue";
			bmClr(7) = "cyan";
			bmClr(8) = "magenta";
			bmClr(9) = "yellow";
			bmClr(10) = "gray";


			beamOnOff_ = new DParameterChoice("beam", "draw beam?",
			                                  "Should the image's restoring beam ellipse be drawn?",
			                                  yesNo, yesNo[0], yesNo[0], "beam_ellipse");

			beamStyle_ = new DParameterChoice("beamoutline", "beam style",
			                                  "whether the ellipse will be drawn in outline or filled",
			                                  outlnFill, outlnFill[0], outlnFill[0], "beam_ellipse");

			beamColor_ = new DParameterChoice("beamcolor", "color",
			                                  "beam ellipse's color",
			                                  bmClr, bmClr[0], bmClr[0], "beam_ellipse");

			beamLineWidth_ = new DParameterRange<casacore::Int>("beamlinewidth", "line width",
			        "width of the line used to draw the ellipse",
			        1, 7,  1,  1, 1, "beam_ellipse");

			beamXCenter_ = new DParameterRange<casacore::Float>("beamxcenter", "x center",
			        "relative horizontal position of the beam's center within\n"
			        "the image drawing area:  0 = left edge, 1 = right edge",
			        0., 1.,  .02,  .1, .1, "beam_ellipse");

			beamYCenter_ = new DParameterRange<casacore::Float>("beamycenter", "y center",
			        "relative vertical position of the beam's center within\n"
			        "the image drawing area:  0 = bottom edge, 1 = top edge",
			        0., 1.,  .02,  .1, .1, "beam_ellipse");

		}

	}


	template<class T>
	void LatticePADisplayData<T>::drawBeamEllipse_(WorldCanvas* wc) {
		// Will draw the beam ellipse if applicable (i.e., the LatticePADD
		// has an image with beam data, beam drawing is turned on, and the
		// WC's casacore::CoordinateSystem is set to sky coordinates).

		if( beams_.size( ) == 0 || beamOnOff_->value()=="No" ) return;

		vector<double> beam = (casacore::Int) beams_.size( ) > activeZIndex_ ? beams_[activeZIndex_] : beams_[0];
		// The major and minor axes must both be positive
		if ( beam[0] <= 0.0 || beam[1] <= 0.0 ) return;

		wc->setColor(beamColor_->value());
		wc->setLineWidth(beamLineWidth_->value());

		wc->drawBeamEllipse( beam[0], beam[1],  beam[2],
		                     majorunit_, minorunit_,  paunit_,
		                     beamXCenter_->value(),   beamYCenter_->value(),
		                     (beamStyle_->value()=="Outline") );
	}




} //# NAMESPACE CASA - END
