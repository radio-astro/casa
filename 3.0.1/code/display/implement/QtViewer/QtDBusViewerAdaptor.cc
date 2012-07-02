//# QtViewerAdaptor.cc: provide viewer services via DBus
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
#include <sys/stat.h>
#include <unistd.h>
#include <climits>
#include <sys/param.h>
#include <signal.h>
#include <stdlib.h>
#include <QTextStream>
#include <display/QtViewer/QtDBusViewerAdaptor.qo.h>
#include <display/QtViewer/QtApp.h>
#include <display/QtViewer/QtViewer.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtViewer/QtDisplayPanel.qo.h>
#include <display/QtViewer/QtCleanPanelGui.qo.h>
#include <display/Display/State.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/List.h>
#include <QtDBus>


namespace casa {

    static void launch_ghostview( const char *printer_file );
    static void launch_lpr( const char *printer_file, const char *printer );

    const QString &QtDBusViewerAdaptor::name( ) {
	return QtViewer::name( );
    }

    bool QtDBusViewerAdaptor::connectToDBus(const QString &dbus_name) {

	bool dbusRegistered = false;

	if ( dbusRegistered || serviceIsAvailable(dbusServiceName(dbus_name)) )
	    return false;

	try {
	    // Register service and object.
	    QObject *xparent = parent();
	    dbusRegistered = connection().isConnected() &&
			     connection().registerService(dbusServiceName(dbus_name)) &&
			     connection().registerObject(dbusObjectName(dbus_name), xparent,
// 			     connection().registerObject(dbusObjectName(), parent(),
// 			     connection().registerObject(dbusObjectName(), this,
							 QDBusConnection::ExportAdaptors);

	} catch(...) { dbusRegistered = false; }

	return dbusRegistered;
    }


    void QtDBusViewerAdaptor::handle_interact( QVariant v ) {
	emit interact(QDBusVariant(v));
    }

    QDBusVariant QtDBusViewerAdaptor::start_interact( const QDBusVariant &input, int panel ) {
	panelmap::iterator iter = managed_panels.find( panel );
	if ( iter == managed_panels.end( ) ) {
	    char buf[50];
	    sprintf( buf, "%d", panel );
	    return QDBusVariant(QVariant(QString("*error* panel '") + buf + "' not found"));
	}
	if ( ! iter->second->panel()->supports( QtDisplayPanelGui::INTERACT ) ) {
	    char buf[50];
	    sprintf( buf, "%d", panel );
	    return QDBusVariant(QVariant(QString("*error* panel '") + buf + "' does not support 'interact'"));
	};
	return QDBusVariant(iter->second->panel()->start_interact(input.variant(),panel));
    }


    QDBusVariant QtDBusViewerAdaptor::setoptions( const QDBusVariant &input, int panel ) {

	panelmap::iterator iter = managed_panels.find( panel );
	if ( iter == managed_panels.end( ) ) {
	    char buf[50];
	    sprintf( buf, "%d", panel );
	    return QDBusVariant(QVariant(QString("*error* panel '") + buf + "' not found"));
	}
	if ( ! iter->second->panel()->supports( QtDisplayPanelGui::SETOPTIONS ) ) {
	    char buf[50];
	    sprintf( buf, "%d", panel );
	    return QDBusVariant(QVariant(QString("*error* panel '") + buf + "' does not support 'interact'"));
	};

	QVariant v = input.variant( );
	QMap<QString,QVariant> map;
	if ( v.type( ) == QVariant::UserType ) {
	    QDBusArgument arg = qvariant_cast<QDBusArgument>(v);
	    arg >> map;
	}

	return QDBusVariant(iter->second->panel()->setoptions(map,panel));
    }

