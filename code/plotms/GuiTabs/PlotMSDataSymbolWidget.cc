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
#include "PlotMSDataSymbolWidget.qo.h"
#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <casaqt/QtUtilities/QtIndexChooser.qo.h>
#include <casaqt/QtUtilities/QtPlotWidget.qo.h>
#include <casaqt/QtUtilities/QtUtilities.h>
#include <plotms/Plots/PlotMSPlotParameterGroups.h>


PlotMSDataSymbolWidget::PlotMSDataSymbolWidget(PlotMSPlotter *parent)
    : QWidget(parent){
	ui.setupUi(this);
	PlotFactoryPtr factory = parent->getPlotFactory();
	PlotSymbolPtr unflaggedSymbol = PMS::DEFAULT_UNFLAGGED_SYMBOL(factory );
	String defaultUnflaggedColor = unflaggedSymbol->getColor();
	unflaggedSymbol->setColor( "#0000FF" );
	itsSymbolWidget_ = new PlotSymbolWidget(factory, unflaggedSymbol, false, false, false,false);

	PlotSymbolPtr flaggedSymbol = PMS::DEFAULT_FLAGGED_SYMBOL( factory );
	String defaultFlaggedColor = flaggedSymbol->getColor();
	flaggedSymbol->setColor( "#FF0000");
	itsMaskedSymbolWidget_ = new PlotSymbolWidget(factory, flaggedSymbol, false, false, false, false);

	QtUtilities::putInFrame(ui.unflaggedFrame, itsSymbolWidget_);
	QtUtilities::putInFrame(ui.flaggedFrame, itsMaskedSymbolWidget_);

	 map<PlotSymbol::Symbol, int> minSymbolSizes = PMS::SYMBOL_MINIMUM_SIZES();
	 itsSymbolWidget_->setMinimumSizes(minSymbolSizes);
	 itsMaskedSymbolWidget_->setMinimumSizes(minSymbolSizes);

	 // Setup colorize axis choices.
	 ui.colorizeChooser->addItem(PMS::axis(PMS::SCAN).c_str());
	 ui.colorizeChooser->addItem(PMS::axis(PMS::FIELD).c_str());
	 ui.colorizeChooser->addItem(PMS::axis(PMS::SPW).c_str());
	 ui.colorizeChooser->addItem(PMS::axis(PMS::ANTENNA1).c_str());
	 ui.colorizeChooser->addItem(PMS::axis(PMS::ANTENNA2).c_str());
	 ui.colorizeChooser->addItem(PMS::axis(PMS::BASELINE).c_str());
	 ui.colorizeChooser->addItem(PMS::axis(PMS::CHANNEL).c_str());
	 ui.colorizeChooser->addItem(PMS::axis(PMS::CORR).c_str());
	 ui.colorizeChooser->addItem(PMS::axis(PMS::TIME).c_str());
	 ui.colorizeChooser->addItem(PMS::axis(PMS::OBSERVATION).c_str());
	 ui.colorizeChooser->addItem(PMS::axis(PMS::INTENT).c_str());

	 connect(itsSymbolWidget_, SIGNAL(changed()), SIGNAL(symbolChanged()));
	 connect(itsMaskedSymbolWidget_, SIGNAL(changed()), SIGNAL(symbolChanged()));
	 connect(ui.colorize, SIGNAL(toggled(bool)), this, SLOT(symbolColorizeChanged()));
	 connect(ui.colorizeChooser, SIGNAL(currentIndexChanged(int)),
	            SIGNAL(symbolChanged()));
}

void PlotMSDataSymbolWidget::symbolColorizeChanged(){
	ui.colorizeChooser->setEnabled( ui.colorize->isChecked());
	emit symbolChanged();
}

void PlotMSDataSymbolWidget::getValue(PMS_PP_Display* d, int index ){
	if ( d != NULL ){
		PlotSymbolPtr symbol = itsSymbolWidget_->getSymbol();
		d->setUnflaggedSymbol(symbol, index);
		d->setFlaggedSymbol(itsMaskedSymbolWidget_->getSymbol(), index);
		d->setColorize(ui.colorize->isChecked(),
	            PMS::axis(ui.colorizeChooser->currentText().toStdString()), index);
	}
}



void PlotMSDataSymbolWidget::setValue( const PMS_PP_Display* d, int index){
	if ( d != NULL ){
		blockSignals( true );
		PlotSymbolPtr unflagged = d->unflaggedSymbol( index );
		itsSymbolWidget_->setSymbol(unflagged);
		itsMaskedSymbolWidget_->setSymbol(d->flaggedSymbol(index));
		ui.colorize->setChecked( d->colorizeFlag(index));
		String colorizeAxis = PMS::axis(d->colorizeAxis(index));
		QString colorizeAxisStr( colorizeAxis.c_str());
		int choiceCount = ui.colorizeChooser->count();
		for ( int i = 0; i < choiceCount; i++ ){
			if ( ui.colorizeChooser->itemText( i ) == colorizeAxisStr ){
				ui.colorizeChooser->setCurrentIndex( i );
				break;
			}
		}
		blockSignals( false );
	}
}

void PlotMSDataSymbolWidget::setLabelDefaults( QMap<QLabel*,QString>& map ){
	map.insert(ui.unflaggedLabel, ui.unflaggedLabel->text());
	map.insert(ui.flaggedLabel, ui.flaggedLabel->text());
	map.insert(ui.colorizeLabel, ui.colorizeLabel->text());
}

void PlotMSDataSymbolWidget::update( const PMS_PP_Display* d, int index ){
	bool unflaggedChanged = false;
	PlotSymbolPtr widgetSymbol = itsSymbolWidget_->getSymbol();
	PlotSymbolPtr paramSymbol = d->unflaggedSymbol(index);
	if ( *widgetSymbol != *paramSymbol ){
		unflaggedChanged = true;
	}

	bool flaggedChanged = false;
	if ( *(d->flaggedSymbol(index)) != *(itsMaskedSymbolWidget_->getSymbol())){
		flaggedChanged = true;
	}
	emit highlightWidgetText(ui.unflaggedLabel, unflaggedChanged );
	emit highlightWidgetText(ui.flaggedLabel, flaggedChanged );

	bool colorizedChanged = false;
	bool colorizingParam = d->colorizeFlag( index );
	if ( colorizingParam != ui.colorize->isChecked() ){
		colorizedChanged = true;
	}
	else if ( colorizingParam ){
		QString colorAxisStr = ui.colorizeChooser->currentText();
		String paramColorAxisStr = PMS::axis(d->colorizeAxis(index));
		if ( colorAxisStr.toStdString() != paramColorAxisStr.c_str()){
			colorizedChanged = true;
		}
	}
	emit highlightWidgetText(ui.colorizeLabel, colorizedChanged );

}

PlotMSDataSymbolWidget::~PlotMSDataSymbolWidget()
{

}
