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
#include "MomentSettingsWidgetOptical.qo.h"
#include <display/Display/DisplayCoordinateSystem.h>
#include <display/QtPlotter/ProfileTaskMonitor.h>
#include <QDir>

namespace casa {

	MomentSettingsWidgetOptical::MomentSettingsWidgetOptical(QWidget *parent)
		: QWidget(parent), collapser(0) {
		ui.setupUi(this);

		connect(ui.collapseType, SIGNAL(currentIndexChanged(const QString &)),
		        this, SLOT(changeCollapseType(const QString &)));
		connect(ui.collapseError, SIGNAL(currentIndexChanged(const QString &)),
		        this, SLOT(changeCollapseError(const QString &)));


		QValidator *validator = new QDoubleValidator(-1.0e+32, 1.0e+32,10,this);
		ui.startValue->setValidator(validator);
		ui.startValue->setMaximumWidth(100);
		ui.endValue->setValidator(validator);
		ui.endValue->setMaximumWidth(100);
		ui.fromLabel->setMinimumWidth(40);
		ui.fromLabel->setMargin(3);
		ui.fromLabel->setAlignment((Qt::Alignment)(Qt::AlignRight|Qt::AlignVCenter));
		ui.toLabel->setMinimumWidth(30);
		ui.toLabel->setMargin(3);
		ui.toLabel->setAlignment((Qt::Alignment)(Qt::AlignRight|Qt::AlignVCenter));
		ui.collapseUnits->setMargin(3);

		connect(ui.collapse, SIGNAL(clicked()), this, SLOT(collapseImage()));
	}

	void MomentSettingsWidgetOptical::clear() {
		ui.startValue->setText("");
		ui.endValue->setText("");
	}

	void MomentSettingsWidgetOptical::reset() {
		if ( collapser ) {
			delete collapser;
		}
		try {
			if ( taskMonitor != NULL ) {
				//ImageInterface<Float>* img = const_cast<ImageInterface <Float>* >(taskMonitor->getImage().get());
				//shared_ptr<ImageInterface<Float> > img(std::tr1::const_pointer_cast<ImageInterface<Float> >(taskMonitor->getImage()));
				shared_ptr<const ImageInterface<Float> > img = taskMonitor->getImage();
				collapser = new SpectralCollapser(img, String(QDir::tempPath().toStdString()));
				changeCollapseType();
				changeCollapseError();
			}

		} catch (AipsError x) {
			String message = "Error when re-setting the profiler:\n" + x.getMesg();
			logWarning( message, true );
		}
	}

	void MomentSettingsWidgetOptical::collapseImage() {
		*logger << LogOrigin("QtProfile", "doImgCollapse");

		// get the values
		QString startStr = ui.startValue->text();
		QString endStr   = ui.endValue->text();
		String msg;

		// make sure the input is reasonable

		if (startStr.isEmpty()) {
			msg = String("No start value specified!");
			logWarning( msg, true );
			postStatus(msg, true);
			return;
		}
		if (endStr.isEmpty()) {
			msg = String("No end value specified!");
			logWarning(msg, true);
			postStatus(msg, true);
			return;
		}

		int pos=0;
		if (ui.startValue->validator()->validate(startStr, pos) != QValidator::Acceptable) {
			String startString(startStr.toStdString());
			msg = String("Start value not correct: ") + startString;
			logWarning( msg, true );
			postStatus(msg, true);
			return;
		}
		if (ui.endValue->validator()->validate(endStr, pos) != QValidator::Acceptable) {
			String endString(endStr.toStdString());
			msg = String("Start value not correct: ") + endString;
			logWarning( msg, true );
			postStatus(msg, true);
			return;
		}

		// convert input values to Float
		Float startVal=(Float)startStr.toFloat();
		Float endVal  =(Float)endStr.toFloat();

		String outname;
		Vector<float> z_xval = taskMonitor->getXValues();
		String xaxisUnit = taskMonitor->getXAxisUnit();
		Bool ok = collapser->collapse(z_xval, startVal, endVal, xaxisUnit, itsCollapseType, itsCollapseError, outname, msg);

		if (ok) {
			*logger << msg << LogIO::POST;
			postStatus(msg, true);
		} else {
			msg = String("Problem collapsing the image: ") + msg;
			logWarning( msg, true );
			postStatus( msg, true );
			return;
		}
		taskMonitor->imageCollapsed(outname, "image", "raster", True, True);

		return;
	}

