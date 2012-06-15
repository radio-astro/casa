//# TBHelp.qo.h: Widget for displaying and updating the browser help files.
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
//# $Id: $#ifndef TBHELP_H_
#ifndef TBHELP_H_
#define TBHELP_H_

#include <casaqt/QtBrowser/TBHelp.ui.h>

#include <QtGui>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>

namespace casa {

// <summary>
// Widget for displaying and updating the browser help files.
// <summary>
//
// <synopsis>
// TBHelp first finds the help files (which should be located under
// %casadir%/code/casaqt/implement/QtBrowser/help/) and checks the modification
// date.  The meta help information is located in help.xml, which is then
// converted into html files (like browsing.html) if help.xml has been modified
// after the html files.  The HTML is then displayed using a QTextBrowser,
// allowing the user to browse.
// </synopsis>

class TBHelp : public QWidget, Ui::Help {
    Q_OBJECT

public:
    // Default Constructor.
    TBHelp();

    ~TBHelp();

    
    // Rebuilds the HTML files based on the XML file.  Returns true if the
    // rebuild succeeded, false otherwise.
    bool rebuild();

private slots:
    // Slot for when the rebuild button is clicked.  If the rebuild is
	// successful, the help browser is reloaded to display the most recent
	// version.
    void rebuildClicked();

private:
    // Location of the directory holding the help files.
    String helpdir;
};

}

#endif /* TBHELP_H_ */
