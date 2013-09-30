#include "SmoothPreferences.qo.h"
#include <display/QtPlotter/SmoothSpinBox.h>
#include <QSettings>
#include <QDebug>

namespace casa {

const QString SmoothPreferences::APPLICATION = "Profiler";
const QString SmoothPreferences::ORGANIZATION = "NRAO/CASA";
const QString SmoothPreferences::SMOOTH_METHOD = "Smoothing Method";
const QString SmoothPreferences::SMOOTH_RADIUS = "Smoothing Radius";


SmoothPreferences::SmoothPreferences(QWidget *parent)
    : QDialog(parent),
	SMOOTH_NONE( "None" ),
    SMOOTH_BOXCAR( "Boxcar" ),
    SMOOTH_HANNING( "Hanning"){

	ui.setupUi(this);
	setWindowTitle( "Smoothing");

	smoothSpinBox = new SmoothSpinBox(this);
	QLayout* spinLayout = new QHBoxLayout();
	spinLayout->addWidget( smoothSpinBox );
	ui.radiusHolder->setLayout(spinLayout);

	ui.smoothingCombo->addItem( SMOOTH_NONE );
	ui.smoothingCombo->addItem( SMOOTH_BOXCAR );
	ui.smoothingCombo->addItem( SMOOTH_HANNING );
	connect( ui.smoothingCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(smoothingMethodChanged()));

	initialize();
	reset();

	connect( ui.okButton, SIGNAL(clicked()), this, SLOT(smoothingAccepted()));
	connect( ui.cancelButton, SIGNAL(clicked()), this, SLOT(smoothingRejected()));
}

//-------------------------------------------------------------------
//                   Persistence
//-------------------------------------------------------------------

void SmoothPreferences::initialize(){
	//Only use the default values passed in if the user has not indicated
	//any preferences.
	QSettings settings( ORGANIZATION, APPLICATION );

	smoothMethod = settings.value( SMOOTH_METHOD, SMOOTH_NONE ).toString();
	smoothRadius = settings.value( SMOOTH_RADIUS, 3 ).toInt();
}

void SmoothPreferences::persist(){
	//Copy the settings from the UI into the internal variables
	smoothMethod = ui.smoothingCombo->currentText();
	smoothRadius = smoothSpinBox->value();

	//Persist the settings
	QSettings settings( ORGANIZATION, APPLICATION );
	settings.clear();
	settings.setValue( SMOOTH_METHOD, smoothMethod );
	settings.setValue( SMOOTH_RADIUS, QString::number( smoothRadius ));
}

void SmoothPreferences::reset(){
	int index = ui.smoothingCombo->findText( smoothMethod );
	if ( index >= 0 ){
		ui.smoothingCombo->setCurrentIndex( index );
	}
	smoothSpinBox->setValue( smoothRadius );
}

void SmoothPreferences::smoothingAccepted(){
	persist();
	emit smoothingChanged();
}

void SmoothPreferences::smoothingRejected(){
	reset();
	QDialog::close();
}

void SmoothPreferences::smoothingMethodChanged(){
	QString currentMethod = ui.smoothingCombo->currentText();
	if ( SMOOTH_NONE == currentMethod ){
		smoothSpinBox->setEnabled( false );
	}
	else {
		smoothSpinBox->setEnabled( true );
	}
}

//--------------------------------------------------------------------
//                   Smoothing
//--------------------------------------------------------------------


Vector<Float> SmoothPreferences::applySmoothing( Vector<Float> values) const {
	QString smoothMethod = ui.smoothingCombo->currentText();
	int valueCount = values.size();
	Vector<Float> result( valueCount );
	if ( SMOOTH_NONE == smoothMethod ){
		for ( int i = 0; i < valueCount; i++ ){
			result[i] = values[i];
		}
	}
	else {
		VectorKernel::KernelTypes kernelType = VectorKernel::BOXCAR;
		if ( SMOOTH_HANNING == smoothMethod ){
			kernelType = VectorKernel::HANNING;
		}
		int width = smoothSpinBox->value();
		result = doConvolve( values, width, kernelType );
	}
	return result;
}



Vector<Float> SmoothPreferences::doConvolve( const Vector<Float>& input,
		int baseKernelSize, VectorKernel::KernelTypes kernelType ) const {
	int inputSize = input.size();
	Vector<Float> result( inputSize );
	for ( int i = 0; i < inputSize; i++ ){
		//Make the kernel.
		int edgeDistance = qMin( i, inputSize - i - 1);
		if ( edgeDistance % 2 == 0 ){
			edgeDistance = edgeDistance - 1;
		}
		int actualKernelSize = qMin( baseKernelSize, edgeDistance + 2 );

		Vector<Float> kernelVector(actualKernelSize);
		if ( actualKernelSize > 1 ){
			float width = actualKernelSize;
			kernelVector = VectorKernel::make( kernelType, width, actualKernelSize, true );
		}
		else {
			kernelVector[0] = 1;
		}

		//Compute the value.
		result[i] = 0;
		int halfKernel = actualKernelSize /2;
		for ( int j = -halfKernel; j < halfKernel + 1; j++ ){
			result[i] = result[i] + kernelVector[halfKernel + j]*input[i+j];
		}
	}
	return result;
}

QString SmoothPreferences::toString() const {
	QString smoothDescription;
	if ( SMOOTH_NONE != smoothMethod ){
		smoothDescription.append( " ( Smoothing: "+ smoothMethod + " " + QString::number(smoothRadius)+" )");
	}
	return smoothDescription;
}

SmoothPreferences::~SmoothPreferences()
{

}
}
