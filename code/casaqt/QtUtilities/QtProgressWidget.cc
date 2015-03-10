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
#include <QMenu>

#ifdef QT_MAC_USE_COCOA
#include <objc/runtime.h>
#include <objc/message.h>
#endif

namespace casa {

//////////////////////////////////
// QTPROGRESSWIDGET DEFINITIONS //
//////////////////////////////////

QtProgressWidget::QtProgressWidget(bool allowBackground, bool allowPauseResume,
        bool allowCancel, bool isDialog, bool useCompact, QWidget* parent) :
        QWidget(parent, isDialog ? Qt::Dialog : Qt::Widget),
        itsProgress_(NULL), itsCompactProgress_(NULL) {
    if(useCompact) {
        itsCompactProgress_ = new Ui::ProgressCompactWidget();
        itsCompactProgress_->setupUi(this);
        
        itsCompactProgress_->actionPauseResume->setCheckable(true);
        connect(itsCompactProgress_->actionBackground, SIGNAL(triggered()),
                SLOT(background()));
        connect(itsCompactProgress_->actionPauseResume,
                SIGNAL(triggered(bool)), SLOT(pauseResume(bool)));
        connect(itsCompactProgress_->actionCancel, SIGNAL(triggered()),
                SLOT(cancel()));
        
    } else {
        itsProgress_ = new Ui::ProgressWidget();
        itsProgress_->setupUi(this);
        
        connect(itsProgress_->backgroundButton, SIGNAL(clicked()),
                SLOT(background()));
        connect(itsProgress_->pauseButton, SIGNAL(clicked(bool)),
                SLOT(pauseResume(bool)));
        connect(itsProgress_->cancelButton, SIGNAL(clicked()), SLOT(cancel()));
    }
    
    setAllowedOperations(allowBackground, allowPauseResume, allowCancel);
}

QtProgressWidget::~QtProgressWidget() { }


void QtProgressWidget::setAllowedOperations(bool background, bool pauseResume,
        bool cancel) {
    itsMutex_.lock();
    if(itsProgress_ != NULL) {
        itsProgress_->backgroundButton->setEnabled(background);
        itsProgress_->pauseButton->setEnabled(pauseResume);

	// CAS-7042 objc Mac error - autorelease with no pool in place
	// so creating thread pool and draining it.
#ifdef QT_MAC_USE_COCOA
#if 0
        id pool = objc_msgSend(objc_getClass("NSAutoreleasePool"), sel_getUid("new"));
#else
//      id pool = class_createInstance (objc_getClass("NSAutoreleasePool"),0);        // creates instance of the pool
//	objc_msgSend(pool, "init");                                                 // initializes it
#endif
#endif
        itsProgress_->cancelButton->setEnabled(cancel);
#ifdef QT_MAC_USE_COCOA
//      objc_msgSend(pool, sel_getUid("drain"));
#endif
    }
    else {
        itsCompactProgress_->menuButton->setVisible(background ||
                pauseResume || cancel);
        
        QMenu* menu = itsCompactProgress_->menuButton->menu();
        if(menu == NULL) {
            menu = new QMenu();
            itsCompactProgress_->menuButton->setMenu(menu);
        } else menu->clear();
        
        if(background) menu->addAction(itsCompactProgress_->actionBackground);
        if(pauseResume)menu->addAction(itsCompactProgress_->actionPauseResume);
        if(cancel) menu->addAction(itsCompactProgress_->actionCancel);
    }
    itsMutex_.unlock();
}

QMutex& QtProgressWidget::mutex() { return itsMutex_; }

void QtProgressWidget::initialize( const String& operationName ){
	QString opName( operationName.c_str() );
	initialize( opName );
}

void QtProgressWidget::initialize(const QString& operationName) {
    itsMutex_.lock();
    QLabel* operationLabel = (itsProgress_ != NULL) ?
            itsProgress_->operationLabel : itsCompactProgress_->operationLabel;
    QProgressBar* progressBar = (itsProgress_ != NULL) ?
            itsProgress_->progressBar : itsCompactProgress_->progressBar;
    QLabel* statusLabel = (itsProgress_ != NULL) ?
            itsProgress_->statusLabel : itsCompactProgress_->statusLabel;
    
    operationLabel->setText(operationName);
    progressBar->setValue(0);
    statusLabel->setText("Starting " + operationName + ".");
    itsMutex_.unlock();
}

void QtProgressWidget::setProgress(unsigned int progress, const String& status) {
	QString statusStr( status.c_str() );
	setProgress( progress, statusStr );
}

void QtProgressWidget::setStatus(const QString& status) {
    itsMutex_.lock();
    QLabel* statusLabel = (itsProgress_ != NULL) ?
            itsProgress_->statusLabel : itsCompactProgress_->statusLabel;
    
    statusLabel->setText(status);
    itsMutex_.unlock();
}

void QtProgressWidget::setProgress(unsigned int progress) {
    itsMutex_.lock();
    QProgressBar* progressBar = (itsProgress_ != NULL) ?
            itsProgress_->progressBar : itsCompactProgress_->progressBar;
    
    if(progress > 100) progress = 100;
    progressBar->setValue(progress);
    itsMutex_.unlock();
}

void QtProgressWidget::setProgress(unsigned int progress,
        const QString& status) {
    itsMutex_.lock();
    QProgressBar* progressBar = (itsProgress_ != NULL) ?
            itsProgress_->progressBar : itsCompactProgress_->progressBar;
    QLabel* statusLabel = (itsProgress_ != NULL) ?
            itsProgress_->statusLabel : itsCompactProgress_->statusLabel;
    
    if(progress > 100) progress = 100;
    progressBar->setValue(progress);
    statusLabel->setText(status);
    itsMutex_.unlock();
}

void QtProgressWidget::finalize() {
    itsMutex_.lock();
    QLabel* operationLabel = (itsProgress_ != NULL) ?
            itsProgress_->operationLabel : itsCompactProgress_->operationLabel;
    QProgressBar* progressBar = (itsProgress_ != NULL) ?
            itsProgress_->progressBar : itsCompactProgress_->progressBar;
    QLabel* statusLabel = (itsProgress_ != NULL) ?
            itsProgress_->statusLabel : itsCompactProgress_->statusLabel;
    
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
    QString text = pause ? "Resume" : "Pause";
    if(itsProgress_ != NULL)
        itsProgress_->pauseButton->setText(text);
    else
        itsCompactProgress_->actionPauseResume->setText(text);
    itsMutex_.unlock();
    if(pause) emit pauseRequested();
    else      emit resumeRequested();
}

void QtProgressWidget::cancel() { emit cancelRequested(); }

}
