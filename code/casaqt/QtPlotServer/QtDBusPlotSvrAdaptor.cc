//# QtDBusPlotSvrAdaptor.cc: provide plotting services via DBus
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
#include <casaqt/QtPlotServer/QtDBusPlotSvrAdaptor.qo.h>
#include <casaqt/QtPlotServer/QtPlotSvrPanel.qo.h>
#include <casaqt/QtUtilities/QtId.h>
#include <display/QtViewer/QtApp.h>

namespace casa {

    QtDBusPlotSvrAdaptor::QtDBusPlotSvrAdaptor( QtPlotServer *s ) : QDBusAbstractAdaptor(new QObject()), server(s) { }

    QtDBusPlotSvrAdaptor::~QtDBusPlotSvrAdaptor( ) { emit exiting( ); }


    QDBusVariant QtDBusPlotSvrAdaptor::panel( const QString &title, const QString &xlabel, const QString &ylabel, const QString &window_title,
					      const QList<int> &size, const QString &legend, const QString &zoom, int with_panel, bool new_row,
					      bool hidden ) {
	QtPlotSvrPanel *companion = 0;

	if ( with_panel != 0 ) {
	    if ( managed_panels.find( with_panel ) == managed_panels.end( ) ) {
		char buf[50];
		sprintf( buf, "%d", with_panel );
		return error(QString("companion panel '") + buf + "' not found");
	    } else {
		companion = managed_panels.find( with_panel )->second->panel();
	    }
	}

	QtPlotSvrPanel *panel = server->panel( title, xlabel, ylabel, window_title, size, legend, zoom, companion, new_row );

	if ( hidden ) panel->hide( );
	else panel->show( );
	connect( panel, SIGNAL(button(QtPlotSvrPanel*,QString)), SLOT(emit_button(QtPlotSvrPanel*,QString)) );
	connect( panel, SIGNAL(check(QtPlotSvrPanel*,QString,int)), SLOT(emit_check(QtPlotSvrPanel*,QString,int)) );
	connect( panel, SIGNAL(radio(QtPlotSvrPanel*,QString,bool)), SLOT(emit_radio(QtPlotSvrPanel*,QString,bool)) );
	connect( panel, SIGNAL(linetext(QtPlotSvrPanel*,QString,const QString &)), SLOT(emit_linetext(QtPlotSvrPanel*,QString,const QString &)) );
	connect( panel, SIGNAL(slidevalue(QtPlotSvrPanel*,QString,int)), SLOT(emit_slidevalue(QtPlotSvrPanel*,QString,int)) );
	connect( panel, SIGNAL(closing(QtPlotSvrPanel*,bool)), SLOT(emit_closing(QtPlotSvrPanel*,bool)) );
	return QDBusVariant(QVariant(get_id(panel)));
    }


    QStringList QtDBusPlotSvrAdaptor::colors( ) {
	return QtPlotSvrPanel::colors( );
    }

    QStringList QtDBusPlotSvrAdaptor::colormaps( ) {
	return QtPlotSvrPanel::colormaps( );
    }

    QStringList QtDBusPlotSvrAdaptor::symbols( ) {
	return QtPlotSvrPanel::symbols( );
    }


    QDBusVariant QtDBusPlotSvrAdaptor::line( const QList<double> &x, const QList<double> &y, const QString &color, const QString &label, int panel_id ) {

	if ( panel_id != 0 && managed_panels.find( panel_id ) == managed_panels.end( ) ) {
	    char buf[50];
	    sprintf( buf, "%d", panel_id );
	    return error(QString("panel '") + buf + "' not found");
	}

	panel_desc *paneldesc = 0;

	if ( panel_id == 0 ) {
	    if ( managed_panels.size( ) == 0 ) {
		QtPlotSvrPanel *panel = server->panel( "", "bottom" );
		panel_id = get_id(panel);					// adds it to the map of managed panels
		paneldesc = managed_panels.find( panel_id )->second;
	    } else {
		paneldesc = managed_panels.begin( )->second;
	    }
	} else {
	    paneldesc = managed_panels.find( panel_id )->second;
	}

	QwtPlotCurve *plot = paneldesc->panel( )->line( x, y, color, label );
	int data_id = get_id(paneldesc->panel( ),plot);
	paneldesc->data( ).push_back(data_id);
	return QDBusVariant(QVariant(data_id));
    }

