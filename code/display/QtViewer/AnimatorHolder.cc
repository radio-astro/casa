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
#include "AnimatorHolder.qo.h"
#include <QDebug>

namespace casa {

const bool AnimatorHolder::BLINK_MODE = false;
const bool AnimatorHolder::NORMAL_MODE = true;

AnimatorHolder::AnimatorHolder(QWidget *parent)
    : QWidget(parent), animatorChannel( NULL ), animatorImage( NULL )
{
	ui.setupUi(this);

	animatorChannel = new AnimatorWidget( ui.channelGroupBox );
	animatorChannel->setModeEnabled( false );
	QHBoxLayout* layoutChannel = new QHBoxLayout();
	layoutChannel->setContentsMargins( 2,2,2,2 );
	layoutChannel->addWidget( animatorChannel );
	ui.channelGroupBox->setLayout( layoutChannel );

	animatorImage = new AnimatorWidget( ui.imageGroupBox );
	animatorImage->setModeEnabled( false );
	QHBoxLayout* layoutImage = new QHBoxLayout();
	layoutImage->setContentsMargins( 2,2,2,2 );
	layoutImage->addWidget( animatorImage );
	ui.imageGroupBox->setLayout( layoutImage );

	previousMode = NORMAL_MODE;

	connect(animatorChannel, SIGNAL(goTo(int)), this, SLOT(goToChannel(int)));
	connect(animatorChannel, SIGNAL(frameNumberEdited(int)), this, SLOT(frameNumberEditedChannel(int)));
	connect(animatorChannel, SIGNAL(setRate(int)), this, SLOT(setRateChannel(int)));
	connect(animatorChannel, SIGNAL(toStart()),  this, SLOT(toStartChannel()));
	connect(animatorChannel, SIGNAL(revStep()),  this, SLOT(revStepChannel()));
	connect(animatorChannel, SIGNAL(revPlay()), this, SLOT(revPlayChannel()));
	connect(animatorChannel, SIGNAL(stop()), this, SLOT(stopChannel()));
	connect(animatorChannel, SIGNAL(fwdPlay()), this, SLOT(fwdPlayChannel()));
	connect(animatorChannel, SIGNAL(fwdStep()), this, SLOT(fwdStepChannel()));
	connect(animatorChannel, SIGNAL(toEnd()), this, SLOT(toEndChannel()));

	connect(animatorImage, SIGNAL(goTo(int)), this, SLOT(goToImage(int)));
	connect(animatorImage, SIGNAL(frameNumberEdited(int)), this, SLOT(frameNumberEditedImage(int)));
	connect(animatorImage,  SIGNAL(setRate(int)), this, SLOT(setRateImage(int)));
	connect(animatorImage, SIGNAL(toStart()), this, SLOT(toStartImage()));
	connect(animatorImage, SIGNAL(revStep()), this, SLOT(revStepImage()));
	connect(animatorImage, SIGNAL(revPlay()), this, SLOT(revPlayImage()));
	connect(animatorImage, SIGNAL(stop()), this, SLOT(stopImage()));
	connect(animatorImage, SIGNAL(fwdPlay()), this, SLOT(fwdPlayImage()));
	connect(animatorImage, SIGNAL(fwdStep()), this, SLOT(fwdStepImage()));
	connect(animatorImage, SIGNAL(toEnd()), this, SLOT(toEndImage()));
}


//-----------------------------------------------------------------------
//                    Setters
//-----------------------------------------------------------------------

void AnimatorHolder::setModeEnabled( int imageCount ){
	if ( imageCount <= 0 ){
		animatorImage->setModeEnabled( false );
		animatorChannel->setModeEnabled( false );
	}
	else if ( imageCount == 1 ){
		animatorImage->setModeEnabled( false );
		animatorChannel->setModeEnabled( true );
	}
	else {
		animatorImage->setModeEnabled( true );
		animatorChannel->setModeEnabled( true );
	}
}

void AnimatorHolder::setFrameInformation( bool mode, int frm, int len ){
	if ( mode == NORMAL_MODE ){
		animatorChannel->setFrameInformation( frm, len );
	}
	else {
		animatorImage->setFrameInformation( frm, len );
	}
}
void AnimatorHolder::setRateInformation( bool mode, int minr, int maxr, int rate ){
	if ( mode == NORMAL_MODE ){
	 		animatorChannel->setRateInformation( minr, maxr, rate );
	 }
	 else {
	 		animatorImage->setRateInformation( minr, maxr, rate );
	 }
}

void AnimatorHolder::setPlaying( bool mode, int play ){
	if ( mode == NORMAL_MODE ){
		 animatorChannel->setPlaying( play );
	}
	else {
		 animatorImage->setPlaying( play );
	}
}

//--------------------------------------------------------------------------
//                       Accessors
//--------------------------------------------------------------------------

int AnimatorHolder::getRate( bool mode ) const {
	int rate = -1;
	if ( mode == NORMAL_MODE ){
		rate = animatorChannel->getRate();
	}
	else {
		rate = animatorImage->getRate();
	}
	return rate;
}


//-------------------------------------------------------------------------
//                        Signal/Slot
//-------------------------------------------------------------------------

void AnimatorHolder::goToChannel(int frame){
	modeChanged( NORMAL_MODE );
	emit goTo( frame );
}
void AnimatorHolder::setRateChannel(int /*rate*/){
	modeChanged( NORMAL_MODE );
	//emit setRate( rate );
}
void AnimatorHolder::frameNumberEditedChannel( int frame ){
	modeChanged( NORMAL_MODE );
	emit frameNumberEdited( frame );
}
void AnimatorHolder::toStartChannel(){
	modeChanged( NORMAL_MODE );
	emit toStart();
}
void AnimatorHolder::revStepChannel(){
	modeChanged( NORMAL_MODE );
	emit revStep();
}
void AnimatorHolder::revPlayChannel(){
	modeChanged( NORMAL_MODE );
	emit revPlay();
}
void AnimatorHolder::fwdPlayChannel(){
	modeChanged( NORMAL_MODE );
	emit fwdPlay();
}
void AnimatorHolder::stopChannel(){
	modeChanged( NORMAL_MODE );
	emit stop();
}
void AnimatorHolder::fwdStepChannel(){
	modeChanged( NORMAL_MODE );
	emit fwdStep();
}
void AnimatorHolder::toEndChannel(){
	modeChanged( NORMAL_MODE );
	emit toEnd();
}
void AnimatorHolder::goToImage(int frame){
	modeChanged( BLINK_MODE );
	emit goTo( frame );
}
void AnimatorHolder::setRateImage(int /*frame*/){
	modeChanged( BLINK_MODE );
	//emit setRate( frame );
}
void AnimatorHolder::frameNumberEditedImage( int frame ){
	modeChanged( BLINK_MODE );
	emit frameNumberEdited( frame );
}
void AnimatorHolder::toStartImage(){
	modeChanged( BLINK_MODE );
	emit toStart();
}
void AnimatorHolder::revStepImage(){
	modeChanged( BLINK_MODE );
	emit revStep();
}
void AnimatorHolder::revPlayImage(){
	modeChanged( BLINK_MODE );
	emit revPlay();
}

void AnimatorHolder::fwdPlayImage(){
	modeChanged( BLINK_MODE );
	emit fwdPlay();
}
void AnimatorHolder::stopImage(){
	modeChanged( BLINK_MODE );
	emit stop();
}
void AnimatorHolder::fwdStepImage(){
	modeChanged( BLINK_MODE );
	emit fwdStep();
}
void AnimatorHolder::toEndImage(){
	modeChanged( BLINK_MODE );
	emit toEnd();
}

void AnimatorHolder::modeChanged( bool mode ){
	if ( mode != previousMode ){
		int rate = 0;
		int frame = 0;
		if ( mode == BLINK_MODE ){
			animatorChannel->setPlaying( false );
			rate = animatorImage->getRate();
			frame = animatorImage->getFrame();
		}
		else {
			animatorImage->setPlaying( false );
			rate = animatorChannel->getRate();
			frame = animatorChannel->getFrame();
		}
		previousMode = mode;
		emit setMode( mode );
		emit setRate( rate );
		emit goTo( frame );
	}
}

AnimatorHolder::~AnimatorHolder()
{

}
}
