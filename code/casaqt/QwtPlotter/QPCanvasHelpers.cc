//# QPCanvasHelpers.cc: Helper classes for QPCanvas.
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

#include <casaqt/QwtPlotter/QPCanvasHelpers.qo.h>

#include <casaqt/QtUtilities/QtLayeredLayout.h>
#include <casaqt/QwtPlotter/QPCanvas.qo.h>

#include <qwt_painter.h>
#include <qwt_scale_engine.h>

#include <QPainter>

namespace casa {

///////////////////////////////
// QPMOUSEFILTER DEFINITIONS //
///////////////////////////////

QPMouseFilter::QPMouseFilter(QwtPlotCanvas* canvas) : m_canvas(canvas) {
    canvas->installEventFilter(this);
}

QPMouseFilter::~QPMouseFilter() { }

void QPMouseFilter::turnTracking(bool on) {
    m_canvas->setMouseTracking(on); }

bool QPMouseFilter::eventFilter(QObject* obj, QEvent* ev) {
    if(obj == m_canvas) {
        QMouseEvent* me = dynamic_cast<QMouseEvent*>(ev);
        if(me != NULL) emit mouseMoveEvent(me);
    }
    return false;
}


/////////////////////////
// QPSCALEDRAW METHODS //
/////////////////////////

QPScaleDraw::QPScaleDraw(QwtPlot* parent, QwtPlot::Axis axis) :
		m_parent(parent), m_axis(axis), m_scale(NORMAL),
		m_dateFormat(Plotter::DEFAULT_DATE_FORMAT),
		m_relativeDateFormat(Plotter::DEFAULT_RELATIVE_DATE_FORMAT),
		m_referenceSet(false), m_referenceValue(0) {
	if ( parent != NULL ){
		parent->setAxisScaleDraw(axis, this);
	}
}



QPScaleDraw::~QPScaleDraw() { }

PlotAxisScale QPScaleDraw::scale() const { return m_scale; }
void QPScaleDraw::setScale(PlotAxisScale scale) {
	if(scale != m_scale) {
		m_scale = scale;
		if ( m_parent != NULL ){
			if(m_scale == LOG10){
				m_parent->setAxisScaleEngine(m_axis, new QwtLog10ScaleEngine());
			}
			else {
				m_parent->setAxisScaleEngine(m_axis, new QwtLinearScaleEngine());
			}
			if(m_parent->autoReplot()){
				m_parent->replot();
			}
		}
	}
}

void QPScaleDraw::draw(QPainter* painter, const QPalette& palette) const {
	QwtScaleDraw::draw( painter, palette );
}

int QPScaleDraw::extent( const QPen& pen, const QFont& font ) const{
	int extent = QwtScaleDraw::extent( pen, font );
	return extent;
}

const String& QPScaleDraw::dateFormat() const { return m_dateFormat; }
void QPScaleDraw::setDateFormat(const String& newFormat) {
	if(newFormat != m_dateFormat) {
		m_dateFormat = newFormat;
		invalidateCache();
	}
}

const String& QPScaleDraw::relativeDateFormat() const {
    return m_relativeDateFormat; }
void QPScaleDraw::setRelativeDateFormat(const String& newFormat) {
    if(newFormat != m_relativeDateFormat) {
        m_relativeDateFormat = newFormat;
        invalidateCache();
    }
}

bool QPScaleDraw::referenceValueSet() const { return m_referenceSet; }
double QPScaleDraw::referenceValue() const { return m_referenceValue; }
void QPScaleDraw::setReferenceValue(bool on, double value) {
	if(on != m_referenceSet || (on && value != m_referenceValue)) {
		m_referenceSet = on;
		m_referenceValue = value;
		invalidateCache();
	}
}

QwtText QPScaleDraw::label(double value) const {
    if(m_referenceSet) value -= m_referenceValue;
    if(m_scale == DATE_MJ_DAY || m_scale == DATE_MJ_SEC) {
        return QString(Plotter::formattedDateString(
                m_referenceSet ? m_relativeDateFormat : m_dateFormat, value,
                m_scale, m_referenceSet).c_str());        
    } else return QwtScaleDraw::label(value);
}


//////////////////////////
// QPLEGEND DEFINITIONS //
//////////////////////////

QPLegend::QPLegend(QWidget* parent) : QwtLegend(parent) {
    setAutoFillBackground(false);
    
    // Clear line and background.
    m_line.setStyle(PlotLine::NOLINE);
    m_areaFill.setPattern(PlotAreaFill::NOFILL);
    
    QPalette p = palette();
    p.setBrush(backgroundRole(), QBrush(Qt::NoBrush));
    setPalette(p);
}

QPLegend::~QPLegend() { }


QSize QPLegend::sizeHint() const {
    QSize size = QwtLegend::sizeHint();
    if(!size.isValid() || m_line.style() == PlotLine::NOLINE){
    	return size;
    }
    
    int width = (int)(m_line.width() + 0.5);
    if(width > 0){
    	int verticalOffset = 20;
    	size += QSize(width * 2 + 1, width * 2 + 1 + verticalOffset );
    }
    return size;
}

const QPLine& QPLegend::line() const { return m_line; }
const QPen& QPLegend::pen() const { return m_line.asQPen(); }
void QPLegend::setLine(const PlotLine& line) {
    if(line != m_line) {
        m_line = QPLine(line);
        if(isVisible()) update();
    }
}

const QPAreaFill& QPLegend::areaFill() const { return m_areaFill; }
const QBrush& QPLegend::brush() const { return m_areaFill.asQBrush(); }
void QPLegend::setAreaFill(const PlotAreaFill& fill, bool updateLegend) {
    if(fill != m_areaFill) {
        m_areaFill = QPAreaFill(fill);
        QBrush fillBrush = m_areaFill.asQBrush();
        if(isVisible() && updateLegend ){
        	update();
        }
    }
}

void QPLegend::drawOutlineAndBackground(QPainter* painter, const QRect& rect,
        bool useQwtPainter) {
    if(painter == NULL || !isVisible() || !rect.isValid() ||
       (m_line.style() == PlotLine::NOLINE &&
        m_areaFill.pattern() == PlotAreaFill::NOFILL)) return;
    
    painter->save();
    if(m_line.style() != PlotLine::NOLINE) painter->setPen(m_line.asQPen());
    if(m_areaFill.pattern() != PlotAreaFill::NOFILL){
    	QBrush brush = m_areaFill.asQBrush();
        painter->setBrush(m_areaFill.asQBrush());
    }

    if(useQwtPainter){
    	QwtPainter::drawRect(painter, rect);
    }
    else {
    	painter->drawRect(rect);
    }
    painter->restore();
}


void QPLegend::paintEvent(QPaintEvent* event) {
    // Draw outline and background if needed.
	QSize legendSize = size();
    QRect geom(QPoint(0, 0), legendSize);
    geom.setRight(geom.right() - 1);   // Compensate for +1 in sizeHint().
    geom.setBottom(geom.bottom() - 1);
    QPainter painter(this);
    drawOutlineAndBackground(&painter, geom, false);
    painter.end();
    
    // Calculate "contents" rect.
    QRect rect = event->rect();
    rect.setRight(rect.right() - 1);   // Compensate for +1 in sizeHint().
    rect.setBottom(rect.bottom() - 1);
    
    int width = 0;
    if(m_line.style() != PlotLine::NOLINE){
    	width = (int)(m_line.width() + 0.5);
    }
    if(width > 0) {
        rect.setTop(rect.top() + width);
        rect.setLeft(rect.left() + width);
        rect.setRight(rect.right() - width);
        rect.setBottom(rect.bottom() - width);
    }

    // Have rest of the legend draw itself normally.
    QPaintEvent innerEvent(rect);
    QwtLegend::paintEvent(&innerEvent);
}


////////////////////////////////
// QPLEGENDHOLDER DEFINITIONS //
////////////////////////////////

const int QPLegendHolder::DEFAULT_INTERNAL_PADDING = 10;

QwtPlot::LegendPosition
QPLegendHolder::legendPosition(PlotCanvas::LegendPosition pos) {
    switch(pos) {
    case PlotCanvas::EXT_TOP:    return QwtPlot::TopLegend;
    case PlotCanvas::EXT_RIGHT:  return QwtPlot::RightLegend;
    case PlotCanvas::EXT_LEFT:   return QwtPlot::LeftLegend;
    case PlotCanvas::EXT_BOTTOM: return QwtPlot::BottomLegend;
    default:                     return QwtPlot::ExternalLegend;
    }
}


// Constructor which takes the canvas and initial position.
QPLegendHolder::QPLegendHolder(QPCanvas* canvas, PlotCanvas::LegendPosition ps,
        int padding) : m_canvas(canvas), m_legend(new QPLegend()),
        m_position(ps), m_padding(padding) {
    // Set up layout.
    QGridLayout* l = new QGridLayout(this);
    int ml, mt, mr, mb;
    canvas->asQwtPlot().getContentsMargins(&ml, &mt, &mr, &mb);
    l->setContentsMargins(ml, mt, mr, mb);
    l->setSpacing(0);
    if(m_padding < 0) m_padding = DEFAULT_INTERNAL_PADDING;
    
    // Put in spacers and legend.
    l->addItem(m_spaceTop = new QSpacerItem(0, 0), 0, 0, 1, 3);
    l->addItem(m_spaceLeft = new QSpacerItem(0, 0), 1, 0);
    l->addItem(m_spaceRight = new QSpacerItem(0, 0), 1, 2);
    l->addItem(m_spaceBottom = new QSpacerItem(0, 0), 2, 0, 1, 3);
    
    // Add to canvas.
    QtLayeredLayout* cl = dynamic_cast<QtLayeredLayout*>(canvas->layout());
    if(cl != NULL) cl->addWidget(this);
    setPosition(ps);
    
    // Set up tracking, etc.
    canvas->asQwtPlot().installLegendFilter(this);
    setMouseTracking(true);
}

QPLegendHolder::~QPLegendHolder() { }


bool QPLegendHolder::legendShown() const {
    return isVisible() && m_legend->isVisible(); }
void QPLegendHolder::showLegend(bool show) {
    m_legend->setVisible(show);
    m_canvas->asQwtPlot().insertLegend(show ? m_legend : NULL,
            legendPosition(m_position));
}

bool QPLegendHolder::isInternal() const {
    return PlotCanvas::legendPositionIsInternal(m_position); }
PlotCanvas::LegendPosition QPLegendHolder::position() const { return m_position; }
void QPLegendHolder::setPosition(PlotCanvas::LegendPosition pos) {
    m_position = pos;

    QGridLayout* l = dynamic_cast<QGridLayout*>(layout());
    l->removeWidget(m_legend);
    if(isInternal()) {
        l->addWidget(m_legend, 1, 1);
        updateSpacers();
    }

    m_canvas->asQwtPlot().insertLegend(m_legend, legendPosition(pos));
}

QRect QPLegendHolder::internalLegendRect(const QRect& canvRect,
        bool useQwtPainter) const {
    if(!legendShown() || !isInternal()) return QRect();
    
    QSize size = m_legend->sizeHint();
    int paddingx = m_padding, paddingy = m_padding;
    
    if(useQwtPainter && !QwtPainter::metricsMap().isIdentity()) {
        size = QwtPainter::metricsMap().layoutToDevice(size);
        paddingx = QwtPainter::metricsMap().layoutToDeviceX(paddingx);
        paddingy = QwtPainter::metricsMap().layoutToDeviceY(paddingy);
    }
    
    QRect rect(QPoint(0, 0), size);
    if(m_position == PlotCanvas::INT_ULEFT ||
       m_position == PlotCanvas::INT_URIGHT)
        rect.moveTop(canvRect.top() + paddingy);
    if(m_position == PlotCanvas::INT_ULEFT ||
       m_position == PlotCanvas::INT_LLEFT)
        rect.moveLeft(canvRect.left() + paddingx);
    if(m_position == PlotCanvas::INT_URIGHT ||
       m_position == PlotCanvas::INT_LRIGHT)
        rect.moveRight(canvRect.right() - paddingx);
    if(m_position == PlotCanvas::INT_LLEFT ||
       m_position == PlotCanvas::INT_LRIGHT)
        rect.moveBottom(canvRect.bottom() - paddingy);
    
    return rect;
}


void QPLegendHolder::updateSpacers() {
    // Adjust spacing for position.
    int top = 0, left = 0, right = 0, bottom = 0;
    QSizePolicy::Policy pt = QSizePolicy::Expanding, pl = pt, pr = pt,
                        pb = pt;
    if(m_position==PlotCanvas::INT_ULEFT||m_position==PlotCanvas::INT_URIGHT){
        top = m_padding; pt = QSizePolicy::Fixed; }
    if(m_position==PlotCanvas::INT_LLEFT||m_position==PlotCanvas::INT_LRIGHT){
        bottom = m_padding; pb = QSizePolicy::Fixed; }
    if(m_position==PlotCanvas::INT_LLEFT||m_position==PlotCanvas::INT_ULEFT){
        left = m_padding; pl = QSizePolicy::Fixed; }
    if(m_position==PlotCanvas::INT_LRIGHT||m_position==PlotCanvas::INT_URIGHT){
        right = m_padding; pr = QSizePolicy::Fixed; }
        
    // Adjust spacing for actual canvas.
    QRect orect = m_canvas->contentsRect();
    orect.moveTopLeft(m_canvas->mapToGlobal(orect.topLeft()));
    QwtPlotCanvas* canv = m_canvas->asQwtPlot().canvas();
    QRect irect = canv->contentsRect();
    irect.moveTopLeft(canv->mapToGlobal(irect.topLeft()));
    if(top > 0) top += irect.top() - orect.top();
    if(left > 0) left += irect.left() - orect.left();
    if(right > 0) right += orect.right() - irect.right();
    if(bottom > 0) bottom += orect.bottom() - irect.bottom();
    
    // Set spacing.
    m_spaceTop->changeSize(0, top, QSizePolicy::Minimum, pt);
    m_spaceLeft->changeSize(left, 0, pl, QSizePolicy::Minimum);
    m_spaceRight->changeSize(right, 0, pr, QSizePolicy::Minimum);
    m_spaceBottom->changeSize(0, bottom, QSizePolicy::Minimum, pb);
}


////////////////////////////
// QPBASEITEM DEFINITIONS //
////////////////////////////

const double QPBaseItem::BASE_Z_CARTAXIS = -2;
const double QPBaseItem::BASE_Z_GRID = -1;

QPBaseItem::QPBaseItem() : m_canvas(NULL) { }

QPBaseItem::~QPBaseItem() { qpDetach(); }

void QPBaseItem::qpAttach(QPCanvas* canvas) {
    if(m_canvas != canvas) {
        qpDetach();
        m_canvas = canvas;
        if(m_canvas != NULL) QwtPlotItem::attach(&m_canvas->asQwtPlot());
    }
}

void QPBaseItem::qpDetach() {
    if(m_canvas != NULL) {
        QwtPlotItem::detach();
        m_canvas = NULL;
    }
}


////////////////////////
// QPGRID DEFINITIONS //
////////////////////////

QPGrid::QPGrid() { QPBaseItem::setZ(BASE_Z_GRID); }
QPGrid::~QPGrid() { }

bool QPGrid::shouldDraw() const {
    bool drawMaj = majPen().style() != Qt::NoPen && (xEnabled() || yEnabled());
    bool drawMin = minPen().style() != Qt::NoPen &&
                   (xMinEnabled() || yMinEnabled());
    return drawMaj || drawMin;
}


/////////////////////////////////
// QPCARTESIANAXIS DEFINITIONS //
/////////////////////////////////

QPCartesianAxis::QPCartesianAxis(QwtPlot::Axis master, QwtPlot::Axis slave) :
        m_axis(master), m_scaleDraw() 
{
    if(master == QwtPlot::yLeft || master == QwtPlot::yRight)
        setAxis(slave, master);
    else
        setAxis(master, slave);
    
    
    switch(master) {
      case QwtPlot::yLeft:    m_scaleDraw.setAlignment(QwtScaleDraw::LeftScale);   break;
      case QwtPlot::yRight:   m_scaleDraw.setAlignment(QwtScaleDraw::RightScale);  break;
      case QwtPlot::xTop:     m_scaleDraw.setAlignment(QwtScaleDraw::TopScale);    break;
      case QwtPlot::xBottom:  m_scaleDraw.setAlignment(QwtScaleDraw::BottomScale); break;
      default:
    	  qDebug() << "Unrecognized Axis: "<<master;
    }
    
    setZ(BASE_Z_CARTAXIS);
}



QPCartesianAxis::~QPCartesianAxis() { }



void QPCartesianAxis::draw_(QPainter* painter, const QwtScaleMap& xMap,
                  const QwtScaleMap& yMap, const QRect& /*drawRect*/,
                  unsigned int /*drawIndex*/, unsigned int /*drawCount*/) const {
	
    QwtScaleDraw* s = const_cast<QwtScaleDraw*>(&m_scaleDraw);
    if(m_axis == QwtPlot::yLeft || m_axis == QwtPlot::yRight) 
		{
        s->move(QPOptions::round(xMap.xTransform(0.0)),
                QPOptions::round(yMap.p2()));
        s->setLength(QPOptions::round(yMap.p1() - yMap.p2()));
		} 
    else if(m_axis == QwtPlot::xBottom || m_axis == QwtPlot::xTop) 
		{
        s->move(QPOptions::round(xMap.p1()),
                QPOptions::round(yMap.xTransform(0.0)));
        s->setLength(QPOptions::round(xMap.p2() - xMap.p1()));
		}
    
    s->setScaleDiv(*(plot()->axisScaleDiv(m_axis)));
    s->draw(painter, plot()->palette());
}

}

#endif
