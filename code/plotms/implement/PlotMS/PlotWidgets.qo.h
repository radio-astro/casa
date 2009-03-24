//# PlotWidgets.qo.h: Common widgets for making plot objects.
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
#ifndef PLOTWIDGETS_QO_H_
#define PLOTWIDGETS_QO_H_

#include <plotms/PlotMS/PlotColorWidget.ui.h>
#include <plotms/PlotMS/PlotFileWidget.ui.h>
#include <plotms/PlotMS/PlotFillWidget.ui.h>
#include <plotms/PlotMS/PlotLabelWidget.ui.h>
#include <plotms/PlotMS/PlotLineWidget.ui.h>
#include <plotms/PlotMS/PlotRangeWidget.ui.h>
#include <plotms/PlotMS/PlotSymbolWidget.ui.h>

#include <graphics/GenericPlotter/PlotFactory.h>

#include <QMutex>
#include <QScrollArea>
#include <QWidget>

#include <casa/namespace.h>

namespace casa {

// Parent for plot widgets.
class PlotMSWidget : public QWidget {
    Q_OBJECT
    
public:
    // Constructor which takes a PlotFactory and an optional parent widget.
    PlotMSWidget(PlotFactoryPtr factory, QWidget* parent = NULL);
    
    // Destructor.
    virtual ~PlotMSWidget();
    
    // Adds any radio buttons in the widget to the given button group.  This
    // method should be overridden by children that have radio buttons.
    virtual void addRadioButtonsToGroup(QButtonGroup* group) const { }
    
signals:
    // This signal should be emitted whenever the user changes any value
    // settings in the GUI.
    void changed();
    
    // This signal should be emitted when the user changes any value settings
    // in the GUI AND the new value is different from the last set value.
    void differentFromSet();
    
protected:
    // Factory.
    PlotFactoryPtr itsFactory_;
};


// Widget for choosing a plot color.
class PlotColorWidget : public PlotMSWidget, Ui::ColorWidget {
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
class PlotFillWidget : public PlotMSWidget, Ui::FillWidget {
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
class PlotLineWidget : public PlotMSWidget, Ui::LineWidget {
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
class PlotSymbolWidget : public PlotMSWidget, Ui::SymbolWidget {
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


// Widget for choosing a label.  Lets the user choose between no label, a
// default label given at construction, or a custom label they can set.
class PlotLabelWidget : public PlotMSWidget, Ui::LabelWidget {
    Q_OBJECT
    
public:
    // Constructor which takes the label associated with the "default" option,
    // and an optional parent widget.
    PlotLabelWidget(const String& defaultLabel, QWidget* parent = NULL);
    
    // Destructor.
    ~PlotLabelWidget();
    
    // Gets/Sets the currently set label on the widget.
    // <group>
    String getValue() const;
    void setValue(const String& value);
    // </group>
    
    // Overrides PlotMSWidget::addRadioButtonsToGroup().
    void addRadioButtonsToGroup(QButtonGroup* group) const;
    
private:
    // Default label.
    String itsDefault_;

    // Last set label.
    String itsValue_;
    
private slots:
    // Slot for when the set label changes.  The "check" flag can be used to
    // avoid emitting the changed signals twice (for example, when one radio
    // button turns off when another turns on).
    void labelChanged(bool check = true);
};


// Widget for choosing a file.
class PlotFileWidget : public PlotMSWidget, Ui::FileWidget {
    Q_OBJECT
    
public:
    // Constructor which takes a flag for whether the file is a directory or
    // not (for the file chooser), a flag for whether the file is for saving
    // (and thus doesn't need to exist, for the file chooser), and an optional
    // parent widget.
    PlotFileWidget(bool chooseDirectory, bool saveFile, QWidget* parent= NULL);
    
    // Destructor.
    ~PlotFileWidget();
    
    // Gets/Sets the currently set file.
    // <group>
    String getFile() const;
    void setFile(const String& file);
    // </group>
    
private:
    // File chooser directory flag.
    bool isDirectory_;
    
    // File chooser save file flag.
    bool isSave_;
    
    // Last set file.
    String itsFile_;
    
private slots:
    // Slot to show a file chooser and set the result in the line edit.
    void browse();

    // Slot for when the set file changes.
    void fileChanged();
};


// Widget for selecting a plot axis range.  Lets the user choose between an
// automatic range or a custom range they can set.  Ranges can either be normal
// doubles, or two dates which are converted to/from doubles as needed using
// PMS::dateDouble().
class PlotRangeWidget : public PlotMSWidget, Ui::RangeWidget {
    Q_OBJECT
    
public:
    // Constructor which takes an optional parent widget.
    PlotRangeWidget(QWidget* parent = NULL);
    
    // Destructor.
    ~PlotRangeWidget();
    
    
    // Gets/Sets whether or not the displayed range widgets are for dates or
    // not.
    // <group>
    bool isDate() const;    
    void setIsDate(bool isDate = true);
    // </group>
    
    // Gets whether the currently set date is custom or not.
    bool isCustom() const;
    
    // Gets/Sets the currently set range.
    pair<double, double> getRange() const;    
    void getRange(double& from, double& to) {
        pair<double, double> r = getRange();
        from = r.first;
        to = r.second;
    }    
    void setRange(bool isDate, bool isCustom, double from, double to);
    // </group>
    
    // Overrides PlotMSWidget::addRadioButtonsToGroup().
    void addRadioButtonsToGroup(QButtonGroup* group) const;
    
private:
    // Whether last set range was custom or not.
    bool isCustom_;
    
    // Last set range.
    double from_, to_;
    
private slots:
    // Slot for when the set range changes.
    void rangeChanged();
};

}

#endif /* PLOTWIDGETS_QO_H_ */
