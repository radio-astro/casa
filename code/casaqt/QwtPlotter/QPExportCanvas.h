//# Copyright (C) 2008
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

#ifndef QPEXPORTCANVAS_H_
#define QPEXPORTCANVAS_H_

#include <graphics/GenericPlotter/PlotCanvas.h>
#include <graphics/GenericPlotter/PlotFactory.h>
#include <QPrinter>

namespace casa {
/**
 * Interface implemented by classes that can export their draw
 * area (QPAxis & QPCanvas).
 */

class QPExportCanvas : public PlotCanvas {
public:
	QPExportCanvas(){}
	virtual ~QPExportCanvas(){}
	virtual bool print( QPrinter& printer ) = 0;
	virtual bool print(  QPainter* painter, PlotAreaFillPtr paf, double widthRatio,
			double heightRatio, int externalAxisWidth, int externalAxisHeight,
			int rowIndex, int colIndex, QRect imageRect ) = 0;
	virtual QImage  grabImageFromCanvas(const PlotExportFormat& format ) = 0;
	virtual PlotLoggerPtr logger() const = 0;
	virtual int canvasWidth() const = 0;
	virtual int canvasHeight() const = 0;
	virtual const QPalette& palette() const = 0;
	virtual QPalette::ColorRole backgroundRole() const = 0;
	virtual PlotFactory* implementationFactory() const = 0;
	virtual bool isAxis() const {
		return false;
	}
	virtual bool isVertical() const {
		return false;
	}
};

}
#endif /* QPEXPORTCANVAS_H_ */
