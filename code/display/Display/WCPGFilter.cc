//# WCPGFilter.cc: interface between WorldCanvas and its PGPLOT driver
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
//# $Id$

#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <display/Display/WCPGFilter.h>
#include <display/Display/WorldCanvasPGPlotDriver.h>
#include <cpgplot.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class PGArrF {

public:
  
  PGArrF(const Array<Float> &thevec) :
    arr_p(thevec) { ptr_p = arr_p.getStorage(delete_p);}
  ~PGArrF() { arr_p.freeStorage(ptr_p, delete_p); }
  
  const Float *ptr() { return ptr_p; }
  int n()  { return (int) arr_p.nelements(); }
  int nx() { return (int) arr_p.shape()(0);  }
  int ny() { return (int) arr_p.shape()(1);  }
  
private:

  PGArrF();
  PGArrF(const PGArrF&);
  PGArrF &operator=(const PGArrF&);
  
  const Array<Float> &arr_p;
  const Float *ptr_p;
  Bool delete_p;
};

// Constructor.
WCPGFilter::WCPGFilter(WorldCanvas *worldcanvas) : 
  itsWorldCanvas(worldcanvas),
  itsRefCount(0) {

  // queue up the world canvas to be used the next time cpgopen is
  // called
  pgplot_queueWorldCanvas(itsWorldCanvas);

  // open the workstation, associate the id with the world canvas, and
  // initialize PGPLOT for this id
  itsPgPlotID = cpgopen("/WCPGFILTER");

  // turn off "next page" requests
  cpgask(0);

  // align pgplot's coordinate grid to the world canvas's linear
  // system
  //realign();
  // This now called by WorldCanvas acquiring / releasing the filter.
}

// Destructor.
WCPGFilter::~WCPGFilter() {
  cpgslct(itsPgPlotID);
  cpgclos();
}

// Realign the PGPLOT domain to the WorldCanvas.
Bool WCPGFilter::realign(const Bool &linear) {

  Float x1 = Float(itsWorldCanvas->canvasDrawXOffset() + 
		   itsWorldCanvas->canvasXOffset()) / 
    Float(itsWorldCanvas->pixelCanvas()->width() - 1.0);
  Float x2 = Float(itsWorldCanvas->canvasDrawXOffset() + 
		   itsWorldCanvas->canvasXOffset() 
		   + itsWorldCanvas->canvasDrawXSize()) / 
    Float(itsWorldCanvas->pixelCanvas()->width() - 1.0);
  Float y1 = Float(itsWorldCanvas->canvasDrawYOffset() + 
		   itsWorldCanvas->canvasYOffset()) /
    Float(itsWorldCanvas->pixelCanvas()->height() -1.0);
  Float y2 = Float(itsWorldCanvas->canvasDrawYOffset() + 
		   itsWorldCanvas->canvasYOffset()
		   + itsWorldCanvas->canvasDrawYSize()) / 
    Float(itsWorldCanvas->pixelCanvas()->height() -1.0);
  cpgslct(itsPgPlotID);
  //cerr << x1 << ", " << x2 << ", " << y1 << ", " << y2 << endl;
  cpgsvp(x1, x2, y1, y2);
  if (linear) {
    cpgswin(itsWorldCanvas->linXMin(), itsWorldCanvas->linXMax(), 
	    itsWorldCanvas->linYMin(), itsWorldCanvas->linYMax());
  } else {
    Vector<Double> linBlc(2), linTrc(2), wBlc(2), wTrc(2);
    linBlc(0) = itsWorldCanvas->linXMin();
    linBlc(1) = itsWorldCanvas->linYMin();
    linTrc(0) = itsWorldCanvas->linXMax();
    linTrc(1) = itsWorldCanvas->linYMax();
    itsWorldCanvas->linToWorld(wBlc, linBlc);
    itsWorldCanvas->linToWorld(wTrc, linTrc);
    cpgswin(wBlc(0), wTrc(0), wBlc(1), wTrc(1));
  }
  cpgpage();
  return True;
}

