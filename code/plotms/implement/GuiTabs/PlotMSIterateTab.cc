//# PlotMSIterateTab.cc: Plot tab to manage plot display parameters.
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
#include <plotms/GuiTabs/PlotMSIterateTab.qo.h>


#include <casaqt/QtUtilities/QtPlotWidget.qo.h>
#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>

namespace casa {

//////////////////////////////////
// PLOTMSDISPLAYTAB DEFINITIONS //
//////////////////////////////////

PlotMSIterateTab::PlotMSIterateTab(PlotMSPlotTab* tab, PlotMSPlotter* parent) 
	: PlotMSPlotSubtab(tab, parent) 
{
    setupUi(this);
    
    // Fill list of available iteration axis choices
    // For now, is same as colorize list
    iterationAxisChooser->addItem(PMS::axis(PMS::SCAN).c_str());
    iterationAxisChooser->addItem(PMS::axis(PMS::FIELD).c_str());
    iterationAxisChooser->addItem(PMS::axis(PMS::SPW).c_str());
    iterationAxisChooser->addItem(PMS::axis(PMS::ANTENNA1).c_str());
    iterationAxisChooser->addItem(PMS::axis(PMS::ANTENNA2).c_str());
    iterationAxisChooser->addItem(PMS::axis(PMS::BASELINE).c_str());
    iterationAxisChooser->addItem(PMS::axis(PMS::CHANNEL).c_str());
    iterationAxisChooser->addItem(PMS::axis(PMS::CORR).c_str());

    // Set up label defaults.
///    itsLabelDefaults_.insert(enableIterationChk,  enableIterationChk->text());
	itsLabelDefaults_.insert(iterationAxisChooserLabel,  iterationAxisChooserLabel->text());
	itsLabelDefaults_.insert(rowsLabel,           rowsLabel->text());
	itsLabelDefaults_.insert(columnsLabel,        columnsLabel->text());
///	itsLabelDefaults_.insert(VertAxesGroupBox,    VertAxesGroupBox->title());
///	itsLabelDefaults_.insert(HorizAxesGroupBox,   HorizAxesGroupBox->title());

    // Connect widgets.
    connect(enableIterationChk,  SIGNAL(toggled(bool)),  SIGNAL(changed()) );
    connect(iterationAxisChooser, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()) );
    connect(nColsSpinBox, SIGNAL(valueChanged(int)),  SIGNAL(changed()) );
    connect(nRowsSpinBox, SIGNAL(valueChanged(int)),  SIGNAL(changed()) );
    
    
    // Radio buttons for shared scale, repeated scale, etc
    /* REMOVED - .getValue and .setValue interact directly with controls.  No need to handle events.
    connect(VertAxisRadio_common,  SIGNAL(toggled(bool)), this, SLOT(changedAxisSharingMode(bool)));
    connect(VertAxisRadio_repeat,  SIGNAL(toggled(bool)), this, SLOT(changedAxisSharingMode(bool)));
    connect(VertAxisRadio_indiv,  SIGNAL(toggled(bool)), this, SLOT(changedAxisSharingMode(bool)));
    connect(VertAxisRadio_none,  SIGNAL(toggled(bool)), this, SLOT(changedAxisSharingMode(bool)));
    connect(HorizAxisRadio_common,  SIGNAL(toggled(bool)), this, SLOT(changedAxisSharingMode(bool)));
    connect(HorizAxisRadio_repeat,  SIGNAL(toggled(bool)), this, SLOT(changedAxisSharingMode(bool)));
    connect(HorizAxisRadio_indiv,  SIGNAL(toggled(bool)), this, SLOT(changedAxisSharingMode(bool)));
    connect(HorizAxisRadio_none,  SIGNAL(toggled(bool)), this, SLOT(changedAxisSharingMode(bool)));
	*/
}




PlotMSIterateTab::~PlotMSIterateTab() { }



