//# PlotRangeWidget.cc: Widget for plot axes ranges.
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
#include <plotms/Gui/PlotRangeWidget.qo.h>

#include <plotms/PlotMS/PlotMSConstants.h>

#include <limits>

namespace casa {

/////////////////////////////////
// PLOTRANGEWIDGET DEFINITIONS //
/////////////////////////////////

PlotRangeWidget::PlotRangeWidget(bool customOnTwoLines, QWidget* parent) :
		QtEditingWidget(parent) {
    setupUi(this);
    
    if(customOnTwoLines) {
    	// double page
    	QVBoxLayout* vl = new QVBoxLayout();
    	vl->setContentsMargins(0, 0, 0, 0);
    	vl->setSpacing(3);
    	
    	QHBoxLayout* hl = new QHBoxLayout();
    	hl->setContentsMargins(0, 0, 0, 0);
    	hl->setSpacing(3);
    	hl->addWidget(doubleFrom);
    	hl->addWidget(doubleLabel);
    	vl->addLayout(hl);
    	vl->addWidget(doubleTo);
    	delete doublePage->layout();
    	doublePage->setLayout(vl);
    	
    	// time page
    	vl = new QVBoxLayout();
    	vl->setContentsMargins(0, 0, 0, 0);
    	vl->setSpacing(3);
    	
    	hl = new QHBoxLayout();
    	hl->setContentsMargins(0, 0, 0, 0);
    	hl->setSpacing(3);
    	hl->addWidget(timeFrom);
    	hl->addWidget(timeLabel);
    	vl->addLayout(hl);
    	vl->addWidget(timeTo);
    	delete timePage->layout();
    	timePage->setLayout(vl);
    }
    
    doubleFrom->setValidator(new QDoubleValidator(doubleFrom));
    doubleTo->setValidator(new QDoubleValidator(doubleTo));
    
    setRange(false, false, 0, 0);
    
    connect(automatic, SIGNAL(toggled(bool)), SLOT(rangeChanged()));
    connect(doubleFrom, SIGNAL(textChanged(const QString&)),
    		SLOT(rangeChanged()));
    connect(doubleTo, SIGNAL(textChanged(const QString&)),
    		SLOT(rangeChanged()));
    connect(timeFrom, SIGNAL(dateTimeChanged(const QDateTime&)),
            SLOT(rangeChanged()));
    connect(timeTo, SIGNAL(dateTimeChanged(const QDateTime&)),
            SLOT(rangeChanged()));
}

PlotRangeWidget::~PlotRangeWidget() { }

bool PlotRangeWidget::isDate() const {
    return stackedWidget->currentIndex() == 1; }
void PlotRangeWidget::setIsDate(bool isDate) {
    stackedWidget->setCurrentIndex(isDate ? 1 : 0); }

bool PlotRangeWidget::isCustom() const { return custom->isChecked(); }

prange_t PlotRangeWidget::getRange() const {
    if(isDate()) {
        QDate date = timeFrom->date(); QTime time = timeFrom->time();
        double from = PMS::dateDouble(date.year(), date.month(), date.day(),
                time.hour(), time.minute(),
                time.second() + (time.msec() / 1000.0));
        
        date = timeTo->date(); time = timeTo->time();
        double to = PMS::dateDouble(date.year(), date.month(), date.day(),
                time.hour(), time.minute(),
                time.second() + (time.msec() / 1000.0));
        return prange_t(from, to);
        
    } else
        return prange_t(doubleFrom->text().toDouble(),
        		        doubleTo->text().toDouble());
}

void PlotRangeWidget::setRange(bool isDate, bool isCustom, double from,
        double to) {
    blockSignals(true);
    
    stackedWidget->setCurrentIndex(isDate ? 1 : 0);
    custom->setChecked(isCustom);
    
    bool changed = isDate != this->isDate() || isCustom != this->isCustom();
    if(!changed) {
        prange_t range = getRange();
        changed = from != range.first || to != range.second;
    }
    
    if(isDate) {
        double temp;
        unsigned int year, mon, day, hour, min; double sec;
        PMS::dateDouble(from, year, mon, day, hour, min, sec);
        timeFrom->setDate(QDate(year, mon, day));
        timeFrom->setTime(QTime(hour, min, (int)sec,
                          (int)((modf(sec, &temp) * 1000) + 0.5)));
        
        PMS::dateDouble(to, year, mon, day, hour, min, sec);
        timeTo->setDate(QDate(year, mon, day));
        timeFrom->setTime(QTime(hour, min, (int)sec,
                          (int)((modf(sec, &temp) * 1000) + 0.5)));
        
    } else {
        doubleFrom->setText(QString::number(from));
        doubleTo->setText(QString::number(to));
    }
    
    isCustom_ = isCustom;
    from_ = from;
    to_ = to;
    
    blockSignals(false);
    if(changed) emit this->changed();
}

void PlotRangeWidget::addRadioButtonsToGroup(QButtonGroup* group) const {
    if(group == NULL) return;
    group->addButton(automatic);
    group->addButton(custom);
}

void PlotRangeWidget::rangeChanged() {
    emit changed();
    prange_t r = getRange();
    if(isCustom() != isCustom_ || r.first != from_ || r.second != to_)
        emit differentFromSet();
}

}
