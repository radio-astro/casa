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

namespace casa {

AnimatorWidget::AnimatorWidget(QWidget *parent)
    : QWidget(parent)
{
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
	ui.frameEdit_->setText(QString::number(frm));
	ui.nFrmsLbl_ ->setText(QString::number(len));
	ui.frameSlider_->setMinimum(0);
	ui.frameSlider_->setMaximum(len-1);
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
	ui.stopTB_->setEnabled(multiframe);	// Tape deck controls.
	ui.playTB_->setEnabled(multiframe);	//
	ui.fwdStep_->setEnabled(multiframe);	//
	ui.toEndTB_->setEnabled(multiframe);	//
	ui.frameEdit_->setEnabled(multiframe);	// Frame number entry.
	ui.nFrmsLbl_->setEnabled(multiframe);	// Total frames label.
	ui.frameSlider_->setEnabled(multiframe);	// Frame number slider.*/
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

AnimatorWidget::~AnimatorWidget()
{

}
}