    QDBusVariant QtDBusViewerAdaptor::load( const QString &path, const QString &displaytype, int panel ) {

	struct stat buf;
	if ( stat(path.toAscii().constData(),&buf) < 0 ) {
	    // file (or dir) does not exist
	    return QDBusVariant(QVariant("*error* path '" + path + "' not found"));
	}

	QtDisplayPanelGui *dpg = findpanel( panel );
	if ( ! dpg ) {
	    return QDBusVariant(QVariant("*error* could not find requested panel"));
	}

	String datatype = viewer_->filetype(path.toStdString()).chars();
	if ( datatype == "image" || datatype == "ms" ) {
	    if ( displaytype == "raster"	||
		 displaytype == "contour"	||
		 displaytype == "vector"	||
		 displaytype == "marker" ) {
		QtDisplayData *result = 0;

		dpg->autoDDOptionsShow = False;
		result = dpg->createDD(to_string(path), datatype, to_string(displaytype), false);
		dpg->displayPanel()->registerDD(result);
		dpg->autoDDOptionsShow = True;

		dpg->addedData( displaytype, result );
		return QDBusVariant(QVariant(get_id( dpg, result, path, displaytype )));
	    }
	}
	return QDBusVariant(QVariant(QString("*error* datatype '") + datatype.c_str( ) + "' not yet implemented"));
    }

    void QtDBusViewerAdaptor::unload_data( QtDisplayPanelGui *panel, int index, bool erase ) {
	datamap::iterator iter = managed_datas.find( index );
	if ( iter == managed_datas.end( ) ) {
	    fprintf( stderr, "error: internal error (data id not found)" );
	    return;
	}
	if ( iter->second->data( ) != 0 ) {
	    iter->second->panel()->removeDD( iter->second->data( ) );
//***	    fails to notify the wrench that things have changed...
// 	    panel->unregisterDD( iter->second->data( ) );
	    iter->second->data( ) = 0;
	    if ( erase ) managed_datas.erase(iter);
	}
    }

    void QtDBusViewerAdaptor::load_data( QtDisplayPanelGui *panel, int index ) {

	datamap::iterator iter = managed_datas.find( index );
	if ( iter == managed_datas.end( ) ) {
	    fprintf( stderr, "error: internal error (data id not found)" );
	    return;
	}

	struct stat buf;
	const QString &path = iter->second->path();
	const QString &displaytype = iter->second->type();
	if ( stat(path.toAscii().constData(),&buf) < 0 ) {
	    // file (or dir) does not exist
	    fprintf( stderr, "error: file does not exist (%s)", path.toAscii().constData());
	    return;
	}

	String datatype = viewer_->filetype(path.toStdString()).chars();
	if ( datatype == "image" ) {
	    if ( displaytype == "raster"	||
		 displaytype == "contour"	||
		 displaytype == "vector"	||
		 displaytype == "marker" ) {

		QtDisplayData *dp = 0;
		if ( panel != 0 ) {
		    if ( iter->second->data( ) != 0 ) {
			panel->displayPanel()->unregisterDD( iter->second->data( ) );
			iter->second->data( ) = 0;
		    }

		    panel->autoDDOptionsShow = False;
		    dp = panel->createDD(to_string(path), datatype, to_string(displaytype), false);
		    panel->displayPanel()->registerDD(dp);
		    panel->autoDDOptionsShow = True;
		} else {
		    fprintf( stderr, "we can no longer support null panels" );
		    return;
		}
		panel->addedData( displaytype, dp );
		iter->second->data( ) = dp;
	    }
	} else {
	    fprintf( stderr, "error: datatype (%s) not yet implemented", datatype.c_str( ) );
	    return;
	}
    }


    QDBusVariant QtDBusViewerAdaptor::reload( int panel_or_data ) {

	if ( panel_or_data == 0 ) {
	    return QDBusVariant(QVariant("*error* no panel (i.e. '0') provided"));
	}

	panelmap::iterator dpiter = managed_panels.find( panel_or_data );
	if ( dpiter != managed_panels.end( ) ) {
	    QtDisplayPanel::panel_state state = dpiter->second->panel( )->displayPanel()->getPanelState( );
	    dpiter->second->panel( )->displayPanel()->hold( );

	    std::list<int> &data = dpiter->second->data( );
	    for ( std::list<int>::iterator diter = data.begin(); diter != data.end(); ++diter ) {
		unload_data( dpiter->second->panel( ), *diter, false );
	    }

	    for ( std::list<int>::iterator diter = data.begin(); diter != data.end(); ++diter ) {
		load_data( dpiter->second->panel( ), *diter );
	    }
	    dpiter->second->panel( )->displayPanel()->setPanelState( state );
	    dpiter->second->panel( )->displayPanel()->release( );

	} else {
	    datamap::iterator dmiter = managed_datas.find( panel_or_data );
	    if ( dmiter != managed_datas.end( ) ) {
		if ( dmiter->second->id() != panel_or_data ) {
		    fprintf( stderr, "error: internal error (data id mismatch)" );
		}
		load_data( dmiter->second->panel(), dmiter->first );
	    } else {
	      char buf[50];
	      sprintf( buf, "%d", panel_or_data );
	      return QDBusVariant(QVariant(QString("*error* id (") + buf + ") does not reference a panel or data"));
	    }
	}
	return QDBusVariant(QVariant(true));
    }


