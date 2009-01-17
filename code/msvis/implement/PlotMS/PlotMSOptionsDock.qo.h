//# PlotMSOptionsDock.qo.h: Widget for options dock.
//# Copyright (C) 2008
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
//#
//# $Id:  $
#ifndef PLOTMSOPTIONSDOCK_QO_H_
#define PLOTMSOPTIONSDOCK_QO_H_

#include <msvis/PlotMS/PlotMSOptionsDock.ui.h>
#include <msvis/PlotMS/PlotMSOptions.h>

#include <QDockWidget>

namespace casa {

class PlotMSOptionsDock : public QWidget, Ui::OptionsDock {
    Q_OBJECT
    
public:
    PlotMSOptionsDock();
    
    ~PlotMSOptionsDock();
    
    // Returns a dock surrounding this widget.
    QDockWidget* getDockWidget();
    
public slots:
    void setOptions(const PlotMSOptions& opts);
    
    void setEditable(bool editable = true);
    
signals:
    void optionsSet(const PlotMSOptions& opts);
    
private:
    QDockWidget* itsParent_;
    vector<QWidget*> itsEditingWidgets_;
    PlotMSOptions itsOptions_;
    
    bool itsChangeFlag_;
    
    QCheckBox* itsApplySelection_;
    QMap<PlotMSSelection::Field, QWidget*> itsSelectionValues_;
    
    PlotMSOptions currentOptions() const;
    
    QString changedText(QString& text, bool changed);
    QString changedText(String& text, bool changed) {
        QString str(text.c_str());
        return changedText(str, changed); }
    QString changedText(const char* text, bool changed) {
        QString str(text);
        return changedText(str, changed); }
    
private slots:
    void optionsChanged();

    void rowColChanged();

    void msBrowse();
    
    void apply();
};

}

#endif /* PLOTMSOPTIONSDOCK_QO_H_ */
