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
#include <QDebug>
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
    iterationAxisChooser->addItem(PMS::axis(PMS::NONE).c_str());
    iterationAxisChooser->addItem(PMS::axis(PMS::SCAN).c_str());
    iterationAxisChooser->addItem(PMS::axis(PMS::FIELD).c_str());
    iterationAxisChooser->addItem(PMS::axis(PMS::SPW).c_str());
    iterationAxisChooser->addItem(PMS::axis(PMS::BASELINE).c_str());
    iterationAxisChooser->addItem(PMS::axis(PMS::ANTENNA).c_str());
    iterationAxisChooser->addItem(PMS::axis(PMS::TIME).c_str());
    /* not yet:
    iterationAxisChooser->addItem(PMS::axis(PMS::ANTENNA1).c_str());
    iterationAxisChooser->addItem(PMS::axis(PMS::ANTENNA2).c_str());
    iterationAxisChooser->addItem(PMS::axis(PMS::CHANNEL).c_str());
    iterationAxisChooser->addItem(PMS::axis(PMS::CORR).c_str());
    */
    hideGridLocation( true );

    // Set up label defaults.
	itsLabelDefaults_.insert(iterationAxisChooserLabel,  iterationAxisChooserLabel->text());
	 itsLabelDefaults_.insert(rowIndexLabel, rowIndexLabel->text());
	 itsLabelDefaults_.insert(colIndexLabel, colIndexLabel->text());

    // Connect widgets.
    connect(iterationAxisChooser, SIGNAL(currentIndexChanged(int)), SIGNAL(changed()) );
    connect(globalXCheck, SIGNAL(stateChanged(int)), SLOT(globalChanged()));
    connect(globalYCheck, SIGNAL(stateChanged(int)), SLOT(globalChanged()));
    connect(sharedXCheck, SIGNAL(stateChanged(int)), SIGNAL(changed()));
    connect(sharedYCheck, SIGNAL(stateChanged(int)), SIGNAL(changed()));
    connect(gridRowSpin, SIGNAL(valueChanged(int)), SIGNAL(changed()));
    connect(gridColSpin, SIGNAL(valueChanged(int)), SIGNAL(changed()));
}

void PlotMSIterateTab::gridChanged( int rowCount, int colCount ){

	//Common x-axis
	if ( rowCount <= 1 ){
		sharedXCheck->setEnabled( false );
		sharedXCheck->setChecked( false );
		globalXCheck->setEnabled( false );
		globalXCheck->setChecked( false );
	}
	else {
		globalXCheck->setEnabled( true );
		if ( globalXCheck->isChecked()){
			sharedXCheck->setEnabled( true );
		}
	}

	//Common y-axis
	if ( colCount <= 1 ){
		sharedYCheck->setEnabled( false );
		sharedYCheck->setChecked( false );
		globalYCheck->setEnabled( false );
		globalYCheck->setChecked( false );
	}
	else {
		globalYCheck->setEnabled( true );
		if ( globalYCheck->isChecked()){
			sharedYCheck->setEnabled( true );
		}
	}
	//emit changed();
}



bool PlotMSIterateTab::setGridSize(unsigned int nRows,unsigned int nCols){
    //Decide whether we let the user choose the location of the plot
	//or not based on the grid size.
	bool hideGrid = true;
    if ( nRows > 1 || nCols > 1 ){
    	hideGrid = false;
    }
    hideGridLocation( hideGrid );

    //Decide whether the current location exceeds the number of rows/columns
    //available now.
    bool validLocation = true;
    int currentRow = gridRowSpin->value() - 1;
    int currentCol = gridColSpin->value() - 1;
    if ( currentRow >= static_cast<int>(nRows) ||
    		currentCol >= static_cast<int>(nCols) ){
    	validLocation = false;
    }

    //Reset the limits on the spins.
    gridRowSpin->setMaximum( nRows );
    gridColSpin->setMaximum( nCols );
    gridChanged( nRows, nCols );
    return validLocation;
}

void PlotMSIterateTab::globalChanged(){
	sharedYCheck->setEnabled( globalYCheck->isChecked());
	sharedXCheck->setEnabled( globalXCheck->isChecked());
	emit changed();
}

void PlotMSIterateTab::hideGridLocation( bool hide ) {
    chooserFrame->setVisible(!hide);
}


