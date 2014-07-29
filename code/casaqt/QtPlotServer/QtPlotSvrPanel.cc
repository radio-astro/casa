//# QtPlotSvrPanel.cc: Qt implementation of main 2D plot server display window.
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

#include <casa/System/Aipsrc.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <casa/Utilities/Regex.h> 
#include <tables/Tables/ScaColDesc.h>
#include <tables/Tables/ArrColDesc.h>
#include <tables/Tables/SetupNewTab.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <casaqt/QtPlotServer/QtPlotSvrPanel.qo.h>
#include <casaqt/QtPlotServer/QtPlotFrame.qo.h>
#include <casaqt/QtPlotServer/QtRasterData.h>
#include <qwt_plot_spectrogram.h>
#include <qwt_legend.h>
#include <qwt_plot_zoomer.h>
#include <QDockWidget>
#include <QSlider>
#include <QPushButton>
#include <QCheckBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QCloseEvent>
#include <QtUiTools>
#include <limits.h>
#include <sys/stat.h>
#include <iostream>

// The maximum number of bytes in a pathname is PATH_MAX 
#if defined(PATH_MAX)
#define PATHMAX PATH_MAX
#else
#define PATHMAX 1024
#endif

namespace casa {

    QtPlotSvrPanel::colormap_map *QtPlotSvrPanel::colormaps_ = 0;

    void QtPlotSvrPanel::hide( ) {
	window_->base( )->hide( );
    }

    void QtPlotSvrPanel::show( ) {
	window_->base( )->show( );
    }

    void QtPlotSvrPanel::closeMainPanel( ) {
	window_->base( )->closeMainPanel( );
    }

    bool QtPlotSvrPanel::isVisible( ) {
	return window_->base( )->isVisible( );
    }

    void QtPlotSvrPanel::releaseMainPanel( ) {
	window_->base( )->releaseMainPanel( );
    }

    QtPlotSvrPanel::QtPlotSvrPanel( QWidget *parent ) : QObject(parent),slider(0), plot(0)  {

	load_colormaps( );

        QtPlotSvrMain *b = new QtPlotSvrMain( );
	connect( b, SIGNAL(closing(bool)), SLOT(emit_closing(bool)) );
	QWidget *c = new QWidget(b);
	plot = new QtPlotFrame(b);
	slider = new QSlider(Qt::Horizontal,b);
	slider->setTickPosition( QSlider::NoTicks );
	slider->setRange(0,99);
	slider->setTickInterval( 10 );
	zoomer = new QwtPlotZoomer( plot->canvas( ) );
	connect( slider, SIGNAL(sliderReleased( )), b, SLOT(zoom( )));
	connect( zoomer, SIGNAL(zoomed(QwtDoubleRect)), b, SLOT(zoomed(QwtDoubleRect)));
	QVBoxLayout *l = new QVBoxLayout;
	l->addWidget(plot);
	l->addWidget(slider);
	c->setLayout(l);
	b->setCentralWidget( c );
	win_desc *wd = new win_desc( b, c, l );
	window_ = wd;
        defaultfont = QFont("sans serif", 12);
    }