	void MomentSettingsWidgetOptical::setUnits( QString unitStr ) {
		ui.collapseUnits->setText( unitStr );
	}

	void MomentSettingsWidgetOptical::setRange( double xmin, double xmax ) {
		if (xmax < xmin) {
			ui.startValue->clear();
			ui.endValue->clear();
		} else {
			QString startStr;
			QString endStr;
			startStr.setNum(xmin);
			endStr.setNum(xmax);
			ui.startValue->setText(startStr);
			ui.endValue->setText(endStr);
		}

	}

	void MomentSettingsWidgetOptical::changeCollapseType(QString  text) {
		bool switchError(false);
		// if no type given means initialization
		if (text.size()<1) {
			switchError=true;

			// read and set a type from the rc-file
			text = taskMonitor->read(".collapse.type");
			if (text.size()>0) {
				int index = ui.collapseType->findText(text);
				if (index > -1)
					ui.collapseType->setCurrentIndex(index);
			} else {
				// just use what's there
				text=ui.collapseType->currentText();
			}
		}

		//set the class data
		SpectralCollapser::stringToCollapseType(String(text.toStdString ()), itsCollapseType);

		// get the coo-sys
		ImageInterface<Float> * image = const_cast<ImageInterface <Float>* > (taskMonitor->getImage().get());
		DisplayCoordinateSystem cSys = image->coordinates();

		// depending on the collapse type,
		// insert the allowed error types
		switch (itsCollapseType) {
		case SpectralCollapser::PMEAN:
			if (ui.collapseError->findText("rmse") < 0)
				ui.collapseError->insertItem(1, "rmse");
			if (cSys.qualityAxisNumber() > -1 && ui.collapseError->findText("propagated") < 0)
				ui.collapseError->insertItem(1, "propagated");
			if (cSys.qualityAxisNumber() < 0 && ui.collapseError->findText("propagated") > -1)
				ui.collapseError->removeItem(ui.collapseError->findText("propagated"));
			break;
		case SpectralCollapser::PMEDIAN:
			if (ui.collapseError->findText("rmse") < 0)
				ui.collapseError->insertItem(1, "rmse");
			if (ui.collapseError->findText("propagated") > -1)
				ui.collapseError->removeItem(ui.collapseError->findText("propagated"));
			break;
		case SpectralCollapser::PSUM:
			if (ui.collapseError->findText("rmse") > -1)
				ui.collapseError->removeItem(ui.collapseError->findText("rmse"));
			if (cSys.qualityAxisNumber() > -1 && ui.collapseError->findText("propagated") < 0)
				ui.collapseError->insertItem(1, "propagated");
			if (cSys.qualityAxisNumber() < 0 && ui.collapseError->findText("propagated") > -1)
				ui.collapseError->removeItem(ui.collapseError->findText("propagated"));
			break;
		case SpectralCollapser::CUNKNOWN:
			break;
		}

		// store the collapse type in the rc-file
		taskMonitor->persist(".collapse.type", text );


		// if initialization
		if (switchError) {

			// read the error type from the rc-file
			QString error = taskMonitor->read(".collerror.type");
			if (error.size()>0) {

				// if the error type does exist, which means
				// if it is allowed, set it
				int index = ui.collapseError->findText(error);
				if (index > -1) {
					ui.collapseError->setCurrentIndex(index);
					SpectralCollapser::stringToCollapseError(String(error.toStdString ()), itsCollapseError);
				}
			}
		}

	}

	void MomentSettingsWidgetOptical::changeCollapseError(QString text) {
		if (text.size()<1)
			text=ui.collapseError->currentText();
		taskMonitor->persist( ".collerror.type", text );
		SpectralCollapser::stringToCollapseError(String(text.toStdString ()), itsCollapseError);
	}

	MomentSettingsWidgetOptical::~MomentSettingsWidgetOptical() {

	}
}
