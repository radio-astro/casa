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

#include <plotms/Gui/PlotMSPlotter.qo.h>
#include <plotms/PlotMS/PlotMS.h>
#include <plotms/Plots/PlotMSPlot.h>

#include <QInputDialog>

namespace casa {

/////////////////////////////////
// PLOTMSANNOTATOR DEFINITIONS //
/////////////////////////////////

// Public Methods //

// itsFactory_ member cannot be initialized here yet.
PlotMSAnnotator::PlotMSAnnotator(PlotMS* parent, Mode startMode) :
        itsParent_(parent), itsMode_(startMode), itsAnnotateAction_(NULL) {
    parent->getPlotManager().addWatcher(this);
}

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


PlotFontPtr PlotMSAnnotator::textFont() const {
    if(itsTextFont_.null()) return PlotFontPtr();
    else                    return itsFactory_->font(*itsTextFont_);
}

void PlotMSAnnotator::setTextFont(const PlotFontPtr font) {
    if(itsTextFont_.null() != font.null() ||
       (!font.null() && *itsTextFont_ != *font)) {
        if(font.null()) itsTextFont_ = font;
        else            itsTextFont_ = itsFactory_->font(*font);
    }
}

PlotLinePtr PlotMSAnnotator::textOutline() const {
    if(itsTextOutline_.null()) return PlotLinePtr();
    else                       return itsFactory_->line(*itsTextOutline_);
}

void PlotMSAnnotator::setTextOutline(const PlotLinePtr outline) {
    if(itsTextOutline_.null() != outline.null() ||
       (!outline.null() && *itsTextOutline_ != *outline)) {
        if(outline.null()) itsTextOutline_ = outline;
        else               itsTextOutline_ = itsFactory_->line(*outline);
    }
}

PlotAreaFillPtr PlotMSAnnotator::textBackground() const {
    if(itsTextFill_.null()) return PlotAreaFillPtr();
    else                    return itsFactory_->areaFill(*itsTextFill_);
}

void PlotMSAnnotator::setTextBackground(const PlotAreaFillPtr background) {
    if(itsTextFill_.null() != background.null() ||
       (!background.null() && *itsTextFill_ != *background)) {
        if(background.null()) itsTextFill_ = background;
        else                  itsTextFill_= itsFactory_->areaFill(*background);
    }
}

PlotLinePtr PlotMSAnnotator::rectangleLine() const {
    if(itsRectLine_.null()) return PlotLinePtr();
    else                    return itsFactory_->line(*itsRectLine_);
}

void PlotMSAnnotator::setRectangleLine(const PlotLinePtr line) {
    if(itsRectLine_.null() != line.null() ||
       (!line.null() && *itsRectLine_ != *line)) {
        if(line.null()) itsRectLine_ = line;
        else            itsRectLine_ = itsFactory_->line(*line);
    }
}

PlotAreaFillPtr PlotMSAnnotator::rectangleAreaFill() const {
    if(itsRectFill_.null()) return PlotAreaFillPtr();
    else                    return itsFactory_->areaFill(*itsRectFill_);
}

void PlotMSAnnotator::setRectangleAreaFill(const PlotAreaFillPtr fill) {
    if(itsRectFill_.null() != fill.null() ||
       (!fill.null() && *itsRectFill_ != *fill)) {
        if(fill.null()) itsRectFill_ = fill;
        else            itsRectFill_ = itsFactory_->areaFill(*fill);
    }
}

void PlotMSAnnotator::clearText(PlotCanvas* canvas) {
    vector<PlotItemPtr> items;
    if(canvas == NULL) {
        foreach(PlotCanvas* canvas, itsAText_.keys()) {
            items.clear();
            foreach(PlotAnnotationPtr text, itsAText_.values(canvas))
                items.push_back(text);
            canvas->removePlotItems(items);
        }
        itsAText_.clear();
    } else if(itsAText_.contains(canvas)) {
        foreach(PlotAnnotationPtr text, itsAText_.values(canvas))
            items.push_back(text);
        canvas->removePlotItems(items);
        itsAText_.remove(canvas);
    }
}

void PlotMSAnnotator::clearRectangles(PlotCanvas* canvas) {
    vector<PlotItemPtr> items;
    if(canvas == NULL) {
        foreach(PlotCanvas* canvas, itsARect_.keys()) {
            items.clear();
            foreach(PlotShapeRectanglePtr rect, itsARect_.values(canvas))
                items.push_back(rect);
            canvas->removePlotItems(items);
        }
        itsARect_.clear();
    } else if(itsARect_.contains(canvas)) {
        foreach(PlotShapeRectanglePtr rect, itsARect_.values(canvas))
            items.push_back(rect);
        canvas->removePlotItems(items);
        itsARect_.remove(canvas);
    }
}

void PlotMSAnnotator::clearAll(PlotCanvas* canvas) {
	(void)canvas;
	
    QMap<PlotCanvas*, vector<PlotItemPtr> > items;
    
    foreach(PlotCanvas* canvas, itsAText_.keys()) {
        if(!items.contains(canvas)) items[canvas] = vector<PlotItemPtr>();
        foreach(PlotAnnotationPtr text, itsAText_.values(canvas))
            items[canvas].push_back(text);
    }
    itsAText_.clear();
    
    foreach(PlotCanvas* canvas, itsARect_.keys()) {
        items[canvas] = vector<PlotItemPtr>();
        foreach(PlotShapeRectanglePtr rect, itsARect_.values(canvas))
            items[canvas].push_back(rect);
    }
    itsARect_.clear();
    
    foreach(PlotCanvas* canvas, items.keys())
        canvas->removePlotItems(items[canvas]);
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
        PlotRegion region = se->region();
        
        // For rectangle mode, draw rectangle where selection is.
        if(itsMode_ == RECTANGLE) {
            PlotShapeRectanglePtr rect = itsFactory_->shapeRectangle(
                    region.upperLeft(), region.lowerRight());
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
            PlotCoordinate coord = me->where();

            // For text mode, ask for a String and draw it where click is.
            if(itsMode_ == TEXT) {
                QString str = QInputDialog::getText(itsParent_->getPlotter(),
                        "New Text Annotation",
                        "Enter the text for the new annotation:");
                if(!str.isEmpty()) {
                    PlotAnnotationPtr text = itsFactory_->annotation(
                            str.toStdString(), coord);
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
            if(itsMode_ == TEXT) clearText(canvas);
            else if(itsMode_ == RECTANGLE) clearRectangles(canvas);
            
        // PRESS/RELEASE EVENT //
        } else if(type==PlotMouseEvent::PRESS&&button==PlotMouseEvent::SINGLE&&
                 itsMode_ == RECTANGLE)
            canvas->setCursor(CROSSHAIR);
        else if(type==PlotMouseEvent::RELEASE&&button==PlotMouseEvent::SINGLE&&
                itsMode_ == RECTANGLE)
            canvas->setCursor(NORMAL_CURSOR);        
        
    }
}

void PlotMSAnnotator::plotsChanged(const PlotMSPlotManager& manager) {
    // Clear out any annotations that are on canvases that are no longer in the
    // plotter.
    
    // Get all unique canvases from plots.
    vector<PlotCanvasPtr> canv, temp;
    bool found;
    unsigned int n = manager.numPlots();
    for(unsigned int i = 0; i < n; i++) {
        temp = manager.plot(i)->canvases();
        for(unsigned int j = 0; j < temp.size(); j++) {
            if(temp[j].null()) continue;
            found = false;
            for(unsigned int k = 0; !found && k < canv.size(); k++)
                if(canv[k] == temp[j]) found = true;
            if(!found) canv.push_back(temp[j]);
        }
    }
    
    // Check that stored canvases are in the list.
    n = canv.size();
    foreach(PlotCanvas* c, itsAText_.keys()) {
        found = false;
        for(unsigned int i = 0; !found && i < n; i++)
            if(c == &*canv[i]) found = true;
        if(!found) itsAText_.remove(c);
    }
    
    foreach(PlotCanvas* c, itsARect_.keys()) {
        found = false;
        for(unsigned int i = 0; !found && i < n; i++)
            if(c == &*canv[i]) found = true;
        if(!found) itsARect_.remove(c);
    }
}


// Protected Methods //

void PlotMSAnnotator::setActions(QAction* annotateAction,
        const QMap<PlotMSAction::Type, QAction*>& actionMap,
        PlotFactoryPtr factory) {
    itsAnnotateAction_ = annotateAction;
    itsModeActions_[TEXT] = actionMap.value(PlotMSAction::TOOL_ANNOTATE_TEXT);
    itsModeActions_[RECTANGLE] = actionMap.value(
            PlotMSAction::TOOL_ANNOTATE_RECTANGLE);
    itsFactory_ = factory;
    setDrawingMode(itsMode_);
    setDefaults();
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


// Private Methods //

void PlotMSAnnotator::setDefaults() {
    itsTextFont_ = PMS::DEFAULT_ANNOTATION_TEXT_FONT(itsFactory_);
    itsTextOutline_ = PMS::DEFAULT_ANNOTATION_TEXT_OUTLINE(itsFactory_);
    itsTextFill_ = PMS::DEFAULT_ANNOTATION_TEXT_BACKGROUND(itsFactory_);
    itsRectLine_ = PMS::DEFAULT_ANNOTATION_RECT_LINE(itsFactory_);
    itsRectFill_ = PMS::DEFAULT_ANNOTATION_RECT_FILL(itsFactory_);
}

}
