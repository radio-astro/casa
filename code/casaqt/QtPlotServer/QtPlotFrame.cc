//# QtPlotFrame.cc: derived class from QwtPlot to give us access to protected data & virtual functions
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

#include <casaqt/QtPlotServer/QtPlotFrame.qo.h>
#include <qwt_legend.h>
#include <qwt_legend_item.h>

namespace casa {


    QtPlotFrame::symbol_map_type QtPlotFrame::symbol_map;

    // QSize QtPlotFrame::minimumSizeHint( ) const { return QSize(500,300); }
    QSize QtPlotFrame::minimumSizeHint( ) const { return QwtPlot::minimumSizeHint( ); }


    void QtPlotFrame::initialize_symbol_map( ) {
	#define INSERT_SYMBOL(STRING,VALUE) \
	symbol_map.insert( symbol_map_type::value_type(STRING, QwtSymbol(QwtSymbol::VALUE, QBrush( ), QPen( ), QSize(default_size))) )

	QSize default_size(12,12);

	if ( symbol_map.size( ) <= 0 ) {
	    INSERT_SYMBOL("ellipse",Ellipse);
	    INSERT_SYMBOL("rect",Rect);
	    INSERT_SYMBOL("diamond",Diamond);
	    INSERT_SYMBOL("triangle",Triangle);
	    INSERT_SYMBOL("dtriangle",DTriangle);
	    INSERT_SYMBOL("utriangle",UTriangle);
	    INSERT_SYMBOL("ltriangle",LTriangle);
	    INSERT_SYMBOL("ltriangle",RTriangle);
	    INSERT_SYMBOL("cross",Cross);
	    INSERT_SYMBOL("xcross",XCross);
	    INSERT_SYMBOL("hline",HLine);
	    INSERT_SYMBOL("vline",VLine);
	    INSERT_SYMBOL("star1",Star1);
	    INSERT_SYMBOL("star2",Star2);
	    INSERT_SYMBOL("hexagon",Hexagon);
	}
    }
      
    QtPlotFrame::QtPlotFrame( const QwtText &title, const QSize &s, QwtPlot::LegendPosition pos, QWidget *parent ) : QwtPlot(title,parent), size_(s) {
	legend = new QwtLegend( );
// 	legend->setDisplayPolicy( QwtLegend::NoIdentifier, QwtLegendItem::ShowLine | QwtLegendItem::ShowSymbol | QwtLegendItem::ShowText );
	insertLegend(legend,pos);

	if ( symbol_map.size( ) <= 0 )
	    initialize_symbol_map( );
    }


  QSize QtPlotFrame::sizeHint( ) const {
	if ( ! size_.isValid() || size_.isNull( ) ) {
	    return QwtPlot::sizeHint( );
	} else {
	    return size_;
	}
  }

  QStringList QtPlotFrame::colors( ) {
	// at some point, we may have our own name to QColor map...
	return QColor::colorNames( );
    }


    QStringList QtPlotFrame::symbols( ) {
	QStringList result;

	if ( symbol_map.size( ) <= 0 )
	    initialize_symbol_map( );

	for ( symbol_map_type::const_iterator iter=symbol_map.begin( );
	      iter != symbol_map.end( ); ++iter ) {
	    result.append(iter->first);
	}

	return result;
    }

    QwtSymbol QtPlotFrame::symbol(const QString &s) {
	symbol_map_type::const_iterator iter = symbol_map.find( s );
	if ( iter == symbol_map.end( ) ) {
	    return QwtSymbol( );
	}
	return iter->second;
    }

}
