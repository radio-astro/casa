//# RegionShapes.cc: Classes for displaying region shapes on the viewer.
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
#include <display/RegionShapes/RegionShapes.h>
#include <display/Display/WorldCanvas.h>
#include <display/Display/PixelCanvas.h>
#include <measures/Measures/MeasConvert.h>

#include <math.h>

namespace casa {

#define SIGN(X) (X == 0 ? 0 : (X < 0 ? -1 : 1))

// Macros for coordinate conversions //

#define _RSCOORDSTART(n)                                                      \
    const CoordinateSystem& cs = m_lastWCH->worldCanvas()->coordinateSystem();\
    MDirection::Types wcSys = RSUtils::worldSystem(cs);                       \
                                                                              \
    bool isValid = true;                                                      \
    const Vector<String>& units = m_lastWCH->worldAxisUnits();                \
    vector<double> v(n);                                                      \
    Quantum<Vector<double> > q(Vector<double>(2), UNIT);                      \
    Quantum<Vector<double> > qx(Vector<double>(1), UNIT),                     \
                             qy(Vector<double>(1), UNIT);                     \
    Quantum<double> t(0.0, UNIT), tx(0.0, units(0)), ty(0.0, units(1));       \
    Vector<double> p(2), w(2), px(1), py(1);                                  \
    MDirection m;

#define RSCOORDSTART(n)                                                       \
    bool toPixel = toSystem == RSUtils::PIXEL;                                \
    if(toPixel && !m_isWorld) return coordParameterValues();                  \
    if(m_lastWCH == NULL) return coordParameterValues();                      \
    MDirection::Types toSys;                                                  \
    if(!toPixel && !MDirection::getType(toSys, toSystem))                     \
        return coordParameterValues();                                        \
                                                                              \
    if(m_isWorld && !toPixel && toSys == m_worldSystem)                       \
        return coordParameterValues();                                        \
                                                                              \
    _RSCOORDSTART(n)

#define RSSCOORDSTART(n)                                                      \
    if(vals.size() < n) return;                                               \
    bool fromPixel = system == RSUtils::PIXEL;                                \
    if(!m_isWorld && fromPixel) {                                             \
        setCoordParameters(vals);                                             \
        return;                                                               \
    }                                                                         \
    if(m_lastWCH == NULL) return;                                             \
    MDirection::Types fromSys;                                                \
    if(!fromPixel) {                                                          \
        if(!MDirection::getType(fromSys, system)) return;                     \
        if(m_isWorld && m_worldSystem == fromSys) {                           \
            setCoordParameters(vals);                                         \
            return;                                                           \
        }                                                                     \
    }                                                                         \
                                                                              \
    _RSCOORDSTART(n)

#define RSCOORDEND                                                            \
    if(isValid) return v;                                                     \
    else return coordParameterValues();

#define RSSCOORDEND if(isValid) setCoordParameters(v);

#define RSCOORDPOSWCONV(x,y,from,to) {                                        \
    q.getValue()(0) = x; q.getValue()(1) = y;                                 \
    if(from != to) {                                                          \
        m = MDirection::Convert(MDirection(q, from), to)();                   \
        q = m.getAngle(UNIT);                                                 \
        if(q.getValue()(0) < 0 && x > 0)  q.getValue()(0) += UNIT_WRAPAROUND; \
        else if(q.getValue()(0)>0 && x<0) q.getValue()(0) -= UNIT_WRAPAROUND; \
        if(q.getValue()(1) < 0 && y > 0)  q.getValue()(1) += UNIT_WRAPAROUND; \
        else if(q.getValue()(1)>0 && y<0) q.getValue()(1) -= UNIT_WRAPAROUND; \
    }                                                                         \
}

#define _RSCOORDPOSW2W(x,y,i,j,from,to) {                                     \
    RSCOORDPOSWCONV(x,y,from,to)                                              \
    v[i] = q.getValue()(0); v[j] = q.getValue()(1);                           \
}

#define RSCOORDPOSW2W(x,y,i,j)  _RSCOORDPOSW2W(x,y,i,j,m_worldSystem,toSys)
#define RSSCOORDPOSW2W(x,y,i,j) _RSCOORDPOSW2W(x,y,i,j,fromSys,m_worldSystem)

#define _RSCOORDPOSW2P(x,y,i,j,from) {                                        \
    q.getValue()(0) = x; q.getValue()(1) = y;                                 \
    if(from != wcSys) {                                                       \
        RSCOORDPOSWCONV(x,y,from,wcSys)                                       \
    }                                                                         \
    t.setValue(q.getValue()(0)); w(0) = t.getValue(units(0));                 \
    t.setValue(q.getValue()(1)); w(1) = t.getValue(units(1));                 \
    cs.toPixel(p, w);                                                         \
    v[i] = p(0); v[j] = p(1);                                                 \
}

#define RSCOORDPOSW2P(x,y,i,j)  _RSCOORDPOSW2P(x,y,i,j,m_worldSystem)
#define RSSCOORDPOSW2P(x,y,i,j) _RSCOORDPOSW2P(x,y,i,j,fromSys)

#define _RSCOORDPOSP2W(x,y,i,j,to) {                                          \
    p(0) = x; p(1) = y;                                                       \
    cs.toWorld(w, p);                                                         \
    tx.setValue(w(0)); q.getValue()(0) = tx.getValue(UNIT);                   \
    ty.setValue(w(1)); q.getValue()(1) = ty.getValue(UNIT);                   \
    if(wcSys != to) {                                                         \
        p(0) = q.getValue()(0); p(1) = q.getValue()(1);                       \
        RSCOORDPOSWCONV(p(0),p(1),wcSys,to)                                   \
    }                                                                         \
    v[i] = q.getValue()(0); v[j] = q.getValue()(1);                           \
}

#define RSCOORDPOSP2W(x,y,i,j)  _RSCOORDPOSP2W(x,y,i,j,toSys)
#define RSSCOORDPOSP2W(x,y,i,j) _RSCOORDPOSP2W(x,y,i,j,m_worldSystem)

#define RSCOORDPOSCONV(x,y,i,j) {                                             \
    if(m_isWorld && !toPixel) { /* world to world */                          \
        RSCOORDPOSW2W(x,y,i,j)                                                \
    } else if(m_isWorld && toPixel) { /* world to pixel */                    \
        RSCOORDPOSW2P(x,y,i,j)                                                \
    } else { /* pixel to world */                                             \
        RSCOORDPOSP2W(x,y,i,j)                                                \
    }                                                                         \
    if(!isValid) return coordParameterValues();                               \
}

#define RSSCOORDPOSCONV(x,y,i,j) {                                            \
    if(!fromPixel && m_isWorld) { /* world to world */                        \
        RSSCOORDPOSW2W(x,y,i,j)                                               \
    } else if(!fromPixel && !m_isWorld) { /* world to pixel */                \
        RSSCOORDPOSW2P(x,y,i,j)                                               \
    } else { /* pixel to world */                                             \
        RSSCOORDPOSP2W(x,y,i,j)                                               \
    }                                                                         \
    if(!isValid) return;                                                      \
}
    
#define RSCOORDPOSCONV_(i,j)  RSCOORDPOSCONV(m_spec[i], m_spec[j], i, j)
#define RSSCOORDPOSCONV_(i,j) RSSCOORDPOSCONV(vals[i],vals[j],i,j)

#define RSCOORDSIZEW2W(x,i) v[i] = x;
#define RSSCOORDSIZEW2W(x,i) RSCOORDSIZEW2W(x,i)
    
#define _RSCOORDSIZEW2P(x,y,l,lIsX,i,from) {                                  \
    qx.getValue()(0) = x; qy.getValue()(0) = y;                               \
    if(!RSUtils::worldToPixel(qx, qy, px, py, *m_lastWCH, from))              \
        isValid = false;                                                      \
    p(0) = px(0); p(1) = py(0);                                               \
    if(lIsX) qx.getValue()(0) += l;                                           \
    else     qy.getValue()(0) += l;                                           \
    if(!RSUtils::worldToPixel(qx, qy, px, py, *m_lastWCH, from))              \
        isValid = false;                                                      \
    v[i] = sqrt(pow(p(0) - px(0), 2) + pow(p(1) - py(0), 2));                 \
}

#define RSCOORDSIZEW2P(x,y,l,lIsX,i)                                          \
    _RSCOORDSIZEW2P(x,y,l,lIsX,i,m_worldSystem)
#define RSSCOORDSIZEW2P(x,y,l,lIsX,i) _RSCOORDSIZEW2P(x,y,l,lIsX,i,fromSys)

#define _RSCOORDSIZEP2W(x,y,l,lIsX,i,to) {                                    \
    px(0) = x; py(0) = y;                                                     \
    if(!RSUtils::pixelToWorld(px, py, qx, qy, *m_lastWCH,to)) isValid = false;\
    q.getValue()(0) = qx.getValue()(0); q.getValue()(1) = qy.getValue()(0);   \
    if(lIsX) px(0) += l;                                                      \
    else     py(0) += l;                                                      \
    if(!RSUtils::pixelToWorld(px, py, qx, qy, *m_lastWCH,to)) isValid = false;\
    v[i] = sqrt(pow(q.getValue()(0) - qx.getValue()(0), 2) +                  \
                pow(q.getValue()(1) - qy.getValue()(0), 2));                  \
}

#define RSCOORDSIZEP2W(x,y,l,lIsX,i) _RSCOORDSIZEP2W(x,y,l,lIsX,i,toSys)
#define RSSCOORDSIZEP2W(x,y,l,lIsX,i)                                         \
    _RSCOORDSIZEP2W(x,y,l,lIsX,i,m_worldSystem)
    
#define RSCOORDSIZECONV(x,y,l,lIsX,i) {                                       \
    if(m_isWorld && !toPixel) { /* world to world */                          \
        RSCOORDSIZEW2W(l,i)                                                   \
    } else if(m_isWorld && toPixel) { /* world to pixel */                    \
        RSCOORDSIZEW2P(x,y,l,lIsX,i)                                          \
    } else { /* pixel to world */                                             \
        RSCOORDSIZEP2W(x,y,l,lIsX,i)                                          \
    }                                                                         \
    if(!isValid) return coordParameterValues();                               \
}

#define RSCOORDSIZECONV2(x,y,lx,ly,i,j) {                                     \
    RSCOORDSIZECONV(x,y,lx,true,i)                                            \
    RSCOORDSIZECONV(x,y,ly,false,j)                                           \
}

#define RSCOORDSIZECONV2_(a,b,i,j)                                            \
    RSCOORDSIZECONV2(m_spec[a], m_spec[b], m_spec[i], m_spec[j], i, j)

#define RSSCOORDSIZECONV(x,y,l,lIsX,i) {                                      \
    if(!fromPixel && m_isWorld) { /* world to world */                        \
        RSSCOORDSIZEW2W(l,i)                                                  \
    } else if(!fromPixel && !m_isWorld) { /* world to pixel */                \
        RSSCOORDSIZEW2P(x,y,l,lIsX,i)                                         \
    } else { /* pixel to world */                                             \
        RSSCOORDSIZEP2W(x,y,l,lIsX,i)                                         \
    }                                                                         \
    if(!isValid) return;                                                      \
}

#define RSSCOORDSIZECONV2(x,y,lx,ly,i,j) {                                    \
    RSSCOORDSIZECONV(x,y,lx,true,i)                                           \
    RSSCOORDSIZECONV(x,y,ly,false,j)                                          \
}
    
#define RSSCOORDSIZECONV2_(a,b,i,j)                                           \
    RSSCOORDSIZECONV2(vals[a], vals[b], vals[i], vals[j], i, j)

#define _RSCOORDANGW2W(x,y,a,l,i,from,to) {                                   \
    RSCOORDPOSWCONV(x,y,from,to)                                              \
    w[0] = q.getValue()(0); w[1] = q.getValue()(1);                           \
    double rad = a * (C::pi / 180);                                           \
    q.getValue()(0) = x + (l * std::cos(rad));                                \
    q.getValue()(1) = y + (l * std::sin(rad));                                \
    p(0) = q.getValue()(0); p(1) = q.getValue()(1);                           \
    RSCOORDPOSWCONV(p(0),p(1),from,to)                                        \
    if(q.getValue()(0) == w[0]) rad = C::pi / 2;                              \
    else rad = std::atan((q.getValue()(1) - w[1]) / (q.getValue()(0) - w[0]));\
    v[i] = rad * (180 / C::pi);                                               \
    if(v[i] < 0 && q.getValue()(0) < w[0]) v[i] -= 180;                       \
    if(v[i] > 0 && q.getValue()(1) < w[1]) v[i] += 180;                       \
    while(v[i] < 0)    v[i] += 360;                                           \
    while(v[i] >= 360) v[i] -= 360;                                           \
}

#define RSCOORDANGW2W(x,y,a,l,i)                                              \
    _RSCOORDANGW2W(x,y,a,l,i,m_worldSystem,toSys)
#define RSSCOORDANGW2W(x,y,a,l,i)                                             \
    _RSCOORDANGW2W(x,y,a,l,i,fromSys,m_worldSystem)

#define RSCOORDANGW2PSA(sa,i) {                                               \
    /* take advantage of the fact that the screen coordinates have been */    \
    /* set, and pixel angle = screen angle */                                 \
    v[i] = sa;                                                                \
}

#define _RSCOORDANGW2P(x,y,a,l,i,from) {                                      \
    RSCOORDPOSWCONV(x,y,from,wcSys)                                           \
    t.setValue(q.getValue()(0)); w[0] = t.getValue(units(0));                 \
    t.setValue(q.getValue()(1)); w[1] = t.getValue(units(1));                 \
    if(!cs.toPixel(p, w)) isValid = false;                                    \
    px[0] = p[0]; py[0] = p[1];                                               \
    double rad = a * (C::pi / 180);                                           \
    p(0) = (x + (l * std::cos(rad))); p(1) = (y + (l * std::sin(rad)));       \
    RSCOORDPOSWCONV(p(0),p(1),from,wcSys)                                     \
    t.setValue(q.getValue()(0)); w[0] = t.getValue(units(0));                 \
    t.setValue(q.getValue()(1)); w[1] = t.getValue(units(1));                 \
    if(!cs.toPixel(p, w)) isValid = false;                                    \
    if(p[0] == px[0]) rad = C::pi / 2;                                        \
    else rad = std::atan((p[1] - py[0]) / (p[0] - px[0]));                    \
    v[i] = rad * (180 / C::pi);                                               \
    if(v[i] < 0 && p[0] < px[0]) v[i] -= 180;                                 \
    if(v[i] > 0 && p[1] < py[0]) v[i] += 180;                                 \
    while(v[i] < 0)    v[i] += 360;                                           \
    while(v[i] >= 360) v[i] -= 360;                                           \
}

#define RSCOORDANGW2P(x,y,a,l,i)                                              \
    _RSCOORDANGW2P(x,y,a,l,i,m_worldSystem)
#define RSSCOORDANGW2P(x,y,a,l,i)                                             \
    _RSCOORDANGW2P(x,y,a,l,i,fromSys)

#define _RSCOORDANGP2W(x,y,a,l,i,to) {                                        \
    p[0] = x; p[1] = y;                                                       \
    if(!cs.toWorld(w, p)) isValid = false;                                    \
    tx.setValue(w[0]); w[0] = tx.getValue(UNIT);                              \
    ty.setValue(w[1]); w[1] = ty.getValue(UNIT);                              \
    RSCOORDPOSWCONV(w[0], w[1], wcSys, to)                                    \
    px[0] = q.getValue()(0); py[0] = q.getValue()(1);                         \
    double rad = a * (C::pi / 180);                                           \
    p[0] += l * std::cos(rad); p[1] += l * std::sin(rad);                     \
    if(!cs.toWorld(w, p)) isValid = false;                                    \
    tx.setValue(w[0]); w[0] = tx.getValue(UNIT);                              \
    ty.setValue(w[1]); w[1] = ty.getValue(UNIT);                              \
    RSCOORDPOSWCONV(w[0], w[1], wcSys, to)                                    \
    if(q.getValue()(0) == px[0]) rad = C::pi / 2;                             \
    else rad = std::atan((q.getValue()(1) - py[0])/(q.getValue()(0) - px[0]));\
    v[i] = rad * (180 / C::pi);                                               \
    if(v[i] < 0 && q.getValue()(0) < px[0]) v[i] -= 180;                      \
    if(v[i] > 0 && q.getValue()(1) < py[0]) v[i] += 180;                      \
    while(v[i] < 0)    v[i] += 360;                                           \
    while(v[i] >= 360) v[i] -= 360;                                           \
}

