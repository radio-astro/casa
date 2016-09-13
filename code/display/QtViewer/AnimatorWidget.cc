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
		: QWidget(parent), Ui::AnimatorWidget( ) {

		setupUi(this);

		revTB_ ->setCheckable(true);
		//stopTB_->setCheckable(true);
		playTB_->setCheckable(true);


		frameEdit_->setValidator(new QIntValidator(frameEdit_));

		connect(frameSlider_, SIGNAL(valueChanged(int)), this, SIGNAL(goTo(int)));
		connect(frameEdit_, SIGNAL(editingFinished()), this, SLOT(frameNumberEdited()));
		connect(rateEdit_,  SIGNAL(valueChanged(int)), this, SIGNAL(setRate(int)));
		connect(toStartTB_, SIGNAL(clicked()), this, SLOT(starting()));
		connect(revStepTB_, SIGNAL(clicked()), this, SLOT(revStepping()));
		connect(revTB_, SIGNAL(clicked()), this, SLOT(revPlaying()));
		connect(stopTB_, SIGNAL(clicked()), this, SLOT(stopping()));
		connect(playTB_, SIGNAL(clicked()), this, SLOT(fwdPlaying()));
		connect(fwdStep_, SIGNAL(clicked()), this, SLOT(fwdStepping()));
		connect(toEndTB_, SIGNAL(clicked()), this, SLOT(ending()));

		rateNotSet = true;

		//Start and end limits
		endSpinBox->setMinimum( 0 );
		startSpinBox->setMinimum( 0 );
		connect( startSpinBox, SIGNAL(valueChanged(int)), this, SLOT(movieLimitLowerChanged(int)));
		connect( endSpinBox,   SIGNAL(valueChanged(int)), this, SLOT(movieLimitUpperChanged(int)));
		connect( frameSlider_, SIGNAL(actionTriggered(int)), this, SLOT(sliderControl(int)));

		//Jump
		connect( endToEndCheckBox, SIGNAL(toggled(bool)), this, SLOT(endToEndMode(bool)));

	}


	void AnimatorWidget::blockSignals( bool block ) {
		frameSlider_->blockSignals(block);
		frameEdit_->blockSignals( block );
		revTB_->blockSignals( block );
		stopTB_->blockSignals( block );
		playTB_->blockSignals( block );
	}

	void AnimatorWidget::setFrameInformation( int frm, int len ) {
		frameCount = len;
		blockSignals( true );
		//If we are changing the number of frames, we need to reset
		//the properties that depend on the number of frames.
		int oldMax = endSpinBox->maximum();
		if ( oldMax + 1 != len ) {
			int maxValue = len - 1;
			if ( maxValue < 0 ) {
				maxValue = 0;
			}
			endSpinBox->setMaximum( maxValue );
			endSpinBox->setValue( maxValue );
			startSpinBox->setMaximum( maxValue );
			startSpinBox->setValue( 0 );
			frameSlider_->setMinimum(0);
			frameSlider_->setMaximum( maxValue );
			nFrmsLbl_ ->setText(QString::number(len));
		}
		frameEdit_->setText(QString::number(frm));
		frameSlider_->setValue(frm);
		blockSignals( false );

		//Set widgets enabled or not depending on whether there is more
		//than one frame.
		bool multiframe = false;
		if ( len > 1 ) {
			multiframe = true;
		}
		setModeEnabled( multiframe );
	}

	int AnimatorWidget::getFrameCount() const {
		return frameCount;
	}

	void AnimatorWidget::setModeEnabled( bool multiframe ) {
		rateLbl_->setEnabled(multiframe);
		rateEdit_->setEnabled(multiframe);
		toStartTB_->setEnabled(multiframe);
		revStepTB_->setEnabled(multiframe);
		revTB_->setEnabled(multiframe);
		stopTB_->setEnabled(multiframe);
		playTB_->setEnabled(multiframe);
		fwdStep_->setEnabled(multiframe);
		toEndTB_->setEnabled(multiframe);
		frameEdit_->setEnabled(multiframe);
		nFrmsLbl_->setEnabled(multiframe);
		frameSlider_->setEnabled(multiframe);
		endSpinBox->setEnabled(multiframe);
		startSpinBox->setEnabled( multiframe );
		endToEndCheckBox->setEnabled( multiframe );
	}

	void AnimatorWidget::setRateInformation( int minr, int maxr, int rate ) {
		if ( rateNotSet  ) {
			rateEdit_->setMinimum(minr);
			rateEdit_->setMaximum(maxr);
			rateEdit_->setValue(rate);
			rateNotSet = false;
		}
	}

	int AnimatorWidget::getRate() const {
		return rateEdit_->value();
	}

	int AnimatorWidget::getFrame() const {
		return frameSlider_->value();
	}

	int AnimatorWidget::getFrameStart() const {
		return startSpinBox->value();
	}

	int AnimatorWidget::getFrameEnd() const {
		return endSpinBox->value();
	}

	int AnimatorWidget::getStepSize() const {
		int stepSize = 1;
		if ( endToEndCheckBox->isChecked() ) {
			int max = endSpinBox->value();
			int min = startSpinBox->value();
			stepSize = max - min;
		}
		return stepSize;
	}

	void AnimatorWidget::disableAll() {
		play = 0;
		blockSignals( true );
		revTB_->setChecked( false );
		playTB_->setChecked( false );
		blockSignals( false );
	}

	void AnimatorWidget::starting() {
		disableAll();
		emit toStart();
	}

	void AnimatorWidget::revStepping() {
		disableAll();
		emit revStep();
	}

	void AnimatorWidget::stopping() {
		disableAll();
		emit stop();
	}

	void AnimatorWidget::fwdStepping() {
		disableAll();
		emit fwdStep();
	}

	void AnimatorWidget::ending() {
		disableAll();
		emit toEnd();
	}

	void AnimatorWidget::revPlaying() {
		play = -1;
		blockSignals( true );
		playTB_->setChecked( false );
		revTB_->setChecked( true );
		blockSignals( false );
		emit revPlay();
	}

	void AnimatorWidget::fwdPlaying() {
		play = 1;
		blockSignals( true );
		revTB_->setChecked( false );
		playTB_->setChecked( true );
		blockSignals( false );
		emit fwdPlay();
	}

	void AnimatorWidget::setPlaying( int play ) {
		blockSignals( true );
		this->play = play;
		revTB_ ->setChecked(play<0);
		playTB_->setChecked(play>0);
		blockSignals( false );
	}

	bool AnimatorWidget::isPlaying() const {
		bool playing = false;
		if ( play < 0 || play > 0 ) {
			playing = true;
		}
		return playing;
	}



	void AnimatorWidget::frameNumberEdited() {
		int frameNumber = frameEdit_->text().toInt();
		emit frameNumberEdited( frameNumber );
	}

	int AnimatorWidget::resetFrameBounded( int frameNumber ) const {
		int adjustedFrameNumber = frameNumber;
		int lowerBound = startSpinBox->value();
		int upperBound = endSpinBox->value();
		if ( frameNumber < lowerBound ) {
			adjustedFrameNumber = lowerBound;
		} else if ( frameNumber > upperBound ) {
			adjustedFrameNumber = upperBound;
		}
		return adjustedFrameNumber;
	}

	void AnimatorWidget::sliderControl( int /*action*/ ) {
		int value = frameSlider_->sliderPosition();
		value = resetFrameBounded( value );
		frameSlider_->setSliderPosition( value );
	}

	void AnimatorWidget::movieLimitLowerChanged( int limit ) {
		endSpinBox->setMinimum( limit );
		emit lowerBoundChanged( limit );
		int sliderValue = frameSlider_->value();
		if ( sliderValue < limit ) {
			frameSlider_->setValue( limit );
		}
		//Recalculate the step size based on the new lower bound
		if ( endToEndCheckBox->isChecked() ) {
			endToEndMode( true );
		}
	}

	void AnimatorWidget::movieLimitUpperChanged( int limit ) {
		startSpinBox->setMaximum( limit );
		emit upperBoundChanged( limit );
		int sliderValue = frameSlider_->value();
		if ( sliderValue > limit ) {
			frameSlider_->setValue( limit );
		}

		//Recalculate the step size based on the new upper bound
		if ( endToEndCheckBox->isChecked() ) {
			endToEndMode( true );
		}
	}

	void AnimatorWidget::endToEndMode( bool endToEnd ) {
		int stepSize = 1;
		if ( endToEnd ) {
			stepSize = endSpinBox->value() - startSpinBox->value();
		}
		emit goTo( startSpinBox->value() );
		emit stepSizeChanged( stepSize );
	}

	AnimatorWidget::~AnimatorWidget() { }

}
