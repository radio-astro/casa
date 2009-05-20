//# QtPlotWidget.qo.h: Classes for GUI editing of plot customization objects.
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
#ifndef QTPLOTWIDGET_QO_H_
#define QTPLOTWIDGET_QO_H_

#include <casaqt/QtUtilities/PlotColorWidget.ui.h>
#include <casaqt/QtUtilities/PlotFillWidget.ui.h>
#include <casaqt/QtUtilities/PlotLineWidget.ui.h>
#include <casaqt/QtUtilities/PlotSymbolWidget.ui.h>

#include <casaqt/QtUtilities/QtEditingWidget.qo.h>
#include <graphics/GenericPlotter/PlotFactory.h>

#include <casa/namespace.h>

namespace casa {

// Parent for plot widgets.
class QtPlotWidget : public QtEditingWidget {
    Q_OBJECT
    
public:
    // Constructor which takes a PlotFactory and an optional parent widget.
    QtPlotWidget(PlotFactoryPtr factory, QWidget* parent = NULL);
    
    // Destructor.
    virtual ~QtPlotWidget();
    
protected:
    // Factory.
    PlotFactoryPtr itsFactory_;
};


// Widget for choosing a plot color.
class PlotColorWidget : public QtPlotWidget, Ui::ColorWidget {
    Q_OBJECT
    
public:
    // Constructor which takes a PlotFactory, a flag for whether or not to show
    // an alpha spinner, and an optional parent widget.
    PlotColorWidget(PlotFactoryPtr factory, bool showAlpha = false,
            QWidget* parent = NULL);
    
    // Destructor.
    ~PlotColorWidget();    
    
    // Gets/Sets the currently set color on the widget.
    // <group>
    PlotColorPtr getColor() const;
    void setColor(PlotColorPtr color);
    // </group>
    
    // Gets/Sets the currently set color on the widget as a String.
    // <group>
    String getColorString() const;
    void setColor(const String& color);
    // </group>
    
private:
    // Last set color.
    PlotColorPtr itsColor_;
    
private slots:
    // Slot to show color chooser.
    void colorChoose();
    
    // Slot for when the set color changes.
    void colorChanged();
};


// Widget for choosing a plot area fill.  Uses a PlotColorWidget for the color.
class PlotFillWidget : public QtPlotWidget, Ui::FillWidget {
    Q_OBJECT
    
public:
    // Constructor which takes a PlotFactory, a flag for whether or not to show
    // an alpha spinner in the color widget, and an optional parent widget.
    PlotFillWidget(PlotFactoryPtr factory, bool showAlpha = false,
            QWidget* parent = NULL);
    
    // Destructor.
    ~PlotFillWidget();
    
    // Gets/Sets the currently set area fill on the widget.
    // <group>
    PlotAreaFillPtr getFill() const;
    void setFill(PlotAreaFillPtr fill);
    // </group>
    
private:
    // Color widget.
    PlotColorWidget* itsColorWidget_;
    
    // Last set area fill.
    PlotAreaFillPtr itsFill_;
    
private slots:
    // Slot for when the set area fill changes.
    void fillChanged();
};


// Widget for choosing a plot line.  Uses a PlotColorWidget for the line
// color.
class PlotLineWidget : public QtPlotWidget, Ui::LineWidget {
    Q_OBJECT
    
public:
    // Constructor which takes a PlotFactory, a flag for whether to use the
    // "compact" version of the widget, a flag for whether or not to show an
    // alpha spinner in the color widget, and an optional parent widget.
    PlotLineWidget(PlotFactoryPtr factory, bool useCompact = true,
            bool showAlpha = false, QWidget* parent = NULL);
    
    // Destructor.
    ~PlotLineWidget();    
    
    // Gets/Sets the currently set line on the widget.
    // <group>
    PlotLinePtr getLine() const;
    void setLine(PlotLinePtr line);
    // </group>
    
private:
    // Color widget.
    PlotColorWidget* itsColorWidget_;
    
    // Last set line.
    PlotLinePtr itsLine_;
    
    
    // Gets/Sets the line style from the combo box.
    // <group>
    PlotLine::Style lineStyle() const;
    void setLineStyle(PlotLine::Style style);
    // </group>
    
private slots:
    // Slot for when the set line changes.
    void lineChanged();
};


// Widget for choosing a plot symbol.  Uses a PlotFillWidget for the area fill
// and a PlotLineWidget for the outline.  Lets the user choose between no
// symbol, a default symbol given at construction, or a custom symbol they can
// set.
class PlotSymbolWidget : public QtPlotWidget, Ui::SymbolWidget {
    Q_OBJECT
    
public:
    // Constructor which takes a PlotFactory, the symbol associated with the
    // "default" options a flag for whether to show an alpha spinner for the
    // area fill, a flag for whether to allow the user to set a custom outline,
    // a flag for whether to show an alpha spinner for the custom outline, and
    // an optional parent widget.
    PlotSymbolWidget(PlotFactoryPtr factory, PlotSymbolPtr defaultSymbol,
            bool showAlphaFill = false, bool showCustom = true,
            bool showAlphaLine = false, QWidget* parent = NULL);
    
    // Destructor.
    ~PlotSymbolWidget();
    
    // Gets/Sets the currently set symbol on the widget.
    // <group>
    PlotSymbolPtr getSymbol() const;
    void setSymbol(PlotSymbolPtr symbol);
    // </group>
    
    // Overrides PlotMSWidget::addRadioButtonsToGroup().
    void addRadioButtonsToGroup(QButtonGroup* group) const;
    
private:
    // Area fill widget.
    PlotFillWidget* itsFillWidget_;
    
    // Line widget.
    PlotLineWidget* itsLineWidget_;
    
    // Default symbol.
    PlotSymbolPtr itsDefault_;
    
    // Last set symbol.
    PlotSymbolPtr itsSymbol_;
    
private slots:
    // Slot for when the set symbol changes.  The "check" flag can be used to
    // avoid emitting the changed signals twice (for example, when one radio
    // button turns off when another turns on).
    void symbolChanged(bool check = true);
};

}

#endif /* QTPLOTWIDGET_QO_H_ */