#define RSCOORDANGP2W(x,y,a,l,i)                                              \
    _RSCOORDANGP2W(x,y,a,l,i,toSys)
#define RSSCOORDANGP2W(x,y,a,l,i)                                             \
    _RSCOORDANGP2W(x,y,a,l,i,m_worldSystem)

#define RSCOORDANGCONVSA(x,y,a,sa,l,i) {                                      \
    if(m_isWorld && !toPixel) { /* world to world */                          \
        RSCOORDANGW2W(x,y,a,l,i)                                              \
    } else if(m_isWorld && toPixel) { /* world to pixel */                    \
        RSCOORDANGW2PSA(sa,i)                                                 \
    } else { /* pixel to world */                                             \
        RSCOORDANGP2W(x,y,a,l,i)                                              \
    }                                                                         \
    if(!isValid) return coordParameterValues();                               \
}

#define RSCOORDANGCONVSA_(a,b,i,c)                                            \
    RSCOORDANGCONVSA(m_spec[a], m_spec[b], m_spec[i], m_screenSpec[i],        \
                     m_spec[c], i)
    
#define RSCOORDANGCONV(x,y,a,l,i) {                                           \
    if(m_isWorld && !toPixel) { /* world to world */                          \
        RSCOORDANGW2W(x,y,a,l,i)                                              \
    } else if(m_isWorld && toPixel) { /* world to pixel */                    \
        RSCOORDANGW2P(x,y,a,l,i)                                              \
    } else { /* pixel to world */                                             \
        RSCOORDANGP2W(x,y,a,l,i)                                              \
    }                                                                         \
    if(!isValid) return coordParameterValues();                               \
}
    
#define RSSCOORDANGCONV(x,y,a,l,i) {                                          \
    if(!fromPixel && m_isWorld) { /* world to world */                        \
        RSSCOORDANGW2W(x,y,a,l,i)                                             \
    } else if(!fromPixel && !m_isWorld) { /* world to pixel */                \
        RSSCOORDANGW2P(x,y,a,l,i)                                             \
    } else { /* pixel to world */                                             \
        RSSCOORDANGP2W(x,y,a,l,i)                                             \
    }                                                                         \
    if(!isValid) return;                                                      \
}

#define RSSCOORDANGCONV_(a,b,i,c)                                             \
    RSSCOORDANGCONV(vals[a],vals[b],vals[i],vals[c],i)

#define RSCOORDNOCONV(x,i) v[i] = x;
#define RSCOORDNOCONV_(i) RSCOORDNOCONV(m_coord[i], i)
#define RSSCOORDNOCONV(x,i) RSCOORDNOCONV(x,i)
#define RSSCOORDNOCONV_(i) RSSCOORDNOCONV_(i)


// RSELLIPSE DEFINITIONS //

RSEllipse::RSEllipse(double x, double y, double xRad, double yRad,
        MDirection::Types wSys, double angle) : RegionShape(wSys), m_spec(5),
        m_screenSpec(5) {
    m_spec[0] = x; m_spec[1] = y;
    m_spec[2] = xRad; m_spec[3] = yRad;
    m_spec[4] = angle;
    
    updateMinMax();
}

RSEllipse::RSEllipse(double x, double y, double xRad, double yRad, double ang):
        RegionShape(), m_spec(5), m_screenSpec(5) {
    m_spec[0] = x; m_spec[1] = y;
    m_spec[2] = xRad; m_spec[3] = yRad;
    m_spec[4] = ang;
    
    updateMinMax();
}

RSEllipse::RSEllipse(const RecordInterface& props): m_spec(5),m_screenSpec(5) {
    setProperties(props);
}

RSEllipse::~RSEllipse() { }

bool RSEllipse::drawAndUpdateBoundingBox(WorldCanvasHolder& wch, String* err) {
    if(m_spec[2] <= 0 || m_spec[3] <= 0 || !updateScreenCoordinates(wch, err)) {
        if(err && (m_spec[2] <= 0 || m_spec[3] <= 0)) {
            if(err->length() > 0) *err += "\n";
            *err += type() + ": cannot draw negative radii.";
        }
        return false;
    }
    
    PixelCanvas* pc = wch.worldCanvas()->pixelCanvas();
    setLineProperties(pc);

    // PixelCanvas::drawEllipse is expecting the axes in major/minor order
    // rather than x/y, so switch if necessary and adjust angle    
    double smaj = m_screenSpec[2], smin = m_screenSpec[3];
    if(smaj < smin) {
        smaj = m_screenSpec[3]; smin = m_screenSpec[2];
        m_screenSpec[4] += 90;
        if(m_screenSpec[4] >= 360) m_screenSpec[4] -= 360;
    }

    if(m_screenSpec[4] < 0) m_screenSpec[4] += 360;
    
    pc->drawEllipse(m_screenSpec[0], m_screenSpec[1], smaj, smin,
                    m_screenSpec[4]);

    // update bounding box
    Vector<double> x(4), y(4);
    if(RSRectangle::getRectanglePoints(m_screenSpec[0], m_screenSpec[1],
            m_screenSpec[2], m_screenSpec[3], m_screenSpec[4], x, y)) {
        m_screenXMin = m_screenXMax = x[0];
        m_screenYMin = m_screenYMax = y[0];
        for(unsigned int i = 1; i < x.size() && i < y.size(); i++) {
            if(x[i] < m_screenXMin) m_screenXMin = x[i];
            if(x[i] > m_screenXMax) m_screenXMax = x[i];
            if(y[i] < m_screenYMin) m_screenYMin = y[i];
            if(y[i] > m_screenYMax) m_screenYMax = y[i];
        }
        
    } else { // shouldn't happen
        m_screenXMin = m_screenSpec[0] - smaj;
        m_screenXMax = m_screenSpec[0] + smaj;
        m_screenYMin = m_screenSpec[1] - smaj;
        m_screenYMax = m_screenSpec[1] + smaj;
    }
    
    // adjust for line width
    if(m_width > 0) {
        m_screenXMin -= m_width / 2; m_screenXMax += m_width / 2;
        m_screenYMin -= m_width / 2; m_screenYMax += m_width / 2;
    }
    
    if(!m_label.isEmpty()) {
        // we want the label to be centered above the top
        m_label.draw(pc, (int)(m_screenSpec[0] + 0.5),
                         (int)(m_screenYMax + 0.5), true);
    }
    
    if(m_lt) {
        double rx = m_screenSpec[2] / sqrt(2.0);
        double ry = m_screenSpec[3] / sqrt(2.0);

        double a = m_screenSpec[4] * C::pi / 180;
        a += std::atan(rx / ry);
        if(m_screenSpec[2] < m_screenSpec[3]) a -= (C::pi / 2);
        a = (C::pi / 2) - a;
        double len = sqrt((rx * rx) + (ry * ry));
        rx = len * std::cos(a); ry = len * std::sin(a);
        
        setLinethroughProperties(pc);

        pc->drawLine((int)(m_screenSpec[0] - rx + 0.5),
                     (int)(m_screenSpec[1] + ry + 0.5),
                     (int)(m_screenSpec[0] + rx + 0.5),
                     (int)(m_screenSpec[1] - ry + 0.5));
    }
    
    if(!m_label.isEmpty()) m_screenYMax += m_label.height(pc);

    // reset line style
    resetLineStyle(pc);
    
    return true;
}

void RSEllipse::getEllipseCoordinates(double& x, double& y, double& xRadius,
        double& yRadius, double& angle, String toSystem) const {
    vector<double> v;
    if(toSystem.empty()) v = coordParameterValues();
    else                 v = coordParameterValues(toSystem);
    x       = v[0];
    y       = v[1];
    xRadius = v[2];
    yRadius = v[3];
    angle   = v[4];
}

vector<double> RSEllipse::coordParameterValues() const { return m_spec; }

vector<double> RSEllipse::coordParameterValues(String toSystem) const {
    RSCOORDSTART(5)
    
    // x, y
    RSCOORDPOSCONV_(0, 1)
    
    // x-radius, y-radius
    RSCOORDSIZECONV2_(0, 1, 2, 3)
    
    // angle
    RSCOORDANGCONVSA_(0, 1, 4, 2)
    
    RSCOORDEND
}

vector<String> RSEllipse::coordParameterNames() const {
    static vector<String> v(5);
    v[0] = "center x"; v[1] = "center y"; v[2] = "x-radius"; v[3] = "y-radius";
    v[4] = "angle";
    return v;
}

vector<RegionShape::CoordinateParameterType>
RSEllipse::coordParameterTypes() const {
    static vector<CoordinateParameterType> v(5);
    v[0] = POSITION; v[1] = POSITION; v[2] = SIZE; v[3] = SIZE; v[4] = ANGLE;
    return v;
}

void RSEllipse::setCoordParameters(const vector<double>& vals) {
    if(vals.size() < m_spec.size()) return;
    bool changed = false;
    
    for(unsigned int i = 0; i < m_spec.size(); i++) {
        changed |= m_spec[i] != vals[i];
        m_spec[i] = vals[i];
    }
    
    while(m_spec[4] < 0) m_spec[4] += 360;
    while(m_spec[4] >= 360) m_spec[4] -= 360;
    
    if(changed) {
        updateMinMax();
        refresh(true);
    }
}

void RSEllipse::setCoordParameters(const vector<double>& vals, String system) {    
    RSSCOORDSTART(m_spec.size())
    
    // x, y
    RSSCOORDPOSCONV_(0, 1)
    
    // x-radius, y-radius
    RSSCOORDSIZECONV2_(0, 1, 2, 3)
    
    // angle
    RSSCOORDANGCONV_(0, 1, 4, 2)
    
    RSSCOORDEND
}

void RSEllipse::move(double dx, double dy, String system) {
    if(dx == 0 && dy == 0) return;
    
    if(system.empty() || (m_isWorld && system == MDirection::showType(
       m_worldSystem)) || (!m_isWorld && system == RSUtils::PIXEL)) {
        m_spec[0] += dx; m_spec[1] += dy;
    } else {
        if(m_lastWCH == NULL) return;
        if(system == RSUtils::PIXEL) { // pixel to world
            Quantum<Vector<double> > wx(Vector<double>(1, m_spec[0]), UNIT),
                                     wy(Vector<double>(1, m_spec[1]), UNIT);
            Vector<double> px(1), py(1);
            if(!worldToPixel(wx, wy, px, py, *m_lastWCH)) return;
            px(0) += dx; py(0) += dy;
            if(!pixelToWorld(px, py, wx, wy, *m_lastWCH)) return;
            m_spec[0] = wx.getValue()(0); m_spec[1] = wy.getValue()(0);
        } else if(m_isWorld) {         // world to world
            MDirection::Types sys;
            if(!MDirection::getType(sys, system)) return;
            Quantum<Vector<double> > q1(Vector<double>(2), UNIT),
                                     q2(Vector<double>(2), UNIT);
            q1.getValue()(0) = m_spec[0]; q1.getValue()(1) = m_spec[1];
            if(!RSUtils::convertWCS(q1, m_worldSystem, q2, sys)) return;
            q2.getValue()(0) += dx; q2.getValue()(1) += dy;
            if(!RSUtils::convertWCS(q2, sys, q1, m_worldSystem)) return;
            m_spec[0] = q1.getValue()(0); m_spec[1] = q1.getValue()(1);
        } else {                       // world to pixel
            MDirection::Types sys;
            if(!MDirection::getType(sys, system)) return;
            Quantum<Vector<double> > wx(Vector<double>(1), UNIT),
                                     wy(Vector<double>(1), UNIT);
            Vector<double> px(1, m_spec[0]), py(1, m_spec[1]);
            if(!RSUtils::pixelToWorld(px, py, wx, wy, *m_lastWCH, sys)) return;
            wx.getValue()(0) += dx; wy.getValue()(0) += dy;
            if(!RSUtils::worldToPixel(wx, wy, px, py, *m_lastWCH, sys)) return;
            m_spec[0] = px(0); m_spec[1] = py(0);
        }
    }
    
    updateMinMax();
    refresh(true);
}

RSHandle RSEllipse::getHandle() const {
    if(m_lastWCH == NULL || m_width <= 0) return RSHandle();
    
    Vector<double> x(4), y(4);
    RSRectangle::getRectanglePoints(m_screenSpec[0], m_screenSpec[1],
            m_screenSpec[2] + (m_width / 2), m_screenSpec[3] + (m_width / 2),
            m_screenSpec[4], x, y);
    return RSHandle(x, y);
}

bool RSEllipse::updateScreenCoordinates(WorldCanvasHolder& wch, String* err) {
    Vector<double> linX(1), linY(1);
    Vector<double> sx(1), sy(1);

    Quantum<Vector<Double> > wx(Vector<Double>(1, m_spec[0]), UNIT),
                             wy(Vector<Double>(1, m_spec[1]), UNIT);
    Vector<double> px(1, m_spec[0]), py(1, m_spec[1]);

    if(m_isWorld) {
        if(!worldToLinear(wx, wy, linX, linY, wch, true, err)) return false;
    } else {
        if(!RSUtils::pixelToLinear(px, py, linX, linY, wch, err)) return false;
    }
    if(!RSUtils::linearToScreen(linX, linY, sx, sy, wch, err)) return false;
    
    m_screenSpec[0] = sx(0);
    m_screenSpec[1] = sy(0);

    // x radius
    if(m_isWorld) {
        wx.getValue()(0) = m_spec[0] + m_spec[2];
        wy.getValue()(0) = m_spec[1];
        if(!worldToLinear(wx, wy, linX, linY, wch, true, err)) return false;
    } else {
        px(0) = m_spec[0] + m_spec[2];
        py(0) = m_spec[1];
        if(!RSUtils::pixelToLinear(px, py, linX, linY, wch, err)) return false;
    }
    if(!RSUtils::linearToScreen(linX, linY, sx, sy, wch, err)) return false;

    m_screenSpec[2] = sqrt(pow(sx(0) - m_screenSpec[0], 2) +
                           pow(sy(0) - m_screenSpec[1], 2));

    // y radius
    if(m_isWorld) {
        wx.getValue()(0) = m_spec[0];
        wy.getValue()(0) = m_spec[1] + m_spec[3];
        if(!worldToLinear(wx, wy, linX, linY, wch, true, err)) return false;
    } else {
        px(0) = m_spec[0];
        py(0) = m_spec[1] + m_spec[3];
        if(!RSUtils::pixelToLinear(px, py, linX, linY, wch, err)) return false;
    }
    if(!RSUtils::linearToScreen(linX, linY, sx, sy, wch, err)) return false;
    
    m_screenSpec[3] = sqrt(pow(sx(0) - m_screenSpec[0], 2) +
                           pow(sy(0) - m_screenSpec[1], 2));
    
    if(m_isWorld) {
        // convert world angle to pixel angle
        double deg = m_spec[4] * (C::pi / 180);
        wx.getValue()(0) = m_spec[0] + (std::cos(deg) * m_spec[2]);
        wy.getValue()(0) = m_spec[1] + (std::sin(deg) * m_spec[2]);
        if(!worldToScreen(wx, wy, sx, sy, wch, true, err)) {
            deg = 0;
        } else if(sx(0) == m_screenSpec[0]) {
            deg = 90;
        } else {
            deg = std::atan((sy(0) - m_screenSpec[1]) /
                            (sx(0) - m_screenSpec[0]));
            deg *= (180 / C::pi);
            if(deg > 0 && (sy(0) < m_screenSpec[1])) deg += 180;
            if(deg < 0 && (sx(0) < m_screenSpec[0])) deg -= 180;
            
            if(deg < 0) deg = 360 + deg;
        }
        m_screenSpec[4] = deg;
    } else m_screenSpec[4] = m_spec[4];

    return true;
}

void RSEllipse::updateMinMax() {
    m_xMin = m_spec[0] - m_spec[2]; m_xMax = m_spec[0] + m_spec[2];
    m_yMin = m_spec[1] - m_spec[3]; m_yMax = m_spec[1] + m_spec[3];
}


// RSCIRCLE DEFINITIONS //

RSCircle::RSCircle(double x, double y, double xRadius,
        MDirection::Types worldSys) : RSEllipse(x, y, xRadius, xRadius,
        worldSys, 0) { }

