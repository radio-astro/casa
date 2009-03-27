//# Plotter.cc: Highest level plotting object that holds one or more canvases.
//# Copyright (C) 2009
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
//# $Id: $
#include <graphics/GenericPlotter/Plotter.h>

namespace casa {

/////////////////////////
// PLOTTER DEFINITIONS //
/////////////////////////

Plotter::Plotter() { }

Plotter::~Plotter() { }


PlotCanvasPtr Plotter::canvasAt(const PlotLayoutCoordinate& coord) {
    PlotCanvasLayoutPtr layout = canvasLayout();
    if(!layout.null()) return layout->canvasAt(coord);
    else               return PlotCanvasPtr();
}

PlotCanvasPtr Plotter::canvas() {
    PlotCanvasLayoutPtr layout = canvasLayout();
    if(!layout.null()) return layout->canvas();
    else               return PlotCanvasPtr();
}

void Plotter::setCanvas(PlotCanvasPtr canvas) {
    if(!canvas.null())
        setCanvasLayout(PlotCanvasLayoutPtr(new PlotLayoutSingle(canvas)));
}

PlotLoggerPtr Plotter::logger() {
    if(m_logger.null()) m_logger = new PlotLogger(this);
    return m_logger;
}

}
