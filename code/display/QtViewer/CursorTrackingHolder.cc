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
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <display/QtViewer/CursorTrackingHolder.qo.h>
#include <display/QtViewer/TrackBox.qo.h>
#include <display/DisplayDatas/PrincipalAxesDD.h>
#include <QDebug>

namespace casa {

    CursorTrackingHolder::CursorTrackingHolder( QtDisplayPanelGui *panel, QWidget *parent )
         : InActiveDock(parent), Ui::CursorTrackingHolder( ), panel_(panel), dismissed(false) {
		setupUi(this);
		connect( this, SIGNAL(visibilityChanged(bool)), SLOT(handle_visibility(bool)) );
	}

	CursorTrackingHolder::~CursorTrackingHolder( ) {

	}

    void CursorTrackingHolder::arrangeTrackBoxes( ) {

		// Reacts to QDP registration change signal.  If necessary, changes
		// the set of cursor position tracking boxes being displayed in
		// container (creating new TrackBoxes as necessary).  A TrackBox
		// will be shown for each qdd in panel_->displayPanel( )->registeredDDs() where
		// qdd->usesTracking() (in the same order).

		// Hide track boxes whose dd has been unregistered and remove them
		// from the container's layout.  (They remain parented to container
		// until deleted, though).
		QList<TrackBox*> trkBoxes = container->findChildren<TrackBox*>();
		for ( int i=0; i<trkBoxes.size(); i++ ) {
			TrackBox* trkBox = trkBoxes[i];
			if( trkBox->isVisibleTo(container) &&
			        !panel_->displayPanel( )->isRegistered(trkBox->dd()) ) {
				container->layout()->removeWidget(trkBox);
				trkBox->hide();
			}
		}

		// Assure that all applicable registered QDDs are showing track boxes.
		DisplayDataHolder::DisplayDataIterator iter = panel_->displayPanel( )->beginRegistered();
		int i = 0;
		while ( iter != panel_->displayPanel( )->endRegistered()) {
            addTrackBox(*iter, i);
			iter++;
			i++;
		}
    }

    TrackBox *CursorTrackingHolder::addTrackBox( QtDisplayData *qdd, int position ) {
		// If qdd->usesTracking(), this method assures that a TrackBox for qdd
		// is visible in the container's layout (creating the TrackBox if it
		// didn't exist).  Used by arrangeTrackBoxes_() above.  Returns the
		// TrackBox (or 0 if none, i.e., if !qdd->usesTracking()).

		if ( ! qdd->usesTracking( ) ) return 0;	// (track boxes N/A to qdd)

		TrackBox *trkBox = container->findChild<TrackBox*>(QString::fromStdString(qdd->name( )));
		Bool notShown = trkBox==0 || trkBox->isHidden( );

		if ( trkBox == 0 ) {
            trkBox = new TrackBox( qdd );
            connect( trkBox, SIGNAL(visibilityChange(bool,QtDisplayData*)), SLOT(handle_folding(bool, QtDisplayData*)));
        } else if ( notShown ) {
            trkBox->clear( );	// (Clear old, hidden trackbox).
        }


		if ( notShown ) {
			QBoxLayout* containerLayout = dynamic_cast<QBoxLayout*>( container->layout());
			if ( position < 0 ){
				containerLayout->addWidget( trkBox );
			}
			else {
				//Keep the order of the track box consistent with the order of
				//the images in the animator and the image manager.
				containerLayout->insertWidget( position, trkBox );
			}
			trkBox->show( );
		}

        update_size( );
		return trkBox;
    }

    void CursorTrackingHolder::removeTrackBox( QtDisplayData *qdd ) {
		// Deletes the TrackBox for the given QDD if it exists.  Deletion
		// automatically removes it from the gui (trkgWidget_ and its layout).
		// Connected to the ddRemoved() signal of QtViewerBase.
        delete container->findChild<TrackBox*>(QString::fromStdString(qdd->name( )));
        update_size( );
    }

    void CursorTrackingHolder::display( const Record &trackingRec ) {
		// Display tracking data gathered by underlying panel.
		for ( uInt i=0; i < trackingRec.nfields(); i++ ) {
            TrackBox* trkBox = container->findChild<TrackBox*>( trackingRec.name(i).c_str( ) );
			if ( trkBox != 0 ) {
				trkBox->setText( trackingRec.asString(i) );
			}
		}
    }

    void CursorTrackingHolder::cursorUpdate( const std::vector<double> &wpt, QtDisplayData *dd ) {
        TrackBox *track = container->findChild<TrackBox*>(QString::fromStdString(dd->name( )));
        PrincipalAxesDD *padd = dynamic_cast<PrincipalAxesDD*>(dd->dd( ));
        if ( track && padd ) {
            stringstream ss;
            ss << padd->showValue(wpt);
            // if the first string is shorter than a typical value, add spaces...
            if(ss.tellp() < 23) while(ss.tellp() < 23) ss << ' ';
            // ...otherwise add a tab
            else ss << '\t';
            // add position information...
            ss << padd->showPosition( wpt );
            track->setText(ss.str( ));
        }
    }

	void CursorTrackingHolder::dismiss( ) {
        hide( );
		dismissed = true;
	}

	void CursorTrackingHolder::closeEvent ( QCloseEvent * event ) {
		dismissed = true;
		QDockWidget::closeEvent(event);
		panel_->putrc( "visible.cursor_tracking", "false" );
	}

    void CursorTrackingHolder::handle_folding( bool /*visible*/, QtDisplayData */*dd*/ ) {
    	update_size( );
    }

	void CursorTrackingHolder::handle_visibility( bool visible ) {
		if ( visible && dismissed ) {
			dismissed = false;
			panel_->putrc( "visible.cursor_tracking", "true" );
		}
	}

    QSize CursorTrackingHolder::find_size( ) const {
        QSize result(size( ));
        QList<TrackBox*> trkBoxes = container->findChildren<TrackBox*>();
        if ( trkBoxes.size( ) > 0 ) {
            result.setHeight( TrackBox::heightHeader( ) );
            for ( int i=0; i < trkBoxes.size( ); ++i ) {
                if ( trkBoxes[i]->isVisible( ) ) {
                    if ( trkBoxes[i]->isChecked( ) )
                        result.setHeight(result.height( ) + TrackBox::heightOpen( ) );
                    else
                        result.setHeight(result.height( ) + TrackBox::heightClosed( ) );
                }
            }
        }
        return result;
    }

    void CursorTrackingHolder::setVisible( bool visible ){
    	QDockWidget::setVisible(visible);
    	if( visible ){
    		update_size();
    	}
    }

    void CursorTrackingHolder::update_size( ) {
        QSize new_size = find_size( );
        setMinimumHeight(new_size.height( ));
        setMaximumHeight(new_size.height( ));
        updateGeometry( );
    }
}
