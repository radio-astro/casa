//# PlotMSAnnotator.h: Annotator tool for PlotMS.
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
#ifndef PLOTMSANNOTATOR_H_
#define PLOTMSANNOTATOR_H_

#include <graphics/GenericPlotter/PlotTool.h>
#include <plotms/Actions/PlotMSAction.h>
#include <plotms/Plots/PlotMSPlotManager.h>

#include <QAction>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations.
class PlotMS;


// Subclass of PlotMouseTool for drawing/managing annotations on the plot
// canvases of PlotMS.
class PlotMSAnnotator : public PlotMouseTool, public PlotMSPlotManagerWatcher {
    
    //# Friend class declarations.
    friend class PlotMSPlotter;
    
public:
    // Static //
    
    // Drawing mode for the annotator.
    enum Mode {
        TEXT = 0,
        RECTANGLE = 1
    };
    
    
    // Non-Static //
    
    // Constructor which takes the PlotMS parent and optional starting mode.
    PlotMSAnnotator(PlotMS* parent, Mode startingMode = TEXT);
    
    // Destructor.
    ~PlotMSAnnotator();
    
    
    // Gets/Sets the current drawing mode.
    // <group>
    Mode drawingMode() const;
    void setDrawingMode(Mode mode);
    // </group>
    
    // Gets/Sets the current text properties (null means default for
    // PlotAnnotation object).  Only applies to future text annotations.
    // <group>
    PlotFontPtr textFont() const;
    void setTextFont(const PlotFontPtr font);
    PlotLinePtr textOutline() const;
    void setTextOutline(const PlotLinePtr outline);
    PlotAreaFillPtr textBackground() const;
    void setTextBackground(const PlotAreaFillPtr background);
    // </group>
    
    // Gets/Sets the current rectangle properties (null means default for
    // PlotShapeRectangle object).  Only applies to future rectangle
    // annotations.
    // <group>
    PlotLinePtr rectangleLine() const;
    void setRectangleLine(const PlotLinePtr line);
    PlotAreaFillPtr rectangleAreaFill() const;
    void setRectangleAreaFill(const PlotAreaFillPtr fill);
    // </group>
    
    // Clears all text/rectangle annotations off the given canvas.  If the
    // given canvas is NULL, then it clears them from all canvases.
    // <group>
    void clearText(PlotCanvas* canvas = NULL);
    void clearRectangles(PlotCanvas* canvas = NULL);
    void clearAll(PlotCanvas* canvas = NULL);
    // </group>
    
    
    // Overrides PlotTool::setActive().
    void setActive(bool isActive = true);
    
    // Implements PlotMouseTool::handleMouseEvent().
    void handleMouseEvent(const PlotEvent& event);
    
    // Implements PlotMSPlotManagerWatcher::plotChanged().  Removes annotations
    // that are attached to canvases that no longer exist.
    void plotsChanged(const PlotMSPlotManager& manager);
    
protected:
    // Sets the annotate and mode actions and the factory to the given.  MUST
    // be called before the annotator is used.
    void setActions(QAction* annotateAction,
            const QMap<PlotMSAction::Type, QAction*>& actionMap,
            PlotFactoryPtr factory);
    
    // Overrides PlotTool::attach().
    void attach(PlotCanvas* canvas);
    
    // Overrides PlotTool::detach().
    void detach();
    
private:
    // Parent.
    PlotMS* itsParent_;
    
    // Factory for generating plot objects.
    PlotFactoryPtr itsFactory_;
    
    // Current drawing mode.
    Mode itsMode_;
    
    // Actions to keep updated.
    // <group>
    QAction* itsAnnotateAction_;    
    QMap<Mode, QAction*> itsModeActions_;
    // </group>
    
    // Text annotations.
    QMultiMap<PlotCanvas*, PlotAnnotationPtr> itsAText_;
    
    // Current text properties.
    // <group>
    PlotFontPtr itsTextFont_;
    PlotLinePtr itsTextOutline_;
    PlotAreaFillPtr itsTextFill_;
    // </group>
    
    // Rectangle annotations.
    QMultiMap<PlotCanvas*, PlotShapeRectanglePtr> itsARect_;
    
    // Current rectangle properties.
    // <group>
    PlotLinePtr itsRectLine_;
    PlotAreaFillPtr itsRectFill_;
    // </group>
    
    
    // Sets properties to their defaults.
    void setDefaults();
};

}

#endif /* PLOTMSANNOTATOR_QO_H_ */
