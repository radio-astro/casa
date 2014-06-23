//# Copyright (C) 2011
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
//# $Id$

#include "HistogramTab.qo.h"
#include <images/Images/ImageInterface.h>
#include <images/Regions/ImageRegion.h>
#include <display/region/HistogramGraph.qo.h>
#include <QDebug>

namespace casa {
	HistogramTab::HistogramTab(QWidget *parent)
		: QWidget(parent) {
		ui.setupUi(this);
		initialStackIndex = -1;
	}

	void HistogramTab::setImageRegion( const std::string& imageName,
	                                   ImageRegion* region, int regionId) {
		QString graphName( imageName.c_str());
		if ( graphs.contains(graphName)) {
			HistogramGraph* graph = graphs[graphName];
			graph->setImageRegion( region, regionId );
		} else {
			qDebug()<<"There is no tab for histogram image="<<graphName;
		}
	}

	void HistogramTab::addImage( std::tr1::shared_ptr<ImageInterface<float> > image ) {
		if ( image != NULL ) {
			QString graphName = image->name(true).c_str();

			if ( !graphs.contains( graphName )) {
				HistogramGraph* histogramGraph = new HistogramGraph( this );
				histogramGraph->setImage( image );
				connect( histogramGraph, SIGNAL(showGraph(int)), this, SLOT(showNextGraph(int)));
				graphs.insert( graphName, histogramGraph );
				int addIndex = ui.stackedWidget->addWidget( histogramGraph );
				if ( initialStackIndex < 0 ) {
					initialStackIndex = addIndex;
					ui.stackedWidget->setCurrentIndex( addIndex );
				}
				histogramGraph->setIndex( addIndex );
				resetNextEnabled();
			}
		}
	}

	void HistogramTab::resetNextEnabled() {
		bool nextEnabled = false;
		int graphCount = graphs.size();
		if ( graphCount > 1 ) {
			nextEnabled = true;
		}
		QList<QString> keys = graphs.keys();
		for ( QList<QString>::iterator iter = keys.begin(); iter != keys.end(); iter++ ) {
			graphs[*iter]->setNextEnabled( nextEnabled );
		}
	}

	void HistogramTab::showNextGraph( int nextIndex ) {
		int graphCount = graphs.size();
		if ( graphCount > 0 ){
			int actualNextIndex = ( nextIndex - initialStackIndex ) % graphCount;
			ui.stackedWidget->setCurrentIndex( actualNextIndex + initialStackIndex );
		}
	}

	void HistogramTab::showGraph( int index ){
		if ( index < 0 ){
			index = 0;
		}
		int actualIndex = initialStackIndex + index;
		if ( 0<=actualIndex && actualIndex < ui.stackedWidget->count() ){
			ui.stackedWidget->setCurrentIndex( actualIndex );
		}
	}


	void HistogramTab::clear() {
		QList<QString> keys = graphs.keys();
		for( QList<QString>::iterator iter = keys.begin(); iter != keys.end(); iter++) {
			HistogramGraph* graph = graphs.take( *iter );
			ui.stackedWidget->removeWidget( graph );
			delete graph;
		}
		initialStackIndex = -1;
	}

	HistogramTab::~HistogramTab() {

	}
}
