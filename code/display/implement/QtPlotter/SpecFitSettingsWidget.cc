#include <display/QtPlotter/SpecFitSettingsWidget.qo.h>
#include <display/QtPlotter/SpecFitSettingsWidgetOptical.qo.h>
#include <display/QtPlotter/SpecFitSettingsWidgetRadio.qo.h>
#include <display/QtPlotter/SpecFitter.h>
#include <display/QtPlotter/SpecFitMonitor.h>
#include <display/QtPlotter/QtCanvas.qo.h>
#include <casa/Logging/LogIO.h>
#include <QtGui>
#include <QHBoxLayout>

namespace casa {


SpecFitSettingsWidget::SpecFitSettingsWidget(QWidget *parent)
    : QWidget(parent), pixelCanvas( NULL ), specFitter( NULL),
      opticalFitter( true )
{
	ui.setupUi(this);
}

bool SpecFitSettingsWidget::isOptical() const {
	return opticalFitter;
}

void SpecFitSettingsWidget::resetSpectralFitter( bool optical ){
	if ( optical != opticalFitter || specFitter == NULL ){
		//We are changing from an optical to a radio or vice versa
		//so we need to change the display to match or we need to
		//initialize the specFitter;
		opticalFitter = optical;
		QLayout* boxLayout = layout();
		if ( boxLayout != NULL ){
			boxLayout->removeWidget( dynamic_cast<QWidget*>(specFitter) );
			if ( specFitter != NULL ){
				delete specFitter;
			}
			delete boxLayout;
		}

		QHBoxLayout* hboxLayout = new QHBoxLayout( this );
		hboxLayout->setMargin( 0 );
		if ( optical ){
			specFitter = new SpecFitSettingsWidgetOptical( this );

		}
		else {
			specFitter = new SpecFitSettingsWidgetRadio( this );
		}

		//Reset the specFitter tools
		if ( pixelCanvas != NULL ){
			specFitter -> setCanvas( pixelCanvas );
		}
		if ( logger != NULL ){
			specFitter -> setLogger( logger );
		}
		if ( specFitMonitor != NULL ){
			specFitter -> setSpecFitMonitor( specFitMonitor );
		}

		hboxLayout->addWidget( dynamic_cast<QWidget*>(specFitter));
		this->setLayout( hboxLayout );
		this->update();
	}
	else {
		//Only the data has changed (not the type of fitter).
		specFitter->resetSpectralFitter();
	}
}


void SpecFitSettingsWidget::setCanvas( QtCanvas* pCanvas ){
	//We store a copy in this class so if the specFitter changes
	//we can still provide it with the canvas for drawing and we
	//can respond to range change requests without reconnecting if
	//the specFitter changes.
	pixelCanvas = pCanvas;
	connect(pixelCanvas, SIGNAL(xRangeChanged(float, float)), this, SLOT(setFitRange(float,float)));

	//We give the current spec fitter a copy of the canvas for drawing purposes.
	if ( specFitter != NULL ){
		specFitter->setCanvas( pixelCanvas );
	}
}


void SpecFitSettingsWidget::setLogger( LogIO* log ){
	logger = log;
	if ( specFitter != NULL ){
		specFitter->setLogger( log );
	}
}

void SpecFitSettingsWidget::setFitMonitor( SpecFitMonitor* fitMonitor ){
	specFitMonitor = fitMonitor;
	if ( specFitter != NULL ){
		specFitter ->setSpecFitMonitor( specFitMonitor );
	}
}

void SpecFitSettingsWidget::setUnits( QString units ){
	if ( specFitter != NULL ){
		specFitter -> setUnits( units );
	}
}

void SpecFitSettingsWidget::setRange( float start, float end ){
	if ( specFitter != NULL ){
		specFitter ->setRange(start, end );
	}
}

void SpecFitSettingsWidget::setFitRange( float start, float end ){
	setRange( start, end );
}


SpecFitSettingsWidget::~SpecFitSettingsWidget()
{
	delete specFitter;
}
} // end namespace
