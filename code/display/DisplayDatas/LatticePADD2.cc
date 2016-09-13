//# LatticePADD2.cc: explicit templates for LatticePADD class
//# Copyright (C) 1999,2000,2001,2002,2003,2004
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
#include <casa/sstream.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Arrays/IPosition.h>
#include <lattices/Lattices/Lattice.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <lattices/LEL/LatticeExpr.h>
#include <lattices/LEL/LatticeExprNode.h>
#include <lattices/LatticeMath/LatticeStatistics.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <lattices/Lattices/SubLattice.h>
#include <display/DisplayDatas/LatticePADD.h>
#include <lattices/LatticeMath/LatticeHistograms.h>
#include <lattices/Lattices/MaskedLattice.h>
#include <images/Images/SubImage.h>
#include <images/Images/ImageInterface.h>

namespace casa { //# NAMESPACE CASA - BEGIN



// Format a string containing the data value and its units at the
// given position (Float and Complex versions).
	template <>
	String LatticePADisplayData<Float>::showValue(const Vector<Double>& world) {
		//if(!conformed()) return "";
		// (kludge: showValue() and showPosition() should also have
		// wch passed in, and call conformsTo(wch) instead...).
		Vector<Double> fullWorld, fullPixel;

		String retval;
		if (!getFullCoord(fullWorld, fullPixel, world)) {
			retval = "invalid";
			return retval;
		}
		Int length = fullPixel.shape()(0);
		IPosition ipos(length);
		for (Int i = 0; i < length; i++) {
			ipos(i) = Int(fullPixel(i) + 0.5);
			if ( (ipos(i) < 0) || (ipos(i) >= dataShape()(i)) ) {
				retval = "invalid";
				return retval;
			}
		}
		if (!maskValue(ipos)) {
			retval = "masked";
			return retval;
		}
		ostringstream oss;
		if(itsNotation == Coordinate::SCIENTIFIC) {
			oss.setf(ios::scientific, ios::floatfield);
			oss.precision(3);
		} else if(itsNotation == Coordinate::FIXED) {
			oss.setf(ios::fixed, ios::floatfield);
		} else if(itsNotation == Coordinate::DEFAULT) { // flexible notation
			oss.precision(4);
		}
		oss.setf(ios::showpos);
		Quantum<Float> qtm(dataValue(ipos), dataUnit());
		qtm.print(oss);
		retval = String(oss);
		return retval;
	}
	template <>
	String LatticePADisplayData<Complex>::showValue(const Vector<Double>& world) {
		//if(!conformed()) return "";
		// (kludge: showValue() and showPosition() should also have
		// wch passed in, and call conformsTo(wch) instead...).
		Vector<Double> fullWorld, fullPixel;
		String retval;
		if (!getFullCoord(fullWorld, fullPixel, world)) {
			retval = "invalid";
			return retval;
		}

		Int length = fullPixel.shape()(0);
		IPosition ipos(length);
		for (Int i = 0; i < length; i++) {
			ipos(i) = Int(fullPixel(i) + 0.5);
			if ( (ipos(i) < 0) || (ipos(i) >= dataShape()(i)) ) {
				retval = "invalid";
				return retval;
			}
		}

		if (!maskValue(ipos)) {
			retval = "masked";
			return retval;
		}

		ostringstream oss;
		if(itsNotation == Coordinate::SCIENTIFIC) {
			oss.setf(ios::scientific, ios::floatfield);
			oss.precision(3);
		} else if(itsNotation == Coordinate::FIXED) {
			oss.setf(ios::fixed, ios::floatfield);
		} else if(itsNotation == Coordinate::DEFAULT) { // flexible notation
			oss.precision(4);
		}
		oss.setf(ios::showpos);
		Quantum<Complex> qtm(dataValue(ipos), dataUnit());
		qtm.print(oss);
		retval = String(oss);
		return retval;
	}

// Update the private store for the statistics calculator (Float and
// Complex versions).
	template <> void LatticePADisplayData<Float>::updateLatticeStatistics() {
		if (itsLatticeStatisticsPtr) {
			delete itsLatticeStatisticsPtr;
			itsLatticeStatisticsPtr = 0;
		}
		itsLatticeStatisticsPtr =
		    new LatticeStatistics<Float>(*itsMaskedLatticePtr, False);
	}
	template <> void LatticePADisplayData<Complex>::updateLatticeStatistics() {



		if (itsLatticeStatisticsPtr) {
			delete itsLatticeStatisticsPtr;
			itsLatticeStatisticsPtr = 0;
		}

		switch (itsComplexToRealMethod) {
		case Display::Phase:
			itsLatticeStatisticsPtr = new LatticeStatistics<Float>
			(LatticeExpr<Float>(arg(*itsMaskedLatticePtr)), False);
			break;
		case Display::Real:
			itsLatticeStatisticsPtr = new LatticeStatistics<Float>
			(LatticeExpr<Float>(real(*itsMaskedLatticePtr)), False);
			break;
		case Display::Imaginary:
			itsLatticeStatisticsPtr = new LatticeStatistics<Float>
			(LatticeExpr<Float>(imag(*itsMaskedLatticePtr)), False);
			break;
		default:
			itsLatticeStatisticsPtr = new LatticeStatistics<Float>
			(LatticeExpr<Float>(abs(*itsMaskedLatticePtr)), False);
			break;
		}
	}



