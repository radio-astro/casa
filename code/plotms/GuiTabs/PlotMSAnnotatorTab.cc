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
#include <plotms/GuiTabs/PlotMSAnnotatorTab.qo.h>

#include <casaqt/QtUtilities/QtPlotWidget.qo.h>
#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/Gui/PlotMSAnnotator.h>
#include <plotms/Gui/PlotMSPlotter.qo.h>

namespace casa {

////////////////////////////////////
// PLOTMSANNOTATORTAB DEFINITIONS //
////////////////////////////////////

PlotMSAnnotatorTab::PlotMSAnnotatorTab(PlotMSPlotter* parent) :
        PlotMSTab(parent), itsAnnotator_(parent->getAnnotator()) {
    setupUi(this);
    
    PlotFactoryPtr factory = parent->getFactory();
    
    // Set up text widgets.
    itsTextFont_ = new PlotFontWidget(factory);
    itsTextOutline_ = new PlotLineWidget(factory, false);
    itsTextBackground_ = new PlotFillWidget(factory, true);
    QtUtilities::putInFrame(textFontFrame, itsTextFont_);
    QtUtilities::putInFrame(textOutlineFrame, itsTextOutline_);
    QtUtilities::putInFrame(textBackgroundFrame, itsTextBackground_);
    
    // Set up rectangle widgets.
    itsRectLine_ = new PlotLineWidget(factory, false);
    itsRectFill_ = new PlotFillWidget(factory);
    QtUtilities::putInFrame(rectLineFrame, itsRectLine_);
    QtUtilities::putInFrame(rectFillFrame, itsRectFill_);
    
    // Set up text properties.
    itsTextFont_->setFont(itsAnnotator_.textFont());
    itsTextOutline_->setLine(itsAnnotator_.textOutline());
    itsTextBackground_->setFill(itsAnnotator_.textBackground());
    
    // Set up rectangle properties.    
    itsRectLine_->setLine(itsAnnotator_.rectangleLine());
    itsRectFill_->setFill(itsAnnotator_.rectangleAreaFill());
    
    // Connect text widgets.
    connect(itsTextFont_, SIGNAL(changed()), SLOT(textPropertiesChanged()));
    connect(itsTextOutline_, SIGNAL(changed()), SLOT(textPropertiesChanged()));
    connect(itsTextBackground_, SIGNAL(changed()),
            SLOT(textPropertiesChanged()));
    
    // Connect rectangle widgets.
    connect(itsRectLine_,SIGNAL(changed()),SLOT(rectanglePropertiesChanged()));
    connect(itsRectFill_,SIGNAL(changed()),SLOT(rectanglePropertiesChanged()));
}

PlotMSAnnotatorTab::~PlotMSAnnotatorTab() { }


void PlotMSAnnotatorTab::textPropertiesChanged() {
    itsAnnotator_.setTextFont(itsTextFont_->getFont());
    itsAnnotator_.setTextOutline(itsTextOutline_->getLine());
    itsAnnotator_.setTextBackground(itsTextBackground_->getFill());
}

void PlotMSAnnotatorTab::rectanglePropertiesChanged() {
    itsAnnotator_.setRectangleLine(itsRectLine_->getLine());
    itsAnnotator_.setRectangleAreaFill(itsRectFill_->getFill());
}

}
