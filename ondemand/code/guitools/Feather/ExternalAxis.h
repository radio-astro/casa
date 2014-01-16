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

#ifndef ExternalAxis_H_
#define ExternalAxis_H_

#include <qwt_scale_draw.h>

class QPainter;

namespace casa {

/**
 * Overrides methods of QwtScaleDraw so that the axis can be enabled
 * (supported) but the one drawn by the plot will take up no space and
 * be invisible so that we can draw our own.
 */
class ExternalAxis : public QwtScaleDraw {
public:
	ExternalAxis( );
	virtual void draw(QPainter* painter, const QPalette& palette) const;
	virtual int extent( const QPen& pen, const QFont& font ) const;
	virtual ~ExternalAxis();

};

} /* namespace casa */
#endif /* ExternalAxis_H_ */
