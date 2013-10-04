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
#ifndef ANIMATORWIDGETT_QO_H
#define ANIMATORWIDGETT_QO_H

#include <QtGui/QWidget>
#include <display/QtViewer/AnimatorWidget.ui.h>

namespace casa {

	/**
	 * Manages an individual panel of the viewer animator.  Objects of
	 * this class may animate frames withen an image or they may animate
	 * between loaded images.
	 */
    class AnimatorWidget : public QWidget, protected Ui::AnimatorWidget {
		Q_OBJECT

	public:
		AnimatorWidget(QWidget *parent = 0);

		void setFrameInformation( int frm, int len );
		void setRateInformation( int minr, int maxr, int rate );
		void setModeEnabled( bool enable );
		void setPlaying( int play );
		bool isPlaying() const;
		int getRate() const;
		int getFrame() const;
		int getFrameCount() const;
		int getFrameStart() const;
		int getFrameEnd() const;
		int getStepSize() const;
		~AnimatorWidget();

        static int heightOpen( ) { return 107; }
        static int heightClosed( ) { return 25; }
        static int heightHeader( ) { return 27; }

	signals:
		void goTo(int frame);
		void setRate(int);
		void toStart();
		void revStep();
		void revPlay();
		void stop();
		void fwdStep();
		void fwdPlay();
		void toEnd();
		void frameNumberEdited( int );
		void lowerBoundChanged( int );
		void upperBoundChanged( int );
		void stepSizeChanged( int );

	public slots:
		void stopping();

	private slots:
		void frameNumberEdited();
		void movieLimitLowerChanged( int value );
		void movieLimitUpperChanged( int value );
		void sliderControl( int action );
		void endToEndMode( bool mode );
		void starting();
		void revStepping();

		void fwdStepping();
		void ending();
		void revPlaying();
		void fwdPlaying();

	private:
		int resetFrameBounded( int frameNumber ) const;
		void disableAll();
		void blockSignals( bool block );
		int frameCount;
		int play;
		bool rateNotSet;
	};
}
#endif // ANIMATORWIDGET_QO_H
