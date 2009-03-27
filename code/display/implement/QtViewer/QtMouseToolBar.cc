//# QtMouseToolBar.cc: 'mouse-tool' toolbar for qtviewer display panel.
//# Copyright (C) 2005
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
//#

#include <casa/iostream.h>
#include <display/QtViewer/QtMouseToolBar.qo.h>
#include <display/QtViewer/QtDisplayPanel.qo.h>


namespace casa { //# NAMESPACE CASA - BEGIN

  
  
QtMouseToolBar::QtMouseToolBar(QtMouseToolState* msbtns, QtDisplayPanel* qdp,
			       QWidget* parent) :
	QToolBar("Mouse Toolbar", parent),
	msbtns_(msbtns) {
  
  // Use a default set of mouse tools, unless specified by a qdp.

  using namespace QtMouseToolNames;
	// Constants (nTools, names, etc.) used by Qt mouse tools.
	// Definitions in QtMouseToolState.cc.
  
  Vector<String> tools;
	// (Overrides a def. in namespace above: these are the tool names
	// for this particular toolbar, not necessarily all tools).  
  
  if(qdp==0 || qdp->mouseToolNames().nelements()==0) {
    tools.resize(8);
    tools[0]=ZOOM;            tools[1]=PAN;       tools[2]=SHIFTSLOPE;
    tools[3]=BRIGHTCONTRAST;  tools[4]=POSITION;  tools[5]=RECTANGLE;
    tools[6]=POLYGON;         tools[7]=POLYLINE;  }
  else tools = qdp->mouseToolNames();

  
  // Create tool buttons ('actions') within the toolbar.
    
  for(uInt i=0; i<tools.nelements(); i++) {
    
    String tool = tools[i];
    
    QtMouseToolButton* mtb = new QtMouseToolButton(this);
    addWidget(mtb);
    
    mtb->setObjectName(tool.chars());
    mtb->setText(tool.chars());
    mtb->setToolTip( ("Click here with the desired mouse button "
		      "to assign that button to \n\'" + longName(tool) +
		      "\'\n" + help(tool)) . chars() );

    // Originally, tool buttons are created as unassigned to mouse buttons.
    // This will change via calls to chgMouseBtn_(), even in initialization.
    
    mtb->setIcon(QIcon( (":/icons/" + iconName(tool) + "0.png").chars() ));
    mtb->setCheckable(True);
    
    // Pressing a button will order a button assignment change from the
    // central registry.
    connect( mtb,     SIGNAL(mouseToolBtnPress(String, Int)),
             msbtns_,   SLOT(chgMouseBtn      (String, Int)) );  }
    


  // Keeps this toolbar up-to-date with central button-state registry.
  connect( msbtns_, SIGNAL(mouseBtnChg (String, Int)),
                      SLOT(chgMouseBtn_(String, Int)) );
      
  msbtns_->emitBtns();  }


   
     
void QtMouseToolBar::chgMouseBtn_(String tool, Int button) {
  // Connected to the QtMouseToolState::mouseBtnChg() signal.  Changes the
  // tool button's (QAction's) state (icon, whether checked), to reflect
  // the [new] mouse button assignment of a given mouse tool.
  
  using namespace QtMouseToolNames;
  
  QToolButton* mtb = findChild<QToolButton*>(tool.chars());
  if(mtb==0) return;	// (shouldn't happen).
  ostringstream os; os << button;
  mtb->setIcon(QIcon(
       (":/icons/"+iconName(tool)+String(os)+".png") . chars()  ));
  mtb->setChecked(button!=0);  }
  

  
} //# NAMESPACE CASA - END
    
