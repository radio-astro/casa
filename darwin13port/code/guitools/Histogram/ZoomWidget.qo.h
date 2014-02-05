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
#ifndef ZOOMWIDGET_QO_H
#define ZOOMWIDGET_QO_H

#include <QtGui/QWidget>
#include <guitools/Histogram/ZoomWidget.ui.h>

#include <tr1/memory.hpp>

namespace casa {

template <class T> class ImageInterface;
class ImageRegion;

class ZoomWidget : public QWidget
{
    Q_OBJECT

public:
    ZoomWidget(bool rangeControls, QWidget *parent = 0);
    void setImage( const std::tr1::shared_ptr<const ImageInterface<float> > image );
    void setRegion( ImageRegion* region );
    void copyState( ZoomWidget* other );

    ~ZoomWidget();

signals:
	void zoomRange( float min, float max );
	void zoomNeutral();
	void zoomGraphicalRange();
	void finished();


private slots:
	void percentageToggled( bool selected );
	void zoom();

private:
	void calculateRange( );
    Ui::ZoomWidgetClass ui;
    QStringList zoomList;
    std::tr1::shared_ptr<const ImageInterface<float> > image;
    ImageRegion* region;
};

}

#endif // ZOOMWIDGET_QO_H
