//# TBConnection.qo.h: Dialog for opening/connecting to a table with options.
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
#ifndef TBCONNECTION_H_
#define TBCONNECTION_H_

#include <casaqt/QtBrowser/TBConnection.ui.h>

#include <QtGui>
#include <casa/BasicSL/String.h>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class TBMain;

// <summary>
// Dialog for opening/connecting to a table with options
// <summary>
//
// <synopsis>
// TBConnection is a dialog to open a table with user-specified options.
// <b>Important</b>: this is not currently supported, but in the future if a
// client/server architecture is used then this widget can be used for
// connecting to a remote table.  Other options include the driver type: direct
// (using the Table module code for a local table) or XML.  TBConnection
// displays helpful explanation text when the user clicks on the different
// options.
// </synopsis>

class TBConnection : public QDialog, Ui::Connection {
    Q_OBJECT

public:
    // Builds a connection dialog with the given TBMain.  If parent is NULL,
	// then the connection is displayed as a modal dialog; otherwise it is
	// displayed in the parent.
    TBConnection(TBMain* main, QWidget* parent = NULL);

    ~TBConnection();

protected:
    // Detects when one of the QLineEdit objects are clicked and calls the
	// relevant clicked[Name]() slot.
    bool eventFilter(QObject *obj, QEvent *event);
    
private:
    // Pointer to TBMain parent.
    TBMain* parent;

private slots:
    // Slot for when the user clicks the "accept" button.  Checks for errors,
	// and then opens the table with the specified options in the browser.
    void accepted();

    // Slot for when the user clicks the "browse" button.  Opens a filebrowser
    // and sets the text of the line edit to the path of the selected file.
    // Updates the help text to TBConstants::OPEN_TEXT_LOCATION.
    void browse();

    // Slot for when the user clicks the local box.  Updates the help text
    // to TBConstants::OPEN_TEXT_LOCAL.
    void clickedLocal();

    // Slot for when the user clicks the remote box.  Updates the help text to
    // TBConstants::OPEN_TEXT_REMOTE.
    void clickedRemote();

    // Slot for when the user clicks the host line edit.  Updates the help text
    // to TBConstants::OPEN_TEXT_HOST.
    void clickedHost();

    // Slot for when the user clicks the port line edit.  Updates the help text
    // to TBConstants::OPEN_TEXT_PORT.
    void clickedPort();

    // Slot for when the user clicks the location line edit.  Updates the help
    // text to TBConstants::OPEN_TEXT_LOCATION.
    void clickedLocation();

    // Slot for when the user clicks the direct box.  Updates the help text to
    // TBConstants::OPEN_TEXT_DIRECT.
    void clickedDirect();

    // Slot for when the user clicks the XML box.  Updates the help text to
    // TBConstants::OPEN_TEXT_XML.
    void clickedXML();

    // Slot for when the user clicks the home parser box.  Updates the help
    // text to TBConstants::OPEN_TEXT_HOME.
    void clickedHome();

    // Slot for when the user clicks the DOM parser box.  Updates the help
    // text to TBConstants::OPEN_TEXT_DOM.
    void clickedDOM();

    // Slot for when the user clicks the SAX parser box.  Updates the help text
    // to TBConstants::OPEN_TEXT_SAX.
    void clickedSAX();

    // Slot for when the user clicks the start line edit.  Updates the help
    // text to TBConstants::OPEN_TEXT_START.
    void clickedStart();

    // Slot for when the user clicks the num line edit.  Updates the help text
    // to TBConstants::OPEN_TEXT_NUM.
    void clickedNum();
};

}

#endif /* TBCONNECTION_H_ */
