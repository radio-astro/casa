//# PlotMSAnnotator.cc: Annotator tool for PlotMS.
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
#include <plotms/Gui/PlotMSAnnotator.h>

namespace casa {

/////////////////////////////////
// PLOTMSANNOTATOR DEFINITIONS //
/////////////////////////////////

// Public Methods //

PlotMSAnnotator::PlotMSAnnotator(Mode startMode) : itsMode_(startMode),
        itsAnnotateAction_(NULL) { }

PlotMSAnnotator::~PlotMSAnnotator() { }


PlotMSAnnotator::Mode PlotMSAnnotator::drawingMode() const { return itsMode_; }
void PlotMSAnnotator::setDrawingMode(Mode mode) {
    itsMode_ = mode;
    QAction* a = itsModeActions_[mode];
    if(itsAnnotateAction_ == NULL || a == NULL) return;
    itsAnnotateAction_->setIcon(a->icon());
    itsAnnotateAction_->setText(a->text());
    itsAnnotateAction_->setIconText(a->iconText());
    itsAnnotateAction_->setToolTip(a->toolTip());
}

void PlotMSAnnotator::handleMouseEvent(const PlotEvent& event) {
    // TODO

    /*
    const PlotSelectEvent* se = dynamic_cast<const PlotSelectEvent*>(&event);    
    const PlotMouseEvent* me = dynamic_cast<const PlotMouseEvent*>(&event);
    const PlotWheelEvent* we = dynamic_cast<const PlotWheelEvent*>(&event);
    
    if(se != NULL && se->canvas() != NULL) {
        cout << "annotator select" << endl;
        
    } else if(me != NULL && me->canvas() != NULL) {
        PlotMouseEvent::Type type = me->type();
        PlotMouseEvent::Button button = me->button();
        if(type == PlotMouseEvent::CLICK && button == PlotMouseEvent::SINGLE)
            cout << "annotator click" << endl;
        else if(type == PlotMouseEvent::CLICK && button == PlotMouseEvent::CONTEXT)
            cout << "annotator right click" << endl;
        
    } else if(we != NULL && we->canvas() != NULL) {
        cout << "annotator wheel " << we->delta() << endl;
    }
    */
}


// Protected Methods //

void PlotMSAnnotator::setActions(QAction* annotateAction,
        const QMap<PlotMSAction::Type, QAction*>& actionMap) {
    itsAnnotateAction_ = annotateAction;
    itsModeActions_[TEXT] = actionMap.value(PlotMSAction::TOOL_ANNOTATE_TEXT);
    itsModeActions_[RECTANGLE] = actionMap.value(
            PlotMSAction::TOOL_ANNOTATE_RECTANGLE);
    setDrawingMode(itsMode_);
}

}
