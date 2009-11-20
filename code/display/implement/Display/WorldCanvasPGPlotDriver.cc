//# WorldCanvasPGPlotDriver.cc: PGPLOT driver for the WorldCanvas
//# Copyright (C) 1993,1994,1995,1996,1998,1999,2000,2001
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
//#$Id$

#include <display/Display/WorldCanvasPGPlotDriver.h>
#include <casa/Exceptions/Error.h>
#include <casa/Utilities/Assert.h>
#include <casa/iostream.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// upto 20 open devices at once
#define PGPLOT_MAX_SELECTIONS 20
static WorldCanvas * wca_[PGPLOT_MAX_SELECTIONS];
static WorldCanvas * wc_ = 0;
static WorldCanvas * wcq_ = 0;
static PixelCanvas * pc_ = 0;
static Bool selectIDs_[PGPLOT_MAX_SELECTIONS];
static Int plotID_ = 0;
static Int selectID_ = 0;
static Int polygonFillCount_[PGPLOT_MAX_SELECTIONS];
static Bool initialized_ = False;

void pgplot_init() {
  for (uInt i = 1; i < PGPLOT_MAX_SELECTIONS; i++) {
    wca_[i] = 0;
    selectIDs_[i] = False;
    polygonFillCount_[i] = 0;
  }
  initialized_ = True;
}

// C++ public function, not used by driver
WorldCanvas * pgplot_queryWorldCanvas(Int selectID) {
  if (selectID < 1 || selectID >= PGPLOT_MAX_SELECTIONS) {
    return NULL;
  } else {
    return wca_[selectID];
  }
}

void pgplot_queueWorldCanvas(WorldCanvas * wc) {
  wcq_ = wc;
}

// C++ public function, not used by driver
void pgplot_setWorldCanvas(Int selectID, WorldCanvas *wc) {
  if (selectID > 0 || selectID < PGPLOT_MAX_SELECTIONS) {
    wca_[selectID] = wc;
    if (selectID == selectID_) {
      wc_ = wca_[selectID]; 
      pc_ = wc_->pixelCanvas(); 
    }
  }
}

// driver helper function
static Int pgplot_allocateID() {
  if (!initialized_) {
    pgplot_init();
  }
  for (uInt i = 1; i < PGPLOT_MAX_SELECTIONS; i++) {
    if (selectIDs_[i] == False) {
      selectIDs_[i] = True;
      return i;
    }
  }
  return -1;
}

// driver helper function
static void pgplot_freeID(Int id) {
  selectIDs_[id] = False;
}

// helper function
int returnString(char * out, const char * in, int outlen) {
  int inlen = strlen(in);
  int n = (inlen > outlen) ? outlen : inlen;
  strncpy(out, in, n);
  for (int i = 0; i < outlen-n; i++) {
    out[i] = ' ';
  }
  return inlen;
}