    QtPlotSvrPanel::QtPlotSvrPanel( const QString &label, const QString &xlabel, const QString &ylabel, const QString &window_title,
				    const QList<int> &size, const QString &legend, const QString &zoom, QtPlotSvrPanel *with_panel,
				    bool new_row, QWidget *parent ) : QObject(parent), slider(0), plot(0) {

	load_colormaps( );

	bool ztop = (zoom == "top");
	bool zleft = (ztop ? false : (zoom == "left"));
	bool zright = (ztop || zleft ? false : (zoom == "right"));
	bool zbottom = (ztop || zleft || zright ? false : (zoom == "bottom"));

	QwtPlot::LegendPosition  legend_position = QwtPlot::BottomLegend;
	if ( legend == "top" ) {
	    legend_position = QwtPlot::TopLegend;
	} else if ( legend == "bottom" ) {
	    legend_position = QwtPlot::BottomLegend;
	} else if ( legend == "left" ) {
	    legend_position = QwtPlot::LeftLegend;
	} else if ( legend == "right" ) {
	    legend_position = QwtPlot::RightLegend;
	}

	memory::cptr<row_desc> rd;
	if ( with_panel && new_row == false && with_panel->rows_->find(with_panel) != with_panel->rows_->end( ) ) {
	    rows_ = with_panel->rows_;
	    rd = with_panel->rows_->find(with_panel)->second;
	    window_ = with_panel->window_;
	} else {

	    if ( with_panel && new_row == true && with_panel->rows_->find(with_panel) != with_panel->rows_->end( ) ) {
		rows_ = with_panel->rows_;
		window_ = with_panel->window_;
	    } else {
		rowmap *rm = new rowmap( );
		rows_ = rm;

		QtPlotSvrMain *b = new QtPlotSvrMain( );
		QWidget *c = new QWidget(b);
		QVBoxLayout *l = new QVBoxLayout;
		c->setLayout(l);
		win_desc *wd = new win_desc(b, c,l);
		window_ = wd;

		b->setCentralWidget( window_->container( ) );
	    }

	    QWidget *row = new QWidget(window_->base( ));
	    QHBoxLayout *row_layout = new QHBoxLayout;
	    row->setLayout(row_layout);
	    window_->layout( )->addWidget(row);

	    row_desc *r = new row_desc( row, row_layout );
	    rd = r;
	}

	connect( window_->base( ), SIGNAL(closing(bool)), SLOT(emit_closing(bool)) );

	QWidget *plot_container = new QWidget(window_->base( ));

	QLayout *plot_layout;

	if ( zleft || zright ) {
	    plot_layout = new QHBoxLayout;
	} else {
	    plot_layout = new QVBoxLayout;
	}

	plot_container->setLayout(plot_layout);
	rd->layout()->addWidget(plot_container);

	QSize qsize;
	if ( size.size() == 2 && size[0] > 0 && size[1] > 0 ) {
	    qsize.setWidth(size[0]);
	    qsize.setHeight(size[1]);
	}

	plot = new QtPlotFrame( label, qsize, legend_position, window_->base( ));

	if ( ylabel.length( ) > 0 )
	    plot->setAxisTitle( QwtPlot::yLeft, ylabel );
	if ( xlabel.length( ) > 0 )
	    plot->setAxisTitle( QwtPlot::xBottom, xlabel );

	if ( window_title.length( ) > 0 ) {
	    window_->base( )->setWindowTitle( window_title );
	}

	if ( zleft || zright || ztop || zbottom ) {
	    slider = new QSlider( zleft || zright ? Qt::Vertical : Qt::Horizontal, window_->base() );
	}

	if ( zleft || ztop ) {
	    if ( slider ) { plot_layout->addWidget(slider); }
	    plot_layout->addWidget(plot);
	} else { 
	    plot_layout->addWidget(plot);
	    if ( slider ) { plot_layout->addWidget(slider); }
	}

	rd->addplot( plot_container, plot_layout, plot, slider );
	rows_->insert(rowmap::value_type(this,rd));

	zoomer = new QwtPlotZoomer( plot->canvas( ) );
	if ( slider ) {
	    slider->setTickPosition( QSlider::NoTicks );
	    slider->setRange(0,99);
	    slider->setTickInterval( 10 );
	    slider_last_value = 0;
	    connect( slider, SIGNAL(sliderReleased( )), this, SLOT(zoom( )));
	}
	connect( zoomer, SIGNAL(zoomed(QwtDoubleRect)), this, SLOT(zoomed(QwtDoubleRect)));

    }

