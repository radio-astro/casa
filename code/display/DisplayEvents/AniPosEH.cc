//# AniPosEH.cc: Animation position event handler for a WorldCanvas
//# Copyright (C) 1996,1997,1998,1999,2000
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
#include <display/Display/WorldCanvas.h>
#include <display/Display/WorldCanvasHolder.h>
#include <display/DisplayEvents/WCPositionEvent.h>
#include <display/Display/Attribute.h>
#include <display/DisplayEvents/Animator.h>
#include <display/DisplayEvents/AniPosEH.h>

namespace casa { //# NAMESPACE CASA - BEGIN

AniPosEH::AniPosEH() {
}

AniPosEH::~AniPosEH() {
  //should cleanup the worldcanvasHolders and the worldcanvases here
};

// The operator invoked by the WorldCanvas
void AniPosEH::operator()(const WCPositionEvent& ev)
{
  // only listen to keystroke down
  if (!ev.keystate())
    return;
  
  // listen to certain keystrokes and mouse clicks
  switch ( ev.key() ) {
    // default is to do nothing
  default: break;
  case Display::K_plus :
  case Display::K_equal : {
    animator.nextCoord();
    break;
  }
  
  case Display::K_minus : {
    animator.prevCoord();
    break;
  }
  
  case Display::K_z : {
    // create new restriction for the axes
    Attribute xAtt("xaxisname", "x-as");
    Attribute yAtt("yaxisname", "y-as");
    cout << "X axis vs Y axis" << endl;

    // write them to the WorldCanvasHolders
    setRestriction(xAtt);
    setRestriction(yAtt);

    // let the DisplayData cleanup to save memory
    cleanup();
    
    // reset the Animator
    animator.reset();    
    
    // refresh all
    refresh();
    
    break;
  }
  case Display::K_y : {
    // create new restrictions for the axes
    Attribute xAtt("xaxisname", "x-as");
    Attribute yAtt("yaxisname", "z-as");
    cout << "X axis vs Z axis" << endl;

    // write them to the WorldCanvasHolders
    setRestriction(xAtt);
    setRestriction(yAtt);

    // reset Animator
    animator.reset();

    // let the DisplayData cleanup to save memory
    cleanup();
    
    // and refresh the canvases
    refresh();
    break;
  }
  case Display::K_x : {
    // create new restrictions for the axes
    Attribute xAtt("xaxisname", "y-as");
    Attribute yAtt("yaxisname", "z-as");
    cout << "Y axis vs Z axis" << endl;

    // write them to the WorldCanvasHolders
    setRestriction(xAtt);
    setRestriction(yAtt);

    // reset the Animator
    animator.reset();
    // let the DisplayData cleanup to save memory
    cleanup();
    
    // and refresh the canvas
    refresh();
    break;
  }
  
  }  // end of switch   

}


// Add a new WorldCanvasHolder to the list of WorldCanvasHolder that are
// controlled by this Animator
void  AniPosEH::addWorldCanvasHolder(WorldCanvasHolder *wcHolder)
{
  if (wcHolder == 0) {
    throw(AipsError("AniPosEH::addWorldCanvasHolder - "
                    "null pointer passed"));
  }
  
   // create iterator
  ListIter<void *> it(&holderList);

  // add to List
  it.addRight((void *) wcHolder);

  animator.addWorldCanvasHolder(wcHolder);
  //wcHolder->registerAniPosEH(this);
  wcHolder->worldCanvas()->addPositionEventHandler(*this);

  animator.reset();
  
}


// remove a WorldCanvasHolder from the buffer
void AniPosEH::removeWorldCanvasHolder(WorldCanvasHolder& wcHolder)
{
  // create iterator
  ListIter<void *> it(&holderList);

  // while something in the list
  while(!it.atEnd()) {
    // check if this is the one
    if (it.getRight() == (void *) &wcHolder) {
      // if so, delete
      it.removeRight();
      wcHolder.worldCanvas()->removePositionEventHandler(*this);
      //wcHolder.unregisterAniPosEH(*this);
      animator.removeWorldCanvasHolder(wcHolder);
      // addWorldCanvasHolder does not prevent to store the same WorldCanvas
      // twice, so we should continue iterating, so we cannot do here: break;
    } else {
      it++;
    }
  }

  animator.reset();
  
}

void AniPosEH::setRestriction(Attribute& att) 
{
  // create iterator
  ListIter<void *> it(&holderList);
  
  // and pointer to a WorldCanvasHolder
  WorldCanvasHolder *wcHolder;
  
  // while something in the list
  while(!it.atEnd()) {
    // get pointer
    wcHolder = (WorldCanvasHolder *) it.getRight();
    // set restriction
    wcHolder->setRestriction(att);
    // go to next WorldCanvasHolder
    it++;
  }
}

void AniPosEH::cleanup()
{  
  // create iterator
  ListIter<void *> it(&holderList);
  
  // and pointer to a WorldCanvasHolder
  WorldCanvasHolder *wcHolder;
  
  // while something in the list
  while(!it.atEnd()) {
    // get pointer
    wcHolder = (WorldCanvasHolder *) it.getRight();
    // cleanup
    wcHolder->cleanup();
    // go to next WorldCanvasHolder
    it++;
  }
}

void AniPosEH::refresh()
{  
  // create iterator
  ListIter<void *> it(&holderList);
  
  // and pointer to a WorldCanvasHolder
  WorldCanvasHolder *wcHolder;
  
  // while something in the list
  while(!it.atEnd()) {
    // get pointer
    wcHolder = (WorldCanvasHolder *) it.getRight();
    // cleanup
    wcHolder->refresh();
    // go to next WorldCanvasHolder
    it++;
  }
}


} //# NAMESPACE CASA - END

