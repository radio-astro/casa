//# PlotWidgets.cc: Common widgets for making plot objects.
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
#include <plotms/PlotMS/PlotWidgets.qo.h>

#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/PlotMS/PlotMSConstants.h>

#include <QCloseEvent>
#include <QColorDialog>
#include <QFileDialog>

#include <limits>

namespace casa {

//////////////////////////////
// PLOTMSWIDGET DEFINITIONS //
//////////////////////////////

PlotMSWidget::PlotMSWidget(PlotFactoryPtr factory, QWidget* parent) :
        QWidget(parent), itsFactory_(factory) { }
PlotMSWidget::~PlotMSWidget() { }


/////////////////////////////////
// PLOTCOLORWIDGET DEFINITIONS //
/////////////////////////////////

PlotColorWidget::PlotColorWidget(PlotFactoryPtr factory, bool showAlpha,
        QWidget* parent) : PlotMSWidget(factory, parent) {
    setupUi(this);
    if(!showAlpha) {
        alphaLabel->setVisible(false);
        alpha->setVisible(false);
    }
    setColor(factory->color("000000"));
    
    connect(colorEdit, SIGNAL(editingFinished()), SLOT(colorChanged()));
    connect(choose, SIGNAL(clicked()), SLOT(colorChoose()));
    if(showAlpha)
        connect(alpha, SIGNAL(valueChanged(int)), SLOT(colorChanged()));
}

PlotColorWidget::~PlotColorWidget() { }

PlotColorPtr PlotColorWidget::getColor() const {
    PlotColorPtr color = itsFactory_->color(colorEdit->text().toStdString());
    if(alpha->isVisible()) color->setAlpha(alpha->value());
    return color;
}

void PlotColorWidget::setColor(PlotColorPtr color) {
    if(!color.null()) {
        blockSignals(true);
        bool changed = itsColor_.null() || *itsColor_ != *color;
        
        itsColor_ = itsFactory_->color(*color);
        colorEdit->setText(color->asHexadecimal().c_str());
        if(alpha->isVisible()) alpha->setValue(color->alpha());
        
        blockSignals(false);
        if(changed) emit this->changed();
    }
}

String PlotColorWidget::getColorString() const {
    return colorEdit->text().toStdString(); }

void PlotColorWidget::setColor(const String& color) {
    setColor(itsFactory_->color(color)); }

void PlotColorWidget::colorChoose() {
    QColor color = QColorDialog::getColor(QColor("#" + colorEdit->text()));
    if(color.isValid()) colorEdit->setText(color.name().replace('#', ""));
}

void PlotColorWidget::colorChanged() {
    emit changed();
    PlotColorPtr currColor = getColor();
    if(*currColor != *itsColor_) emit differentFromSet();
}


////////////////////////////////
// PLOTFILLWIDGET DEFINITIONS //
////////////////////////////////

PlotFillWidget::PlotFillWidget(PlotFactoryPtr factory, bool showAlpha,
        QWidget* parent) : PlotMSWidget(factory, parent) {
    setupUi(this);
    itsColorWidget_ = new PlotColorWidget(factory, showAlpha);
    QtUtilities::putInFrame(colorFrame, itsColorWidget_);
    
    setFill(factory->areaFill("black"));
    
    connect(itsColorWidget_, SIGNAL(changed()), SLOT(fillChanged()));
    connect(fillChooser, SIGNAL(currentIndexChanged(int)), SLOT(fillChanged()));
}

PlotFillWidget::~PlotFillWidget() { }

PlotAreaFillPtr PlotFillWidget::getFill() const {
    PlotAreaFillPtr fill = itsFactory_->areaFill("");
    fill->setColor(itsColorWidget_->getColor());
    
    int index = fillChooser->currentIndex();
    PlotAreaFill::Pattern pattern = PlotAreaFill::NOFILL;
    if(index == 0)      pattern = PlotAreaFill::FILL;
    else if(index == 1) pattern = PlotAreaFill::MESH1;
    else if(index == 2) pattern = PlotAreaFill::MESH2;
    else if(index == 3) pattern = PlotAreaFill::MESH3;
    fill->setPattern(pattern);
    
    return fill;
}

void PlotFillWidget::setFill(PlotAreaFillPtr fill) {
    if(!fill.null()) {
        blockSignals(true);
        bool changed = itsFill_.null() || *itsFill_ != *fill;
        
        itsFill_ = itsFactory_->areaFill(*fill);
        itsColorWidget_->setColor(fill->color());
        
        PlotAreaFill::Pattern pattern = fill->pattern();
        int index = 4;
        if(pattern == PlotAreaFill::FILL)       index = 0;
        else if(pattern == PlotAreaFill::MESH1) index = 1;
        else if(pattern == PlotAreaFill::MESH2) index = 2;
        else if(pattern == PlotAreaFill::MESH3) index = 3;
        fillChooser->setCurrentIndex(index);
        
        blockSignals(false);
        if(changed) emit this->changed();
    }
}

void PlotFillWidget::fillChanged() {
    emit changed();
    PlotAreaFillPtr currFill = getFill();
    if(*currFill != *itsFill_) emit differentFromSet();
}


////////////////////////////////
// PLOTLINEWIDGET DEFINITIONS //
////////////////////////////////

PlotLineWidget::PlotLineWidget(PlotFactoryPtr factory, bool useCompact,
        bool showAlpha, QWidget* parent) : PlotMSWidget(factory, parent) {
    setupUi(this);
    stackedWidget->setCurrentIndex(useCompact ? 0 : 1);
    
    itsColorWidget_ = new PlotColorWidget(factory, showAlpha);
    QtUtilities::putInFrame(useCompact ? cColorFrame : nColorFrame,
                            itsColorWidget_);
    
    setLine(factory->line("black"));
    
    if(useCompact) {
        connect(cWidth, SIGNAL(valueChanged(int)), SLOT(lineChanged()));
        connect(cStyle, SIGNAL(currentIndexChanged(int)), SLOT(lineChanged()));
    } else {
        connect(nWidth, SIGNAL(valueChanged(int)), SLOT(lineChanged()));
        connect(nStyle, SIGNAL(currentIndexChanged(int)), SLOT(lineChanged()));
    }
    connect(itsColorWidget_, SIGNAL(changed()), SLOT(lineChanged()));
}

PlotLineWidget::~PlotLineWidget() { }

PlotLinePtr PlotLineWidget::getLine() const {
    PlotLinePtr line = itsFactory_->line("");
    line->setWidth((stackedWidget->currentIndex() == 0) ?
                   cWidth->value() : nWidth->value());
    line->setStyle(lineStyle());
    line->setColor(itsColorWidget_->getColor());
    return line;
}

void PlotLineWidget::setLine(PlotLinePtr line) {
    if(!line.null()) {
        blockSignals(true);
        bool changed = itsLine_.null() || *itsLine_ != *line;
        
        itsLine_ = itsFactory_->line(*line);
        int width = (int)(line->width() + 0.5);
        if(stackedWidget->currentIndex() == 0) cWidth->setValue(width);
        else                                   nWidth->setValue(width);
        setLineStyle(line->style());
        itsColorWidget_->setColor(line->color());
        
        blockSignals(false);
        if(changed) emit this->changed();
    }
}

PlotLine::Style PlotLineWidget::lineStyle() const {
    int index = (stackedWidget->currentIndex() == 0) ?
                cStyle->currentIndex() : nStyle->currentIndex();
    if(index == 0) return PlotLine::SOLID;
    else if(index == 1) return PlotLine::DASHED;
    else if(index == 2) return PlotLine::DOTTED;
    else return PlotLine::NOLINE;
}

void PlotLineWidget::setLineStyle(PlotLine::Style style) {
    int index;
    if(style == PlotLine::SOLID)       index = 0;
    else if(style == PlotLine::DASHED) index = 1;
    else if(style == PlotLine::DOTTED) index = 2;
    else                               index = 3;
    if(stackedWidget->currentIndex() == 0) cStyle->setCurrentIndex(index);
    else                                   nStyle->setCurrentIndex(index);
}

void PlotLineWidget::lineChanged() {
    emit changed();
    PlotLinePtr currLine = getLine();
    if(*currLine != *itsLine_) emit differentFromSet();
}


//////////////////////////////////
// PLOTSYMBOLWIDGET DEFINITIONS //
//////////////////////////////////

PlotSymbolWidget::PlotSymbolWidget(PlotFactoryPtr factory,
        PlotSymbolPtr defaultSymbol, bool showAlphaFill, bool showCustom,
        bool showAlphaLine, QWidget* parent) : PlotMSWidget(factory, parent) {
    setupUi(this);
    itsFillWidget_ = new PlotFillWidget(factory, showAlphaFill);
    itsLineWidget_ = new PlotLineWidget(factory, false, showAlphaLine);
    QtUtilities::putInFrame(fillFrame, itsFillWidget_);
    QtUtilities::putInFrame(outlineFrame, itsLineWidget_);
    outlineFrame->setEnabled(false);
    
    if(!showCustom) outlineCustomFrame->setVisible(false);
    
    if(defaultSymbol.null()) itsDefault_ = PMS::DEFAULT_SYMBOL(itsFactory_);
    else                     itsDefault_ = itsFactory_->symbol(*defaultSymbol);
    
    setSymbol(itsDefault_);
    
    // only emit change for radio buttons turned on
    connect(noneButton, SIGNAL(toggled(bool)), SLOT(symbolChanged(bool)));
    connect(defaultButton, SIGNAL(toggled(bool)), SLOT(symbolChanged(bool)));
    connect(customButton, SIGNAL(toggled(bool)), SLOT(symbolChanged(bool)));
    connect(outlineNone, SIGNAL(toggled(bool)), SLOT(symbolChanged(bool)));
    connect(outlineDefault, SIGNAL(toggled(bool)), SLOT(symbolChanged(bool)));
    connect(outlineCustom, SIGNAL(toggled(bool)), SLOT(symbolChanged(bool)));
    
    connect(SymbolWidget::size, SIGNAL(valueChanged(int)),
            SLOT(symbolChanged()));
    connect(SymbolWidget::style, SIGNAL(currentIndexChanged(int)),
            SLOT(symbolChanged()));
    connect(charEdit, SIGNAL(editingFinished()), SLOT(symbolChanged()));
    connect(itsFillWidget_, SIGNAL(changed()), SLOT(symbolChanged()));
    connect(itsLineWidget_, SIGNAL(changed()), SLOT(symbolChanged()));
}

PlotSymbolWidget::~PlotSymbolWidget() { }

PlotSymbolPtr PlotSymbolWidget::getSymbol() const {
    if(defaultButton->isChecked()) return itsFactory_->symbol(*itsDefault_);
    else if(customButton->isChecked()) {
        int i = SymbolWidget::style->currentIndex();
        PlotSymbol::Symbol s = PlotSymbol::NOSYMBOL;
        if(i == 0)      s = PlotSymbol::CIRCLE;
        else if(i == 1) s = PlotSymbol::SQUARE;
        else if(i == 2) s = PlotSymbol::DIAMOND;
        else if(i == 3) s = PlotSymbol::PIXEL;
        else if(i == 4) s = PlotSymbol::CHARACTER;
        
        PlotSymbolPtr sym = itsFactory_->symbol(s);
        QString text = charEdit->text();
        if(s == PlotSymbol::CHARACTER && text.size() >= 1)
            sym->setUSymbol(text[0].unicode());
        
        i = SymbolWidget::size->value();
        sym->setSize(i, i);
        sym->setAreaFill(itsFillWidget_->getFill());
        
        if(outlineNone->isChecked())
            sym->setLine(itsFactory_->line("black", PlotLine::NOLINE, 1));
        else if(outlineDefault->isChecked())
            sym->setLine(itsFactory_->line("black", PlotLine::SOLID, 1));
        else
            sym->setLine(itsLineWidget_->getLine());
        
        return sym;
    }
    return itsFactory_->symbol(PlotSymbol::NOSYMBOL);
}

void PlotSymbolWidget::setSymbol(PlotSymbolPtr symbol) {
    if(symbol.null()) itsSymbol_ = itsFactory_->symbol(PlotSymbol::NOSYMBOL);
    else              itsSymbol_ = itsFactory_->symbol(*symbol);

    blockSignals(true);
    bool changed = itsSymbol_.null() || *itsSymbol_ != *symbol;
    
    if(itsSymbol_->symbol() == PlotSymbol::NOSYMBOL)
        noneButton->setChecked(true);
    else if(*itsSymbol_ == *itsDefault_) defaultButton->setChecked(true);
    else customButton->setChecked(true);
    
    SymbolWidget::size->setValue((int)(itsSymbol_->size().first + 0.5));

    PlotSymbol::Symbol s = itsSymbol_->symbol();
    int index = 0;
    if(s == PlotSymbol::SQUARE)    index = 1;
    else if(s == PlotSymbol::DIAMOND)   index = 2;
    else if(s == PlotSymbol::PIXEL)     index = 3;
    else if(s == PlotSymbol::CHARACTER) index = 4;
    SymbolWidget::style->setCurrentIndex(index);
    charEdit->setEnabled(s == PlotSymbol::CHARACTER);
    if(s == PlotSymbol::CHARACTER)
        charEdit->setText(QString(itsSymbol_->symbolUChar()));

    itsFillWidget_->setFill(itsSymbol_->areaFill());
    PlotLinePtr line = itsSymbol_->line();
    if(line->style() == PlotLine::NOLINE) outlineNone->setChecked(true);
    else if(*line == *itsFactory_->line("black", PlotLine::SOLID, 1))
        outlineDefault->setChecked(true);
    else {
        outlineCustom->setChecked(true);
        itsLineWidget_->setLine(line);
    }
    
    blockSignals(false);
    if(changed) emit this->changed();
}

void PlotSymbolWidget::addRadioButtonsToGroup(QButtonGroup* group) const {
    if(group == NULL) return;
    group->addButton(noneButton);
    group->addButton(defaultButton);
    group->addButton(customButton);
}

void PlotSymbolWidget::symbolChanged(bool check) {
    if(!check) return;
    
    emit changed();
    PlotSymbolPtr currSymbol = getSymbol();
    charEdit->setEnabled(currSymbol->symbol() == PlotSymbol::CHARACTER);
    SymbolWidget::size->setEnabled(currSymbol->symbol() != PlotSymbol::PIXEL);
    if(*currSymbol != *itsSymbol_) emit differentFromSet();
}


/////////////////////////////////
// PLOTLABELWIDGET DEFINITIONS //
/////////////////////////////////

PlotLabelWidget::PlotLabelWidget(const String& defaultLabel, QWidget* parent) :
        PlotMSWidget(PlotFactoryPtr(), parent), itsDefault_(defaultLabel) {
    setupUi(this);
    
    setValue(defaultLabel);
    
    // only emit change for radio buttons turned on
    connect(noneButton, SIGNAL(toggled(bool)), SLOT(labelChanged(bool)));
    connect(defaultButton, SIGNAL(toggled(bool)), SLOT(labelChanged(bool)));
    connect(customButton, SIGNAL(toggled(bool)), SLOT(labelChanged(bool)));
    
    connect(customEdit, SIGNAL(editingFinished()), SLOT(labelChanged()));
}

PlotLabelWidget::~PlotLabelWidget() { }

String PlotLabelWidget::getValue() const {
    if(noneButton->isChecked()) return "";
    else if(defaultButton->isChecked()) return itsDefault_;
    else return customEdit->text().toStdString();
}

void PlotLabelWidget::setValue(const String& value) {
    blockSignals(true);
    bool changed = value != itsValue_;
    
    itsValue_ = value;
    if(value == "")               noneButton->setChecked(true);
    else if(value == itsDefault_) defaultButton->setChecked(true);
    else                          customButton->setChecked(true);
    customEdit->setText(value.c_str());
    
    blockSignals(false);
    if(changed) emit this->changed();
}

void PlotLabelWidget::addRadioButtonsToGroup(QButtonGroup* group) const {
    if(group == NULL) return;
    group->addButton(noneButton);
    group->addButton(defaultButton);
    group->addButton(customButton);
}

void PlotLabelWidget::labelChanged(bool check) {
    if(!check) return;
    
    emit changed();
    if(getValue() != itsValue_) emit differentFromSet();
}


////////////////////////////////
// PLOTFILEWIDGET DEFINITIONS //
////////////////////////////////

PlotFileWidget::PlotFileWidget(bool directory, bool save, QWidget* parent) :
        PlotMSWidget(PlotFactoryPtr(), parent), isDirectory_(directory),
        isSave_(save) {
    setupUi(this);
    connect(file, SIGNAL(editingFinished()), SLOT(fileChanged()));
    connect(FileWidget::browse, SIGNAL(clicked()), SLOT(browse()));
}

PlotFileWidget::~PlotFileWidget() { }

String PlotFileWidget::getFile() const { return file->text().toStdString(); }
void PlotFileWidget::setFile(const String& f) {
    blockSignals(true);
    bool changed = f != itsFile_;
    
    itsFile_ = f;
    file->setText(f.c_str());
    
    blockSignals(false);
    if(changed) emit this->changed();
}

void PlotFileWidget::browse() {
    QString f;
    if(isDirectory_) f = QFileDialog::getExistingDirectory();
    else if(isSave_) f = QFileDialog::getSaveFileName();
    else             f = QFileDialog::getOpenFileName();
    if(!f.isEmpty()) file->setText(f);
}

void PlotFileWidget::fileChanged() {
    emit changed();
    if(getFile() != itsFile_) emit differentFromSet();
}


/////////////////////////////////
// PLOTRANGEWIDGET DEFINITIONS //
/////////////////////////////////

PlotRangeWidget::PlotRangeWidget(QWidget* parent) :
        PlotMSWidget(PlotFactoryPtr(), parent) {
    setupUi(this);
    
    doubleFrom->setMinimum(numeric_limits<double>::min());
    doubleFrom->setMaximum(numeric_limits<double>::max());
    doubleTo->setMinimum(numeric_limits<double>::min());
    doubleTo->setMaximum(numeric_limits<double>::max());
    
    setRange(false, false, 0, 0);
    
    connect(automatic, SIGNAL(toggled(bool)), SLOT(rangeChanged()));
    connect(doubleFrom, SIGNAL(valueChanged(double)), SLOT(rangeChanged()));
    connect(doubleTo, SIGNAL(valueChanged(double)), SLOT(rangeChanged()));
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

pair<double, double> PlotRangeWidget::getRange() const {
    if(isDate()) {
        QDate date = timeFrom->date(); QTime time = timeFrom->time();
        double from = PMS::dateDouble(date.year(), date.month(), date.day(),
                time.hour(), time.minute(),
                time.second() + (time.msec() / 1000.0));
        
        date = timeTo->date(); time = timeTo->time();
        double to = PMS::dateDouble(date.year(), date.month(), date.day(),
                time.hour(), time.minute(),
                time.second() + (time.msec() / 1000.0));
        return pair<double, double>(from, to);
        
    } else
        return pair<double, double>(doubleFrom->value(), doubleTo->value());
}

void PlotRangeWidget::setRange(bool isDate, bool isCustom, double from,
        double to) {
    blockSignals(true);
    
    stackedWidget->setCurrentIndex(isDate ? 1 : 0);
    custom->setChecked(isCustom);
    
    bool changed = isDate != this->isDate() || isCustom != this->isCustom();
    if(!changed) {
        pair<double, double> range = getRange();
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
        doubleFrom->setValue(from);
        doubleTo->setValue(to);
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
    pair<double, double> r = getRange();
    if(isCustom() != isCustom_ || r.first != from_ || r.second != to_)
        emit differentFromSet();
}

}
