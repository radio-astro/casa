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
#ifndef PLOTHOLDER_QO_H
#define PLOTHOLDER_QO_H

#include <QtGui/QWidget>
#include <QMenu>
#include <qwt_plot.h>
#include <guitools/Feather/PlotHolder.ui.h>
#include <guitools/Feather/PreferencesColor.qo.h>
#include <casa/aipstype.h>
#include <casa/Arrays/Vector.h>
class QGridLayout;


namespace casa {

class FeatherPlotWidget;

class PlotHolder : public QWidget {
    Q_OBJECT

public:
    PlotHolder(QWidget *parent = 0);

    //Data related
    void setSingleDishWeight( const Vector<Float>& sDx, const Vector<Float>& sDxAmp,
    		const Vector<Float>& sDy, const Vector<Float>& sDyAmp );
    void setInterferometerWeight( const Vector<Float>& intx, const Vector<Float>& intxAmp,
    		const Vector<Float>& inty, const Vector<Float>& intyAmp );
    void setSingleDishData( const Vector<Float>& sDx, const Vector<Float>& sDxAmp,
    		const Vector<Float>& sDy, const Vector<Float>& sDyAmp );
    void setInterferometerData( const Vector<Float>& intx, const Vector<Float>& intxAmp,
    		const Vector<Float>& inty, const Vector<Float>& intyAmp );
    void setSingleDishDataOriginal( const Vector<Float>& sDx, const Vector<Float>& sDxAmp,
    		const Vector<Float>& sDy, const Vector<Float>& sDyAmp );
    void setInterferometerDataOriginal( const Vector<Float>& intx, const Vector<Float>& intxAmp,
    		const Vector<Float>& inty, const Vector<Float>& intyAmp );
    void updateScatterData( );
    void clearPlots();

    //Preference changes
    void setLineThickness( int thickness );
    void setDotSize( int dotSize );
    void setLegendVisibility( bool visible );
    void setDisplayScatterPlot( bool visible );
    void setDisplayOutputSlice( bool visible );
    void setDisplayOriginalSlice( bool visible );
    void setDisplayYGraphs( bool visible );
    void setColors( const QMap<PreferencesColor::FunctionColor,QColor>& colorMap,
    		const QColor& scatterPlotColor, const QColor& dishDiameterLineColor,
    		const QColor& zoomRectColor );
    void layoutPlotWidgets();

    //Dish changes
    void dishDiameterXChanged( double value );
    void dishDiameterYChanged( double value );

    ~PlotHolder();

signals:
	void dishDiameterChangedX( double newValue);
	void dishDiameterChangedY( double newValue);

public slots:
	//Left mouse modes
 	 void setRectangleZoomMode();
 	 void setDiameterSelectorMode();

private slots:
	void changePlotType();
	void changeZoom90();
	void zoomNeutral();
	void showContextMenu( const QPoint& pt );
	void rectangleZoomed( double minX, double maxX, double minY, double maxY );

private:
	enum Plots {  SLICE_X_ORIGINAL, SLICE_Y_ORIGINAL, SLICE_X, SLICE_Y, SCATTER_X, SCATTER_Y };
	void initializePlots();
	void initializeActions();
	void emptyLayout(QLayout* layout );
	void addPlotAxis( int rowIndex, int columnIndex, QGridLayout* layout, QwtPlot::Axis axis, int basePlotIndex );
    void addPlots( QGridLayout*& layout, bool displayYGraphs, int rowIndex, int basePlotIndex );
    void adjustLayout( bool scatterPlot );

    QList<FeatherPlotWidget*> plots;
    QAction plotTypeAction;
    QAction zoom90Action;
    QAction zoomNeutralAction;
    QMenu contextMenu;
    Ui::PlotHolderClass ui;
    QWidget* legendHolder;
    bool legendVisible;
    bool displayOutputSlice;
    bool displayOriginalSlice;
    bool displayScatter;
    bool tempScatterPlot;
    bool displayYGraphs;
};
}

#endif // PLOTHOLDER_QO_H
