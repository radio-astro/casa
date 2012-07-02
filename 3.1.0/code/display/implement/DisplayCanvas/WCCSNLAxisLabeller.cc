//# WCCSNLAxisLabeller.cc: non-linear axis labelling using a CS on a WC
//# Copyright (C) 1999,2000,2001,2002
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
#include <casa/Arrays/ArrayMath.h>
#include <casa/BasicMath/Math.h>
#include <measures/Measures/MDirection.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/Unit.h>
#include <casa/BasicSL/Constants.h>
#include <casa/Utilities/Assert.h>
#include <coordinates/Coordinates/CoordinateUtil.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <coordinates/Coordinates/SpectralCoordinate.h>
#include <display/Display/State.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/DisplayCanvas/WCCSNLAxisLabeller.h>

#include <cpgplot.h>
#include <casa/sstream.h>


namespace casa { //# NAMESPACE CASA - BEGIN

WCCSNLAxisLabeller::WCCSNLAxisLabeller() 
: WCCSAxisLabeller(),
  itsValid(False)
{
  setDefaultOptions();
}

WCCSNLAxisLabeller::~WCCSNLAxisLabeller() 
{
  invalidate();
}

////////////////////////////////////////////////////////////////////////
// PGSBOX interface 

extern "C" {

  typedef void(*S_fp)(int *, int *, int *, int *, char *, int *, 
		      double *, double *, double *, int *, 
		      double *, int *, int);

  // Decide whether external FORTRAN references need an underscore appended
  // (they usually do).  Unilaterally declare that we need underscores for now

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
  // global options the BASIC way. Since aips++isn't multi-threaded,
  // multiprocessing, etc. we can get away with it.

  CoordinateSystem* WCCSNLAxisLabellerCoordinateSystem = NULL;
//
  Int WCCSNLAxisLabellerLongAxis = -1;         
  Int WCCSNLAxisLabellerLatAxis = -1;
  Int WCCSNLAxisLabellerVelAxis = -1;
  Int WCCSNLAxisLabellerSpecCoordIdx = -1;
  SpectralCoordinate WCCSNLAxisLabellerSpecCoord;
  Bool WCCSNLAxisLabellerAbsolute = True;
  Bool WCCSNLAxisLabellerWorldLabels = True;
  Double WCCSNLAxisLabellerVelRef = 0.0;
  Double WCCSNLAxisLabellerFreqRef = 0.0;
  Vector<Double> WCCSNLAxisLabellerRefPix(2);
  Int WCCSNLAxisLabellerUIBase = 1;

// The supplied CoordinateSystem should only have 2 Pixel axes, those being displayed
// The coordinate for these axes will always be supplied as a world coordinate

  Vector<Bool> WCCSNLAxisLabellerPixelAxes(2, False);

// The CS may have more than 2 world axes.  This vector will be intiialized
// when the CS has been set.
 
  Vector<Bool> WCCSNLAxisLabellerWorldAxes;

// These specify the range of the world coordinate for each axis
// contained within the image being displayed.  Used by toMix.
// Only DirectionCoordinates matter.

  Vector<Double> WCCSNLAxisLabellerWorldMin, WCCSNLAxisLabellerWorldMax;

  // nlfunc is called from pgsbox to do world to/from pixel conversions.
  // The version used here just passes the requests to the CoordinateSystem.
  // The return code is supposed to indicate whether the pixel or world
  // values were invalid. We ignore that possibility since it isn't clear
  // what that means. pgcrvl intentionally calls with pixel/world values
  // outside the range of the data set.

  static void nlfunc(int *opcode, int *, int *, int *,
		     char *, int *, double *,
		     double *world, double *pixel, int *contrl,
		     /* comment out contxt until spherical projs avail
		     double *contxt, int *ierr, int) { 
		     */
		     double *, int *ierr, int) 
  {

    // ASSUME everything has been set up correctly, ie.
    // (WCCSNLAxisLabellerCoordinateSystem != NULL)

    int err;
    err = 0;
//
    Vector<Double> vPixelIn(2);
    Vector<Double> vPixelOut(2);
//
    Vector<Double> vWorldOut(WCCSNLAxisLabellerCoordinateSystem->nWorldAxes());
    Vector<Double> vWorldIn(WCCSNLAxisLabellerCoordinateSystem->nWorldAxes());
    Double frequency, velocity;
//
    switch(*opcode) {
    case 0:		// Initialize.
      *contrl = 0;
      break;
    case 1:		// Compute pixel coordinates.
    case 2:		// Similar.
      {

// Do NOT modify the input argument 'world'


        if (!WCCSNLAxisLabellerWorldLabels) {

// The world value is a pixel coordinate.  May be relative or absolute.
// If absolute, deal with 0/1-relness, otherwise, make it absolute

           if (WCCSNLAxisLabellerAbsolute) {
              pixel[0] = world[0] - WCCSNLAxisLabellerUIBase;
              pixel[1] = world[1] - WCCSNLAxisLabellerUIBase;
           } else {

// abs = rel + ref

              pixel[0] = world[0] + WCCSNLAxisLabellerRefPix[0];
              pixel[1] = world[1] + WCCSNLAxisLabellerRefPix[1];
           }
           break;
        }

// What comes in is the last vector of world values.  We may need 
// to convert these from velocity back to frequency before
// toMix (which operates, for the SpectralCoordinate in frequency units)
// can be called.  We may also need to convert them from relative
// back to absolute so toMix can do its thing.
//
	vWorldIn = 0.0;
        vWorldIn[0] = world[0]; 
        vWorldIn[1] = world[1];

// If we are doing relative labels, first convert to absolute world
// If we are showing velocity labels, use a relative world 
// coordinate of 0, and then handle velocity afterwards

        if (WCCSNLAxisLabellerVelAxis != -1) {
           vWorldIn[WCCSNLAxisLabellerVelAxis] = 0.0;
        }
        if (!WCCSNLAxisLabellerAbsolute) {
           WCCSNLAxisLabellerCoordinateSystem->makeWorldAbsolute(vWorldIn);
        }

// Now we must overwrite the velocity value with the absolute frequency

        if (WCCSNLAxisLabellerVelAxis != -1) {
           velocity = world[WCCSNLAxisLabellerVelAxis];
           if (!WCCSNLAxisLabellerAbsolute) {

// Convert relative to absolute velocity ; rel = abs - ref

              velocity += WCCSNLAxisLabellerVelRef;
           }

// Convert absolute velocity to absolute frequency

           if (!WCCSNLAxisLabellerSpecCoord.velocityToFrequency(frequency, velocity)) {
              err = 2;
              break;
           } else {
             vWorldIn[WCCSNLAxisLabellerVelAxis] = frequency;
           }
        }

//
	if (*contrl == 0) {

// contrl == 0 : normal operation + check for discontinuities


// Ok we have now generated vWorldIn with absolute world coordinates
// in the native units of the CoordinateSystem. 
//
// For the first two pixel axes, world coordinates are provided.
// For any other axes (movie, hidden) only pixel coordinates are known.
// Now the CoordinateSystem that has been provided has had all pixel
// axes above 0 and 1 removed, with the replacement pixel value 
// set to the pixel coordinate.  This will be used in the toMix
// conversions.  
//
/*
cerr << "Calling toMix with" << endl;
cerr << "  pixel, world in = " << vPixelIn << vWorldIn << endl;
cerr << "  pixel, world axes = " << WCCSNLAxisLabellerPixelAxes << WCCSNLAxisLabellerWorldAxes << endl;
*/
	  vPixelIn = 0.0;
          if (!WCCSNLAxisLabellerCoordinateSystem->toMix(vWorldOut, vPixelOut, vWorldIn, vPixelIn,
                        WCCSNLAxisLabellerWorldAxes, WCCSNLAxisLabellerPixelAxes,
                        WCCSNLAxisLabellerWorldMin, WCCSNLAxisLabellerWorldMax)) {
             err = 2;
             break;
	  }
//cerr << "  pixel, world out = " << vPixelOut << vWorldOut << endl;



// Fill output pixel coordinates returned by nlfunc

          pixel[0] = vPixelOut[0];
          pixel[1] = vPixelOut[1];
	 
/* COMMENTED OUT UNTIL SPHERICAL PROJECTIONS AVAILABLE
// I think this requires the wcs phi,theta parameters (defect 190) [nebk]

	  if (WCCSNLAxisLabellerLongAxis > -1) {
	    // we have a longtitude coordinate which could "wrap"
	    static Double phi, theta;
	    phi = (vPixelOut(WCCSNLAxisLabellerLongAxis) -
		   WCCSNLAxisLabellerCoordinateSystem->
		   referencePixel()(WCCSNLAxisLabellerLongAxis)) *
	      WCCSNLAxisLabellerCoordinateSystem->
	      increment()(WCCSNLAxisLabellerLongAxis);
	    if (abs(phi - contxt[3]) > 180.0) {
	      // hit a discontinuity at phi = +/- 180.0
	      contxt[5] = pixel[0];
	      contxt[6] = pixel[1];
	      *contrl = 1;
	    } else {
	      // no discontinuity
	      contxt[1] = vWorldIn(0);
	      contxt[2] = vWorldIn(1);
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
	break;
      }
    case -1:	// Compute pure world coordinates
      {
        if (!WCCSNLAxisLabellerWorldLabels) {

// Pixel coordinates

           if (WCCSNLAxisLabellerAbsolute) {

// abs

              world[0] = pixel[0] + WCCSNLAxisLabellerUIBase;
              world[1] = pixel[1] + WCCSNLAxisLabellerUIBase;
           } else {

// rel = abs - ref

              world[0] = pixel[0] - WCCSNLAxisLabellerRefPix[0];
              world[1] = pixel[1] - WCCSNLAxisLabellerRefPix[1];
           }
           break;
        }
//
	vPixelIn[0] = pixel[0];
	vPixelIn[1] = pixel[1];

// Convert absolute pixel to absolute world in native units of CS

/*
cerr << "Calling toWorld with " << endl;
cerr << "  pixel, world = " << vPixelIn;
*/
	if (WCCSNLAxisLabellerCoordinateSystem->toWorld(vWorldOut, vPixelIn)) { 
//cerr << vWorldOut << endl;
           if (WCCSNLAxisLabellerVelAxis != -1) {
              frequency = vWorldOut[WCCSNLAxisLabellerVelAxis];
           }

// Now if needed, make relative.  We use the reference value
// for the spectral axis if velocity conversions are happening

           if (!WCCSNLAxisLabellerAbsolute) {
              if (WCCSNLAxisLabellerVelAxis != -1) {
                 vWorldOut[WCCSNLAxisLabellerVelAxis] = WCCSNLAxisLabellerFreqRef;
              }
              WCCSNLAxisLabellerCoordinateSystem->makeWorldRelative(vWorldOut);
           }

// Assign output returned by nlfunc. 

           world[0] = vWorldOut[0];
           world[1] = vWorldOut[1];

// Overwrite the value for the spectral axis if doing velocity conversions

           if (WCCSNLAxisLabellerVelAxis != -1) {

// Convert absolute frequency to absolute velocity

              if (!WCCSNLAxisLabellerSpecCoord.frequencyToVelocity (velocity, frequency)) {
                 err = 3;
              } else {
                 world[WCCSNLAxisLabellerVelAxis] = velocity;
              }

// Now make relative if required; rel = abs - ref

              if (!WCCSNLAxisLabellerAbsolute) {
                 world[WCCSNLAxisLabellerVelAxis] -= WCCSNLAxisLabellerVelRef;
              }
           }
   	} else {
	  err = 3;
	}
	break;
      }
    default:
      err = 1;
    }
//
    *ierr = err;
  }
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


////////////////////////////////////////////////////////////////////////


Bool WCCSNLAxisLabeller::draw(const WCRefreshEvent &ev) 
{
  WorldCanvas *wc = ev.worldCanvas();
  Bool dolist = wc->pixelCanvas()->supportsLists();
  //wc->verifyPGFilterAlignment();
  PixelCanvas *pc = wc->pixelCanvas();

  AttributeBuffer worldCanvasState;
  if (dolist) {

// figure out if we can draw the cached list

    Double linMinX = wc->linXMin();
    Double linMaxX = wc->linXMax();
    Double linMinY = wc->linYMin();
    Double linMaxY = wc->linYMax();
    uInt canvasDrawXSize = wc->canvasDrawXSize();
    uInt canvasDrawYSize = wc->canvasDrawYSize();
    uInt canvasDrawXOffset = wc->canvasDrawXOffset();
    uInt canvasDrawYOffset = wc->canvasDrawYOffset();
    uInt canvasXOffset = wc->canvasXOffset();
    uInt canvasYOffset = wc->canvasYOffset();
    uInt canvasXSize = wc->canvasXSize();
    uInt canvasYSize = wc->canvasYSize(); 
    
// store the parameters of this images

    worldCanvasState.set("canvasDrawXSize", canvasDrawXSize);
    worldCanvasState.set("canvasDrawYSize", canvasDrawYSize);
    worldCanvasState.set("canvasDrawXOffset", canvasDrawXOffset);
    worldCanvasState.set("canvasDrawYOffset", canvasDrawYOffset);
    worldCanvasState.set("linXMin", linMinX);
    worldCanvasState.set("linXMax", linMaxX);
    worldCanvasState.set("linYMin", linMinY);
    worldCanvasState.set("linYMax", linMaxY);
    worldCanvasState.set("canvasXOffset", canvasXOffset);
    worldCanvasState.set("canvasYOffset", canvasYOffset);
    worldCanvasState.set("canvasXSize", canvasXSize);
    worldCanvasState.set("canvasYSize", canvasYSize);
    
    worldCanvasState.set("deviceBackgroundColor", 
			 wc->pixelCanvas()->deviceBackgroundColor());
    worldCanvasState.set("deviceForegroundColor",
			 wc->pixelCanvas()->deviceForegroundColor());

    // dk note (12/03): DDs using this object and calling draw()
    // on it normally invoke invalidate() in their notifyUnregister(wch),
    // assuring that itsLastWorldCanvas still exists whenever
    // itsValid==True.  Note that it _is_ the responsibility of the
    // caller of draw() to assure this object is invalidated before
    // the corresponding wc is destroyed.

    if (itsValid && itsLastWorldCanvas->validList(itsDrawListNumber)) {

      if(itsLastWorldCanvas==wc &&
         itsDrawStateBuffer.matches(worldCanvasState)) {

        // reuse applicable drawlist.

        wc->drawList(itsDrawListNumber);
        return True;
      }

      // otherwise, delete the old drawlist.

      invalidate();
    }

    // start a new list; save the new draw state.

    itsDrawListNumber = wc->newList();
    itsLastWorldCanvas = wc;
    itsDrawStateBuffer = worldCanvasState;
    itsValid = True;
  }

 try {

// Get a copy of the CS
  
  CoordinateSystem cSys;
  
  if(useWCCS && wc->hasCS()) {
  
    // dk note:
    // As of 9/07, the preferred usage for this class is to set useWCCS True.
    // If this is done, the labeller will use the CS of the WC on which it is
    // to draw for creating labels (modified as always according to user
    // formatting preferences).  itsCoordinateSystem is still used/needed (in
    // getOptions(), e.g., for coordinate types and user interface) when the
    // WC is unknown.  If useWCCS is False (the default), itsCoordinateSystem
    // is still used for everything, as before.
    //
    // (Actually, it is preferred to use WorldAxesDD _instead_ of this class
    // in new code where possible; WorldAxesDD always uses the WC CS, and
    // can cache many renderings within a single object.  However, not all
    // labelling features of this class have been ported there yet).
    
    
    cSys = wc->coordinateSystem();
    
    // Set proper formatting, etc. onto the labelling cSys
    
    CoordinateUtil::setNiceAxisLabelUnits(cSys);  // (overkill?)

    setSpectralState(cSys);
    setDirectionState(cSys);  }

  else {	// (the old-fashioned way: use externally-set CS for all).
  
    if(!hasCoordinateSystem()) return False;	// No CS set: can't draw.

    cSys = coordinateSystem();  }


    
// We use the specAxis variable to trigger velocity conversions

  Int specAxis = -1;
  for (uInt i = 0; i < cSys.nCoordinates(); i++) {
    switch(cSys.type(i)) {
    case Coordinate::SPECTRAL:
      {

// itsDoVelocity just tells us that the user has asked for
// a km/s conformant spectral unit in the labeller (the CS state
// is already set).  We now need to see if either of the first two display axes are
// in fact Spectral.   specAxis = 0 or 1 will then 
// trigger velocity conversions

        if (itsDoVelocity) {
           specAxis = cSys.worldAxes(i)[0];
           if (specAxis>1) {
              specAxis = -1;     // Neither of display axes is spectral
           }
        }

// nlfunc needs thse

        WCCSNLAxisLabellerSpecCoordIdx = i;
        WCCSNLAxisLabellerSpecCoord = cSys.spectralCoordinate(i);

// We are going to need the velocity at the reference value
// so we can convert relative velocity back to frequency

        if (specAxis != -1) {
           Double velRef;
           if (!WCCSNLAxisLabellerSpecCoord.frequencyToVelocity (velRef, 
                       WCCSNLAxisLabellerSpecCoord.referenceValue()[0])) {
              return False;
           }
           WCCSNLAxisLabellerVelRef = velRef;
           WCCSNLAxisLabellerFreqRef = WCCSNLAxisLabellerSpecCoord.referenceValue()(0);
        }
      }
      break;
    default:
      break;
    }
  }
//
  WCCSNLAxisLabellerVelAxis = specAxis;  
  WCCSNLAxisLabellerAbsolute = itsAbsolute;      // From WCCSAxisLabeller
  WCCSNLAxisLabellerWorldLabels = itsWorldAxisLabels;
  WCCSNLAxisLabellerUIBase = uiBase();
	// Determines whether 'Absolute Pixel' labelling is from 0 or 1.

// Set up the toMix world ranges

  const uInt nWorld = cSys.nWorldAxes();
  WCCSNLAxisLabellerWorldMin.resize(nWorld);
  WCCSNLAxisLabellerWorldMax.resize(nWorld);
  WCCSNLAxisLabellerWorldMin = cSys.worldMixMin();
  WCCSNLAxisLabellerWorldMax = cSys.worldMixMax();

// Get some other bits and pieces

  Vector<Double> refPixel = cSys.referencePixel();
  WCCSNLAxisLabellerRefPix[0] = refPixel[0];
  WCCSNLAxisLabellerRefPix[1] = refPixel[1];

// Set first two axes as having a world coordinate specified.
// The rest have a pixel coordinate given by the removed pixel 
// axes replacement value

  WCCSNLAxisLabellerWorldAxes.resize(nWorld);
  WCCSNLAxisLabellerWorldAxes = False;
  WCCSNLAxisLabellerWorldAxes[0] = WCCSNLAxisLabellerWorldAxes[1] = True;

// Assign reference to CS for nlfunc

  WCCSNLAxisLabellerCoordinateSystem = &cSys;
//
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

// Set up the cartesian coordinate corners.  These are in
// reality image pixel coordinates because the coordinate
// system has been set up that way.  They are zero relative.
// For "edge"    min = -0.5, max = n + 0.5
// For "center"  min =  0.0, max = n

  blc[0] = wc->linXMin();
  blc[1] = wc->linYMin();
  trc[0] = wc->linXMax();
  trc[1] = wc->linYMax();
  labctl = 0;
  nlcprm[0] = '\0';
  
// Figure out reasonable default for opt to indicate labelling

  for (uInt i = 0; i < 2; i++) {
    WCCSNLAxisLabellerLongAxis = -1;
    WCCSNLAxisLabellerLatAxis = -1;
    Int coordinate, axisInCoord;
    cSys.findPixelAxis(coordinate, axisInCoord, i);
//
    opt[i] = ' ';
//
    Coordinate::Type cType = cSys.type(coordinate);
    if (cType==Coordinate::DIRECTION) {
	DirectionCoordinate dcoord = cSys.directionCoordinate(coordinate);
	MDirection::Types mdirtype;
        dcoord.getReferenceConversion(mdirtype);
	MDirection::GlobalTypes globtype = MDirection::globalType(mdirtype);
	if (axisInCoord==0) {
	  WCCSNLAxisLabellerLongAxis = i;
	  if (globtype==MDirection::GRADEC) {
             if (itsAbsolute) {
                opt[i] = 'G'; // hms [0, 24)
             } else {
                opt[i] = ' '; // plain numeric
             }
             if (!itsWorldAxisLabels) opt[i] = ' ';
	  } else if (globtype==MDirection::GHADEC) {
             opt[i] = ' '; // decimal - needs to be 'T' after fixing units
             if (!itsWorldAxisLabels) opt[i] = ' ';
	  } else if (globtype==MDirection::GAZEL || 
	             globtype==MDirection::GLONGLAT) {
             if (itsAbsolute) {
                opt[i] = 'A'; // decimal degrees [0, 360)
             } else {
                opt[i] = 'B'; // (-180,180]
             }
             if (!itsWorldAxisLabels) opt[i] = ' ';
          }
	} else if (axisInCoord==1) {
	  WCCSNLAxisLabellerLatAxis = i;
	  if (globtype==MDirection::GRADEC || globtype==MDirection::GHADEC) {
             if (itsAbsolute) {
                opt[i] = 'E'; // sexigesimal degrees (-180, 180]
             } else {
                opt[i] = ' '; // plain numeric
             }
             if (!itsWorldAxisLabels) opt[i] = ' ';
	  } else if (globtype==MDirection::GAZEL || 
	             globtype==MDirection::GLONGLAT) {
             opt[i] = 'B'; // decimal degrees (-180, 180]
             if (!itsWorldAxisLabels) opt[i] = ' ';
	  }
	}
    }
  }
//
  wc->setColor(wc->getWorldForegroundColor());
  wc->pixelCanvas()->setCapStyle(Display::CSButt);
  wc->pixelCanvas()->setJoinStyle(Display::JSMiter);

  // grid type options
  String stemp = xGridType();
  if (stemp == "Tick marks") {
    gcode[0] = 1;
  } else if (stemp == "Full grid") {
    gcode[0] = 2;
  } 
  stemp = yGridType();
  if (stemp == "Tick marks") {
    gcode[1] = 1;
  } else if (stemp == "Full grid") {
    gcode[1] = 2;
  }

  // tick length
  tiklen = (Double)tickLength();
  
  // colors
  Float r, g, b;
  pc->getColorComponents(xGridColor(), r, g, b);
  cpgscr(16, r, g, b);
  pc->getColorComponents(yGridColor(), r, g, b);
  cpgscr(17, r, g, b);
  pc->getColorComponents(xAxisTextColor(), r, g, b);
  cpgscr(18, r, g, b);
  pc->getColorComponents(yAxisTextColor(), r, g, b);
  cpgscr(19, r, g, b);
  pc->getColorComponents(xAxisTextColor(), r, g, b);
  cpgscr(20, r, g, b);
  pc->getColorComponents(yAxisTextColor(), r, g, b);
  cpgscr(21, r, g, b);
  pc->getColorComponents(titleTextColor(), r, g, b);
  cpgscr(22, r, g, b);
  pc->getColorComponents(plotOutlineColor(), r, g, b);
  cpgscr(23, r, g, b);
  pc->getColorComponents("background", r, g, b);
  cpgscr(24, r, g, b);

  // width
  pc->setLineWidth(lineWidth());
  uInt nl = 80;
  char *idents = new Char[nl * 3];
  uInt ididx;
  
  String xAxTxt=xAxisText(wc),
         yAxTxt=yAxisText(wc);
  
  strncpy(idents + 0, xAxTxt.chars(), nl);
  for (ididx = xAxTxt.length(); ididx < nl; ididx++) {
    idents[0 + ididx] = 32;
  }
  strncpy(idents + nl, yAxTxt.chars(), nl);
  for (ididx = yAxTxt.length(); ididx < nl; ididx++) {
    idents[nl + ididx] = 32;
  }
  strncpy(idents + 2*nl, titleText().chars(), nl);
  for (ididx = titleText().length(); ididx < nl; ididx++) {
    idents[2*nl + ididx] = 32;
  }
//
  cpgsch(charSize());
  stemp = charFont();
  if (stemp == "roman") {
      cpgscf(2);
  } else if (stemp == "italic") {
    cpgscf(3);
  } else if (stemp == "script") {
    cpgscf(4);
  } else { // "normal" or anything else
    cpgscf(1);
  }
  stemp = labelPosition();
  if (stemp == "Auto") {
    labctl = 0;
  } else if (stemp == "bottom-left") {
    labctl = 21;
  } else if (stemp == "bottom-right") {
    labctl = 2001;
  } else if (stemp == "top-right") {
    labctl = 2100;
  } else if (stemp == "top-left") {
    labctl = 120;
  } else {
    labctl = 0;
  }


  
// Do the real work
  cpgsbox(blc, trc, idents, opt, labctl, labden, ci, gcode,
	  tiklen, ng1, grid1, ng2, grid2, doeq, nlfunc, 
	  nlc, nli, nld, nlcprm, nliprm, nldprm,
	  nc, ic, cache, ierr);
//  
  if (plotOutline()) {
    cpgsci(23);
    cpgbox("BC", 0.0, 0, "BC", 0.0, 0);
  }
//

  if ( zLabelType() != "none" && ! display::state::instance().fileOutputMode( ) ) {

    // store bgcolor
    Int bgcol;
    cpgqtbg(&bgcol);
    cpgstbg(24);

    String zLabel;    
    if (zLabelType() == "pixel") {
      if (itsZIndex > -1) {
	ostringstream oss;
	oss << "Channel " << (itsZIndex+uiBase());
	String zlab(oss);
	cpgmtxt("T",-1.75,0.95,1.0,zlab.chars());
      }
    } else if (zLabelType() == "world") {
      CoordinateSystem cs = coordinateSystem();
	// itsCoordinateSystem used advisedly here even when useWCCS==True:
	// itsCoordinateSystem may have better information about our owner's
	// 'movie axis' than the WC CS (esp. in the case where our 'owner'
	// is an Image PADD with a spectral axis, but the 'CS master' (which
	// sets the WC CS) does _not_ have a spectral axis).
      if (cs.nWorldAxes() > 2) {
	Vector<Double> tPix,tWrld;

// We have a removed pixel axis for the movie axis....
// The formatter for any SpectralCoordinate has already been
// set up to use km/s or Hz conformant units according to
// itsSpectralUnit

        tPix = cs.referencePixel();
	String tStr;
        if (!cs.toWorld(tWrld,tPix)) {
        } else {
           zLabel = cs.format(tStr, Coordinate::DEFAULT, tWrld(2), 2);
        }
//
	zLabel += " ";
	zLabel += tStr;
	cpgmtxt("T",-1.75,0.95,1.0,zLabel.chars());
      }
    }
    cpgstbg(bgcol);
  }


  delete [] idents;
 
 }	// try
 
 catch (const casa::AipsError& err) {
   //cerr<<"WCN Err:"<<err.getMesg()<<endl;	//#diag
   if (dolist) {
     wc->endList();	// At least clean up drawlist state...
     invalidate();  }
   throw;  }		//  ...before passing exception on.

 catch (...) {
   //cerr<<"WCN Err"<<endl;			//#diag
   if (dolist) {
     wc->endList();
     invalidate();  }
   throw;  }


 if (dolist) {
   wc->endList();                      // finish the drawlist
   wc->drawList(itsDrawListNumber);    // draw the new drawlist
 }

 return True;
}


void WCCSNLAxisLabeller::invalidate() {
  if(itsValid && itsLastWorldCanvas->validList(itsDrawListNumber))
     itsLastWorldCanvas->deleteList(itsDrawListNumber);
  itsValid = False;
}
    

void WCCSNLAxisLabeller::setCoordinateSystem(const CoordinateSystem& cSys) 
{

// Any DirectionCoordinate must be in degrees as this is assumed
// in the binding to pgplot in this class. So make sure here.

   CoordinateSystem cSys2(cSys);
   CoordinateUtil::setDirectionUnit (cSys2, String("deg"));
   WCCSAxisLabeller::setCoordinateSystem(cSys2);
}

void WCCSNLAxisLabeller::setDefaultOptions() 
{
  WCCSAxisLabeller::setDefaultOptions();
}

Bool WCCSNLAxisLabeller::setOptions(const Record &rec, Record &updatedOptions) 
{
  Bool ret = WCCSAxisLabeller::setOptions(rec, updatedOptions);
  Bool localchange = False;
//
  ret = (ret || localchange);
  return ret;
}

Record WCCSNLAxisLabeller::getOptions() const 
{
  Record rec = WCCSAxisLabeller::getOptions();
  return rec;
}


} //# NAMESPACE CASA - END

