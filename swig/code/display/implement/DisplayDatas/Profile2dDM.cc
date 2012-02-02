//# Profile2dDM.cc: drawing for 2d Profile DisplayDatas
//# Copyright (C) 2000,2001,2003
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

#include <casa/Arrays/ArrayMath.h>
#include <lattices/Lattices/LatticeIterator.h>
#include <display/Display/WorldCanvas.h>
#include <display/DisplayDatas/Profile2dDM.h>
#include <display/DisplayDatas/Profile2dDD.h>
namespace casa { //# NAMESPACE CASA - BEGIN

//#include <cpgplot.h>

Profile2dDM::Profile2dDM(WorldCanvas *worldCanvas, 
			     AttributeBuffer *wchAttributes,
			     AttributeBuffer *ddAttributes,
			     CachingDisplayData *dd) :
  CachingDisplayMethod(worldCanvas, wchAttributes, ddAttributes, dd) {
}

Profile2dDM::~Profile2dDM() {
  cleanup();
}

Bool Profile2dDM::drawIntoList(Display::RefreshReason reason,
				 WorldCanvasHolder &wcHolder) {
  // Locate the WorldCanvas to draw upon
  WorldCanvas *wc = wcHolder.worldCanvas();
  Profile2dDD *parent = dynamic_cast<Profile2dDD *>
    (parentDisplayData());
  if (!parent) {
    throw(AipsError("Invalid parent of Profile2dDM"));
  }
  Matrix<Double> data;
  Vector<Bool> mask;
  parent->getDrawData(data);
  parent->getMaskData(mask);
  if (data.nelements()) {
    setStyles(wc, parent); 
    if (mask.nelements()) {
      // Segment unmasked data
      uInt start, finish, length;
      start = 0;
      finish = 0;
      while (finish < mask.nelements()) {
	start = finish;
	while (finish < mask.nelements()
	       && mask(finish)) {
	  finish++;
	}
	if (start != finish) {
	  // insert data into a temp matrix for display
	  Matrix<Double> subSection(finish-start, 2);
	  length = subSection.shape()(0);
	  for (uInt i=0; i < length; i++) {
	    subSection(i,0) = data(start+i, 0);
	    subSection(i,1) = data(start+i, 1);
	  }
	  wc->drawPolyline(subSection, True);
	}      
	finish++;
      }
    } else { 
      wc->drawPolyline(data, True);
    }
    Double restFrequency = parent->restFrequency();
    if (parent->showRestFrequency() && restFrequency != 0) {
      // display the Rest Frequency
      Vector<Double> a(2), b(2);
      a(0) = restFrequency;
      a(1) = parent->profileYMin();
      b(0) = a(0);
      b(1) = parent->profileYMax();
      wc->setLineStyle(Display::LSDashed);
      wc->drawLine(a, b, False);
      }
    wc->clearNonDrawArea();
    restoreStyles(wc);
  }
  
  return True;
}

void Profile2dDM::setStyles(WorldCanvas *wc, Profile2dDD *parent) {
  wc->setColor(parent->profileColor());
  wc->setLineWidth(parent->profileLineWidth());
  wc->setLineStyle(parent->profileLineStyle());
}

void Profile2dDM::restoreStyles(WorldCanvas *wc) {
  wc->setLineStyle(Display::LSSolid);
}

void Profile2dDM::cleanup() {
}

Profile2dDM::Profile2dDM() {
}

Profile2dDM::Profile2dDM(const Profile2dDM &other) :
  CachingDisplayMethod(other) {
}

void Profile2dDM::operator=(const Profile2dDM &) {
}

} //# NAMESPACE CASA - END

