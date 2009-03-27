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
//#include <casa/BasicSL/String.h>
//#include <display/DisplayEvents/MWCRTRegion.h>
#include <display/DisplayEvents/MWCCrosshairTool.h>

#include <display/Display/PanelDisplay.h>
#include <display/QtViewer/QtMouseTools.qo.h>

#include <graphics/X11/X_enter.h>
#include <QObject>
#include <QtGui>
#include <graphics/X11/X_exit.h>

namespace casa {

class QtMWCTool
{

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

class QtPolyTool: public QtPTRegion,  public QtMWCTool
{
  Q_OBJECT

public: 
  QtPolyTool(PanelDisplay* pd);
  virtual ~QtPolyTool() {}
 
public slots:
  void setCoordType(const String& t) { QtMWCTool::setCoordType(t);  }

signals:
  void wcNotify(const String c, 
           const Vector<Double> x, const Vector<Double> y);

protected:
  virtual void updateRegion();

};


class QtRectTool: public QtRTRegion,  public QtMWCTool
{
  Q_OBJECT

public: 
  QtRectTool(PanelDisplay* pd);
  virtual ~QtRectTool() {}

public slots:
  void setCoordType(const String& t) { QtMWCTool::setCoordType(t);  }

signals:
  void wcNotify(const String c, 
           const Vector<Double> x, const Vector<Double> y);

protected:
  virtual void updateRegion();
};

class QtCrossTool: public QObject, public QtMWCTool, public MWCCrosshairTool
{
  Q_OBJECT

public: 
  QtCrossTool();
  virtual ~QtCrossTool() {}
  virtual void crosshairReady(const String& evtype);

public slots:
  void setCoordType(const String& t) { QtMWCTool::setCoordType(t);  }

signals:
  void wcNotify(const String c, 
           const Vector<Double> x, const Vector<Double> y);


};


}

#endif

