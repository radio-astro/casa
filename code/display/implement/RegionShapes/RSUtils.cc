//# RSUtils.h: Common utilities/constants for region shapes.
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
//# $Id$
#include <display/RegionShapes/RSUtils.qo.h>
#include <display/RegionShapes/RegionShape.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <display/Display/PixelCanvas.h>

namespace casa {

// RSUTILS DEFINITIONS //

QHBoxLayout* RSUtils::hlayout(QWidget* parent) {
    QHBoxLayout* l = new QHBoxLayout(parent);
    setupLayout(l);
    return l;
}

QVBoxLayout* RSUtils::vlayout(QWidget* parent) {
    QVBoxLayout* l = new QVBoxLayout(parent);
    setupLayout(l);
    return l;
}

void RSUtils::setupLayout(QLayout* layout) {
#if QT_VERSION >= 0x040300
    layout->setContentsMargins(0, 0, 0, 0);
#else
    layout->setMargin(0);
#endif
    layout->setSpacing(3);
}

QWidget* RSUtils::hline() {
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}

QWidget* RSUtils::vline() {
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}

QtColorWidget* RSUtils::colorWidget(bool showButton, String setColor,
        QWidget* parent) {
    QtColorWidget* w = new QtColorWidget(showButton, setColor, parent);
    setupLayout(w);
    return w;
}

const String RSUtils::PIXEL = "PIXEL";


bool RSUtils::hasDirectionCoordinate(const CoordinateSystem& cs) {
    for(unsigned int i = 0; i < cs.nCoordinates(); i++)
        if(cs.type(i) == Coordinate::DIRECTION) return true;
    return false;
}

MDirection::Types RSUtils::worldSystem(const CoordinateSystem& cs) {
    unsigned int dind;
    for(dind = 0; dind < cs.nCoordinates(); dind++)
        if(cs.type(dind) == Coordinate::DIRECTION) break;
    return cs.directionCoordinate(dind).directionType(true);
}

bool RSUtils::convertWCS(const Quantum<Vector<Double> >& from,
        MDirection::Types fromSys, Quantum<Vector<Double> >& to,
        MDirection::Types toSys) {
    if(fromSys == toSys) {
        to = from;
        to.convert(RegionShape::UNIT);
        return true;
    }
    MDirection md = MDirection::Convert(MDirection(from, fromSys), toSys)();
    to = md.getAngle(RegionShape::UNIT);
    return true;
}

bool RSUtils::worldToPixel(const Quantum<Vector<double> >& worldX,
        const Quantum<Vector<double> >& worldY, Vector<double>& pixelX,
        Vector<double>& pixelY, WorldCanvasHolder& wch,
        MDirection::Types fromSys, bool wrap, String* error) {
    if(worldX.getValue().size() != worldY.getValue().size()) {
        appendUniqueMessage(error, "W2P: World vectors must be same length.");
        return false;
    }
    
    if(pixelX.size() != worldX.getValue().size())
        pixelX.resize(worldX.getValue().size());
    if(pixelY.size() != worldY.getValue().size())
        pixelY.resize(worldY.getValue().size());
    
    const Vector<String>& units = wch.worldAxisUnits();
    const CoordinateSystem& cs = wch.worldCanvas()->coordinateSystem();
    MDirection::Types wcSys = worldSystem(cs);
    
    Quantum<Vector<Double> > q(Vector<Double>(2), RegionShape::UNIT);
    Quantum<Vector<Double> > qc(Vector<Double>(2), RegionShape::UNIT);
    Quantum<double> t(0.0, RegionShape::UNIT);
    
    Vector<double> world(2), pixel(2);
    bool convert = fromSys != wcSys;
    bool wasError = false;
    
    for(unsigned int i = 0; i < worldX.getValue().size(); i++) {
        q.getValue()(0) = worldX.getValue()(i);
        q.getValue()(1) = worldY.getValue()(i);
        
        if(convert) {
            try {
                if(!convertWCS(q,fromSys, qc, wcSys)) wasError = true;
            } catch(...) { wasError = true; }            
            if(wasError) {
                appendUniqueMessage(error, "W2P: Could not convert between "
                                           "world system.");
                return false;
            }
            
            if(wrap) {
                // stop-gap for conversion weirdness
                if(qc.getValue()(0) < 0 && q.getValue()(0) > 0)
                    qc.getValue()(0) += RegionShape::UNIT_WRAPAROUND;
                else if(qc.getValue()(0) > 0 && q.getValue()(0) < 0)
                    qc.getValue()(0) -= RegionShape::UNIT_WRAPAROUND;
                if(qc.getValue()(1) < 0 && q.getValue()(1) > 0)
                    qc.getValue()(1) += RegionShape::UNIT_WRAPAROUND;
                else if(qc.getValue()(1) > 0 && q.getValue()(1) < 0)
                    qc.getValue()(1) -= RegionShape::UNIT_WRAPAROUND;
            }
        } else {
            qc.getValue()(0) = q.getValue()(0);
            qc.getValue()(1) = q.getValue()(1);
        }
        
        t.setValue(qc.getValue()(0)); world[0] = t.getValue(units(0));
        t.setValue(qc.getValue()(1)); world[1] = t.getValue(units(1));
        
        try { if(!cs.toPixel(pixel, world)) wasError = true; }
        catch(...) { wasError = true; }
        if(wasError) {
            appendUniqueMessage(error, "W2P: Could not convert to pixel.");
            return false;
        }
        
        pixelX(i) = pixel(0);
        pixelY(i) = pixel(1);
    }
    
    return true;
}

bool RSUtils::pixelToWorld(const Vector<double>& pixelX,
        const Vector<double>& pixelY, Quantum<Vector<double> >& worldX,
        Quantum<Vector<double> >& worldY, WorldCanvasHolder& wch,
        MDirection::Types toSys, bool wrap, String* error) {
    if(pixelX.size() != pixelY.size()) {
        appendUniqueMessage(error, "P2W: Pixel vectors must be same length.");
        return false;
    }
    
    if(pixelX.size() != worldX.getValue().size())
        worldX.getValue().resize(pixelX.size());
    if(pixelY.size() != worldY.getValue().size())
        worldY.getValue().resize(pixelY.size());
    
    const Vector<String>& units = wch.worldAxisUnits();
    const CoordinateSystem& cs = wch.worldCanvas()->coordinateSystem();
    MDirection::Types wcSys = worldSystem(cs);
    
    Quantum<Vector<Double> > q(Vector<Double>(2), RegionShape::UNIT);
    Quantum<Vector<Double> > qc(Vector<Double>(2), RegionShape::UNIT);
    Quantum<double> tx(0.0, units(0)), ty(0.0, units(1));
    
    Vector<double> world(2), pixel(2);
    bool convert = wcSys != toSys;
    bool wasError = false;
    
    for(unsigned int i = 0; i < pixelX.size(); i++) {
        pixel[0] = pixelX(i);
        pixel[1] = pixelY(i);
        
        try { if(!cs.toWorld(world, pixel)) wasError = true; }
        catch(...) { wasError = true; }
        if(wasError) {
            appendUniqueMessage(error, "P2W: Could not convert to world.");
            return false;
        }
        
        tx.setValue(world[0]); q.getValue()(0)= tx.getValue(RegionShape::UNIT);
        ty.setValue(world[1]); q.getValue()(1)= ty.getValue(RegionShape::UNIT);
        
        if(convert) {
            try {
                if(!convertWCS(q, wcSys, qc, toSys)) wasError = true;
            } catch(...) { wasError = true; }
            if(wasError) {
                appendUniqueMessage(error, "P2W: Could not convert between "
                                           "world systems.");
                return false;
            }
            
            if(wrap) {
                // stop-gap for conversion weirdness
                if(qc.getValue()(0) < 0 && q.getValue()(0) > 0)
                    qc.getValue()(0) += RegionShape::UNIT_WRAPAROUND;
                else if(qc.getValue()(0) > 0 && q.getValue()(0) < 0)
                    qc.getValue()(0) -= RegionShape::UNIT_WRAPAROUND;
                if(qc.getValue()(1) < 0 && q.getValue()(1) > 0)
                    qc.getValue()(1) += RegionShape::UNIT_WRAPAROUND;
                else if(qc.getValue()(1) > 0 && q.getValue()(1) < 0)
                    qc.getValue()(1) -= RegionShape::UNIT_WRAPAROUND;
            }
        } else {
            qc.getValue()(0) = q.getValue()(0);
            qc.getValue()(1) = q.getValue()(1);
        }
        
        worldX.getValue()(i) = qc.getValue()(0);
        worldY.getValue()(i) = qc.getValue()(1);
    }
    
    return true;
}

bool RSUtils::worldToLinear(const Quantum<Vector<double> >& worldX,
        const Quantum<Vector<double> >& worldY, Vector<double>& linearX,
        Vector<double>& linearY, WorldCanvasHolder& wch,
        MDirection::Types fromSys, bool wrap, String* error) {
    if(worldX.getValue().size() != worldY.getValue().size()) {
        appendUniqueMessage(error, "W2L: World vectors must be same length.");
        return false;
    }

    if(linearX.size() != worldX.getValue().size())
        linearX.resize(worldX.getValue().size());
    if(linearY.size() != worldY.getValue().size())
        linearY.resize(worldY.getValue().size());

    const Vector<String>& units = wch.worldAxisUnits();
    Quantum<Vector<Double> > world(Vector<Double>(2), units(0));
    Vector<Double> linear(2);

    const CoordinateSystem& cs = wch.worldCanvas()->coordinateSystem();
    MDirection::Types wcSys = worldSystem(cs);
    bool convert = wcSys != fromSys;
    
    Quantum<Double> tx(1.0, worldX.getFullUnit());
    Quantum<Double> ty(1.0, worldY.getFullUnit());
    Quantum<Vector<Double> > convWorld(Vector<Double>(2), RegionShape::UNIT);
    Quantum<Double> cx(1.0, RegionShape::UNIT);
    Quantum<Double> cy(1.0, RegionShape::UNIT);
    Vector<Double> wcWorld(2), wcWorld2(cs.nWorldAxes(), 0);
    
    double lxMin = wch.worldCanvas()->linXMinLimit(),
           lxMax = wch.worldCanvas()->linXMaxLimit(),
           lyMin = wch.worldCanvas()->linYMinLimit(),
           lyMax = wch.worldCanvas()->linYMaxLimit();
    
    bool usingPixel = false;
    bool wasError = false;
    
    for(unsigned int i = 0; i < worldX.getValue().size(); i++) {       
       if(convert) {
           tx.setValue(worldX.getValue()(i));
           ty.setValue(worldY.getValue()(i));
           
           world.getValue()(0) = tx.getValue(units(0));
           world.getValue()(1) = ty.getValue(units(1));
           try {
               if(!convertWCS(world, fromSys, convWorld, wcSys))
                   wasError = true;
           } catch(...) { wasError = true; }
           if(wasError) {
               appendUniqueMessage(error, "W2L: Could not convert between "
                                          "world systems.");
               return false;
           }
       } else {
           convWorld.getValue()(0) = worldX.getValue()(i);
           convWorld.getValue()(1) = worldY.getValue()(i);
       }
       
       cx.setValue(convWorld.getValue()(0));
       cy.setValue(convWorld.getValue()(1));
       
       wcWorld(0) = cx.getValue(units(0));
       wcWorld(1) = cy.getValue(units(1));
       
       usingPixel = false;
       try { if(!wch.worldToLin(linear, wcWorld)) wasError = true; }
       catch(...) { wasError = true; }
       if(wasError) {
           wasError = false;
           // more than two world axes, so try best guess with pixels which
           // should work...
           wcWorld2(0) = wcWorld(0); wcWorld2(1) = wcWorld(1);
           try { if(!cs.toPixel(linear, wcWorld2)) wasError = true; }
           catch(...) { wasError = true; }
           if(wasError) {
               appendUniqueMessage(error, "W2L: Coordinate system has too "
                                          "many world axes.");
               return false;
           }
           usingPixel = true;
       }
           
       linearX(i) = linear(0);
       linearY(i) = linear(1);
       
       if(wrap) {
           tx.setValue(RegionShape::UNIT_WRAPAROUND);
           ty.setValue(RegionShape::UNIT_WRAPAROUND);
           double wrapx = tx.getValue(units(0)), wrapy = ty.getValue(units(1));
           if(linear(0) < lxMin || linear(0) > lxMax) {
               // try wrapping around
               if(!usingPixel) {
                   if(wcWorld(0) <= 0) wcWorld(0) += wrapx;
                   else                wcWorld(0) -= wrapx;
                   
                   try { if(!wch.worldToLin(linear, wcWorld)) wasError = true;}
                   catch(...) { wasError = true; }
                   if(wasError) {
                       appendUniqueMessage(error, "W2L: Could not convert to "
                                                  "linear.");
                       return false;
                   }
               } else {
                   if(wcWorld2(0) <= 0) wcWorld2(0) += wrapx;
                   else                 wcWorld2(0) -= wrapx;
                   
                   try { if(!cs.toPixel(linear, wcWorld2)) wasError = true; }
                   catch(...) { wasError = true; }
                   if(wasError) {
                       appendUniqueMessage(error, "W2L: Could not convert to "
                                                  "pixel as backup.");
                       return false;
                   }
               }
               
               if(linear(0) >= lxMin && linear(0) <= lxMax &&
                  linear(1) >= lyMin && linear(1) <= lyMax) {
                   linearX(i) = linear(0);
                   linearY(i) = linear(1);
               }
           }
           if(linear(1) < lyMin || linear(1) > lyMax) {
               // try wrapping around
               if(!usingPixel) {
                   if(wcWorld(1) <= 0) wcWorld(1) += wrapy;
                   else                wcWorld(1) -= wrapy;
                   
                   try { if(!wch.worldToLin(linear, wcWorld)) wasError = true;}
                   catch(...) { wasError = true; }
                   if(wasError) {
                       appendUniqueMessage(error, "W2L: Could not convert to "
                                                  "linear.");
                       return false;
                   }
               } else {
                   if(wcWorld2(1) <= 0) wcWorld2(1) += wrapy;
                   else                 wcWorld2(1) -= wrapy;
                   
                   try { if(!cs.toPixel(linear, wcWorld2)) wasError = true; }
                   catch(...) { wasError = true; }
                   if(wasError) {
                       appendUniqueMessage(error, "W2L: Could not convert to "
                                                  "pixel as backup.");
                       return false;
                   }
               }
               
               if(linear(0) >= lxMin && linear(0) <= lxMax &&
                  linear(1) >= lyMin && linear(1) <= lyMax) {
                   linearX(i) = linear(0);
                   linearY(i) = linear(1);
               }
           }
       }
    }

    return true;
}

bool RSUtils::pixelToLinear(const Vector<double>& pixelX,
        const Vector<double>& pixelY, Vector<double>& linearX,
        Vector<double>& linearY, WorldCanvasHolder& wch, String* error) {    
    if(pixelX.size() != pixelY.size()) {
        appendUniqueMessage(error, "P2L: Pixel vectors must be same length.");
        return false;
    }
    
    if(linearX.size() != pixelX.size()) linearX.resize(pixelX.size());
    if(linearY.size() != pixelY.size()) linearY.resize(pixelY.size());
    
    const CoordinateSystem& cs = wch.worldCanvas()->coordinateSystem();
    
    Vector<double> linear(2), world(cs.nWorldAxes()), world2(2), pixel(2);
    bool wasError = false;
    
    for(unsigned int i = 0; i < pixelX.size(); i++) {
        pixel(0) = pixelX(i);
        pixel(1) = pixelY(i);
        
        // Unfortunately WorldCanvas::pixToLin seems to do screen pixels
        // instead of image pixels...
        // if(!wc->pixToLin(linear, pixel)) return false;
        
        try { if(!cs.toWorld(world, pixel)) wasError = true; }
        catch(...) { wasError = true; }
        if(wasError) {
            appendUniqueMessage(error, "P2L: Could not convert to world.");
            return false;
        }
        world2(0) = world(0); world2(1) = world(1);
        try { if(!wch.worldToLin(linear, world2)) wasError = true; }
        catch(...) { wasError = true; }
        if(wasError) {
            appendUniqueMessage(error, "P2L: Could not convert to linear.");
            return false;
        }
        
        linearX(i) = linear(0);
        linearY(i) = linear(1);
    }
    
    return true;
}

bool RSUtils::linearToScreen(const Vector<double>& linearX,
        const Vector<double>& linearY, Vector<double>& screenX,
        Vector<double>& screenY, WorldCanvasHolder& wch, String* error) {
    if(linearX.size() != linearY.size()) {
        appendUniqueMessage(error, "L2S: Linear vectors must be same length.");
        return false;
    }

    if(!screenX.size() == linearX.size()) screenX.resize(linearX.size());
    if(!screenY.size() == linearY.size()) screenX.resize(linearX.size());

    WorldCanvas* wc = wch.worldCanvas();
    
    double scaleX = wc->canvasDrawXSize() / (wc->linXMax() - wc->linXMin()),
           scaleY = wc->canvasDrawYSize() / (wc->linYMax() - wc->linYMin());
    unsigned int offX = wc->canvasDrawXOffset() + wc->canvasXOffset(),
                 offY = wc->canvasDrawYOffset() + wc->canvasYOffset();
    double minX = wc->linXMin(), minY = wc->linYMin();

    for(unsigned int i = 0; i < linearX.size(); i++) {
        screenX(i) = ((linearX(i) - minX) * scaleX) + offX;
        screenY(i) = ((linearY(i) - minY) * scaleY) + offY;
    }

    return true;
}

bool RSUtils::screenToLinear(const Vector<double>& screenX,
        const Vector<double>& screenY, Vector<double>& linearX,
        Vector<double>& linearY, WorldCanvasHolder& wch, String* error) {
    if(screenX.size() != screenY.size()) {
        appendUniqueMessage(error, "S2L: Screen vectors must be same length.");
        return false;
    }

    if(screenX.size() != linearX.size()) linearX.resize(screenX.size());
    if(screenY.size() != linearY.size()) linearY.resize(screenY.size());

    WorldCanvas* wc = wch.worldCanvas();
    
    double scaleX = wc->canvasDrawXSize() / (wc->linXMax() - wc->linXMin()),
           scaleY = wc->canvasDrawYSize() / (wc->linYMax() - wc->linYMin());
    unsigned int offX = wc->canvasDrawXOffset() + wc->canvasXOffset(),
                 offY = wc->canvasDrawYOffset() + wc->canvasYOffset();
    double minX = wc->linXMin(), minY = wc->linYMin();

    for(unsigned int i = 0; i < screenX.size(); i++) {
        linearX(i) = ((screenX(i) - offX) / scaleX) + minX;
        linearY(i) = ((screenY(i) - offY) / scaleY) + minY;
    }
    return true;
}

bool RSUtils::linearToWorld(const Vector<double>& linearX,
        const Vector<double>& linearY, Quantum<Vector<double> >& worldX,
        Quantum<Vector<double> >& worldY, WorldCanvasHolder& wch,
        MDirection::Types toSys, const vector<int>& xSign,
        const vector<int>& ySign, bool wrap, String* error) {
    if(linearX.size() != linearY.size()) {
        appendUniqueMessage(error, "L2W: Linear vectors must be same length.");
        return false;
    }
    
    if(wrap && (xSign.size() != ySign.size() || xSign.size() != linearX.size())) {
        appendUniqueMessage(error, "L2W: Sign vectors must be same length.");
        return false;
    }
    
    if(linearX.size() != worldX.getValue().size())
        worldX.getValue().resize(linearX.size());
    if(linearY.size() != worldY.getValue().size())
        worldY.getValue().resize(linearY.size());
    
    worldX.setUnit(RegionShape::UNIT);
    worldY.setUnit(RegionShape::UNIT);
    
    const Vector<String>& units = wch.worldAxisUnits();
    
    Quantum<Vector<double> > world(Vector<double>(2), RegionShape::UNIT);
    Vector<double> linear(2), pixel(2);
    Quantum<double> tx(1.0, units(0)), ty(1.0, units(1));
    Quantum<Vector<Double> > worldConv(Vector<double>(2), RegionShape::UNIT);
    
    MDirection::Types wcSys = worldSystem(wch);
    bool convert = toSys != wcSys;

    double val;
    
    for(uInt i = 0; i < linearX.size(); i++) {
        linear(0) = linearX(i);
        linear(1) = linearY(i);
        
        if(!wch.linToWorld(world.getValue(), linear)) {
            appendUniqueMessage(error, "L2W: Could not convert to world.");
            return false;
        }
        
        tx.setValue(world.getValue()(0));
        ty.setValue(world.getValue()(1));
        
        if(convert) {
            world.getValue()(0) = tx.getValue(RegionShape::UNIT);
            world.getValue()(1) = ty.getValue(RegionShape::UNIT);
            if(!convertWCS(world, wcSys, worldConv, toSys)) {
                appendUniqueMessage(error, "L2W: Could not convert between "
                                           "world systems");
                return false;
            }
            
            if(wrap) {
                if(xSign[i] != 0) {
                    val = worldConv.getValue()(0);
                    if(xSign[i] < 0 && val > 0)
                        worldConv.getValue()(0) = -RegionShape::UNIT_WRAPAROUND
                                                  - val;
                    else if(xSign[i] > 0 && val < 0)
                        worldConv.getValue()(0) = RegionShape::UNIT_WRAPAROUND
                                                  + val;
                }
                if(ySign[i] != 0) {
                    val = worldConv.getValue()(1);
                    if(ySign[i] < 0 && val > 0)
                        worldConv.getValue()(1) = -RegionShape::UNIT_WRAPAROUND
                                                  - val;
                    else if(ySign[i] > 0 && val < 0)
                        worldConv.getValue()(1) = RegionShape::UNIT_WRAPAROUND
                                                  + val;
                }
            }
            
            worldX.getValue()(i) = worldConv.getValue()(0);
            worldY.getValue()(i) = worldConv.getValue()(1);
        } else {                
            worldX.getValue()(i) = tx.getValue(RegionShape::UNIT);
            worldY.getValue()(i) = ty.getValue(RegionShape::UNIT);
        }
    }
    
    return true;
}

bool RSUtils::linearToPixel(const Vector<double>& linearX,
        const Vector<double>& linearY, Vector<double>& pixelX,
        Vector<double>& pixelY, WorldCanvasHolder& wch, String* error) {
    if(linearX.size() != linearY.size()) {
        appendUniqueMessage(error, "L2P: Linear vectors must be same size.");
        return false;
    }
    
    if(linearX.size() != pixelX.size()) pixelX.resize(linearX.size());
    if(linearY.size() != pixelY.size()) pixelY.resize(linearY.size());
    
    const CoordinateSystem& cs = wch.worldCanvas()->coordinateSystem();    
    
    Vector<double> linear(2), world(cs.nWorldAxes(), 0), world2(2), pixel(2);
    
    for(unsigned int i = 0; i < pixelX.size(); i++) {
        linear(0) = linearX(i);
        linear(1) = linearY(i);
        
        // Unfortunately WorldCanvas::linToPix seems to do screen pixels
        // instead of image pixels...
        // if(!wc->linToPix(pixel, linear)) return false;
        
        if(!wch.linToWorld(world2, linear)) {
            appendUniqueMessage(error, "L2P: Could not convert to world.");
            return false;
        }
        for(unsigned int j = 0; j < world2.size() && j < world.size(); j++)
            world(j) = world2(j);
        if(!cs.toPixel(pixel, world)) {
            appendUniqueMessage(error, "L2P: Could not convert to pixel.");
            return false;
        }
        
        pixelX(i) = pixel(0);
        pixelY(i) = pixel(1);
    }
    
    return true;
}

void RSUtils::appendUniqueMessage(stringstream& ss, const String& message) {
    if(message.empty()) return;
    String str = ss.str();
    if(str.size() == 0) ss << message;
    else {
        bool unique = true;
        unsigned int i = 0, j = str.find('\n');
        String line;
        
        while(unique && i << str.size()) {
            line = str.substr(i, (j - i));
            if(line == message) unique = false;
            else if(j >= str.size() - 1) {
                i = str.size();
            } else {
                i = j + 1;
                j = str.find('\n', i);
            }
        }
        
        if(unique) ss << '\n' << message;
    }
}

void RSUtils::appendUniqueMessage(String& ss, const String& message) {
    if(message.empty()) return;
    if(ss.size() == 0) ss += message;
    else {
        unsigned int i = 0, j = ss.find('\n');
        String line;
        bool unique = true;
        while(unique && i < ss.size()) {
            line = ss.substr(i, (j - i));
            if(line == message) unique = false;
            else if(j >= ss.size() - 1) {
                i = ss.size();
            } else {
                i = j + 1;
                j = ss.find('\n', i);
            }
        }
        if(unique) ss += "\n" + message;
    }
}


// QTCOLORWIDGET DEFINITIONS //

QtColorWidget::QtColorWidget(bool showButton, String setText, QWidget* parent):
        QHBoxLayout(parent) {
    init(defaultColors(), setText, showButton);
}

QtColorWidget::QtColorWidget(const vector<String>& colors, bool showButton,
        String setText, QWidget* parent) : QHBoxLayout(parent) {
    init(colors, setText, showButton);
}

QtColorWidget::~QtColorWidget() { }

String QtColorWidget::getColor() const {
    if(m_chooser->currentIndex() < m_chooser->count() - 1)
        return m_chooser->currentText().toStdString();
    else return m_edit->text().toStdString();
}

void QtColorWidget::setColor(const String& color) {
    if(color.empty()) return;
    
    m_chooser->blockSignals(true);
    bool found = false;
    for(int i = 0; i < m_chooser->count() - 1; i++) {
        if(m_chooser->itemText(i) == color.c_str()) {
            found = true;
            m_chooser->setCurrentIndex(i);
            break;
        }
    }
    if(!found) {
        m_chooser->setCurrentIndex(m_chooser->count() - 1);
        m_edit->setText(color.c_str());
    }
    m_edit->setEnabled(!found);
    if(m_button != NULL) m_button->setEnabled(!found);
    m_chooser->blockSignals(false);
}

void QtColorWidget::init(const vector<String>& colors, const String& setText,
        bool showButton) {
    m_chooser = new QComboBox();
    for(unsigned int i = 0; i < colors.size(); i++)
        m_chooser->addItem(colors[i].c_str());
    m_chooser->addItem("custom");    
    m_edit = new QLineEdit();    
    addWidget(m_chooser);
    addWidget(m_edit);

    if(showButton) {
        m_button = new QPushButton("Pick");
        addWidget(m_button);
        connect(m_button, SIGNAL(clicked()), SLOT(colorPick()));
    } else m_button = NULL;
    
    setColor(setText);
    
    connect(m_chooser, SIGNAL(currentIndexChanged(int)),
            SLOT(colorChanged(int)));
}

void QtColorWidget::colorChanged(int index) {
    bool cust = index == m_chooser->count() - 1;
    m_edit->setEnabled(cust);
    if(m_button != NULL) m_button->setEnabled(cust);
    if(m_button == NULL && cust) colorPick();
}

void QtColorWidget::colorPick() {
    QColor color = QColorDialog::getColor();
    if(color.isValid()) m_edit->setText(color.name());
}


// RSVALUE DEFINITIONS //

const String RSValue::HMS    = "H:M:S";
const String RSValue::DMS    = "D:M:S";
const String RSValue::DEG    = "deg";
const String RSValue::RAD    = "rad";
const String RSValue::ARCSEC = "arcsec";
const String RSValue::ARCMIN = "arcmin";

bool RSValue::convertBetween(const QString& from, const String& fromUnits,
        RSValue& to) {
    bool ok = true;
    if(fromUnits == HMS || fromUnits == DMS) {
        bool isHMS = fromUnits == HMS;
        QStringList split = from.split(':');
        if(split.size() < 3) return false;
        long hOrD = (long)split[0].toDouble(&ok);
        if(!ok) return false;
        bool minusZero = hOrD == 0 && split[0][0] == '-';
        long min = (long)split[1].toDouble(&ok);
        if(!ok) return false;
        double sec = split[2].toDouble(&ok);
        if(!ok) return false;
        to = RSValue(isHMS, hOrD, min, sec, minusZero);
    } else {
        double d = from.toDouble(&ok);
        to = RSValue(d, fromUnits);
    }
    return ok;
}

bool RSValue::convertBetween(const RSValue& from, QString& to, int precision) {
    if(from.isQuantum) {
        to = QString("%1").arg(from.val.getValue(), 0, 'f', precision);
    } else {
        if((from.isMinusZero && from.hOrD == 0) || from.hOrD < 0) to = "-";
        else                                                      to = "";
        if((from.hOrD >= 0 && from.hOrD <= 9) ||
           (from.hOrD < 0 && from.hOrD >= -9)) to += "0";
        if(from.hOrD >= 0) to += QString("%1").arg( from.hOrD) + ":";
        else               to += QString("%1").arg(-from.hOrD) + ":";
        if(from.min <= 9) to += "0";
        to += QString("%1").arg(from.min) + ":";
        if(from.sec <= 9) to += "0";
        to += QString("%1").arg(from.sec, 0, 'f', precision);
    }
    return true;
}

bool RSValue::convertBetween(const RSValue& from, RSValue& to,
        const String& toUnits) {
    if((from.isQuantum && from.val.getUnit() == toUnits) ||
       (!from.isQuantum && from.isHMS && toUnits == HMS) ||
       (!from.isQuantum && !from.isHMS && toUnits == DMS)) {
        to = from;
        return true;
    }
    
    double fromDeg = 0;
    if(from.isQuantum) {
        fromDeg = from.val.getValue(DEG);
    } else if(from.isHMS) {
        if(from.hOrD < 0 || (from.hOrD == 0 && from.isMinusZero))
            fromDeg = (from.hOrD * 15) - (from.min / 4.0) - (from.sec / 240);
        else
            fromDeg = (from.hOrD * 15) + (from.min / 4.0) + (from.sec / 240);
    } else { // DMS
        if(from.hOrD < 0 || (from.hOrD == 0 && from.isMinusZero))
            fromDeg = from.hOrD - (from.min / 60.0) - (from.sec / 3600);
        else
            fromDeg = from.hOrD + (from.min / 60.0) + (from.sec / 3600);
    }
    
    if(toUnits == DEG) {
        to = RSValue(fromDeg);
    } else if(toUnits == HMS || toUnits == DMS) {
        bool isHMS = toUnits == HMS;
        bool minusZero = fromDeg < 0;
        long hOrD, min; double sec;
        if(isHMS) {
            fromDeg *= 240;
            hOrD = (long)(fromDeg / 3600);
            fromDeg -= (hOrD * 3600);
            min = (long)(fromDeg / 60);
            fromDeg -= (min * 60);
            sec = fromDeg;
        } else {
            double temp;
            fromDeg = modf(fromDeg, &temp);
            hOrD = (long)temp;
            fromDeg *= 60;
            fromDeg = modf(fromDeg, &temp);
            min = (long)temp;
            sec = fromDeg * 60;
        }
        if(min < 0) min = -min;
        if(sec < 0) sec = -sec;
        minusZero &= hOrD == 0;
        to = RSValue(isHMS, hOrD, min, sec, minusZero);
    } else {
        Quantum<double> q(fromDeg, DEG);
        to = RSValue(q.getValue(toUnits), toUnits);
    }
    return true;
}


// RSOPTION DEFINITIONS //

RSOption::RSOption(const String& str) : m_isString(true), m_string(str),
        m_isBool(false), m_isDouble(false), m_isStringArray(false) { }

RSOption::RSOption(bool b) : m_isString(false), m_isBool(true), m_bool(b),
        m_isDouble(false), m_isStringArray(false) { }

RSOption::RSOption(double d) : m_isString(false), m_isBool(false),
        m_isDouble(true), m_double(d), m_isStringArray(false) { }

RSOption::RSOption(const vector<String>& v): m_isString(false),m_isBool(false),
        m_isDouble(false), m_isStringArray(true), m_stringArray(v) { }

RSOption::~RSOption() { }

bool RSOption::isString() const { return m_isString; }
bool RSOption::isBool() const { return m_isBool; }
bool RSOption::isDouble() const { return m_isDouble; }
bool RSOption::isStringArray() const { return m_isStringArray; }

const String& RSOption::asString() const { return m_string; }
bool RSOption::asBool() const { return m_bool; }
double RSOption::asDouble() const { return m_double; }
const vector<String>& RSOption::asStringArray() const { return m_stringArray; }

bool RSOption::operator==(const RSOption& other) {
    if(m_isString) return other.m_isString && m_string == other.m_string;
    else if(m_isBool) return other.m_isBool && m_bool == other.m_bool;
    else if(m_isDouble)
        return other.m_isDouble && m_double == other.m_double;
    else if(m_isStringArray)
        return other.m_isStringArray && m_stringArray==other.m_stringArray;
        
    else return false;
}

bool RSOption::operator!=(const RSOption& other) { return !operator==(other); }

RSOption& RSOption::operator=(const String& str) {
    m_isString = true; m_isBool = m_isDouble = m_isStringArray = false;
    m_string = str;
    return *this;
}

RSOption& RSOption::operator=(bool b) {
    m_isBool = true; m_isString = m_isDouble = m_isStringArray = false;
    m_bool = b;
    return *this;
}

RSOption& RSOption::operator=(double d) {
    m_isDouble = true; m_isBool = m_isString = m_isStringArray = false;
    m_double = d;
    return *this;
}

RSOption& RSOption::operator=(const vector<String>& v) {
    m_isStringArray = true; m_isBool = m_isDouble = m_isString = false;
    m_stringArray = v;
    return *this;
}


//////////////////////////
// RSHANDLE DEFINITIONS //
//////////////////////////

const int RSHandle::DEFAULT_MARKER_HEIGHT           = 5;
const String RSHandle::DEFAULT_MARKER_COLOR         = "green";
const Display::Marker RSHandle::DEFAULT_MARKER_TYPE = Display::Circle;

RSHandle::RSHandle() : m_isValid(false), m_markerHeight(DEFAULT_MARKER_HEIGHT),
        m_markerColor(DEFAULT_MARKER_COLOR),
        m_markerType(DEFAULT_MARKER_TYPE) { }

RSHandle::RSHandle(const vector<double>& x, const vector<double>& y,
        int markerHeight, const String& markerColor, Display::Marker type) :
        m_isValid(false), m_markerType(type) {
    setMarkerHeight(markerHeight);
    setMarkerColor(markerColor);
    if(x.size() == 4 && y.size() == 4) {
        m_x = x; m_y = y;
        m_isValid = true;
    }
}

RSHandle::RSHandle(const Vector<double>& x, const Vector<double>& y,
        int markerHeight, const String& markerColor, Display::Marker type) :
        m_isValid(false), m_markerType(type) {
    setMarkerHeight(markerHeight);
    setMarkerColor(markerColor);
    if(x.size() == 4 && y.size() == 4) {
        x.tovector(m_x); y.tovector(m_y);
        m_isValid = true;
    }
}

RSHandle::~RSHandle() { }

void RSHandle::setMarkerHeight(int height) {
    if(height > 0) m_markerHeight = height;
    else           m_markerHeight = DEFAULT_MARKER_HEIGHT;
}

void RSHandle::setMarkerColor(const String& color) {
    if(!color.empty()) m_markerColor = color;
    else               m_markerColor = DEFAULT_MARKER_COLOR;
}

void RSHandle::setMarkerType(Display::Marker type) { m_markerType = type; }

bool RSHandle::containsPoint(double x, double y) const {
    if(!m_isValid) return false;
    
    QPolygonF poly;
    for(unsigned int i = 0; i < m_x.size() && i < m_y.size(); i++)
        poly << QPointF(m_x[i], m_y[i]);
    
    return poly.containsPoint(QPointF(x, y), Qt::OddEvenFill);
}

bool RSHandle::getVertices(vector<double>& x, vector<double>& y) const {
    if(!m_isValid) return false;
    x = m_x; y = m_y;
    return true;
}

bool RSHandle::draw(PixelCanvas* pc) const {
    if(!m_isValid || pc == NULL) return false;
    
    pc->setColor(m_markerColor);
    pc->setLineWidth(1);
    pc->setLineStyle(Display::LSSolid);
    
    for(unsigned int i = 0; i < m_x.size() && i < m_y.size(); i++)
        pc->drawMarker(m_x[i], m_y[i], m_markerType, m_markerHeight);

    return true;
}

}
