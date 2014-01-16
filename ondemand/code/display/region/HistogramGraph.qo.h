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

#ifndef HISTOGRAMGRAPH_QO_H
#define HISTOGRAMGRAPH_QO_H

#include <QtGui/QWidget>
#include <display/region/HistogramGraph.ui.h>

#include <tr1/memory.hpp>

namespace casa {

	template <class T> class ImageInterface;
	class BinPlotWidget;
	class ImageRegion;

	/**
	 * Displays a histogram specific to a region and an image; contains
	 * a "Next" button that toggles to a histogram displaying the same
	 * region, but a different image.
	 */


	class HistogramGraph : public QWidget {
		Q_OBJECT

	public:
		HistogramGraph(QWidget *parent = 0);
		~HistogramGraph();
		void initPlot();
		void setIndex( int stackIndex );
		void setNextEnabled( bool enabled );
		void setImage( std::tr1::shared_ptr<ImageInterface<float> > image );
		void setImageRegion( ImageRegion* region, int id );

	signals:
		void showGraph( int nextIndex );

	private slots:
		void nextGraph();

	private:
		HistogramGraph( const HistogramGraph& other );
		HistogramGraph operator=( const HistogramGraph& other );
		int index;
		Ui::HistogramGraphClass ui;
		BinPlotWidget* histogram;
	};
}

#endif // HISTOGRAMGRAPH_QO_H