RSCircle::RSCircle(double x, double y, double xRadius) :
        RSEllipse(x, y, xRadius, xRadius, 0) { }

RSCircle::RSCircle(const RecordInterface& props) : RSEllipse(props) {
    setProperties(props);
}

RSCircle::~RSCircle() { }

void RSCircle::getCircleCoordinates(double& x, double& y, double& xrad,
        String toSystem) const {
    vector<double> v;
    if(toSystem.empty()) v = coordParameterValues();
    else                 v = coordParameterValues(toSystem);
    x    = v[0];
    y    = v[1];
    xrad = v[2];
}

vector<double> RSCircle::coordParameterValues() const {
    vector<double> v(3);
    v[0] = m_spec[0]; v[1] = m_spec[1]; v[2] = m_spec[2];
    return v;
}

vector<double> RSCircle::coordParameterValues(String toSystem) const {
    RSCOORDSTART(3)
    
    // x, y
    RSCOORDPOSCONV_(0, 1)
    
    // x-radius
    RSCOORDSIZECONV(m_spec[0], m_spec[1], m_spec[2], true, 2)
    
    RSCOORDEND
}

vector<String> RSCircle::coordParameterNames() const {
    static vector<String> v(3);
    v[0] = "center x"; v[1] = "center y"; v[2] = "x-radius";
    return v;
}

vector<RegionShape::CoordinateParameterType>
RSCircle::coordParameterTypes() const {
    static vector<CoordinateParameterType> v(3);
    v[0] = POSITION; v[1] = POSITION; v[2] = SIZE;
    return v;
}
    
void RSCircle::setCoordParameters(const vector<double>& vals) {
    if(vals.size() < 3) return;
    bool changed = false;
    for(unsigned int i = 0; i < 3; i++) {
        changed |= m_spec[i] != vals[i];
        m_spec[i] = vals[i];
    }
    m_spec[3] = m_spec[2]; m_spec[4] = 0;
    if(changed) {
        updateMinMax();
        refresh(true);
    }
}

void RSCircle::setCoordParameters(const vector<double>& vals, String system) {
    RSSCOORDSTART(3)
    
    // x, y
    RSSCOORDPOSCONV_(0, 1)
    
    // x-radius
    RSSCOORDSIZECONV(vals[0], vals[1], vals[2], true, 2)
    
    RSSCOORDEND
}

bool RSCircle::updateScreenCoordinates(WorldCanvasHolder& wch, String* err) {
    if(!RSEllipse::updateScreenCoordinates(wch, err)) return false;
    
    // replace y radius with x radius and make angle 0
    m_screenSpec[3] = m_screenSpec[2];
    m_screenSpec[4] = 0;
    
    // get accurate y radius for world for min/max
    Vector<double> screenX(1, m_screenSpec[0]),
                   screenY(1, m_screenSpec[1] + m_screenSpec[2]);
    if(m_isWorld) {
        Quantum<Vector<double> > worldX(Vector<double>(1), UNIT),
                                 worldY(Vector<double>(1), UNIT);
        vector<int> signX(1, SIGN(m_spec[0])), signY(1, SIGN(m_spec[1]));
        if(screenToWorld(screenX,screenY,worldX,worldY,wch,signX,signY,err)) {
            m_spec[3] = sqrt(std::pow(m_spec[0] - worldX.getValue()(0), 2) +
                             std::pow(m_spec[1] - worldY.getValue()(0), 2));
        }
    } else {
        Vector<double> pixelX(1), pixelY(1);
        if(RSUtils::screenToPixel(screenX, screenY, pixelX, pixelY, wch, err)){
            m_spec[3] = sqrt(std::pow(m_spec[0] - pixelX(0), 2) +
                             std::pow(m_spec[1] - pixelY(0), 2));
        }
    }
    
    return true;
}


// RSRECTANGLE DEFINITIONS //

RSRectangle::RSRectangle(double x, double y, double width, double height,
        MDirection::Types wSys, double angle) : RegionShape(wSys), m_spec(5),
        m_x(4), m_y(4), m_screenX(4), m_screenY(4) {
    m_spec[0] = x;     m_spec[1] = y;
    m_spec[2] = width; m_spec[3] = height;
    m_spec[4] = angle;
    
    updatePoints();
    updateMinMax();
}

RSRectangle::RSRectangle(double x, double y, double width, double height,
        double angle) : RegionShape(), m_spec(5), m_x(4), m_y(4), m_screenX(4),
        m_screenY(4) {
    m_spec[0] = x;     m_spec[1] = y;
    m_spec[2] = width; m_spec[3] = height;
    m_spec[4] = angle;
    
    updatePoints();
    updateMinMax();
}

RSRectangle::RSRectangle(const RecordInterface& props) : m_spec(5), m_x(4),
        m_y(4), m_screenX(4), m_screenY(4) {
    setProperties(props);
}

RSRectangle::~RSRectangle() { }

bool RSRectangle::drawAndUpdateBoundingBox(WorldCanvasHolder& wch,String* err){
    if(m_spec[2] <= 0 || m_spec[3] <= 0 || !updateScreenCoordinates(wch,err)) {
        if(err && (m_spec[2] <= 0 || m_spec[3] <= 0)) {
            if(err->length() > 0) *err += "\n";
            *err += type() + ": cannot draw negative width/height.";
        }
        return false;
    }
    
    PixelCanvas* pc = wch.worldCanvas()->pixelCanvas();
    setLineProperties(pc);

    bool drawRect = false;
    
    // strange bug where sometimes dashed/dotted lines don't draw properly
    int x1 = (int)(m_screenX(0) + 0.5), x2 = (int)(m_screenX(1) + 0.5),
        y1 = (int)(m_screenY(0) + 0.5), y2 = (int)(m_screenY(1) + 0.5);
    if(x1 < (x2 - 1) || x1 > (x2 + 1)) {
        x1 = x2; x2 = (int)(m_screenX(2) + 0.5);
        if(x1 >= (x2 - 1) && x1 <= (x2 + 1)) {
            x2 = (int)(m_screenX(3) + 0.5);
            if(y1 < (y2 - 1) || y1 > (y2 + 1)) {
                y1 = y2;
                y2 = (int)(m_screenY(2) + 0.5);
                if(y1 >= (y2 - 1) && y1 <= (y2 + 1)) {
                    drawRect = true;
                    y2 = (int)(m_screenY(3) + 0.5);
                }
            } else {
                drawRect = true;
                y2 = (int)(m_screenY(2) + 0.5);
            }
        }
    } else {
        x2 = (int)(m_screenX(2) + 0.5);
        if(y1 < (y2 - 1) || y1 > (y2 + 1)) {
            y1 = y2; y2 = (int)(m_screenY(2) + 0.5);
            if(y1 >= (y2 - 1) && y1 <= (y2 + 1)) {
                drawRect = true;
                y2 = (int)(m_screenY(3) + 0.5);
            }
        } else {
            drawRect = true;
            y2 = (int)(m_screenY(2) + 0.5);
        }
    }
    
    if(!drawRect) pc->drawPolygon(m_screenX, m_screenY);
    else          pc->drawRectangle(x1, y1, x2, y2);
    
    // update bounding box
    if(!drawRect) {
        m_screenXMin = m_screenXMax = m_screenX(0);
        m_screenYMin = m_screenYMax = m_screenY(0);
        for(unsigned int i = 1; i < m_screenX.size(); i++) {
            if(m_screenX(i) < m_screenXMin) m_screenXMin = m_screenX(i);
            if(m_screenX(i) > m_screenXMax) m_screenXMax = m_screenX(i);
            if(m_screenY(i) < m_screenYMin) m_screenYMin = m_screenY(i);
            if(m_screenY(i) > m_screenYMax) m_screenYMax = m_screenY(i);
        }
    } else {
        m_screenXMin = m_screenXMax = x1;
        m_screenYMin = m_screenYMax = y1;
        if(x2 < m_screenXMin) m_screenXMin = x2;
        if(x2 > m_screenXMax) m_screenXMax = x2;
        if(y2 < m_screenYMin) m_screenYMin = y2;
        if(y2 > m_screenYMax) m_screenYMax = y2;
    }

    // adjust for line width
    if(m_width > 0) {
        m_screenXMin -= m_width / 2; m_screenXMax += m_width / 2;
        m_screenYMin -= m_width / 2; m_screenYMax += m_width / 2;
    }
    
    if(!m_label.isEmpty()) {
        // we want the label to be centered above the top
        m_label.draw(pc, (int)(((m_screenXMax + m_screenXMin)/2) + 0.5),
                         (int)(m_screenYMax + 0.5), true);
    }

    if(m_lt) {
        setLinethroughProperties(pc);
        
        int x1 = (int)(m_screenX[0] + 0.5), y1 = (int)(m_screenY[0] + 0.5),
            x2 = (int)(m_screenX[2] + 0.5), y2 = (int)(m_screenY[2] + 0.5);
        
        pc->drawLine(x1, y1, x2, y2);
    }
    
    if(!m_label.isEmpty()) m_screenYMax += m_label.height(pc);

    // reset line style
    resetLineStyle(pc);
    
    return true;
}

void RSRectangle::getRectangleCoordinates(double& x, double& y, double& width,
        double& height, double& angle, String toSystem) const {
    vector<double> v;
    if(toSystem.empty()) v = coordParameterValues();
    else                 v = coordParameterValues(toSystem);
    x      = v[0];
    y      = v[1];
    width  = v[2];
    height = v[3];
    angle  = v[4];
}

vector<double> RSRectangle::coordParameterValues() const {
    return m_spec;
}

vector<double> RSRectangle::coordParameterValues(String toSystem) const {
    RSCOORDSTART(5)
    
    // x, y
    RSCOORDPOSCONV_(0, 1)
    
    // width, height
    RSCOORDSIZECONV2_(0, 1, 2, 3)
    
    // angle
    RSCOORDANGCONV(m_spec[0], m_spec[1], m_spec[4], m_spec[2], 4)
    
    RSCOORDEND
}

vector<String> RSRectangle::coordParameterNames() const {
    static vector<String> v(5);
    v[0] = "center x"; v[1] = "center y"; v[2] = "width"; v[3] = "height";
    v[4] = "angle";
    return v;
}

vector<RegionShape::CoordinateParameterType>
RSRectangle::coordParameterTypes() const {
    static vector<CoordinateParameterType> v(5);
    v[0] = POSITION; v[1] = POSITION; v[2] = SIZE; v[3] = SIZE; v[4] = ANGLE;
    return v;
}

void RSRectangle::setCoordParameters(const vector<double>& vals) {
    if(vals.size() < m_spec.size()) return;
    bool changed = false;
    for(unsigned int i = 0; i < m_spec.size(); i++) {
        changed |= m_spec[i] != vals[i];
        m_spec[i] = vals[i];
    }
    
    while(m_spec[4] < 0) m_spec[4] += 360;
    while(m_spec[4] >= 360) m_spec[4] -= 360;
    
    if(changed) {
        updatePoints();
        updateMinMax();
        refresh(true);
    }
}

void RSRectangle::setCoordParameters(const vector<double>& vals,String system){   
    RSSCOORDSTART(m_spec.size())
    
    // x, y
    RSSCOORDPOSCONV_(0, 1)
    
    // width, height
    RSSCOORDSIZECONV2_(0, 1, 2, 3)
    
    // angle
    RSSCOORDANGCONV_(0, 1, 4, 2)
    
    RSSCOORDEND
}

void RSRectangle::move(double dx, double dy, String system) {
    if(dx == 0 && dy == 0) return;
    
    if(system.empty() || (m_isWorld && system == MDirection::showType(
       m_worldSystem)) || (!m_isWorld && system == RSUtils::PIXEL)) {
        m_spec[0] += dx;
        m_spec[1] += dy;
    } else {
        if(m_lastWCH == NULL) return;
        if(system == RSUtils::PIXEL) { // pixel to world
            Quantum<Vector<double> > wx(Vector<double>(1, m_spec[0]), UNIT),
                                     wy(Vector<double>(1, m_spec[1]), UNIT);
            Vector<double> px(1), py(1);
            if(!worldToPixel(wx, wy, px, py, *m_lastWCH)) return;
            px(0) += dx; py(0) += dy;
            if(!pixelToWorld(px, py, wx, wy, *m_lastWCH)) return;
            m_spec[0] = wx.getValue()(0); m_spec[1] = wy.getValue()(0);
        } else if(m_isWorld) {         // world to world
            MDirection::Types sys;
            if(!MDirection::getType(sys, system)) return;
            Quantum<Vector<double> > q1(Vector<double>(2), UNIT),
                                     q2(Vector<double>(2), UNIT);
            q1.getValue()(0) = m_spec[0]; q1.getValue()(1) = m_spec[1];
            if(!RSUtils::convertWCS(q1, m_worldSystem, q2, sys)) return;
            q2.getValue()(0) += dx; q2.getValue()(1) += dy;
            if(!RSUtils::convertWCS(q2, sys, q1, m_worldSystem)) return;
            m_spec[0] = q1.getValue()(0); m_spec[1] = q1.getValue()(1);
        } else {                       // world to pixel
            MDirection::Types sys;
            if(!MDirection::getType(sys, system)) return;
            Quantum<Vector<double> > wx(Vector<double>(1), UNIT),
                                     wy(Vector<double>(1), UNIT);
            Vector<double> px(1, m_spec[0]), py(1, m_spec[1]);
            if(!RSUtils::pixelToWorld(px, py, wx, wy, *m_lastWCH, sys)) return;
            wx.getValue()(0) += dx; wy.getValue()(0) += dy;
            if(!RSUtils::worldToPixel(wx, wy, px, py, *m_lastWCH, sys)) return;
            m_spec[0] = px(0); m_spec[1] = py(0);
        }
    }
    
    updatePoints();
    updateMinMax();
    refresh(true);
}

RSHandle RSRectangle::getHandle() const {
    if(m_lastWCH == NULL || m_width <= 0) return RSHandle();
    
    // get center in screen coords
    Vector<double> centX(1), centY(1);
    if(m_isWorld) {
        Quantum<Vector<double> > qx(Vector<double>(1, m_spec[0]), UNIT),
                                 qy(Vector<double>(1, m_spec[1]), UNIT);
        worldToScreen(qx, qy, centX, centY, *m_lastWCH);
    } else {
        Vector<double> px(1, m_spec[0]), py(1, m_spec[1]);
        RSUtils::pixelToScreen(px, py, centX, centY, *m_lastWCH);
    }
    
    // get width and height, with line width added
    double width = sqrt(pow(m_screenX[1] - m_screenX[0], 2) +
                        pow(m_screenY[1] - m_screenY[0], 2)) + m_width,
           height= sqrt(pow(m_screenX[2] - m_screenX[1], 2) +
                        pow(m_screenY[2] - m_screenY[1], 2)) + m_width;
    
    // get angle
    double angle = 90;
    if(m_screenX[0] != m_screenX[1]) {
        angle = std::atan((m_screenY[1] - m_screenY[0])/
                          (m_screenX[1] - m_screenX[0])) * (180 / C::pi);
    }
    while(angle < 0)    angle += 360;
    while(angle >= 360) angle -= 360;
    bool isRect1 = (angle >= 0 && angle < 1) || (angle > 179 && angle < 181) ||
                   (angle > 359 && angle <= 360),
         isRect2 = (angle > 89 && angle < 91) || (angle > 269 && angle < 271);
    
    Vector<double> x(4), y(4);
    if(isRect1) {
        x[0] = x[2] = centX(0) + width / 2;
        x[1] = x[3] = centX(0) - width / 2;
        y[0] = y[1] = centY(0) + height / 2;
        y[2] = y[3] = centY(0) - height / 2;
    } else if(isRect2) {
        x[0] = x[2] = centX(0) + height / 2;
        x[1] = x[3] = centX(0) - height / 2;
        y[0] = y[1] = centY(0) + width / 2;
        y[2] = y[3] = centY(0) - width / 2;
        
    } else {    
        RSRectangle::getRectanglePoints(centX(0), centY(0), width / 2, height / 2,
                angle, x, y);
    }
    return RSHandle(x, y);
}

bool RSRectangle::updateScreenCoordinates(WorldCanvasHolder& wch, String* err){    
    if(m_isWorld) {
        Quantum<Vector<Double> > wx(m_x, UNIT), wy(m_y, UNIT);
        return worldToScreen(wx, wy, m_screenX, m_screenY, wch, true, err);
    } else {
        return RSUtils::pixelToScreen(m_x, m_y, m_screenX, m_screenY, wch,err);
    }
}

