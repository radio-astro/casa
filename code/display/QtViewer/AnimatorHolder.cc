//# Copyright (C) 2005,2013
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
#include <display/QtViewer/AnimatorHolder.qo.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <QDebug>

namespace casa {

	const bool AnimatorHolder::BLINK_MODE = false;
	const bool AnimatorHolder::NORMAL_MODE = true;

	AnimatorHolder::AnimatorHolder( QtDisplayPanelGui *qdp, QWidget *parent )
         : InActiveDock(parent), Ui::AnimatorHolder( ),
		  animatorChannel( NULL ), animatorImage( NULL ),
		  selectedColor( Qt::white), panel_(qdp), dismissed(false) {

		setupUi(this);
        int height = find_height( );
        setFixedHeight( height );
        setMinimumHeight( height );
        setMaximumHeight( height );

		QPalette pal( palette());
		backgroundColor = pal.color( QPalette::Background );

		//Initialize the channel and image animators so they
		//are ready to go.
		initChannel();
		initImage();

		//Initially there are no images so we should not show
		//the animator.
		removeImageGroupBox();
		removeChannelGroupBox();

		previousMode = CHANNEL_MODE;

        connect( this, SIGNAL(lowerBoundAnimatorChannelChanged(int)), panel_->displayPanel( ), SLOT(lowerBoundAnimatorChannelChanged(int)));
        connect( this, SIGNAL(upperBoundAnimatorChannelChanged(int)), panel_->displayPanel( ), SLOT(upperBoundAnimatorChannelChanged(int)));
        connect( this, SIGNAL(lowerBoundAnimatorImageChanged(int)), panel_->displayPanel( ), SLOT(lowerBoundAnimatorImageChanged(int)));
        connect( this, SIGNAL(upperBoundAnimatorImageChanged(int)), panel_->displayPanel( ), SLOT(upperBoundAnimatorImageChanged(int)));
        connect( this, SIGNAL(goTo(int)), panel_->displayPanel( ), SLOT(goTo(int)));
        connect( this, SIGNAL(frameNumberEdited(int)), panel_->displayPanel( ), SLOT(goTo(int)));
        connect( this,  SIGNAL(setRate(int)), panel_->displayPanel( ), SLOT(setRate(int)));
        connect( this, SIGNAL(toStart()), panel_->displayPanel( ), SLOT(toStart()));
        connect( this, SIGNAL(revStep()), panel_->displayPanel( ), SLOT(revStep()));
        connect( this, SIGNAL(stop()), panel_->displayPanel( ), SLOT(stop()));
        connect( this, SIGNAL(fwdStep()), panel_->displayPanel( ), SLOT(fwdStep()));
        connect( this, SIGNAL(toEnd()), panel_->displayPanel( ), SLOT(toEnd()));

		connect( this, SIGNAL(visibilityChanged(bool)), SLOT(handle_visibility(bool)) );

        connect( channelGroupBox, SIGNAL(toggled(bool)), SLOT(handle_folding(bool)) );
        connect( imageGroupBox, SIGNAL(toggled(bool)), SLOT(handle_folding(bool)) );

	}

	int AnimatorHolder::getChannelCount() const {
		int frameCount = 0;
		if ( animatorChannel != NULL ){
			frameCount=animatorChannel->getFrameCount();
		}
		return frameCount;
	}

	int AnimatorHolder::getImageCount() const {
		int imageCount = 0;
		if ( animatorImage != NULL ){
			imageCount=animatorImage->getFrameCount();
		}
		return imageCount;
	}

	int AnimatorHolder::getAnimationCount() const {
		int count = 0;
		QLayout* layoutBase = layout();
		int channelIndex = layoutBase->indexOf( channelGroupBox );
		int imageIndex = layoutBase->indexOf( imageGroupBox );
		if ( channelIndex >= 0 ) {
			count++;
		}
		if ( imageIndex >= 0 ) {
			count++;
		}
		return count;
	}

	void AnimatorHolder::setHeightFixed() {
#if 0
		int boxCount = getAnimationCount();
		const int BASE_HEIGHT = 83;
		int height = 20 + BASE_HEIGHT * boxCount;
		setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
		setFixedHeight( height );
#endif
	}