    std::pair<QDockWidget*,QString> QtPlotSvrPanel::loaddock( const QString &file_or_xml, const QString &loc, const QStringList &dockable ) {

	QWidget *widget = loaddock( file_or_xml );
	if ( widget == 0 ) {
	    return std::pair<QDockWidget*,QString>(0,QString( "failed to load a widget" ));
	}

	QDockWidget *dockwidget = dynamic_cast<QDockWidget*>(widget);
	if ( dockwidget == 0 ) {
	    delete widget;
	    return std::pair<QDockWidget*,QString>(0,QString("widget loaded was not a dock widget" ));
	}

	Qt::DockWidgetArea location = ( loc == "right" ? Qt::RightDockWidgetArea : loc == "left" ? Qt::LeftDockWidgetArea :
					loc == "top" ? Qt::TopDockWidgetArea : Qt::BottomDockWidgetArea);

	window_->base( )->addDockWidget( location, dockwidget, Qt::Vertical );

	if ( dockable.size( ) == 0 ) {
	    dockwidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
	} else {
	    QFlags<Qt::DockWidgetArea> l = location;
	    for (int i=0; i < dockable.size( ); ++i ) {
		Qt::DockWidgetArea newl = ( dockable[i] == "right" ? Qt::RightDockWidgetArea : 
					    dockable[i] == "left" ? Qt::LeftDockWidgetArea :
					    dockable[i] == "top" ? Qt::TopDockWidgetArea :
					    Qt::BottomDockWidgetArea );
		l |= newl;
	    }

	    // if the only dockable location is the specified location, then it is not movable...
	    if ( l == location ) {
		dockwidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
	    } else {
		dockwidget->setFeatures(QDockWidget::DockWidgetMovable);
		dockwidget->setAllowedAreas( l );
	    }
	}

	QList<QPushButton*> buttons = dockwidget->findChildren<QPushButton*>( );
	for ( QList<QPushButton*>::iterator iter = buttons.begin(); iter != buttons.end(); ++iter ) {
	  connect( *iter, SIGNAL(clicked( )), SLOT(emit_button( )) );
	}
	QList<QCheckBox*> checks = dockwidget->findChildren<QCheckBox*>( );
	for ( QList<QCheckBox*>::iterator iter = checks.begin(); iter != checks.end(); ++iter ) {
	  connect( *iter, SIGNAL(stateChanged(int)), SLOT(emit_check(int)) );
	}
	QList<QRadioButton*> radios = dockwidget->findChildren<QRadioButton*>( );
	for ( QList<QRadioButton*>::iterator iter = radios.begin(); iter != radios.end(); ++iter ) {
	  connect( *iter, SIGNAL(toggled(bool)), SLOT(emit_radio(bool)) );
	}
	QList<QLineEdit*> lines = dockwidget->findChildren<QLineEdit*>( );
	for ( QList<QLineEdit*>::iterator iter = lines.begin(); iter != lines.end(); ++iter ) {
	  connect( *iter, SIGNAL(textChanged(const QString&)), SLOT(emit_linetext(const QString&)) );
	}
	QList<QSlider*> sliders = dockwidget->findChildren<QSlider*>( );
	for ( QList<QSlider*>::iterator iter = sliders.begin(); iter != sliders.end(); ++iter ) {
	  connect( *iter, SIGNAL(valueChanged(int)), SLOT(emit_slidevalue(int)) );
	}
	return std::pair<QDockWidget*,QString>(dockwidget,QString(""));
    }

    QWidget *QtPlotSvrPanel::loaddock( QString file ) {
	QUiLoader loader;
	struct stat buf;
	if ( file.size( ) <= PATHMAX && stat(file.toAscii().constData(),&buf) == 0 ) {
	    QFile qfile(file);
	    qfile.open(QFile::ReadOnly);
	    // return 0 upon error...
	    QWidget *dock = loader.load(&qfile, window_->base( ));
	    qfile.close( );
	    return dock;
	} else {
	    QBuffer qfile;
	    qfile.setData( file.toAscii().constData(), file.size( ) );
	    QWidget *dock = loader.load(&qfile, window_->base( ));
	    qfile.close( );
	    return dock;
	}
    }


    QwtPlotCurve *QtPlotSvrPanel::line(  const QList<double> &x, const QList<double> &y, const QString &color, const QString &label ) {
	QwtPlotCurve *curve = new QwtPlotCurve(label);

	if ( label.length( ) == 0 ) {
	    curve->setItemAttribute( QwtPlotItem::Legend, false );
	}

#if 0
	std::vector<int> v( 10);
	a = new int [v.size()];
	copy( v.begin(), v.end(), a);
	delete [] a;
#endif

	curve->setData( x.toVector( ), y.toVector( ) );
	curve->setPen(QPen(QColor(color)));
	curve->attach(plot);

	plot->replot( );
	zoomer->setZoomBase( );
	return curve;
    }

