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

#ifndef LEGENDCURVE_H_
#define LEGENDCURVE_H_

#include <qwt_plot_curve.h>
#include <guitools/Feather/ColorProvider.h>
namespace casa {

class LegendItemRect;

/**
 * Custom QwtPlotCurve that provides the capability of a thicker (rectangular)
 * legend rather than a hard-to-see line that mirrors the thickness of the curve.
 */

class LegendCurve : public QwtPlotCurve, public ColorProvider {
public:
	LegendCurve( ColorProvider* colorProvider );
	virtual QColor getRectColor( ) const;
	QWidget* legendItem() const;
	virtual ~LegendCurve();
private:
	LegendCurve( const LegendCurve& other );
	LegendCurve operator=( const LegendCurve& other );
	const ColorProvider* colorSource;
};

} /* namespace casa */
#endif /* LEGENDCURVE_H_ */