void RSRectangle::updatePoints() {
    getRectanglePoints(m_spec[0], m_spec[1], m_spec[2] / 2, m_spec[3] / 2,
                       m_spec[4], m_x, m_y);
}

void RSRectangle::updateMinMax() {
    m_xMin = m_x(0); m_xMax = m_x(0); m_yMin = m_y(0); m_yMax = m_y(0);
    for(unsigned int i = 1; i < m_x.size(); i++) {
        if(m_x(i) < m_xMin) m_xMin = m_x(i);
        if(m_x(i) > m_xMax) m_xMax = m_x(i);
        if(m_y(i) < m_yMin) m_yMin = m_y(i);
        if(m_y(i) > m_yMax) m_yMax = m_y(i);
    }
}

bool RSRectangle::getRectanglePoints(double centerX, double centerY,
        double radiusX, double radiusY, double angle, Vector<double>& x,
        Vector<double>& y) {
    if(radiusX <= 0 || radiusY <= 0) return false;
    
    double rad = angle * C::pi / 180;
    double w = 2 * radiusX, h = 2 * radiusY;
    double alpha = (C::pi / 2) - rad - std::atan(h / w);
    double diag = sqrt((radiusY * radiusY) + (radiusX * radiusX));
    
    // Find top right corner
    double dx = diag * std::sin(alpha), dy = diag * std::cos(alpha);
    double trX  = centerX + dx, trY = centerY + dy;
    
    // Find top left corner
    dx = w * std::cos(rad); dy = w * std::sin(rad);
    double tlX = trX - dx, tlY = trY - dy;
    
    // Find bottom left corner
    dx = h * std::sin(rad); dy = h * std::cos(rad);
    double blX = tlX + dx, blY = tlY - dy;
    
    // Find bottom right corner
    dx = w * std::cos(rad); dy = w * std::sin(rad);
    double brX = blX + dx, brY = blY + dy;
    
    if(x.size() != 4) x.resize(4);
    if(y.size() != 4) y.resize(4);
    x(0) = tlX; x(1) = blX; x(2) = brX; x(3) = trX;
    y(0) = tlY; y(1) = blY; y(2) = brY; y(3) = trY;
    return true;
}


// RSPOLYGON DEFINITIONS //

const String RSPolygon::PROPXCOORDINATES = "xcoords";
const String RSPolygon::PROPYCOORDINATES = "ycoords";

RSPolygon::RSPolygon(const Vector<Double>& x, const Vector<Double>& y,
        MDirection::Types worldSystem, String cname): RegionShape(worldSystem),
        m_custName(cname) {
    setPolygonCoordinates(x, y);
}

RSPolygon::RSPolygon(const Vector<double>& x, const Vector<double>& y,
        String cname) : RegionShape(), m_custName(cname) {
    setPolygonCoordinates(x, y);
}

RSPolygon::RSPolygon(const RecordInterface& props) {
    setProperties(props);
}

RSPolygon::~RSPolygon() { }

void RSPolygon::setPolygonCoordinates(const Vector<double>& x,
        const Vector<double>& y) {
    m_x = x; m_y = y;
    m_n = m_x.size();
    if(m_y.size() < m_n) m_n = m_y.size();
    if(m_x.size() > m_n) m_x.resize(m_n);
    if(m_y.size() > m_n) m_y.resize(m_n);

    m_screenX.resize(m_n);
    m_screenY.resize(m_n);
    
    updateMinMax();
}

bool RSPolygon::drawAndUpdateBoundingBox(WorldCanvasHolder& wch, String* err) {
    if(m_n < 2 || !updateScreenCoordinates(wch, err)) {
        if(err && m_n < 2) {
            if(err->length() > 0) *err += "\n";
            *err += type() + ": cannot draw < 2 points.";
        }
        return false;
    }
    
    PixelCanvas* pc = wch.worldCanvas()->pixelCanvas();
    setLineProperties(pc);

    pc->drawPolygon(m_screenX, m_screenY);

    // update bounding box
    m_screenXMin = m_screenX(0); m_screenXMax = m_screenX(0);
    m_screenYMin = m_screenY(0); m_screenYMax = m_screenY(0);
    for(unsigned int i = 1; i < m_n; i++) {
        if(m_screenX(i) < m_screenXMin) m_screenXMin = m_screenX(i);
        if(m_screenX(i) > m_screenXMax) m_screenXMax = m_screenX(i);
        if(m_screenY(i) < m_screenYMin) m_screenYMin = m_screenY(i);
        if(m_screenY(i) > m_screenYMax) m_screenYMax = m_screenY(i);
    }

    // adjust for line width
    if(m_width > 0) {
        m_screenXMin -= m_width / 2; m_screenXMax += m_width / 2;
        m_screenYMin -= m_width / 2; m_screenYMax += m_width / 2;
    }
    
    if(!m_label.isEmpty()) {
        // we want the label to be centered above the top
        m_label.draw(pc, (int)(((m_screenXMax + m_screenXMin) / 2) + 0.5),
                         (int)(m_screenYMax + 0.5), true);
    }

    if(m_lt) {
        setLinethroughProperties(pc);

        pc->drawLine((int)(m_screenXMin + 0.5), (int)(m_screenYMax + 0.5),
                     (int)(m_screenXMax + 0.5), (int)(m_screenYMin + 0.5));
    }
    
    if(!m_label.isEmpty()) m_screenYMax += m_label.height(pc);

    // reset line style
    resetLineStyle(pc);
    
    return true;
}

void RSPolygon::getPolygonCoordinates(double& centerX, double& centerY,
        String toSystem) const {
    vector<double> v;
    if(toSystem.empty()) v = coordParameterValues();
    else                 v = coordParameterValues(toSystem);
    centerX = v[0];
    centerY = v[1];
}

#define RSPOLYCOORDSTART                                                      \
    bool toPixel = toSystem == RSUtils::PIXEL;                                \
    if((toPixel && !m_isWorld) || m_lastWCH == NULL) {                        \
        getPolygonCoordinates(x,y,""); return; }                              \
    MDirection::Types toSys;                                                  \
    if((!toPixel && !MDirection::getType(toSys, toSystem)) || (m_isWorld &&   \
        !toPixel && toSys == m_worldSystem)) {                                \
        getPolygonCoordinates(x,y,""); return; }                              \
                                                                              \
    const CoordinateSystem& cs = m_lastWCH->worldCanvas()->coordinateSystem();\
    MDirection::Types wcSys = RSUtils::worldSystem(cs);                       \
                                                                              \
    bool isValid = true;                                                      \
    const Vector<String>& units = m_lastWCH->worldAxisUnits();                \
    vector<double> v(2);                                                      \
    Quantum<Vector<double> > q(Vector<double>(2), UNIT);                      \
    Quantum<Vector<double> > qx(Vector<double>(1), UNIT),                     \
                             qy(Vector<double>(1), UNIT);                     \
    Quantum<double> t(0.0, UNIT), tx(0.0, units(0)), ty(0.0, units(1));       \
    Vector<double> p(2), w(2), px(1), py(1);                                  \
    MDirection m;

#define RSPOLYCOORDCONV(i) {                                                  \
    if(m_isWorld && !toPixel) { /* world to world */                          \
        RSCOORDPOSW2W(m_x(i),m_y(i),0,1)                                      \
    } else if(m_isWorld && toPixel) { /* world to pixel */                    \
        RSCOORDPOSW2P(m_x(i),m_y(i),0,1)                                      \
    } else { /* pixel to world */                                             \
        RSCOORDPOSP2W(m_x(i),m_y(i),0,1)                                      \
    }                                                                         \
    if(!isValid) { getPolygonCoordinates(x,y,""); return; }                   \
    x[i] = v[0]; y[i] = v[1];                                                 \
}

void RSPolygon::getPolygonCoordinates(vector<double>& x, vector<double>& y,
        String toSystem) const {
    x.resize(m_n); y.resize(m_n);
    if(toSystem.empty() || (!m_isWorld && toSystem == RSUtils::PIXEL) ||
       (m_isWorld && toSystem == MDirection::showType(m_worldSystem))) {
        for(unsigned int i = 0; i < m_n; i++) {
            x[i] = m_x(i); y[i] = m_y(i);
        }
    } else {
        RSPOLYCOORDSTART
        for(unsigned int i = 0; i < m_n; i++)
            RSPOLYCOORDCONV(i)
    }
}

vector<double> RSPolygon::coordParameterValues() const {
    vector<double> v(2);
    v[0] = m_centerX; v[1] = m_centerY;
    return v;
}

vector<double> RSPolygon::coordParameterValues(String toSystem) const {
    RSCOORDSTART(2)
    
    // center x, center y
    RSCOORDPOSCONV(m_centerX, m_centerY, 0, 1)
    
    RSCOORDEND
}

vector<String> RSPolygon::coordParameterNames() const {
    static vector<String> v(2);
    v[0] = "center x"; v[1] = "center y";
    return v;
}

vector<RegionShape::CoordinateParameterType>
RSPolygon::coordParameterTypes() const {
    static vector<CoordinateParameterType> v(2);
    v[0] = POSITION; v[1] = POSITION;
    return v;
}

void RSPolygon::setCoordParameters(const vector<double>& vals) {
    if(vals.size() < 2 || m_n < 2) return;
    double dx = vals[0] - m_centerX, dy = vals[1] - m_centerY;
    bool changed = dx != 0 || dy != 0;
    if(changed) move(dx, dy);
}

void RSPolygon::setCoordParameters(const vector<double>& vals, String system) {
    RSSCOORDSTART(2)
    
    // center x, center y
    RSSCOORDPOSCONV_(0, 1)
    
    RSSCOORDEND
}

void RSPolygon::move(double dx, double dy, String system) {
    if(dx == 0 && dy == 0) return;
    
    if(system.empty() || (m_isWorld && system == MDirection::showType(
       m_worldSystem)) || (!m_isWorld && system == RSUtils::PIXEL)) {
        for(unsigned int i = 0; i < m_n; i++) {
            m_x(i) += dx;
            m_y(i) += dy;
        }
    } else {
        if(m_lastWCH == NULL) return;
        if(system == RSUtils::PIXEL) { // pixel to world
            Quantum<Vector<double> > wx(m_x, UNIT), wy(m_y, UNIT);
            Vector<double> px(m_n), py(m_n);
            if(!worldToPixel(wx, wy, px, py, *m_lastWCH)) return;
            for(unsigned int i = 0; i < m_n; i++) {
                px(i) += dx;
                py(i) += dy;
            }
            if(!pixelToWorld(px, py, wx, wy, *m_lastWCH)) return;
            
        } else if(m_isWorld) {         // world to world
            MDirection::Types sys;
            if(!MDirection::getType(sys, system)) return;
            Quantum<Vector<double> > q1(Vector<double>(2), UNIT),
                                     q2(Vector<double>(2), UNIT);
            for(unsigned int i = 0; i < m_n; i++) {
                q1.getValue()(0) = m_x(i); q1.getValue()(1) = m_y(i);
                if(!RSUtils::convertWCS(q1, m_worldSystem, q2, sys)) return;
                q2.getValue()(0) += dx; q2.getValue()(1) += dy;
                if(!RSUtils::convertWCS(q2, sys, q1, m_worldSystem)) return;
                m_x(i) = q1.getValue()(0); m_y(i) = q1.getValue()(1);
            }
            
        } else {                       // world to pixel
            MDirection::Types sys;
            if(!MDirection::getType(sys, system)) return;
            Quantum<Vector<double> > wx(Vector<double>(m_n), UNIT),
                                     wy(Vector<double>(m_n), UNIT);
            if(!RSUtils::pixelToWorld(m_x, m_y, wx, wy, *m_lastWCH, sys)) return;
            for(unsigned int i = 0; i < m_n; i++) {
                wx.getValue()(i) += dx;
                wy.getValue()(i) += dy;
            }
            if(!RSUtils::worldToPixel(wx, wy, m_x, m_y, *m_lastWCH, sys)) return;
        }
    }
    
    updateMinMax();
    refresh(true);
}

RSHandle RSPolygon::getHandle() const {
    if(m_lastWCH == NULL || m_width <= 0) return RSHandle();
    
    vector<double> x(4), y(4);
    x[0] = x[1] = m_screenXMin;
    x[2] = x[3] = m_screenXMax;
    y[0] = y[2] = m_screenYMin;
    if(m_label.isEmpty()) y[1] = y[3] = m_screenYMax;
    else y[1] = y[3] = m_screenYMax - m_label.height(
                       m_lastWCH->worldCanvas()->pixelCanvas());
    return RSHandle(x, y);
}

vector<String> RSPolygon::optionNames() const {
    return vector<String>(1, "Custom name");
}

vector<RegionShape::OptionType> RSPolygon::optionTypes() const {
    return vector<OptionType>(1, STRING);
}

vector<RSOption> RSPolygon::optionValues() const {
    return vector<RSOption>(1, RSOption(m_custName));
}

void RSPolygon::setOptionValues(const vector<RSOption>& options) {
    if(options.size() >= 1 && options[0].isString() && options[0].asString() !=
       m_custName) {
        m_custName = options[0].asString();
        refresh(true);
    }
}

void RSPolygon::getShapeSpecificProperties(Record& properties) const {
    properties.define(PROPTYPE, PROPTYPE_POLYGON);
    properties.define(PROPXCOORDINATES, m_x);
    properties.define(PROPYCOORDINATES, m_y);
    
    // remove PROPCOORDINATES
    properties.removeField(PROPCOORDINATES);
}

void RSPolygon::setShapeSpecificProperties(const RecordInterface& p) {
    bool x = p.isDefined(PROPXCOORDINATES) &&
             p.dataType(PROPXCOORDINATES) == TpArrayDouble,
         y = p.isDefined(PROPYCOORDINATES) &&
             p.dataType(PROPYCOORDINATES) == TpArrayDouble;
    if(x || y) {
        setPolygonCoordinates(!x ? m_x :
                             Vector<double>(p.asArrayDouble(PROPXCOORDINATES)),
                             !y ? m_y :
                             Vector<double>(p.asArrayDouble(PROPYCOORDINATES)));
    }
}

bool RSPolygon::updateScreenCoordinates(WorldCanvasHolder& wch, String* err) {
    Vector<double> linX(m_n), linY(m_n);

    if(m_isWorld) {
        Quantum<Vector<Double> > wx(m_x, UNIT), wy(m_y, UNIT);
        if(!worldToLinear(wx, wy, linX, linY, wch, true, err)) return false;
    } else {
        if(!RSUtils::pixelToLinear(m_x, m_y, linX, linY, wch, err))
            return false;
    }

    return RSUtils::linearToScreen(linX, linY, m_screenX, m_screenY, wch, err);
}

void RSPolygon::updateMinMax() {
    if(m_n == 0) {
        m_xMin = 0; m_xMax = 0; m_yMin = 0; m_yMax = 0;
        m_centerX = 0; m_centerY = 0;
        return;
    }
    
    // update min/max
    m_xMin = m_x(0); m_xMax = m_x(0); m_yMin = m_y(0); m_yMax = m_y(0);
    for(unsigned int i = 1; i < m_n; i++) {
        if(m_x(i) < m_xMin) m_xMin = m_x(i);
        if(m_x(i) > m_xMax) m_xMax = m_x(i);
        if(m_y(i) < m_yMin) m_yMin = m_y(i);
        if(m_y(i) > m_yMax) m_yMax = m_y(i);
    }
    
    // update center
    m_centerX = (m_xMin + m_xMax) / 2;
    m_centerY = (m_yMin + m_yMax) / 2;
}


// RSLINE DEFINITIONS //

RSLine::RSLine(double x1, double y1, double x2, double y2,
        MDirection::Types worldSystem, int len, bool arrow1, bool arrow2,
        ArrowType type1, ArrowType type2): RegionShape(worldSystem), m_spec(4),
        m_screenSpec(4), m_arrowLength(len), m_arrow1(arrow1),m_arrow2(arrow2),
        m_aType1(type1), m_aType2(type2) {
    m_spec[0] = x1; m_spec[1] = y1;
    m_spec[2] = x2; m_spec[3] = y2;
    
    updateMinMax();
}