    QDBusVariant QtDBusViewerAdaptor::unload( int data ) {
	datamap::iterator dmiter = managed_datas.find( data );
	if ( dmiter == managed_datas.end( ) ) {
	    char buf[50];
	    sprintf( buf, "%", data );
	    return QDBusVariant(QVariant(QString("*error* data id (") + buf + ") not found"));
	}
	if ( dmiter->second->id() != data ) {
	    fprintf( stderr, "error: internal error (data id mismatch)" );
	}
	unload_data( dmiter->second->panel( ), data );
	return QDBusVariant(QVariant(true));
    }

    QDBusVariant QtDBusViewerAdaptor::restore( const QString &qpath, int panel ) {

	struct stat buf;
	QByteArray qpatha(qpath.toAscii());
	const char *path = qpatha.constData();
	if ( stat(path,&buf) < 0 || ! S_ISREG(buf.st_mode) ) {
	    // file (or dir) does not exist
	    QDBusVariant(QVariant("*error* file or dir(" + qpath + ") does not exist"));
	}

	QtDisplayPanelGui *dpg = findpanel( panel );
	if ( ! dpg ) {
	    return QDBusVariant(QVariant("*error* could not find requested panel"));
	}

	bool result = dpg->displayPanel()->restorePanelState(path);

	if ( result ) {
	    return QDBusVariant(get_id( dpg ));
	}

	return QDBusVariant(0);
    }

    QtDisplayPanelGui *create_panel( int id ) {
    }

    QDBusVariant QtDBusViewerAdaptor::panel( const QString &type, bool hidden ) {
        int result = 0;
	if ( type == "clean" ) {

	    // <drs> somehow it seems like we must be leaking this...
	    //       probably need to mirror the createDD( ) functionality...
	    QtCleanPanelGui *cpg_ = new QtCleanPanelGui(viewer_);
	    result = get_id( cpg_ );

	    if ( hidden ) cpg_->hide( );

	    connect(cpg_, SIGNAL(interact(QVariant)), this, SLOT(handle_interact(QVariant)));

	} else {

	    QtDisplayPanelGui *dpg = create_panel( );
	    result = get_id( dpg );

	    if ( hidden ) dpg->hide( );
	}

	return QDBusVariant(QVariant(result));
    }


    void QtDBusViewerAdaptor::erase_data( int index ) {
	datamap::iterator dditer = managed_datas.find(index);
	if ( dditer != managed_datas.end( ) ) {
	    delete dditer->second;
	    dditer->second = 0;
	    managed_datas.erase(dditer);
	}
    }

    QtDisplayPanelGui *QtDBusViewerAdaptor::erase_panel( int panel ) {

	if ( panel == 0 ) panel = INT_MAX;

	QtDisplayPanelGui *win;
	panelmap::iterator dpiter = managed_panels.find( panel );
	if ( dpiter != managed_panels.end( ) ) {
	    std::list<int> &data = dpiter->second->data();
	    for ( std::list<int>::iterator diter = data.begin( ); diter != data.end(); ++diter ) {
		erase_data(*diter);
	    }
	    win = dpiter->second->panel();
	    delete dpiter->second;
	    managed_panels.erase(dpiter);
	}
	return win;
    }


    QDBusVariant QtDBusViewerAdaptor::hide( int panel ) {
	QtDisplayPanelGui *dpg = findpanel( panel, false );
	if ( ! dpg ) {
	    return QDBusVariant(QVariant("*error* could not find requested panel"));
	}
	dpg->hide( );
	return QDBusVariant(QVariant(true));
    }

