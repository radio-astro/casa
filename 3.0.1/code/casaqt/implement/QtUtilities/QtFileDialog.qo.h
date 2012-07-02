//# QtFileDialog.qo.h: Subclass of QFileDialog with additional functionality.
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
#ifndef QTFILEDIALOG_QO_H_
#define QTFILEDIALOG_QO_H_

#include <casa/BasicSL/String.h>

#include <QFileDialog>
#include <QLabel>
#include <QTimer>

#include <casa/namespace.h>
using namespace std;

namespace casa {

// Subclass of QFileDialog with additional functionality.
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
class QtFileDialog : public QFileDialog {
    Q_OBJECT
    
public:
    // Static //
    
    // Returns an existing directory using the given optional parent, caption,
    // and starting directory parameters.  See QFileDialog.
    // <group>
    static QString qgetExistingDir(QWidget* parent = NULL,
            const QString& caption = QString(),
            const QString& directory = lastDirectory(),
            int histLimit = historyLimit()) {
        return qgetHelper(AcceptOpen, DirectoryOnly, parent, caption,
                          directory, QString(), histLimit); }
    static String getExistingDir(QWidget* parent = NULL,
            const QString& caption = QString(),
            const QString& directory = lastDirectory(),
            int histLimit = historyLimit()) {
        return qgetExistingDir(parent, caption, directory,
                               histLimit).toStdString(); }
    // </group>
    
    // Returns an existing file using the given optional parent, caption,
    // starting directory, and filter parameters.  See QFileDialog.
    // <group>
    static QString qgetExistingFile(QWidget* parent = NULL,
            const QString& caption = QString(),
            const QString& directory = lastDirectory(),
            const QString& filter = QString(), int histLimit = historyLimit()){
        return qgetHelper(AcceptOpen, ExistingFile, parent, caption, directory,
                          QString(), histLimit); }
    static String getExistingFile(QWidget* parent = NULL,
            const QString& caption = QString(),
            const QString& directory = lastDirectory(),
            const QString& filter = QString(), int histLimit = historyLimit()){
        return qgetExistingFile(parent, caption, directory, filter,
                                histLimit).toStdString(); }
    // </group>
    
    // Returns a new filename using the given optional parent, caption,
    // starting directory, and filter parameters.  See QFileDialog.
    // <group>
    static QString qgetAnyFile(QWidget* parent = NULL,
            const QString& caption = QString(),
            const QString& directory = lastDirectory(),
            const QString& filter = QString(), int histLimit = historyLimit()){
        return qgetHelper(AcceptSave, AnyFile, parent, caption, directory,
                          QString(), histLimit); }
    static String getAnyFile(QWidget* parent = NULL,
            const QString& caption = QString(),
            const QString& directory = lastDirectory(),
            const QString& filter = QString(), int histLimit = historyLimit()){
        return qgetAnyFile(parent, caption, directory, filter,
                           histLimit).toStdString(); }
    // </group>
    
    // Gets/Sets the last directory to be used by a QtFileDialog.  Blank means
    // the current directory, which is the default.
    // <group>
    static const QString& lastDirectory();
    static void setNextDirectory(const QString& directory);
    // </group>
    
    // Gets/Sets the limit for the history size used by a QtFileDialog.  -1
    // means it is not managed, which is the default.
    // <group>
    static int historyLimit();
    static void setHistoryLimit(int limit);
    // </group>
    
    
    // Non-Static //
    
    // See QFileDialog constructors.
    // <group>
    QtFileDialog(QWidget* parent, Qt::WindowFlags flags);
    QtFileDialog(QWidget* parent = NULL, const QString& caption = QString(),
            const QString& directory = lastDirectory(),
            const QString& filter = QString());
    // </group>
    
    // Destructor.
    ~QtFileDialog();
    
public slots:
    // Overrides QFileDialog::accept().
    void accept();
    
private:
    // Label to display the currently chosen file(s).
    QLabel* chosenLabel;
    
    // Timer.
    QTimer* timer;
    
    
    // To be called from the constructors.
    void initialize();
    
    
    // Static //
    
    // Last directory.
    static QString lastDirectory_;
    
    // Set history limit.
    static int historyLimit_;
    
    
    // Helper method for the static qget* functions.
    static QString qgetHelper(AcceptMode acceptMode, FileMode fileMode,
            QWidget* parent, const QString& caption, const QString& directory,
            const QString& filter, int histLimit);
    
private slots:
    // Signal for timer timeout.
    void timeout();
};

}

#endif /* QTFILEDIALOG_QO_H_ */
