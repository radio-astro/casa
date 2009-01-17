//# QPTool.cc: Qwt implementations of PlotTools.
//# Copyright (C) 2008
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
#ifdef AIPS_HAS_QWT
#include <casaqt/QwtPlotter/QPTool.qo.h>

#include <casaqt/QwtPlotter/QPCanvas.qo.h>

namespace casa {

//////////////////////////////
// QPSELECTTOOL DEFINITIONS //
//////////////////////////////

QPSelectTool::QPSelectTool(PlotCoordinate::System sys): PlotSelectTool(sys) { }

QPSelectTool::QPSelectTool(PlotAxis xAxis, PlotAxis yAxis,
        PlotCoordinate::System sys) : PlotSelectTool(xAxis, yAxis, sys) { }

QPSelectTool::~QPSelectTool() { }


////////////////////////////
// QPZOOMTOOL DEFINITIONS //
////////////////////////////

QPZoomTool::QPZoomTool(PlotCoordinate::System sys) : PlotZoomTool(sys) { }

QPZoomTool::QPZoomTool(PlotAxis xAxis, PlotAxis yAxis,
        PlotCoordinate::System sys) : PlotZoomTool(xAxis, yAxis, sys) { }

QPZoomTool::~QPZoomTool() { }


///////////////////////////
// QPPANTOOL DEFINITIONS //
///////////////////////////

QPPanTool::QPPanTool(PlotCoordinate::System sys) : PlotPanTool(sys),
        m_panner(NULL) { }

QPPanTool::QPPanTool(PlotAxis xAxis, PlotAxis yAxis, PlotCoordinate::System sys) :
        PlotPanTool(xAxis, yAxis, sys), m_panner(NULL) { }

QPPanTool::~QPPanTool() {
    if(m_panner != NULL) delete m_panner;
}


void QPPanTool::setActive(bool isActive) {
    PlotPanTool::setActive(isActive);
    if(m_canvas != NULL) m_panner->setEnabled(isActive);
}


void QPPanTool::handleMouseEvent(const PlotEvent& event) {
    m_lastEventHandled = true;
    if(m_canvas == NULL) {
        m_lastEventHandled = false;
        return;
    }
    
    const PlotWheelEvent* w; const PlotClickEvent* c;
    const PlotMouseEvent* m;
    
    // handle wheel
    if((w = dynamic_cast<const PlotWheelEvent*>(&event)) != NULL) {
        PlotRegion r = m_stack->moveAndReturn(w->delta());
        m_canvas->setAxesRegion(r, m_stack->currentXAxis(),
                                m_stack->currentYAxis());
        notifyWatchers();
        
    // handle right-click
    } else if((c = dynamic_cast<const PlotClickEvent*>(&event)) != NULL &&
            c->button() == PlotClickEvent::CONTEXT) {
        PlotRegion r = m_stack->moveAndReturn(0);
        m_canvas->setAxesRegion(r, m_stack->currentXAxis(),
                                m_stack->currentYAxis());
        notifyWatchers();
        
        // cursor niceness
    } else if((m = dynamic_cast<const PlotMouseEvent*>(&event)) != NULL &&
              m->type() == PlotMouseEvent::PRESS)
        m_canvas->setCursor(HAND_CLOSED);
    else if(m != NULL && m->type() == PlotMouseEvent::RELEASE)
        m_canvas->setCursor(HAND_OPEN);
}


// Protected Methods //

void QPPanTool::attach(PlotCanvas* canvas) {
    QPCanvas* c = dynamic_cast<QPCanvas*>(canvas);
    if(c != NULL) {
        PlotPanTool::attach(c);
        if(m_panner != NULL) delete m_panner;
        m_panner = new QwtPlotPanner(c->asQwtPlot().canvas());
        m_panner->setEnabled(m_active);
        
        // set axes
        m_panner->setAxisEnabled(QwtPlot::yLeft, false);
        m_panner->setAxisEnabled(QwtPlot::yRight, false);
        m_panner->setAxisEnabled(QwtPlot::xBottom, false);
        m_panner->setAxisEnabled(QwtPlot::xTop, false);
        m_panner->setAxisEnabled(QPOptions::axis(m_xAxis), true);
        m_panner->setAxisEnabled(QPOptions::axis(m_yAxis), true);
        
        connect(m_panner, SIGNAL(panned(int, int)), SLOT(panned(int, int)));
    }
}


// Private Slots //

void QPPanTool::panned(int px, int py) {
    QPCanvas* c = dynamic_cast<QPCanvas*>(m_canvas);
    if(c != NULL) {
        m_stack->addRegion(c->axesRanges(m_xAxis, m_yAxis), m_xAxis, m_yAxis);
        notifyWatchers();
    }
}


///////////////////////////////
// QPTRACKERTOOL DEFINITIONS //
///////////////////////////////

QPTrackerTool::QPTrackerTool(PlotCoordinate::System sys) :
        PlotTrackerTool(sys), m_tracker(NULL) { }

QPTrackerTool::QPTrackerTool(PlotAxis xAxis, PlotAxis yAxis,
        PlotCoordinate::System sys) : PlotTrackerTool(xAxis, yAxis, sys),
        m_tracker(NULL) { }

QPTrackerTool::~QPTrackerTool() {
    if(m_tracker != NULL) delete m_tracker;
}


void QPTrackerTool::setActive(bool isActive) {
    if(isActive != m_active) {
        PlotMouseTool::setActive(isActive);
        if(m_tracker != NULL) {
            m_tracker->setTrackerMode((isActive && m_drawText) ?
                       QwtPlotPicker::AlwaysOn : QwtPlotPicker::AlwaysOff);
        }
    }
}


void QPTrackerTool::handleMouseEvent(const PlotEvent& e) {
    m_lastEventHandled = false;
    if(m_canvas == NULL || m_annotation.null()) return;
    m_lastEventHandled = true;
    
    const PlotMouseMoveEvent* mm = dynamic_cast<const PlotMouseMoveEvent*>(&e);
    if(mm != NULL) {
        PlotCoordinate c = mm->where();
        if(c.system() != m_coordSystem)
            c = m_canvas->convertCoordinate(c, m_coordSystem);
        m_annotation->setCoordinate(c);
        m_annotation->setText(formattedString(c.x(), c.y()));
        notifyWatchers();
    }
}


void QPTrackerTool::setDrawText(bool draw) {
    PlotTrackerTool::setDrawText(draw);
    if(m_tracker != NULL)
        m_tracker->setTrackerMode((m_active && draw)? QwtPlotPicker::AlwaysOn :
                                                     QwtPlotPicker::AlwaysOff);
}


// Protected Methods //

void QPTrackerTool::attach(PlotCanvas* canvas) {
    QPCanvas* c = dynamic_cast<QPCanvas*>(canvas);
    if(c != NULL) {
        PlotMouseTool::attach(c);
        m_tracker = new QPTracker(*this, c->asQwtPlot().canvas());
        m_annotation = m_tracker->getAnnotation();
        m_tracker->setTrackerMode((m_active && m_drawText) ?
                       QwtPlotPicker::AlwaysOn : QwtPlotPicker::AlwaysOff);
        c->reinstallTrackerFilter();
    }
}

void QPTrackerTool::detach() {
    if(m_tracker != NULL) {
        delete m_tracker;
        m_tracker = NULL;
    }
    m_annotation = NULL;
    PlotMouseTool::detach();
}


///////////////////////////
// QPTRACKER DEFINITIONS //
///////////////////////////

QPTracker::QPTracker(QPTrackerTool& tracker, QwtPlotCanvas* canvas) :
        QwtPlotPicker(QPOptions::axis(tracker.getXAxis()),
        QPOptions::axis(tracker.getYAxis()), canvas), m_tracker(tracker),
        m_annotation("", PlotCoordinate()) { }

QPTracker::~QPTracker() { }


QwtText QPTracker::trackerText(const QwtDoublePoint& pos) const {
    const_cast<QPAnnotation&>(m_annotation).setText(m_tracker.formattedString(
                                                    pos.x(), pos.y()));
    return m_annotation.asQwtText();
}

}

#endif