    QDBusVariant QtDBusViewerAdaptor::show( int panel ) {
	QtDisplayPanelGui *dpg = findpanel( panel, false );
	if ( ! dpg ) {
	    return QDBusVariant(QVariant("*error* could not find requested panel"));
	}
	dpg->show( );
	dpg->displayPanel( )->refresh( );
	return QDBusVariant(QVariant(true));
    }

    QDBusVariant QtDBusViewerAdaptor::close( int panel ) {
	QtDisplayPanelGui *dpg = findpanel( panel, false );
	if ( ! dpg ) {
	    return panel == 0 ? QDBusVariant(QVariant(true)) : QDBusVariant(QVariant("*error* could not find requested panel"));
	}

	QtDisplayPanelGui *win = erase_panel( panel );
	if ( win == 0 ) {
	    return QDBusVariant(QVariant("*error* internal error closing panel"));
	}

	win->closeMainPanel( );
	return QDBusVariant(QVariant(true));
    }

    QDBusVariant QtDBusViewerAdaptor::popup( const QString &what, int panel ) {
	QtDisplayPanelGui *dpg = findpanel( panel, false );
	if ( ! dpg ) {
	    return panel == 0 ? QDBusVariant(QVariant(true)) : QDBusVariant(QVariant("*error* could not find requested panel"));
	}
	if ( what == "open" ) {
	    dpg->showDataManager();
	} else {
	    return QDBusVariant(QVariant("*error* unknown popup-widow name, expected 'open'"));
	}
	return QDBusVariant(QVariant(true));
    }

    QDBusVariant QtDBusViewerAdaptor::frame( int num, int panel ) {
	QtDisplayPanelGui *dpg = findpanel( panel, false );
	if ( ! dpg ) {
	    return QDBusVariant(QVariant("*error* could not find requested panel"));
	}

	if ( num >= 0 ) { dpg->displayPanel()->goTo(num); }

	return QDBusVariant(QVariant(dpg->displayPanel()->frame()));
    }


    QDBusVariant QtDBusViewerAdaptor::zoom( int level, int panel ) {
	QtDisplayPanelGui *dpg = findpanel( panel, false );
	if ( ! dpg ) {
	    return QDBusVariant(QVariant("*error* could not find requested panel"));
	}

	if ( level == 0 )
	    dpg->displayPanel()->unzoom( );
	else if ( level < 0 )
	    dpg->displayPanel()->zoomOut( abs(level) );
	else
	    dpg->displayPanel()->zoomIn( level );

	return QDBusVariant(QVariant(true));
    }


    QDBusVariant QtDBusViewerAdaptor::release( int panel ) {
	QtDisplayPanelGui *dpg = findpanel( panel, false );
	if ( ! dpg ) {
	    return panel == 0 ? QDBusVariant(QVariant(true)) : QDBusVariant(QVariant("*error* could not find requested panel"));
	}

	QtDisplayPanelGui *win = erase_panel( panel );
	if ( win == 0 ) {
	    return QDBusVariant(QVariant("*error* internal error releasing panel"));
	}

	win->releaseMainPanel( );
	return QDBusVariant(QVariant(true));
    }

