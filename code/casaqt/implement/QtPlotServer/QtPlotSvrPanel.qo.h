//# QtPlotSvrPanel.qo.h: Qt implementation of main 2D plot server display window.
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

#ifndef CASAQT_QTPLOTSRVPANEL_H
#define CASAQT_QTPLOTSRVPANEL_H

#include <QMainWindow>
#include <QStringList>
#include <qwt_plot_curve.h>
#include <casaqt/QtUtilities/QtPanelBase.qo.h>

class QwtPlotCurve;
class QwtPlotSpectrogram;
class QVBoxLayout;
class QSlider;
class QwtPlotZoomer;

namespace casa {

    class QtPlotFrame;

    class QtPlotSvrPanel : public QtPanelBase {
    Q_OBJECT
	public:

	    QtPlotSvrPanel( QWidget *parent=0 );
	    QtPlotSvrPanel( const QString &title,  const QString &xlabel="", const QString &ylabel="",
			    const QString &window_title="", const QString &legend="bottom", QWidget *parent=0 );

	    static QStringList colors( );
	    static QStringList symbols( );

	    QwtPlotCurve *line( const QList<double> &x, const QList<double> &y, const QString &color="black", const QString &label="" );
	    QwtPlotCurve *scatter( const QList<double> &x, const QList<double> &y, const QString &color="black", const QString &label="",
				   const QString &symbol="", int symbol_size=-1, int dot_size=-1 );
	    QwtPlotSpectrogram *raster( const QList<double> &matrix, int sizex, int sizey );

	    void replot( );

	protected slots:
	    void zoom( int x=-1 );
	    void zoomed( const QwtDoubleRect & );

	protected: 

	    // scripted (via dbus) panels should override the closeEvent( ) and hide the gui
	    // instead of deleting it when it was created via a dbus script...
	    void closeEvent(QCloseEvent *event);

	private:
	    QWidget *container;
	    QVBoxLayout *layout;
	    QSlider *slider;
	    QtPlotFrame *plot;
	    QwtPlotZoomer *zoomer;
	    double current_zoom_level;
	    int slider_last_value;
    };
}

#endif
