//# TrackBox.qo.h: boxes used for cursor tracking for display data.
//# Copyright (C) 2013
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
//# $Id: QtDisplayPanelGui.cc,v 1.12 2006/10/10 21:59:19 dking Exp $

#include <display/QtViewer/TrackBox.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <QVBoxLayout>

namespace casa {
	TrackBox::TrackBox(QtDisplayData* qdd, QWidget* parent) :
		QGroupBox(parent), qdd_(qdd) {

		trkgEdit_ = new QTextEdit;

		new QVBoxLayout(this);
		layout()->addWidget(trkgEdit_);
		layout()->setMargin(1);

        connect( this, SIGNAL(toggled(bool)),  SLOT(visibility_event(bool)) );
		// (User can hide edit area with a checkbox by the track box title).


		setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);

		// (TrackBox as QroupBox)
		setFlat(true);
		setObjectName(qdd_->name().c_str());
		setTitle(objectName());
		setCheckable(True);
		setChecked(True);
		// setAlignment(Qt::AlignHCenter);
		// setAlignment(Qt::AlignRight);
		String tltp="Uncheck if you do not need to see position tracking data for\n"
		            + name() + "  (it will remain registered).";
		setToolTip(tltp.chars());

		trkgEdit_->setMinimumWidth(495);
		trkgEdit_->setFixedHeight( qdd_->isMS() ? 84 : 47 );
		// trkgEdit_->setFixedHeight(81);	// (obs.)
		//trkgEdit_->setPlainText("\n  ");	// (Doesn't work on init,
		//setTrackingHeight_();		// for some reason...).

		QFont trkgFont("Monospace");
		trkgFont.setStyleHint(QFont::TypeWriter);
		// trkgFont.setBold(True);
		trkgFont.setPixelSize(11);
		trkgEdit_->setFont(trkgFont);

		trkgEdit_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		trkgEdit_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		trkgEdit_->setLineWrapMode(QTextEdit::NoWrap);
		trkgEdit_->setReadOnly(True);
		trkgEdit_->setAcceptRichText(True);
	}


    TrackBox::~TrackBox( ) { }

    void TrackBox::setText( std::string trkgString) {
        trkgEdit_->setPlainText(QString::fromStdString(trkgString));
		// setTrackingHeight_();
	}



	void TrackBox::setTrackingHeight_() {
		// Set tracking edit height according to contents.
		// Note: setting a 'fixed' height is necessary to cause Dock Area
		// to return unneeded space.  'Fixed' size policy alone is _not_
		// adequate, apparently because [min.] size hints cannot be relied
		// upon to be recalculated correctly (or at least in a way that
		// makes sense to me...).  Issues outstanding with Trolltech (Qt).
		//
		// (Even the working behavior was broken again in Qt-4.2.0;  awaiting
		// further fixes before upgrading beyond 4.1.3).
		//
		// (Broken even in 4.1.3 is ability to use this routine to set the
		// initial trackbox height, before the widget is ever shown...).
		//
		// I don't understand all these issues.  As usual with Qt sizing,
		// this kludgy routine was arrived at by laborious trial-and-error....
		// (dk  11/06)

		trkgEdit_->setUpdatesEnabled(False);
		// temporarily disables widget painting, avoiding flicker
		// from next statement.

		trkgEdit_->setFixedHeight(1000);	// (More than is ever needed.
		// Necessary so that the cursorRect() call below will return the
		// proper height needed for the text, without truncating to the
		// widget's current height...).

		QTextCursor c = trkgEdit_->textCursor();
		c.movePosition(QTextCursor::End);
		trkgEdit_->setTextCursor(c);

		trkgEdit_->setFixedHeight(trkgEdit_->cursorRect().bottom()+10);
        trkgEdit_->setMinimumSize(QSize(300, 80));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        //sizePolicy.setHeightForWidth(AnimatorHolder->sizePolicy().hasHeightForWidth());
        trkgEdit_->setSizePolicy(sizePolicy);
        trkgEdit_->setFixedHeight(50);
        this->setFixedHeight(80);

		c.movePosition(QTextCursor::Start);
		trkgEdit_->setTextCursor(c);		// (assures left edge is visible
		trkgEdit_->ensureCursorVisible();	//  when tracking is updated).

		trkgEdit_->setUpdatesEnabled(True);
	}

    void TrackBox::visibility_event( bool visible ) {
        trkgEdit_->setVisible(visible);
        if ( visible ) {
            trkgEdit_->setFixedHeight( qdd_->isMS() ? 84 : 47 );
            setMinimumHeight( 90 );
            setMaximumHeight( 90 );
        } else {
            trkgEdit_->setFixedHeight( 0 );
            setMinimumHeight( 25 );
            setMaximumHeight( 25 );
        }
        emit visibilityChange( visible, qdd_ );
    }


}