    bool QtDBusViewerAdaptor::output( const QString &device, const QString &devicetype, int panel, double scale,
				      int dpi, const QString &format, const QString &orientation, const QString &media ) {

	QString base;
	QString path;
	QString suffix;

	char printer_file[80];
	char printer_base[80];

        if ( devicetype == "printer" || devicetype == "ghostview" ) {
	    pid_t pid = getpid( );
	    sprintf( printer_file, "/tmp/dbviewer-out-%06d.ps", pid );
	    sprintf( printer_base, "dbviewer-out-%06d", pid );
	    base = printer_base;
	    path = "/tmp";
	    suffix = "ps";
	} else {
	    QFileInfo file(device);
	    base = file.completeBaseName( );
	    path = file.absolutePath();
	    suffix = file.suffix( ).toLower( );

	    if ( suffix != "jpg" && suffix != "pdf" && suffix != "eps" && suffix != "ps" &&
		 suffix != "png" && suffix != "xbm" && suffix != "xpm" && suffix != "ppm" ) {
		suffix = format.toLower( );
		if ( suffix != "jpg" && suffix != "pdf" && suffix != "eps" && suffix != "ps" &&
		     suffix != "png" && suffix != "xbm" && suffix != "xpm" && suffix != "ppm" ) {
		    suffix = "jpg";
		}
	    }

	}


	QtDisplayPanelGui *dpg = findpanel( panel );
	if ( ! dpg ) return false;

	if ( suffix == "pdf" || suffix == "ps" || suffix == "eps" ) {
	    printps( dpg->displayPanel(), suffix, path + "/" + base + "." + suffix, dpi, orientation, media );
	} else if ( suffix == "jpg" || suffix == "png" ||
		    suffix == "xbm" || suffix == "xpm" || suffix == "ppm" ) {
	    printraster( dpg->displayPanel(), suffix, path + "/" + base + "." + suffix, scale );
	} else {
	    return false;
	}

	if ( devicetype == "ghostview" ) {
	    launch_ghostview( printer_file );
	} else if ( devicetype == "printer" ) {
	    launch_lpr( printer_file, device.toAscii( ).constData( ) );
	}

	return true;
    }


    bool QtDBusViewerAdaptor::printps( QtDisplayPanel *panel, const QString &type, const QString &file,
				       int dpi, const QString &orientation, const QString &media ) {
	QPrinter *printer = new QPrinter;

	if ( type == "ps" || type == "eps" ) {
	    printer->setOutputFormat(QPrinter::PostScriptFormat);
	} else if ( type == "pdf" ) {
	    printer->setOutputFormat(QPrinter::PdfFormat);
	}

	printer->setResolution(dpi);
	if ( orientation == "landscape" ) {
	    printer->setOrientation(QPrinter::Landscape);
	} else {
	    printer->setOrientation(QPrinter::Portrait);
	}

	if ( media == "a4" ) {
	    printer->setPageSize(QPrinter::A4);
	} else {
	    printer->setPageSize(QPrinter::Letter);
	}

	char eps_file_name[40];
	if ( type == "eps" ) {
	    pid_t pid = getpid( );
	    sprintf( eps_file_name, "/tmp/eps-out.%06d", pid );
	    printer->setFullPage(true);
	    printer->setOutputFileName(eps_file_name);
	} else {
	    printer->setOutputFileName(file);
	}

	QPainter painter(printer);
	QRect viewport = painter.viewport();
	QRect rect = viewport;
	rect.adjust(72, 72, -72, -72);

	QSize sz = panel->size();

	int xs = sz.width();
	int ys = sz.height();
	QSize pSz(xs, ys);

	double ratio = 1;
	if ( orientation == "landscape" ) {
	    double rx = (double)rect.height() / xs;
	    double ry = (double)rect.width() / ys;
	    ratio = min(rx, ry);
	} else {
	    double rx = (double)rect.width() / xs;
	    double ry = (double)rect.height() / ys;
	    ratio = min(rx, ry);
	}

	pSz.setWidth(int(xs * ratio));
	pSz.setHeight(int(ys * ratio));

	QPixmap pmp(pSz);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	panel->setAllowBackToFront(false);
	String backColor = "white";
	String foreColor = "black";
	panel->setBackgroundPS(backColor, foreColor);
	panel->beginLabelAndAxisCaching( );
	panel->resize(pSz);
	pmp = panel->getBackBuffer()->copy();
	painter.drawPixmap(0,0,pmp);
	panel->endLabelAndAxisCaching( painter );
	painter.end();
	panel->setBackgroundPS(backColor, foreColor);
	panel->setAllowBackToFront(true);
	panel->resize(sz);
	QApplication::restoreOverrideCursor();

	if ( type == "eps" ) {
	    adjusteps( eps_file_name, file.toAscii( ).constData( ), pmp.size(), viewport );
	    remove( eps_file_name );
	}

	delete printer;
	return true;
    }

