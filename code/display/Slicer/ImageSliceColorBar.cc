//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000
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

#include "ImageSliceColorBar.h"
#include <QPainter>
#include <QDebug>

namespace casa {

	ImageSliceColorBar::ImageSliceColorBar(QWidget* parent)
		:QWidget( parent ) {
		// TODO Auto-generated constructor stub

	}

	void ImageSliceColorBar::setColors( const QList<QColor>& segmentColors ) {
		colorList.clear();
		int colorCount = segmentColors.size();
		for ( int i = 0; i < colorCount; i++ ) {
			colorList.append( segmentColors[i]);
		}
	}

	void ImageSliceColorBar::paintEvent( QPaintEvent* event ) {
		int colorCount = colorList.size();
		if ( colorCount > 0 ) {
			QSize barSize = size();
			int rectWidth = barSize.width() / colorCount;
			int rectHeight = barSize.height() / 2;
			QPainter painter( this );
			for ( int i = 0; i < colorCount; i++ ) {
				int xLocation = i * rectWidth;
				QRect rect( xLocation, 0, rectWidth, rectHeight );
				QBrush brush( colorList[i]);
				painter.fillRect( rect, brush );
			}
		} else {
			QWidget::paintEvent( event );
		}
	}

	ImageSliceColorBar::~ImageSliceColorBar() {
		// TODO Auto-generated destructor stub
	}

} /* namespace casa */
