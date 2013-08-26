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
#ifndef HISTOGRAMTAB_QO_H
#define HISTOGRAMTAB_QO_H

#include <QtGui/QWidget>
#include <display/region/HistogramTab.ui.h>

#include <tr1/memory>

namespace casa {

	template <class T> class ImageInterface;
	class ImageRegion;
	class HistogramGraph;

	/**
	 * Manages a stack widget that displays histograms for a single region
	 * but multiple images.
	 */

	class HistogramTab : public QWidget {
		Q_OBJECT

	public:
		HistogramTab(QWidget *parent = 0);
		void addImage( std::tr1::shared_ptr<ImageInterface<float> > image );
		void setImageRegion( const std::string& imageName, ImageRegion* region, int regionId);
		void clear();
		/**
		 * This method was written so that the image showing on the histogram
		 * will be the same as the one indicated on the image animator.
		 */
		void showGraph( int index );
		~HistogramTab();

	private slots:
		/**
		 * When the 'next' button is pressed on the histogram.
		 */
		void showNextGraph( int nextIndex );

	private:
		int initialStackIndex;
		void resetNextEnabled();
		QMap<QString,HistogramGraph*> graphs;
		Ui::HistogramTabClass ui;
	};
}

#endif // HISTOGRAMTAB_QO_H
