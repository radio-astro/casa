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
#ifndef PIXEL_RANGE_DIALOG_QO_H_1
#define PIXEL_RANGE_DIALOG_QO_H_1

#include <display/Fit/PixelRangeDialog.ui.h>
#include <casa/aips.h>
#include <casa/cppconfig.h>
#include <QDialog>

#include <casa/cppconfig.h>

using namespace std;

namespace casa {

	template <class T> class ImageInterface;
	class BinPlotWidget;
	class ImageRegion;

//Displays a histogram that allows the user
//to set a include/exclude pixel range for the fit.

	class PixelRangeDialog : public QDialog {
		Q_OBJECT

	public:
		PixelRangeDialog(QWidget *parent = 0);
		void setImage( const shared_ptr<const ImageInterface<Float> > img );
		void setInterval( double minValue, double maxValue );
		void setImageMode( bool imageMode );
		void setChannelValue( int channel );
		bool setImageRegion( ImageRegion* imageRegion, int id );
		void deleteImageRegion( int id );
		void imageRegionSelected( int id );
		pair<double,double> getInterval() const;
		vector<float> getXValues() const;
		~PixelRangeDialog();

	protected:
		void keyPressEvent( QKeyEvent* event );



	private:
		PixelRangeDialog( const PixelRangeDialog& other );
		PixelRangeDialog operator=( const PixelRangeDialog& other );

		Ui::PixelRangeDialogClass ui;
		BinPlotWidget* plotWidget;
		int channelCount;
		int spectralIndex;
		int channelIndex;

	};




}
#endif // THRESHOLDINGBINPLOTDIALOG_H
