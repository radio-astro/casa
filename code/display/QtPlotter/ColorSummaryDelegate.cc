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

/**
 * Written to provide a customized view for color list items.  In particular,
 * the standard method of showing that a list item is selected is to color
 * it.  However, coloring a selected item, obscures the underlying color that
 * was displayed.  This class substitutes coloring a selected item with drawing
 * a black box around it.
 */
#include "ColorSummaryDelegate.h"
#include <QPen>
#include <QDebug>
#include <QPainter>

namespace casa {

	ColorSummaryDelegate::ColorSummaryDelegate( QObject* parent ) :
		QStyledItemDelegate( parent ) {
	}

	void ColorSummaryDelegate::paint( QPainter * painter, const QStyleOptionViewItem &option,
	                                  const QModelIndex & index ) const {
		// Call the original paint method with the selection state cleared
		// to prevent painting the original selection background
		QStyleOptionViewItemV4 optionV4 = *qstyleoption_cast<const QStyleOptionViewItemV4 *>(&option);
		int selected = optionV4.state & QStyle::State_Selected;
		optionV4.state &= ~QStyle::State_Selected;

		QStyledItemDelegate::paint(painter, optionV4, index);

		if ( selected != 0 ) {
			QRect viewport = painter->viewport();
			QPen pen = painter->pen();
			int oldWidth = pen.width();
			const int PEN_WIDTH = 4;
			pen.setWidth( PEN_WIDTH );
			QColor oldColor = pen.color();
			pen.setColor( Qt::black );
			painter->setPen( pen );

			QRect drawRect =option.rect;
			int startY = drawRect.y();
			int drawHeight = drawRect.height();

			painter->drawRect(PEN_WIDTH/2,startY,viewport.width(), drawHeight-PEN_WIDTH/2);
			pen.setWidth( oldWidth );
			pen.setColor( oldColor );
			painter->setPen( pen );
		}
	}

	ColorSummaryDelegate::~ColorSummaryDelegate() {
		// TODO Auto-generated destructor stub
	}

} /* namespace casa */
