//# PlotMSAnnotatorTab.qo.h: Subclass of PlotMSTab for the annotator.
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
#ifndef PLOTMSANNOTATORTAB_QO_H_
#define PLOTMSANNOTATORTAB_QO_H_

#include <plotms/GuiTabs/PlotMSAnnotatorTab.ui.h>

#include <casaqt/QtUtilities/QtPlotWidget.qo.h>
#include <plotms/Gui/PlotMSAnnotator.h>
#include <plotms/GuiTabs/PlotMSTab.qo.h>

#include <casa/namespace.h>

namespace casa {

// Subclass of PlotMSTab that handles interaction with the PlotMSAnnotator.
class PlotMSAnnotatorTab : public PlotMSTab, Ui::AnnotatorTab {
    Q_OBJECT
    
public:
    // Constructor which takes the parent plotter.
    PlotMSAnnotatorTab(PlotMSPlotter* parent);
    
    // Destructor.
    ~PlotMSAnnotatorTab();
    
    
    // Implements PlotMSTab::tabName().
    QString tabName() const { return "Annotator"; }
    
    // Implements PlotMSTab::toolButtons().
    QList<QToolButton*> toolButtons() const { return QList<QToolButton*>(); }
    
    // Implements PlotMSParametersWatcher::parametersHaveChanged().  Does
    // nothing.
    void parametersHaveChanged(const PlotMSWatchedParameters& params,
            int updateFlag, bool redrawRequired) { }
    
private:
    // Reference to annotator.
    PlotMSAnnotator& itsAnnotator_;
    
    // Text property widgets.
    // <group>
    PlotFontWidget* itsTextFont_;
    PlotLineWidget* itsTextOutline_;
    PlotFillWidget* itsTextBackground_;
    // </group>
    
    // Rectangle property widgets.
    // <group>
    PlotLineWidget* itsRectLine_;
    PlotFillWidget* itsRectFill_;
    // </group>
    
private slots:
    // Slot for when any text properties changed.
    void textPropertiesChanged();
    
    // Slot for when any rectangle properties changed.
    void rectanglePropertiesChanged();
};

}

#endif /* PLOTMSANNOTATORTAB_QO_H_ */
