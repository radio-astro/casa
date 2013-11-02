//# QPOptions.cc: Qwt implementation of generic PlotOption classes.
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

#include <casaqt/QwtPlotter/QPOptions.h>

#include <QPainter>

namespace casa {


/////////////////////////
// QPCOLOR DEFINITIONS //
/////////////////////////

QPColor::QPColor() : m_color() { }

QPColor::QPColor(const String& color) : m_color() {
    setAsHexadecimalOrName(color); }

QPColor::QPColor(const QColor& c) : m_color(c) { }

QPColor::QPColor(const PlotColor& color) : m_color() {
    operator=(color);
}

QPColor::QPColor(const PlotColorPtr color) : m_color() {
    if(!color.null()) operator=(*color);
}

QPColor::QPColor(const QPColor& color) : m_color(color.asQColor()) { }

QPColor::~QPColor() { }


String QPColor::asHexadecimal() const {
    String hex = m_color.name().toStdString();
    if(hex.size() > 0 && hex[0] == '#') hex.erase(0, 1);
    return hex;
}

String QPColor::asName() const {
    // there has to be a better way to do this?
    QStringList colors = QColor::colorNames();
    QColor color;
    QString name = m_color.name();
    for(int i = 0; i < colors.size(); i++) {
        color.setNamedColor(colors[i]);
        if(color.name() == name) return colors[i].toStdString();
    }
    return "";
}

void QPColor::setAsHexadecimalOrName(const String& str) {
    QString name = m_color.name();
    
    // three possibilities: a name (i.e. "black"), a hex value (i.e. "000000"),
    // or a #'ed hex value (i.e. "#000000").  QColor::setNamedColor can
    // natively handle the first and third, so just check for second.
    
    String hex = str;
    if(hex.size() == 6 && ((hex[0] >= '0' && hex[0] <= '9') ||
       (hex[0] >= 'A' && hex[0] <= 'F') || (hex[0] >= 'a' && hex[0] <= 'f'))) {
        bool isHex = true;
        // hopefully no color names also fit this pattern..
        for(unsigned int i = 1; i < hex.size(); i++) {
            if(!((hex[i] >= '0' && hex[i] <= '9') ||
                 (hex[i] >= 'A' && hex[i] <= 'F') ||
                 (hex[i] >= 'a' && hex[i] <= 'f'))) {
                isHex = false;
                break;
            }
        }
        if(isHex) hex = "#" + hex;
    }
    
    m_color.setNamedColor(hex.c_str());
    if(!m_color.isValid()) m_color.setNamedColor(name);
}

double QPColor::alpha() const {
    // QColor is 0 to 255
    return m_color.alpha() / 255.0;
}
    
void QPColor::setAlpha(double a) {
    // QColor is 0 to 255
    if(a < 0) a = 0;
    if(a > 1) a = 1;
    m_color.setAlpha((int)(a * 255 + 0.5));
}

const QColor& QPColor::asQColor() const { return m_color; }
void QPColor::setAsQColor(const QColor& c) { m_color = c; }


////////////////////////
// QPFONT DEFINITIONS //
////////////////////////

QPFont::QPFont() { }

QPFont::QPFont(const QFont& f, const QColor& c) : m_font(f), m_color(c) { }

QPFont::QPFont(const PlotFont& font) {
    operator=(font);
}

QPFont::QPFont(const PlotFontPtr font) {
    if(!font.null()) operator=(*font);
}

QPFont::QPFont(const QPFont& font) : m_font(font.asQFont()),
        m_color(font.color()) { }

QPFont::~QPFont() { }


double QPFont::pointSize() const { return m_font.pointSizeF(); }
void QPFont::setPointSize(double size) {
    if(size > 0) m_font.setPointSizeF(size); }

int QPFont::pixelSize() const { return m_font.pixelSize(); }
void QPFont::setPixelSize(int size) { if(size > 0) m_font.setPixelSize(size); }

String QPFont::fontFamily() const { return m_font.family().toStdString(); }
void QPFont::setFontFamily(const String& f) { m_font.setFamily(f.c_str()); }

PlotColorPtr QPFont::color() const { return new QPColor(m_color); }
void QPFont::setColor(const PlotColor& c) { m_color = QPColor(c); }

bool QPFont::italics() const { return m_font.italic(); }
void QPFont::setItalics(bool i) { m_font.setItalic(i); }

bool QPFont::bold() const { return m_font.bold(); }
void QPFont::setBold(bool b) { m_font.setBold(b); }

bool QPFont::underline() const { return m_font.underline(); }
void QPFont::setUnderline(bool u) { m_font.setUnderline(u); }

const QFont& QPFont::asQFont() const { return m_font; }
void QPFont::setAsQFont(const QFont& f) { m_font = f; }

const QColor& QPFont::asQColor() const { return m_color.asQColor(); }
void QPFont::setAsQColor(const QColor& color) { m_color.setAsQColor(color); }


////////////////////////
// QPLINE DEFINITIONS //
////////////////////////

QPLine::QPLine() : m_pen(), m_color(m_pen.color()) { }

QPLine::QPLine(const QPen& p) : m_pen(p), m_color(p.color()) { }

QPLine::QPLine(const PlotLine& copy) {
    operator=(copy);
}

QPLine::QPLine(const PlotLinePtr copy) {
    if(!copy.null()) operator=(*copy);
}

QPLine::~QPLine() { }


double QPLine::width() const { return m_pen.widthF(); }
void QPLine::setWidth(double width) { m_pen.setWidthF(width); }

PlotLine::Style QPLine::style() const {
    return QPOptions::linestyle(m_pen.style()); }
void QPLine::setStyle(Style style) {
    m_pen.setStyle(QPOptions::linestyle(style)); }

PlotColorPtr QPLine::color() const { return new QPColor(m_color); }
void QPLine::setColor(const PlotColor& color) {
    m_color = color;
    m_pen.setColor(m_color.asQColor());
}

const QPen& QPLine::asQPen() const { return m_pen; }
void QPLine::setAsQPen(const QPen& p) {
    m_pen = p;
    m_color.setAsQColor(p.color());
}


////////////////////////////
// QPAREAFILL DEFINITIONS //
////////////////////////////

QPAreaFill::QPAreaFill() : m_brush(), m_color(m_brush.color()) { }

QPAreaFill::QPAreaFill(const QBrush& b) : m_brush(b), m_color(b.color()) { }

QPAreaFill::QPAreaFill(const PlotAreaFill& copy) {
    operator=(copy);
}

QPAreaFill::QPAreaFill(const PlotAreaFillPtr copy) {
    if(!copy.null()) operator=(*copy);
}

QPAreaFill::~QPAreaFill() { }


PlotColorPtr QPAreaFill::color() const { return new QPColor(m_color); }
void QPAreaFill::setColor(const PlotColor& color) {
    m_color = color;
    m_brush.setColor(m_color.asQColor());
}

PlotAreaFill::Pattern QPAreaFill::pattern() const {
    return QPOptions::areapattern(m_brush.style()); }
void QPAreaFill::setPattern(Pattern pattern) {
    m_brush.setStyle(QPOptions::areapattern(pattern)); }

const QBrush& QPAreaFill::asQBrush() const { return m_brush; }
void QPAreaFill::setAsQBrush(const QBrush& b) {
    m_brush = b;
    m_color.setAsQColor(b.color());
}


//////////////////////////
// QPSYMBOL DEFINITIONS //
//////////////////////////

QPSymbol::QPSymbol() : m_style(QPOptions::symbol(style())), m_char('o'),
        m_heightIsPixel(true), m_drawPen(&pen()), m_drawBrush(&brush()) { }

QPSymbol::QPSymbol(const QwtSymbol& s): QwtSymbol(s),
        m_style(QPOptions::symbol(s.style())), m_char('o'),
        m_heightIsPixel(true), m_drawPen(&pen()), m_drawBrush(&brush()) { }

QPSymbol::QPSymbol(const PlotSymbol& copy) : m_heightIsPixel(true),
        m_drawPen(&pen()), m_drawBrush(&brush()) {
    operator=(copy); }

QPSymbol::QPSymbol(const PlotSymbolPtr copy) : m_style(CIRCLE), m_char('o'),
        m_heightIsPixel(true), m_drawPen(&pen()), m_drawBrush(&brush()) {
    if(!copy.null()) operator=(*copy); }

QPSymbol::~QPSymbol() { }


psize_t QPSymbol::size() const {
    if(m_style == PIXEL) return psize_t(1, 1);
    const QSize& s = QwtSymbol::size();
    return psize_t(s.width(), s.height());
}

void QPSymbol::setSize(double width, double height, bool heightIsPixel) {
    const QSize& s = QwtSymbol::size();
    if(s.width() != width || s.height() != height || (isCharacter() &&
       heightIsPixel != m_heightIsPixel)) {
        m_heightIsPixel = heightIsPixel;
        QwtSymbol::setSize((int)(width + 0.5), (int)(height + 0.5));
    } else m_heightIsPixel = heightIsPixel;
}

bool QPSymbol::heightIsPixel() const { return m_heightIsPixel; }

void QPSymbol::setHeightIsPixel(bool pixel) {
    if(isCharacter() && pixel != m_heightIsPixel) {
        m_heightIsPixel = pixel;
        QwtSymbol::setSize(QwtSymbol::size());
    } else m_heightIsPixel = pixel;
}

PlotSymbol::Symbol QPSymbol::symbol() const { return m_style; }
void QPSymbol::setSymbol(Symbol symbol) {
    if(symbol != m_style) {
        m_style = symbol;
        setStyle(QPOptions::symbol(m_style));
        
        if(symbol == PIXEL) {
            m_pixelPen.setColor(pen().color());
            m_drawPen = &m_pixelPen;
            m_drawBrush = &m_pixelBrush;
        } else {
            m_drawPen = &pen();
            m_drawBrush = &brush();
        }
    }
}

void QPSymbol::setSymbol(char c) {
    if(m_style != CHARACTER || m_char != c) {
        m_char = c;
        setSymbol(CHARACTER);
    }
}

void QPSymbol::setUSymbol(unsigned short unicode) {
    if(m_style != CHARACTER || m_char != QChar(unicode)) {
        m_char = QChar(unicode);
        setSymbol(CHARACTER);
    }
}

char QPSymbol::symbolChar() const { return m_char.unicode(); }
unsigned short QPSymbol::symbolUChar() const { return m_char.unicode(); }

PlotLinePtr QPSymbol::line() const { return new QPLine(pen()); }
void QPSymbol::setLine(const PlotLine& line) {
    setPen(QPLine(line).asQPen());
    m_pixelPen.setColor(pen().color());
}

PlotAreaFillPtr QPSymbol::areaFill() const { return new QPAreaFill(brush()); }
void QPSymbol::setAreaFill(const PlotAreaFill& fill) {
    setBrush(QPAreaFill(fill).asQBrush());
    m_pixelPen.setColor(brush().color());
}


void QPSymbol::draw(QPainter* p, const QRect& r) const {
    if(m_style == CHARACTER) {
        QFont font = p->font();
        if(m_heightIsPixel) font.setPixelSize(QwtSymbol::size().height());
        else                font.setPointSize(QwtSymbol::size().height());
        p->setFont(font);
        p->drawText(r, Qt::AlignCenter | Qt::AlignVCenter, QString(m_char));
    } else if(m_style == PIXEL || QwtSymbol::size() == QSize(1,1)) {
        p->drawPoint(r.center());
    } else {
        // draw using the normal QwtSymbol::draw() method.
        QwtSymbol::draw(p, r);
    }
}

QwtSymbol* QPSymbol::clone() const { return new QPSymbol(*this); }


const QPen& QPSymbol::drawPen() const { return *m_drawPen; }
const QBrush& QPSymbol::drawBrush() const { return *m_drawBrush; }


/////////////////////////
// QPMUTEX DEFINITIONS //
/////////////////////////

QPMutex::QPMutex() { }
QPMutex::~QPMutex() { }
void QPMutex::lock() { m_mutex.lock(); }
void QPMutex::unlock() { m_mutex.unlock(); }
bool QPMutex::tryLock() { return m_mutex.tryLock(); }


///////////////////////////
// QPOPTIONS DEFINITIONS //
///////////////////////////





QwtPlot::Axis  QPOptions::axis(PlotAxis a)    {
        switch(a) {
          case X_BOTTOM: return QwtPlot::xBottom;
          case X_TOP: return QwtPlot::xTop;
          case Y_LEFT: return QwtPlot::yLeft;
          case Y_RIGHT: return QwtPlot::yRight;
          default: return QwtPlot::xBottom;
        }
}

bool QPOptions::isAxisX( PlotAxis a ){
	bool horizontal = false;
	if ( a == X_BOTTOM || a == X_TOP ){
		horizontal = true;
	}
	return horizontal;
}
    

PlotAxis  QPOptions::axis(QwtPlot::Axis a)    {
        switch(a) {
          case QwtPlot::xBottom: return X_BOTTOM;
          case QwtPlot::xTop: return X_TOP;
          case QwtPlot::yLeft: return Y_LEFT;
          case QwtPlot::yRight: return Y_RIGHT;
        default: return X_BOTTOM;
        }
    }



QwtLinearColorMap QPOptions::standardSpectrogramMap() {
    QwtLinearColorMap colorMap(Qt::darkCyan, Qt::red);
    colorMap.addColorStop(0.1, Qt::cyan);
    colorMap.addColorStop(0.6, Qt::green);
    colorMap.addColorStop(0.95, Qt::yellow);
    return colorMap;
}

QwtLinearColorMap QPOptions::standardRasterMap() {
    QwtLinearColorMap colorMap(QColor("#000000"), QColor("#FFFFFF"));
    
    int numStops = 25;
    unsigned int rgb = 0;
    unsigned int delta = 0xFFFFFF / (numStops + 1);
    double interval = 1.0 / (numStops + 1);
    for(int i = 1; i <= numStops; i++) {
        rgb += delta;
        colorMap.addColorStop(i * interval, QColor(0xFF000000 + rgb));
    }
    
    return colorMap;
}

QwtLinearColorMap QPOptions::rasterMap(const vector<double>& vals) {
    QwtLinearColorMap colorMap(QColor("#000000"), QColor("#FFFFFF"));
    
    unsigned int white = 0xFFFFFF;
    for(unsigned int i = 0; i < vals.size(); i++) {
        if(vals[i] == white || vals[i] == 0xFFFFFF) continue;
        colorMap.addColorStop(vals[i] / white, QColor((unsigned int)vals[i]));
    }
    
    return colorMap;
}

}

#endif
