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


#ifndef QTOLDMWCTOOLS_H
#define QTOLDMWCTOOLS_H

#include <casa/aips.h>
//#include <casa/BasicSL/String.h>
//#include <display/DisplayEvents/MWCRTRegion.h>
#include <display/DisplayEvents/MWCCrosshairTool.h>

#include <display/Display/PanelDisplay.h>
#include <display/QtViewer/QtOldMouseTools.qo.h>
#include <display/QtPlotter/QtMWCTools.qo.h>

#include <graphics/X11/X_enter.h>
#include <QObject>
#include <QtGui>
#include <graphics/X11/X_exit.h>

namespace casa {

	class QtOldMWCTool {

	public:
		QtOldMWCTool(): coordType("world") {}
		virtual ~QtOldMWCTool() {}
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

	class QtOldPolyTool: public QtOldPTRegion,  public QtOldMWCTool {
		Q_OBJECT

	public:
		QtOldPolyTool(PanelDisplay* pd);
		virtual ~QtOldPolyTool() {}

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


	class QtOldEllipseTool: public QtOldELRegion,  public QtOldMWCTool {
		Q_OBJECT

	public:
		QtOldEllipseTool(PanelDisplay* pd);
		virtual ~QtOldEllipseTool() {}

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


	class QtOldRectTool: public QtOldRTRegion,  public QtOldMWCTool {
		Q_OBJECT

	public:
		QtOldRectTool(PanelDisplay* pd);
		virtual ~QtOldRectTool() {}

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

	class QtOldCrossTool: public QObject, public QtOldMWCTool, public MWCCrosshairTool {
		Q_OBJECT

	public:
		QtOldCrossTool();
		virtual ~QtOldCrossTool() {}
		virtual void crosshairReady(const String& evtype);

	public slots:
		void setCoordType(const String& t);

	signals:
		void wcNotify( const String c,
		               const Vector<Double> px, const Vector<Double> py,
		               const Vector<Double> wx, const Vector<Double> wy,
		               const ProfileType ptype);


	};


}

#endif

