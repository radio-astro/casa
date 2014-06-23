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

#ifndef COLORHISTOGRAM_QO_H
#define COLORHISTOGRAM_QO_H

#include <QtGui/QMainWindow>
#include <casa/aipstype.h>
#include <casa/Arrays/Vector.h>
#include <display/QtViewer/ColorHistogram.ui.h>

namespace casa {

	class BinPlotWidget;
	class QtDisplayData;
	class ColorHistogramScale;
	class WCPowerScaleHandler;
	class ColorTransferWidget;

	/**
	 * Displays a histogram that allows the user to set the
	 * color scale for an image.
	 */

	class ColorHistogram : public QMainWindow {
		Q_OBJECT

	public:
		ColorHistogram(QWidget *parent = 0);
		void setDisplayData( QtDisplayData* img );
		~ColorHistogram();

	private slots:
		void resetIntensityRange();
		void acceptRange();
		void cancelRange();
		void colorsChanged();
		void invertColorMap( int invert );
		void powerCyclesChangedSlider(int value );
		void powerCyclesChangedLineEdit( const QString& str );
		void histogramColorModeChanged( bool useColors );
		void logScaleChanged ( int logScaleAmount );

	private:
		void updateColorMap( bool invertChanged = false);
		void resetColorLookups();
		void resetPowerCycles( float powerCycles);
		Vector<uInt> computeScaledIntensities(const std::vector<float>& intensities );
		ColorHistogram( const ColorHistogram& other );
		ColorHistogram operator=( const ColorHistogram& other );

		QtDisplayData* displayData;
		Ui::ColorHistogramClass ui;
		BinPlotWidget* histogram;
		WCPowerScaleHandler* powerScaler;
		ColorHistogramScale* colorScale;
		ColorTransferWidget* colorTransferWidget;
		double minIntensity;
		double maxIntensity;
		int logScale;
		const int COLOR_MAX;
		const int SLIDER_MIN;
		const int SLIDER_MAX;
		int SCALE_LIMIT;
	};

}
#endif // COLORHISTOGRAM_QO_H