void PlotMSIterateTab::getValue(PlotMSPlotParameters& params) const   {
    PMS_PP_Iteration* d = params.typedGroup<PMS_PP_Iteration>();
    if(d == NULL) {
        params.setGroup<PMS_PP_Iteration>();
        d = params.typedGroup<PMS_PP_Iteration>();
    }
        
	d->setEnableIteration( enableIterationChk->isChecked() );
	
    d->setIterationAxis(
           PMS::axis(iterationAxisChooser->currentText().toStdString()) );
           
           
	if      (VertAxisRadio_common->isChecked())  
		d->setYAxisSharingMode( PMS_PP_Iteration::COMMON_AXIS );
	else if (VertAxisRadio_repeat->isChecked())  
		d->setYAxisSharingMode( PMS_PP_Iteration::REPEAT_AXES );
	else if (VertAxisRadio_indiv->isChecked())  
		d->setYAxisSharingMode( PMS_PP_Iteration::INDIV_AXES );
	else if (VertAxisRadio_none->isChecked())  
		d->setYAxisSharingMode( PMS_PP_Iteration::REPEAT_AXES );
		
	if      (HorizAxisRadio_common->isChecked())  
		d->setXAxisSharingMode( PMS_PP_Iteration::COMMON_AXIS );
	else if (HorizAxisRadio_repeat->isChecked())  
		d->setXAxisSharingMode( PMS_PP_Iteration::REPEAT_AXES );
	else if (HorizAxisRadio_indiv->isChecked())  
		d->setXAxisSharingMode( PMS_PP_Iteration::INDIV_AXES );
	else if (HorizAxisRadio_none->isChecked())  
		d->setXAxisSharingMode( PMS_PP_Iteration::REPEAT_AXES );
	
	d->setNumColumns( nColsSpinBox->value() );
	d->setNumRows( nRowsSpinBox->value() );
}



void PlotMSIterateTab::setValue(const PlotMSPlotParameters& params) {

    const PMS_PP_Iteration* d = params.typedGroup<PMS_PP_Iteration>();
    
    
    enableIterationChk->setChecked( d->enableIteration() );

    // wrongway:  iterationAxisChooser->setCurrentIndex( (int)d->iterationAxis() );
    PlotMSTab::setChooser(iterationAxisChooser, PMS::axis(d->iterationAxis()));
	
	switch (d->xAxisSharingMode())   {
	case PMS_PP_Iteration::COMMON_AXIS:  HorizAxisRadio_common->setChecked(true); break;
	case PMS_PP_Iteration::REPEAT_AXES:  HorizAxisRadio_repeat->setChecked(true); break;
	case PMS_PP_Iteration::INDIV_AXES:   HorizAxisRadio_indiv->setChecked(true); break;
	case PMS_PP_Iteration::NO_AXES:      HorizAxisRadio_none->setChecked(true); break;
	}

	switch (d->yAxisSharingMode())   {
	case PMS_PP_Iteration::COMMON_AXIS:  VertAxisRadio_common->setChecked(true); break;
	case PMS_PP_Iteration::REPEAT_AXES:  VertAxisRadio_repeat->setChecked(true); break;
	case PMS_PP_Iteration::INDIV_AXES:   VertAxisRadio_indiv->setChecked(true); break;
	case PMS_PP_Iteration::NO_AXES:      VertAxisRadio_none->setChecked(true); break;
	}
	
	nRowsSpinBox->setValue( d->numRows() );
	nColsSpinBox->setValue( d->numColumns() );
}



void PlotMSIterateTab::update(const PlotMSPlot& plot) {    

	const PlotMSPlotParameters &params = plot.parameters();
	PlotMSPlotParameters  newParams(params);
	getValue(newParams);
	
	const PMS_PP_Iteration *d = params.typedGroup<PMS_PP_Iteration>();
	const PMS_PP_Iteration *d2 = newParams.typedGroup<PMS_PP_Iteration>();
	
	if (d==0 || d2==0)  return;

	highlightWidgetText(iterationAxisChooserLabel, d->iterationAxis() != d2->iterationAxis() );
	highlightWidgetText(columnsLabel,  d->numColumns() != d2->numColumns() );
	highlightWidgetText(rowsLabel,     d->numRows() != d2->numRows() );

#if (0)  // turn off for now - text shows explicitly the html, not making text red!	
	highlightWidgetText(HorizAxisSharingGroup, d->xAxisSharingMode() != d2->xAxisSharingMode());
	highlightWidgetText(VertAxisSharingGroup, d->yAxisSharingMode() != d2->yAxisSharingMode());	
	highlightWidgetText(enableIterationChk,  d->enableIteration()  != d2->enableIteration());
#endif
}

} //namespace