RSLine::RSLine(double x1, double y1, double x2, double y2, int length,
        bool arrow1, bool arrow2, ArrowType type1, ArrowType type2) :
        RegionShape(), m_spec(4), m_screenSpec(4), m_arrowLength(length),
        m_arrow1(arrow1), m_arrow2(arrow2), m_aType1(type1), m_aType2(type2) {
    m_spec[0] = x1; m_spec[1] = y1;
    m_spec[2] = x2; m_spec[3] = y2;
    
    updateMinMax();
}

RSLine::RSLine(const RecordInterface& props) : m_spec(4), m_screenSpec(4),
        m_arrowLength(7), m_arrow1(false), m_arrow2(false),
        m_aType1(FilledDoubleV), m_aType2(FilledDoubleV) {
    setProperties(props);
}

RSLine::~RSLine() { }

bool RSLine::drawAndUpdateBoundingBox(WorldCanvasHolder& wch, String* err) {
    if(!updateScreenCoordinates(wch, err)) return false;
    
    PixelCanvas* pc = wch.worldCanvas()->pixelCanvas();
    setLineProperties(pc);
    
    pc->drawLine(m_screenSpec[0], m_screenSpec[1],
                 m_screenSpec[2], m_screenSpec[3]);

    if(m_arrow1 || m_arrow2) {
        double x1, y1, x2, y2;
        // use a method written for qwt plotter to find arrow points
        if(m_arrow1) {
            arrowPoints(m_screenSpec[2], m_screenSpec[3],
                        m_screenSpec[0], m_screenSpec[1],
                        m_arrowLength, x1, y1, x2, y2);
            Vector<double> polyX(3), polyY(3);
            polyX(0) = m_screenSpec[0]; polyY(0) = m_screenSpec[1];
            polyX(1) = x1;              polyY(1) = y1;
            polyX(2) = x2;              polyY(2) = y2;
            switch(m_aType1) {                
            case FilledTriangle: pc->drawFilledPolygon(polyX, polyY); break;
            case Triangle:       pc->drawPolygon(polyX, polyY); break;
            case V:       pc->drawLine(polyX(0), polyY(0), polyX(1), polyY(1));
                   pc->drawLine(polyX(0), polyY(0), polyX(2), polyY(2)); break;
            case FilledDoubleV: {
                double dx = m_screenSpec[2] - m_screenSpec[0],
                       dy = m_screenSpec[3] - m_screenSpec[1];
                double a;
                if(dx == 0) a = C::pi / 2;
                else        a = std::atan(dy / dx);
                dx = (m_arrowLength / 3.0) * std::cos(a);
                dy = (m_arrowLength / 3.0) * std::sin(a);
                polyX.resize(4);     polyY.resize(4);
                polyX(0) = m_screenSpec[0]; polyY(0) = m_screenSpec[1];
                polyX(1) = x1;              polyY(1) = y1;
                polyX(3) = x2;              polyY(3) = y2;
                polyX(2) = m_screenSpec[0] + dx;
                polyY(2) = m_screenSpec[1] + dy;
                pc->drawFilledPolygon(polyX, polyY);
                break; }
            }            
        }
        if(m_arrow2) {
            arrowPoints(m_screenSpec[0], m_screenSpec[1],
                        m_screenSpec[2], m_screenSpec[3],
                        m_arrowLength, x1, y1, x2, y2);
            Vector<double> polyX(3), polyY(3);
            polyX(0) = m_screenSpec[2]; polyY(0) = m_screenSpec[3];
            polyX(1) = x1;              polyY(1) = y1;
            polyX(2) = x2;              polyY(2) = y2;
            switch(m_aType2) {                
            case FilledTriangle: pc->drawFilledPolygon(polyX, polyY); break;
            case Triangle:       pc->drawPolygon(polyX, polyY); break;
            case V:       pc->drawLine(polyX(0), polyY(0), polyX(1), polyY(1));
                   pc->drawLine(polyX(0), polyY(0), polyX(2), polyY(2)); break;
            case FilledDoubleV: {
                double dx = m_screenSpec[2] - m_screenSpec[0],
                       dy = m_screenSpec[3] - m_screenSpec[1];
                double a;
                if(dy == 0) a = 0;
                else        a = std::atan(dx / dy);
                dx = (m_arrowLength / 3.0) * std::sin(a);
                dy = (m_arrowLength / 3.0) * std::cos(a);
                polyX.resize(4);     polyY.resize(4);
                polyX(0) = m_screenSpec[2]; polyY(0) = m_screenSpec[3];
                polyX(1) = x1;              polyY(1) = y1;
                polyX(3) = x2;              polyY(3) = y2;
                polyX(2) = m_screenSpec[2] - dx;
                polyY(2) = m_screenSpec[3] - dy;
                pc->drawFilledPolygon(polyX, polyY);
                break; }
            }   
        }
    }
    
    // update bounding box
    m_screenXMin = m_screenSpec[0]; m_screenXMax = m_screenSpec[0];
    m_screenYMin = m_screenSpec[1]; m_screenYMax = m_screenSpec[1];
    if(m_screenSpec[2] < m_screenXMin) m_screenXMin = m_screenSpec[2];
    if(m_screenSpec[2] > m_screenXMax) m_screenXMax = m_screenSpec[2];
    if(m_screenSpec[3] < m_screenYMin) m_screenYMin = m_screenSpec[3];
    if(m_screenSpec[3] > m_screenYMax) m_screenYMax = m_screenSpec[3];
    
    // adjust for line width
    if(m_width > 0) {
        m_screenXMin -= m_width / 2; m_screenXMax += m_width / 2;
        m_screenYMin -= m_width / 2; m_screenYMax += m_width / 2;
    }
    
    if(!m_label.isEmpty()) {
        // we want the label to be centered above the top
        m_label.draw(pc, (int)(((m_screenSpec[2] + m_screenSpec[0])/ 2) + 0.5),
                         (int)(m_screenYMax + 0.5), true);
    }

    if(m_lt) {        
        setLinethroughProperties(pc);

        pc->drawLine((int)(m_screenXMin + 0.5), (int)(m_screenYMax + 0.5),
                     (int)(m_screenXMax + 0.5), (int)(m_screenYMin + 0.5));
    }
    
    if(!m_label.isEmpty()) m_screenYMax += m_label.height(pc);

    // reset line style
    resetLineStyle(pc);
    
    return true;
}

void RSLine::getLineCoordinates(double& x1, double& y1, double& x2,
        double& y2, int& len, String toSystem) const {
    vector<double> v;
    if(toSystem.empty()) v = coordParameterValues();
    else                 v = coordParameterValues(toSystem);
    x1  = v[0];
    y1  = v[1];
    x2  = v[2];
    y2  = v[3];
    len = (int)(v[4] + 0.5);
}

vector<double> RSLine::coordParameterValues() const {
    vector<double> v = m_spec;
    v.push_back(m_arrowLength);
    return v;
}

vector<double> RSLine::coordParameterValues(String toSystem) const {
    RSCOORDSTART(5)
    
    // x1, y1
    RSCOORDPOSCONV_(0, 1)
    
    // x2, y2
    RSCOORDPOSCONV_(2, 3)
    
    // length
    RSCOORDNOCONV(m_arrowLength, 4)
    
    RSCOORDEND
}

vector<String> RSLine::coordParameterNames() const {
    static vector<String> v(5);
    v[0] = "x1"; v[1] = "y1"; v[2] = "x2"; v[3] = "y2";
    v[4] = "arrow length";
    return v;
}

vector<RegionShape::CoordinateParameterType>
RSLine::coordParameterTypes() const {
    static vector<CoordinateParameterType> v(5);
    v[0] = POSITION; v[1] = POSITION; v[2] = POSITION; v[3] = POSITION;
    v[4] = PIXEL;
    return v;
}

void RSLine::setCoordParameters(const vector<double>& vals) {
    if(vals.size() < m_spec.size() + 1) return;
    bool changed = false;
    for(unsigned int i = 0; i < m_spec.size(); i++) {
        changed |= m_spec[i] != vals[i];
        m_spec[i] = vals[i];
    }
    changed |= (int)(vals[4] + 0.5) != m_arrowLength;
    m_arrowLength = (int)(vals[4] + 0.5);
    if(changed) {
        updateMinMax();
        refresh(true);
    }
}

void RSLine::setCoordParameters(const vector<double>& vals, String system) {
    RSSCOORDSTART(5)
    
    // x1, y1
    RSSCOORDPOSCONV_(0, 1)
    
    // x2, y2
    RSSCOORDPOSCONV_(2, 3)
    
    // len
    RSSCOORDNOCONV(m_arrowLength, 4)
    
    RSSCOORDEND
}

void RSLine::move(double dx, double dy, String system) {
    if(dx == 0 && dy == 0) return;
    
    if(system.empty() || (m_isWorld && system == MDirection::showType(
       m_worldSystem)) || (!m_isWorld && system == RSUtils::PIXEL)) {
        m_spec[0] += dx; m_spec[1] += dy;
        m_spec[2] += dx; m_spec[3] += dy;
    } else {
        if(m_lastWCH == NULL) return;
        if(system == RSUtils::PIXEL) { // pixel to world
            Quantum<Vector<double> > wx(Vector<double>(2), UNIT),
                                     wy(Vector<double>(2), UNIT);
            wx.getValue()(0) = m_spec[0]; wy.getValue()(0) = m_spec[1];
            wx.getValue()(1) = m_spec[2]; wy.getValue()(1) = m_spec[3];
            Vector<double> px(2), py(2);
            if(!worldToPixel(wx, wy, px, py, *m_lastWCH)) return;
            px(0) += dx; py(0) += dy;
            px(1) += dx; py(1) += dy;
            if(!pixelToWorld(px, py, wx, wy, *m_lastWCH)) return;
            m_spec[0] = wx.getValue()(0); m_spec[1] = wy.getValue()(0);
            m_spec[2] = wx.getValue()(1); m_spec[3] = wy.getValue()(1);
        } else if(m_isWorld) {         // world to world
            MDirection::Types sys;
            if(!MDirection::getType(sys, system)) return;
            Quantum<Vector<double> > q1(Vector<double>(2), UNIT),
                                     q2(Vector<double>(2), UNIT);
            q1.getValue()(0) = m_spec[0]; q1.getValue()(1) = m_spec[1];
            if(!RSUtils::convertWCS(q1, m_worldSystem, q2, sys)) return;
            q2.getValue()(0) += dx; q2.getValue()(1) += dy;
            if(!RSUtils::convertWCS(q2, sys, q1, m_worldSystem)) return;
            m_spec[0] = q1.getValue()(0); m_spec[1] = q1.getValue()(1);
            q1.getValue()(0) = m_spec[2]; q1.getValue()(1) = m_spec[3];
            if(!RSUtils::convertWCS(q1, m_worldSystem, q2, sys)) return;
            q2.getValue()(0) += dx; q2.getValue()(1) += dy;
            if(!RSUtils::convertWCS(q2, sys, q1, m_worldSystem)) return;
            m_spec[2] = q1.getValue()(0); m_spec[3] = q1.getValue()(1);
            
        } else {                       // world to pixel
            MDirection::Types sys;
            if(!MDirection::getType(sys, system)) return;
            Quantum<Vector<double> > wx(Vector<double>(2), UNIT),
                                     wy(Vector<double>(2), UNIT);
            Vector<double> px(2), py(2);
            px(0) = m_spec[0]; py(0) = m_spec[1];
            px(1) = m_spec[2]; py(1) = m_spec[3];
            if(!RSUtils::pixelToWorld(px, py, wx, wy, *m_lastWCH, sys)) return;
            wx.getValue()(0) += dx; wy.getValue()(0) += dy;
            wx.getValue()(1) += dx; wy.getValue()(1) += dy;
            if(!RSUtils::worldToPixel(wx, wy, px, py, *m_lastWCH, sys)) return;
            m_spec[0] = px(0); m_spec[1] = py(0);
            m_spec[2] = px(1); m_spec[3] = py(1);
        }
    }
    
    updateMinMax();
    refresh(true);
}

RSHandle RSLine::getHandle() const {
    if(m_lastWCH == NULL || m_width <= 0) return RSHandle();
    
    double centerX = (m_screenSpec[0] + m_screenSpec[2]) / 2,
           centerY = (m_screenSpec[1] + m_screenSpec[3]) / 2;
    
    double width = (sqrt(pow(m_screenSpec[2] - m_screenSpec[0], 2) +
                    pow(m_screenSpec[3] - m_screenSpec[1], 2)) + m_width) / 2;
    
    double height = m_width / 2;
    if(m_arrow1 || m_arrow2) height += m_arrowLength / sqrt(2.0);
    
    double alpha = C::pi / 2;
    if(m_screenSpec[2] != m_screenSpec[0])
        alpha = std::atan((m_screenSpec[3] - m_screenSpec[1]) /
                          (m_screenSpec[2] - m_screenSpec[0]));
    alpha *= 180 / C::pi;
    
    Vector<double> x(4), y(4);
    RSRectangle::getRectanglePoints(centerX, centerY, width,height,alpha,x,y);
    return RSHandle(x, y);
}

vector<String> RSLine::optionNames() const {
    vector<String> v(4);
    v[0] = "Arrow 1";      v[1] = "Arrow 2";
    v[2] = "Arrow Type 1"; v[3] = "Arrow Type 2";
    return v;
}

vector<RegionShape::OptionType> RSLine::optionTypes() const {
    vector<OptionType> v(4, BOOL);
    v[2] = STRINGCHOICE; v[3] = STRINGCHOICE;
    return v;
}

vector<vector<String> > RSLine::optionChoices() const {
    vector<vector<String> > v(4);
    v[2] = allArrowTypesStrings(); v[3] = allArrowTypesStrings();
    return v;
}

vector<RSOption> RSLine::optionValues() const {
    vector<RSOption> v(4);
    v[0] = m_arrow1;            v[1] = m_arrow2;
    v[2] = arrowType(m_aType1); v[3] = arrowType(m_aType2);
    return v;
}

void RSLine::setOptionValues(const vector<RSOption>& options) {
    if(options.size() >= 4 && options[0].isBool() && options[1].isBool() &&
       options[2].isString() && options[3].isString() &&
       (options[0].asBool() != m_arrow1 || options[1].asBool() != m_arrow2 ||
        options[2].asString() != arrowType(m_aType1) ||
        options[3].asString() != arrowType(m_aType2))) {
        m_arrow1 = options[0].asBool();
        m_arrow2 = options[1].asBool();
        m_aType1 = arrowType(options[2].asString());
        m_aType2 = arrowType(options[3].asString());
        refresh(true);
    }
}

bool RSLine::updateScreenCoordinates(WorldCanvasHolder& wch, String* err) {
    Vector<double> linX(2), linY(2);
    Vector<double> sx(2), sy(2);

    if(m_isWorld) {
        Vector<Double> vx(2); vx(0) = m_spec[0]; vx(1) = m_spec[2];
        Vector<Double> vy(2); vy(0) = m_spec[1]; vy(1) = m_spec[3];
        Quantum<Vector<Double> > wx(vx, UNIT), wy(vy, UNIT);
        if(!worldToLinear(wx, wy, linX, linY, wch, true, err)) return false;
    } else {
        Vector<double> px(2); px(0) = m_spec[0]; px(1) = m_spec[2];
        Vector<double> py(2); py(0) = m_spec[1]; py(1) = m_spec[3];
        if(!RSUtils::pixelToLinear(px, py, linX, linY, wch, err)) return false;
    }    
    if(!RSUtils::linearToScreen(linX, linY, sx, sy, wch, err)) return false;
    m_screenSpec[0] = sx(0); m_screenSpec[1] = sy(0);
    m_screenSpec[2] = sx(1); m_screenSpec[3] = sy(1);
    
    return true;
}

void RSLine::updateMinMax() {
    m_xMin = m_spec[0]; m_xMax = m_spec[0];
    m_yMin = m_spec[1]; m_yMax = m_spec[1];
    if(m_spec[2] < m_xMin) m_xMin = m_spec[2];
    if(m_spec[2] > m_xMax) m_xMax = m_spec[2];
    if(m_spec[3] < m_yMin) m_yMin = m_spec[3];
    if(m_spec[3] > m_yMax) m_yMax = m_spec[3];
}