	bool AnimatorHolder::addChannelGroupBox() {
#if 0
		QLayout* layout = container->layout();
		int channelGroupIndex = layout->indexOf( channelGroupBox );
		bool channelAdded = false;
		if ( channelGroupIndex == -1 ) {
			channelGroupBox->setParent( this );
			// channelGroupBox->setVisible( true );
			//We want the channel group box to always be first
			//so we may need to remove the image group box and put it
			//in later if it is there.
			bool removedImage = removeImageGroupBox();
			layout->addWidget( channelGroupBox );
			if ( removedImage ) {
				addImageGroupBox();
			}
			setHeightFixed();
			channelAdded = true;
		}
		return channelAdded;
#else
        return false;
#endif
	}

	void AnimatorHolder::addImageGroupBox() {
#if 0
		QLayout* layout = container->layout();
		int imageGroupIndex = layout->indexOf( imageGroupBox );
		if ( imageGroupIndex == -1 ) {
			imageGroupBox->setParent( this );
			//imageGroupBox->setVisible( true );
			layout->addWidget( imageGroupBox );
			setHeightFixed();
		}
#endif
	}

	void AnimatorHolder::removeChannelGroupBox() {
#if 0
		QLayout* layout = container->layout();
		int channelGroupIndex = layout->indexOf( channelGroupBox );
		if ( channelGroupIndex >= 0 ) {
			layout->removeWidget( channelGroupBox );
			//channelGroupBox->setVisible( false );
			channelGroupBox->setParent( NULL );
			setHeightFixed();
		}
#endif
	}

	bool AnimatorHolder::removeImageGroupBox() {
#if 0
		QLayout* layout = container->layout();
		int imageGroupIndex = layout->indexOf( imageGroupBox );
		bool removed = false;
		if ( imageGroupIndex >= 0 ) {
			layout->removeWidget( imageGroupBox );
			imageGroupBox->setParent( NULL );
			//imageGroupBox->setVisible( false );
			setHeightFixed();
			removed = true;
		}
		return removed;
#else
        return false;
#endif
	}
	int AnimatorHolder::getLowerBoundChannel() const {
		return animatorChannel->getFrameStart();
	}
	int AnimatorHolder::getUpperBoundChannel() const {
		return animatorChannel->getFrameEnd();
	}
	void AnimatorHolder::initChannel() {
		if ( animatorChannel == NULL ) {
			animatorChannel = new AnimatorWidget( channelGroupBox );
            if ( channelGroupBox->isChecked( ) == false ) {
                animatorChannel->setVisible(false);
            }
			animatorChannel->setModeEnabled( false );

            QVBoxLayout *layout = new QVBoxLayout(channelGroupBox);
            layout->addWidget(animatorChannel);
            layout->setMargin(1);
            animatorChannel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

            sizeGroupBox(channelGroupBox);

			connect( channelGroupBox, SIGNAL(clicked()), this, SLOT(modeChange()));
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
			connect(animatorChannel, SIGNAL(lowerBoundChanged(int)), this, SLOT(lowerBoundChangedChannel(int)));
			connect(animatorChannel, SIGNAL(upperBoundChanged(int)), this, SLOT(upperBoundChangedChannel(int)));
			connect(animatorChannel, SIGNAL(stepSizeChanged(int)), this, SLOT(stepSizeChangedChannel(int)));

		}
		// channelGroupBox->setVisible( true );
	}

