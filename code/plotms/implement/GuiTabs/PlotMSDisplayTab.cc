//# PlotMSDisplayTab.cc: Plot tab to manage plot display parameters.
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
#include <plotms/GuiTabs/PlotMSDisplayTab.qo.h>

#include <casaqt/QtUtilities/QtIndexChooser.qo.h>
#include <casaqt/QtUtilities/QtPlotWidget.qo.h>
#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>

namespace casa {

//////////////////////////////////
// PLOTMSDISPLAYTAB DEFINITIONS //
//////////////////////////////////

PlotMSDisplayTab::PlotMSDisplayTab(PlotMSPlotTab* tab, PlotMSPlotter* parent) :
        PlotMSPlotSubtab(tab, parent), itsPDisplay_(parent->getFactory()) {
    setupUi(this);
    
    // Setup widgets.
    itsIndexChooser_ = new QtIndexChooser(QtIndexChooser::ROW_COL);
    QtUtilities::putInFrame(chooserFrame, itsIndexChooser_);
    hideIndex();
    
    itsTitleWidget_ = new QtLabelWidget(PMS::DEFAULT_TITLE_FORMAT);
    PlotFactoryPtr factory = parent->getFactory();
    itsSymbolWidget_ = new PlotSymbolWidget(factory,
            PMS::DEFAULT_UNFLAGGED_SYMBOL(factory), false, false, false,false);
    itsMaskedSymbolWidget_ = new PlotSymbolWidget(factory,
            PMS::DEFAULT_FLAGGED_SYMBOL(factory), false, false, false, false);
    QtUtilities::putInFrame(titleFrame, itsTitleWidget_);
    QtUtilities::putInFrame(unflaggedFrame, itsSymbolWidget_);
    QtUtilities::putInFrame(flaggedFrame, itsMaskedSymbolWidget_);
    
    map<PlotSymbol::Symbol, int> minSymbolSizes = PMS::SYMBOL_MINIMUM_SIZES();
    itsSymbolWidget_->setMinimumSizes(minSymbolSizes);
    itsMaskedSymbolWidget_->setMinimumSizes(minSymbolSizes);
    
    // Setup colorize axis choices.
    colorizeChooser->addItem(PMS::axis(PMS::SCAN).c_str());
    colorizeChooser->addItem(PMS::axis(PMS::FIELD).c_str());
    colorizeChooser->addItem(PMS::axis(PMS::SPW).c_str());
    colorizeChooser->addItem(PMS::axis(PMS::ANTENNA1).c_str());
    colorizeChooser->addItem(PMS::axis(PMS::ANTENNA2).c_str());
    colorizeChooser->addItem(PMS::axis(PMS::BASELINE).c_str());
    colorizeChooser->addItem(PMS::axis(PMS::CHANNEL).c_str());
    colorizeChooser->addItem(PMS::axis(PMS::CORR).c_str());
    // Old way, not used because non-index METADATA items don't work!
    //    for(unsigned int i = 0; i < PlotMSCache::N_METADATA; i++)
    //        colorizeChooser->addItem(PMS::axis(PlotMSCache::METADATA[i]).c_str());

    // Setup colorize axis choices.

    
    // Set up label defaults.
    itsLabelDefaults_.insert(titleLabel, titleLabel->text());
    itsLabelDefaults_.insert(unflaggedLabel, unflaggedLabel->text());
    itsLabelDefaults_.insert(flaggedLabel, flaggedLabel->text());
    itsLabelDefaults_.insert(colorizeLabel, colorizeLabel->text());
    
    // Connect widgets.
    connect(itsIndexChooser_, SIGNAL(indexChanged(unsigned int)),
            SLOT(indexChanged(unsigned int)));
    connect(itsTitleWidget_, SIGNAL(changed()), SIGNAL(changed()));
    connect(itsSymbolWidget_, SIGNAL(changed()), SIGNAL(changed()));
    connect(itsMaskedSymbolWidget_, SIGNAL(changed()), SIGNAL(changed()));
    connect(colorize, SIGNAL(toggled(bool)), SIGNAL(changed()));
    connect(colorizeChooser, SIGNAL(currentIndexChanged(int)),
            SIGNAL(changed()));
    
}

PlotMSDisplayTab::~PlotMSDisplayTab() { }


void PlotMSDisplayTab::getValue(PlotMSPlotParameters& params) const {
    PMS_PP_Display* d = params.typedGroup<PMS_PP_Display>();
    if(d == NULL) {
        params.setGroup<PMS_PP_Display>();
        d = params.typedGroup<PMS_PP_Display>();
    }
        
    *d = itsPDisplay_;
        
    unsigned int index = 0;
    if(itsIndexChooser_->isVisible()) index = itsIndexChooser_->index();
    
    d->setTitleFormat(itsTitleWidget_->getValue(), index);
    d->setUnflaggedSymbol(itsSymbolWidget_->getSymbol(), index);
    d->setFlaggedSymbol(itsMaskedSymbolWidget_->getSymbol(), index);
    
    d->setColorize(colorize->isChecked(),
            PMS::axis(colorizeChooser->currentText().toStdString()), index);
}

void PlotMSDisplayTab::setValue(const PlotMSPlotParameters& params) {
    const PMS_PP_Display* d = params.typedGroup<PMS_PP_Display>();
    if(d == NULL) return; // shouldn't happen
    
    itsPDisplay_ = *d;
    
    unsigned int index = 0;
    if(itsIndexChooser_->isVisible()) index = itsIndexChooser_->index();
    indexChanged(index);
}

void PlotMSDisplayTab::update(const PlotMSPlot& plot) {    
    const PlotMSPlotParameters& params = plot.parameters();
    PlotMSPlotParameters newParams(params);
    getValue(newParams);
    
    const PMS_PP_Display* d = params.typedGroup<PMS_PP_Display>(),
                        *d2 = newParams.typedGroup<PMS_PP_Display>();
    if(d == NULL || d2 == NULL) return; // shouldn't happen
    
    unsigned int index = 0;
    if(itsIndexChooser_->isVisible()) index = itsIndexChooser_->index();
    
    highlightWidgetText(titleLabel, d->titleFormat(index) != d2->titleFormat(index));
    highlightWidgetText(unflaggedLabel,
                *d->unflaggedSymbol(index) != *d2->unflaggedSymbol(index));
    highlightWidgetText(flaggedLabel,
                *d->flaggedSymbol(index) != *d2->flaggedSymbol(index));
    highlightWidgetText(colorizeLabel,
                d->colorizeFlag(index) != d2->colorizeFlag(index) ||
                (d->colorizeFlag(index) &&
                 d->colorizeAxis(index)!= d2->colorizeAxis(index)));
}

void PlotMSDisplayTab::hideIndex() {
    chooserFrame->setVisible(false);
    chooserLine->setVisible(false);
}

void PlotMSDisplayTab::setIndexRowsCols(unsigned int nRows,unsigned int nCols){
    chooserFrame->setVisible(true);
    chooserLine->setVisible(true);
    itsIndexChooser_->setType(QtIndexChooser::ROW_COL);
    itsIndexChooser_->setRowsCols(nRows, nCols);
    
    itsPDisplay_.resizeVectors(nRows * nCols);
}


// Private Slots //

void PlotMSDisplayTab::indexChanged(unsigned int index) {
    // TODO
    // set old index values in itsPDisplay_
    
    itsTitleWidget_->setValue(itsPDisplay_.titleFormat(index).format);
    itsSymbolWidget_->setSymbol(itsPDisplay_.unflaggedSymbol(index));
    itsMaskedSymbolWidget_->setSymbol(itsPDisplay_.flaggedSymbol(index));
    
    colorize->setChecked(itsPDisplay_.colorizeFlag(index));
    setChooser(colorizeChooser, PMS::axis(itsPDisplay_.colorizeAxis(index)));
}

}