bool RSLine::arrowPoints(double x_1, double y_1, double x_2, double y_2,
        double length, double& resX1, double& resY1, double& resX2,
        double& resY2) {
    if((x_1 == x_2 && y_1 == y_2) || length <= 0) return false;
    
    double x1 = x_1, y1 = y_1, x2 = x_2, y2 = y_2;
    if(x1 > x2) {
        double temp = x1;
        x1 = x2;
        x2 = temp;
        temp = y1;
        y1 = y2;
        y2 = temp;
    }
    
    if(!arrowPointsHelper(x1, y1, x2, y2, length, resX1, resY1, resX2, resY2))
        return false;
    
    if(x_1 > x_2) {
        double dx = x_1 - resX1;
        double dy = y_1 - resY1;
        resX1 = x_2 + dx;
        resY1 = y_2 + dy;
        
        dx = x_1 - resX2;
        dy = y_1 - resY2;
        resX2 = x_2 + dx;
        resY2 = y_2 + dy;
    }

    return true;
}

bool RSLine::arrowPointsHelper(double x1, double y1, double x2, double y2,
        double length, double& resX1, double& resY1, double& resX2,
        double& resY2) {
    double alpha = 0;
    if(x2 != x1)     alpha = std::atan((y2 - y1) / (x2 - x1));
    else if(y2 > y1) alpha = C::pi / 2;
    else             alpha = C::pi * 3 / 2;
    double x = x2, y = y2;
    
    double i1, i2, j1, j2;
    
    // i1, j1
    double beta = alpha - (C::pi / 4);
    if(beta == 0) {
        i1 = x - length;
        j1 = y;
    } else if(beta > 0) {
        i1 = x - (length * cos(beta));
        j1 = y - (length * sin(beta));
    } else {
        beta = -beta;
        i1 = x - (length * cos(beta));
        j1 = y + (length * sin(beta));
    }
    resX1 = i1; resY1 = j1;
    
    // i2, j2
    alpha = (C::pi / 2) - alpha;
    beta = alpha - (C::pi / 4);
    if(beta == 0) {
        i2 = x;
        j2 = y - length;
    } else if(beta > 0) {
        i2 = x - (length * sin(beta));
        j2 = y - (length * cos(beta));
    } else {
        beta = -beta;
        i2 = x + (length * sin(beta));
        j2 = y - (length * cos(beta));
    }
    resX2 = i2; resY2 = j2;
    
    return true;
}


// RSVECTOR DEFINITIONS //

RSVector::RSVector(double x, double y, double length, double angle,
        MDirection::Types worldSystem, int arrowLength, bool arrow,
        ArrowType aType) : RSLine(x, y, 0, 0, worldSystem, arrowLength, false,
        arrow, aType, aType), m_length(length), m_angle(angle) {
    double rad = angle * (C::pi / 180);
    m_spec[2] = x + (std::cos(rad) * length);
    m_spec[3] = y + (std::sin(rad) * length);
    
    updateMinMax();
}

RSVector::RSVector(double x, double y, double length, double angle,
        int arrowLength, bool arrow, ArrowType aType) : RSLine(x, y, 0, 0,
        arrowLength, false, arrow, aType, aType), m_length(length),
        m_angle(angle) {
    double rad = angle * (C::pi / 180);
    m_spec[2] = x + std::cos(rad) * length;
    m_spec[3] = y + std::sin(rad) * length;
    
    updateMinMax();
}

RSVector::RSVector(const RecordInterface& props) : RSLine(props) {
    setProperties(props);
}

RSVector::~RSVector() { }

void RSVector::getVectorCoordinates(double& x, double& y, double& angle,
        double& length, int& arrowLength, String toSystem) const {
    vector<double> v;
    if(toSystem.empty()) v = coordParameterValues();
    else                 v = coordParameterValues(toSystem);
    x           = v[0];
    y           = v[1];
    angle       = v[2];
    length      = v[3];
    arrowLength = (int)(v[4] + 0.5);
}

vector<double> RSVector::coordParameterValues() const {
    vector<double> v(5);
    v[0] = m_spec[0]; v[1] = m_spec[1];
    v[2] = m_angle;   v[3] = m_length;
    v[4] = m_arrowLength;
    return v;
}

vector<double> RSVector::coordParameterValues(String toSystem) const {
    RSCOORDSTART(5)
    
    // x, y
    RSCOORDPOSCONV_(0, 1)
    
    // angle
    RSCOORDANGCONV(m_spec[0], m_spec[1], m_angle, m_length, 2)
    
    // length
    RSCOORDSIZECONV(m_spec[0], m_spec[1], m_length, true, 3)
    
    // arrow length
    RSCOORDNOCONV(m_arrowLength, 4)
    
    RSCOORDEND
}

vector<String> RSVector::coordParameterNames() const {
    static vector<String> v(5);
    v[0] = "x";     v[1] = "y";
    v[2] = "angle"; v[3] = "length";
    v[4] = "Arrow Length";
    return v;
}

vector<RegionShape::CoordinateParameterType>
RSVector::coordParameterTypes() const {
    static vector<CoordinateParameterType> v(4);
    v[0] = POSITION; v[1] = POSITION; v[2] = ANGLE; v[3] = SIZE;
    v[4] = PIXEL;
    return v;
}

void RSVector::setCoordParameters(const vector<double>& vals) {
    if(vals.size() < 5) return;
    bool changed = m_spec[0] != vals[0] || m_spec[1] != vals[1] ||
                   m_angle != vals[2] || m_length != vals[3] ||
                   m_arrowLength != (int)(vals[4] + 0.5);
    if(changed) {
        m_spec[0] = vals[0];
        m_spec[1] = vals[1];
        m_angle =   vals[2];
        
        while(m_angle < 0) m_angle += 360;
        while(m_angle >= 360) m_angle -= 360;
        
        m_length =  vals[3];
        double rad = m_angle * (C::pi / 180);
        m_spec[2] = m_spec[0] + (std::cos(rad) * m_length);
        m_spec[3] = m_spec[1] + (std::sin(rad) * m_length);
        
        m_arrowLength = (int)(vals[4] + 0.5);
        
        updateMinMax();
        refresh(true);
    }
}

void RSVector::setCoordParameters(const vector<double>& vals, String system) {
    RSSCOORDSTART(5)
    
    // x, y
    RSSCOORDPOSCONV_(0, 1)
    
    // angle
    RSSCOORDANGCONV(vals[0], vals[1], vals[2], vals[3], 2)
    
    // length
    RSSCOORDSIZECONV(vals[0], vals[1], vals[3], true, 3)
    
    // arrow length
    RSSCOORDNOCONV(m_arrowLength, 4)
    
    RSSCOORDEND
}

vector<String> RSVector::optionNames() const {
    vector<String> v(2);
    v[0] = "Arrow"; v[1] = "Arrow Type";
    return v;
}

vector<RegionShape::OptionType> RSVector::optionTypes() const {
    vector<OptionType> v(2, BOOL);
    v[1] = STRINGCHOICE;
    return v;
}

vector<vector<String> > RSVector::optionChoices() const {
    vector<vector<String> > v(2);
    v[1] = allArrowTypesStrings();
    return v;
}

vector<RSOption> RSVector::optionValues() const {
    vector<RSOption> v(2);
    v[0] = m_arrow2; v[1] = arrowType(m_aType2);
    return v;
}

void RSVector::setOptionValues(const vector<RSOption>& options) {
    if(options.size() >= 2 && options[0].isBool() && options[1].isString() &&
       (options[0].asBool() != m_arrow2 ||
        options[1].asString() != arrowType(m_aType2))) {
        m_arrow2 = options[0].asBool();
        m_aType2 = arrowType(options[1].asString());
        refresh(true);
    }
}


// RSMARKER DEFINITIONS //

RSMarker::RSMarker(double x, double y, Display::Marker marker,
        int pixelHeight, MDirection::Types worldSys) : RegionShape(worldSys),
        m_x(x), m_y(y), m_markers(1, marker), m_pixelHeight(pixelHeight) {
    updateMinMax();
}

RSMarker::RSMarker(double x, double y, const vector<Display::Marker>& markers,
        int pixelHeight, MDirection::Types worldSys) : RegionShape(worldSys),
        m_x(x), m_y(y), m_markers(markers), m_pixelHeight(pixelHeight) {
    updateMinMax();
}

RSMarker::RSMarker(double x, double y, Display::Marker marker, int pHeight) :
        RegionShape(), m_x(x), m_y(y), m_markers(1, marker),
        m_pixelHeight(pHeight) {
    updateMinMax();
}

RSMarker::RSMarker(double x, double y, const vector<Display::Marker>& markers,
        int pixelHeight) : RegionShape(), m_x(x), m_y(y), m_markers(markers),
        m_pixelHeight(pixelHeight) {
    updateMinMax();
}

RSMarker::RSMarker(const RecordInterface& props) : m_x(0), m_y(0),
        m_markers(1, Display::X), m_pixelHeight(10) {
    setProperties(props);
}

RSMarker::~RSMarker() { }

vector<Display::Marker> RSMarker::markers() const { return m_markers; }

void RSMarker::setMarker(Display::Marker marker) {
    if(m_markers.size() != 1 || m_markers[0] != marker) {
        m_markers = vector<Display::Marker>(1, marker);
        refresh(true);
    }
}

void RSMarker::setMarkers(const vector<Display::Marker>& markers) {
    if(m_markers != markers) {
        m_markers = markers;
        refresh(true);
    }
}

bool RSMarker::drawAndUpdateBoundingBox(WorldCanvasHolder& wch, String* err) {
    if(!updateScreenCoordinates(wch, err)) return false;
    
    PixelCanvas* pc = wch.worldCanvas()->pixelCanvas();
    setLineProperties(pc);
    
    for(unsigned int i = 0; i < m_markers.size(); i++)
        pc->drawMarker(m_screenX, m_screenY, m_markers[i], m_pixelHeight);

    // update bounding box
    m_screenXMin = m_screenX - (m_pixelHeight / 2);
    m_screenXMax = m_screenX + (m_pixelHeight / 2);
    m_screenYMin = m_screenY - (m_pixelHeight / 2);
    m_screenYMax = m_screenY + (m_pixelHeight / 2);
    
    // adjust for line width
    if(m_width > 0) {
        m_screenXMin -= m_width / 2; m_screenXMax += m_width / 2;
        m_screenYMin -= m_width / 2; m_screenYMax += m_width / 2;
    }
    
    if(!m_label.isEmpty()) {
        // we want the label to be centered above the top
        m_label.draw(pc, (int)(m_screenX + 0.5),
                         (int)(m_screenYMax + 0.5), true);
    }

    if(m_lt) {
        setLinethroughProperties(pc);

        pc->drawLine((int)(m_screenXMin + 0.5), (int)(m_screenYMax + 0.5),
                     (int)(m_screenXMax + 0.5), (int)(m_screenYMin + 0.5));
    }
    
    if(!m_label.isEmpty()) m_screenYMax += m_label.height(pc);

    // reset line style
    resetLineStyle(pc);
    
    return true;
}

void RSMarker::getMarkerCoordinates(double& x, double& y, int& pheight,
        String toSystem) const {
    vector<double> v;
    if(toSystem.empty()) v = coordParameterValues();
    else                 v = coordParameterValues(toSystem);
    x       = v[0];
    y       = v[1];
    pheight = (int)(v[2] + 0.5);
}

vector<double> RSMarker::coordParameterValues() const {
    vector<double> v(3);
    v[0] = m_x; v[1] = m_y; v[2] = m_pixelHeight;
    return v;
}

vector<double> RSMarker::coordParameterValues(String toSystem) const {
    RSCOORDSTART(3)
    
    // x, y
    RSCOORDPOSCONV(m_x, m_y, 0, 1)
    
    // height
    RSCOORDNOCONV(m_pixelHeight, 2)
    
    RSCOORDEND
}

vector<String> RSMarker::coordParameterNames() const {
    static vector<String> v(3);
    v[0] = "x"; v[1] = "y"; v[2] = "height";
    return v;
}

vector<RegionShape::CoordinateParameterType>
RSMarker::coordParameterTypes() const {
    static vector<CoordinateParameterType> v(3);
    v[0] = POSITION; v[1] = POSITION; v[2] = PIXEL;
    return v;
}

void RSMarker::setCoordParameters(const vector<double>& vals) {
    if(vals.size() < 3) return;
    bool changed = m_x != vals[0] || m_y != vals[1] ||
                   m_pixelHeight != (int)(vals[2] + 0.5);
    if(changed) {
        m_x = vals[0]; m_y = vals[1]; m_pixelHeight = (int)(vals[2] + 0.5);
        updateMinMax();
        refresh(true);
    }
}

void RSMarker::setCoordParameters(const vector<double>& vals, String system) {
    RSSCOORDSTART(3)
    
    // x, y
    RSSCOORDPOSCONV_(0, 1)
    
    // height
    RSSCOORDNOCONV(m_pixelHeight, 2)
    
    RSSCOORDEND
}

void RSMarker::move(double dx, double dy, String system) {
    if(dx == 0 && dy == 0) return;
    
    if(system.empty() || (m_isWorld && system == MDirection::showType(
       m_worldSystem)) || (!m_isWorld && system == RSUtils::PIXEL)) {
        m_x += dx; m_y += dy;
    } else {
        if(m_lastWCH == NULL) return;
        if(system == RSUtils::PIXEL) { // pixel to world
            Quantum<Vector<double> > wx(Vector<double>(1, m_x), UNIT),
                                     wy(Vector<double>(1, m_y), UNIT);
            Vector<double> px(1), py(1);
            if(!worldToPixel(wx, wy, px, py, *m_lastWCH)) return;
            px(0) += dx; py(0) += dy;
            if(!pixelToWorld(px, py, wx, wy, *m_lastWCH)) return;
            m_x = wx.getValue()(0); m_y = wy.getValue()(0);
        } else if(m_isWorld) {         // world to world
            MDirection::Types sys;
            if(!MDirection::getType(sys, system)) return;
            Quantum<Vector<double> > q1(Vector<double>(2), UNIT),
                                     q2(Vector<double>(2), UNIT);
            q1.getValue()(0) = m_x; q1.getValue()(1) = m_y;
            if(!RSUtils::convertWCS(q1, m_worldSystem, q2, sys)) return;
            q2.getValue()(0) += dx; q2.getValue()(1) += dy;
            if(!RSUtils::convertWCS(q2, sys, q1, m_worldSystem)) return;
            m_x = q1.getValue()(0); m_y = q1.getValue()(1);
        } else {                       // world to pixel
            MDirection::Types sys;
            if(!MDirection::getType(sys, system)) return;
            Quantum<Vector<double> > wx(Vector<double>(1), UNIT),
                                     wy(Vector<double>(1), UNIT);
            Vector<double> px(1, m_x), py(1, m_y);
            if(!RSUtils::pixelToWorld(px, py, wx, wy, *m_lastWCH, sys)) return;
            wx.getValue()(0) += dx; wy.getValue()(0) += dy;
            if(!RSUtils::worldToPixel(wx, wy, px, py, *m_lastWCH, sys)) return;
            m_x = px(0); m_y = py(0);
        }
    }
    
    updateMinMax();
    refresh(true);
}

RSHandle RSMarker::getHandle() const {
    if(m_lastWCH == NULL) return RSHandle();
    
    vector<double> x(4), y(4);
    double h = m_pixelHeight / 2.0, w = m_width / 2;
    x[0] = x[1] = m_screenX - h - w;
    x[2] = x[3] = m_screenX + h + w;
    y[0] = y[2] = m_screenY - h - w;
    y[1] = y[3] = m_screenY + h + w;
    
    return RSHandle(x, y);
}

String RSMarker::type() const {
    stringstream ss;
    for(unsigned int i = 0; i < m_markers.size(); i++) {
        switch(m_markers[i]) {
        case Display::Cross:            ss << "cross "; break;
        case Display::X:                ss << "x "; break;
        case Display::Diamond:          ss << "diamond "; break;
        case Display::Circle:           ss << "circle "; break;
        case Display::Square:           ss << "square "; break;
        case Display::Triangle:         ss << "triangle "; break;
        case Display::InvertedTriangle: ss << "inverted triangle "; break;
        case Display::FilledCircle:     ss << "filled circle "; break;
        case Display::FilledSquare:     ss << "filled square "; break;
        case Display::FilledDiamond:    ss << "filled diamond "; break;
        case Display::FilledTriangle:   ss << "filled triangle "; break;
        case Display::FilledInvertedTriangle:
            ss << "filled inverted triangle "; break;
        case Display::CircleAndCross:   ss << "circle cross "; break;
        case Display::CircleAndX:       ss << "circle x "; break;
        case Display::CircleAndDot:     ss << "circle dot "; break;
        }
    }
    ss << "marker";
    return ss.str();
}

