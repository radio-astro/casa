//# QtLayeredLayout.h: Subclass of QLayout to have layered widgets.
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
#ifndef QTLAYEREDLAYOUT_H_
#define QTLAYEREDLAYOUT_H_

#include <casa/BasicSL/String.h>

#include <QEvent>
#include <QLayout>

#include <casa/namespace.h>
using namespace std;

namespace casa {

// Subclass of QLayout to have multiple widgets layered on top of each other.
// Since the top widget will be the only one who receives certain types of Qt
// events, the layout can also propagate events to lower children if desired.
// NOTE: EVENT PROPAGATION IS NOT CURRENTLY WORKING.
class QtLayeredLayout : public QLayout {
public:
    // Static //
    
    // Returns true if the given event type is propagate-able, false otherwise.
    static bool propagateEventType(QEvent::Type type);
    
    
    // Non-Static //
    
    // Constructor which takes no parent.
    QtLayeredLayout();
    
    // Constructor which takes a parent (cannot be null).
    QtLayeredLayout(QWidget* parent);
    
    // Destructor.
    ~QtLayeredLayout();
    
    
    // QLayoutItem Methods //
    
    // Implements QLayoutItem::geometry().  Returns the last geometry set using
    // setGeometry().
    QRect geometry() const;
    
    // Implements QLayoutItem::isEmpty().
    bool isEmpty() const;
    
    // Implements QLayoutItem::setGeometry().  Sets the geometry of all
    // children to the given.
    void setGeometry(const QRect& r);
    
    // Implements QLayoutItem::sizeHint().  Returns minimumSizeHint().
    QSize sizeHint() const;
    
    
    // QObject Methods //
    
    // Overrides QObject::eventFilter() to propagate events as needed.
    virtual bool eventFilter(QObject* watched, QEvent* event);
    
    
    // QLayout Methods //
    
    // Implements QLayout::addItem().  Does not add duplicates.
    void addItem(QLayoutItem* item);
    
    // Implements QLayout::count().
    int count() const;
    
    // Overrides QLayout::expandingDirections().
    Qt::Orientations expandingDirections() const;
    
    // Overrides QLayout::indexOf().
    int indexOf(QWidget* widget) const;
    
    // Implements QLayout::itemAt().
    QLayoutItem* itemAt(int index) const;
    
    // Overrides QLayout::maximumSize().  Returns the smallest valid maximum
    // size for its items.
    QSize maximumSize() const;
    
    // Overrides QLayout::minimumSize().  Returns the largest valid minimum
    // size for its items.
    QSize minimumSize() const;
    
    // Implements QLayout::takeAt().
    QLayoutItem* takeAt(int index);
    
    
    // QtLayeredLayout Methods //
    
    // Inserts the given item at the given index.  If index is outside the
    // bounds of the item list, the item is inserted at the end.  Null or
    // duplicate items are not inserted.
    // <group>
    void insertItem(int index, QLayoutItem* item);
    void insertWidget(int index, QWidget* widget);
    // </group>
    
private:
    // Items.
    QList<QLayoutItem*> itsItems_;
    
    // Last set geometry.
    QRect itsGeometry_;
    
    // Flag to propagate events or not.
    bool itsPropagateEvents_;
    
    
    // Installs or removes this layout as an event filter on the given item.
    void installOrRemoveSelf(QLayoutItem* item, bool install);
    
    // Redoes the sibling widget stack in the parent, if applicable.
    void redoParentStack();

    // Returns the layered index of the given widget, using recursive
    // parameters.
    // <group>
    int layeredIndexOf(QWidget* widget) const;
    static int layeredIndexOf(QWidget* widget, QLayoutItem* item);
    // </group>
};

}

#endif /* QTLAYEREDLAYOUT_H_ */
