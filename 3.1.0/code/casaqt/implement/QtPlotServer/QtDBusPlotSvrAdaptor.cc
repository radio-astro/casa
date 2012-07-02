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

namespace casa {

    QtDBusPlotSvrAdaptor::QtDBusPlotSvrAdaptor( QtPlotServer *s ) : QDBusAbstractAdaptor(new QObject()), server(s) { }

    QtDBusPlotSvrAdaptor::~QtDBusPlotSvrAdaptor( ) { }


    QDBusVariant QtDBusPlotSvrAdaptor::panel( const QString &title, const QString &xlabel, const QString &ylabel,
					      const QString &window_title, const QString &legend, bool hidden ) {
	QtPlotSvrPanel *panel = server->panel( title, xlabel, ylabel, window_title, legend );
	if ( hidden ) panel->hide( );
	else panel->show( );
	return QDBusVariant(QVariant(get_id(panel)));
    }


    QStringList QtDBusPlotSvrAdaptor::colors( ) {
	return QtPlotSvrPanel::colors( );
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

	// erase the one curve that matches
	dataiter->second->data( )->detach( );
	dataiter->second->panel( )->replot( );
	delete dataiter->second;
	managed_datas.erase( dataiter );
	return QDBusVariant(QVariant(true));
    }


    QDBusVariant QtDBusPlotSvrAdaptor::raster( const QList<double> &matrix, int sizex, int sizey, int panel_id ) {
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

	QwtPlotSpectrogram *spect = paneldesc->panel( )->raster(matrix, sizex, sizey);
	return QDBusVariant(QVariant(1));
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

    bool QtDBusPlotSvrAdaptor::done( ) {
	close_everything( );
	return true;
    }

    int QtDBusPlotSvrAdaptor::get_id( QtPlotSvrPanel *panel ) {

	for ( panelmap::iterator iter = managed_panels.begin(); iter != managed_panels.end(); ++iter ) {
	    if ( iter->second->panel() == panel )
		return iter->first;
	}

	int index = QtDBusApp::get_id( );
	managed_panels.insert(panelmap::value_type(index, new panel_desc(panel)));
	return index;
    }

    int QtDBusPlotSvrAdaptor::get_id( QtPlotSvrPanel *panel, QwtPlotItem *data ) {
      
	for ( datamap::iterator iter = managed_datas.begin(); iter != managed_datas.end(); ++iter ) {
	    if ( iter->second->data() == data )
		return iter->second->id();
	}

	int index = QtDBusApp::get_id( );
	data_desc *dd = new data_desc(index, panel, data );
	managed_datas.insert(datamap::value_type(index, dd));
	return index;
    }

}