vector<String> RSMarker::optionNames() const {
    return vector<String>(1, "Marker Type");
}

vector<RegionShape::OptionType> RSMarker::optionTypes() const {
    return vector<OptionType>(1, STRINGCHOICEARRAY);
}

vector<vector<String> > RSMarker::optionChoices() const {
    vector<Display::Marker> markers = allMarkers();
    vector<String> mstr(markers.size());
    for(unsigned int i = 0; i < mstr.size(); i++)
        mstr[i] = markerToString(markers[i]);
    return vector<vector<String> >(1, mstr);
}

vector<RSOption> RSMarker::optionValues() const {
    vector<String> markers(m_markers.size());
    for(unsigned int i = 0; i < m_markers.size(); i++)
        markers[i] = markerToString(m_markers[i]);
    return vector<RSOption>(1, RSOption(markers));
}

void RSMarker::setOptionValues(const vector<RSOption>& options) {
    if(options.size() >= 1 && options[0].isStringArray() &&
       options[0].asStringArray().size() >= 1) {
        vector<Display::Marker> m(options[0].asStringArray().size());
        for(unsigned int i = 0; i < options[0].asStringArray().size(); i++)
            if(!stringToMarker(options[0].asStringArray()[i], m[i])) return;
        bool changed = m.size() != m_markers.size();
        for(unsigned int i = 0; !changed && i < m.size(); i++)
            if(m[i] != m_markers[i]) changed = true;
        if(changed) {
            m_markers = m;
            refresh(true);
        }
    }
}

bool RSMarker::updateScreenCoordinates(WorldCanvasHolder& wch, String* err) {
    Vector<double> linX(1), linY(1);
    Vector<double> sx(1), sy(1);

    if(m_isWorld) {
        Quantum<Vector<Double> > wx(Vector<Double>(1, m_x), UNIT),
                                 wy(Vector<Double>(1, m_y), UNIT);
        if(!worldToLinear(wx, wy, linX, linY, wch, true, err)) return false;
    } else {
        Vector<double> px(1, m_x), py(1, m_y);
        if(!RSUtils::pixelToLinear(px, py, linX, linY, wch, err)) return false;
    }    
    if(!RSUtils::linearToScreen(linX, linY, sx, sy, wch, err)) return false;
    m_screenX = sx(0); m_screenY = sy(0);
    
    return true;
}

void RSMarker::updateMinMax() {
    // ignore the pixel height, which can be seen in screen max/min
    m_xMin = m_x; m_xMax = m_x; m_yMin = m_y; m_yMax = m_y;
}


// RSTEXT DEFINITIONS //

RSText::RSText(double x, double y, const String& text,
        MDirection::Types worldSystem, double angle): RegionShape(worldSystem),
        m_x(x), m_y(y), m_angle(angle), m_wWidth(0) {
    m_label.setText(text);    
    updateMinMax();
}

RSText::RSText(double x, double y, const String& text, double angle) :
        RegionShape(), m_x(x), m_y(y), m_angle(angle), m_wWidth(0) {
    m_label.setText(text);    
    updateMinMax();
}

RSText::RSText(const RecordInterface& props) : m_x(0), m_y(0), m_angle(0),
        m_wWidth(0) {
    setProperties(props);
}

RSText::~RSText() { }

bool RSText::drawAndUpdateBoundingBox(WorldCanvasHolder& wch, String* err) {
    if(m_label.isEmpty() || !updateScreenCoordinates(wch, err)) {
        if(err && m_label.isEmpty()) {
            if(err->length() > 0) *err += "\n";
            *err += type() + ": cannot draw empty text.";
        }
        return false;
    }
    
    PixelCanvas* pc = wch.worldCanvas()->pixelCanvas();
    if(!m_label.draw(pc, (int)(m_screenX + 0.5), (int)(m_screenY + 0.5),
                     false, err)) return false;

    // update bounding box
    int width = m_label.width(pc);
    if(width > 0) {
        m_screenXMin = m_screenX - width/2;
        m_screenXMax = m_screenX + width/2;
        m_screenYMin = m_screenY - width/2;
        m_screenYMax = m_screenY + width/2;
    }

    if(m_lt) {
        setLinethroughProperties(pc);

        pc->drawLine((int)(m_screenXMin + 0.5),(int)(m_screenYMax + 0.5),
                     (int)(m_screenXMax + 0.5),(int)(m_screenYMin + 0.5));
    }

    // reset line style
    resetLineStyle(pc);
    
    return true;
}

void RSText::getTextCoordinates(double& x, double& y, double& angle,
        String toSystem) const {
    vector<double> v;
    if(toSystem.empty()) v = coordParameterValues();
    else                 v = coordParameterValues(toSystem);
    x     = v[0];
    y     = v[1];
    angle = v[2];
}

vector<double> RSText::coordParameterValues() const {
    vector<double> v(3);
    v[0] = m_x; v[1] = m_y; v[2] = m_angle;
    return v;
}

vector<double> RSText::coordParameterValues(String toSystem) const {
    RSCOORDSTART(3)
    
    // x, y
    RSCOORDPOSCONV(m_x, m_y, 0, 1)
    
    // angle
    RSCOORDANGCONVSA(m_x, m_y, m_angle, m_label.angle(), (m_wWidth/2), 2)
    
    RSCOORDEND
}

vector<String> RSText::coordParameterNames() const {
    static vector<String> v(3);
    v[0] = "center x"; v[1] = "center y"; v[2] = "angle";
    return v;
}

vector<RegionShape::CoordinateParameterType>
RSText::coordParameterTypes() const {
    static vector<CoordinateParameterType> v(3);
    v[0] = POSITION; v[1] = POSITION; v[2] = ANGLE;
    return v;
}

void RSText::setCoordParameters(const vector<double>& vals) {
    if(vals.size() < 3) return;
    bool changed = m_x != vals[0] || m_y != vals[1] || m_angle != vals[2];
    if(changed) {
        m_x = vals[0]; m_y = vals[1]; m_angle = vals[2];
        
        while(m_angle < 0) m_angle += 360;
        while(m_angle >= 360) m_angle -= 360;
        
        updateMinMax();
        refresh(true);
    }
}

void RSText::setCoordParameters(const vector<double>& vals, String system) {
    RSSCOORDSTART(3)
    
    // x, y
    RSSCOORDPOSCONV_(0, 1)
    
    // angle
    double len = m_wWidth / 2;
    if(!m_isWorld && !fromPixel) {
        // convert len from pixels to world
        _RSCOORDSIZEP2W(v[0],v[1],len,true,2,fromSys)
        len = v[2];
    } else if(m_isWorld && fromPixel) {
        // convert len from world to pixel
        _RSCOORDSIZEW2P(v[0],v[1],len,true,2,m_worldSystem)
        len = v[2];
    }
    RSSCOORDANGCONV(vals[0], vals[1], vals[2], len, 2)
    
    RSSCOORDEND
}

void RSText::move(double dx, double dy, String system) {
    if(dx == 0 && dy == 0) return;
    
    if(system.empty() || (m_isWorld && system == MDirection::showType(
       m_worldSystem)) || (!m_isWorld && system == RSUtils::PIXEL)) {
        m_x += dx; m_y += dy;
    } else {
        if(m_lastWCH == NULL) return;
        if(system == RSUtils::PIXEL) { // pixel to world
            Quantum<Vector<double> > wx(Vector<double>(1, m_x), UNIT),
                                     wy(Vector<double>(1, m_y), UNIT);
            Vector<double> px(1), py(1);
            if(!worldToPixel(wx, wy, px, py, *m_lastWCH)) return;
            px(0) += dx; py(0) += dy;
            if(!pixelToWorld(px, py, wx, wy, *m_lastWCH)) return;
            m_x = wx.getValue()(0); m_y = wy.getValue()(0);
        } else if(m_isWorld) {         // world to world
            MDirection::Types sys;
            if(!MDirection::getType(sys, system)) return;
            Quantum<Vector<double> > q1(Vector<double>(2), UNIT),
                                     q2(Vector<double>(2), UNIT);
            q1.getValue()(0) = m_x; q1.getValue()(1) = m_y;
            if(!RSUtils::convertWCS(q1, m_worldSystem, q2, sys)) return;
            q2.getValue()(0) += dx; q2.getValue()(1) += dy;
            if(!RSUtils::convertWCS(q2, sys, q1, m_worldSystem)) return;
            m_x = q1.getValue()(0); m_y = q1.getValue()(1);
        } else {                       // world to pixel
            MDirection::Types sys;
            if(!MDirection::getType(sys, system)) return;
            Quantum<Vector<double> > wx(Vector<double>(1), UNIT),
                                     wy(Vector<double>(1), UNIT);
            Vector<double> px(1, m_x), py(1, m_y);
            if(!RSUtils::pixelToWorld(px, py, wx, wy, *m_lastWCH, sys)) return;
            wx.getValue()(0) += dx; wy.getValue()(0) += dy;
            if(!RSUtils::worldToPixel(wx, wy, px, py, *m_lastWCH, sys)) return;
            m_x = px(0); m_y = py(0);
        }
    }
    
    updateMinMax();
    refresh(true);
}

RSHandle RSText::getHandle() const {
    if(m_lastWCH == NULL) return RSHandle();

    PixelCanvas* pc = m_lastWCH->worldCanvas()->pixelCanvas();
    int width = m_label.width(pc), height = m_label.height(pc);
    if(width <= 0 || height <= 0) return RSHandle();
    
    Vector<double> x(4), y(4);
    RSRectangle::getRectanglePoints(m_screenX, m_screenY, width/ 2, height/ 2,
                                    m_label.angle(), x, y);
    return RSHandle(x, y);
}

bool RSText::updateScreenCoordinates(WorldCanvasHolder& wch, String* err) {
    Vector<double> linX(1), linY(1);
    Vector<double> sx(1), sy(1);
    
    Quantum<Vector<Double> > wx(Vector<Double>(1, m_x), UNIT),
                             wy(Vector<Double>(1, m_y), UNIT);
    Vector<Double> px(1, m_x), py(1, m_y);
    if(m_isWorld) {
        if(!worldToLinear(wx, wy, linX, linY, wch, true, err)) return false;
    } else {
        if(!RSUtils::pixelToLinear(px, py, linX, linY, wch, err)) return false;
    }
    if(!RSUtils::linearToScreen(linX, linY, sx, sy, wch, err)) return false;
    m_screenX = sx(0); m_screenY = sy(0);
    
    // update angle
    int width = m_label.width(wch.worldCanvas()->pixelCanvas());
    if(m_isWorld) {        
        if(width > 0) {
            sx(0) += width / 2;
            vector<int> signX(1, SIGN(m_x)), signY(1, SIGN(m_y));
            if(!screenToWorld(linX, linY, wx, wy, wch, signX, signY, err)) {
                m_label.setAngle(0);
                return false;
            }
            double len = sqrt(pow(wx.getValue()(0) - m_x, 2) +
                              pow(wy.getValue()(0) - m_y, 2));
            m_wWidth = len * 2;
            
            double deg = m_angle * (C::pi / 180);
            wx.getValue()(0) = m_x + (std::cos(deg) * len);
            wy.getValue()(0) = m_y + (std::sin(deg) * len);
            if(!worldToScreen(wx, wy, sx, sy, wch, true, err)) {
                m_label.setAngle(0);
                return true;
            } else if(sx(0) == m_screenX) {
                deg = 90;
            } else {
                deg = std::atan((sy(0) - m_screenY) / (sx(0) - m_screenX));
                deg *= (180 / C::pi);
                if(deg > 0 && (sy(0) < m_screenY)) deg += 180;
                if(deg < 0 && (sx(0) < m_screenX)) deg -= 180;
                
                if(deg < 0) deg = 360 + deg;
            }
            m_label.setAngle(deg);
        } else m_label.setAngle(0);        
    } else {
        m_label.setAngle(m_angle);
        
        if(width > 0) {
            // update m_wWidth for pixel width
            sx(0) += width / 2;
            if(RSUtils::screenToPixel(sx, sy, px, py, wch)) {
                double len = sqrt(pow(px[0] - m_x, 2) + pow(py[0] - m_y, 2));
                m_wWidth = len * 2;
            }
        }
    }

    return true;
}

void RSText::updateMinMax() {
    // ignore the pixel width, which can be seen in screen max/min
    m_xMin = m_x; m_xMax = m_x; m_yMin = m_y; m_yMax = m_y;
}


// RSCOMPOSITE DEFINITIONS //

const String RSComposite::PROPDEPENDENTCHILDREN = "dependentchildren";
const String RSComposite::PROPCHILDREN          = "children";

RSComposite::RSComposite(MDirection::Types worldSys, bool dependent,
        String customName) : RegionShape(worldSys), m_custName(customName),
        m_dependentChildren(dependent), m_centerX(0), m_centerY(0),
        m_syncAngleDepChildren(false), m_applyChangesToChildren(false) {
    updateMinMax();
}

RSComposite::RSComposite(bool dependent, String customName) : RegionShape(),
        m_custName(customName), m_dependentChildren(dependent), m_centerX(0),
        m_centerY(0), m_syncAngleDepChildren(false),
        m_applyChangesToChildren(false) {
    updateMinMax();
}

RSComposite::RSComposite(const RecordInterface& props) :
        m_dependentChildren(false), m_centerX(0), m_centerY(0),
        m_syncAngleDepChildren(false), m_applyChangesToChildren(false) {
    setProperties(props);
}

RSComposite::~RSComposite() {
    for(unsigned int i = 0; i < m_shapes.size(); i++)
        delete m_shapes[i];
}

void RSComposite::addShape(RegionShape* shape) {
    if(shape == NULL) return;
    
    if(addShapeHelper(shape)) addShapeUpkeep();
}

void RSComposite::addShapes(const vector<RegionShape*> shapes) {
    if(shapes.size() == 0) return;
    
    bool added = false;
    for(unsigned int i = 0; i < shapes.size(); i++) {
        if(shapes[i] == NULL) continue;
        added |= addShapeHelper(shapes[i]);
    }
    if(added) addShapeUpkeep();
}

bool RSComposite::childrenAreDependent() const { return m_dependentChildren; }

void RSComposite::setChildrenAreDependent(bool dependent) {
    m_dependentChildren = dependent;
}

vector<RegionShape*> RSComposite::children() { return m_shapes; }

vector<const RegionShape*> RSComposite::children() const {
    vector<const RegionShape*> v(m_shapes.size());
    for(unsigned int i = 0; i < v.size(); i++) v[i] = m_shapes[i];
    return v;
}

void RSComposite::setLineColor(const String& newColor, bool slabel) {
    if(m_dependentChildren || m_applyChangesToChildren) {
        for(unsigned int i = 0; i < m_shapes.size(); i++)
            m_shapes[i]->setLineColor(newColor, slabel);
    }
    RegionShape::setLineColor(newColor, slabel);
}

void RSComposite::setLineWidth(double width) {
    if(m_dependentChildren || m_applyChangesToChildren) {
        for(unsigned int i = 0; i < m_shapes.size(); i++)
            m_shapes[i]->setLineWidth(width);
    }
    RegionShape::setLineWidth(width);
}

void RSComposite::setLineStyle(LineStyle style) {
    if(m_dependentChildren || m_applyChangesToChildren) {
        for(unsigned int i = 0; i < m_shapes.size(); i++)
            m_shapes[i]->setLineStyle(style);
    }
    RegionShape::setLineStyle(style);
}

void RSComposite::setLabel(const RegionShapeLabel& label) {
    if(m_dependentChildren || m_applyChangesToChildren) {
        for(unsigned int i = 0; i < m_shapes.size(); i++) {
            RegionShapeLabel l = m_shapes[i]->label();
            l.setFont(label.font());
            l.setSize(label.size());
            l.setColor(label.color());
            l.setItalic(label.isItalic());
            l.setBold(label.isBold());
            m_shapes[i]->setLabel(l);
        }
    }
    RegionShape::setLabel(label);
}

