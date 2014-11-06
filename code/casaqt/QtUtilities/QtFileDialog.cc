//# QtFileDialog.cc: Subclass of QFileDialog with additional functionality.
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
#include <casaqt/QtUtilities/QtFileDialog.qo.h>

#include <QLayout>
#include <QSortFilterProxyModel>
#include <QUrl>

namespace casa {

//////////////////////////////
// QTFILEDIALOG DEFINITIONS //
//////////////////////////////

// Public Static //

const QString& QtFileDialog::lastDirectory() { return lastDirectory_; }
void QtFileDialog::setNextDirectory(const QString& directory) {
    lastDirectory_ = directory; }

int QtFileDialog::historyLimit() { return historyLimit_; }
void QtFileDialog::setHistoryLimit(int limit) { historyLimit_ = limit; }


// Private Static //

QString QtFileDialog::lastDirectory_ = "";
int QtFileDialog::historyLimit_ = -1;


QString QtFileDialog::qgetHelper(AcceptMode acceptMode, FileMode fileMode,
        QWidget* parent, const QString& caption, const QString& directory,
        const QString& filter, int histLimit) {
    QtFileDialog chooser(parent, caption, directory, filter);
    chooser.setModal(true);
    chooser.setAcceptMode(acceptMode);
    chooser.setFileMode(fileMode);

    // "/Volumes" on Mac is hidden, add manually to sidebar
    QDir volumesDir(QDir("/Volumes"));
    if ( volumesDir.exists() ) {
	QList<QUrl> urls;
        urls = chooser.sidebarUrls();
	urls << QUrl::fromLocalFile(volumesDir.absolutePath() );
	chooser.setSidebarUrls(urls);
    }
    
#if (0)
	// The next three lines fixes issue CSV-433, except
	// for the resulting plotms crashes if the user clicks 
	// on the left-side list of commonly used locations in 
	// the file open dialog.
	// This is due to a bug in Qt4.3, fixed in Qt4.5 or 4.6.  
	// We will keep this code here, since it is the proper solution
	// to the issue, but disable it with #if until Qt can be 
	// officially updated for CASA.
	QSortFilterProxyModel *sorter = new QSortFilterProxyModel();
	sorter->setDynamicSortFilter(true); // This ensures the proxy will resort when the model changes
	chooser.setProxyModel(sorter);
#endif
	if(histLimit >= 0) {
        QStringList hist = chooser.history();
        if(histLimit < hist.size()) {
            while(histLimit < hist.size()) hist.removeFirst();
            chooser.setHistory(hist);
        }
    }
    
    if(chooser.exec()) {    
        QStringList files = chooser.selectedFiles();
        if(files.size() > 0) return files[0];
    }
    
    return QString();
}


// Non-Static //

QtFileDialog::QtFileDialog(QWidget* parent, Qt::WindowFlags flags) :
        QFileDialog(parent, flags) {
    initialize();
}

QtFileDialog::QtFileDialog(QWidget* parent, const QString& caption,
        const QString& directory, const QString& filter) : QFileDialog(parent,
        caption, directory, filter) {
    initialize();
}

QtFileDialog::~QtFileDialog() { }

void QtFileDialog::initialize() {
    // NOTICE: This is only valid as long as QFileDialog continues to use a
    // QGridLayout.  I don't really like adding the label this way, but think
    // it's the best of the current options unless I've missed something..    
    QGridLayout* gl = dynamic_cast<QGridLayout*>(layout());
    if(gl == NULL) return;
    
    chosenLabel = new QLabel();
    chosenLabel->setWordWrap(true);
    gl->addWidget(new QLabel("Selected:"), gl->rowCount(), 0);
    gl->addWidget(chosenLabel, gl->rowCount() -1, 1, 1, gl->columnCount() -1);
    
    timeout();
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), SLOT(timeout()));
    timer->start(250);
}

void QtFileDialog::accept() {
    QFileDialog::accept();
    
    // Update last directory.
    QStringList files = selectedFiles();
    if(files.size() > 0)
        lastDirectory_ = QFileInfo(files[0]).dir().absolutePath();
}


void QtFileDialog::timeout() {
    chosenLabel->setText(selectedFiles().join(", ").replace("/", "/ "));
}

}