    QwtPlotCurve *QtPlotSvrPanel::scatter(  const QList<double> &x, const QList<double> &y, const QString &color,
					    const QString &label, const QString &symbol, int symbol_size, int dot_size ) {
	QwtPlotCurve *curve = new QwtPlotCurve(label);


	if ( label.length( ) == 0 ) {
	    curve->setItemAttribute( QwtPlotItem::Legend, false );
	}

	curve->setData( x.toVector( ), y.toVector( )  );
	QPen pen = QPen(QColor(color));
	if ( symbol.length( ) == 0 ) {
	    curve->setStyle(QwtPlotCurve::Dots);
	    pen.setWidth( dot_size < 0 ? 2 : dot_size );
	    curve->setPen(pen);
	} else {
	    curve->setStyle( dot_size < 0 ? QwtPlotCurve::NoCurve : QwtPlotCurve::Dots );
	    QwtSymbol sym = plot->symbol(symbol);
	    if ( symbol_size > 0 ) {
		QSize size(symbol_size,symbol_size);
		sym.setSize(size);
	    }
	    sym.setPen(pen);
	    curve->setSymbol(sym);
	    QPen plot_pen(pen);
	    plot_pen.setWidth(dot_size < 0 ? 0 : dot_size);
	    curve->setPen(plot_pen);
	}
	curve->attach(plot);

	plot->replot( );
	zoomer->setZoomBase( );
	return curve;
    }

    QtPlotHistogram *QtPlotSvrPanel::histogram( const QList<double> &srcValues, int bins, const QString &color, const QString &label ) {
	QtPlotHistogram *histogram = new QtPlotHistogram(label);
	histogram->setColor(QColor(color));

	if ( label.length( ) == 0 ) {
	    histogram->setItemAttribute( QwtPlotItem::Legend, false );
	}

	const int numBins = (bins > 0 ? bins : 25);
	double minimumValue = (double) LONG_MAX;
	double maximumValue = (double) LONG_MIN;
	for ( int x = 0; x < srcValues.size( ); ++x ) {
	    if ( srcValues[x] < minimumValue ) minimumValue = srcValues[x];
	    if ( srcValues[x] > maximumValue ) maximumValue = srcValues[x];
	}

	QwtArray<QwtDoubleInterval> intervals(numBins);
	QwtArray<double> values(numBins);
	double pos = (double) minimumValue;
	double bucketWidth = (double) (maximumValue - minimumValue) / (double) numBins;
	for ( int i = 0; i < (int) intervals.size(); ++i ) {
	    intervals[i] = QwtDoubleInterval(pos, pos + bucketWidth);
	    values[i] = 0.0;
	    pos += bucketWidth;
	}

	for ( int x = 0; x < srcValues.size( ); ++x ) {
	    int bucket = (int) ((srcValues[x]-minimumValue) / bucketWidth);

	    if ( bucket < 0 ) { bucket = 0; }
	    if ( bucket >= numBins ) { bucket = numBins-1; }
	    values[bucket] += 1.0;
	}

	double maximumBar = 0.0;
	for ( int x = 0; x < (int) values.size(); ++x ) {
	    values[x] = (((double) values[x] / (double) srcValues.size( )) * 100.0);
	    if ( values[x] > maximumBar ) maximumBar = values[x];
	}

	histogram->setData(QwtIntervalData(intervals, values));
	histogram->attach(plot);
	plot->setAxisScale(QwtPlot::yLeft, 0.0, maximumBar + (0.1 * maximumBar));
	plot->setAxisScale(QwtPlot::xBottom, (double) minimumValue, pos);

	plot->replot( );
	zoomer->setZoomBase( );
	return histogram;
    }

    QwtPlotSpectrogram *QtPlotSvrPanel::raster( const QList<double> &matrix, int sizex, int sizey, const QString &colormap ) {
	QwtPlotSpectrogram *result = new QwtPlotSpectrogram( );
	QtRasterData data(result);
	QwtDoubleRect box;
	box.setLeft(0);
	box.setRight(sizex);
	box.setTop(0);
	box.setBottom(sizey);
	data.setBoundingRect( box );
	data.setData( matrix, sizex, sizey );
	result->setData(data);
	colormap_map::iterator it = colormaps_->find(colormap);
	if ( it != colormaps_->end() ) {
	    result->setColorMap( *(it->second) );
	}
	result->attach(plot);
	plot->replot( );
	return result;
    }

