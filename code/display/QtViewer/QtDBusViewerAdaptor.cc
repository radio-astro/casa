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
#include <casaqt/QtUtilities/QtId.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtViewer/QtDisplayPanel.qo.h>
#include <display/QtViewer/QtCleanPanelGui.qo.h>
#include <display/QtViewer/QtCleanPanelGui2.qo.h>
#include <display/Display/State.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/List.h>
#include <QtDBus>


namespace casa {

	static void launch_ghostview( const char *printer_file );
	static void launch_lpr( const char *printer_file, const char *printer );

	void QtDBusViewerAdaptor::handle_interact( QVariant v ) {
		emit interact(QDBusVariant(v));
	}

	QDBusVariant QtDBusViewerAdaptor::start_interact( const QDBusVariant &input, int panel ) {
		panelmap::iterator iter = managed_panels.find( panel );
		if ( iter == managed_panels.end( ) ) {
			char buf[50];
			sprintf( buf, "%d", panel );
			return error(QString("panel '") + buf + "' not found");
		}
		if ( ! iter->second->panel()->supports( QtDisplayPanelGui::INTERACT ) ) {
			char buf[50];
			sprintf( buf, "%d", panel );
			return error(QString("panel '") + buf + "' does not support 'interact'");
		};
		return QDBusVariant(iter->second->panel()->start_interact(input.variant(),panel));
	}


	QDBusVariant QtDBusViewerAdaptor::setoptions( const QDBusVariant &input, int panel ) {

		panelmap::iterator iter = managed_panels.find( panel );
		if ( iter == managed_panels.end( ) ) {
			char buf[50];
			sprintf( buf, "%d", panel );
			return error(QString("panel '") + buf + "' not found");
		}
		if ( ! iter->second->panel()->supports( QtDisplayPanelGui::SETOPTIONS ) ) {
			char buf[50];
			sprintf( buf, "%d", panel );
			return error(QString("panel '") + buf + "' does not support 'interact'");
		};

		QVariant v = input.variant( );
		QMap<QString,QVariant> map;
		if ( v.type( ) == QVariant::UserType ) {
			QDBusArgument arg = qvariant_cast<QDBusArgument>(v);
			arg >> map;
		}

		return QDBusVariant(iter->second->panel()->setoptions(map,panel));
	}

	QDBusVariant QtDBusViewerAdaptor::axes( const QString &x, const QString &y, const QString &z, int panel ) {

		Record rec;
		bool update_axes = false;

		// -- should not be necessary, but when "setOptions( )" is called  --
		// -- from a script the axes values being changed are not updated  --
		// -- in the data options panel...                                 --
		Record optionsChangedRec;

		if ( x.size( ) > 0 ) {
			rec.define( "xaxis", x.toAscii( ).constData( ) );
			update_axes = true;
			Record xaxis;
			xaxis.define("value",x.toAscii( ).constData( ) );
			optionsChangedRec.defineRecord("xaxis",xaxis);
		}

		if ( y.size( ) > 0 ) {
			rec.define( "yaxis", y.toAscii( ).constData( ) );
			update_axes = true;
			Record yaxis;
			yaxis.define("value",y.toAscii( ).constData( ) );
			optionsChangedRec.defineRecord("yaxis",yaxis);
		}

		if ( z.size( ) > 0 ) {
			rec.define( "zaxis", z.toAscii( ).constData( ) );
			update_axes = true;
			Record zaxis;
			zaxis.define("value",z.toAscii( ).constData( ) );
			optionsChangedRec.defineRecord("zaxis",zaxis);
		}

		if ( update_axes == false )
			return QDBusVariant(QVariant(false));

		panelmap::iterator dpiter = managed_panels.find( panel == 0 ? INT_MAX : panel );

		if ( dpiter == managed_panels.end( ) ) {
			char buf[50];
			sprintf( buf, "%d", panel );
			return error(QString("could not find a panel or data with id '") + buf + "'");
		}

		bool set_option = false;
		std::list<int> &data = dpiter->second->data( );
		for ( std::list<int>::iterator diter = data.begin(); diter != data.end(); ++diter ) {
			datamap::iterator dditer = managed_datas.find(*diter);
			if ( dditer != managed_datas.end( ) ) {
				QtDisplayData *dd = dditer->second->data( );
				dd->setOptions(rec,true);
				set_option = true;
				// -- it seems like a better idea to signal change here (when they    --
				// -- fail to update due to scripting ops ) instead of littering the  --
				// -- code with unnecessary updates for the GUI case                  --
				dd->emitOptionsChanged( optionsChangedRec );
			}
		}

		return QDBusVariant(QVariant(set_option));
	}


