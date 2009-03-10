//# QPAnnotation.h: Qwt implementation of generic PlotAnnotation class.
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
#ifndef QPANNOTATION_H_
#define QPANNOTATION_H_

#ifdef AIPS_HAS_QWT

#include <graphics/GenericPlotter/PlotAnnotation.h>
#include <casaqt/QwtPlotter/QPPlotItem.qo.h>

#include <qwt_text.h>

#include <casa/namespace.h>

namespace casa {

//# Forward Declarations
class QPCanvas;

// Implementation of PlotAnnotation for Qwt plotter.
class QPAnnotation : public QPPlotItem, public PlotAnnotation {
public:
    // Static //
    
    // Convenient access to class name (QPAnnotation).
    static const String CLASS_NAME;
    
    
    // Non-Static //
    
    // Constructor which takes text and location.
    QPAnnotation(const String& text, const PlotCoordinate& coord);
    
    // Constructor which takes text, location, font, and (optional) orientation
    // in counterclockwise degrees.
    QPAnnotation(const String& text, const PlotCoordinate& coord,
                 const PlotFont& font, int orientation = 0);
    
    // Constructor which copies the given generic PlotAnnotation.
    QPAnnotation(const PlotAnnotation& copy);
    
    // Destructor.
    ~QPAnnotation();
    
    
    // Include overloaded methods.
    using PlotAnnotation::setFont;
    using PlotAnnotation::setOutline;
    using PlotAnnotation::setBackground;
    
    
    // PlotItem Methods //
    
    // Implements PlotItem::isValid().
    bool isValid() const { return true; }
    
    
    // QPPlotItem Methods //
    
    // Overrides QwtPlotItem::boundingRect().
    QwtDoubleRect boundingRect() const;
    
    // Overrides QwtPlotItem::legendItem().
    QWidget* legendItem() const;
    
    
    // PlotAnnotation Methods //
    
    // Implements PlotAnnotation::text().
    String text() const;
    
    // Implements PlotAnnotation::setText().
    void setText(const String& newText);
    
    // Implements PlotAnnotation::font().
    PlotFontPtr font() const;
    
    // Implements PlotAnnotation::setFont().
    void setFont(const PlotFont& font);
    
    // Implements PlotAnnotation::orientation().
    int orientation() const;
    
    // Implements PlotAnnotation::setOrientation().
    void setOrientation(int orientation);
    
    // Implements PlotAnnotation::coordinate().
    PlotCoordinate coordinate() const;
    
    // Implements PlotAnnotation::setCoordinate().
    void setCoordinate(const PlotCoordinate& coord);
    
    // Implements PlotAnnotation::outlineShown().
    bool outlineShown() const;
    
    // Implements PlotAnnotation::setOutlineShown().
    void setOutlineShown(bool show = true);
    
    // Implements PlotAnnotation::outline().
    PlotLinePtr outline() const;
    
    // Implements PlotAnnotation::setOutline().
    void setOutline(const PlotLine& line);
    
    // Implements PlotAnnotation::background().
    PlotAreaFillPtr background() const;
    
    // Implements PlotAnnotation::setBackground().
    void setBackground(const PlotAreaFill& area);
    
    
    // QPAnnotation Methods //
    
    // Provides access to the underlying QwtText object.
    // <group>
    QwtText& asQwtText();
    const QwtText& asQwtText() const;
    // </group>
    
protected:
    // Implements QPPlotItem::className().
    const String& className() const { return CLASS_NAME; }
    
    // Implements QPPlotItem::draw_().  Ignores draw index and count.
    void draw_(QPainter* painter, const QwtScaleMap& xMap,
              const QwtScaleMap& yMap, const QRect& canvasRect,
              unsigned int drawIndex, unsigned int drawCount) const;
    
private:
    QwtText m_label;        // text, font, outline, and background
    PlotCoordinate m_coord; // location
    int m_orient;           // orientation
};

}

#endif

#endif /*QPANNOTATION_H_*/