    QDBusVariant QtDBusPlotSvrAdaptor::scatter( const QList<double> &x, const QList<double> &y, const QString &color, const QString &label,
						const QString &symbol, int symbol_size, int dot_size, int panel_id ) {

	if ( panel_id != 0 && managed_panels.find( panel_id ) == managed_panels.end( ) ) {
	    char buf[50];
	    sprintf( buf, "%d", panel_id );
	    return error(QString("panel '") + buf + "' not found");
	}

	panel_desc *paneldesc = 0;

	if ( panel_id == 0 ) {
	    if ( managed_panels.size( ) == 0 ) {
		QtPlotSvrPanel *panel = server->panel( "", "bottom" );
		panel_id = get_id(panel);					// adds it to the map of managed panels
		paneldesc = managed_panels.find( panel_id )->second;
	    } else {
		paneldesc = managed_panels.begin( )->second;
	    }
	} else {
	    paneldesc = managed_panels.find( panel_id )->second;
	}

	QwtPlotCurve *plot = paneldesc->panel( )->scatter( x, y, color, label, symbol, symbol_size, dot_size );
	int data_id = get_id(paneldesc->panel( ), plot);
	paneldesc->data( ).push_back(data_id);
	return QDBusVariant(QVariant(data_id));
    }

    QDBusVariant QtDBusPlotSvrAdaptor::histogram( const QList<double> &values, int bins, const QString &color, const QString &label, int panel_id ) {

	if ( panel_id != 0 && managed_panels.find( panel_id ) == managed_panels.end( ) ) {
	    char buf[50];
	    sprintf( buf, "%d", panel_id );
	    return error(QString("panel '") + buf + "' not found");
	}

	panel_desc *paneldesc = 0;

	if ( panel_id == 0 ) {
	    if ( managed_panels.size( ) == 0 ) {
		QtPlotSvrPanel *panel = server->panel( "", "bottom" );
		panel_id = get_id(panel);					// adds it to the map of managed panels
		paneldesc = managed_panels.find( panel_id )->second;
	    } else {
		paneldesc = managed_panels.begin( )->second;
	    }
	} else {
	    paneldesc = managed_panels.find( panel_id )->second;
	}

	QwtPlotItem *plot = paneldesc->panel( )->histogram( values, bins, color, label );
	int data_id = get_id(paneldesc->panel( ),plot);
	paneldesc->data( ).push_back(data_id);
	return QDBusVariant(QVariant(data_id));
    }

    QDBusVariant QtDBusPlotSvrAdaptor::erase( int data ) {
	if ( data == 0 ) {
	    for ( datamap::iterator iter = managed_datas.begin();
		  iter != managed_datas.end( ); ++iter ) {
		iter->second->data( )->detach( );
		delete iter->second;
	    }
	    managed_datas.erase( managed_datas.begin( ), managed_datas.end( ) );
	    return QDBusVariant(QVariant(true));
	}

	datamap::iterator dataiter = managed_datas.find( data );
	if ( dataiter == managed_datas.end( ) ) {
	    // see if the id we have maches any panels...
	    panelmap::iterator paneliter = managed_panels.find( data );
	    if ( paneliter == managed_panels.end( ) ) {
		char buf[50];
		sprintf( buf, "%d", data );
		return error(QString("data (or panel) '") + buf + "' not found");
	    } else {
		// fish through the data assigned to this panel and erase these plots...
		std::list<int> &datas = paneliter->second->data( );
		for ( std::list<int>::iterator iter = datas.begin( );
		      iter != datas.end( ); ++iter ) {
		    datamap::iterator data = managed_datas.find(*iter);
		    if ( data != managed_datas.end( ) ) {
			data->second->data( )->detach( );
			delete data->second;
			managed_datas.erase(data);
		    }
		}
		paneliter->second->panel( )->replot( );
		return QDBusVariant(QVariant(true));
	    }
	}

	for ( panelmap::iterator pi = managed_panels.begin( ); pi != managed_panels.end(); ++pi ) {
	    if ( pi->second->panel() == dataiter->second->panel( ) ) {
		std::list<int> &pd = pi->second->data();
		for ( std::list<int>::iterator pdi=pd.begin( ); pdi != pd.end( ); ++pdi ) {
		    if ( *pdi == dataiter->second->id( ) ) {
			pd.erase(pdi);
			break;
		    }
		}
		break;
	    }
	}
	      
	// erase the one curve that matches
	dataiter->second->data( )->detach( );
	dataiter->second->panel( )->replot( );
	delete dataiter->second;
	managed_datas.erase( dataiter );
	return QDBusVariant(QVariant(true));
    }


    QDBusVariant QtDBusPlotSvrAdaptor::raster( const QList<double> &matrix, int sizex, int sizey, const QString &colormap, int panel_id ) {
	if ( panel_id != 0 && managed_panels.find( panel_id ) == managed_panels.end( ) ) {
	    char buf[50];
	    sprintf( buf, "%d", panel_id );
	    return error(QString("panel '") + buf + "' not found");
	}

	panel_desc *paneldesc = 0;
	if ( panel_id == 0 ) {
	    if ( managed_panels.size( ) == 0 ) {
		QtPlotSvrPanel *panel = server->panel( "", "bottom" );
		panel_id = get_id(panel);					// adds it to the map of managed panels
		paneldesc = managed_panels.find( panel_id )->second;
	    } else {
		paneldesc = managed_panels.begin( )->second;
	    }
	} else {
	    paneldesc = managed_panels.find( panel_id )->second;
	}

	QwtPlotSpectrogram *spect = paneldesc->panel( )->raster(matrix, sizex, sizey, colormap);
	int data_id = get_id(paneldesc->panel( ),spect);
	paneldesc->data( ).push_back(data_id);
	return QDBusVariant(QVariant(data_id));
    }


