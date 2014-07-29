//# Copyright (C) 2005
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

#ifndef LEGENDITEMRECT_H_
#define LEGENDITEMRECT_H_
#include <qwt_legend_item.h>
#include <QPainter>
#include <QRect>
namespace casa {

class ColorProvider;

/**
 * Custom QwtLegendItem that draws a rectangular
 * legend symbol in a color matching the curve.
 */

class LegendItemRect : public QwtLegendItem {
public:
	LegendItemRect( const ColorProvider* colorProvider, QWidget* parent = 0  );
	virtual ~LegendItemRect();
	void setRectColor( QColor rectColor );
	virtual void drawIdentifier( QPainter* painter, const QRect& rect ) const;
private:
	LegendItemRect( const LegendItemRect& other );
	LegendItemRect operator=( const LegendItemRect& other );
	const int MARGIN;
	const ColorProvider* colorSource;
};

} /* namespace casa */
#endif /* LEGENDITEMRECT_H_ */
