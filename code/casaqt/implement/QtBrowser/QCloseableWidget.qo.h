//# QCloseableWidget.qo.h: Wrapper around a QWidget with a "Close" button.
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
#ifndef QCLOSEABLEWIDGET_H_
#define QCLOSEABLEWIDGET_H_

#include <casaqt/QtBrowser/QCloseableWidget.ui.h>

#include <QtGui>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>

namespace casa {

// <summary>
// Wrapper around a QWidget with a "Close" button
// </summary>
//
// <synopsis>
// A QCloseableWidget is a convenience class that adds a "close" functionality
// to any QWidget subclass.  It does this by providing methods (such as getting,
// setting, and clearing the QWidget) and signals indicating when the close
// button has been pressed.  <b>Important</b>: it is the responsibility of the
// caller/parent to actually close the object; the QCloseableWidget just
// provides the signal.
// </synopsis>

class QCloseableWidget : public QWidget, Ui::Widget {
    Q_OBJECT
    
public:
    // Default Constructor.  Creates a blank widget with a close button.
    QCloseableWidget();
    
    // Constructor that takes the initial widget to display and the text for
    // the close button.  If the text is empty, the default close button text
    // is displayed.
    QCloseableWidget(QWidget* widget, String closeText = "");

    ~QCloseableWidget();

    
    // Returns the current widget held by the wrapper.
    QWidget* getCurrentWidget();

    // Sets the current widget to the given widget.  If shouldDeleteOld is true,
    // the old widget held by the wrapper is deleted.
    void setWidget(QWidget* widget, bool shouldDeleteOld = false);
    
    // Sets the text on the "close" button to the given String.
    void setCloseButtonText(String name);

    
    // "Clears" the current widget by replacing it with a blank widget.
    // This method does delete the old widget.
    void clearWidget();

signals:
    // The closeRequested signal is emitted whenever the "Close" button is
    // pressed.  The widget paramater points to the currently shown widget,
	// or NULL if there is none.
    void closeRequested(QWidget* widget);
    
private:
    // Convenience pointer to the layout of this QCloseableWidget.
    QGridLayout* grid;

    // The current widget held by the wrapper.
    QWidget* currWidget;
    
    
    // Initial set-up of GUI components.
    void setup();

private slots:
    // Slot to handle the "Close" button's clicked() signal.  Emits the
    // closeRequested() signal.
    void closeRequest();
};

}

#endif /* QCLOSEABLEWIDGET_H_ */