    // adjust EPS bounding box...
    void  QtDBusViewerAdaptor::adjusteps( const char *from, const char *to,
					  const QSize &wcmax, const QRect &viewport ) {

	FILE *in = fopen( from, "r" );
	FILE *out = fopen( to, "w" );
	bool found = false;
	char buf[2049];
	while ( ! feof(in) ) {
	    char *line = fgets( buf, 2049, in );
	    if ( line ) {
	        if ( ! found && ! strncmp( "%%BoundingBox: ", line, 15 ) ) {

		    float xmin, xmax, ymin, ymax;
		    int g = sscanf( line, "%%%%BoundingBox: %f %f %f %f", &xmin, &ymin, &xmax, &ymax );

		    if ( g != 4 ) {
			fputs( line, out );
		    } else {
			float ratio_y = ymax / float(viewport.height());
			float ratio_x = xmax / float(viewport.width());

			fprintf( out, "%%%%BoundingBox: 0 %d %d %d\n",
				 int((ymax - (wcmax.height() * ratio_y)) + 1),
				 int((wcmax.width() * ratio_x) + 1),
				 int(ymax + 1) );
		    }

		    found = true;

		} else {
		    fputs( line, out );
		}
	    }
	}
	fclose( out );
	fclose( in );
    }

    bool QtDBusViewerAdaptor::printraster( QtDisplayPanel *panel, const QString &type,
					   const QString &file, double scale ) {
	QSize s = panel->canvasSize();
        int width = s.width();
        int height = s.height();

        width =  (int)(((double) width  * scale) + 0.5);
        height = (int)(((double) height * scale) + 0.5);

	// ensure that bitmap is redrawn to avoid displaying labeling within the plot
	if ( s.width() == width && s.height() == height ) {
	    width += 1;
	    height += 1;
	}

	// resized
	QApplication::setOverrideCursor(Qt::WaitCursor);
	display::state::instance().beginFileOutputMode( );

	QSize oldSize = panel->size();
	QSize scaledSize = s;
	int dw = oldSize.width() - scaledSize.width(),
	dh = oldSize.height() - scaledSize.height();
	scaledSize.scale(width, height, Qt::KeepAspectRatio);

	panel->setUpdateAllowed(False);
	// (Prevent display widget flashing during temporary resize.)
	panel->resize(scaledSize.width() + dw, scaledSize.height() + dh);
	QPixmap* mp = panel->contents();
	display::state::instance().endFileOutputMode( );
	panel->setUpdateAllowed(True);
	panel->resize(oldSize);
	QCoreApplication::processEvents();

	QApplication::restoreOverrideCursor();

	if ( ! mp->save(file, type.toAscii().constData( ) ) ) {
	    delete mp;
	    return false;
	}

	delete mp;
	return true;
    }

    QString QtDBusViewerAdaptor::cwd( const QString &new_path ) {
	if ( new_path != "" ) {
	    struct stat buf;
	    const char *p = new_path.toAscii().constData();
	    if ( stat(p,&buf) == 0 && S_ISDIR(buf.st_mode) )
		chdir(p);
	}
	char buf[MAXPATHLEN+1];
	getcwd(buf,MAXPATHLEN+1);
	return QString((const char*)buf);
    }

    QStringList QtDBusViewerAdaptor::keyinfo( int key ) {
	QStringList result;

	if ( managed_datas.find( key ) != managed_datas.end( ) ) {
	    QString ele;
	    QTextStream(&ele) << "data key (" << key << ")";
	    result.append(ele);
	} else if ( managed_panels.find( key ) != managed_panels.end( ) ) {
	    QString ele;
	    QTextStream(&ele) << "panel key (" << key << ")";
	    result.append(ele);
	}
	return result;
    }

   bool QtDBusViewerAdaptor::done( ) {
	// Close all open panels, which will exit Qt loop.  (Note that the
	// loop might be restarted (and is, in interactive clean, e.g.),
	// with existing widgets intact).  This does not in itself delete
	// objects or exit the process, although the driver program might
	// do that.  Also, some of the panels may have WA_DeleteOnClose set,
	// which would cause their deletion (see, e.g., QtViewer::createDPG()).
	for ( panelmap::iterator iter = managed_panels.begin();
	      iter != managed_panels.end(); ++iter ) {
	    iter->second->panel()->closeMainPanel( );
	}
	QtApp::app()->quit();
	return true;
    }

    QtDBusViewerAdaptor::QtDBusViewerAdaptor(QtViewer  *viewer) :
	QDBusAbstractAdaptor(new QObject()), viewer_(viewer) {
    }

