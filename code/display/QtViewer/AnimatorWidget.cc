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
#include "AnimatorWidget.qo.h"
#include <QDebug>
namespace casa {

AnimatorWidget::AnimatorWidget(QWidget *parent)
    : QWidget(parent) {
	ui.setupUi(this);

	ui.revTB_ ->setCheckable(true);
	ui.stopTB_->setCheckable(true);
	ui.playTB_->setCheckable(true);

	ui.frameEdit_->setValidator(new QIntValidator(ui.frameEdit_));

	connect(ui.frameSlider_, SIGNAL(valueChanged(int)), this, SIGNAL(goTo(int)));
	connect(ui.frameEdit_, SIGNAL(editingFinished()), this, SLOT(frameNumberEdited()));
	connect(ui.rateEdit_,  SIGNAL(valueChanged(int)), this, SIGNAL(setRate(int)));
	connect(ui.toStartTB_, SIGNAL(clicked()), this, SIGNAL(toStart()));
	connect(ui.revStepTB_, SIGNAL(clicked()), this, SIGNAL(revStep()));
	connect(ui.revTB_, SIGNAL(clicked()), this, SIGNAL(revPlay()));
	connect(ui.stopTB_, SIGNAL(clicked()), this, SIGNAL(stop()));
	connect(ui.playTB_, SIGNAL(clicked()), this, SIGNAL(fwdPlay()));
	connect(ui.fwdStep_, SIGNAL(clicked()), this, SIGNAL(fwdStep()));
	connect(ui.toEndTB_, SIGNAL(clicked()), this, SIGNAL(toEnd()));

	rateNotSet = true;

	//Start and end limits
	ui.endSpinBox->setMinimum( 0 );
	ui.startSpinBox->setMinimum( 0 );
	connect( ui.startSpinBox, SIGNAL(valueChanged(int)), this, SLOT(movieLimitLowerChanged(int)));
	connect( ui.endSpinBox,   SIGNAL(valueChanged(int)), this, SLOT(movieLimitUpperChanged(int)));
	connect( ui.frameSlider_, SIGNAL(actionTriggered(int)), this, SLOT(sliderControl(int)));

	//Jump
	connect( ui.endToEndCheckBox, SIGNAL(toggled(bool)), this, SLOT(endToEndMode(bool)));
}



void AnimatorWidget::blockSignals( bool block ){
	ui.frameSlider_->blockSignals(block);
	ui.frameEdit_->blockSignals( block );
	ui.revTB_->blockSignals( block );
	ui.stopTB_->blockSignals( block );
	ui.playTB_->blockSignals( block );
}

void AnimatorWidget::setFrameInformation( int frm, int len ){
	frameCount = len;
	blockSignals( true );
	//If we are changing the number of frames, we need to reset
	//the properties that depend on the number of frames.
	int oldMax = ui.endSpinBox->maximum();
	if ( oldMax + 1 != len ){
		int maxValue = len - 1;
		if ( maxValue < 0 ){
			maxValue = 0;
		}
		ui.endSpinBox->setMaximum( maxValue );
		ui.endSpinBox->setValue( maxValue );
		ui.startSpinBox->setMaximum( maxValue );
		ui.startSpinBox->setValue( 0 );
		ui.frameSlider_->setMinimum(0);
		ui.frameSlider_->setMaximum( maxValue );
		ui.nFrmsLbl_ ->setText(QString::number(len));
	}
	ui.frameEdit_->setText(QString::number(frm));
	ui.frameSlider_->setValue(frm);
	blockSignals( false );

	//Set widgets enabled or not depending on whether there is more
	//than one frame.
	bool multiframe = false;
	if ( len > 1 ){
		multiframe = true;
	}
	setModeEnabled( multiframe );
}

int AnimatorWidget::getFrameCount() const {
	return frameCount;
}

void AnimatorWidget::setModeEnabled( bool multiframe ){
	ui.rateLbl_->setEnabled(multiframe);
	ui.rateEdit_->setEnabled(multiframe);
	ui.toStartTB_->setEnabled(multiframe);
	ui.revStepTB_->setEnabled(multiframe);
	ui.revTB_->setEnabled(multiframe);
	ui.stopTB_->setEnabled(multiframe);
	ui.playTB_->setEnabled(multiframe);
	ui.fwdStep_->setEnabled(multiframe);
	ui.toEndTB_->setEnabled(multiframe);
	ui.frameEdit_->setEnabled(multiframe);
	ui.nFrmsLbl_->setEnabled(multiframe);
	ui.frameSlider_->setEnabled(multiframe);
	ui.endSpinBox->setEnabled(multiframe);
	ui.startSpinBox->setEnabled( multiframe );
	ui.endToEndCheckBox->setEnabled( multiframe );
}

void AnimatorWidget::setRateInformation( int minr, int maxr, int rate ){
	if ( rateNotSet  ){
		ui.rateEdit_->setMinimum(minr);
		ui.rateEdit_->setMaximum(maxr);
		ui.rateEdit_->setValue(rate);
		rateNotSet = false;
	}
}

int AnimatorWidget::getRate() const {
	return ui.rateEdit_->value();
}

int AnimatorWidget::getFrame() const {
	return ui.frameSlider_->value();
}

int AnimatorWidget::getFrameStart() const {
	return ui.startSpinBox->value();
}

int AnimatorWidget::getFrameEnd() const {
	return ui.endSpinBox->value();
}

int AnimatorWidget::getStepSize() const {
	int stepSize = 1;
	if ( ui.endToEndCheckBox->isChecked() ){
		int max = ui.endSpinBox->value();
		int min = ui.startSpinBox->value();
		stepSize = max - min;
	}
	return stepSize;
}

void AnimatorWidget::setPlaying( int play ){
	blockSignals( true );
	this->play = play;
	ui.revTB_ ->setChecked(play<0);
	ui.stopTB_->setChecked(play==0);
	ui.playTB_->setChecked(play>0);
	blockSignals( false );
}

bool AnimatorWidget::isPlaying() const{
	bool playing = false;
	if ( play < 0 || play > 0 ){
		playing = true;
	}
	return playing;
}

void AnimatorWidget::frameNumberEdited(){
	int frameNumber = ui.frameEdit_->text().toInt();
	emit frameNumberEdited( frameNumber );
}

int AnimatorWidget::resetFrameBounded( int frameNumber ) const {
	int adjustedFrameNumber = frameNumber;
	int lowerBound = ui.startSpinBox->value();
	int upperBound = ui.endSpinBox->value();
	if ( frameNumber < lowerBound ){
		adjustedFrameNumber = lowerBound;
	}
	else if ( frameNumber > upperBound ){
		adjustedFrameNumber = upperBound;
	}
	return adjustedFrameNumber;
}

void AnimatorWidget::sliderControl( int /*action*/ ){
	int value = ui.frameSlider_->sliderPosition();
	value = resetFrameBounded( value );
	ui.frameSlider_->setSliderPosition( value );
}

void AnimatorWidget::movieLimitLowerChanged( int limit ){
	ui.endSpinBox->setMinimum( limit );
	emit lowerBoundChanged( limit );
	int sliderValue = ui.frameSlider_->value();
	if ( sliderValue < limit ){
		ui.frameSlider_->setValue( limit );
	}
	//Recalculate the step size based on the new lower bound
	if ( ui.endToEndCheckBox->isChecked() ){
		endToEndMode( true );
	}
}

void AnimatorWidget::movieLimitUpperChanged( int limit ){
	ui.startSpinBox->setMaximum( limit );
	emit upperBoundChanged( limit );
	int sliderValue = ui.frameSlider_->value();
	if ( sliderValue > limit ){
		ui.frameSlider_->setValue( limit );
	}

	//Recalculate the step size based on the new upper bound
	if ( ui.endToEndCheckBox->isChecked() ){
		endToEndMode( true );
	}
}

void AnimatorWidget::endToEndMode( bool endToEnd ){
	int stepSize = 1;
	if ( endToEnd ){
		stepSize = ui.endSpinBox->value() - ui.startSpinBox->value();
	}
	emit goTo( ui.startSpinBox->value() );
	emit stepSizeChanged( stepSize );
}

AnimatorWidget::~AnimatorWidget(){

}
}