void PlotMSIterateTab::setGridIndices( int rowIndex, int colIndex ){
	//Note that the rows and columns we display to the user begin with 1,
	//whereas what we store in the code begins at 0.
	int maxRows = gridRowSpin->maximum();
	if ( 0< rowIndex && rowIndex <= maxRows ){
		gridRowSpin->setValue( rowIndex );
	}
	else {
	    qDebug() << "PlotMSIterateTab::setValue maxRows="<<maxRows<<" rowIndex="<<rowIndex;
	}
	int maxCols = gridColSpin->maximum();
	if ( 0 < colIndex && colIndex <= maxCols ){
		gridColSpin->setValue( colIndex );
	}
	else {
		 qDebug() << "PlotMSIterateTab::setValue maxCols="<<maxCols<<" colIndex="<<colIndex;
	}
}

PlotMSIterateTab::~PlotMSIterateTab() { }



void PlotMSIterateTab::getValue(PlotMSPlotParameters& params) const   {
    PMS_PP_Iteration* d = params.typedGroup<PMS_PP_Iteration>();
    if(d == NULL) {
        params.setGroup<PMS_PP_Iteration>();
        d = params.typedGroup<PMS_PP_Iteration>();
    }
        
	
    d->setIterationAxis(
           PMS::axis(iterationAxisChooser->currentText().toStdString()) );

	d->setGlobalScaleX( globalXCheck->isChecked());
	d->setGlobalScaleY( globalYCheck->isChecked());

	d->setCommonAxisX( sharedXCheck->isChecked());
	d->setCommonAxisY( sharedYCheck->isChecked());

	//Note, we are subtracting 1 because UI values start with 1, but internal
	//values are 0 based.
	int rowSpinIndex = gridRowSpin->value() - 1;
	int colSpinIndex = gridColSpin->value() - 1;
	d->setGridRow( rowSpinIndex );
	d->setGridCol( colSpinIndex );
}



void PlotMSIterateTab::setValue(const PlotMSPlotParameters& params) {

  const PMS_PP_Iteration* d = params.typedGroup<PMS_PP_Iteration>();
  
  PlotMSTab::setChooser(iterationAxisChooser, PMS::axis(d->iterationAxis()));

  sharedXCheck->setChecked( d->isCommonAxisX() );
  sharedYCheck->setChecked( d->isCommonAxisY() );
  globalXCheck->setChecked( d->isGlobalScaleX() );
  globalYCheck->setChecked( d->isGlobalScaleY() );

  int rowIndex = d->getGridRow();
  int colIndex = d->getGridCol();
  setGridIndices( rowIndex+ 1, colIndex+1 );
}



void PlotMSIterateTab::update(const PlotMSPlot& plot) {    

	const PlotMSPlotParameters &params = plot.parameters();
	PlotMSPlotParameters  newParams(params);
	getValue(newParams);
	
	const PMS_PP_Iteration *d = params.typedGroup<PMS_PP_Iteration>();
	const PMS_PP_Iteration *d2 = newParams.typedGroup<PMS_PP_Iteration>();
	
	if (d==0 || d2==0)  return;

	highlightWidgetText(iterationAxisChooserLabel, d->iterationAxis() != d2->iterationAxis() );

	bool commonXChange = d->isCommonAxisX() != d2->isCommonAxisX();
	bool commonYChange = d->isCommonAxisY() != d2->isCommonAxisY();
	bool globalXChange = d->isGlobalScaleX() != d2->isGlobalScaleX();
	bool globalYChange = d->isGlobalScaleY() != d2->isGlobalScaleY();
	highlightWidgetText(commonAxisLabel, commonXChange || commonYChange );
	highlightWidgetText(globalAxisLabel, globalXChange || globalYChange );

	bool gridRowChanged = false;
	int oldRowCount = d->getGridRow();
	int newRowCount = d2->getGridRow();
	if (oldRowCount != newRowCount ){
		 gridRowChanged = true;
	}
	highlightWidgetText( rowIndexLabel, gridRowChanged );
	bool gridColChanged = false;
	int oldColCount = d->getGridCol();
	int newColCount = d2->getGridCol();
	if (oldColCount != newColCount ){
		 gridColChanged = true;
	}
	highlightWidgetText(colIndexLabel, gridColChanged );
}

} //namespace