	QDBusVariant QtDBusViewerAdaptor::datarange( const QList<double> &range, int data ) {

		Vector<Float> value;
		value.resize(2);
		value(0) = (Float) range[0];
		value(1) = (Float) range[1];

		Record rec;
		Record minmax;
		minmax.define( "value", value );
		rec.defineRecord( "minmaxhist", minmax );

		QtDisplayData *dd = finddata(data);

		if ( range.size( ) != 2 ) {

			return error( QString("range must be a list of two numbers") );

		} else if ( dd == 0 ) {

			// if we have a "id-less" panel (INT_MAX), see if we can
			// set the range on rasters there...
			if ( data == 0 ) {
				panelmap::iterator dpiter = managed_panels.find( INT_MAX );

				if ( dpiter != managed_panels.end( ) ) {
					std::list<int> &data = dpiter->second->data( );
					bool found = false;
					for ( std::list<int>::iterator diter = data.begin(); diter != data.end(); ++diter ) {
						datamap::iterator dditer = managed_datas.find(*diter);
						if ( dditer != managed_datas.end( ) ) {
							QtDisplayData *dd = dditer->second->data( );
							if ( dd->displayType( ) == "raster" ) {
								dd->setOptions(rec,true);
								found = true;
							}
						}
					}

					if ( found )
						return QDBusVariant(QVariant(true));
				}
			}

			char buf[50];
			sprintf( buf, "%d", data );
			return error(QString("data id '") + buf + "' not found");

		}

		dd->setOptions(rec,true);

		return QDBusVariant(QVariant(true));
	}

	QDBusVariant QtDBusViewerAdaptor::contourlevels( const QList<double> &levels, double baselevel,
	        double unitlevel, int panel_or_data ) {

		Record rec;
		bool update_contours = false;

		if ( levels.size( ) > 0 ) {
			Vector<Float> value;
			value.resize( levels.size( ) );
			for ( int i=0; i < levels.size( ); ++i ) {
				value(i) = (Float) levels[i];
			}
			rec.define( "rellevels", value );
			update_contours = true;
		}

		if ( baselevel != 2147483648.0 ) {
			rec.define( "basecontour", baselevel );
			update_contours = true;
		}

		if ( unitlevel != 2147483648.0 ) {
			rec.define( "unitcontour", unitlevel );
			update_contours = true;
		}

		if ( update_contours == false ) {
			return QDBusVariant(QVariant(false));
		}

		QtDisplayData *dd = finddata(panel_or_data);

		if ( dd == 0 ) {

			panelmap::iterator dpiter = managed_panels.find( panel_or_data == 0 ? INT_MAX : panel_or_data );

			if ( dpiter == managed_panels.end( ) ) {
				char buf[50];
				sprintf( buf, "%d", panel_or_data );
				return error(QString("could not find a panel or data with id '") + buf + "'");
			}

			bool set_contour = false;
			std::list<int> &data = dpiter->second->data( );
			for ( std::list<int>::iterator diter = data.begin(); diter != data.end(); ++diter ) {
				datamap::iterator dditer = managed_datas.find(*diter);
				if ( dditer != managed_datas.end( ) ) {
					QtDisplayData *dd = dditer->second->data( );
					if ( dd->displayType( ) == "contour" ) {
						dd->setOptions(rec,true);
						set_contour = true;
					}
				}
			}
			return QDBusVariant(QVariant(set_contour));
		}

		dd->setOptions(rec,true);
		return QDBusVariant(QVariant(true));
	}

