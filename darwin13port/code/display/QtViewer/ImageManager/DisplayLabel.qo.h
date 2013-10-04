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
#ifndef DISPLAYLABEL_QO_H
#define DISPLAYLABEL_QO_H

#include <QTextEdit>
#include <QWidget>

namespace casa {

/**
 * Provides a quick view of a particular display property that has
 * been set.  For example, the data display type could be raster,
 * contour, or vector.  The DisplayLabel corresponding to
 * the display type will show 'R', 'C', or 'V'.
 */

class DisplayLabel : public QTextEdit {
    Q_OBJECT

public:
    DisplayLabel(int characterCount, QWidget *parent = 0);

    /**
     * Sets the background color that will be used when no
     * text is displayed.
     */
    void setEmptyColor( QColor color );

    /**
     * Returns whether or not the contents contains text.
     */
    bool isEmpty() const;


    ~DisplayLabel();

private:
    void setBackgroundColor( );
    QColor bgColor;
};

}

#endif // DISPLAYLABEL_QO_H