void WCPGFilter::cont(const Matrix<Float> &m,
		      const Vector<Float> &levels,
		      const Matrix<Float> &tr)
{
  PGArrF pgf_m(m);
  PGArrF pgf_l(levels);
  PGArrF pgf_tr(tr);
  cpgslct(itsPgPlotID);
//
  String attNeg("dashNegativeContours");
  String attPos("dashPositiveContours");
  Bool dashNeg, dashPos;
  if (itsWorldCanvas->existsAttribute(attNeg)) {
    itsWorldCanvas->getAttributeValue(attNeg, dashNeg);
  }
  if (itsWorldCanvas->existsAttribute(attPos)) {
    itsWorldCanvas->getAttributeValue(attPos, dashPos);
  }
//
  Int ls;
  cpgqls(&ls);
//
  Vector<Float> neg,pos;
  splitContours(neg, pos, levels);
//
  Int ls2 = 1;
  if (dashNeg) ls2 = 2;
  if (neg.nelements()>0) {  
     cpgsls(ls2);
     PGArrF pgf_l(neg);
     cpgcont(pgf_m.ptr(), pgf_m.nx(), pgf_m.ny(), 
             1, pgf_m.nx(), 1, pgf_m.ny(),
             pgf_l.ptr(), -neg.nelements(), pgf_tr.ptr());
  }
//
  ls2 = 1;
  if (dashPos) ls2 = 2;
  if (pos.nelements()>0) {  
     cpgsls(ls2);
     PGArrF pgf_l(pos);
     cpgcont(pgf_m.ptr(), pgf_m.nx(), pgf_m.ny(), 
             1, pgf_m.nx(), 1, pgf_m.ny(),
             pgf_l.ptr(), -pos.nelements(), pgf_tr.ptr());
  }
//
  cpgsls(ls);
}


void WCPGFilter::conb(const Matrix<Float> &m,
                      const Vector<Float> &levels,
                      const Matrix<Float> &tr,
                      const Float blank)
{
  PGArrF pgf_m(m);
  PGArrF pgf_l(levels);
  PGArrF pgf_tr(tr);
  cpgslct(itsPgPlotID);
//
  String attNeg("dashNegativeContours");
  String attPos("dashPositiveContours");
  Bool dashNeg, dashPos;
  if (itsWorldCanvas->existsAttribute(attNeg)) {
    itsWorldCanvas->getAttributeValue(attNeg, dashNeg);
  }
  if (itsWorldCanvas->existsAttribute(attPos)) {
    itsWorldCanvas->getAttributeValue(attPos, dashPos);
  }
//
  Int ls;
  cpgqls(&ls);
//
  Vector<Float> neg,pos;
  splitContours(neg, pos, levels);
//
  Int ls2 = 1;
  if (dashNeg) ls2 = 2;
  if (neg.nelements()>0) {  
     cpgsls(ls2);
     PGArrF pgf_l(neg);
     cpgconb(pgf_m.ptr(), pgf_m.nx(), pgf_m.ny(),
             1, pgf_m.nx(), 1, pgf_m.ny(),
             pgf_l.ptr(), neg.nelements(), pgf_tr.ptr(), blank);
  }
//
  ls2 = 1;
  if (dashPos) ls2 = 2;
  if (pos.nelements()>0) {  
     cpgsls(ls2);
     PGArrF pgf_l(pos);
     cpgconb(pgf_m.ptr(), pgf_m.nx(), pgf_m.ny(),
             1, pgf_m.nx(), 1, pgf_m.ny(),
             pgf_l.ptr(), pos.nelements(), pgf_tr.ptr(), blank);

  }
//
  cpgsls(ls);
}



void WCPGFilter::splitContours (Vector<Float>& neg, Vector<Float>& pos,
                                const Vector<Float>& levels)
{
   neg.resize(levels.nelements());
   pos.resize(levels.nelements());
//
   Int iPos = 0;
   Int iNeg = 0;
   for (uInt i=0; i<levels.nelements(); i++) {
      if (levels[i]<0) {
         neg[iNeg++] = levels[i];
      } else {
         pos[iPos++] = levels[i];
      }
   }
   pos.resize(iPos,True);
   neg.resize(iNeg,True);
}


} //# NAMESPACE CASA - END

