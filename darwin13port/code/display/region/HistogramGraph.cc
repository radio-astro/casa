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
#include "HistogramGraph.qo.h"
#include <guitools/Histogram/BinPlotWidget.qo.h>
#include <images/Images/ImageInterface.h>
#include <images/Regions/ImageRegion.h>

namespace casa {


	HistogramGraph::HistogramGraph(QWidget *parent )
		: QWidget(parent) {
		ui.setupUi(this);
		initPlot();
		connect( ui.nextButton, SIGNAL(clicked()), this, SLOT(nextGraph()));
	}

	void HistogramGraph::setIndex( int stackIndex ) {
		index = stackIndex;
	}

	void HistogramGraph::nextGraph() {
		emit showGraph( index+1);
	}

	void HistogramGraph::initPlot() {
		histogram = new BinPlotWidget( false, false, false, NULL );
		histogram->setDisplayPlotTitle( true );
		histogram->setDisplayAxisTitles( true );

		QHBoxLayout* boxLayout = new QHBoxLayout();
		boxLayout->addWidget( histogram );
		ui.histogramHolder->setLayout( boxLayout );
	}

	void HistogramGraph::setNextEnabled( bool enabled ) {
		ui.nextButton->setEnabled( enabled );
	}

	void HistogramGraph::setImageRegion( ImageRegion* region, int id ) {
		histogram->setImageRegion( region, id );
	}

	void HistogramGraph::setImage(std::tr1::shared_ptr<ImageInterface<float> > image ) {
		histogram->setImage( image );
	}

	HistogramGraph::~HistogramGraph() {

	}
}
