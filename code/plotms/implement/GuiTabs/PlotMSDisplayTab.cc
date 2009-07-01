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

#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/Gui/PlotMSPlotter.qo.h>

namespace casa {

//////////////////////////////////
// PLOTMSDISPLAYTAB DEFINITIONS //
//////////////////////////////////

PlotMSDisplayTab::PlotMSDisplayTab(PlotMSPlotter* parent) :
        PlotMSPlotSubtab(parent) {
    setupUi(this);
    
    // Setup widgets.
    itsTitleWidget_ = new QtLabelWidget(PMS::DEFAULT_TITLE_FORMAT);
    PlotFactoryPtr factory = parent->getFactory();
    itsSymbolWidget_ = new PlotSymbolWidget(factory,
            PMS::DEFAULT_SYMBOL(factory), false, false, false, false);
    itsMaskedSymbolWidget_ = new PlotSymbolWidget(factory,
            PMS::DEFAULT_MASKED_SYMBOL(factory), false, false, false, false);
    QtUtilities::putInFrame(titleFrame, itsTitleWidget_);
    QtUtilities::putInFrame(unflaggedFrame, itsSymbolWidget_);
    QtUtilities::putInFrame(flaggedFrame, itsMaskedSymbolWidget_);
    
    map<PlotSymbol::Symbol, int> minSymbolSizes = PMS::SYMBOL_MINIMUM_SIZES();
    itsSymbolWidget_->setMinimumSizes(minSymbolSizes);
    itsMaskedSymbolWidget_->setMinimumSizes(minSymbolSizes);
    
    itsLabelDefaults_.insert(titleLabel, titleLabel->text());
    itsLabelDefaults_.insert(unflaggedLabel, unflaggedLabel->text());
    itsLabelDefaults_.insert(flaggedLabel, flaggedLabel->text());
    
    // Connect widgets.
    connect(itsTitleWidget_, SIGNAL(changed()), SIGNAL(changed()));
    connect(itsSymbolWidget_, SIGNAL(changed()), SIGNAL(changed()));
    connect(itsMaskedSymbolWidget_, SIGNAL(changed()), SIGNAL(changed()));
}

PlotMSDisplayTab::~PlotMSDisplayTab() { }


void PlotMSDisplayTab::getValue(PlotMSPlotParameters& params) const {
    PlotMSSinglePlotParameters* sp =
        dynamic_cast<PlotMSSinglePlotParameters*>(&params);
    if(sp == NULL) return;
    
    sp->setPlotTitleFormat(itsTitleWidget_->getValue());
    sp->setSymbol(itsSymbolWidget_->getSymbol());
    sp->setMaskedSymbol(itsMaskedSymbolWidget_->getSymbol());
}

void PlotMSDisplayTab::setValue(const PlotMSPlotParameters& params) {
    const PlotMSSinglePlotParameters* sp =
        dynamic_cast<const PlotMSSinglePlotParameters*>(&params);
    if(sp == NULL) return;
    
    itsTitleWidget_->setValue(sp->plotTitleFormat().format);
    itsSymbolWidget_->setSymbol(sp->symbol());
    itsMaskedSymbolWidget_->setSymbol(sp->maskedSymbol());
}

void PlotMSDisplayTab::update(const PlotMSPlot& plot) {
    const PlotMSSinglePlot* p = dynamic_cast<const PlotMSSinglePlot*>(&plot);
    if(p == NULL) return;
    
    const PlotMSSinglePlotParameters& params = p->singleParameters();
    PlotMSSinglePlotParameters newParams(params);
    getValue(newParams);
    
    changedText(titleLabel,
                params.plotTitleFormat() != newParams.plotTitleFormat());
    changedText(unflaggedLabel, *params.symbol() != *newParams.symbol());
    changedText(flaggedLabel,
                *params.maskedSymbol() != *newParams.maskedSymbol());
}

}
