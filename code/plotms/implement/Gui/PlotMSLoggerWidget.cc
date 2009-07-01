//# PlotMSLoggerWidget.cc: Widget to set log events and priority filter.
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
#include <plotms/Gui/PlotMSLoggerWidget.qo.h>

#include <graphics/GenericPlotter/PlotLogger.h>

#include <QLabel>

namespace casa {

////////////////////////////////////
// PLOTMSLOGGERWIDGET DEFINITIONS //
////////////////////////////////////

PlotMSLoggerWidget::PlotMSLoggerWidget(const String& filename, int events,
        LogMessage::Priority priority, int maxWidth, bool editFilename,
        bool editPriority, QWidget* parent): QtEditingWidget(parent),
        itsFilename_(filename), itsFilenameWidget_(NULL), itsEvents_(events),
        itsEventsBox_(new QtComboCheckBox()), itsPriority_(priority),
        itsPriorityBox_(NULL) {
    QVBoxLayout* vl = new QVBoxLayout();
    vl->setContentsMargins(0, 0, 0, 0);
    
    QHBoxLayout* l;
    if(editFilename) {
        l = new QHBoxLayout();
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(3);
        l->addWidget(new QLabel("Log Location:"));
        vl->addWidget(itsFilenameWidget_ = new QtFileWidget(false, true));
        connect(itsFilenameWidget_, SIGNAL(changed()), SLOT(widgetChanged()));
    }
    
    itsEventsBox_->setDisplayText("Log Events");
    vector<int> e = PlotLogger::ALL_EVENTS();
    l = new QHBoxLayout();
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(3);
    for(unsigned int i = 0; i < e.size(); i++) {
        itsEventsBox_->addItem(PlotLogger::EVENT(e[i]).c_str(),
                               events & e[i]);
        itsEventsIndexValues_.insert(i, e[i]);
    }
    l->addWidget(itsEventsBox_);
    l->addStretch();
    vl->addLayout(l);
    connect(itsEventsBox_, SIGNAL(stateChanged()), SLOT(widgetChanged()));
    
    if(editPriority) {
        itsPriorityBox_ = new QComboBox();
        for(int i = LogMessage::DEBUGGING; i <= LogMessage::SEVERE; i++)
            itsPriorityBox_->addItem(LogMessage::toString(
                    LogMessage::Priority(i)).c_str());
        
        l = new QHBoxLayout();
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(3);
        l->addWidget(new QLabel("Log Priority Filter:"));
        l->addWidget(itsPriorityBox_);
        l->addStretch();
        vl->addLayout(l);
        connect(itsPriorityBox_, SIGNAL(currentIndexChanged(int)),
                SLOT(widgetChanged()));
    }
    
    setLayout(vl);
    
    setFilename(itsFilename_);
    setEvents(itsEvents_);
    setPriority(itsPriority_);
}

PlotMSLoggerWidget::~PlotMSLoggerWidget() { }

String PlotMSLoggerWidget::filename() const {
    if(itsFilenameWidget_ != NULL) return itsFilenameWidget_->getFile();
    else return itsFilename_;
}
void PlotMSLoggerWidget::setFilename(const String& filename) {
    if(itsFilenameWidget_ != NULL) itsFilenameWidget_->setFile(filename);
    itsFilename_ = filename;
}

int PlotMSLoggerWidget::events() const {
    int flag = 0;
    for(int i = 0; i < itsEventsBox_->count(); i++)
        if(itsEventsBox_->indexIsToggled(i)) flag |= itsEventsIndexValues_[i];
    return flag;
}

void PlotMSLoggerWidget::setEvents(int events) {
    for(int i = 0; i < itsEventsBox_->count(); i++)
        itsEventsBox_->toggleIndex(i, events & itsEventsIndexValues_[i]);
    itsEvents_ = events;
}

LogMessage::Priority PlotMSLoggerWidget::priority() const {
    if(itsPriorityBox_ != NULL)
        return LogMessage::Priority(itsPriorityBox_->currentIndex() -
                                    LogMessage::DEBUGGING);
    else return itsPriority_;
}

void PlotMSLoggerWidget::setPriority(LogMessage::Priority priority) {
    if(itsPriorityBox_ != NULL)
        itsPriorityBox_->setCurrentIndex(priority - LogMessage::DEBUGGING);
    itsPriority_ = priority;
}


void PlotMSLoggerWidget::widgetChanged() {
    emit changed();
    if(filename() != itsFilename_ || events() != itsEvents_ ||
       priority() != itsPriority_)
        emit differentFromSet();
}

}