    QtDBusViewerAdaptor::~QtDBusViewerAdaptor() {


    }

    QtDisplayPanelGui *QtDBusViewerAdaptor::findpanel( int key, bool create ) {

	if ( key == 0 ) key = INT_MAX;

	if ( managed_panels.find( key ) != managed_panels.end( ) )
	    return managed_panels.find( key )->second->panel( );

	if ( key == INT_MAX && create ) {
	    QtDisplayPanelGui *dpg = create_panel( );
	    managed_panels.insert(panelmap::value_type(INT_MAX, new panel_desc(dpg)));
	    return dpg;
	}

	return 0;
    }

    int QtDBusViewerAdaptor::get_id( QtDisplayPanelGui *panel, QtDisplayData *data, const QString &path, const QString &type ) {

	for ( datamap::iterator iter = managed_datas.begin(); iter != managed_datas.end(); ++iter ) {
	    if ( iter->second->data() == data )
		return iter->second->id();
	}

	int index = QtDBusApp::get_id( );
	data_desc *dd = new data_desc(index, path, type, data, panel );
	managed_datas.insert(datamap::value_type(index, dd));

	for ( panelmap::iterator dpiter = managed_panels.begin(); dpiter != managed_panels.end(); ++dpiter ) {
	    if ( dpiter->second->panel() == panel ) {
		dpiter->second->data().push_back(index);
		break;
	    }
	}

	return index;
    }

    int QtDBusViewerAdaptor::get_id( QtDisplayPanelGui *panel ) {

	for ( panelmap::iterator iter = managed_panels.begin(); iter != managed_panels.end(); ++iter ) {
	    if ( iter->second->panel() == panel )
		return iter->first;
	}

	int index = QtDBusApp::get_id( );
	managed_panels.insert(panelmap::value_type(index, new panel_desc(panel)));
	return index;
    }


    void QtDBusViewerAdaptor::handle_destroyed_panel( QObject *panel ) {
	for ( panelmap::iterator iter = managed_panels.begin();
	      iter != managed_panels.end(); ++iter ) {
	    if ( iter->second->panel( ) == panel ) {
		for ( std::list<int>::iterator diter = iter->second->data().begin();
		      diter != iter->second->data().end(); ++diter ) {
		    datamap::iterator kill = managed_datas.find(*diter);
		    if ( kill != managed_datas.end( ) ) {
			delete kill->second;
			managed_datas.erase(kill);
		    }
		}
		delete iter->second;
		managed_panels.erase(iter);
		break;
	    }
	}
    }

    QtDisplayPanelGui *QtDBusViewerAdaptor::create_panel( ) {
	QtDisplayPanelGui *result = viewer_->createDPG();
	connect( result, SIGNAL(destroyed(QObject*)), SLOT(handle_destroyed_panel(QObject*)) );
	return result;
    }

    ////////////////////////////////////////////////////////////////////////
    /////////File Statics///////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////
    void launch_ghostview( const char *printer_file ) {
	pid_t pid;
	int status;
	if ( fork( ) == 0 ) {
	    setpgrp( );
	    signal( SIGHUP, SIG_IGN );
	    if ( fork( ) != 0 ) {
		// parent-child exits after becoming process group leader to free
		// gv from either having a claim on the user's terminal or being
		// subject to shell process management... csh (in particular) makes
		// all of its children process group leaders...
		exit(0);
	    } else {
		char buf[1024];
		sprintf( buf, "gv %s", printer_file );
		system( buf );
		remove( printer_file );
		exit(0);
	    }
	}
    }

    void launch_lpr( const char *printer_file, const char *printer  ) {
	pid_t pid;
	int status;
	if ( fork( ) == 0 ) {
	    setpgrp( );
	    signal( SIGHUP, SIG_IGN );
	    if ( fork( ) != 0 ) {
		// parent-child exits after becoming process group leader to free
		// gv from either having a claim on the user's terminal or being
		// subject to shell process management... csh (in particular) makes
		// all of its children process group leaders...
		exit(0);
	    } else {
		char buf[4096];
		sprintf( buf, "lpr -P %s %s", printer, printer_file );
		system( buf );
		remove( printer_file );
		exit(0);
	    }
	}
    }

}
