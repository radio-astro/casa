//# WorldAxesDM.cc: axis label drawing for AxesDisplayData
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
#include <casa/Arrays/Vector.h>
#include <measures/Measures/MDirection.h>
#include <casa/BasicSL/Constants.h>
/* COMMENTED OUT UNTIL SPHERICAL PROJECTIONS AVAILABLE
#include <wcslib/cel.h>
*/
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <display/Display/DisplayCoordinateSystem.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <coordinates/Coordinates/LinearCoordinate.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <coordinates/Coordinates/TabularCoordinate.h>
#include <display/Display/Attribute.h>
#include <display/Display/AttributeBuffer.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayDatas/WorldAxesDD.h>
#include <display/DisplayDatas/WorldAxesDM.h>
#include <cpgplot.h>


#include <casa/Logging/LogIO.h>
#include <casa/Quanta/Unit.h>
#include <casa/BasicSL/String.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicMath/Math.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MDirection.h>

namespace casa { //# NAMESPACE CASA - BEGIN

//#define CDEBUG

	WorldAxesDM::WorldAxesDM(WorldCanvas *worldCanvas,
	                         AttributeBuffer *wchAttributes,
	                         AttributeBuffer *ddAttributes,
	                         CachingDisplayData *dd) :
		CachingDisplayMethod(worldCanvas, wchAttributes, ddAttributes, dd) {
	}

