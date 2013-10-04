#ifndef RESIDUALHISTOGRAMDIALOG_QO_H
#define RESIDUALHISTOGRAMDIALOG_QO_H

//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000
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
#include <QtGui/QDialog>
#include <display/Fit/ResidualHistogramDialog.ui.h>
#include <casa/aips.h>
#include <casa/BasicSL/String.h>

#include <imageanalysis/ImageAnalysis/ImageTask.h>
#include <tr1/memory.hpp>

namespace casa {

	template <class T> class ImageInterface;
	class BinPlotWidget;

	class ResidualHistogramDialog : public QDialog {
		Q_OBJECT

	public:
		ResidualHistogramDialog(QWidget *parent = 0);
		bool setImage( const String& path );
		~ResidualHistogramDialog();

	private:
		ResidualHistogramDialog( const ResidualHistogramDialog& other );
		ResidualHistogramDialog operator=( const ResidualHistogramDialog& other );
		BinPlotWidget* plotWidget;
		ImageTask::shCImFloat residualImage;
		Ui::ResidualHistogramDialogClass ui;
	};
}

#endif // RESIDUALHISTOGRAMDIALOG_QO_H