    QStringList QtPlotSvrPanel::colors( ) {
	return QtPlotFrame::colors( );
    }

    QStringList QtPlotSvrPanel::colormaps( ) {
	load_colormaps( );
	QStringList result;
	for ( colormap_map::iterator it=colormaps_->begin(); it != colormaps_->end(); ++it ) {
	    result.push_back(it->first);
	}
	return result;
    }

    QStringList QtPlotSvrPanel::symbols( ) {
	return QtPlotFrame::symbols( );
    }

    void QtPlotSvrPanel::setxlabel( const QString &xlabel ) {
	if ( plot ) {
	    QwtText qxlabel(xlabel);
	    qxlabel.setFont(defaultfont);
	    plot->setAxisTitle( QwtPlot::xBottom, qxlabel );
	  }
    }

    void QtPlotSvrPanel::setylabel( const QString &ylabel ) {
      if ( plot ) {
	    QwtText qylabel(ylabel);
	    qylabel.setFont(defaultfont);
	    plot->setAxisTitle( QwtPlot::yLeft, qylabel );
	  }
    }

    void QtPlotSvrPanel::settitle( const QString &title ) {
      if ( plot ) { 
 	  plot->setTitle(title);
	  ((QWidget*) (plot->titleLabel()))->setFont(defaultfont);
	}
    }

    void QtPlotSvrPanel::replot( ) {
	if ( plot ) plot->replot( );
    }

    bool disable_it = false;
    void QtPlotSvrPanel::zoom( int index ) {
	// allow this slot to work both with QSlider's
	// valueChanged( ) or sliderReleased( )...
	if ( index < 0 ) {
	    if ( slider ) index = slider->value( );
	}

	// slider runs (only) from min (0) to max (99), but
	// we want it to run from max to min... indicating
	// the amount of zoom...
	index = 100 - index;

	// slider was moved around, but returned to the same place...
	if ( index == slider_last_value ) { return; }

	if ( index == 100 ) {
	    zoomer->zoom(0);
	    zoomer->setZoomBase( );
	} else {

	    QwtDoubleRect base = zoomer->zoomBase( );
	    QwtDoubleRect cur = zoomer->zoomRect( );
	    double base_area = base.width( ) * base.height( );
	    double target_area = base_area * ( (double) index / 100.0 );
	    double current_area = cur.width( ) * cur.height( );

	    if ( current_area == target_area ) { return; }
	      
	    double delta = sqrt(target_area/current_area) - 1;
	    double target_width = cur.width() + cur.width() * delta;
	    double target_height = cur.height() + cur.height() * delta;
	    double new_left, new_top;
	    if ( target_area < current_area ) {
		new_left = cur.left( ) - (target_width - cur.width()) / 2.0;
		new_top = cur.top( ) - (target_height - cur.height()) / 2.0;
	    } else {
		new_left = cur.left( ) - (target_width - cur.width()) / 2.0;
		new_top = cur.top( ) - (target_height - cur.height()) / 2.0;
	    }

	    disable_it = true;
	    QwtDoubleRect new_rect( new_left, new_top, target_width, target_height );
	    zoomer->zoom(new_rect);
	    replot( );

	}

	slider_last_value = index;
    }

    void QtPlotSvrPanel::zoomed( const QwtDoubleRect &r ) {

	if ( disable_it ) {
	    disable_it = false;
	    return;
	}
	if ( zoomer->zoomRectIndex( ) == 0 ) return;

	QwtDoubleRect base = zoomer->zoomBase( );
	if ( (r.width()*r.height()) >= (base.width()*base.height()) ) {
	    if ( slider ) slider->setValue( 0 );
	    current_zoom_level = 100;
	} else {
	    double fraction = (((r.width()*r.height()) / (base.width()*base.height())) * 100.0);
	    current_zoom_level = fraction;
	    if ( slider ) {
		slider->setValue( (int) (100.0 - current_zoom_level) );
		slider_last_value = slider->value( );
	    }
	}
    }