	void AnimatorHolder::initImage() {
		if ( animatorImage == NULL ) {
			animatorImage = new AnimatorWidget( imageGroupBox );
            if ( imageGroupBox->isChecked( ) == false ) {
                animatorImage->setVisible(false);
            }
			// animatorImage->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
			animatorImage->setModeEnabled( false );

            QVBoxLayout *layout = new QVBoxLayout(imageGroupBox);
            layout->addWidget(animatorImage);
            layout->setMargin(1);
            animatorImage->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

            sizeGroupBox(imageGroupBox);

			connect( imageGroupBox, SIGNAL(clicked()), this, SLOT(modeChange()));
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
			connect(animatorImage, SIGNAL(lowerBoundChanged(int)), this, SLOT(lowerBoundChangedImage(int)));
			connect(animatorImage, SIGNAL(upperBoundChanged(int)), this, SLOT(upperBoundChangedImage(int)));
			connect(animatorImage, SIGNAL(stepSizeChanged(int)), this, SLOT(stepSizeChangedImage(int)));
		}
		// imageGroupBox->setVisible( true );
	}


//-----------------------------------------------------------------------
//                    Setters
//-----------------------------------------------------------------------

	void AnimatorHolder::setChannelModeEnabled( int frameCount, bool select) {
		//This method was added because if the case where two images were
		//initially loaded, each with one channel, the animator comes up
		//with only the "image animator" showing in image mode.
		//If you then unregister the single channel images and load
		//at least one image with multiple channels, the channel
		//animator won't come up because it is stuck in image mode.
		bool channelAdded=addChannelGroupBox();
		if ( channelAdded ) {
			channelGroupBox->setCheckable(true);
			animatorChannel->setEnabled(true);
			if ( frameCount > 1 && select ){
				channelGroupBox->setChecked( true );
			}
			else {
				channelGroupBox->setChecked( false );
			}
		}

		int oldFrameCount = animatorChannel->getFrameCount();
		if ( oldFrameCount != frameCount ) {
			animatorChannel->setFrameInformation(0, frameCount );
		}
	}

	void AnimatorHolder::setChannelZAxis( const QString& zAxisTitle ){
		//The label of the z-axis should mirror the display options z-axis,
		//except in the case of Frequency where it should say Channels.
		if ( !zAxisTitle.startsWith( "Freq")){
			channelGroupBox->setTitle( zAxisTitle);
		}
		else {
			channelGroupBox->setTitle( "Channels");
		}
	}

	void AnimatorHolder::setModeEnabled( int imageCount ) {
		int animationCount = getAnimationCount();
		bool modeChanged = false;
		if ( imageCount <= 0 ) {
			if ( animationCount != 0 ) {
				modeChanged = true;
				removeImageGroupBox();
				removeChannelGroupBox();
				animatorImage->setModeEnabled( false );
				animatorChannel->setModeEnabled( false );
			}
		} else if ( imageCount == 1 ) {
			bool imageEnabled = imageGroupBox->isCheckable();
			if ( imageEnabled ) {
				modeChanged = true;
				removeImageGroupBox();
			}
			addRemoveChannelAnimatorBasedOnFrameCount();
			//More than one image
		} else {
			addImageGroupBox();
			bool imageEnabled = imageGroupBox->isCheckable();
			//Image mode is coming up after having been unavailable
			if ( !imageEnabled ) {
				modeChanged = true;
				animatorImage->setModeEnabled( true );
				imageGroupBox->setCheckable( true );
				imageGroupBox->setChecked( false );
			}
			addRemoveChannelAnimatorBasedOnFrameCount();
			int displayCount = getAnimationCount();
			bool oldImageChecked = imageGroupBox->isChecked();
			if ( displayCount == 1 ) {
				imageGroupBox->setChecked( true );
				if ( !oldImageChecked ){
					modeChanged = true;
				}
			}
			else if ( displayCount == 2 && !channelGroupBox->isChecked()){
				imageGroupBox->setChecked( true );
				if ( !oldImageChecked ){
					modeChanged = true;
				}
			}
		}
		animatorImage->setFrameInformation( 0, imageCount );
		if ( modeChanged ) {
			modeChange();
		}
	}

	void AnimatorHolder::addRemoveChannelAnimatorBasedOnFrameCount() {
		if ( animatorChannel->getFrameCount() > 1 ) {
			addChannelGroupBox();
			animatorChannel->setModeEnabled( true );
			if ( !channelGroupBox->isCheckable() ){
				channelGroupBox->setCheckable( true );
				channelGroupBox->setChecked( false );
			}
			changePalette( channelGroupBox, selectedColor );
		} else {
			removeChannelGroupBox();
		}
	}



