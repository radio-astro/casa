//# DParameterButton.h: class to package interface to button autoguis.
//# Copyright (C) 2000,2002
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

#ifndef TRIALDISPLAY_DPARAMETERBUTTON_H
#define TRIALDISPLAY_DPARAMETERBUTTON_H

#include <casa/aips.h>
#include <display/Display/DisplayParameter.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// <summary>
// Implementation of DisplayParameter to package interface to button autoguis.
// </summary>

// <use visibility=export>

// <reviewed reviewer="" date="" test="" demos="">
// </reviewed>

// <prerequisite>
// <li> <linkto class=DisplayParameter>DisplayParameter</linkto>
// </prerequisite>

// <etymology>
// DParameterButton is an implementation of a DisplayParameter for Buttons.
// It defines the necessary information to define a button in autogui.g,
// and packages it into the parameter record field autogui requires.
//
// </etymology>

// <synopsis>
// This class is derived from <linkto
// class=DisplayParameter>DisplayParameter</linkto>
// It holds the meta-information neccessary to define a button widget,
// packaging it into the 'parameter record' field required by autogui.g.
// It can determine whether the button was pressed.
// The 'value' has no meaning here (it will be True).
// </synopsis>

// <example>

// <srcblock>
// DParameterButton pbtn("unique_btn_id", "Label Beside Button",
//			 "Text on Button", "Help Message for Button");
//
// Record rec;			// (usually within 'getoptions()')
// pstring.toRecord(rec);	// rec is returned to autogui.g
//				// to define the button.
//
// // ...when inRec is received recording user gui updates
// // (usually within 'setOptions()'):
// 
// if(pbtn.fromRecord(inRec)) /* button was pressed; respond */ ;
//
// </srcblock>
// </example>

// <motivation>
// Allows buttons to be defined using the same compact DParameter
// interface used for other autogui widgets.
// </motivation>

// <thrown>
// <li> None.
// </thrown>

class DParameterButton : public DisplayParameter {

 public:

  // Constructor taking the name (id) of the button, a label which will
  // appear beside the button, and the text which will appear on the button.
  // Optionally, popup help text, 'context' string, and a flag indicating
  // whether the button is enabled (disable is not fully supported yet).
  // The context string determines which autogui rollup the button will
  // appear in (replace spaces with underscores in this string).
  DParameterButton(const String name, const String description,
		   const String text, const String help="",
		   const String context="", const Bool editable=True);

  // copy constructor.
  DParameterButton(const DParameterButton &other);

  // Destructor.
  virtual ~DParameterButton();

  // copy assignment.
  DParameterButton &operator=(const DParameterButton &other);

  // Determine whether the record (passed originally from autogui.g)
  // indicates that the button was pressed.
  virtual Bool fromRecord(const RecordInterface &record);

  // Place a record describing this button in a sub-field of
  // <src>record</src> with name matching that of this parameter.  If
  // <src>overwrite</src> is <src>True</src>, then any existing field
  // with matching name will be overwritten.  If <src>fullrecord</src>
  // is <src>True</src>, then a complete description of the parameter
  // is given, otherwise just its current value is stored in
  // <src>record</src>.  (At present, <src>fullrecord</src> is ignored;
  // the full record is always returned).
  virtual void toRecord(RecordInterface &record, const Bool fullrecord = True,
			const Bool overwrite = False);

 private:

  // default constructor disabled--do not use.
  DParameterButton() : DisplayParameter(), itsText("") {  }

  // Store for the text on the button.
  String itsText;

};


} //# NAMESPACE CASA - END

#endif
