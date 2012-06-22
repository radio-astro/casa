//# QtLayeredLayout.cc: Subclass of QLayout to have layered widgets.
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
#include <casaqt/QtUtilities/QtLayeredLayout.h>

#include <QWidget>

namespace casa {

/////////////////////////////////
// QTLAYEREDLAYOUT DEFINITIONS //
/////////////////////////////////

bool QtLayeredLayout::propagateEventType(QEvent::Type type) {
    return type == QEvent::ContextMenu || type == QEvent::Enter ||
           type == QEvent::KeyPress || type == QEvent::KeyRelease ||
           type == QEvent::Leave || type == QEvent::MouseButtonDblClick ||
           type == QEvent::MouseButtonPress ||
           type == QEvent::MouseButtonRelease || type == QEvent::MouseMove ||
           type == QEvent::Wheel;
}


QtLayeredLayout::QtLayeredLayout() : QLayout(), itsPropagateEvents_(false) { }

QtLayeredLayout::QtLayeredLayout(QWidget* parent) : QLayout(parent),
        itsPropagateEvents_(false) { }

QtLayeredLayout::~QtLayeredLayout() {
    QLayoutItem* item;
    while((item = takeAt(0)) != NULL) delete item;
}


QRect QtLayeredLayout::geometry() const { return itsGeometry_; }

bool QtLayeredLayout::isEmpty() const { return itsItems_.isEmpty(); }

void QtLayeredLayout::setGeometry(const QRect& r) {
    QLayout::setGeometry(r);
    for(int i = 0; i < itsItems_.size(); i++)
        itsItems_[i]->setGeometry(r);
    itsGeometry_ = r;
}

QSize QtLayeredLayout::sizeHint() const { return minimumSize(); }


bool QtLayeredLayout::eventFilter(QObject* watched, QEvent* event) {
    if(!itsPropagateEvents_ || event == NULL ||
       !propagateEventType(event->type())) return false;
    
    QWidget* w = dynamic_cast<QWidget*>(watched);
    if(w == NULL) return false;
    
    int index = layeredIndexOf(w);    
    if(index < 0 || index >= itsItems_.size()) return false;
    
    QWidget* nextWidget = NULL;
    for(int i = index; i >= 0; i--) {
        if(itsItems_[i]->widget()) {
            nextWidget = itsItems_[i]->widget();
            break;
        }
    }
    if(nextWidget == NULL) return false;
    
    watched->event(event);
    if(!event->isAccepted()) return eventFilter(nextWidget, event);
    else return true;
}


void QtLayeredLayout::addItem(QLayoutItem* item) { insertItem(-1, item); }

int QtLayeredLayout::count() const { return itsItems_.count(); }

Qt::Orientations QtLayeredLayout::expandingDirections() const {
    Qt::Orientations o = 0;    
    for(int i = 0; i < itsItems_.size(); i++)
        o |= itsItems_[i]->expandingDirections();
    return o;
}

int QtLayeredLayout::indexOf(QWidget* widget) const {
    for(int i = 0; i < itsItems_.size(); i++)
        if(itsItems_[i]->widget() == widget) return i;
    return -1;
}

QLayoutItem* QtLayeredLayout::itemAt(int index) const {
    return itsItems_.value(index); }

QSize QtLayeredLayout::maximumSize() const {
    QSize maxSize, temp;
    
    for(int i = 0; i < itsItems_.size(); i++) {
        temp = itsItems_[i]->maximumSize();
        if(temp.isValid()) {
            if(!maxSize.isValid()) maxSize = temp;
            else maxSize = maxSize.boundedTo(temp);
        }
    }
    
    return maxSize;
}

QSize QtLayeredLayout::minimumSize() const {
    QSize minSize, temp;
    
    for(int i = 0; i < itsItems_.size(); i++) {
        temp = itsItems_[i]->minimumSize();
        if(temp.isValid()) {
            if(!minSize.isValid()) minSize = temp;
            else minSize = minSize.expandedTo(temp);
        }
    }
    
    return minSize;
}

QLayoutItem* QtLayeredLayout::takeAt(int index) {
    if(index >= 0 && index < itsItems_.size()) {
        QLayoutItem* i = itsItems_.takeAt(index);
        if(i != NULL) {
            redoParentStack();
            if(itsPropagateEvents_) installOrRemoveSelf(i, false);
        }
        return i;
    }
    else return NULL;
}


void QtLayeredLayout::insertItem(int index, QLayoutItem* item) {
    if(item != NULL && !itsItems_.contains(item)) {
        if(index < 0 || index >= itsItems_.size()) itsItems_.append(item);
        else itsItems_.insert(index, item);

        redoParentStack();
        
        // Add event filter if needed.
        if(itsPropagateEvents_) installOrRemoveSelf(item, true);
    }
}

void QtLayeredLayout::insertWidget(int index, QWidget* widget) {
    if(widget != NULL) insertItem(index, new QWidgetItem(widget));
}


void QtLayeredLayout::installOrRemoveSelf(QLayoutItem* item, bool install) {
    if(item == NULL) return;
    else if(item->widget()) {
        if(install) item->widget()->installEventFilter(this);
        else        item->widget()->removeEventFilter(this);
    }
    else if(item->layout()) {
        QLayout* l = item->layout();
        for(int i = 0; i < l->count(); i++)
            installOrRemoveSelf(l->itemAt(i), install);
    }
}

void QtLayeredLayout::redoParentStack() {
    QWidget* lastWidget = NULL, *temp;
    for(int i = itsItems_.size() - 1; i >= 0; i--) {
        temp = itsItems_[i]->widget();
        if(temp != NULL) {
            if(lastWidget == NULL) temp->raise();
            else temp->stackUnder(lastWidget);
            lastWidget = temp;
        }
    }
}

int QtLayeredLayout::layeredIndexOf(QWidget* widget) const {
    if(itsItems_.size() == 0) return -1;
    else return layeredIndexOf(widget, const_cast<QtLayeredLayout*>(this));
}

int QtLayeredLayout::layeredIndexOf(QWidget* widget, QLayoutItem* item) {
    // Invalid.
    if(widget == NULL || item == NULL) return -1;
    
    // Base case: widget.
    if(item->widget() && item->widget() == widget) return 0;
    
    // Recursive case: layout.
    else if(item->layout()) {
        QLayout* l = item->layout();
        for(int i = 0; i < l->count(); i++) {
            if(layeredIndexOf(widget, l->itemAt(i)) >= 0) return i;
        }
    }
    
    // Other.
    return -1;
}

}
