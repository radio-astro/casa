//# TBOptions.qo.h: Widget to edit the browsing options.
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
#ifndef TBOPTIONS_H_
#define TBOPTIONS_H_

#include <casaqt/QtBrowser/TBOptions.ui.h>

#include <QtGui>

#include <casa/BasicSL/String.h>
#include <casa/namespace.h>

namespace casa {

// <summary>
// Widget to edit the browsing options.
// </summary>
//
// <synopsis>
// TBOptions is used to display and edit any current browsing options.
// Currently, the only available options are: save current view on save, and
// setting the debug threshold.  Important: the parent/caller is responsible
// for connecting the saveOptions() signal and actually setting the options as
// requested.
// </synopsis>

class TBOptions : public QDialog, Ui::Options {
    Q_OBJECT

public:
    // Default Constructor.
    TBOptions();

    ~TBOptions();

    
    // Returns true if the "save view on close" check box is checked, false
    // otherwise.
    bool saveView();

    // Sets the state on the "save view on close" check box.
    void setSaveView(bool saveView);

    // Returns the debug threshold as indicated by the QComboBox.
    int debugLevel();

    // Sets the debug threshold via the QComboBox.
    void setDebugLevel(int newLevel);
    
    // Gets/Sets the file chooser history limit.
    // <group>
    int chooserHistoryLimit() const;
    void setChooserHistoryLimit(int limit);
    // </group>
    
signals:
    // saveOptions is emitted when the user has configured the options and
    // clicked the "Save" button.  A pointer to this TBOptions object is
    // included as the parent/caller should delete it.
    void saveOptions(TBOptions* opt);

private slots:
    // Restores the default options to the dialog.
    void restoreDefaults();

    // Slot for the "Save" button.  Emits the saveOptions() signal.
    void save();

    // Slot for the "Cancel" button.  Closes and deletes the widget.
    void cancel();
};

}

#endif /* TBOPTIONS_H_ */
