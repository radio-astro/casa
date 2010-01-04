//# QProgressPanel.qo.h: Convenience class with a label and progress meter.
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
#ifndef QPROGRESSPANEL_H_
#define QPROGRESSPANEL_H_

#include <casaqt/QtBrowser/QProgressPanel.ui.h>

#include <QtGui>

#include <casa/BasicSL/String.h>

#include <casa/namespace.h>

namespace casa {

// <summary>
// Convenience class with a label and progress meter.
// </summary>
//
// <synopsis>
// A QProgressPanel is a convenience class that provides a common
// functionality: displaying a progress meter along with a label.
// QProgressPanel also has two optional buttons: a "Hide" button and a "Close"
// button - pressing either of these buttons sends a signal. <b>Important</b>:
// it is the responsibility of the caller/parent to handle these signals.
// </synopsis>

class QProgressPanel : public QWidget, Ui::ProgressPanel {
    Q_OBJECT

public:
    // Builds a QProgressPanel with the given label and a progress meter at 0%
	// completion.  If hideable is true, the optional "Hide" button is shown;
	// similarly, if cancelable is true, the optional "Cancel" button is shown.
    QProgressPanel(String label, bool hideable = true, bool cancelable = true);

    ~QProgressPanel();

    
    // Returns the progress meter (QProgressBar).
    QProgressBar* getProgressBar();

    // Returns the label (QLabel).
    QLabel* getLabel();

    // Sets the text of the label to the given String.
    void setLabel(String newLabel);

public slots:
    // Sets the value of the progress meter to the given value.  Note that the
	// progress meter is on a scale of 0 - 100.  Also, due to some weirdness
	// with Qt's GUI thread system, calling setValue actually emits a signal
	// which is caught by the progress meter rather than setting the value
	// directly.
    void setValue(int value);

signals:
    // The cancelRequested signal is emitted whenever the "Cancel" button is
	// clicked.  If the QProgressPanel was created without the cancel button,
	// this signal is never emitted.
    void cancelRequested();

    // The hideRequested signal is emitted whenever the "Hide" button is
    // clicked.  If the QProgressPanel was created without the hide button,
    // this signal is never emitted.
    void hideRequested();

    // This is the signal used by setValue() to get around Qt's GUI thread
    // issues.
    void updateValue(int value);

private slots:
    // Slot to handle the "Cancel" button's clicked() signal.  Emits the
	// cancelRequested() signal.
    void cancel();

    // Slot to handle the "Hide" button's clicked() signal.  Emits the
    // hideRequested() signal.
    void hide();
};

// <summary>
// Wrapper around a QProgressPanel or other QLabel/QProgressBar pairing.
// </summary>
//
// <synopsis>
// A ProgressHelper is a convenience class that provides methods dealing with
// any QLabel/QProgressBar pair.  Progress is seen as a number of steps, and as
// progress is made the "steps" counter increases.
// </synopsis>

class ProgressHelper {
public:
    // Constructor that takes a pointer to a QProgressPanel.  If qpp is NULL,
	// the ProgressHelper is invalid.
    ProgressHelper(QProgressPanel* qpp);

    // Constructor that takes a reference to a QProgressPanel.
    ProgressHelper(QProgressPanel& qpp);

    // Constructor that takes any QLabel and QProgressBar pointers.  If either
    // are NULL, the ProgressHelper is invalid.
    ProgressHelper(QLabel* label, QProgressBar* pb);

    // Constructor that takes any QLabel and QProgressBar references.
    ProgressHelper(QLabel& label, QProgressBar& pb);

    // Copy Constructor.  If ph is NULL, the ProgressHelper is invalid.
    ProgressHelper(ProgressHelper* ph);

    // Copy Constructor.
    ProgressHelper(ProgressHelper& ph);

    ~ProgressHelper();

    
    // Returns the label (QLabel).
    QLabel* getLabel();

    // Returns the progress meter (QProgressBar).
    QProgressBar* getBar();
    
    // Set the text of the QLabel to the given String.
    void setLabel(String label);
    
    // Set the total number of steps in the task.
    void setSteps(int steps);

    
    // Indicates that one step of progress has been made.  The progress meter
    // is updated accordingly.
    void step();

    // Indicate that the task has been completed; the label and progress meter
    // are updated accordingly.
    void done();

    // Rest the progress meter and set the label with the given text.
    void reset(String newLabel);

private:
    // Indicates whether this object is valid or not.  (See constructors.)
    bool valid;
    
    // Pointer to the label.
    QLabel* label;

    // Pointer to the progress meter.
    QProgressBar* bar;

    // Steps counter.
    int s;
};

}

#endif /* QPROGRESSPANEL_H_ */
