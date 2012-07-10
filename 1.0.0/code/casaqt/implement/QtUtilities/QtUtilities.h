//# QtUtilities.h: Common methods/utilities for Qt applications.
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
//# $Id: $
#ifndef QTUTILITIES_H_
#define QTUTILITIES_H_

#include <QScrollArea>

//#include <casa/namespace.h>

namespace casa {

class QtUtilities {
public:
    // Puts the given widget into the given frame.
    static void putInFrame(QFrame* frame, QWidget* widget);
    
    // Puts the given layout into the given frame.  If the given frame already
    // has a layout, it is deleted.
    static void putInFrame(QFrame* frame, QLayout* layout);
    
    // Puts the given widget into a QScrollArea, then replaces the widget in
    // its parent's layout (if applicable), then returns the scroll area.
    static QScrollArea* putInScrollArea(QWidget* widget,
            bool showFrame = false);
};

}

#endif /* QTUTILITIES_H_ */