    QDBusVariant QtDBusPlotSvrAdaptor::setlabel( const QString &xlabel, const QString &ylabel, const QString &title, int panel_id ) {

	if ( panel_id != 0 && managed_panels.find( panel_id ) == managed_panels.end( ) ) {
	    char buf[50];
	    sprintf( buf, "%d", panel_id );
	    return error(QString("panel '") + buf + "' not found");
	}

	panel_desc *paneldesc = 0;
	if ( panel_id == 0 ) {
	    if ( managed_panels.size( ) == 0 ) {
		return error(QString("no panels have been created"));
	    } else {
		paneldesc = managed_panels.begin( )->second;
	    }
	} else {
	    paneldesc = managed_panels.find( panel_id )->second;
	}

	if ( xlabel != "" ) { paneldesc->panel( )->setxlabel( xlabel ); }
	if ( ylabel != "" ) { paneldesc->panel( )->setylabel( ylabel ); }
	if ( title != "" ) { paneldesc->panel( )->settitle( title ); }
	return QDBusVariant(QVariant(true));
    }


    QDBusVariant QtDBusPlotSvrAdaptor::hide( int panel ) {

	if ( panel == 0 ) {
	    for ( panelmap::iterator iter = managed_panels.begin();
		  iter != managed_panels.end(); ++iter )
		iter->second->panel( )->hide( );
	    return QDBusVariant(QVariant(true));
	}

	panelmap::iterator iter = managed_panels.find( panel );
	if ( iter == managed_panels.end( ) ) {
	    return error(QString("could now find requested panel"));
	}

	iter->second->panel( )->hide( );
	return QDBusVariant(QVariant(true));
    }

    QDBusVariant QtDBusPlotSvrAdaptor::show( int panel ) {
	if ( panel == 0 ) {
	    for ( panelmap::iterator iter = managed_panels.begin();
		  iter != managed_panels.end(); ++iter )
		iter->second->panel( )->show( );
	    return QDBusVariant(QVariant(true));
	}

	panelmap::iterator iter = managed_panels.find( panel );
	if ( iter == managed_panels.end( ) ) {
	    return error(QString("could now find requested panel"));
	}

	iter->second->panel( )->show( );
	return QDBusVariant(QVariant(true));
    }

    QDBusVariant QtDBusPlotSvrAdaptor::loaddock( const QString &file_or_xml, const QString &loc, const QStringList &dockable, int panel ) {

	if ( panel == 0 ) {
	    if ( managed_panels.size( ) == 1 ) {
		std::pair<QDockWidget*,QString> result = managed_panels.begin()->second->panel( )->loaddock( file_or_xml, loc, dockable );
		if ( result.first == 0 ) {
		    return error( result.second == "" ? "dock widget creation failure" : result.second );
		} else {
		    int id = QtId::get_id( );
		    managed_docks.insert(dockmap::value_type(id,result.first));
		    return QDBusVariant(QVariant(id));
		}
	    } else {
		return error(QString("must specify a panel when multiple panels exist"));
	    }
	}

	panelmap::iterator iter = managed_panels.find( panel );
	if ( iter == managed_panels.end( ) ) {
	    return error(QString("could now find requested panel"));
	}

	std::pair<QDockWidget*,QString> result = iter->second->panel( )->loaddock( file_or_xml, loc, dockable );
	if ( result.first == 0 ) {
	    return error(result.second == "" ? "dock widget creation failure" : result.second);
	} else {
	    int id = QtId::get_id( );
	    managed_docks.insert(dockmap::value_type(id,result.first));
	    return QDBusVariant(QVariant(id));
	}
    }

    QDBusVariant QtDBusPlotSvrAdaptor::close( int panel ) {

	if ( panel == 0 ) {
	    close_everything( );
	    return QDBusVariant(QVariant(true));
	}

	panelmap::iterator iter = managed_panels.find( panel );
	if ( iter == managed_panels.end( ) ) {
	    return error(QString("could now find requested panel"));
	}

	// fish through the data assigned to this panel and remove
	// these from our cache...
	std::list<int> &datas = iter->second->data( );
	for ( std::list<int>::iterator iter = datas.begin( );
	      iter != datas.end( ); ++iter ) {
	    datamap::iterator data = managed_datas.find(*iter);
	    if ( data != managed_datas.end( ) ) {
		delete data->second;
		managed_datas.erase(data);
	    }
	}

	// now close the panel
	QtPlotSvrPanel *pp = iter->second->panel();
	delete iter->second;
	managed_panels.erase(iter);
	pp->closeMainPanel( );
	return QDBusVariant(QVariant(true));

    }


