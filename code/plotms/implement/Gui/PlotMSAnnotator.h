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

#include <QAction>

#include <casa/namespace.h>

namespace casa {

// Subclass of PlotMouseTool for drawing/managing annotations on the plot
// canvases of PlotMS.
class PlotMSAnnotator : public PlotMouseTool {
    
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
    
    // Constructor which optionally takes the starting mode.
    PlotMSAnnotator(Mode startingMode = TEXT);
    
    // Destructor.
    ~PlotMSAnnotator();
    
    
    // Gets/Sets the current drawing mode.
    // <group>
    Mode drawingMode() const;
    void setDrawingMode(Mode mode);
    // </group>
    
    // Implements PlotMouseTool::handleMouseEvent().
    void handleMouseEvent(const PlotEvent& event);
    
protected:
    // Sets the annotate and mode actions to the given.  MUST be called before
    // the annotator is used.
    void setActions(QAction* annotateAction,
            const QMap<PlotMSAction::Type, QAction*>& actionMap);
    
private:
    // Current drawing mode.
    Mode itsMode_;
    
    // Actions to keep updated.
    // <group>
    QAction* itsAnnotateAction_;    
    QMap<Mode, QAction*> itsModeActions_;
    // </group>
    
    // Text annotations.
    QMultiMap<PlotCanvas*, PlotAnnotationPtr> itsAText_;
    
    // Rectangle annotations.
    QMultiMap<PlotCanvas*, PlotShapeRectanglePtr> itsARect_;
};

}

#endif /* PLOTMSANNOTATOR_QO_H_ */
