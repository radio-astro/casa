//# QtUtilities.cc: Common methods/utilities for Qt applications.
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
#include <casaqt/QtUtilities/QtUtilities.h>

#include <casaqt/QtUtilities/QtLayeredLayout.h>

#include <QBoxLayout>
#include <QStackedLayout>

namespace casa {

/////////////////////////////
// QTUTILITIES DEFINITIONS //
/////////////////////////////

void QtUtilities::putInFrame(QFrame* frame, QWidget* widget) {
    if(frame == NULL || widget == NULL) return;
    
    QHBoxLayout* l = new QHBoxLayout(frame);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);
    l->addWidget(widget);
}

void QtUtilities::putInFrame(QFrame* frame, QLayout* layout) {
    if(frame == NULL || layout == NULL) return;
    if(frame->layout() != NULL) delete frame->layout();
    frame->setLayout(layout);
}

QScrollArea* QtUtilities::putInScrollArea(QWidget* widget, bool showFrame) {
    if(widget == NULL) return NULL;
    
    QWidget* parent = widget->parentWidget();    
    QScrollArea* sa = new QScrollArea();
    
    if(parent != NULL && parent->layout() != NULL) {
        QLayout* l = parent->layout();
        int index = l->indexOf(widget);
        if(index >= 0) {
            // QLayout subclasses we know about, including our own.
            QBoxLayout* bl; QGridLayout* gl; QStackedLayout* sl;
            QtLayeredLayout* ll;
            
            if((bl = dynamic_cast<QBoxLayout*>(l)) != NULL) {
                bl->removeWidget(widget);
                bl->insertWidget(index, sa);
                
            } else if((gl = dynamic_cast<QGridLayout*>(l)) != NULL) {
                int row, col, rowSpan, colSpan;
                gl->getItemPosition(index, &row, &col, &rowSpan, &colSpan);
                gl->removeWidget(widget);                
                gl->addWidget(sa, row, col, rowSpan, colSpan);
                
            } else if((sl = dynamic_cast<QStackedLayout*>(l)) != NULL) {
                sl->removeWidget(widget);
                sl->insertWidget(index, widget);
                
            } else if((ll = dynamic_cast<QtLayeredLayout*>(l)) != NULL) {
                ll->removeWidget(widget);
                ll->insertWidget(index, sa);                
            }
        }
    }
    
    sa->setWidget(widget);
    sa->setWidgetResizable(true);
    sa->setSizePolicy(widget->sizePolicy());
    if(!showFrame) sa->setFrameShape(QFrame::NoFrame);
    
    return sa;
}

}
