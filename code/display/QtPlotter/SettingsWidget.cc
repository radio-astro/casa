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

#include <display/QtPlotter/SettingsWidget.qo.h>
#include <display/QtPlotter/SpecFitSettingsWidgetOptical.qo.h>
#include <display/QtPlotter/SpecFitSettingsWidgetRadio.qo.h>
#include <display/QtPlotter/MomentSettingsWidgetOptical.qo.h>
#include <display/QtPlotter/MomentSettingsWidgetRadio.qo.h>
#include <display/QtPlotter/ProfileTaskMonitor.h>
#include <display/QtPlotter/QtCanvas.qo.h>
#include <casa/Logging/LogIO.h>
#include <QtGui>
#include <QHBoxLayout>
#include <QDoubleValidator>

namespace casa {


	bool SettingsWidget::optical = true;

	SettingsWidget::SettingsWidget(QWidget *parent)
		: QWidget(parent), pixelCanvas( NULL ), taskHelper( NULL),
		  taskHelperOptical(NULL),taskHelperRadio( NULL),
		  specLineFitting( true ), newCollapseVals( True ) {
		ui.setupUi(this);
	}

	bool SettingsWidget::isOptical() {
		return optical;
	}

	void SettingsWidget::setOptical(bool opt ) {
		optical = opt;
	}

	void SettingsWidget::setTaskSpecLineFitting( bool specLineFit ) {
		bool resetNeeded = false;
		if ( specLineFit != specLineFitting ) {
			resetNeeded = true;
		}
		specLineFitting = specLineFit;
		reset( resetNeeded );
	}

	void SettingsWidget::reset( bool taskChanged ) {
		bool opticalChange = false;
		if ( taskHelper != NULL ) {
			opticalChange = optical != taskHelper->isOptical();
		}
		if ( taskHelper == NULL || opticalChange || taskChanged ) {

			//We are changing from an optical to a radio or vice versa
			//so we need to change the display to match or we need to
			//initialize the task helper
			QLayout* stackedLayout = layout();
			if ( stackedLayout == NULL ) {
				stackedLayout = new QStackedLayout( this );
				stackedLayout->setContentsMargins( 2, 1, 2, 1 );
			}
			if ( taskHelper == NULL || taskChanged ) {
				if ( specLineFitting ) {
					taskHelperOptical = new SpecFitSettingsWidgetOptical( this );
					taskHelperRadio = new SpecFitSettingsWidgetRadio( this );
				} else {
					taskHelperOptical = new MomentSettingsWidgetOptical( this );
					taskHelperRadio = new MomentSettingsWidgetRadio( this );
				}
				stackedLayout->addWidget( dynamic_cast<QWidget*>(taskHelperOptical) );
				stackedLayout->addWidget( dynamic_cast<QWidget*>(taskHelperRadio) );
				setLayout( stackedLayout );
			}
			if ( optical ) {
				dynamic_cast<QStackedLayout*>(stackedLayout)->setCurrentWidget( dynamic_cast<QWidget*>(taskHelperOptical) );
				taskHelper = taskHelperOptical;
			} else {
				dynamic_cast<QStackedLayout*>(stackedLayout)->setCurrentWidget( dynamic_cast<QWidget*>(taskHelperRadio) );
				taskHelper = taskHelperRadio;
			}
			if ( opticalChange ) {
				taskHelper -> setOptical( optical );
				taskHelper -> reset();
			}
		} else {
			//Only the data has changed (not the type of fitter).
			taskHelper->reset();
		}
		newCollapseVals= true;
	}

	void SettingsWidget::setCollapseVals(const Vector<Float> &spcVals) {

		if ( newCollapseVals ) {
			taskHelper ->setCollapseVals( spcVals );
			newCollapseVals = False;
		}
	}

	void SettingsWidget::pixelsChanged( int pixX, int pixY ) {
		taskHelper->pixelsChanged( pixX, pixY );
	}

	void SettingsWidget::setCanvas( QtCanvas* pCanvas ) {
		//We store a copy in this class so if the specFitter changes
		//we can still provide it with the canvas for drawing and we
		//can respond to range change requests without reconnecting if
		//the specFitter changes.
		pixelCanvas = pCanvas;
		connect(pixelCanvas, SIGNAL(xRangeChanged(double, double)), this, SLOT(setFitRange(double,double)));

		//We give the current spec fitter a copy of the canvas for drawing purposes.
		if ( taskHelper != NULL ) {
			taskHelperOptical->setCanvas( pixelCanvas );
			taskHelperRadio->setCanvas( pixelCanvas );
		}
	}

	void SettingsWidget::clear() {
		if ( taskHelper != NULL ) {
			taskHelper->clear();
		}
	}

	void SettingsWidget::setLogger( LogIO* log ) {
		logger = log;
		if ( taskHelper != NULL ) {
			taskHelperOptical->setLogger( log );
			taskHelperRadio->setLogger( log );
		}
	}

	void SettingsWidget::setTaskMonitor( ProfileTaskMonitor* profileTaskMonitor ) {
		taskMonitor = profileTaskMonitor;
		if ( taskHelper != NULL ) {
			taskHelperOptical ->setTaskMonitor( profileTaskMonitor );
			taskHelperRadio->setTaskMonitor( profileTaskMonitor );
		}
	}

	void SettingsWidget::setUnits( QString units ) {
		if ( taskHelper != NULL ) {
			taskHelper -> setUnits( units );
		}
	}

	void SettingsWidget::setDisplayYUnits( const QString& units ) {
		if ( taskHelper != NULL ) {
			taskHelperRadio -> setDisplayYUnits( units );
		}
	}

	void SettingsWidget::setImageYUnits( const QString& units ) {
		if ( taskHelper != NULL ) {
			taskHelperRadio -> setImageYUnits( units );
		}
	}

	void SettingsWidget::setCurveName( const QString& curveName ) {
		if ( taskHelper != NULL ) {
			taskHelperRadio -> setCurveName( curveName );
		}
	}

	void SettingsWidget::addCurveName( const QString& curveName ) {
		if ( taskHelper != NULL ) {
			taskHelperRadio->addCurveName( curveName );
		}
	}

	void SettingsWidget::setRange( double start, double end ) {
		if ( taskHelper != NULL ) {
			taskHelper ->setRange(start, end );
		}
	}

	void SettingsWidget::setFitRange( double start, double end ) {
		setRange( start, end );
	}


	SettingsWidget::~SettingsWidget() {
		if ( taskHelperRadio != NULL ) {
			delete taskHelperRadio;
		}
		if ( taskHelperOptical  != NULL ) {
			delete taskHelperOptical;
		}
	}
} // end namespace