	QDBusVariant QtDBusViewerAdaptor::colormap( const QString &map, int panel_or_data ) {
		QtDisplayData *dd = finddata(panel_or_data);
		if ( dd == 0 ) {

			panelmap::iterator dpiter = managed_panels.find( panel_or_data == 0 ? INT_MAX : panel_or_data );

			if ( dpiter == managed_panels.end( ) ) {
				char buf[50];
				sprintf( buf, "%d", panel_or_data );
				return error(QString("could not find a panel or data with id '") + buf + "'");
			}

			std::list<int> &data = dpiter->second->data( );
			// first verify that the colormap name is valid for all qt display datas
			for ( std::list<int>::iterator diter = data.begin(); diter != data.end(); ++diter ) {
				datamap::iterator dditer = managed_datas.find(*diter);
				if ( dditer != managed_datas.end( ) ) {
					QtDisplayData *dd = dditer->second->data( );
					if ( dd->hasColormap( ) && dd->isValidColormap( map ) == false ) {
						return error(QString("'") + map + "' is not a valid colormap for one (or more) display data(s)");
					}
				}
			}
			// next replace the colormap
			String colormap_name(map.toAscii().constData());
			for ( std::list<int>::iterator diter = data.begin(); diter != data.end(); ++diter ) {
				datamap::iterator dditer = managed_datas.find(*diter);
				if ( dditer != managed_datas.end( ) ) {
					QtDisplayData *dd = dditer->second->data( );
					if ( dd->hasColormap( ) ) {
						dd->setColormap( colormap_name );
					}
				}
			}
			return QDBusVariant(QVariant(true));
		}

		if ( dd->hasColormap( ) ) {
			if ( dd->isValidColormap( map ) == false ) {
				return error(QString("'") + map + "' is not a valid colormap");
			}
			String colormap_name(map.toAscii().constData());
			dd->setColormap( colormap_name );
		}
		return QDBusVariant(QVariant(true));
	}

	QDBusVariant QtDBusViewerAdaptor::colorwedge( bool show, int panel_or_data ) {
		QtDisplayData *dd = finddata(panel_or_data);

		Record cw;
		cw.define( "wedge", show ? "Yes" : "No" );

		if ( dd == 0 ) {

			panelmap::iterator dpiter = managed_panels.find( panel_or_data == 0 ? INT_MAX : panel_or_data );

			if ( dpiter == managed_panels.end( ) ) {
				char buf[50];
				sprintf( buf, "%d", panel_or_data );
				return error(QString("could not find a panel or data with id '") + buf + "'");
			}

			std::list<int> &data = dpiter->second->data( );
			for ( std::list<int>::iterator diter = data.begin(); diter != data.end(); ++diter ) {
				datamap::iterator dditer = managed_datas.find(*diter);
				if ( dditer != managed_datas.end( ) ) {
					QtDisplayData *dd = dditer->second->data( );
					if ( dd->hasColorBar( ) ) {
						dd->setOptions( cw, true);
					}
				}
			}
			return QDBusVariant(QVariant(true));
		}

		if ( dd->hasColorBar( ) ) {
			dd->setOptions( cw, true);
		}
		return QDBusVariant(QVariant(true));
	}

	QDBusVariant QtDBusViewerAdaptor::load( const QString &path, const QString &displaytype, int panel, double scaling ) {

		struct stat buf;
		if ( stat(path.toAscii().constData(),&buf) < 0 ) {
			// file (or dir) does not exist
			return error("path '" + path + "' not found");
		}

		QtDisplayPanelGui *dpg = findpanel( panel );
		if ( ! dpg ) {
			return error("could not find requested panel");
		}

		String datatype = viewer_->filetype(path.toStdString()).chars();
		if ( datatype == "image" || datatype == "ms" ) {
			if ( displaytype == "raster"	||
			        displaytype == "contour"	||
			        displaytype == "vector"	||
			        displaytype == "marker" ) {
				QtDisplayData *result = 0;
				viewer::DisplayDataOptions ddo;

				if ( scaling != 0.0 ) {
					char buf[1024];
					sprintf( buf, "%f", scaling );
					ddo.insert( "powercycles", buf );
				}

				result = dpg->createDD( to_string(path), datatype, to_string(displaytype), true,
										-1, false, false, false, ddo );
				dpg->addedData( displaytype, result );

				return QDBusVariant(QVariant(get_id( dpg, result, path, displaytype )));
			}
		}
		return error(QString("datatype '") + datatype.c_str( ) + "' not yet implemented");
	}