	void AnimatorHolder::setFrameInformation( bool mode, int frm, int len ) {
		if ( previousMode == CHANNEL_IMAGES_MODE && mode==NORMAL_MODE) {
			//The number of frames should match channels
			//since it is scrolling through channels
			int frameCount = animatorChannel->getFrameCount();
			animatorChannel->setFrameInformation( frm, frameCount );
		} else if ( mode == NORMAL_MODE ) {
			animatorChannel->setFrameInformation( frm, len );
			addRemoveChannelAnimatorBasedOnFrameCount();
		} else {
			animatorImage->setFrameInformation( frm, len );
			emit animationImageChanged( frm );
		}
	}

	void AnimatorHolder::setRateInformation( bool mode, int minr, int maxr, int rate ) {

		if ( previousMode != CHANNEL_IMAGES_MODE ) {
			if ( mode == NORMAL_MODE ) {
				animatorChannel->setRateInformation( minr, maxr, rate );
			} else {
				animatorImage->setRateInformation( minr, maxr, rate );
			}
		}
	}



//--------------------------------------------------------------------------
//                       Accessors
//--------------------------------------------------------------------------

	int AnimatorHolder::getRate( bool mode ) const {
		int rate = -1;
		if ( mode == NORMAL_MODE ) {
			rate = animatorChannel->getRate();
		} else {
			rate = animatorImage->getRate();
		}
		return rate;
	}


//-------------------------------------------------------------------------
//                        Signal/Slot
//-------------------------------------------------------------------------

//Channels

	void AnimatorHolder::goToChannel(int frame) {
		stopImagePlay();
		if ( previousMode == CHANNEL_MODE ) {
			emit goTo( frame );
		} else {
			emit channelSelect( frame );
		}
	}
	void AnimatorHolder::setRateChannel(int rate) {
		stopImagePlay();
		emit setRate( rate );
	}
	void AnimatorHolder::frameNumberEditedChannel( int frame ) {
		stopImagePlay();
		if ( previousMode == CHANNEL_MODE ) {
			emit frameNumberEdited( frame );
		} else {
			emit channelSelect( frame );
		}
	}
	void AnimatorHolder::toStartChannel() {
		stopImagePlay();
		if ( previousMode == CHANNEL_MODE ) {
			emit toStart();
		} else {
			int startFrame = animatorChannel->getFrameStart();
			emit channelSelect(startFrame);
		}
	}
	void AnimatorHolder::revStepChannel() {
		stopImagePlay();
		if ( previousMode == CHANNEL_MODE ) {
			emit revStep();
		} else {
			int currentFrame = this->animatorChannel->getFrame();
			int stepSize = animatorChannel->getStepSize();
			int minFrame = animatorChannel->getFrameStart();
			currentFrame = currentFrame-stepSize;
			if ( currentFrame < minFrame ) {
				int diff = minFrame - currentFrame -1;
				int maxFrame = animatorChannel->getFrameEnd();
				currentFrame = maxFrame - diff;
			}
			emit channelSelect( currentFrame );
		}
	}
	void AnimatorHolder::revPlayChannel() {
		stopImagePlay();
		if ( previousMode == CHANNEL_MODE ) {
			emit revPlay();
		} else {
			animatorChannel->setPlaying( -1 );
			emitMovieChannels( false );

		}

	}
	void AnimatorHolder::fwdPlayChannel() {
		stopImagePlay();
		if ( previousMode == CHANNEL_MODE ) {
			emit fwdPlay();
		} else {
			animatorChannel->setPlaying( 1 );
			emitMovieChannels( true );
		}
	}
	void AnimatorHolder::stopChannel() {
		stopImagePlay();
		if ( previousMode == CHANNEL_MODE ) {
			emit stop();
		} else {
			animatorChannel->setPlaying( 0 );
			emit stopMovie();
		}
	}
	void AnimatorHolder::fwdStepChannel() {
		stopImagePlay();
		if ( previousMode == CHANNEL_MODE ) {
			emit fwdStep();
		} else {
			int currentFrame = animatorChannel->getFrame();
			int stepSize = animatorChannel->getStepSize();
			currentFrame = currentFrame + stepSize;
			int maxFrame = animatorChannel->getFrameEnd();
			if ( currentFrame > maxFrame  ) {
				int diff = currentFrame - maxFrame - 1;
				int minFrame = animatorChannel->getFrameStart();
				currentFrame = minFrame + diff;
			}
			emit channelSelect( currentFrame );
		}
	}
	void AnimatorHolder::toEndChannel() {
		stopImagePlay();
		if ( previousMode == CHANNEL_MODE ) {
			emit toEnd();
		} else {
			int lastFrame = this->animatorChannel->getFrameEnd();
			emit channelSelect( lastFrame );
		}
	}