	WorldAxesDM::~WorldAxesDM() {
		cleanup();
	}

////////////////////////////////////////////////////////////////////////
// begin PGSBOX interface - derived from AvAxis.cc via WCCSNLAxisLabeller.cc
	extern "C" {

		typedef void(*S_fp)(int *, int *, int *, int *, char *, int *,
		                    double *, double *, double *, int *,
		                    double *, int *, int);

		// Decide whether external FORTRAN references need an underscore appended
		// (they usually do). For now, stand alone aipsview ships with its own
		// f2c'd version which has one.
#if !defined(NEED_FORTRAN_UNDERSCORES)
#define NEED_FORTRAN_UNDERSCORES 1
#endif

#if NEED_FORTRAN_UNDERSCORES
#define PGSBOX pgsbox_
#else
#define PGSBOX pgsbox
#endif

		extern void PGSBOX (float* blc, float* trc, char *idents, char *opt,
		                    int *labctl, int *labden, int *ci, int *gcode,
		                    double *tiklen, int *ng1, double *grid1, int *ng2,
		                    double *grid2, int *doeq, S_fp nlfunc, int *nlc,
		                    int *nli, int *nld, char *nlcprm, int *nliprm,
		                    double *nldprm, int *nc, int *ic, double *cache,
		                    int *ierr, int idents_len, int opt_len, int nlcprm_len);


		// In case ints are smaller than pointers, just pass the pointer to the
		// global options the BASIC way. Since aipsview isn't multi-threaded,
		// multiprocessing, etc. we can get away with it.
		// static AxisGlobalOptions *gOptions_=NULL;
		DisplayCoordinateSystem* WADMAxisLabellerCoordinateSystem = NULL;
		Vector<Double> WADMvWorldIn, WADMvWorldOut;
		Int WADMAxisLabellerLongCoord = -1, WADMAxisLabellerLatCoord = -1;
		struct celprm *WADMAxisLabellerCelPrm = 0;
		struct prjprm *WADMAxisLabellerPrjPrm = 0;
		Vector<Bool> WADMAxisLabellerWorldAxes, WADMAxisLabellerPixelAxes;
		Vector<Double> WADMAxisLabellerWorldMin, WADMAxisLabellerWorldMax;

		Int WADMAxisLabellerSpecCoordIdx = -1;
		SpectralCoordinate WADMAxisLabellerSpecCoord;

		Int WADMAxisLabellerVelAxis = -1;
		Bool WADMAxisLabellerAbsolute = True;
		Bool WADMAxisLabellerWorldLabels = True;
		Double WADMAxisLabellerVelRef = 0.0;
		Double WADMAxisLabellerFreqRef = 0.0;


		// nlfunc is called from pgsbox to do world to/from pixel conversions.
		// The version used here just passes the requests to aipsview's routines
		// via the global options variable.
		// The return code is supposed to indicate whether the pixel or world
		// values were invalid. We ignore that possibility since it isn't clear
		// what that means. pgsbox intentionally calls with pixel/world values
		// outside the range of the data set.
		// (2001/09: handle all exceptions here because they won't unwind back
		// through the fortran pg code).
		static void nlfunc(int *opcode, int *, int *, int *,
		                   char *, int *, double *,
		                   double *world, double *pixel, int *contrl,
		                   /* comment out contxt until spherical projs avail
		                   double *contxt, int *ierr, int) {
		                   */
		                   double *, int *ierr, int) {
			// ASSUME everything has been set up correctly, ie.
			// (WADMAxisLabellerCoordinateSystem != NULL)

			static int err;
			err = 0;

			static Vector<Double> vPixelIn(2);
			Double frequency, velocity;

			switch (*opcode) {
			case 0:		// Initialize.
				*contrl = 0;
				break;
			case 1:		// Compute pixel coordinates.
			case 2:		// Similar.

				try {
					static Vector<Double> vPixelOut(2);

					WADMvWorldIn = 0.0;
					WADMvWorldIn(0) = world[0];
					WADMvWorldIn(1) = world[1];
					if (WADMAxisLabellerLatCoord > -1) {
						// restrict latitude coords to range [-90, 90]
						if (WADMvWorldIn(WADMAxisLabellerLatCoord) > 90.0) {
							WADMvWorldIn(WADMAxisLabellerLatCoord) = 90.0;
						} else if (WADMvWorldIn(WADMAxisLabellerLatCoord) < -90.0) {
							WADMvWorldIn(WADMAxisLabellerLatCoord) = -90.0;
						}
					}
					/*
					// TEMPORARY - REMOVE WHEN SPHERICAL PROJECTIONS AVAILABLE?
					if (WADMAxisLabellerLongCoord > -1) {
					  // restrict longitude coords to range [-180, 180]
					  if (WADMvWorldIn(WADMAxisLabellerLongCoord) > 180.0) {
					    WADMvWorldIn(WADMAxisLabellerLongCoord) = 180.0;
					  } else if (WADMvWorldIn(WADMAxisLabellerLongCoord) < -180.0) {
					    WADMvWorldIn(WADMAxisLabellerLongCoord) = -180.0;
					  }
					}
					*/

// If we are doing relative labels, first convert to absolute world
// If we are showing velocity labels, use a relative world
// coordinate of 0, and then handle velocity afterwards

					if (WADMAxisLabellerVelAxis != -1) {
						WADMvWorldIn[WADMAxisLabellerVelAxis] = 0.0;
					}
					if (!WADMAxisLabellerAbsolute) {
						WADMAxisLabellerCoordinateSystem->makeWorldAbsolute(WADMvWorldIn);
					}

// Now we must overwrite the velocity value with the absolute frequency

					if (WADMAxisLabellerVelAxis != -1) {
						velocity = world[WADMAxisLabellerVelAxis];
						if (!WADMAxisLabellerAbsolute) {

// Convert relative to absolute velocity ; rel = abs - ref

							velocity += WADMAxisLabellerVelRef;
#ifdef CDEBUG
							cerr << "Converting velocity to absolute: reference = "
							     << WADMAxisLabellerVelRef
							     << ", abs. velocity = " << velocity
							     << endl;
#endif
						}

// Convert absolute velocity to absolute frequency

						if (!WADMAxisLabellerSpecCoord.velocityToFrequency(frequency, velocity)) {
							err = 2;
							cerr << "Problem (err=2) when converting velocity to frequency." << endl;
							break;
						} else {
							WADMvWorldIn[WADMAxisLabellerVelAxis] = frequency;
#ifdef CDEBUG
							cerr << "Velocity converted; frequency = " << frequency << endl;
#endif
						}
					}

					if (*contrl == 0) {
						// contrl == 0 : normal operation + check for discontinuities
						vPixelIn = 0.0;
						static Bool result;
						result = WADMAxisLabellerCoordinateSystem->
						         toMix(WADMvWorldOut, vPixelOut, WADMvWorldIn, vPixelIn,
						               WADMAxisLabellerWorldAxes, WADMAxisLabellerPixelAxes,
						               WADMAxisLabellerWorldMin, WADMAxisLabellerWorldMax);
						if (!result) {
							err = 2;
							break;
						}
						pixel[0] = vPixelOut(0);
						pixel[1] = vPixelOut(1);

						/* COMMENTED OUT UNTIL SPHERICAL PROJECTIONS AVAILABLE
						if (WADMAxisLabellerLongCoord > -1) {
						  // we have a longtitude coordinate which could "wrap"
						  static Double phi, theta;
						  phi = (vPixelOut(WADMAxisLabellerLongCoord) -
						   WADMAxisLabellerCoordinateSystem->
						   referencePixel()(WADMAxisLabellerLongCoord)) *
						    WADMAxisLabellerCoordinateSystem->
						    increment()(WADMAxisLabellerLongCoord);
						  if (abs(phi - contxt[3]) > 180.0) {
						    // hit a discontinuity at phi = +/- 180.0
						    contxt[5] = pixel[0];
						    contxt[6] = pixel[1];
						    *contrl = 1;
						  } else {
						    // no discontinuity
						    contxt[1] = WADMvWorldIn(0);
						    contxt[2] = WADMvWorldIn(1);
						    contxt[3] = phi;
						    contxt[4] = theta;
						  }
						}
						} else if (*contrl == 1) {
						  // contrl == 1 : move to the other side of the discontinuity
						  pixel[0] = contxt[7];
						  pixel[1] = contxt[8];
						  *contrl = 2;
						} else {
						  // contrl == 2 : complete the traversal
						  pixel[0] = contxt[5];
						  pixel[1] = contxt[6];
						  *contrl = 0;
						  */
					}
				}	// try
				catch (...) {
					err=2;
				}
				break;

			case -1:	// Compute World coordinates
				try {
					vPixelIn(0) = pixel[0];
					vPixelIn(1) = pixel[1];
					if (WADMAxisLabellerCoordinateSystem->toWorld(WADMvWorldOut,
					        vPixelIn)) {
						world[0] = WADMvWorldOut(0);
						world[1] = WADMvWorldOut(1);

// Overwrite the value for the spectral axis if doing velocity conversions

						if (WADMAxisLabellerVelAxis != -1) {

							frequency = WADMvWorldOut[WADMAxisLabellerVelAxis];
#ifdef CDEBUG
							cerr << "OK, WADMAxisLabellerVelAxis = " << WADMAxisLabellerVelAxis
							     << ", converting frequency = " << frequency
							     << " to velocity now." << endl;
#endif
// Convert absolute frequency to absolute velocity

							if (!WADMAxisLabellerSpecCoord.frequencyToVelocity (velocity, frequency)) {
								cerr << "Problem (err=3) when converting frequency to velocity!" << endl;
								err = 3;
							} else {
								world[WADMAxisLabellerVelAxis] = velocity;
#ifdef CDEBUG
								cerr << "Successfully converted frequency = " << frequency
								     << " to velocity = " << velocity
								     << endl;
#endif
							}

// Now make relative if required; rel = abs - ref

							if (!WADMAxisLabellerAbsolute) {
								world[WADMAxisLabellerVelAxis] -= WADMAxisLabellerVelRef;
							}
						}

					} else {
						err = 3;
					}
				} catch (...) {
					err=3;
				}
				break;

			default:
				err = 1;
			}
			*ierr = err;
		}	// end nlfunc

	}	// extern "C"