	template<>
	Bool LatticePADisplayData<Float>::updateHistogram(Record &rec,
	        ImageInterface<Float> &pImage) {

		if (!calcHist) return False;

		ImageInterface<Float>* histImage = &pImage;

		// Compute the histogram on a strided subset of pImage when necessary
		// for efficiency.  The histogram needn't be more accurate for
		// its purpose (which is to set color scaling).

		IPosition stride;
		Bool stridingNeeded = useStriding(pImage.shape(), stride);

		uInt nAxes = pImage.ndim();

		if(stridingNeeded) {
			IPosition start(nAxes, 0);
			IPosition end  (nAxes, Slicer::MimicSource);
			Slicer histSlicer(start, end, stride);

			histImage = new SubImage<Float>(pImage, histSlicer);
		}



		LatticeHistograms<Float>* histogram =
		    new LatticeHistograms<Float>(*histImage, False);

		histogram->setNBins(1000);

		Vector<Float> vals;
		Vector<Float> counts;

		Bool histOK = histogram->getHistograms(vals,counts);

		/*
		  //#dk: (This is unnecessary, since no scale is shown for the counts;
		  // it just makes the logarithmic histogram more ugly...)

		  if(stridingNeeded) {
		    // Multiply the counts by the factor by which the SubImage was reduced.
		    uInt reductionFactor = 1;
		    for(uInt axis=0; axis<nAxes; axis++) reductionFactor *= stride[axis];
		    for(uInt i=0; i<counts.nelements(); i++) counts[i] *= reductionFactor;  }
		*/

		if(histOK) {

			Record newHist;
			newHist.define("values", vals);
			newHist.define("counts", counts);
			imageHistogram = newHist;

			Record histSubRecord;
			if (rec.isDefined(PrincipalAxesDD::HISTOGRAM_RANGE)) {
				histSubRecord = rec.subRecord(PrincipalAxesDD::HISTOGRAM_RANGE);
			} else histSubRecord = getOptions().subRecord(PrincipalAxesDD::HISTOGRAM_RANGE);

			histSubRecord.define("newdata", True);	// alerts autogui
			histSubRecord.defineRecord("histarray", newHist);
			// Append new hist array
			rec.defineRecord(PrincipalAxesDD::HISTOGRAM_RANGE, histSubRecord);
		}

		delete histogram;
		if (stridingNeeded) delete histImage;

		return histOK;
	}



	template <>
	Bool LatticePADisplayData<Complex>::updateHistogram(Record &rec,
	        const Array<Complex>* theArray) {

		Bool error = False;

		if (calcHist) {
			LatticeHistograms<Float>* histP = 0;

			ArrayLattice<Complex> tempAL(*theArray);
			SubLattice<Complex> tempSL(tempAL);

			switch (itsComplexToRealMethod) {
			case Display::Phase:
				histP = new LatticeHistograms<Float> (LatticeExpr<Float>(arg(tempSL)), False);
				break;
			case Display::Real:
				histP = new LatticeHistograms<Float> (LatticeExpr<Float>(real(tempSL)), False);
				break;
			case Display::Imaginary:
				histP = new LatticeHistograms<Float> (LatticeExpr<Float>(imag(tempSL)), False);
				break;
			default:
				try {
					histP = new LatticeHistograms<Float> (LatticeExpr<Float>(abs(tempSL)), False);
				} catch (...) {
					error = True;
				}
				break;

			}
			if (!error) {
				Record histSubRecord;
				if(rec.isDefined(PrincipalAxesDD::HISTOGRAM_RANGE)) {
					histSubRecord = rec.subRecord(PrincipalAxesDD::HISTOGRAM_RANGE);     // Record to append to
				} else {
					histSubRecord = getOptions().subRecord(PrincipalAxesDD::HISTOGRAM_RANGE);
				}

				histP->setNBins(1000);
				Vector<Float> vals;
				Vector<Float> counts;

				if (histP->getHistograms(vals,counts)) {
					Record newHist;
					newHist.define("values", vals);
					newHist.define("counts", counts);
					imageHistogram = newHist;

					histSubRecord.define("newdata", True);       // Flag for autogui
					histSubRecord.defineRecord("histarray", newHist); // Append new hist array
					rec.defineRecord(PrincipalAxesDD::HISTOGRAM_RANGE, histSubRecord);
					// clean up pointer !!!
					delete histP;
					histP=0;
					return True;
				} else {
					// clean up pointer !!!
					delete histP;
					histP=0;
					return False;   // Oh-ow.
				}
			} else {
				// clean up pointer !!!
				delete histP;
				histP=0;
				return False;
			}
		}
		return False;
	}