	void AnimatorHolder::lowerBoundChangedChannel(int bound ) {
		emit lowerBoundAnimatorChannelChanged( bound );
	}

	void AnimatorHolder::upperBoundChangedChannel(int bound ) {
		emit upperBoundAnimatorChannelChanged( bound );
	}

	void AnimatorHolder::stepSizeChangedChannel(int size ) {
		emit stepSizeAnimatorChannelChanged( size );
	}

//Images

	void AnimatorHolder::goToImage(int frame) {
		stopChannelPlay();
		emit goTo( frame );
	}
	void AnimatorHolder::setRateImage(int rate) {
		stopChannelPlay();
		emit setRate( rate );
	}

	void AnimatorHolder::frameNumberEditedImage( int frame ) {
		stopChannelPlay();
		emit frameNumberEdited( frame );
	}

	void AnimatorHolder::toStartImage() {
		stopChannelPlay();
		emit toStart();
	}
	void AnimatorHolder::revStepImage() {
		stopChannelPlay();
		emit revStep();
	}
	void AnimatorHolder::revPlayImage() {
		stopChannelPlay();
		animatorImage->setPlaying( -1 );
		emit revPlay();
	}

	void AnimatorHolder::fwdPlayImage() {
		stopChannelPlay();
		animatorImage->setPlaying( 1 );
		emit fwdPlay();
	}
	void AnimatorHolder::stopImage() {
		animatorImage->setPlaying( 0 );
		emit stop();
	}
	void AnimatorHolder::fwdStepImage() {
		stopChannelPlay();
		emit fwdStep();
	}
	void AnimatorHolder::toEndImage() {
		stopChannelPlay();
		emit toEnd();
	}

	void AnimatorHolder::emitMovieChannels( bool direction ) {
		int frameStart = animatorChannel->getFrameStart();
		int currentFrame = frameStart;
		int frameEnd = animatorChannel->getFrameEnd();
		int stepSize = animatorChannel->getStepSize();
		emit movieChannels( currentFrame, direction, stepSize, frameStart, frameEnd );
	}

	void AnimatorHolder::stopImagePlay() {
		if ( animatorImage->isPlaying()) {
			animatorImage->stopping();
		}
	}

	void AnimatorHolder::stopChannelPlay() {
		if ( animatorChannel->isPlaying()) {
			animatorChannel->stopping();
		}
	}

	void AnimatorHolder::lowerBoundChangedImage(int bound ) {
		emit lowerBoundAnimatorImageChanged( bound );
	}

	void AnimatorHolder::upperBoundChangedImage(int bound ) {
		emit upperBoundAnimatorImageChanged( bound );
	}

	void AnimatorHolder::stepSizeChangedImage(int size ) {
		emit stepSizeAnimatorImageChanged( size );
	}

//----------------------------------------------------------------
//                 Change the mode between channel, image, and
//                 channel_image
//----------------------------------------------------------------

