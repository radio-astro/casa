//# QPData.cc: Connector classes between generic plotter and Qwt data classes.
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
//# $Id: $
#ifdef AIPS_HAS_QWT

#include <casaqt/QwtPlotter/QPData.h>

namespace casa {

// TODO PlotRasterDataImpl: possible indexing bug

/////////////////////////////
// QPPOINTDATA DEFINITIONS //
/////////////////////////////

QPPointData::QPPointData(PlotPointDataPtr data) : m_data(data) { }

QPPointData::~QPPointData() { }


PlotPointDataPtr QPPointData::data() { return m_data; }
const PlotPointDataPtr QPPointData::data() const { return m_data; }

QwtData* QPPointData::copy() const { return new QPPointData(m_data); }

size_t QPPointData::size() const {
    if(m_data.null()) return 0;
    else              return m_data->size();
}

double QPPointData::x(size_t i) const { return m_data->xAt(i); }

double QPPointData::y(size_t i) const { return m_data->yAt(i); }

QwtDoubleRect QPPointData::boundingRect() const {
    if(m_data.null()) return QRectF();
    
    double xMin, xMax, yMin, yMax;
    if(const_cast<PlotPointDataPtr&>(m_data)->minsMaxes(xMin, xMax, yMin,yMax))
        // have to switch the y min and max for some reason..
        return QRectF(QPointF(xMin, yMin), QPointF(xMax, yMax));
    
    else return QRectF(); // error
}


//////////////////////////////
// QPRASTERDATA DEFINITIONS //
////////////////////....//////

QPRasterData::QPRasterData(PlotRasterDataPtr data) : m_data(data) { }

QPRasterData::~QPRasterData() { }


PlotRasterDataPtr QPRasterData::data() { return m_data; }
const PlotRasterDataPtr QPRasterData::data() const { return m_data; }

bool QPRasterData::isValid() const { return !m_data.null(); }


QwtRasterData* QPRasterData::copy() const { return new QPRasterData(m_data); }

double QPRasterData::value(double x, double y) const {
    return m_data->valueAt(x, y); }

QwtDoubleInterval QPRasterData::range() const {
    if(m_data.null()) return QwtDoubleInterval();
    pair<double, double> r = m_data->valueRange();
    return QwtDoubleInterval(r.first, r.second);
}

QwtDoubleRect QPRasterData::boundingRect() const {
    if(m_data.null()) return QwtDoubleRect();
    
    pair<double, double> xrange = m_data->xRange(),
                         yrange = m_data->yRange();
    
    // have to switch the min and max y values for some reason
    return QwtDoubleRect(xrange.first, yrange.first,
                         xrange.second - xrange.first,
                         yrange.second - yrange.first);
}

}

#endif
