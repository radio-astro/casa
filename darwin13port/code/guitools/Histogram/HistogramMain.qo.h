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
#ifndef HISTOGRAMMAIN_QO_H
#define HISTOGRAMMAIN_QO_H

#include <QtGui/QMainWindow>
#include <images/Images/PagedImage.h>
#include <casa/aipstype.h>
#include <guitools/Histogram/HistogramMain.ui.h>
#include <guitools/Histogram/ImageLoader.qo.h>
#include <guitools/Histogram/SaveHistogramWidget.qo.h>

#include <tr1/memory.hpp>

class QCloseEvent;


namespace casa {

class BinPlotWidget;
class ColorPreferences;
template <class T> class ImageInterface;
class ImageRegion;

/**
 * Used when the histogram needs to come up as a main window.
 */

class HistogramMain : public QMainWindow {
    Q_OBJECT

public:
    /**
     * showFileLoader:  true, for allowing the user to load image files
     * 		(as opposed to just setting them programmatically).
     * fitControls:  true, to allow the user to fit various curves to the
     *       histogram (Gaussian, Poisson, etc)
     * rangeControls: true, allows the user to specify a min/max value
     * plotModeControls: true, allows the user to specify whether to histogram
     * 		images, a selected region, or multiple regions.
     */
    HistogramMain(bool showFileLoader, bool fitControls, bool rangeControls,
    		bool plotModeControls, QWidget *parent);
    bool setImage(std::tr1::shared_ptr<const ImageInterface<Float> > img );
    bool setImageRegion( ImageRegion* imageRegion, int id );
    std::pair<double,double> getRange() const;
    void deleteImageRegion( int id );
    void imageRegionSelected( int id );
    void setChannelCount( int count );
    void setChannelValue( int value );
    void setDisplayPlotTitle( bool display );
    void setDisplayAxisTitles( bool display );
    void setPlotMode( int mode );
    ~HistogramMain();

signals:
	void rangeChanged();
	void closing();

protected:
	//Clean up images and regions because we are going down
	virtual void closeEvent(QCloseEvent* event);

private slots:
	void openFileLoader();
	void openHistogramSaver();
	void imageFileChanged();
	void openColorPreferences();
	void colorsChanged();
	void postStatusMessage( const QString& statusMsg );

private:
	HistogramMain( const HistogramMain& histMain );
	HistogramMain& operator=( const HistogramMain& histMain );
	bool generateImage( const QString& imagePath, std::tr1::shared_ptr<const ImageInterface<Float> > image );

    ImageLoader fileLoader;
    SaveHistogramWidget histogramSaver;
    ColorPreferences* preferencesColor;
    BinPlotWidget* plotWidget;
    LogIO logger;

    Ui::HistogramMainClass ui;

};
}

#endif // HISTOGRAMMAIN_QO_H
