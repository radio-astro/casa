//# PlotMSMultiAxesTab.qo.h: Plot tab for multi axes parameters.
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
#ifndef PLOTMSMULTIAXESTAB_QO_H_
#define PLOTMSMULTIAXESTAB_QO_H_

#include <plotms/GuiTabs/PlotMSMultiAxesTab.ui.h>

#include <plotms/GuiTabs/PlotMSPlotTab.qo.h>

#include <casa/namespace.h>

namespace casa {

//# Forward declarations.
class PlotMSAxisWidget;


// Subclass of PlotMSPlotSubtab that manages multiple plot axes parameters.
class PlotMSMultiAxesTab : public PlotMSPlotSubtab, Ui::MultiAxesTab {
    Q_OBJECT
    
public:
    // Constructor which takes the parent tab and plotter.
    PlotMSMultiAxesTab(PlotMSPlotTab* plotTab, PlotMSPlotter* parent);
    
    // Destructor.
    ~PlotMSMultiAxesTab();
    
    
    // Implements PlotMSTab::tabName().
    QString tabName() const { return "Axes"; }
    
    // Implements PlotMSPlotSubtab::getValue().
    void getValue(PlotMSPlotParameters& params) const;
    
    // Implements PlotMSPlotSubtab::setValue().
    void setValue(const PlotMSPlotParameters& params);
    
    // Implements PlotMSPlotSubtab::update().
    void update(const PlotMSPlot& plot);
    
private:
    // Class that manages the list of axes widgets.
    class MultiAxes {        
    public:
        // Constructor that takes parent.
        MultiAxes(PlotMSMultiAxesTab* parent);
        
        // Destructor.
        ~MultiAxes();
        
        
        // Sets up into the given widget.
        void setupUi(QWidget* widget);
        
        // Returns the size of the axes list.
        int size() const;
        
        // Returns the axis widget at the given index.
        // <group>
        PlotMSAxisWidget* axis(int index);
        const PlotMSAxisWidget* axis(int index) const;
        // </group>
        
        // Adds the given axis with the given canvas attach axes flag.
        // Connects the remove button to the given slot on the parent.
        void addAxis(PMS::Axis value, int attachAxes, const char* removeSlot);
        
        // Removes the axis at the given index.
        void removeAxis(int index);
        
        // Returns the index of the given button.
        int indexOfButton(QPushButton* button);
        
    private:
        // Parent.
        PlotMSMultiAxesTab* itsParent_;
        
        // Main layout.
        QVBoxLayout* itsLayout_;
        
        // Layouts, that hold the widget and close buttons.
        QList<QHBoxLayout*> itsLayouts_;
        
        // Axes widgets.
        QList<PlotMSAxisWidget*> itsAxisWidgets_;
        
        // Buttons.
        QList<QPushButton*> itsButtons_;
    };
    
    // X and Y axes lists.
    MultiAxes itsXAxes_, itsYAxes_;
    
    
private slots:
    // Slots for adding new axes.
    // <group>
    void addX();
    void addY();
    // </group>
    
    // Slots for removing an axis.
    // <group>
    void removeX();
    void removeY();
    // </group>
};

}

#endif /* PLOTMSMULTIAXESTAB_QO_H_ */
