//# LatticeAsContour.cc: Class to display lattice objects as contoured images
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002
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
#include <casa/System/Aipsrc.h>
#include <casa/System/AipsrcValue.h>
#include <casa/Arrays/Array.h>
#include <casa/Containers/Record.h>
#include <lattices/Lattices/Lattice.h>
#include <images/Images/ImageInterface.h>
#include <display/DisplayDatas/LatticePADMContour.h>
#include <display/DisplayDatas/LatticeAsContour.h>
#include <casa/BasicMath/Math.h>
#include <casa/iostream.h>
#include <casa/sstream.h>
#include <casa/iomanip.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// >2d array-based ctor
	template <class T>
	LatticeAsContour<T>::LatticeAsContour(Array<T> *array, const uInt xAxis,
	                                      const uInt yAxis, const uInt mAxis,
	                                      const IPosition fixedPos) :
		LatticePADisplayData<T>(array, xAxis, yAxis, mAxis, fixedPos),
		itsBaseContour(0), itsUnitContour(0) {
		constructParameters_();
		setupElements();
		setDefaultOptions();
	}

// 2d array-based ctor
	template <class T>
	LatticeAsContour<T>::LatticeAsContour(Array<T> *array, const uInt xAxis,
	                                      const uInt yAxis) :
		LatticePADisplayData<T>(array, xAxis, yAxis),
		itsBaseContour(0), itsUnitContour(0) {
		constructParameters_();
		setupElements();
		setDefaultOptions();
	}

// >2d image-based ctor
	template <class T>
	LatticeAsContour<T>::LatticeAsContour(
		SHARED_PTR<ImageInterface<T> > image, const uInt xAxis,
		const uInt yAxis, const uInt mAxis,const IPosition fixedPos, viewer::StatusSink * /*sink*/
	) :
		LatticePADisplayData<T>(image, xAxis, yAxis, mAxis, fixedPos),
		itsBaseContour(0), itsUnitContour(0) {
		constructParameters_();
		setupElements();
		setDefaultOptions();
	}

// 2d image-based ctor
	template <class T>
	LatticeAsContour<T>::LatticeAsContour(SHARED_PTR<ImageInterface<T> > image,
	                                      const uInt xAxis, const uInt yAxis) :
		LatticePADisplayData<T>(image, xAxis, yAxis),
		itsBaseContour(0), itsUnitContour(0) {
		constructParameters_();
		setupElements();
		setDefaultOptions();
	}

	template <class T>
	LatticeAsContour<T>::~LatticeAsContour() {
		for (uInt i = 0; i < nelements(); i++) {
			delete ((LatticePADMContour<T> *)DDelement[i]);
		}
		if(itsBaseContour!=0) {
			delete itsBaseContour;
			itsBaseContour=0;
		}
		if(itsUnitContour!=0) {
			delete itsUnitContour;
			itsUnitContour=0;
		}
	}