    void QtPlotSvrMain::closeEvent(QCloseEvent *event) {
	if ( ! isOverridedClose( ) ) {
	    emit closing( false );
	    event->ignore( );
	    hide( );	  
	} else {
	    emit closing( true );
	    QtPanelBase::closeEvent(event);
	}
    }

    void QtPlotSvrPanel::emit_button( ) {
	emit button( this, sender( )->objectName( ) );
    }

    void QtPlotSvrPanel::emit_check( int state ) {
	emit check( this, sender( )->objectName( ), state );
    }

    void QtPlotSvrPanel::emit_radio( bool state ) {
	emit radio( this, sender( )->objectName( ), state );
    }

    void QtPlotSvrPanel::emit_linetext( const QString &text ) {
	emit linetext( this, sender( )->objectName( ), text );
    }

    void QtPlotSvrPanel::emit_slidevalue( int value ) {
	emit slidevalue( this, sender( )->objectName( ), value );
    }

    void QtPlotSvrPanel::emit_closing( bool v ) {
	emit closing( this, v );
    }

    void QtPlotSvrPanel::load_colormaps( ) {
	static bool loaded = false;

	if ( loaded ) return;
	loaded = true;

	colormaps_ = new colormap_map( );

        casa::Table colormap_table;

	casa::String root = Aipsrc::aipsRoot();
	casa::String defaultpath = root+"/data/gui/colormaps/default.tbl";
	casa::String useSystemCmap;
	casa::String altpath,userpath;
	casa::Aipsrc::find(useSystemCmap,"display.colormaps.usedefault","yes");
	casa::Aipsrc::find(userpath,"display.colormaps.usertable","");  
	try {
	    if (!useSystemCmap.matches(Regex("[ \t]*(([nN]o)|([fF]alse))[ \t\n]*"))) {
		// default cmaps
		colormap_table = casa::Table(defaultpath);
		if (!userpath.empty()) {
		    // default and user cmaps
		    colormap_table = casa::Table(userpath);
		}
	    } else {      
		if (!userpath.empty()) {
		    // user cmaps only
		    colormap_table = casa::Table(userpath);
		}
	    }    
	} catch (const casa::AipsError &x) {
	    fprintf( stderr, "could not load default CASA colormaps: %s\n", x.what() );
	    return;
	} catch (...) {
	    fprintf( stderr, "could not load default colormaps...\n" );
	    return;
	}

        unsigned int n = colormap_table.nrow();

        casa::ROArrayColumn<casa::String> synonym_col(colormap_table, "SYNONYMS");
        casa::ROScalarColumn<casa::String> name_col (colormap_table, "CMAP_NAME");
        casa::ROArrayColumn<casa::Float> red_col (colormap_table, "RED");
        casa::ROArrayColumn<casa::Float> green_col (colormap_table, "GREEN");
        casa::ROArrayColumn<casa::Float> blue_col (colormap_table, "BLUE");

        for ( unsigned int i=0; i < n; ++i ) {
	    memory::cptr<QwtLinearColorMap> cmap(new QwtLinearColorMap(QwtColorMap::RGB));
            casa::String name = name_col(i);
            casa::Vector<casa::String> synonyms(synonym_col(i));

            casa::Vector<casa::Float> red(red_col(i));
            casa::Vector<casa::Float> green(green_col(i));
            casa::Vector<casa::Float> blue(blue_col(i));

            unsigned int len = red.nelements() < green.nelements() ? red.nelements() : green.nelements();
            len = len < blue.nelements() ? len : blue.nelements( );
	    QColor c;
            for ( unsigned int m=0; m < len; ++m ) {
		c.setRgbF(red[m],green[m],blue[m]);
		cmap->addColorStop( (double) m / (double) (len-1), c );
	    }

            colormaps_->insert( std::make_pair(QString::fromStdString(name),cmap) );

            if ( synonyms.nelements( ) > 0 ) {
		const unsigned int len = synonyms.nelements( );
                for ( unsigned int s=0; s < len; ++s ) {
		    colormaps_->insert( std::make_pair(QString::fromStdString(synonyms(s)),cmap) );
                }
            }
        }
    }

}
