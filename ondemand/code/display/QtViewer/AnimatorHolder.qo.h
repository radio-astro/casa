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
#ifndef ANIMATORHOLDER_QO_H
#define ANIMATORHOLDER_QO_H

#include <QtGui/QWidget>
#include <display/QtViewer/InActiveDock.qo.h>
#include <display/QtViewer/AnimatorHolder.ui.h>
#include <display/QtViewer/AnimatorWidget.qo.h>

namespace casa {

    class QtDisplayPanelGui;
	/**
	 * Manages the Animator display on the viewer that allows users to scroll through
	 * either the channels withen an image or between loaded images.
	 */
    class AnimatorHolder : public InActiveDock, protected Ui::AnimatorHolder {
		Q_OBJECT

	public:
		const static bool BLINK_MODE;
		const static bool NORMAL_MODE;
		AnimatorHolder( QtDisplayPanelGui *qdp, QWidget *parent = 0 );
		void setFrameInformation( bool mode, int frm, int len );
		void setRateInformation( bool mode, int minr, int maxr, int rate );
		void setModeEnabled( int count );
		void setChannelModeEnabled( int count, bool select=true);
		int getRate( bool mode ) const;
		int getLowerBoundChannel() const;
		int getUpperBoundChannel() const;

		int getChannelCount() const;
		int getImageCount() const;

		void setChannelZAxis( const QString& zAxisTite );

		void foldChannel( ) { channelGroupBox->setChecked(false); }
		void unfoldChannel( ) { channelGroupBox->setChecked(true); }
		void foldImage( ) { imageGroupBox->setChecked(false); }
		void unfoldImage( ) { imageGroupBox->setChecked(true); }

		~AnimatorHolder();

		void dismiss( );

	protected:
		void closeEvent ( QCloseEvent * event );

	signals:

		//--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
		// animation for channels
		//--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
		void setChannelMode( );
/*goTo*///displayPanel
		void goToChannel( int channel );
/*frameNumberEdited*/
		void channelNumEdited( int frame );
/*channelSelect*/
		void selectChannel( int channel );
/*movieChannels*/
		void channelMovieState( int currentFrame, bool direction, int stepSize, int frameStart, int frameEnd );
/*stopMovie*///display
		//void stopChannelMovie( );
/*setRate*///displayPanel
		void setChannelMovieRate( int frame );
/*toStart*///displayPanel
		void toChannelMovieStart( );
/*revStep*///displayPanel
		void revStepChannelMovie( );
/*revPlay*/
		void revPlayChannelMovie( );
/*stop*///displayPanel
		void stopChannelMovie( );
/*fwdStep*///displayPanel
		void fwdStepChannelMovie( );
/*fwdPlay*/
		void fwdPlayChannelMovie( );
/*toEnd*///displayPanel
		void toChannelMovieEnd( );

		//--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
		// animation for images
		//--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
		void setImageMode( bool channelCubes );
/*goTo*///displayPanel
		void goToImage( int image );
/*frameNumberEdited*/
		void imageNumEdited( int frame );
/*channelSelect*/
		void selectImage( int channel );
/*movieChannels*/
		void imageMovieState( int currentFrame, bool direction, int stepSize, int frameStart, int frameEnd );
/*stopMovie*/
		void stopImageMovie( );
/*setRate*///setImageMovieRate
		void setImageMovieRate( int frame );
/*toStart*///displayPanel
		void toImageMovieStart( );
/*revStep*///displayPanel
		void revStepImageMovie( );
/*revPlay*/
		void revPlayImageMovie( );
/*stop*///displayPanel
		//void stopImageMovie( );
/*fwdStep*///displayPanel
		void fwdStepImageMovie( );
/*fwdPlay*/
		void fwdPlayImageMovie( );
/*toEnd*///displayPanel
		void toImageMovieEnd( );

		void lowerBoundAnimatorImageChanged( int );
		void upperBoundAnimatorImageChanged(int);
		void stepSizeAnimatorImageChanged(int);
		void lowerBoundAnimatorChannelChanged( int );
		void upperBoundAnimatorChannelChanged(int);
		void stepSizeAnimatorChannelChanged(int);
		void animationImageChanged(int);

	private slots:
		void gotochannel_p(int frame);
		void setRateChannel(int frame);
		void toStartChannel();
		void frameNumberEditedChannel( int frame );
		void revStepChannel();
		void fwdPlayChannel();
		void revPlayChannel();
		void stopChannel();
		void fwdStepChannel();
		void toEndChannel();
		void upperBoundChangedChannel(int);
		void lowerBoundChangedChannel(int);
		void stepSizeChangedChannel(int);
		void gotoimage_p(int frame);
		void setRateImage(int frame);
		void toStartImage();
		void frameNumberEditedImage( int frame );
		void revStepImage();
		void revPlayImage();
		void stopImage();
		void fwdStepImage();
		void fwdPlayImage();
		void toEndImage();
		void lowerBoundChangedImage(int);
		void upperBoundChangedImage(int);
		void stepSizeChangedImage(int);
		void modeChange();

        void handle_folding( bool visible );
		void handle_visibility(bool);

	private:
		void initChannel();
		void initImage();
		bool addChannelGroupBox();
		void addImageGroupBox();
        void sizeGroupBox( QGroupBox* );
		void removeChannelGroupBox();
		bool removeImageGroupBox();
		void setHeightFixed();
		int getAnimationCount() const;
		void addRemoveChannelAnimatorBasedOnFrameCount();
        int find_height( ) const;

		enum Mode {CHANNEL_MODE, IMAGE_MODE, CHANNEL_IMAGES_MODE, END_MODE };
		void modeChanged(Mode mode );
		void changePalette( QGroupBox* box, QColor color );
		void emitMovieChannels( bool direction );
		void setSelected( bool mode );

		//Because the user can switch between image and channel mode by
		//just pressing the play button, there must be a way to turn a previous
		//play off if one was running.  The following two methods do that.
		void stopImagePlay();
		void stopChannelPlay();

		Mode previousMode;
		AnimatorWidget* animatorChannel;
		AnimatorWidget* animatorImage;
		QColor selectedColor;
		QColor backgroundColor;

        QtDisplayPanelGui *panel_;

		bool dismissed;
	};
}
#endif // ANIMATORHOLDER_QO_H
