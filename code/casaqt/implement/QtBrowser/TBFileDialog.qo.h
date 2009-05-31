//# TBFileDialog.qo.h: Subclass of QFileDialog with additional functionality.
//# Copyright (C) 2009
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
#ifndef TBFILEDIALOG_QO_H_
#define TBFILEDIALOG_QO_H_

#include <casa/BasicSL/String.h>

#include <QFileDialog>
#include <QLabel>
#include <QTimer>

#include <casa/namespace.h>
using namespace std;

namespace casa {

// Subclass of QFileDialog with additional functionality for the table browser.
// IMPORTANT: This class needs to be checked when the Qt version changes!
// Specifically:
// 1) The label at the bottom assumes that the layout is a QGridLayout, where
//    the first column is for a label and the rest can be used for a widget.
// 2) As of Qt 4.3.4, the QFileDialog::filesSelected() signal is NOT emitted
//    whenever the selection changes, but only when the selection is finalized.
//    Because we're using a label to keep track of the current selection, this
//    class uses a timer to update the label at a set time to the current
//    selection.  This is slightly inefficient, but there's no better way to do
//    it without making our own file chooser dialog.
class TBFileDialog : public QFileDialog {
    Q_OBJECT
    
public:
    // Static //
    
    // Similar to QFileDialog::getExistingDirectory(), except that it uses the
    // special functionality of TBFileDialog.
    // <group>
    static QString getExistingTable(QWidget* parent = NULL,
            const QString& caption = QString(),
            const QString& directory = QString());
    static String getExistingTable2(QWidget* parent = NULL,
            const QString& caption = QString(),
            const QString& directory = QString()) {
        return getExistingTable(parent, caption, directory).toStdString(); }
    // </group>
    
    
    // Non-Static //
    
    // See QFileDialog constructors.
    // <group>
    TBFileDialog(QWidget* parent, Qt::WindowFlags flags);
    TBFileDialog(QWidget* parent = NULL, const QString& caption = QString(),
            const QString& directory = QString(),
            const QString& filter = QString());
    // </group>
    
    // Destructor.
    ~TBFileDialog();
    
private:
    // Label to display the currently chosen file(s).
    QLabel* chosenLabel;
    
    // Timer.
    QTimer* timer;
    
    
    // To be called from the constructors.
    void initialize();
    
private slots:
    // Signal for timer timeout.
    void timeout();
};

}

#endif /* TBFILEDIALOG_QO_H_ */
