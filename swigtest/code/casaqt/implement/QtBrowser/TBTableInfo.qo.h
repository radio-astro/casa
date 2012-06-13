//# TBTableInfo.qo.h: Displays general information about a table.
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
#ifndef TBTABLEINFO_H_
#define TBTABLEINFO_H_

#include <casaqt/QtBrowser/TBTableInfo.ui.h>

#include <QtGui>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>
using namespace std;

namespace casa {

//# Forward Declarations
class TBField;
class TBTable;

// <summary>
//Displays general information about a table.
// </summary>
//
// <synopsis>
// TBTableInfo is a widget that displays the following table information:
// location, total rows, can insert rows, can delete rows, and fields.  For
// each field the name and type is shown as well as any additional information
// if it is provided.
// </synopsis>

class TBTableInfo : public QWidget, Ui::TableInfo {
    Q_OBJECT

public:
	// Constructor that takes the table to be displayed.
    TBTableInfo(TBTable* table);

    ~TBTableInfo();

private:
	// Table pointer.
    TBTable* table;

    
    // Sets the fields information in the QTableWidget.
    void setFields(vector<TBField*>* fields);
};

}

#endif /* TBTABLEINFO_H_ */
