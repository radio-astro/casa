//# PlotMSTabs.cc: Tab GUI widgets.
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
#include <plotms/GuiTabs/PlotMSTab.qo.h>

#include <plotms/PlotMS/PlotMS.h>

namespace casa {

///////////////////////////
// PLOTMSTAB DEFINITIONS //
///////////////////////////

PlotMSTab::PlotMSTab(PlotMSPlotter* parent) : itsParent_(parent->getParent()),
        itsPlotter_(parent) { }

PlotMSTab::~PlotMSTab() { }


void PlotMSTab::changedText(QLabel* label, bool changed) {
    if(itsLabelDefaults_.contains(label))
        label->setText(changedText(itsLabelDefaults_.value(label),
                       changed));
}

QString PlotMSTab::changedText(const QString& t, bool changed) {
    QString str(t);
    //str.replace(' ', "&nbsp;");
    if(changed) str = "<font color=\"#FF0000\">"+ t + "</font>";
    return str;
}

bool PlotMSTab::setChooser(QComboBox* chooser, const QString& value) {
    for(int i = 0; i < chooser->count(); i++) {
        if(chooser->itemText(i) == value) {
            chooser->setCurrentIndex(i);
            return true;
        }
    }
    return false;
}

}
