//# QProgressPanel.cc: Convenience class with a label and progress meter.
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
#include <casaqt/QtBrowser/QProgressPanel.qo.h>

namespace casa {

////////////////////////////////
// QPROGRESSPANEL DEFINITIONS //
////////////////////////////////

// Constructors/Destructors //

QProgressPanel::QProgressPanel(String l, bool hideable, bool cancelable):
																QWidget() {
    setupUi(this);
    setBackgroundRole(QPalette::NoRole); // clear background
    connect(this, SIGNAL(updateValue(int)), progressBar, SLOT(setValue(int)));
    label->setText(l.c_str());

    if(cancelable)
    	connect(cancelButton, SIGNAL(clicked()),this, SLOT(cancel()));
    else
    	cancelButton->hide();

    if(hideable) connect(hideButton, SIGNAL(clicked()), this, SLOT(hide()));
    else hideButton->hide();
}

QProgressPanel::~QProgressPanel() { }

// Accessors/Mutators //

QProgressBar* QProgressPanel::getProgressBar() {
    return progressBar;
}

QLabel* QProgressPanel::getLabel() {
    return label;
}

void QProgressPanel::setLabel(String newLabel) {
    label->setText(newLabel.c_str());
}

// Public Slots //

void QProgressPanel::setValue(int value) {
    emit updateValue(value);
}

// Private Slots //

void QProgressPanel::cancel() {
    emit cancelRequested();
}

void QProgressPanel::hide() {
    emit hideRequested();
}

////////////////////////////////
// PROGRESSHELPER DEFINITIONS //
////////////////////////////////

// Constructors/Destructors //

ProgressHelper::ProgressHelper(QProgressPanel* qpp): valid(false), label(NULL),
                                                     bar(NULL), s(0) {
    if(qpp != NULL) {
        label = qpp->getLabel();
        bar = qpp->getProgressBar();
        bar->setMinimum(0);
        bar->setMaximum(100);
        bar->setValue(0);
        valid = true;
    }
}

ProgressHelper::ProgressHelper(QProgressPanel& qpp): valid(false), label(NULL),
                                                     bar(NULL), s(0) {
    label = qpp.getLabel();
    bar = qpp.getProgressBar();
    bar->setMinimum(0);
    bar->setMaximum(100);
    bar->setValue(0);
    valid = true;
}

ProgressHelper::ProgressHelper(QLabel* l, QProgressBar* pb): valid(false),
                                              label(NULL), bar(NULL), s(0) {
    if(l != NULL && pb != NULL) {
        label = l;
        bar = pb;
        bar->setMinimum(0);
        bar->setMaximum(100);
        bar->setValue(0);
        valid = true;
    }
}

ProgressHelper::ProgressHelper(QLabel& l, QProgressBar& pb): valid(false),
                                              label(NULL), bar(NULL), s(0) {
    label = &l;
    bar = &pb;
    bar->setMinimum(0);
    bar->setMaximum(100);
    bar->setValue(0);
    valid = true;
}

ProgressHelper::ProgressHelper(ProgressHelper* ph): valid(false), label(NULL),
                                                     bar(NULL), s(0) {
    if(ph != NULL) {
        label = ph->getLabel();
        bar = ph->getBar();
        bar->setMinimum(0);
        bar->setMaximum(100);
        bar->setValue(0);
        valid = true;
    }
}

ProgressHelper::ProgressHelper(ProgressHelper& ph): valid(false), label(NULL),
                                                     bar(NULL), s(0) {
    label = ph.getLabel();
    bar = ph.getBar();
    bar->setMinimum(0);
    bar->setMaximum(100);
    bar->setValue(0);
    valid = true;
}

ProgressHelper::~ProgressHelper() {

}

// Accessor/Mutator Methods //

QLabel* ProgressHelper::getLabel() {
    if(valid) return label;
    else return NULL;
}

QProgressBar* ProgressHelper::getBar() {
    if(valid) return bar;
    else return NULL;
}

void ProgressHelper::setLabel(String l) {
    if(!valid) return;
    label->setText(l.c_str());
    QCoreApplication::processEvents();
}

// Public Methods //

void ProgressHelper::setSteps(int steps) {
    if(!valid) return;
    if(steps > 0) bar->setMaximum(steps);
    else bar->setMaximum(100);
}

void ProgressHelper::step() {
    if(!valid) return;

    s++;
    bar->setValue(s);
}

void ProgressHelper::done() {
    if(!valid) return;
    QString str = label->text();
    str = "[Finished] " + str;
    label->setText(str);
    bar->setValue(bar->maximum());
    QCoreApplication::processEvents();
}

void ProgressHelper::reset(String nl) {
    s = 0;
    label->setText(nl.c_str());
    QCoreApplication::processEvents();
}

}
