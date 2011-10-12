//# QtPanelBase.qo.h: base class for independent (main window) panels whether plotter or viewer
//# with surrounding Gui functionality
//# Copyright (C) 2009
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
//# $Id: QtPanelBase.qo.h,v 1.7 2006/10/10 21:42:05 dking Exp $

#ifndef QTPANELBASE_H_
#define QTPANELBASE_H_

 #include <QMainWindow>

namespace casa {

    class QtPanelBase : public QMainWindow {
    public:
	QtPanelBase( QWidget *parent=0 ) : QMainWindow(parent), close_override(false) { }

	// Used to close this panel (i.e. QMainWindow)... This function
	// should be used instead of close() to programmatically close
	// the window, i.e. cause it to no longer be valid.
	virtual void closeMainPanel( );

	// Used to indicate that "closing" the window should be reinterpreted
	// as "hiding" the window, i.e. QtDBusViewerAdaptor is done with the
	// window so when the user closes it it should go away.
	virtual void releaseMainPanel( );

    protected:
	bool isOverridedClose( ) const { return close_override; }

    private:
	bool close_override;
    };
}

#endif