// called by fortran
extern "C" {
#if defined(__APPLE__)  && ! (!defined(__MAC_10_6) ||defined(__USE_WS_X11__))
void wcdriv_(int * opc, float * rbuf, int * nbuf,
	     char * chr, int * lchr, int len)
#else
void wcdriv_(int * opc, float * rbuf, int * nbuf,
	     char * chr, int * lchr, int * /* mode */, int len)
#endif
{
  // PGPlot standard colors, black/white reversed
  // 16 additional colors for multicolor functions
  static float localPgPlotColors[32*3] = {
    0,0,0,
    1,1,1,
    1,0,0,
    0,1,0,
    0,0,1,
    0,1,1,
    1,0,1,
    1,1,0,
    1,0.5,0,
    0.5,1,0,
    0,1,0.5,
    0,0.5,1,
    0.5,0,1,
    1,0,0.5,
    0.33,0.33,0.33,
    0.67,0.67,0.67,
    0,0,0,
    0,0,0,
    0,0,0,
    0,0,0,
    0,0,0,
    0,0,0,
    0,0,0,
    0,0,0,
    0,0,0,
    0,0,0,
    0,0,0,
    0,0,0,
    0,0,0,
    0,0,0,
    0,0,0,
    0,0,0,
  };

  uInt c;
  switch (*opc)
    {
    case 1: // return device type
      *lchr = returnString(chr, "WCPGFILTER (WorldCanvas driver for DisplayLibrary)", len);
      break;
    case 2: // return maximum dimensions of view surface, color index
      // although the worldcanvas is probably using much more than 16
      // colors, we limit the color selection to 32 to allow PGPLOT
      // standard colors to be available.
      rbuf[0] = 0.0;
      rbuf[1] = -1.0;  /* Report no effective max plot width */
      rbuf[2] = 0.0;
      rbuf[3] = -1.0;  /* Report no effective max plot height */
      rbuf[4] = 0.0;	  
      rbuf[5] = 31;
      *nbuf = 6;
      break;
    case 3: // return device scale
      { 
	Float dpix, dpiy;
	pc_->pixelDensity(dpix, dpiy);
	rbuf[0] = (float)dpix;
	rbuf[1] = (float)dpiy;
      }
      rbuf[2] = 1;
      *nbuf = 3;
      break;
    case 4: // device capabilities
      *lchr = returnString(chr, "INDNTRNNYNN", len);
      break;
    case 5: // return default device
      *lchr = returnString(chr, "pgplot", len);
      break;
    case 6: // return default size of view surface
      if (!wc_)
	{
	  rbuf[0] = 0.0;
	  rbuf[1] = 1.0;
	  rbuf[2] = 0.0;
	  rbuf[3] = 1.0;
	}
      else
	{
	  rbuf[0] = 0;
	  rbuf[1] = wc_->pixelCanvas()->width() - 1;
	  rbuf[2] = 0;
	  rbuf[3] = wc_->pixelCanvas()->height() - 1;
	} 
      *nbuf = 4;
      break;
    case 7: // return miscellaneous defaults
      rbuf[0] = 1.0;
      *nbuf = 1;
      break;
    case 8: // select device
      plotID_ = (Int) rbuf[0];
      selectID_ = (Int) rbuf[1];
      wc_ = wca_[selectID_];
      pc_ = wc_->pixelCanvas();
      break;
    case 9: // open workstation
      {
	Int id = pgplot_allocateID();
	if (id == -1)
	  {
	    // failed
	    rbuf[0] = -1;
	    rbuf[1] = 0.0; // error
	    *nbuf = 2;
	  }
	else
	  {
	    // success
	    if (!wcq_)
	      throw(AipsError("No WorldCanvas queued for wcdriv_ op 9"));
	    pgplot_setWorldCanvas(id, wcq_);
	    wcq_ = 0;
	    Float idf = (float) id;
	    //cout << "setting id (rbuf[0]) to float value " << idf << endl;
	    rbuf[0] = idf;
	    rbuf[1] = 1.0;
	    *nbuf = 2;
	    
	    selectID_ = id;
	    wc_ = wca_[selectID_];
	    pc_ = wc_->pixelCanvas();
	    //cout << "selected id is      : " << selectID_ << endl;
	    //cout << "WC for that id is: " << (void *) wc_ << endl;
	  }
      }
      break;
    case 10: // close workstation
      pgplot_freeID(selectID_);
      break;
    case 11: // begin picture
      break;
    case 12: // draw line
      pc_->bufferLine(rbuf[0], rbuf[1], rbuf[2], rbuf[3]);
      break;
    case 13: // draw dot
      pc_->bufferPoint(rbuf[0], rbuf[1]);
      break;
    case 14: // end picture
      pc_->flushBuffer();
      break;
    case 15: // set color index
      pc_->flushBuffer();
      pc_->setRGBColor(localPgPlotColors[((uInt)rbuf[0])*3],
      		       localPgPlotColors[((uInt)rbuf[0])*3+1],
      		       localPgPlotColors[((uInt)rbuf[0])*3+2]);
      break;
    case 16: // flush buffer
      pc_->flushBuffer();
      break;
    case 17: // read cursor
      break;
    case 18: // Erase alpha screen
      break;
    case 19: // Set line style
      pc_->flushBuffer();
      switch((uInt)(rbuf[0]))
	{
	case 1:
	  pc_->setLineStyle(Display::LSSolid);
	  break;
	case 2:
	case 4:
	  pc_->setLineStyle(Display::LSDashed);
	  break;
	case 3:
	case 5:
	  pc_->setLineStyle(Display::LSDoubleDashed);
	  break;
	}
      break;
    case 20: // Polygon fill
      // Need to enable filling
      pc_->setFillStyle(Display::FSSolid);
      if (polygonFillCount_[selectID_] == 0)
	{
	  polygonFillCount_[selectID_] = (Int) rbuf[0];
	  pc_->flushBuffer();
	}
      else
	{
	  polygonFillCount_[selectID_]--;
	  pc_->bufferPolygonPoint((Int)rbuf[0], (Int)rbuf[1]);
	  if (polygonFillCount_[selectID_] == 0)
	    {
	      pc_->flushBuffer();
	    }
	}
      break;
    case 21: // set color values
      c = (uInt)(rbuf[0]);
      if (c < 32) {
	localPgPlotColors[c * 3] = rbuf[1];
	localPgPlotColors[c * 3 + 1] = rbuf[2];
	localPgPlotColors[c * 3 + 2] = rbuf[3];
      }
      break;
    case 22: // set line width
      //pc_->setLineWidth((uInt)(rbuf[0]/10));
      pc_->setLineWidth(rbuf[0]/10.0);
      break;
    case 23: // escape function
      break;
    case 24: // rectangle fill
      pc_->setFillStyle(Display::FSSolid);
      pc_->drawRectangle(rbuf[0], rbuf[1], rbuf[2], rbuf[3]);
      break;
    case 25: // set fill pattern
      break;
    case 26: // Image
      break;
    case 27: // Cursor scaling info
      break;
    case 28: // draw marker
      // disabled
      break;
    case 29: // query color values
      c = (uInt) (rbuf[0]);
      rbuf[1] = localPgPlotColors[c*3];
      rbuf[2] = localPgPlotColors[c*3+1];
      rbuf[3] = localPgPlotColors[c*3+2];
      break;
    case 30: // scroll
      break;
    }
}

}
      

} //# NAMESPACE CASA - END

