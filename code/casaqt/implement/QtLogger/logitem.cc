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
//# Correspondence concerning AIPS++ should be addressed as follow s:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id: $

#include <casaqt/QtLogger/logitem.h>

#include <graphics/X11/X_enter.h>
#include <QStringList>
#include <QDebug>
#include <graphics/X11/X_exit.h>

namespace casa {

LogItem::LogItem(const QList<QVariant> &data, LogItem *parent)
{
    parentItem = parent;
    itemData = data;
}

LogItem::~LogItem()
{
    qDeleteAll(childItems);
}

void LogItem::appendChild(LogItem *item)
{
    childItems.append(item);
}

void LogItem::removeChildAt(int item)
{
    //qDebug() << "remove at=" << item;
    childItems.removeAt(item);
}
void LogItem::insertChildAt(int item, LogItem* child)
{
    //qDebug() << "insert at=" << item;
    childItems.insert(item, child);
}
LogItem *LogItem::child(int row)
{
    return childItems.value(row);
}

int LogItem::childCount() const
{
    return childItems.count();
}

int LogItem::columnCount() const
{
    return itemData.count();
}

QVariant LogItem::data(int column) const
{
    return itemData.value(column);
}

LogItem *LogItem::parent()
{
    return parentItem;
}

int LogItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<LogItem*>(this));

    return 0;
}

}
