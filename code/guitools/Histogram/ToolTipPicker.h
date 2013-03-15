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
#ifndef TOOLTIPPICKER_H_
#define TOOLTIPPICKER_H_

#include <qwt_plot_picker.h>

namespace casa {

class ToolTipPicker :QwtPlotPicker {

public:
	ToolTipPicker( int xAxis, int yAxis, int selectionFlags, RubberBand rubberBand,
			DisplayMode trackorMode, QwtPlotCanvas* canvas );
	void setData( const std::vector<float>& xVal, const std::vector<float>& yVal );
	void setLogScaleY( bool logScale );
	virtual QwtText trackerText( const QwtDoublePoint & pos ) const;
	virtual ~ToolTipPicker();

private:
	int findClosestPoint( float xValue, float yValue ) const;
	std::pair<float,float> getRangeY() const;
	std::vector<float> xVector;
	std::vector<float> yVector;
	bool logScaleY;
};

} /* namespace casa */
#endif /* TOOLTIPPICKER_H_ */
