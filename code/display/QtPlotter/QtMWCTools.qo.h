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


#ifndef QTMWCTOOLS_H
#define QTMWCTOOLS_H

#include <casa/aips.h>

#include <display/Display/PanelDisplay.h>
#include <display/QtViewer/QtMouseTools.qo.h>
#include <display/region/RegionSourceFactory.h>

#include <graphics/X11/X_enter.h>
#include <QObject>
#include <QtGui>
#include <graphics/X11/X_exit.h>

namespace casa {

	enum ProfileType {
	    UNKNPROF,
	    SINGPROF,
	    RECTPROF,
	    ELLPROF,
	    POLYPROF,
	    POLYLINEPROF,
	    PVPROF
	};

	class QtMWCTool {

	public:
		QtMWCTool(): coordType("world") {}
		virtual ~QtMWCTool() {}
	protected:
		String coordType;

	public:
		virtual void setCoordType(const String& t) {
			if (t == "world") coordType = t;
			else coordType = "pixel";
			// cout << "Object=" << objectName().toStdString()
			//      << " setCoordType: coordType=" << coordType
			//      << " t=" << t << endl;
		}
		//String getCoordType() {return coordType;}

	};

	class QtPolyTool: public QtPTRegion,  public QtMWCTool {
		Q_OBJECT

	public:
		QtPolyTool(viewer::RegionSourceFactory *rf, PanelDisplay* pd);
		virtual ~QtPolyTool() {}

	public slots:
		void setCoordType(const String& t);

	signals:
		void wcNotify( const String c,
		               const Vector<Double> px, const Vector<Double> py,
		               const Vector<Double> wx, const Vector<Double> wy,
		               const ProfileType ptype);

	protected:
		virtual void updateRegion();

	};

	class QtPolylineTool: public QtPolylineToolRegion,  public QtMWCTool {
		Q_OBJECT

	public:
		QtPolylineTool(viewer::RegionSourceFactory *rf, PanelDisplay* pd);
		virtual ~QtPolylineTool() {}

	public slots:
		void setCoordType(const String& t);

	signals:
		void wcNotify( const String c,
		               const Vector<Double> px, const Vector<Double> py,
		               const Vector<Double> wx, const Vector<Double> wy,
		               const ProfileType ptype);

	protected:
		virtual void updateRegion();

	};

	class QtEllipseTool: public QtELRegion,  public QtMWCTool {
		Q_OBJECT

	public:
		QtEllipseTool(viewer::RegionSourceFactory *rf, PanelDisplay *pd);
		virtual ~QtEllipseTool() {}

	public slots:
		void setCoordType(const String& t);

	signals:
		void wcNotify( const String c,
		               const Vector<Double> px, const Vector<Double> py,
		               const Vector<Double> wx, const Vector<Double> wy,
		               const ProfileType ptype);

	protected:
		virtual void updateRegion();
	};


	class QtRectTool: public QtRTRegion,  public QtMWCTool {
		Q_OBJECT

	public:
		QtRectTool(viewer::RegionSourceFactory *rf, PanelDisplay* pd);
		virtual ~QtRectTool() {}

	public slots:
		void setCoordType(const String& t);

	signals:
		void wcNotify( const String c,
		               const Vector<Double> px, const Vector<Double> py,
		               const Vector<Double> wx, const Vector<Double> wy,
		               const ProfileType ptype);

	protected:
		virtual void updateRegion();
	};

	class QtCrossTool: public QtPointRegion, public QtMWCTool {
		Q_OBJECT

	public:
		QtCrossTool(viewer::RegionSourceFactory *rf, PanelDisplay* pd);
		virtual ~QtCrossTool() {}
		/* virtual void crosshairReady(const String& evtype); */

	public slots:
		void setCoordType(const String& t);

	signals:
		void wcNotify( const String c,
		               const Vector<Double> px, const Vector<Double> py,
		               const Vector<Double> wx, const Vector<Double> wy,
		               const ProfileType ptype);

	protected:
		virtual void updateRegion();
	};

	class QtPVTool: public QtPVToolRegion,  public QtMWCTool {
		Q_OBJECT

	public:
		QtPVTool(viewer::RegionSourceFactory *rf, PanelDisplay* pd);
		virtual ~QtPVTool() {}

	public slots:
		void setCoordType(const String& t);

	signals:
		void wcNotify( const String c,
		               const Vector<Double> px, const Vector<Double> py,
		               const Vector<Double> wx, const Vector<Double> wy,
		               const ProfileType ptype);

	protected:
		virtual void updateRegion();

	};

}

#endif

