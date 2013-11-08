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
#ifndef THRESHOLDINGBINPLOTDIALOG_QO_H_1
#define THRESHOLDINGBINPLOTDIALOG_QO_H_1

#include <display/QtPlotter/ThresholdingBinPlotDialog.ui.h>
#include <casa/aips.h>
#include <utility>
#include <QMainWindow>

#include <imageanalysis/ImageAnalysis/ImageTask.h>
#include <tr1/memory>

using namespace std;

namespace casa {

	template <class T> class ImageInterface;
	class BinPlotWidget;

//This dialog displays a histogram that allows the user
//to set a minimum/maximum moment collapse threshold graphically.
//Intensity is displayed on the x-axis.  The y-axis is a count of
//the number of pixels at each intensity.

	class ThresholdingBinPlotDialog : public QMainWindow {
		Q_OBJECT

	public:
		ThresholdingBinPlotDialog(QString yAxisUnits, QWidget *parent = 0);
		void setImage( const ImageTask::shCImFloat img );
		void setInterval( double minValue, double maxValue );
		pair<double,double> getInterval() const;
		~ThresholdingBinPlotDialog();

	signals:
		void accepted();

	private slots:
		void accept();
		void postStatusMessage( const QString& warning );

	private:
		Ui::ThresholdingBinPlotDialogClass ui;
		BinPlotWidget* plotWidget;

	};




}
#endif // THRESHOLDINGBINPLOTDIALOG_H