    QDBusVariant QtDBusPlotSvrAdaptor::release( int panel ) {

	if ( panel == 0 ) {
	    release_everything( );
	    return QDBusVariant(QVariant(true));
	}

	panelmap::iterator iter = managed_panels.find( panel );
	if ( iter == managed_panels.end( ) ) {
	    return error(QString("could now find requested panel"));
	}

	release( iter );
	return QDBusVariant(QVariant(true));
    }

    void QtDBusPlotSvrAdaptor::release( panelmap::iterator &iter ) {

	if ( iter == managed_panels.end( ) ) { return; }

	// now close the panel
	QtPlotSvrPanel *pp = iter->second->panel();
	if ( pp->isVisible( ) == false ) {
	    // releasing this panel will result in closing it...
	    // --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
	    // fish through the data assigned to this panel and remove
	    // these from our cache...
	    std::list<int> &datas = iter->second->data( );
	    for ( std::list<int>::iterator dataiter = datas.begin( );
		  dataiter != datas.end( ); ++dataiter ) {
		datamap::iterator data = managed_datas.find(*dataiter);
		if ( data != managed_datas.end( ) ) {
		    delete data->second;
		    managed_datas.erase(data);
		}
	    }

	    delete iter->second;
	    managed_panels.erase(iter);
	}
	pp->releaseMainPanel( );
	pp->deleteLater( );
    }


    void QtDBusPlotSvrAdaptor::close_everything( ) {
	// Close all open panels, which will exit Qt loop. This does not in
	// itself delete objects or exit the process, although the driver
	// program might do that. Also, some of the panels may have
	// WA_DeleteOnClose set, which would cause their deletion (see, e.g.,
	// QtViewer::createDPG()).
	for ( panelmap::iterator iter = managed_panels.begin();
	      iter != managed_panels.end(); ++iter ) {
	    iter->second->panel()->closeMainPanel( );
	}
    }


    void QtDBusPlotSvrAdaptor::release_everything( ) {
	// Close all open panels, which will exit Qt loop. This does not in
	// itself delete objects or exit the process, although the driver
	// program might do that. Also, some of the panels may have
	// WA_DeleteOnClose set, which would cause their deletion (see, e.g.,
	// QtViewer::createDPG()).
	for ( panelmap::iterator iter = managed_panels.begin();
	      iter != managed_panels.end(); ++iter ) {
	    release( iter );
	}
    }

    void QtDBusPlotSvrAdaptor::done( ) {
	close_everything( );
    }

    void QtDBusPlotSvrAdaptor::emit_button(QtPlotSvrPanel *panel, QString name ) {
	emit button( get_id(panel), name );
    }

    void QtDBusPlotSvrAdaptor::emit_check(QtPlotSvrPanel *panel, QString name, int state ) {
	emit check( get_id(panel), name, state );
    }

    void QtDBusPlotSvrAdaptor::emit_radio(QtPlotSvrPanel *panel, QString name, bool state ) {
	emit radio( get_id(panel), name, state );
    }

    void QtDBusPlotSvrAdaptor::emit_linetext(QtPlotSvrPanel *panel, QString name, const QString &text ) {
	emit linetext( get_id(panel), name, text );
    }

    void QtDBusPlotSvrAdaptor::emit_slidevalue(QtPlotSvrPanel *panel, QString name, int value ) {
	emit slidevalue( get_id(panel), name, value );
    }

    void QtDBusPlotSvrAdaptor::emit_closing(QtPlotSvrPanel *panel, bool gone ) {
	emit closing( get_id(panel), gone );
    }

    int QtDBusPlotSvrAdaptor::get_id( QtPlotSvrPanel *panel ) {

	for ( panelmap::iterator iter = managed_panels.begin(); iter != managed_panels.end(); ++iter ) {
	    if ( iter->second->panel() == panel )
		return iter->first;
	}

	int index = QtId::get_id( );
	managed_panels.insert(panelmap::value_type(index, new panel_desc(panel)));
	return index;
    }

    int QtDBusPlotSvrAdaptor::get_id( QtPlotSvrPanel *panel, QwtPlotItem *data ) {
      
	for ( datamap::iterator iter = managed_datas.begin(); iter != managed_datas.end(); ++iter ) {
	    if ( iter->second->data() == data )
		return iter->second->id();
	}

	int index = QtId::get_id( );
	data_desc *dd = new data_desc(index, panel, data );
	managed_datas.insert(datamap::value_type(index, dd));
	return index;
    }

}
