//# QtDBusViewerAdaptor.qo.h: provides viewer services via dbus
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
//# $Id$

#ifndef QTDBUSVIEWERADAPTOR_QO_H_
#define QTDBUSVIEWERADAPTOR_QO_H_

#include <map>
#include <set>
#include <casaqt/QtUtilities/QtDBusXmlApp.qo.h>
#include <casa/namespace.h>

namespace casa {

    class QtViewer;
    class QtDisplayData;
    class QtDisplayPanel;

    class QtDBusViewerAdaptor : public QDBusAbstractAdaptor, public QtDBusApp {
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "edu.nrao.casa.viewer")

    public:    
	// Connects to the DBus server using the dbusName() method with the current
	// process ID.  Returns whether the connection succeeded or not.
	bool connectToDBus();

	static const QString &name( );
	const QString &getName( ) const { return name( ); }

	// Constructor which takes the application.
	QtDBusViewerAdaptor( QtViewer * );
    
	// Destructor.
	~QtDBusViewerAdaptor();
        
    public slots:
	int load( const QString &path, const QString &displaytype = "raster", int panel=0 );
	int restore( const QString &path, bool new_window = true );
	QString cwd( const QString &new_path = "" );
	int panel( const QString &type="viewer" );
	// device:	file name or printer name
	// devicetype:	"file", "printer", or "ghostscript"
	// format:	"jpg", "pdf", "eps", "ps", "png", "xbm", "xpm", "ppm"
	//		[only used with devicetype == "file"] [extension on device, e.g. "outfile.pdf" overrides "format"]
	// scale:	size scale factor for raster output (e.g. jpg etc.)
	// dpi:		resolution of PS or EPS images
	// orientation: "portrait", "landscape"
	// media:	"letter" or "a4"
	bool output( const QString &device, const QString &devicetype = "file", int panel = 0,
		     double scale = 1.0, int dpi = 300, const QString &format = "jpg",
		     const QString &orientation = "portrait", const QString &media = "letter" );


	QStringList keyinfo( int );
	bool done( );
    
    private:
	QtViewer *viewer_;

	bool printps( QtDisplayPanel *panel, const QString &type, const QString &file, int dpi,
		      const QString &orientation, const QString &media );
	bool printraster( QtDisplayPanel *panel, const QString &type, const QString &file, double scale );
	void  adjusteps( const char *from, const char *to, const QSize &wcmax, const QRect &viewport );

	typedef std::map<int, QtDisplayPanel*> panelmap;
	typedef std::map<int, QtDisplayData*> datamap;
	datamap managed_datas;
	panelmap managed_panels;

	std::set<int> used_ids;
	int get_id( QtDisplayData * );
	int get_id( QtDisplayPanel* );
	int get_id( );
	QtDisplayPanel *findpanel( int );
    };

}

#endif
