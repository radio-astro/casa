//# QtDBusPlotSvrAdaptor.qo.h: provides viewer services via dbus
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

#ifndef QTDBUSPLOTSVRADAPTOR_QO_H_
#define QTDBUSPLOTSVRADAPTOR_QO_H_

#include <map>
#include <casaqt/QtUtilities/QtDBusXmlApp.qo.h>
#include <QVariantMap>
#include <QString>

class QwtPlotCurve;

namespace casa {

    class QtPlotServer;
    class QtPlotSvrPanel;

    class QtDBusPlotSvrAdaptor : public QDBusAbstractAdaptor, public QtDBusApp {
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "edu.nrao.casa.plotserver")
    public:

	// Connects to the DBus server using the dbusName() method with the current
	// process ID.  Returns whether the connection succeeded or not.
	bool connectToDBus();

	// name used to initialize connection to dbus
	static const QString &name( );
	const QString &getName( ) const { return name( ); }

        QtDBusPlotSvrAdaptor( QtPlotServer * );
	~QtDBusPlotSvrAdaptor( );

    public slots:

	QDBusVariant panel( const QString &title, const QString &xlabel="", const QString &ylabel="", const QString &window_title="",
			    const QString &legend="bottom", bool hidden=false  );
	QStringList colors( );
	QStringList symbols( );
	QDBusVariant line( const QList<double> &x, const QList<double> &y, const QString &color="black",
			   const QString &label="", int panel=0 );
	QDBusVariant scatter( const QList<double> &x, const QList<double> &y, const QString &color="black",
			      const QString &label="", const QString &symbol="", int symbol_size=-1, int dot_size=-1, int panel=0 );
	QDBusVariant raster( const QList<double> &matrix, int sizex, int sizey, int panel=0 );

	QDBusVariant erase( int data=0 );
	QDBusVariant close( int panel=0 );
	QDBusVariant release( int panel=0 );

	QDBusVariant hide( int panel );
	QDBusVariant show( int panel );

	bool done( );

    private:

	class panel_desc {
	public:

	    panel_desc(QtPlotSvrPanel*p) : panel_(p) { }

	    std::list<int> &data( ) { return data_; }
	    const std::list<int> &data( ) const { return data_; }
	    QtPlotSvrPanel *&panel( ) { return panel_; }
	    const QtPlotSvrPanel *panel( ) const { return panel_; }

	private:
	    std::list<int> data_;
	    QtPlotSvrPanel *panel_;
	};


	class data_desc {
	public:

	    data_desc( int index, QtPlotSvrPanel *panel, QwtPlotCurve *data ) : id_(index), data_(data), panel_(panel) { }

	    int &id( ) { return id_; }
	    int id( ) const { return id_; }
	    QwtPlotCurve *&data( ) { return data_; }
	    const QwtPlotCurve *data( ) const { return data_; }
	    QtPlotSvrPanel *&panel( ) { return panel_; }
	    const QtPlotSvrPanel *panel( ) const { return panel_; }

	private:
	    int id_;
	    QwtPlotCurve *data_;
	    QtPlotSvrPanel *panel_;

	    // QtDisplayData does not have a copy constructor...
	    // wonder if we'll need to copy our descriptor...
	    data_desc( const data_desc &other);
	    data_desc &operator=( const data_desc &);
	};

	int get_id( QtPlotSvrPanel *panel );
	int get_id( QtPlotSvrPanel *panel, QwtPlotCurve *data );

	void close_everything( );
	void release_everything( );

	QtPlotServer *server;
	typedef std::map<int,panel_desc*> panelmap;
	typedef std::map<int,data_desc*> datamap;
	panelmap managed_panels;
	datamap managed_datas;
        void release( panelmap::iterator & );
  };

}
#endif
