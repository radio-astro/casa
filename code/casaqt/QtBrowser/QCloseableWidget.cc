//# QCloseableWidget.cc: Wrapper around a QWidget with a "Close" button.
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
#include <casaqt/QtBrowser/QCloseableWidget.qo.h>

namespace casa {

//////////////////////////////////
// QCLOSEABLEWIDGET DEFINITIONS //
//////////////////////////////////

// Constructors/Destructors //

QCloseableWidget::QCloseableWidget() {
    setup();
}

QCloseableWidget::QCloseableWidget(QWidget* widget, String closeText) {
	setup();
	if(widget != NULL) setWidget(widget, false);
	if(!closeText.empty()) setCloseButtonText(closeText);
}

QCloseableWidget::~QCloseableWidget() {
    if(currWidget != NULL && currWidget->parentWidget() == NULL)
        delete currWidget;
}

// Accessors/Mutators //

QWidget* QCloseableWidget::getCurrentWidget() {
    return currWidget;
}

void QCloseableWidget::setWidget(QWidget* widget, bool shouldDeleteOld) {
	if(widget == NULL) return;
	
    grid->removeWidget(currWidget);
    if(shouldDeleteOld && currWidget != blankWidget) delete currWidget;
    
    grid->addWidget(widget, 0, 0, 1, 2);
    currWidget = widget;
}

void QCloseableWidget::setCloseButtonText(String text) {
	closeButton->setText(text.c_str());
}

// Public Methods //

void QCloseableWidget::clearWidget() {
    setWidget(blankWidget, true);
}

// Private Methods //

void QCloseableWidget::setup() {
	setupUi(this);
	    grid = (QGridLayout*) layout();
	    currWidget = blankWidget;
	    connect(closeButton, SIGNAL(clicked()), this, SLOT(closeRequest()));
}

// Private Slots //

void QCloseableWidget::closeRequest() {
    emit closeRequested((currWidget == blankWidget) ? NULL : currWidget);
}

}
