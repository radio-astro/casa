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

#include <map>
#include <QMainWindow>
#include <QStringList>
#include <qwt_plot_curve.h>
#include <qwt_color_map.h>
#include <casaqt/QtUtilities/QtId.h>
#include <casaqt/QtUtilities/QtPanelBase.qo.h>
#include <casaqt/QtPlotServer/QtPlotHistogram.h>
#include <casadbus/types/ptr.h>

class QwtPlotCurve;
class QwtPlotSpectrogram;
class QVBoxLayout;
class QHBoxLayout;
class QSlider;
class QwtPlotZoomer;

namespace casa {

    class QtPlotFrame;

    class QtPlotSvrMain : public QtPanelBase {
    Q_OBJECT
	public:
	    QtPlotSvrMain( QWidget */*parent*/=0 ) : QtPanelBase( ) { }

	signals:
	    void closing( bool );

	protected: 
	    // scripted (via dbus) panels should override the closeEvent( ) and hide the gui
	    // instead of deleting it when it was created via a dbus script...
	    void closeEvent(QCloseEvent *event);
    };

    class QtPlotSvrPanel : public QObject {
    Q_OBJECT
	public:

	    void hide( );
	    void show( );
	    void closeMainPanel( );
	    bool isVisible( );
	    void releaseMainPanel( );

	    QtPlotSvrPanel( QWidget *parent=0 );
	    QtPlotSvrPanel( const QString &title,  const QString &xlabel="", const QString &ylabel="", const QString &window_title="",
			    const QList<int> &size=QList<int>( ), const QString &legend="bottom", const QString &zoom="bottom",
			    QtPlotSvrPanel *with_panel=0, bool new_row=false, QWidget *parent=0 );

	    static QStringList colors( );
	    static QStringList colormaps( );
	    static QStringList symbols( );

	    QwtPlotCurve *line( const QList<double> &x, const QList<double> &y, const QString &color="black", const QString &label="" );
	    QwtPlotCurve *scatter( const QList<double> &x, const QList<double> &y, const QString &color="black", const QString &label="",
				   const QString &symbol="", int symbol_size=-1, int dot_size=-1 );
	    QtPlotHistogram *histogram( const QList<double> &y, int bins=0, const QString &color="blue", const QString &label="" );
	    QwtPlotSpectrogram *raster( const QList<double> &matrix, int sizex, int sizey, const QString &colormap="Rainbow 2" );

	    void replot( );

	    void setxlabel( const QString &xlabel );
	    void setylabel( const QString &ylabel );
	    void settitle( const QString &title );
	    
	    std::pair<QDockWidget*,QString> loaddock( const QString &file_or_xml, const QString &loc, const QStringList &dockable );

	    virtual ~QtPlotSvrPanel( ) { }

	signals:

	    void button( QtPlotSvrPanel *, QString );
	    void check( QtPlotSvrPanel *, QString, int );
	    void radio( QtPlotSvrPanel *, QString, bool );
	    void linetext( QtPlotSvrPanel *, QString, const QString &text );
	    void slidevalue( QtPlotSvrPanel *, QString, int );

	    void closing( QtPlotSvrPanel *, bool );

	protected slots:

	    void emit_button( );
	    void emit_check( int );
	    void emit_radio( bool );
	    void emit_linetext( const QString &text );
	    void emit_slidevalue( int );

	    void emit_closing( bool );

	    void zoom( int x=-1 );
	    void zoomed( const QwtDoubleRect & );

	protected: 

	    QWidget *loaddock( QString file );
            QFont defaultfont;

	    typedef std::map<QString,memory::cptr<QwtLinearColorMap> > colormap_map;
	    static void load_colormaps( );
	    static colormap_map *colormaps_;

	private:

	    class row_desc {
	    public:

		class plot_desc {
		public:
		    plot_desc( QWidget *c, QLayout *l, QtPlotFrame *p, QSlider *s ) :
			container_(c), layout_(l), plot_(p), slider_(s) { }
		    QWidget *&container( ) { return container_; }
		    QLayout *&layout( ) { return layout_; }
		    QtPlotFrame *&plot( ) { return plot_; }
		    QSlider *&slider( ) { return slider_; }

		private:
		    QWidget *container_;
		    QLayout *layout_;
		    QtPlotFrame *plot_;
		    QSlider *slider_;
		};

		row_desc( QWidget *c, QHBoxLayout *l ) : container_(c), layout_(l), id_(QtId::get_id( )) { }
		QWidget *container( ) { return container_; }
		QHBoxLayout *layout( ) { return layout_; }
		void addplot( QWidget *container, QLayout *layout, QtPlotFrame *plot, QSlider *slider )
				{ plots.push_back( new plot_desc(container, layout, plot, slider ) ); }
		int id( ) { return id_; }
	    private:
		typedef std::list<plot_desc*> plotlist;
		plotlist plots;
		QWidget *container_;
		QHBoxLayout *layout_;
		int id_;
	    };

	    class win_desc {
	    public:
		win_desc( QtPlotSvrMain *b, QWidget *c, QVBoxLayout *l ) : base_(b), container_(c), layout_(l) { }
		QtPlotSvrMain *& base( ) { return base_; }
		QWidget *&container( ) { return container_; }
		QVBoxLayout *&layout( ) { return layout_; }
	    private:
		QtPlotSvrMain *base_;
		QWidget *container_;
		QVBoxLayout *layout_;
	    };

	    typedef std::map<QtPlotSvrPanel*,memory::cptr<row_desc> > rowmap;
	    memory::cptr<rowmap> rows_;

	    memory::cptr<win_desc> window_;

	    QSlider *slider;
	    QtPlotFrame *plot;
	    QwtPlotZoomer *zoomer;
	    double current_zoom_level;
	    int slider_last_value;
    };
}

#endif