// Oke, here we setup the elements using LatticePADMContour
	template <class T>
	void LatticeAsContour<T>::setupElements() {

		for (uInt i=0; i<nelements(); i++) if(DDelement[i]!=0) {
				delete static_cast<LatticePADMContour<T>*>(DDelement[i]);
				DDelement[i]=0;
			}
		// Delete old DMs, if any.

		IPosition fixedPos = fixedPosition();
		Vector<Int> dispAxes = displayAxes();
		if (nPixelAxes > 2) {
			setNumImages(dataShape()(dispAxes(2)));
			DDelement.resize(nelements());
			for (uInt index = 0; index < nelements(); index++) {
				fixedPos(dispAxes(2)) = index;
				DDelement[index] = (LatticePADisplayMethod<T> *)new
				                   LatticePADMContour<T>(dispAxes(0), dispAxes(1), dispAxes(2),
				                           fixedPos, this);
			}
		} else {
			setNumImages(1);
			DDelement.resize(nelements());
			DDelement[0] = (LatticePADisplayMethod<T> *)new
			               LatticePADMContour<T>(dispAxes(0), dispAxes(1), this);
		}
		PrincipalAxesDD::setupElements();
	}

	template <class T>
	void LatticeAsContour<T>::setDefaultOptions() {
		LatticePADisplayData<T>::setDefaultOptions();
		Record rec, recOut;
		rec.define("resample", "bilinear");
		LatticePADisplayData<T>::setOptions(rec, recOut);
		getMinAndMax();	// (actually, this _computes_ and sets min and max.)

		setStdContourLimits_();

		AipsrcValue<Float>::find(itsLine,"display.contour.linewidth",0.5f);
		itsDashNeg = True;
		itsDashPos = False;
		Aipsrc::find(itsColor,"display.contour.color","foreground");
	}

	template <class T>
	void LatticeAsContour<T>::constructParameters_() {
		// Construct user option DisplayParameters (for min/max contour.)
		// (To be used by constructors only.)

		if(itsBaseContour!=0) return;	// (already done).

		itsBaseContour = new DParameterRange<Float>("basecontour",
		        "Base Contour Level",
		        "Actual contour level corresping to\n"
		        "a '0' in the 'Relative Levels' textbox.",
		        0., 1.,  .001,  0., 0.,	// (set for real below).
		        "", True, True);

		itsUnitContour = new DParameterRange<Float>("unitcontour",
		        "Unit Contour Level",
		        "Actual contour level corresping to to a '1' in\n"
		        "the 'Relative Levels' textbox.",
		        0., 1.,  .001,  1., 1.,	// (set for real below).
		        "", True, True);
	}


	template <class T>
	void LatticeAsContour<T>::setStdContourLimits_(Record* recOut) {
		// Set standard limits/values for contour sliders and relative levels.
		// If recOut is provided, the defaults will be set onto it in the
		// standard form used for updating the gui during/after setOptions().

		// These heuristics could probably be improved.  This latest version
		// is based on scientific input from Steve Myers.

		Float dmin=getDataMin(), dmax=getDataMax();
		Float absmax = max(abs(dmin), abs(dmax));


		// Compute default values, limits and resolution for sliders.

		Float minLimit=dmin, maxLimit=dmax,
		      baseVal=dmin,  unitVal=dmax;

		Bool isIntensity = dataUnit() == Unit("Jy/beam");

		if(isIntensity) {
			minLimit=min(dmin, 0.);
			maxLimit=absmax;
			baseVal=0.;
			unitVal=absmax;
		}

		Float range = abs(maxLimit-minLimit);
		Float res = range*.005;
		if(res<=0.) res = .001;
		else res = pow(10., floor(log10(res)));
		// slider resolution (will be a power of ten.)

		if(res<=0.) res = .001;	// (Safety... doubtful need/usefulness.)


		if(unitVal-baseVal > 2*res) {
			baseVal = floor(baseVal/res + .5)*res;	// round to
			unitVal = floor(unitVal/res + .5)*res;
		}	// nearest res

		if(maxLimit-minLimit > 2*res) {
			minLimit = floor(minLimit/res + .5)*res;
			maxLimit = floor(maxLimit/res + .5) *res;
		}

		// Set standard relative levels:  [-.8 -.6 -.4 -.2  .2  .4 . 6 . 8]

		Vector<Float> rellevels(8);
		for(Int i=0; i<4; i++) {
			rellevels[i]   = .2*(i-4);
			rellevels[i+4] = .2*(i+1);
		}

		// Eliminate relative levels that will not be needed.

		Int nlevels = rellevels.size();
		itsLevels.resize(nlevels);
		Int j = 0;
		Float delta = unitVal - baseVal;
		for(Int i=0; i<nlevels; i++) {
			Float abslevel = baseVal + rellevels[i]*delta;
			if(abslevel>=dmin && abslevel<=dmax) itsLevels[j++] = rellevels[i];
		}
		itsLevels.resize(j, True);

		// Post defaults to sliders

		itsBaseContour->setMinimum(minLimit);
		itsBaseContour->setMaximum(maxLimit);
		itsBaseContour->setResolution(res);
		itsBaseContour->setDefaultValue(baseVal);
		itsBaseContour->setValue(baseVal);

		itsUnitContour->setMinimum(minLimit);
		itsUnitContour->setMaximum(maxLimit);
		itsUnitContour->setResolution(res);
		itsUnitContour->setDefaultValue(unitVal);
		itsUnitContour->setValue(unitVal);


		if(recOut!=0) {
			Bool wholeRecord=True, overwrite=True;
			itsBaseContour->toRecord(*recOut, wholeRecord, overwrite);
			itsUnitContour->toRecord(*recOut, wholeRecord, overwrite);
			Record levels;
			levels.define("dlformat", "rellevels");
			levels.define("value", itsLevels);
			recOut->defineRecord("rellevels", levels);
		}
	}



	template <class T>
	Vector<Float> LatticeAsContour<T>::levels() {
		// Compute/return actual selected contour levels, from user's relative
		// levels and the actual min and max contour values (from the sliders.)

		Int nlevels = itsLevels.nelements();
		Vector<Float> abslevels(nlevels);
		Vector<Float> rellevels(itsLevels);	// Entered (relative) levels.

		if(nlevels>0) {
			Float dmin=getDataMin(),
			      dmax=getDataMax();

			Float baseVal = itsBaseContour->value(),	// 'slider' values.
			      unitVal = itsUnitContour->value();

			Float delta = unitVal - baseVal;

			// The linear scaling: from [0, 1] -> [baseVal, unitVal]
			// Relative levels that will outside data bounds are eliminated,
			// for contouring efficiency.  (Similar code in setStdContourLimits_()
			// above is just for tidiness in the defaults...)

			Int j=0;
			for(Int i=0; i<nlevels; i++) {
				Float abslevel = baseVal + rellevels[i]*delta;
				if(abslevel>=dmin && abslevel<=dmax) abslevels[j++] = abslevel;
			}

			abslevels.resize(j, True);
			nlevels = abslevels.nelements();

			// Assure the levels are sorted, for good measure.

			for(Int i=0; i<nlevels; i++) {
				for(Int j=i+1; j<nlevels; j++) {
					if(abslevels[j]<abslevels[i]) {
						Float t=abslevels[i];
						abslevels[i]=abslevels[j];
						abslevels[j]=t;
					}
				}
			}
		}

		return abslevels;
	}



	template <class T>
	String LatticeAsContour<T>::levelString(Int prec) {
		// Actual levels, in String format

		Vector<Float> lvls;
		lvls = levels();
		Int nlvls = lvls.size();

		if(prec<=0) {
			// Try to determine  a precision that is low enough not to
			// clutter up tracking.
			Float absmax = max(abs(itsBaseContour->minimum()),
			                   abs(itsUnitContour->maximum()));
			Float res = itsUnitContour->resolution();
			if(absmax==0.) absmax = 1.;	// (safety)
			if(res<=0.)    res = .001;	// (safety)
			prec = max (2, ifloor(log10(absmax/res))+1);
		}

		ostringstream os;
		os<<setprecision(prec);
		if(nlvls>0) os<<lvls[0];
		for(Int i=1; i<nlvls; i++) os<<" "<<lvls[i];
		os<<flush;

		return String(os);
	}



	template <class T>
	String LatticeAsContour<T>::showPosition(const Vector<Double> &wld,
	        const Bool &abs, const Bool &dsp) {
		// Adds contour level information to the standard position tracking
		// string from PADD.
		String pos = PrincipalAxesDD::showPosition(wld, abs, dsp)+"\nContours: ";
		String lvls = levelString();
		pos += (lvls=="")? "[none]" : lvls;

		return pos;
	}





	template <class T>
	Bool LatticeAsContour<T>::setOptions(Record &rec, Record &recOut) {
		Bool ret = LatticePADisplayData<T>::setOptions(rec, recOut);

		Bool localchange = False;
		Bool error;

		localchange = (readOptionRecord(itsLine, error, rec, "line") ||
		               localchange);
		localchange = (readOptionRecord(itsDashNeg, error, rec, "dashneg") ||
		               localchange);
		localchange = (readOptionRecord(itsDashPos, error, rec, "dashpos") ||
		               localchange);
		localchange = (readOptionRecord(itsColor, error, rec, "color") ||
		               localchange);

		Bool lvlChg=False;

		if (rec.isDefined("rellevels")) {
			DataType dtype = rec.dataType("rellevels");

			Record* valrec = &rec;
			String fldnm = "rellevels";
			Record subrec;

			if(dtype==TpRecord) {
				subrec = rec.subRecord("rellevels");
				if(subrec.isDefined("value")) {
					valrec = &subrec;
					dtype = subrec.dataType("value");
					fldnm = "value";
				}
			}	// (All this is a bit of a pain....)

			Vector<Float> newlevels;

			if ((dtype == TpArrayFloat) || (dtype == TpArrayDouble) ||
			        (dtype == TpArrayInt) ||
			        (dtype == TpFloat) || (dtype == TpDouble) || (dtype == TpInt)) {

				switch (dtype) {
				case TpFloat:
				case TpArrayFloat: {
					Vector<Float> temp;
					valrec->get(fldnm, temp);
					newlevels.resize(temp.nelements());
					for (uInt i = 0; i < newlevels.nelements(); i++) {
						newlevels(i) = temp(i);
					}
					break;
				}
				case TpDouble:
				case TpArrayDouble: {
					Vector<Double> temp;
					valrec->get(fldnm, temp);
					newlevels.resize(temp.nelements());
					for (uInt i = 0; i < newlevels.nelements(); i++) {
						newlevels(i) = temp(i);
					}
					break;
				}
				case TpInt:
				case TpArrayInt: {
					Vector<Int> temp;
					valrec->get(fldnm, temp);
					newlevels.resize(temp.nelements());
					for (uInt i = 0; i < newlevels.nelements(); i++) {
						newlevels(i) = temp(i);
					}
					break;
				}
				default:
					// not possible!
					break;
				}
//
				Bool diff = (newlevels.nelements() != itsLevels.nelements());
				if (!diff) {
					for (uInt i = 0; i < newlevels.nelements(); i++) {
						diff = (newlevels(i) != itsLevels(i));
						if (diff) break;
					}
				}
				if (diff) {
					itsLevels.resize(newlevels.nelements());
					for (uInt i = 0; i < newlevels.nelements(); i++) {
						itsLevels(i) = newlevels(i);
					}
					lvlChg = True;
				}
			}
		}

		lvlChg = itsBaseContour->fromRecord(rec) || lvlChg;
		lvlChg = itsUnitContour->fromRecord(rec) || lvlChg;

		if(lvlChg) recOut.define("trackingchange", True);
		// 'Signals' desire to update tracking info for this DD (where
		// contour levels are also displayed).  (I wish we _could_ use true
		// (Qt) signals on this level, and not have to wait for setOpts()
		// calls in order to change things...)

		localchange = localchange || lvlChg;

		// must come last - this forces ret to be True or False:
		// (dk: no, this is probably obs. rubbish (and done wrong initially...))
		if (rec.isDefined("refresh") && (rec.dataType("refresh") == TpBool)) {
			Bool ref;
			rec.get("refresh", ref);
			localchange = localchange || ref;
		}

		ret = (ret || localchange);

		return ret;
	}


	template <class T>
	Record LatticeAsContour<T>::getOptions( bool scrub ) const {
		Record rec = LatticePADisplayData<T>::getOptions(scrub);

		Record levels;
		levels.define("dlformat", "rellevels");
		levels.define("listname", "Relative Contour Levels");
		levels.define("ptype", "array");
		levels.define("default", itsLevels);
		levels.define("value", itsLevels);
		levels.define("allowunset", False);
		levels.define("help", "These are relative contour levels, which will be\n"
		              "scaled linearly: 0 and 1 map to the actual contour\n"
		              "levels set on the 'Base Contour Level' and\n"
		              "'Unit Contour Level' sliders, respectively.");
		rec.defineRecord("rellevels", levels);

		itsBaseContour->toRecord(rec);
		itsUnitContour->toRecord(rec);

		Record line;
		line.define("dlformat", "line");
		line.define("listname", "Line width");
		line.define("ptype", "floatrange");
		line.define("pmin", Float(0.0));
		line.define("pmax", Float(5.0));
		line.define("presolution", Float(0.1));
		line.define("default", Float(0.5));
		line.define("value", itsLine);
		line.define("allowunset", False);
		rec.defineRecord("line", line);

		Record dashNeg;
		dashNeg.define("dlformat", "dashneg");
		dashNeg.define("listname", "Dash negative contours?");
		dashNeg.define("ptype", "boolean");
		dashNeg.define("default", True);
		dashNeg.define("value", itsDashNeg);
		dashNeg.define("allowunset", False);
		rec.defineRecord("dashneg", dashNeg);

		Record dashPos;
		dashPos.define("dlformat", "dashpos");
		dashPos.define("listname", "Dash positive contours?");
		dashPos.define("ptype", "boolean");
		dashPos.define("default", False);
		dashPos.define("value", itsDashPos);
		dashPos.define("allowunset", False);
		rec.defineRecord("dashpos", dashPos);

		Record color;
		color.define("dlformat", "color");
		color.define("listname", "Line color");
		color.define("ptype", "userchoice");
		Vector<String> vcolor(11);
		vcolor(0) = "foreground";
		vcolor(1) = "background";
		vcolor(2) = "black";
		vcolor(3) = "white";
		vcolor(4) = "red";
		vcolor(5) = "green";
		vcolor(6) = "blue";
		vcolor(7) = "cyan";
		vcolor(8) = "magenta";
		vcolor(9) = "yellow";
		vcolor(10) = "gray";
		color.define("popt", vcolor);
		color.define("default", "foreground");
		color.define("value", itsColor);
		color.define("allowunset", False);
		rec.defineRecord("color", color);

		return rec;
	}


} //# NAMESPACE CASA - END

