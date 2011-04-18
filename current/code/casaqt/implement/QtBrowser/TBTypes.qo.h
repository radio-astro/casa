//# TBTypes.qo.h: (Not finished) Widget for entering a value based on its type.
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
//# $Id: $
#ifndef TBTYPES_H_
#define TBTYPES_H_

#include <casaqt/QtBrowser/TBTypes.ui.h>

#include <QtGui>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

// <summary>
// (Not finished) Widget for entering a value based on its type.
// </summary>
//
// <synopsis>
// TBTypes is a widget with a number of different panels.  Each panel has
// a different type of input that correspondings to different data types.
// (For example, when entering a number a text field is used whereas entering
// a complex has two spinners.)  Using the TBTypes::setType() method, the
// different panels are shown or hidden as necessary.  Important: the
// functionality of actually collecting the input is not yet implemented.
// </synopsis>

class TBTypes : public QWidget, Ui::Types {
    Q_OBJECT

public:
	// Constructor that takes the starting type and whether or not to include
	// arrays in the list of types available.
    TBTypes(String type, bool incArrays = true);

    ~TBTypes();

    
    // Sets the current display to allow values for the given type.
    void setType(String type);

private:
	// Current type.
    String type;
    
    // All available types.
    vector<String>* types;
    
    // TBTypes panel for entering single array cell values.
    TBTypes* arrayTypes;
    
    // TBTypes panel for entering single record values.
    TBTypes* recordTypes;

    
    // Sets up the display needed for arrays.
    void setupArray();

    // Sets up the display needed for records.
    void setupRecord();

private slots:
	// Slot for when the user selects a type in the record chooser.
    void recordChooserChanged(int index);

    // Slot for when the user enters the dimensions for an array.
    void arrayDimSet();

    // Slot for when the user sets the value of an array cell.  (Not
    // implemented.)
    void arrayValSet();
};

}

#endif
