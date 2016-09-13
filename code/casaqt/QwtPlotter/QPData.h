//# QPData.h: Connector classes between generic plotter and Qwt data classes.
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
#ifndef QPDATA_H_
#define QPDATA_H_

#ifdef AIPS_HAS_QWT

#include <casaqt/QwtConfig.h>
#include <graphics/GenericPlotter/PlotData.h>

#if QWT_VERSION < 0x060000
#include <qwt_data.h>
#endif

#include <qwt_raster_data.h>

namespace casa {

// Connects PlotPointData and QwtData.
#if QWT_VERSION >= 0x060000
class QPPointData : public QwtSeriesData<QPointF>
#else
class QPPointData : public QwtData
#endif
    {
public:
    // Constructor which takes the data.
    QPPointData(PlotPointDataPtr data);
    // Destructor.
    ~QPPointData();
    
    // Returns the point data.
    // <group>
    PlotPointDataPtr data();
    const PlotPointDataPtr data() const;
    // </group>
    
    
    // QwtData Methods //
    
    // Implements QwtData::copy().
#if QWT_VERSION >= 0x060000
    QPPointData* copy() const;

    // Implements QwtSeriesData::sample().
    QPointF sample(size_t i) const;

#else
    QwtData* copy() const;
#endif
    
    // Implements QwtData::size().
    size_t size() const;
    
    // Implements QwtData::x().
    double x(size_t i) const;
    
    // Implements QwtData::y().
    double y(size_t i) const;
    
    // Overrides QwtData::boundingRect().
    QwtDoubleRect boundingRect() const;
    
private:
    PlotPointDataPtr m_data; // Data
};


// Connects PlotRasterData and QwtRasterData.
class QPRasterData : public QwtRasterData {
public:
    // Constructor which takes the raster data.
    QPRasterData(PlotRasterDataPtr data);
    
    // Destructor.
    ~QPRasterData();
    
    
    // Returns the raster data.
    // <group>
    PlotRasterDataPtr data();
    const PlotRasterDataPtr data() const;
    // </group>
    
    // Returns true if this data is valid, false otherwise.
    bool isValid() const;
    
    
    // QwtRasterData Methods //
    
    // Implements QwtRasterData::copy().
    QwtRasterData* copy() const;
    
    // Implements QwtRasterData::value().
    double value(double x, double y) const;
    
    // Implements QwtRasterData::range().
    QwtDoubleInterval range() const;
    
    // Overrides QwtRasterData::boundingRect().
    QwtDoubleRect boundingRect() const;
    
private:
    PlotRasterDataPtr m_data; // Data
};

} // end namespace casa

#endif

#endif /* QPDATA_H_ */
