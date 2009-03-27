//# QtProgressWidget.cc: Qt widget for displaying progress information.
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
#include <casaqt/QtUtilities/QtProgressWidget.qo.h>

#include <QCloseEvent>

namespace casa {

//////////////////////////////////
// QTPROGRESSWIDGET DEFINITIONS //
//////////////////////////////////

QtProgressWidget::QtProgressWidget(bool allowBackground, bool allowPauseResume,
        bool allowCancel, bool isDialog, QWidget* parent) :
        QWidget(parent, isDialog ? Qt::Dialog : Qt::Widget) {
    setupUi(this);
    
    //setWindowModality(Qt::WindowModal);
    
    connect(backgroundButton, SIGNAL(clicked()), SLOT(background()));
    connect(pauseButton, SIGNAL(clicked(bool)), SLOT(pauseResume(bool)));
    connect(cancelButton, SIGNAL(clicked()), SLOT(cancel()));
    
    setAllowedOperations(allowBackground, allowPauseResume, allowCancel);
}

QtProgressWidget::~QtProgressWidget() { }


void QtProgressWidget::setAllowedOperations(bool background, bool pauseResume,
        bool cancel) {
    itsMutex_.lock();
    backgroundButton->setEnabled(background);
    pauseButton->setEnabled(pauseResume);
    cancelButton->setEnabled(cancel);
    itsMutex_.unlock();
}

QMutex& QtProgressWidget::mutex() { return itsMutex_; }

void QtProgressWidget::initialize(const QString& operationName) {
    itsMutex_.lock();
    operationLabel->setText(operationName);
    progressBar->setValue(0);
    statusLabel->setText("Starting " + operationName + ".");
    itsMutex_.unlock();
}

void QtProgressWidget::setStatus(const QString& status) {
    itsMutex_.lock();
    statusLabel->setText(status);
    itsMutex_.unlock();
}

void QtProgressWidget::setProgress(unsigned int progress) {
    itsMutex_.lock();
    if(progress > 100) progress = 100;
    progressBar->setValue(progress);
    itsMutex_.unlock();
}

void QtProgressWidget::setProgress(unsigned int progress,
        const QString& status) {
    itsMutex_.lock();
    if(progress > 100) progress = 100;
    progressBar->setValue(progress);
    statusLabel->setText(status);
    itsMutex_.unlock();
}

void QtProgressWidget::finalize() {
    itsMutex_.lock();
    progressBar->setValue(100);
    statusLabel->setText("Finished " + operationLabel->text() + ".");
    itsMutex_.unlock();
}

void QtProgressWidget::closeEvent(QCloseEvent* event) {
    background();
    event->ignore();
}

void QtProgressWidget::background() { emit backgroundRequested(); }

void QtProgressWidget::pauseResume(bool pause) {
    itsMutex_.lock();
    pauseButton->setText(pause ? "Resume" : "Pause");
    itsMutex_.unlock();
    if(pause) emit pauseRequested();
    else      emit resumeRequested();
}

void QtProgressWidget::cancel() { emit cancelRequested(); }

}