	void QtDBusViewerAdaptor::unload_data( QtDisplayPanelGui */*panel*/, int index, bool erase ) {
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
						panel->removeDD( iter->second->data( ) );
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
			return error("no panel (i.e. '0') provided");
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
				return error(QString("id (") + buf + ") does not reference a panel or data");
			}
		}
		return QDBusVariant(QVariant(true));
	}


	QDBusVariant QtDBusViewerAdaptor::unload( int data ) {
		datamap::iterator dmiter = managed_datas.find( data );
		if ( dmiter == managed_datas.end( ) ) {
			char buf[50];
			sprintf( buf, "%d", data );
			return error(QString("data id (") + buf + ") not found");
		}
		if ( dmiter->second->id() != data ) {
			fprintf( stderr, "error: internal error (data id mismatch)" );
		}
		unload_data( dmiter->second->panel( ), data );
		return QDBusVariant(QVariant(true));
	}

	QDBusVariant QtDBusViewerAdaptor::restore( const QString &qpath, int panel ) {

		struct stat buf;
		char path[MAXPATHLEN+1];

		sprintf( path, "%s", qpath.toAscii().constData( ) );
		if ( stat(path,&buf) < 0 || ! S_ISREG(buf.st_mode) ) {
			// file (or dir) does not exist
			return error("file or dir(" + qpath + ") does not exist");
		}

		QtDisplayPanelGui *dpg = findpanel( panel );
		if ( ! dpg ) {
			return error("could not find requested panel");
		}

		bool result = dpg->displayPanel()->restorePanelState(path);

		if ( result ) {
			return QDBusVariant(get_id( dpg ));
		}

		return QDBusVariant(0);
	}

	QDBusVariant QtDBusViewerAdaptor::panel( const QString &type, bool hidden ) {
		int result = 0;
		if ( type == "clean" ) {

			// <drs> need to ensure that this is not leaked...
			QtCleanPanelGui *cpg_ = viewer_->createInteractiveCleanGui( );
			result = get_id( cpg_ );

			if ( hidden ) cpg_->hide( );

			connect(cpg_, SIGNAL(interact(QVariant)), this, SLOT(handle_interact(QVariant)));

		} else if ( type == "clean2" ) {

			// <drs> need to ensure that this is not leaked...
			QtCleanPanelGui2 *cpg_ = viewer_->createInteractiveCleanGui2( );
			result = get_id( cpg_ );

			if ( hidden ) cpg_->hide( );

			connect(cpg_, SIGNAL(interact(QVariant)), this, SLOT(handle_interact(QVariant)));

		} 
		else {

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

		QtDisplayPanelGui *win = NULL;
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
			return error("could not find requested panel");
		}
		dpg->hide( );
		return QDBusVariant(QVariant(true));
	}

	QDBusVariant QtDBusViewerAdaptor::show( int panel ) {
		QtDisplayPanelGui *dpg = findpanel( panel, false );
		if ( ! dpg ) {
			return error("could not find requested panel");
		}
		dpg->show( );
		dpg->displayPanel( )->refresh( );
		return QDBusVariant(QVariant(true));
	}

	QDBusVariant QtDBusViewerAdaptor::close( int panel ) {
		QtDisplayPanelGui *dpg = findpanel( panel, false );
		if ( ! dpg ) {
			return panel == 0 ? QDBusVariant(QVariant(true)) : error("could not find requested panel");
		}

		QtDisplayPanelGui *win = erase_panel( panel );
		if ( win == 0 ) {
			return error("internal error closing panel");
		}

		win->closeMainPanel( );
		return QDBusVariant(QVariant(true));
	}

	QDBusVariant QtDBusViewerAdaptor::popup( const QString &what, int panel ) {
		QtDisplayPanelGui *dpg = findpanel( panel, false );
		if ( ! dpg ) {
			return panel == 0 ? QDBusVariant(QVariant(true)) : error("could not find requested panel");
		}
		if ( what == "open" ) {
			dpg->showDataManager();
		} else {
			return error("unknown popup-widow name, expected 'open'");
		}
		return QDBusVariant(QVariant(true));
	}


	QDBusVariant QtDBusViewerAdaptor::freeze( int panel ) {
		QtDisplayPanelGui *dpg = findpanel( panel, false );
		if ( ! dpg )
			error( "could not find requested panel id" );
		dpg->displayPanel()->hold();
		return QDBusVariant(QVariant(true));
	}

	QDBusVariant QtDBusViewerAdaptor::unfreeze( int panel ) {
		QtDisplayPanelGui *dpg = findpanel( panel, false );
		if ( ! dpg )
			error( "could not find requested panel id" );
		dpg->displayPanel()->release();
		return QDBusVariant(QVariant(true));
	}

	QDBusVariant QtDBusViewerAdaptor::channel( int num, int panel ) {
		QtDisplayPanelGui *dpg = findpanel( panel, false );
		if ( ! dpg ) {
			return error("could not find requested panel");
		}

		if ( num >= 0 ) {
			dpg->displayPanel()->goTo(num);
		}

		return QDBusVariant(QVariant(dpg->displayPanel()->frame()));
	}


	QDBusVariant QtDBusViewerAdaptor::zoom( int level, int panel ) {
		QtDisplayPanelGui *dpg = findpanel( panel, false );
		if ( ! dpg ) {
			return error("could not find requested panel");
		}

		if ( level == 0 )
			dpg->displayPanel()->unzoom( );
		else if ( level < 0 )
			dpg->displayPanel()->zoomOut( abs(level) );
		else
			dpg->displayPanel()->zoomIn( level );

		return QDBusVariant(QVariant(true));
	}

	QDBusVariant QtDBusViewerAdaptor::zoom( const QList<double> &blc, const QList<double> &trc, const QString &coordinates, int panel ) {
		QtDisplayPanelGui *dpg = findpanel( panel, false );
		if ( ! dpg ) {
			return error("could not find requested panel");
		}

		if ( blc.size() != 2 || trc.size() != 2 )
			return error("the zoom rectangle corners (blc and trc) must have two elements");

		Vector<Double> v_blc((uInt)2);
		Vector<Double> v_trc((uInt)2);
		for ( int i = 0; i < 2; ++i ) {
			v_blc[i] = blc[i];
			v_trc[i] = trc[i];
		}

		if ( coordinates == "pixel" ) {
			dpg->displayPanel()->zoom( v_blc, v_trc );
		} else if ( coordinates == "world" ) {
			Vector<Double> vp_blc((uInt)2);
			Vector<Double> vp_trc((uInt)2);
			bool OK = ( dpg->displayPanel( )->worldToLin( vp_blc, v_blc ) &&
			            dpg->displayPanel( )->worldToLin( vp_trc, v_trc ) );
			if ( OK == false )
				return error("zoom() conversion from world to pixel coordinates failed");

			dpg->displayPanel()->zoom( vp_blc, vp_trc );
		} else {
			return error("coordinates must be either 'world' or 'pixel'");
		}
		return QDBusVariant(QVariant(true));
	}


	QDBusVariant QtDBusViewerAdaptor::release( int panel ) {
		QtDisplayPanelGui *dpg = findpanel( panel, false );
		if ( ! dpg ) {
			return panel == 0 ? QDBusVariant(QVariant(true)) : error("could not find requested panel");
		}

		QtDisplayPanelGui *win = erase_panel( panel );
		if ( win == 0 ) {
			return error("internal error releasing panel");
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
			char *dev = strdup( device.toAscii().constData( ) );
			launch_lpr( printer_file, dev );
			free( dev );
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
			char path[MAXPATHLEN+1];
			sprintf( path, "%s", file.toAscii( ).constData( ) );
			adjusteps( eps_file_name, path, pmp.size(), viewport );
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

		char *ctype = strdup(type.toAscii().constData( ));
		if ( ! mp->save(file, ctype ) ) {
			free( ctype );
			delete mp;
			return false;
		}

		free( ctype );
		delete mp;
		return true;
	}

	QDBusVariant QtDBusViewerAdaptor::cwd( const QString &new_path ) {
		char p[MAXPATHLEN+1];
		if ( new_path != "" ) {
			struct stat buf;
			sprintf( p, "%s", new_path.toAscii().constData() );
			if ( stat(p,&buf) == 0 && S_ISDIR(buf.st_mode) )
				chdir(p);
			else if ( stat(p,&buf) != 0 )
				return error( new_path + " does not exist" );
			else if ( ! S_ISDIR(buf.st_mode) )
				return error( new_path + " is not a directory" );
			else
				return error( "cannot change to " + new_path );
		}
		getcwd(p,MAXPATHLEN+1);
		return QDBusVariant(QVariant(QString((const char*)p)));
	}

	QDBusVariant QtDBusViewerAdaptor::fileinfo( const QString &path ) {
		QMap<QString,QVariant> map;
		map.insert("path",QVariant(path));
		struct stat buf;
		if ( stat(path.toAscii().constData(),&buf) < 0 ) {
			// file (or dir) does not exist
			map.insert("type",QVariant("nonexistent"));
		} else {
			String datatype = viewer_->filetype(path.toStdString()).chars();
			map.insert("type",QVariant((const char*) datatype.c_str( )));
		}
		return QDBusVariant(QVariant(map));
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
		QVariant v(true);
		emit exiting(QDBusVariant(v));
	}

	QtDisplayData *QtDBusViewerAdaptor::finddata( int key ) {

		datamap::iterator iter = managed_datas.find( key );
		if ( iter != managed_datas.end( ) )
			return iter->second->data( );

		return 0;
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

		int index = QtId::get_id( );
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

		int index = QtId::get_id( );
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
