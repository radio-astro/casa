//# QtPlotFrame.qo.h: derived class from QwtPlot to give us access to protected data & virtual functions
//# with surrounding Gui functionality
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
//# $Id: QtDisplayPanelGui.qo.h,v 1.7 2006/10/10 21:42:05 dking Exp $

#ifndef CASAQT_QTPLOTFRAME_H
#define CASAQT_QTPLOTFRAME_H

#include <map>
#include <string>
#include <qwt_symbol.h>
#include <qwt_plot.h>

namespace casa {

    class QtPlotFrame : public QwtPlot {
    Q_OBJECT
	public:
	    QtPlotFrame( QWidget *parent=NULL ) : QwtPlot(parent), legend(0) { }
	    QtPlotFrame( const QwtText &title, QwtPlot::LegendPosition pos=QwtPlot::BottomLegend, QWidget *parent=NULL );
	    QSize minimumSizeHint( ) const;
	    void addLegend( QwtPlot::LegendPosition pos );
	    void removeLegend( );

	    static QStringList colors( );
	    static QStringList symbols( );
	    QwtSymbol symbol(const QString &s);

	private:
	    typedef std::map<QString,QwtSymbol> symbol_map_type;
	    static symbol_map_type symbol_map;
	    static void initialize_symbol_map( );
	    QwtLegend *legend;
    };
}
#endif
