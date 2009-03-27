//# PlotOptions.cc: Customization classes for plotter objects.
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
#include <graphics/GenericPlotter/PlotOptions.h>

namespace casa {

///////////////////////////
// PLOTCOLOR DEFINITIONS //
///////////////////////////

PlotColor::PlotColor() { }

PlotColor::~PlotColor() { }

PlotColor& PlotColor::operator=(const PlotColor& rh) {
    setAsHexadecimal(rh.asHexadecimal());
    setAlpha(rh.alpha());
    return *this;
}

bool PlotColor::operator==(const PlotColor& rh) const {
    return asHexadecimal() == rh.asHexadecimal() && alpha() == rh.alpha(); }
bool PlotColor::operator!=(const PlotColor& rh) const{ return !(*this == rh); }

void PlotColor::setAsHexadecimal(const String& hex) {
    setAsHexadecimalOrName(hex); }
void PlotColor::setAsName(const String& name) { setAsHexadecimalOrName(name); }


//////////////////////////
// PLOTFONT DEFINITIONS //
//////////////////////////

PlotFont::PlotFont() { }

PlotFont::~PlotFont() { }

void PlotFont::setColor(const PlotColorPtr c) { if(!c.null()) setColor(*c); }
void PlotFont::setColor(const String& col) {
    PlotColorPtr c = color();
    c->setAsHexadecimalOrName(col);
    setColor(*c);
}

PlotFont& PlotFont::operator=(const PlotFont& rh) {
    setPointSize(rh.pointSize());
    setPixelSize(rh.pixelSize());
    setFontFamily(rh.fontFamily());
    setColor(rh.color());
    setItalics(rh.italics());
    setBold(rh.bold());
    setUnderline(rh.underline());
    return *this;
}

bool PlotFont::operator==(const PlotFont& rh) const {
    return pointSize() == rh.pointSize() &&
           pixelSize() == rh.pixelSize() &&
           fontFamily() == rh.fontFamily() &&
           *color() == *rh.color() && italics() == rh.italics() &&
           bold() == rh.bold() && underline() == rh.underline();
}
bool PlotFont::operator!=(const PlotFont& rh) const { return !(*this == rh); }


//////////////////////////////
// PLOTAREAFILL DEFINITIONS //
//////////////////////////////

PlotAreaFill::PlotAreaFill() { }

PlotAreaFill::~PlotAreaFill() { }

void PlotAreaFill::setColor(const PlotColorPtr c){ if(!c.null()) setColor(*c);}
void PlotAreaFill::setColor(const String& co) {
    PlotColorPtr c = color();
    c->setAsHexadecimalOrName(co);
    setColor(*c);
}

PlotAreaFill& PlotAreaFill::operator=(const PlotAreaFill& rh) {
    setColor(rh.color());
    setPattern(rh.pattern());
    return *this;
}

bool PlotAreaFill::operator==(const PlotAreaFill& rh) const {
    if(pattern() == rh.pattern() && pattern() == NOFILL) return true;
    return *color() == *rh.color() && pattern() == rh.pattern(); }
bool PlotAreaFill::operator!=(const PlotAreaFill& rh) const {
    return !(*this == rh); }


//////////////////////////
// PLOTLINE DEFINITIONS //
//////////////////////////

PlotLine::PlotLine() { }

PlotLine::~PlotLine() { }

void PlotLine::setColor(const PlotColorPtr c) { if(!c.null()) setColor(*c); }
void PlotLine::setColor(const String& col) {
    PlotColorPtr c = color();
    c->setAsHexadecimalOrName(col);
    setColor(*c);
}

PlotLine& PlotLine::operator=(const PlotLine& rh) {
    setWidth(rh.width());
    setStyle(rh.style());
    setColor(rh.color());
    return *this;
}

bool PlotLine::operator==(const PlotLine& rh) const {
    if(style() == rh.style() == style() == NOLINE) return true;
    if(width() == rh.width() && width() == 0) return true;
    return width() == rh.width() && style() == rh.style() &&
           *color() == *rh.color();
}

bool PlotLine::operator!=(const PlotLine& rh) const { return !(*this == rh); }


////////////////////////////
// PLOTSYMBOL DEFINITIONS //
////////////////////////////

PlotSymbol::PlotSymbol() { }

PlotSymbol::~PlotSymbol() { }

void PlotSymbol::setSize(pair<double, double> size) {
    setSize(size.first, size.second); }

bool PlotSymbol::isCharacter() const { return symbol() == CHARACTER; }

void PlotSymbol::setLine(const PlotLinePtr l) {
    if(!l.null()) setLine(*l);
    else          setLine("black", PlotLine::NOLINE);
}

void PlotSymbol::setLine(const String& color, PlotLine::Style style,
        double width) {
    PlotLinePtr l = line();
    l->setColor(color);
    l->setStyle(style);
    l->setWidth(width);
    setLine(*l);
}

void PlotSymbol::setAreaFill(const PlotAreaFillPtr a) {
    if(!a.null()) setAreaFill(*a);
    else          setAreaFill("black", PlotAreaFill::NOFILL);
}

void PlotSymbol::setAreaFill(const String& color,
        PlotAreaFill::Pattern pattern) {
    PlotAreaFillPtr a = areaFill();
    a->setColor(color);
    a->setPattern(pattern);
    setAreaFill(*a);
}

void PlotSymbol::setColor(const PlotColor& color) {
    PlotLinePtr l = line();
    l->setColor(color);
    setLine(*l);
    
    PlotAreaFillPtr a = areaFill();
    a->setColor(color);
    setAreaFill(*a);
}

void PlotSymbol::setColor(const PlotColorPtr color) {
    if(!color.null()) setColor(*color); }
void PlotSymbol::setColor(const String& color) {
    PlotLinePtr l = line();
    l->setColor(color);
    setLine(*l);
    
    PlotAreaFillPtr a = areaFill();
    a->setColor(color);
    setAreaFill(*a);
}

PlotSymbol& PlotSymbol::operator=(const PlotSymbol& rh) {
    setSize(rh.size());
    setHeightIsPixel(rh.heightIsPixel());
    setUSymbol(rh.symbolUChar());
    setSymbol(rh.symbol());
    setLine(rh.line());
    setAreaFill(rh.areaFill());
    return *this;
}

bool PlotSymbol::operator==(const PlotSymbol& rh) const {
    if(symbol() == rh.symbol() && symbol() == NOSYMBOL) return true;
    if(size()== rh.size() && size()== pair<double, double>(0, 0)) return true;
    return size() == rh.size() && symbol() == rh.symbol() &&
           (isCharacter() ? symbolChar() == rh.symbolChar() : true) &&
           (isCharacter() ? heightIsPixel() == rh.heightIsPixel() : true) &&
           *line() == *rh.line() && *areaFill() == *rh.areaFill();
}

bool PlotSymbol::operator!=(const PlotSymbol& rh) const {
    return !(*this == rh); }


////////////////////////////////
// PLOTCOORDINATE DEFINITIONS //
////////////////////////////////

PlotCoordinate::PlotCoordinate() : m_system(WORLD), m_x(0), m_y(0) { }

PlotCoordinate::PlotCoordinate(double dx, double dy, System s): m_system(s),
        m_x(dx), m_y(dy) { }

PlotCoordinate::PlotCoordinate(const PlotCoordinate& c) { operator=(c); }

PlotCoordinate::~PlotCoordinate() { }

PlotCoordinate::System PlotCoordinate::system() const { return m_system; }
double PlotCoordinate::x() const { return m_x; }
double PlotCoordinate::y() const { return m_y; }

PlotCoordinate& PlotCoordinate::operator=(const PlotCoordinate& rh) {
    m_system = rh.system();
    m_x = rh.x();
    m_y = rh.y();
    return *this;
}

bool PlotCoordinate::operator==(const PlotCoordinate& rh) const {
    return m_system == rh.system() && m_x == rh.x() && m_y == rh.y(); }

bool PlotCoordinate::operator!=(const PlotCoordinate& rh) const {
    return !(*this == rh); }


////////////////////////////
// PLOTREGION DEFINITIONS //
////////////////////////////

PlotRegion::PlotRegion() { }

PlotRegion::PlotRegion(const PlotCoordinate& upperLeft,
        const PlotCoordinate& lowerRight) : m_upperLeft(upperLeft),
        m_lowerRight(lowerRight) { }

PlotRegion::PlotRegion(const PlotRegion& copy) : m_upperLeft(copy.upperLeft()),
        m_lowerRight(copy.lowerRight()) { }

PlotRegion::~PlotRegion() { }

const PlotCoordinate& PlotRegion::upperLeft() const { return m_upperLeft; }

const PlotCoordinate& PlotRegion::lowerRight() const { return m_lowerRight; }

double PlotRegion::top() const { return m_upperLeft.y(); }

double PlotRegion::bottom() const { return m_lowerRight.y(); }

double PlotRegion::left() const { return m_upperLeft.x(); }

double PlotRegion::right() const { return m_lowerRight.x(); }

bool PlotRegion::isValid() const {
    return m_upperLeft.system() == m_lowerRight.system() &&
           m_upperLeft.x() < m_lowerRight.x() &&
           m_upperLeft.y() > m_lowerRight.y();
}


///////////////////////////////
// PLOTAXESSTACK DEFINITIONS //
///////////////////////////////

PlotAxesStack::PlotAxesStack(int lengthLimit) : m_lengthLimit(lengthLimit),
        m_stackIndex(0) {
    if(m_lengthLimit <= 1) m_lengthLimit = -1;
}

PlotAxesStack::~PlotAxesStack() { }


int PlotAxesStack::lengthLimit() const { return m_lengthLimit; }
void PlotAxesStack::setLengthLimit(int lengthLimit) {
    if(lengthLimit <= 1) lengthLimit = -1;
    if(m_lengthLimit != lengthLimit) {
        m_lengthLimit = lengthLimit;
        if(lengthLimit > 0 && m_stack.size() > (unsigned int)lengthLimit)
            shrinkStacks((unsigned int)lengthLimit);
    }
}

bool PlotAxesStack::isValid() const { return m_stack.size() > 0; }
unsigned int PlotAxesStack::stackIndex() const { return m_stackIndex; }
unsigned int PlotAxesStack::size() const { return m_stack.size(); }

vector<PlotRegion> PlotAxesStack::stack() const { return m_stack; }
vector<pair<PlotAxis, PlotAxis> > PlotAxesStack::stackAxes() const {
    return m_axes; }

void PlotAxesStack::setBase(const PlotRegion& base, PlotAxis xAxis,
        PlotAxis yAxis) {
    m_stack.resize(1, base);
    m_axes.resize(1, pair<PlotAxis, PlotAxis>(xAxis, yAxis));
    m_stackIndex = 0;
}

void PlotAxesStack::addRegion(const PlotRegion& region, PlotAxis xAxis,
        PlotAxis yAxis) {
    if(m_stack.size() == 0) setBase(region, xAxis, yAxis);
    else {
        unsigned int newSize = m_stackIndex + 2;
        
        if(m_lengthLimit > 0 && newSize > (unsigned int)m_lengthLimit) {
            // resize by discarding old values
            shrinkStacks((unsigned int)m_lengthLimit);
            
            // Check for special indexing case.  Otherwise we override the
            // current stack index member.
            if(m_stackIndex < (unsigned int)(m_lengthLimit - 1))
                m_stackIndex++;
            
        } else if(m_stack.size() != newSize) {
            m_stack.resize(newSize);
            m_axes.resize(newSize);
            m_stackIndex++;
            
        } else m_stackIndex++;
        
        m_stack[m_stackIndex] = region;
        m_axes[m_stackIndex] = pair<PlotAxis, PlotAxis>(xAxis, yAxis);
    }
}

void PlotAxesStack::clearStack(bool keepBase) {
    if(m_stack.size() == 0) return;
    m_stack.resize(keepBase ? 1 : 0);
    m_axes.resize(keepBase ? 1 : 0);
    m_stackIndex = 0;
}

PlotRegion PlotAxesStack::currentRegion() const {
    if(isValid()) return m_stack[m_stackIndex];
    else          return PlotRegion();
}

PlotAxis PlotAxesStack::currentXAxis() const {
    if(isValid()) return m_axes[m_stackIndex].first;
    else          return X_BOTTOM;
}

PlotAxis PlotAxesStack::currentYAxis() const {
    if(isValid()) return m_axes[m_stackIndex].second;
    else          return Y_LEFT;
}

void PlotAxesStack::move(int delta) {
    if(!isValid()) return;
    if(delta == 0) delta = -m_stackIndex;
    int temp = m_stackIndex + delta;
    if(temp < 0) temp = 0;
    else if(temp >= (int)m_stack.size()) temp = m_stack.size() - 1;
    m_stackIndex = (unsigned int)temp;
}

PlotRegion PlotAxesStack::moveAndReturn(int delta) {
    move(delta);
    return currentRegion();
}


void PlotAxesStack::shrinkStacks(unsigned int n) {
    if(n == 0 || m_stack.size() >= n) return;
    
    // Shorten the stacks.
    unsigned int diff = m_stack.size() - n;
    vector<PlotRegion> stack(n);
    vector<pair<PlotAxis, PlotAxis> > axes(n);
    
    // Copy over bases.
    stack[0] = m_stack[0];
    axes[0] = m_axes[0];

    if(m_stackIndex > 0 && m_stackIndex <= diff) {
        // Current index is in the part that is to be discarded, so
        // keep that member and copy over the rest, discarding the
        // oldest.
        stack[1] = m_stack[m_stackIndex];
        axes[1] = m_axes[m_stackIndex];
        
        for(unsigned int i = 2; i < n; i++) {
            stack[i] = m_stack[i + diff];
            axes[i] = m_axes[i + diff];
        }
        
        // Update stack index.
        m_stackIndex = 1;
        
    } else {
        // Copy over the rest, discarding the oldest.
        for(unsigned int i = 1; i < n; i++) {
            stack[i] = m_stack[i + diff];
            axes[i] = m_axes[i + diff];
        }
        
        // Update the stack index if not at base.
        if(m_stackIndex > 0) m_stackIndex -= diff;
    }
    
    m_stack = stack;
    m_axes = axes;
}


//////////////////////////////////
// PLOTEXPORTFORMAT DEFINITIONS //
//////////////////////////////////

PlotExportFormat::PlotExportFormat(Type t, const String& file) : type(t),
        location(file), resolution(SCREEN), dpi(-1), width(-1), height(-1) { }

PlotExportFormat::~PlotExportFormat() { }

String PlotExportFormat::exportFormat(Type t) {
    switch(t) {
    case PNG: return "PNG";
    case JPG: return "JPG";
    case PS: return "PS";
    case PDF: return "PDF";
    default: return "";
    }
}

PlotExportFormat::Type PlotExportFormat::exportFormat(String t, bool* ok) {
    t.downcase();
    
    if(ok != NULL) *ok = true;
    if(t == "png")                     return PNG;
    else if(t == "jpg" || t == "jpeg") return JPG;
    else if(t == "ps")                 return PS;
    else if(t == "pdf")                return PDF;
    
    // error
    if(ok != NULL) *ok = false;
    return PNG;
}

String PlotExportFormat::extensionFor(Type t) {
    switch(t) {
    case PNG: return "png";
    case JPG: return "jpg";
    case PS: return "ps";
    case PDF: return "pdf";
    default: return "";
    }
}

PlotExportFormat::Type PlotExportFormat::typeForExtension(String f, bool* ok) {
    f.downcase();
    unsigned int n = f.size();
    if(ok != NULL) *ok = true;
    if(f.matches("png", n - 3))      return PNG;
    else if(f.matches("jpg", n - 3) || f.matches("jpeg", n - 4))
                                     return JPG;
    else if(f.matches("ps", n - 2))  return PS;
    else if(f.matches("pdf", n - 3)) return PDF;
    
    // error
    if(ok != NULL) *ok = false;
    return PNG;
}

vector<PlotExportFormat::Type> PlotExportFormat::supportedFormats() {
    vector<Type> v(4);
    v[0] = PNG;
    v[1] = JPG;
    v[2] = PS;
    v[3] = PDF;
    return v;
}    
vector<String> PlotExportFormat::supportedFormatStrings() {
    vector<Type> v = supportedFormats();
    vector<String> s(v.size());
    for(unsigned int i = 0; i < s.size(); i++) s[i] = exportFormat(v[i]);
    return s;
}

vector<PlotExportFormat::Type> PlotExportFormat::supportedImageFormats() {
    vector<Type> v(2);
    v[0] = PNG;
    v[1] = JPG;
    return v;
}

vector<String> PlotExportFormat::supportedImageFormatStrings() {
    vector<Type> v = supportedImageFormats();
    vector<String> s(v.size());
    for(unsigned int i = 0; i < s.size(); i++) s[i] = exportFormat(v[i]);
    return s;
}

}
