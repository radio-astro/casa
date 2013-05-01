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
//# $Id: $


#ifndef DIRITEM_H
#define DIRITEM_H

#include <graphics/X11/X_enter.h>
#include <QList>
#include <QVariant>
#include <graphics/X11/X_exit.h>

namespace casa {

class DirItem
{
public:
    DirItem(const QList<QVariant> &data, DirItem *parent = 0);
    ~DirItem();

    void appendChild(DirItem *child);
    void removeChildAt(int child);
    void insertChildAt(int row, DirItem* child);

    DirItem *child(int row);
    int childCount() const;
    int columnCount() const;
    QVariant data(int column) const;
    int row() const;
    DirItem *parent();

private:
    QList<DirItem*> childItems;
    QList<QVariant> itemData;
    DirItem *parentItem;
};
}
#endif
