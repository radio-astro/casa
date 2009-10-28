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
#include <casaqt/QtUtilities/QtDBusXmlApp.qo.h>
#include <casa/namespace.h>
#include <QVariantMap>

namespace casa {

    class QtViewer;
    class QtDisplayData;
    class QtDisplayPanel;
    class QtDisplayPanelGui;

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

	QDBusVariant start_interact( const QDBusVariant &input, int panel );
	QDBusVariant load( const QString &path, const QString &displaytype = "raster", int panel=0 );
	QDBusVariant reload( int panel_or_data );
	QDBusVariant unload( int data );
	QDBusVariant restore( const QString &path, bool new_window = true );
	QString cwd( const QString &new_path = "" );
	QDBusVariant panel( const QString &type="viewer", bool hidden=false  );
	QDBusVariant hide( int panel );
	QDBusVariant show( int panel );
	QDBusVariant close( int panel );
	// like "close()", but leaves the closing up to the user if the window is not hidden
	QDBusVariant release( int panel );
	//To change certain things so the panel 
	QDBusVariant setoptions( const QDBusVariant &input, int panel);
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
    
    signals:
	void interact( QDBusVariant );

    protected slots:
	void handle_interact( QVariant );

    private:

	class data_desc {
	public:
	    data_desc( int idx, const QString &pathx, const QString &typex,
		       QtDisplayData *ddx, QtDisplayPanel *dpx ) :
				id_(idx), path_(pathx), type_(typex), dd_(ddx), dp_(dpx) { }

	    data_desc( int idx ) : id_(idx), dd_(0) { }
	    data_desc( ) : id_(0), dd_(0) { }

	    int &id( ) { return id_; }
	    int id( ) const { return id_; }
	    QString &path( ) { return path_; }
	    const QString &path( ) const { return path_; }
	    QString &type( ) { return type_; }
	    const QString &type( ) const { return type_; }
	    QtDisplayData *&data( ) { return dd_; }
	    const QtDisplayData *data( ) const { return dd_; }
	    QtDisplayPanel *&panel( ) { return dp_; }
	    const QtDisplayPanel *panel( ) const { return dp_; }

	private:
	    int id_;
	    QString path_;
	    QString type_;
	    QtDisplayData *dd_;
	    QtDisplayPanel *dp_;

	    // QtDisplayData does not have a copy constructor...
	    // wonder if we'll need to copy our descriptor...
	    data_desc( const data_desc &other);
	    data_desc &operator=( const data_desc &);
	};


	class panel_desc {
	public:

	    panel_desc(QtDisplayPanel*p) : panel_(p) { }

	    std::list<int> &data( ) { return data_; }
	    const std::list<int> &data( ) const { return data_; }
	    QtDisplayPanel *&panel( ) { return panel_; }
	    const QtDisplayPanel *panel( ) const { return panel_; }

	private:
	    std::list<int> data_;
	    QtDisplayPanel *panel_;
	};


	QtViewer *viewer_;

	bool printps( QtDisplayPanel *panel, const QString &type, const QString &file, int dpi,
		      const QString &orientation, const QString &media );
	bool printraster( QtDisplayPanel *panel, const QString &type, const QString &file, double scale );
	void  adjusteps( const char *from, const char *to, const QSize &wcmax, const QRect &viewport );

	typedef std::map<int,panel_desc*> panelmap;
	typedef std::map<int, QtDisplayPanelGui*> mainwinmap;
	typedef std::map<int,data_desc*> datamap;
	datamap managed_datas;
	panelmap managed_panels;
	mainwinmap managed_windows;

	int get_id( QtDisplayPanel *, QtDisplayData *, const QString &path, const QString &type );
	int get_id( QtDisplayPanel* );
	QtDisplayPanel *findpanel( int );

    protected:
	void load_data( QtDisplayPanel *panel, int index );
	void unload_data( QtDisplayPanel *panel, int index, bool erase=true );
	void erase_panel( QtDisplayPanel *panel );
	void erase_data( int );

    };

}

#endif
