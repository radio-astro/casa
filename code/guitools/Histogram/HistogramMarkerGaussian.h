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
#ifndef HISTOGRAM_MARKER_GAUSSIAN_H_
#define HISTOGRAM_MARKER_GAUSSIAN_H_

#include <qwt_plot_marker.h>
#include <qwt_scale_map.h>

class QPainter;

namespace casa {

/**
 * Marks initial (center,peak) and FWHM Gaussian estimates
 * on the histogram.
 */

class HistogramMarkerGaussian : public QwtPlotMarker {

public:
	HistogramMarkerGaussian();
	void setColor( QColor markerColor );
	void setCenterPeak( int xVal, int yVal );
	void setFWHM( int fwhm, int fwhmHeight );

	virtual void draw(QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRect&) const;

	virtual ~HistogramMarkerGaussian();

private:
	int getFWHMHeight() const;

	int center;
	int peak;
	int fwhm;
	int fwhmHeight;

	bool centerPeakSpecified;
	bool fwhmSpecified;

	QColor markerColor;
};

} /* namespace casa */
#endif /* PROFILEFITMARKER_H_ */
