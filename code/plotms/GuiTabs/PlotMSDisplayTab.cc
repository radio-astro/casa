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
#include <plotms/GuiTabs/PlotMSDataSymbolWidget.qo.h>
#include <plotms/Plots/PlotMSPlot.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>

namespace casa {

//////////////////////////////////
// PLOTMSDISPLAYTAB DEFINITIONS //
//////////////////////////////////

PlotMSDisplayTab::PlotMSDisplayTab(PlotMSPlotTab* tab, PlotMSPlotter* parent) :
        PlotMSPlotSubtab(tab, parent) {
    setupUi(this);

    makeDataSymbol();
    changeSymbolViewed();

    connect(yDataCombo,SIGNAL(currentIndexChanged(int)), this, SLOT(changeSymbolViewed()));
}

void PlotMSDisplayTab::updateMultipleAxisSupport(){
	int symbolCount = dataSymbols.size();
	bool multipleAxes = false;
	if ( symbolCount > 1 ){
		multipleAxes = true;
	}
	axisDataLabel->setVisible( multipleAxes );
	yDataCombo->setVisible( multipleAxes );
}


void PlotMSDisplayTab::makeDataSymbol(){
	PlotMSDataSymbolWidget* dataSymbol = new PlotMSDataSymbolWidget(itsPlotter_);
	dataSymbol->setLabelDefaults( itsLabelDefaults_ );
	connect( dataSymbol, SIGNAL(highlightWidgetText(QLabel*,bool)),
	    		this, SLOT(updateText(QLabel*, bool)));
	connect( dataSymbol, SIGNAL(symbolChanged()), this, SIGNAL(changed()));
	dataSymbols.append( dataSymbol );
	updateMultipleAxisSupport();
}

void PlotMSDisplayTab::changeSymbolViewed(){
	QLayout* yLayout = symbolFrame->layout();
	if ( yLayout == NULL ){
		yLayout = new QVBoxLayout();
	}
	else {
		//Clear the layout which should only have one thing
		//in it.
		QLayoutItem* layoutItem = yLayout->takeAt( 0 );
		if ( layoutItem != NULL ){
			QWidget* widget = layoutItem->widget();
			if ( widget != NULL ){
				widget->setParent( NULL );
			}
			delete layoutItem;
		}
	}
	int index = yDataCombo->currentIndex();
	if ( index < 0 ){
		index = 0;
	}
	yLayout->addWidget( dataSymbols[index] );
	symbolFrame->setLayout( yLayout );
	symbolFrame->update();
}

void PlotMSDisplayTab::setAxisIdentifier( int index, QString id ){
	int existingAxesCount = yDataCombo->count();
	if ( existingAxesCount == 0 ){
		axisDataLabel->setVisible( true );
		yDataCombo->setVisible( true );
		yDataCombo->addItem( id );
	}
	else if ( index >= existingAxesCount ){
		yDataCombo->addItem( id );
		makeDataSymbol();
	}
	else {
		yDataCombo->setItemText(index, id);
	}
}

void PlotMSDisplayTab::removeAxisIdentifier( int index ){
	int existingAxesCount = yDataCombo->count();
	if ( 0 <= index && index < existingAxesCount ){
		PlotMSDataSymbolWidget* dataSymbol = dataSymbols[index];
		dataSymbols.takeAt( index );
		yDataCombo->removeItem( index );
		delete dataSymbol;
		updateMultipleAxisSupport();
	}
}




void PlotMSDisplayTab::getValue(PlotMSPlotParameters& params) const {
    PMS_PP_Display* d = params.typedGroup<PMS_PP_Display>();
    if(d == NULL) {
        params.setGroup<PMS_PP_Display>();
        d = params.typedGroup<PMS_PP_Display>();
    }

    int dataCount = dataSymbols.size();
    for ( int i = 0; i < dataCount; i++ ){
    	dataSymbols[i]->getValue(d, i);
    }
}

void PlotMSDisplayTab::setValue(const PlotMSPlotParameters& params) {
    const PMS_PP_Display* d = params.typedGroup<PMS_PP_Display>();
    if(d == NULL) return; // shouldn't happen
    
    int dataCount = dataSymbols.size();
    for ( int i = 0; i < dataCount; i++ ){
    	dataSymbols[i]->setValue( /*&itsPDisplay_*/d, i );
    }
}

void PlotMSDisplayTab::updateText( QLabel* source, bool highlight ){
	highlightWidgetText( source, highlight );
}

void PlotMSDisplayTab::update(const PlotMSPlot& plot) {    
    const PlotMSPlotParameters& params = plot.parameters();
    const PMS_PP_Display* d = params.typedGroup<PMS_PP_Display>();
    if ( d == NULL ){
    	return;
    }

    int dataSymbolCount = dataSymbols.size();
    for ( int i = 0; i < dataSymbolCount; i++ ){
    	dataSymbols[i]->update( d, i );
    }
}

PlotMSDisplayTab::~PlotMSDisplayTab() {
	while( !dataSymbols.isEmpty() ){
		PlotMSDataSymbolWidget* dataSymbol = dataSymbols.takeAt(0);
		dataSymbol->setParent( NULL );
		delete dataSymbol;
	}
}

}