	static void cpgsbox(float* blc, float* trc, char *idents, char *opt,
	                    int labctl, int labden, int *ci, int *gcode,
	                    double tiklen, int ng1, double *grid1, int ng2,
	                    double *grid2, int doeq, S_fp nlfunc, int nlc, int nli,
	                    int nld, char *nlcprm, int *nliprm, double *nldprm,
	                    int nc, int &ic, double *cache, int &ierr) {
		int idents_len=80, opt_len=1, nlcprm_len = 1;

		PGSBOX(blc, trc, idents, opt,
		       &labctl, &labden, ci, gcode,
		       &tiklen, &ng1, grid1, &ng2,
		       grid2,  &doeq, nlfunc, &nlc, &nli,
		       &nld,  nlcprm, nliprm, nldprm,
		       &nc, &ic, cache, &ierr, idents_len, opt_len, nlcprm_len);
	}


// end PGSBOX interface - derived from AvAxis.cc via WCCSNLAxisLabeller.cc
////////////////////////////////////////////////////////////////////////

	Bool WorldAxesDM::drawIntoList(Display::RefreshReason /*reason*/,
	                               WorldCanvasHolder &wcHolder) {

		// Locate the WorldCanvas to draw upon
		WorldCanvas* wc = wcHolder.worldCanvas();
		// Find the parent DisplayData
		WorldAxesDD* parent = dynamic_cast<WorldAxesDD* >(parentDisplayData());
		if (!parent) {
			throw(AipsError("invalid parent of WorldAxesDM"));
		}

		// 1. retrieve coordsys from WorldCanvas, and select appropriate units
		DisplayCoordinateSystem* pCS;
		if (&(wc->coordinateSystem())) {
			//if (0) {
			pCS = new DisplayCoordinateSystem(wc->coordinateSystem());
		} else {
			// do nothing!
			return False;
		}

// None of the specAxis stuff, used in WCCSNLAxisLabeller is actually
// implemented in this class as far as I can see [nebk]
		Int specAxis = -1;
		for (uInt i = 0; i < pCS->nCoordinates(); i++) {
			switch(pCS->type(i)) {
//
			case Coordinate::DIRECTION: {
				pCS->setDirectionUnit ( "deg", Int(i) );
			}
			break;
			case Coordinate::SPECTRAL: {
				String spectralUnit = parent->spectralUnit();
				String spectralDoppler = parent->velocityType();

// Set velocity and/or world unit state in SpectralCoordinate

				String errorMsg;
				if ( ! pCS->setSpectralState( errorMsg, parent->spectralUnit( ),parent->velocityType( ) ) ) {
// What to do with error ?
				}

// Are either of the first two display axes Spectral ?
// specAxis = 0 or 1 will trigger velocity conversions

				specAxis = pCS->worldAxes(i)(0);
				if (specAxis>1) specAxis = -1;

				if (Unit("km/s") != Unit(spectralUnit)) { // when don't want velocity
					specAxis = -1;
					WADMAxisLabellerVelAxis = -1;
				}

// nlfunc needs these

				WADMAxisLabellerSpecCoordIdx = i;
				WADMAxisLabellerSpecCoord = pCS->spectralCoordinate(i);

// We are going to need the velocity at the reference value
// so we can convert relative velocity back to frequency

				if (specAxis != -1) {
					Double velRef;
					if (!WADMAxisLabellerSpecCoord.frequencyToVelocity(velRef,
					        WADMAxisLabellerSpecCoord.referenceValue()[0])) {
						return False;
					}
					WADMAxisLabellerVelRef = velRef;
					WADMAxisLabellerVelAxis = specAxis;
					WADMAxisLabellerFreqRef = WADMAxisLabellerSpecCoord.referenceValue()(0);
				}
			}
			break;
			default:
				break;
			}
		}

		// 2. setup a bunch of things

		WADMAxisLabellerCoordinateSystem = pCS;
		WADMvWorldIn.resize(pCS->nWorldAxes());
		WADMvWorldOut.resize(pCS->nWorldAxes());

		float blc[2];
		float trc[2];
		char opt[] = "  ";
		int labctl = 0, labden=0, ci[7] = {16, 17, 18, 19, 20, 21, 22};
		int gcode[2] = {0, 0};
		double tiklen = 0.00;
		int ng1 = 0;
		double grid1[1] = {0};
		int ng2 = 0;
		double grid2[1] = {0};
		int doeq = 0;
		static const int NLC=10, NLI=10, NLD=10, NCACHE=100;
		int nlc=NLC, nli=NLI, nld=NLD, nc=NCACHE, ic=-1;
		char nlcprm[NLC+1];
		int nliprm[NLI];
		double nldprm[NLD];
		double cache[4*(NCACHE+1)];	// cache indexes go from 0 to NCACHE.
		int ierr;

		blc[0] = wc->linXMin();
		blc[1] = wc->linYMin();
		trc[0] = wc->linXMax();
		trc[1] = wc->linYMax();
		labctl = 0;
		nlcprm[0] = '\0';

		// figure out reasonable default for opt:

		WADMAxisLabellerLongCoord = -1;
		WADMAxisLabellerLatCoord = -1;

		for (uInt i = 0; i < 2; i++) {

			Int coordinate, axisInCoord;
			pCS->findPixelAxis(coordinate, axisInCoord, i);
			switch (pCS->type(coordinate)) {
			case Coordinate::SPECTRAL: {
				// nothing special for SPECTRAL ...
				break;
			}
			case Coordinate::LINEAR: {
				String axisUnit = pCS->worldAxisUnits()(i);

				if ( Unit(axisUnit) == Unit("s")) {
					opt[i] = 'T';
				}
				break;
			}
			case Coordinate::DIRECTION: {
				DirectionCoordinate dcoord = pCS->directionCoordinate(coordinate);
				// COMMENTED OUT - UNCOMMENT SOON!
				/*
				  if (WADMAxisLabellerCelPrm) {
				  delete WADMAxisLabellerCelPrm;
				  WADMAxisLabellerCelPrm = new celprm;
				  }
				  if (WADMAxisLabellerPrjPrm) {
				  delete WADMAxisLabellerPrjPrm;
				  WADMAxisLabellerPrjPrm = new prjprm;
				  }
				  celset(Projection::name(dcoord.projection().type()).chars(),
				  WADMAxisLabellerCelPrm, WADMAxisLabellerPrjPrm);
				*/
				MDirection::Types mdirtype = dcoord.directionType();
				MDirection::GlobalTypes globtype = MDirection::globalType(mdirtype);
				switch(axisInCoord) {
				case 0:
					WADMAxisLabellerLongCoord = i;
					switch(globtype) {
					case MDirection::GRADEC:
						opt[i] = 'G'; // hms [0, 24)
						break;
					case MDirection::GHADEC:
						opt[i] = ' '; // decimal - needs to be 'T' after fixing units
						break;
					case MDirection::GAZEL:
					case MDirection::GLONGLAT:
						opt[i] = 'A'; // decimal degrees [0, 360)
						break;
					default:
						opt[i] = ' ';
						break;
					}
					break;
				case 1:
					WADMAxisLabellerLatCoord = i;
					switch(globtype) {
					case MDirection::GRADEC:
					case MDirection::GHADEC:
						opt[i] = 'E'; // sexigesimal degrees (-180, 180]
						break;
					case MDirection::GAZEL:
					case MDirection::GLONGLAT:
						opt[i] = 'B'; // decimal degrees (-180, 180]
						break;
					default:
						opt[i] = ' ';
						break;
					}
					break;
				default:
					opt[i] = ' ';
					break;
				}
			}
			break;
			default:
				opt[i] = ' ';
				break;
			}
		}

		wc->setColor(wc->getWorldForegroundColor());
		wc->pixelCanvas()->setCapStyle(Display::CSButt);
		wc->pixelCanvas()->setJoinStyle(Display::JSMiter);

		WADMAxisLabellerWorldAxes.resize(pCS->nWorldAxes());
		WADMAxisLabellerWorldAxes = False;
		WADMAxisLabellerWorldAxes(0) = WADMAxisLabellerWorldAxes(1) = True;
		WADMAxisLabellerPixelAxes.resize(2);
		WADMAxisLabellerPixelAxes = False;

		WADMAxisLabellerWorldMin.resize(pCS->nWorldAxes());
		WADMAxisLabellerWorldMax.resize(pCS->nWorldAxes());
		for (uInt i = 0; i < WADMAxisLabellerWorldMin.nelements(); i++) {
			Int coord, axisInCoord;
			pCS->findWorldAxis(coord, axisInCoord, i);
			if (pCS->type(coord) == Coordinate::DIRECTION) {
				if (axisInCoord == 0) {
					WADMAxisLabellerWorldMin(i) = 0.0;
					WADMAxisLabellerWorldMax(i) = 360.0;
				} else {
					WADMAxisLabellerWorldMin(i) = -90.0;
					WADMAxisLabellerWorldMax(i) = 90.0;
				}
			} else {
				WADMAxisLabellerWorldMin(i) = -99e99;
				WADMAxisLabellerWorldMax(i) = +99e99;
			}
		}

		// grid type options
		String stemp = parent->xGridType();
		if (stemp == "Tick marks") {
			gcode[0] = 1;
		} else if (stemp == "Full grid") {
			gcode[0] = 2;
		}
		stemp = parent->yGridType();
		if (stemp == "Tick marks") {
			gcode[1] = 1;
		} else if (stemp == "Full grid") {
			gcode[1] = 2;
		}

		// tick length
		//tiklen = (Double)tickLength();
		tiklen = 4.0;

		// colors
		PixelCanvas* pc = wc->pixelCanvas();
		Float r, g, b;
		pc->getColorComponents(parent->xAxisColor(), r, g, b);
		cpgscr(16, r, g, b);
		pc->getColorComponents(parent->yAxisColor(), r, g, b);
		cpgscr(17, r, g, b);
		pc->getColorComponents(parent->xAxisColor(), r, g, b);
		cpgscr(18, r, g, b);
		pc->getColorComponents(parent->yAxisColor(), r, g, b);
		cpgscr(19, r, g, b);
		pc->getColorComponents(parent->xAxisColor(), r, g, b);
		cpgscr(20, r, g, b);
		pc->getColorComponents(parent->yAxisColor(), r, g, b);
		cpgscr(21, r, g, b);
		pc->getColorComponents(parent->titleTextColor(), r, g, b);
		cpgscr(22, r, g, b);
		pc->getColorComponents(parent->outlineColor(), r, g, b);
		cpgscr(23, r, g, b);

		// width
		pc->setLineWidth(parent->lineWidth());

		char idents[80 * 3];
		uInt ididx;
		String axtxt = parent->xAxisText(wc);
		strncpy(idents + 0, axtxt.chars(), 80);
		for (ididx = axtxt.length(); ididx < 80; ididx++) {
			idents[0 + ididx] = 32;
		}
		axtxt = parent->yAxisText(wc);
		strncpy(idents + 80, axtxt.chars(), 80);
		for (ididx = axtxt.length(); ididx < 80; ididx++) {
			idents[80 + ididx] = 32;
		}
		strncpy(idents + 160, parent->titleText().chars(), 80);
		for (ididx = parent->titleText().length(); ididx < 80; ididx++) {
			idents[160 + ididx] = 32;
		}

		cpgsch(parent->charSize());
		stemp = parent->charFont();
		if (stemp == "roman") {
			cpgscf(2);
		} else if (stemp == "italic") {
			cpgscf(3);
		} else if (stemp == "script") {
			cpgscf(4);
		} else { // "normal" or anything else
			cpgscf(1);
		}

		// Do the real work

		cpgsbox(blc, trc, idents, opt, labctl, labden, ci, gcode,
		        tiklen, ng1, grid1, ng2, grid2, doeq, nlfunc,
		        nlc, nli, nld, nlcprm, nliprm, nldprm,
		        nc, ic, cache, ierr);

		//if (plotOutline()) {
		if (True) {
			cpgsci(23);
			cpgbox("BC", 0.0, 0, "BC", 0.0, 0);
		}

		if (pCS) {
			delete pCS;
			pCS = 0;
		}
		return True;
	}

	void WorldAxesDM::cleanup() {
	}

	WorldAxesDM::WorldAxesDM() {
	}

	WorldAxesDM::WorldAxesDM(const WorldAxesDM &other) :
		CachingDisplayMethod(other) {
	}

	void WorldAxesDM::operator=(const WorldAxesDM &) {
	}

} //# NAMESPACE CASA - END

