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

#include <plotms/PlotMS/PlotMS.h>

#include <QInputDialog>

namespace casa {

/////////////////////////////////
// PLOTMSANNOTATOR DEFINITIONS //
/////////////////////////////////

// Public Methods //

PlotMSAnnotator::PlotMSAnnotator(PlotMS* parent, Mode startMode) :
        itsParent_(parent), itsMode_(startMode), itsAnnotateAction_(NULL) { }

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

void PlotMSAnnotator::setActive(bool active) {
    if(active != isActive()) {
        PlotMouseTool::setActive(active);
        PlotCanvas* c = canvas();
        if(c != NULL) {
            if(active && itsMode_ == RECTANGLE) c->setSelectLineShown(true);
            if(!active) {
                c->setCursor(NORMAL_CURSOR);
                c->setSelectLineShown(false);
            }
        }
    }
}

void PlotMSAnnotator::handleMouseEvent(const PlotEvent& event) {
    const PlotSelectEvent* se = dynamic_cast<const PlotSelectEvent*>(&event);
    const PlotMouseEvent* me = dynamic_cast<const PlotMouseEvent*>(&event);
    
    PlotCanvas* canvas;
    
    // SELECT EVENT //
    if(se != NULL && (canvas = se->canvas()) != NULL) {
        PlotFactory* f = factory();
        if(f == NULL) return;
        PlotRegion region = se->region();
        
        // For rectangle mode, draw rectangle where selection is.
        if(itsMode_ == RECTANGLE) {
            PlotShapeRectanglePtr rect= f->shapeRectangle(region.upperLeft(),
                    region.lowerRight());
            if(!itsRectLine_.null()) rect->setLine(itsRectLine_);
            if(!itsRectFill_.null()) rect->setAreaFill(itsRectFill_);
            itsARect_.insert(canvas, rect);
            canvas->plotItem(rect, ANNOTATION);
        }
        
    // MOUSE EVENT //
    } else if(me != NULL && (canvas = me->canvas()) != NULL) {
        PlotMouseEvent::Type type = me->type();
        PlotMouseEvent::Button button = me->button();
        
        // LEFT-CLICK EVENT //
        if(type == PlotMouseEvent::CLICK && button == PlotMouseEvent::SINGLE) {
            PlotFactory* f = factory();
            if(f == NULL) return;
            PlotCoordinate coord = me->where();

            // For text mode, ask for a String and draw it where click is.
            if(itsMode_ == TEXT) {
                QString str = QInputDialog::getText(itsParent_->getPlotter(),
                        "New Text Annotation",
                        "Enter the text for the new annotation:");
                if(!str.isEmpty()) {
                    PlotAnnotationPtr text = f->annotation(str.toStdString(),
                            coord);
                    if(!itsTextFont_.null()) text->setFont(itsTextFont_);
                    if(!itsTextOutline_.null())
                        text->setOutline(itsTextOutline_);
                    if(!itsTextFill_.null()) text->setBackground(itsTextFill_);
                    itsAText_.insert(canvas, text);
                    canvas->plotItem(text, ANNOTATION);
                }
            }
        
        // RIGHT-CLICK EVENT //
        }else if(type==PlotMouseEvent::CLICK&&button==PlotMouseEvent::CONTEXT){
            // Remove all attached rectangles/text on the given canvas.
            //canvas->holdDrawing();
            vector<PlotItemPtr> items;
            if(itsMode_ == RECTANGLE) {
                foreach(PlotShapeRectanglePtr rect, itsARect_.values(canvas))
                    items.push_back(rect);
                itsARect_.remove(canvas);
                
            } else if(itsMode_ == TEXT) {
                foreach(PlotAnnotationPtr text, itsAText_.values(canvas))
                    items.push_back(text);
                itsAText_.remove(canvas);
            }
            canvas->removePlotItems(items);
            //canvas->releaseDrawing();
            
        // PRESS/RELEASE EVENT //
        } else if(type==PlotMouseEvent::PRESS&&button==PlotMouseEvent::SINGLE&&
                 itsMode_ == RECTANGLE)
            canvas->setCursor(CROSSHAIR);
        else if(type==PlotMouseEvent::RELEASE&&button==PlotMouseEvent::SINGLE&&
                itsMode_ == RECTANGLE)
            canvas->setCursor(NORMAL_CURSOR);        
        
    }
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

void PlotMSAnnotator::attach(PlotCanvas* canvas) {
    PlotMouseTool::attach(canvas);
    if(canvas != NULL && isActive() && itsMode_ == RECTANGLE)
        canvas->setSelectLineShown(true);
}

void PlotMSAnnotator::detach() {
    PlotCanvas* c = canvas();
    if(c != NULL) {
        c->setCursor(NORMAL_CURSOR);
        c->setSelectLineShown(false);
    }
    PlotMouseTool::detach();
}

}