bool RSComposite::drawAndUpdateBoundingBox(WorldCanvasHolder& wch,String* err){
    if(m_shapes.size() == 0) return true;
    
    for(unsigned int i = 0; i < m_shapes.size(); i++)
        // use draw() instead of drawAndUpdateBoundingBox so that children will
        // also get their m_lastWCH updated.
        if(!m_shapes[i]->draw(wch)) {
            RSUtils::appendUniqueMessage(err, m_shapes[i]->lastDrawingError());
            return false;
        }
    
    // update bounding box
    m_screenXMin = m_shapes[0]->screenXMin();
    m_screenXMax = m_shapes[0]->screenXMax();
    m_screenYMin = m_shapes[0]->screenYMin();
    m_screenYMax = m_shapes[0]->screenYMax();
    for(unsigned int i = 1; i < m_shapes.size(); i++) {
        if(m_shapes[i]->screenXMin() < m_screenXMin)
            m_screenXMin = m_shapes[i]->screenXMin();
        if(m_shapes[i]->screenXMax() > m_screenXMax)
            m_screenXMax = m_shapes[i]->screenXMax();
        if(m_shapes[i]->screenYMin() < m_screenYMin)
            m_screenYMin = m_shapes[i]->screenYMin();
        if(m_shapes[i]->screenYMax() > m_screenYMax)
            m_screenYMax = m_shapes[i]->screenYMax();
    }
    
    PixelCanvas* pc = wch.worldCanvas()->pixelCanvas();
    if(!m_label.isEmpty()) {
        // we want the label to be centered above the top
        m_label.draw(pc, (int)(((m_screenXMax + m_screenXMin) / 2) + 0.5),
                         (int)(m_screenYMax + 0.5), true);
    }
    
    if(m_lt) {
        setLinethroughProperties(pc);

        pc->drawLine((int)(m_screenXMin + 0.5), (int)(m_screenYMax + 0.5),
                     (int)(m_screenXMax + 0.5), (int)(m_screenYMin + 0.5));
    }
    
    if(!m_label.isEmpty()) m_screenYMax += m_label.height(pc);
    
    // reset line style
    resetLineStyle(pc);
    
    updateMinMax();
    
    return true;
}

String RSComposite::type() const {
    if(!m_custName.empty())       return m_custName;
    else if(m_shapes.size() == 0) return "composite";
    
    String type;
    vector<String> seenTypes;
    unsigned int j = 0;
    for(unsigned int i = 0; i < m_shapes.size(); i++) {
        type = m_shapes[i]->oneWordType();
        for(j = 0; j < seenTypes.size(); j++)
            if(seenTypes[j] == type) break;
        if(j == seenTypes.size()) seenTypes.push_back(type);
    }
    stringstream ss;
    ss << "composite [";
    for(unsigned int i = 0; i < seenTypes.size(); i++) {
        ss << seenTypes[i];
        if(i < seenTypes.size() - 1) ss << " ";
    }
    ss << "]";
    return ss.str();
}

#define RSCOMPANGSTART                                                        \
    if(toSystem.empty()) {                                                    \
        if(m_isWorld) toSystem = MDirection::showType(m_worldSystem);         \
        else          toSystem = RSUtils::PIXEL;                              \
    }                                                                         \
    bool toPixel = toSystem == RSUtils::PIXEL;                                \
    if(toPixel || m_lastWCH == NULL) return 0;                                \
    MDirection::Types toSys;                                                  \
    if(!toPixel && !MDirection::getType(toSys, toSystem)) return 0;           \
                                                                              \
    const CoordinateSystem& cs = m_lastWCH->worldCanvas()->coordinateSystem();\
    MDirection::Types wcSys = RSUtils::worldSystem(cs);                       \
                                                                              \
    bool isValid = true;                                                      \
    const Vector<String>& units = m_lastWCH->worldAxisUnits();                \
    vector<double> v(3);                                                      \
    Quantum<Vector<double> > q(Vector<double>(2), UNIT);                      \
    Quantum<Vector<double> > qx(Vector<double>(1), UNIT),                     \
                             qy(Vector<double>(1), UNIT);                     \
    Quantum<double> t(0.0, UNIT), tx(0.0, units(0)), ty(0.0, units(1));       \
    Vector<double> p(2), w(2), px(1), py(1);                                  \
    MDirection m;

#define RSCOMPANGCONV                                                         \
    if(m_isWorld) { /* world to world */                                      \
        RSCOORDPOSW2P(m_centerX, m_centerY, 0, 1)                             \
        double x = v[0]; double y = v[1];                                     \
        RSCOORDPOSW2P(m_xMax, m_centerY, 0, 1)                                \
        double len = sqrt(pow(v[0] - x, 2) + pow(v[1] - y, 2));               \
        RSCOORDANGP2W(x,y,0,len,2)                                            \
    } else { /* pixel to world */                                             \
        double len = m_xMax - m_centerX;                                      \
        RSCOORDANGP2W(m_centerX, m_centerY, 0, len, 2)                        \
    }
    

#define RSCOMPANGEND                                                          \
    if(!isValid) return 0;                                                    \
    else         return v[2];

double RSComposite::getAngle(String toSystem) const {
    RSCOMPANGSTART
    
    RSCOMPANGCONV
    
    RSCOMPANGEND
}

void RSComposite::getCompositeCoordinates(double& centerX, double& centerY,
        String toSystem) const {
    vector<double> v;
    if(toSystem.empty()) v = coordParameterValues();
    else                 v = coordParameterValues(toSystem);
    centerX = v[0];
    centerY = v[1];
}

vector<double> RSComposite::coordParameterValues() const {
    vector<double> v(2);
    v[0] = m_centerX; v[1] = m_centerY;
    return v;
}

vector<double> RSComposite::coordParameterValues(String toSystem) const {
    RSCOORDSTART(2)
    
    // center x, center y
    RSCOORDPOSCONV(m_centerX, m_centerY, 0, 1)
    
    RSCOORDEND
}

vector<String> RSComposite::coordParameterNames() const {
    static vector<String> v(2);
    v[0] = "center x"; v[1] = "center y";
    return v;
}

vector<RegionShape::CoordinateParameterType>
RSComposite::coordParameterTypes() const {
    static vector<CoordinateParameterType> v(2);
    v[0] = POSITION; v[1] = POSITION;
    return v;
}

void RSComposite::setCoordParameters(const vector<double>& vals) {
    if(vals.size() < 2 || m_shapes.size() < 1) return;
    double dx = vals[0] - m_centerX, dy = vals[1] - m_centerY;
    bool changed = dx != 0 || dy != 0;
    if(changed) move(dx, dy);
}

void RSComposite::setCoordParameters(const vector<double>& vals,String system){
    RSSCOORDSTART(2)
    
    // center x, center y
    RSSCOORDPOSCONV_(0, 1)
    
    RSSCOORDEND
}

void RSComposite::move(double dx, double dy, String system) {
    if(dx == 0 && dy == 0) return;
    
    if(system.empty()) {
        if(m_isWorld) system = MDirection::showType(m_worldSystem);
        else          system = RSUtils::PIXEL;
    }
    
    for(unsigned int i = 0; i < m_shapes.size(); i++)
        m_shapes[i]->move(dx, dy, system);
        
    updateMinMax();
    refresh(true);
}

RSHandle RSComposite::getHandle() const {
    if(m_lastWCH == NULL || m_shapes.size() == 0) return RSHandle();

    RSHandle handle = m_shapes[0]->getHandle();
    vector<double> x(4), y(4);
    if(!handle.isValid() || !handle.getVertices(x, y)) return RSHandle();
    double xmin = x[0], xmax = x[0], ymin = y[0], ymax = y[0];
    for(unsigned int i = 1; i < x.size() && i < y.size(); i++) {
        if(x[i] < xmin) xmin = x[i];
        if(x[i] > xmax) xmax = x[i];
        if(y[i] < ymin) ymin = y[i];
        if(y[i] > ymax) ymax = y[i];
    }
    
    for(unsigned int i = 1; i < m_shapes.size(); i++) {
        handle = m_shapes[i]->getHandle();
        if(!handle.isValid() || !handle.getVertices(x, y)) return RSHandle();
        for(unsigned int j = 0; j < x.size() && j < y.size(); j++) {
            if(x[j] < xmin) xmin = x[j];
            if(x[j] > xmax) xmax = x[j];
            if(y[j] < ymin) ymin = y[j];
            if(y[j] > ymax) ymax = y[j];
        }
    }
    
    x[0] = x[1] = xmin;
    x[2] = x[3] = xmax;
    y[0] = y[2] = ymin;
    y[1] = y[3] = ymax;
    return RSHandle(x, y);
}

vector<String> RSComposite::optionNames() const {
    vector<String> v(1, "Custom Name");
    if(!m_dependentChildren) v.push_back("Apply Property Changes to Children");
    return v;
}

vector<RegionShape::OptionType> RSComposite::optionTypes() const {
    vector<OptionType> v(1, STRING);
    if(!m_dependentChildren) v.push_back(BOOL);
    return v;
}

vector<RSOption> RSComposite::optionValues() const {
    vector<RSOption> v(1, RSOption(m_custName));
    if(!m_dependentChildren) v.push_back(RSOption(m_applyChangesToChildren));
    return v;
}

void RSComposite::setOptionValues(const vector<RSOption>& options) {
    if(options.size() >= 1 && options[0].isString() && options[0].asString() !=
       m_custName) { m_custName = options[0].asString(); }
    if(options.size() >= 2 && options[1].isBool() && options[1].asBool() !=
       m_applyChangesToChildren && !m_dependentChildren) {
        m_applyChangesToChildren = options[1].asBool();
    }
}

void RSComposite::getShapeSpecificProperties(Record& properties) const {
    properties.define(PROPTYPE, PROPTYPE_COMPOSITE);
    properties.define(PROPDEPENDENTCHILDREN, m_dependentChildren);
    Record children;
    for(unsigned int i = 0; i < m_shapes.size(); i++) {
        String s = String::toString(i);
        children.defineRecord(s, m_shapes[i]->getProperties());
    }
    if(children.nfields() > 0) properties.defineRecord(PROPCHILDREN, children);
    
    // remove PROPCOORDINATES
    properties.removeField(PROPCOORDINATES);
}

void RSComposite::setShapeSpecificProperties(const RecordInterface& p) {
    if(p.isDefined(PROPDEPENDENTCHILDREN) &&
       p.dataType(PROPDEPENDENTCHILDREN) == TpBool)
        m_dependentChildren = p.asBool(PROPDEPENDENTCHILDREN);
    
    if(p.isDefined(PROPCHILDREN) && p.dataType(PROPCHILDREN) == TpRecord) {
        const RecordInterface& r = p.asRecord(PROPCHILDREN);
        RegionShape* s;
        vector<RegionShape*> children;
        String str;
        for(unsigned int i = 0; i < r.nfields(); i++) {
            if(r.dataType(i) == TpRecord) {
                s = RegionShape::shapeFromRecord(r.asRecord(i), str);
                if(s != NULL) children.push_back(s);
            }
        }
        if(children.size() > 0) addShapes(children);
    }
}

void RSComposite::updateMinMax() {
    if(m_shapes.size() == 0) {
        m_xMin = 0; m_xMax = 0; m_yMin = 0; m_yMax = 0;
        m_centerX = 0; m_centerY = 0;
        return;
    }
    
    // min/max
    convertMinMax(m_xMin, m_xMax, m_yMin, m_yMax, 0);
    
    if(m_shapes.size() > 1) {
        double xmin, xmax, ymin, ymax;
        
        for(unsigned int i = 1; i < m_shapes.size(); i++) {
            convertMinMax(xmin, xmax, ymin, ymax, i);
            
            if(xmin < m_xMin) m_xMin = xmin;
            if(xmax > m_xMax) m_xMax = xmax;
            if(ymin < m_yMin) m_yMin = ymin;
            if(ymax > m_yMax) m_yMax = ymax;
        }
    }
    
    // center
    m_centerX = (m_xMin + m_xMax) / 2; m_centerY = (m_yMin + m_yMax) / 2;
}

bool RSComposite::addShapeHelper(RegionShape* shape) {
    if(m_dependentChildren || m_applyChangesToChildren) {
        shape->setText("");
        shape->setLineWidth(m_width);
        shape->setLineColor(m_color);
        shape->setLineStyle(m_style);
        shape->setLinethrough(false);
    }
    
    unsigned int i = 0;
    for(; i < m_shapes.size(); i++)
        if(m_shapes[i] == shape) break;
    if(i == m_shapes.size()) {
        m_shapes.push_back(shape);
        return true;
    } else return false;
}

void RSComposite::addShapeUpkeep() {
    if(m_dependentChildren) {            
        // check for synched angle case
        m_syncAngleDepChildren = true;
        RSEllipse* e; RSCircle* c; RSRectangle* r; RSVector* v; RSText* t;
        double temp, angle; int temp2;
        vector<double> angles;
        for(unsigned int i = 0; m_syncAngleDepChildren &&
                              i < m_shapes.size(); i++) {
            e = dynamic_cast<RSEllipse*>(m_shapes[i]);
            c = dynamic_cast<RSCircle*>(m_shapes[i]);
            r = dynamic_cast<RSRectangle*>(m_shapes[i]);
            v = dynamic_cast<RSVector*>(m_shapes[i]);
            t = dynamic_cast<RSText*>(m_shapes[i]);
            if(e == NULL && r == NULL && v == NULL && t == NULL) {
                m_syncAngleDepChildren = false;
                break;
            }
            if(e != NULL && c == NULL) {
                e->getEllipseCoordinates(temp, temp, temp, temp, angle);
                angles.push_back(angle);
            } else if(r != NULL) {
                r->getRectangleCoordinates(temp, temp, temp, temp, angle);
                angles.push_back(angle);
            } else if(v != NULL) {
                v->getVectorCoordinates(temp, temp, angle, temp, temp2);
                angles.push_back(angle);
            } else if(t != NULL) {
                t->getTextCoordinates(temp, temp, angle);
                angles.push_back(angle);
            }
        }
        
        if(m_syncAngleDepChildren) {
            bool allSame = true;
            for(unsigned int i = 1; allSame && i < angles.size(); i++)
                if(angles[i] != angles[i - 1]) allSame = false;
            if(allSame && angles.size() >= 1) m_syncAngle = angles[0];
            else m_syncAngleDepChildren = false;
        }
    }

    updateMinMax();
}

void RSComposite::convertMinMax(double& xmin, double& xmax, double& ymin,
        double& ymax, unsigned int index) const {
    xmin = m_shapes[index]->xMin();
    xmax = m_shapes[index]->xMax();
    ymin = m_shapes[index]->yMin();
    ymax = m_shapes[index]->yMax();
    
    if(m_lastWCH == NULL) return;
    
    if(m_shapes[index]->isWorld() && m_isWorld &&
       m_shapes[index]->worldSystem() != m_worldSystem) {
        static Quantum<Vector<double> > q1(Vector<double>(2), UNIT),
                                        q2(Vector<double>(2), UNIT);
        q1.getValue()(0) = xmin; q1.getValue()(1) = ymin;
        RSUtils::convertWCS(q1, m_shapes[index]->worldSystem(),
                            q2, m_worldSystem);
        xmin = q2.getValue()(0); ymin = q2.getValue()(1);
        
        q1.getValue()(0) = xmax; q1.getValue()(1) = ymax;
        RSUtils::convertWCS(q1, m_shapes[index]->worldSystem(),
                            q2, m_worldSystem);
        xmax = q2.getValue()(0); ymax = q2.getValue()(1);
        
    } else if(m_shapes[index]->isWorld() && !m_isWorld) {
        static Quantum<Vector<double> > wx(Vector<double>(2), UNIT),
                                        wy(Vector<double>(2), UNIT);
        Vector<double> px(2), py(2);
        wx.getValue()(0) = xmin; wx.getValue()(1) = xmax;
        wy.getValue()(0) = ymin; wy.getValue()(1) = ymax;
        
        RSUtils::worldToPixel(wx, wy, px, py, *m_lastWCH,
                              m_shapes[index]->worldSystem());
        xmin = px(0); xmax = px(1);
        ymin = py(0); ymax = py(1);
        
    } else if(!m_shapes[index]->isWorld() && m_isWorld) {
        static Quantum<Vector<double> > wx(Vector<double>(2), UNIT),
                                        wy(Vector<double>(2), UNIT);
        Vector<double> px(2), py(2);
        px(0) = xmin; px(1) = xmax;
        py(0) = ymin; py(1) = ymax;
        
        RSUtils::pixelToWorld(px, py, wx, wy, *m_lastWCH, m_worldSystem);
        xmin = wx.getValue()(0); xmax = wx.getValue()(1);
        ymin = wy.getValue()(0); ymax = wy.getValue()(1);
    }
}

}