	template <>
	Bool LatticePADisplayData<Float>::updateHistogram(Record &rec, const Array<Float>* theArray) {

		if (calcHist) {
			ArrayLattice<Float> tempAL(*theArray);
			SubLattice<Float> tempSL(tempAL);
			Record histSubRecord;

			if (rec.isDefined(PrincipalAxesDD::HISTOGRAM_RANGE)) {
				histSubRecord = rec.subRecord(PrincipalAxesDD::HISTOGRAM_RANGE);     // Record to append to
			} else {
				histSubRecord = getOptions().subRecord(PrincipalAxesDD::HISTOGRAM_RANGE);
			}


			LatticeHistograms<Float> newHistogram(tempSL, False);

			newHistogram.setNBins(1000);
			Vector<Float> vals;
			Vector<Float> counts;

			if (newHistogram.getHistograms(vals,counts)) {
				Record newHist;
				newHist.define("values", vals);
				newHist.define("counts", counts);
				imageHistogram = newHist;

				histSubRecord.define("newdata", True);  // Flag for autogui
				histSubRecord.defineRecord("histarray", newHist);  // Append new hist array
				rec.defineRecord(PrincipalAxesDD::HISTOGRAM_RANGE, histSubRecord);
				return True;
			} else {
				return False;   // Oh-ow.
			}
		} else {
			return False;
		}

	}



	template<>
	Bool LatticePADisplayData<Complex>::updateHistogram(Record &rec,
	        MaskedLattice<Complex> &pImage) {
		if(!calcHist) return False;

		MaskedLattice<Float>* floatLattice = 0;

		switch (itsComplexToRealMethod) {
		case Display::Phase:
			floatLattice = new LatticeExpr<Float>(arg(pImage));
			break;
		case Display::Real:
			floatLattice = new LatticeExpr<Float>(real(pImage));
			break;
		case Display::Imaginary:
			floatLattice = new LatticeExpr<Float>(imag(pImage));
			break;
		default:
			floatLattice = new LatticeExpr<Float>(abs(pImage));
		}

		MaskedLattice<Float>* histLattice = floatLattice;

		// Compute the histogram on a strided subset of floatLattice when
		// necessary for efficiency.  The histogram needn't be more accurate
		// for its purpose (which is to set color scaling).

		IPosition stride;
		Bool stridingNeeded = useStriding(pImage.shape(), stride);

		uInt nAxes = pImage.ndim();

		if(stridingNeeded) {
			IPosition start(nAxes, 0);
			IPosition end  (nAxes, Slicer::MimicSource);
			Slicer histSlicer(start, end, stride);

			histLattice = new SubLattice<Float>(*floatLattice, histSlicer);
		}


		LatticeHistograms<Float>* hist =
		    new LatticeHistograms<Float>(*histLattice, False);


		hist->setNBins(1000);
		Vector<Float> vals;
		Vector<Float> counts;

		Bool histOK = hist->getHistograms(vals, counts);

		if(histOK) {
			Record newHist;
			newHist.define("values", vals);
			newHist.define("counts", counts);
			imageHistogram = newHist;

			Record histSubRecord;
			if(rec.isDefined(PrincipalAxesDD::HISTOGRAM_RANGE)) {
				histSubRecord = rec.subRecord(PrincipalAxesDD::HISTOGRAM_RANGE);
			} else histSubRecord = getOptions().subRecord(PrincipalAxesDD::HISTOGRAM_RANGE);

			histSubRecord.define("newdata", True);	// alerts autogui
			histSubRecord.defineRecord("histarray", newHist);
			// Append new hist array

			rec.defineRecord(PrincipalAxesDD::HISTOGRAM_RANGE, histSubRecord);
		}

		delete hist;
		if(stridingNeeded) delete histLattice;
		delete floatLattice;

		return histOK;
	}

	template <>
	SHARED_PTR<ImageInterface<Float> > LatticePADisplayData<Complex>::imageinterface( ) {
		return SHARED_PTR<ImageInterface<Float> >();
	}

	template <>
	SHARED_PTR<ImageInterface<Float> > LatticePADisplayData<Float>::imageinterface( ) {
		return itsBaseImagePtr;
	}


} //# NAMESPACE CASA - END

