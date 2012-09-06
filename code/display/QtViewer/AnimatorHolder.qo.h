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
#ifndef ANIMATORHOLDER_QO_H
#define ANIMATORHOLDER_QO_H

#include <QtGui/QWidget>
#include <display/QtViewer/AnimatorHolder.ui.h>
#include <display/QtViewer/AnimatorWidget.qo.h>
namespace casa {

/**
 * Manages the Animator display on the viewer that allows users to scroll through
 * either the channels withen an image or between loaded images.
 */
class AnimatorHolder : public QWidget
{
    Q_OBJECT

public:
    const static bool BLINK_MODE;
    const static bool NORMAL_MODE;
    AnimatorHolder(QWidget *parent = 0);
    void setFrameInformation( bool mode, int frm, int len );
    void setPlaying( bool mode, int play );
    void setRateInformation( bool mode, int minr, int maxr, int rate );
    void setModeEnabled( int count );
    int getRate( bool mode ) const;
    ~AnimatorHolder();

signals:
	void goTo(int frame);
	void frameNumberEdited( int frame );
	void setRate(int frame);
	void toStart();
	void revStep();
	void revPlay();
	void stop();
	void fwdStep();
	void fwdPlay();
	void toEnd();
	void setMode( bool mode );

private slots:
	void goToChannel(int frame);
	void setRateChannel(int frame);
	void toStartChannel();
	void frameNumberEditedChannel( int frame );
	void revStepChannel();
	void fwdPlayChannel();
	void revPlayChannel();
	void stopChannel();
	void fwdStepChannel();
	void toEndChannel();
	void goToImage(int frame);
	void setRateImage(int frame);
	void toStartImage();
	void frameNumberEditedImage( int frame );
	void revStepImage();
	void revPlayImage();
	void stopImage();
	void fwdStepImage();
	void fwdPlayImage();
	void toEndImage();

private:
	void modeChanged( bool mode );
	bool previousMode;
    Ui::AnimatorHolder ui;
    AnimatorWidget* animatorChannel;
    AnimatorWidget* animatorImage;

};
}
#endif // ANIMATORHOLDER_QO_H