	void AnimatorHolder::modeChanged( Mode mode ) {
		if ( mode != previousMode ) {
			int rate = 0;
			int frame = 0;
			bool channelMode = NORMAL_MODE;
			bool channelImages = false;
			if ( mode == IMAGE_MODE ) {
				animatorChannel->setPlaying( false );
				rate = animatorImage->getRate();
				frame = animatorImage->getFrame();
				channelMode = BLINK_MODE;
			} else if ( mode == CHANNEL_MODE  ) {
				animatorImage->setPlaying( false );
				rate = animatorChannel->getRate();
				frame = animatorChannel->getFrame();
			} else if ( mode == CHANNEL_IMAGES_MODE ) {
				channelImages = true;
				animatorImage->setPlaying( false );
				rate = animatorChannel->getRate();
				frame = animatorChannel->getFrame();
				channelMode = BLINK_MODE;
			}
			emit setMode( channelMode );
			emit setRate( rate );
			emit goTo( frame );
			previousMode = mode;
		}
	}

	void AnimatorHolder::modeChange() {
		bool channelMode = channelGroupBox->isChecked();
		bool imageMode = imageGroupBox->isChecked();
		Mode mode = END_MODE;
		if ( channelMode && !imageMode ) {
			mode = CHANNEL_MODE;
			changePalette( channelGroupBox, selectedColor );
			changePalette( imageGroupBox, backgroundColor );
		} else if ( !channelMode && imageMode ) {
			mode = IMAGE_MODE;
			changePalette( imageGroupBox, selectedColor );
			changePalette( channelGroupBox, backgroundColor );
		} else if ( channelMode && imageMode ) {
			mode = CHANNEL_IMAGES_MODE;
			changePalette( channelGroupBox, selectedColor );
			changePalette( imageGroupBox, selectedColor );
		} else {
			changePalette( channelGroupBox, backgroundColor );
			changePalette( imageGroupBox, backgroundColor );
		}
		if ( mode != END_MODE ) {
			modeChanged( mode );
		}
	}


	void AnimatorHolder::changePalette( QGroupBox* box, QColor color ) {
		QPalette pal = box->palette();
		pal.setColor( QPalette::Background, color );
		box->setPalette( pal );
	}

	AnimatorHolder::~AnimatorHolder() { }

	void AnimatorHolder::dismiss( ) {
		hide( );
		dismissed = true;
	}

	void AnimatorHolder::closeEvent ( QCloseEvent * event ) {
		dismissed = true;
		QDockWidget::closeEvent(event);
		panel_->putrc( "visible.animator", "false" );
	}

    int AnimatorHolder::find_height( ) const {
        int result = AnimatorWidget::heightHeader( );

        if ( channelGroupBox->isChecked( ) )
             result += AnimatorWidget::heightOpen( );
        else
             result += AnimatorWidget::heightClosed( );

        if ( imageGroupBox->isChecked( ) )
             result += AnimatorWidget::heightOpen( );
        else
             result += AnimatorWidget::heightClosed( );
        return result;
    }

    void AnimatorHolder::sizeGroupBox( QGroupBox *gb ) {
        if ( gb->isChecked( ) ) {
            gb->setFixedHeight( AnimatorWidget::heightOpen( ) );
            gb->setMinimumHeight( AnimatorWidget::heightOpen( ) );
            gb->setMaximumHeight( AnimatorWidget::heightOpen( ) );
        } else {
            gb->setFixedHeight( AnimatorWidget::heightClosed( ) );
            gb->setMinimumHeight( AnimatorWidget::heightClosed( ) );
            gb->setMaximumHeight( AnimatorWidget::heightClosed( ) );
        }
        gb->updateGeometry( );

    }

    void AnimatorHolder::handle_folding( bool visible ) {
        QObject *obj = sender( );
        QGroupBox *gb = dynamic_cast<QGroupBox*>(obj);
		QList <AnimatorWidget*> animators = gb->findChildren<AnimatorWidget*>( );
        if ( animators.size( ) == 1 ) {
            animators[0]->setVisible(visible);
            sizeGroupBox(gb);
            int height = find_height( );
            setFixedHeight( height );
            setMinimumHeight( height );
            setMaximumHeight( height );
            updateGeometry( );
        }
    }

	void AnimatorHolder::handle_visibility( bool visible ) {
		if ( visible && dismissed ) {
			dismissed = false;
			panel_->putrc( "visible.animator", "true" );
		}
	}

}
