//# Copyright (C) 2005
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

#include <casa/aips.h>
#include <casa/Containers/Record.h>

#include <display/QtPlotter/QtCanvas.qo.h>


#include <graphics/X11/X_enter.h>
#include <QDir>
#include <QColor>
#include <QHash>
#include <QWidget>
#include <QMouseEvent>
#include <cmath>
#include <QtGui>
#include <iostream>
#include <graphics/X11/X_exit.h>


 
namespace casa { 

QtCanvas::~QtCanvas()
{}

QtCanvas::QtCanvas(QWidget *parent)
        : QWidget(parent), 
          title(), xLabel(), yLabel(), welcome()
{    
	
    setAttribute(Qt::WA_NoBackground);
    setBackgroundRole(QPalette::Dark);
    setForegroundRole(QPalette::Light);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);
    rubberBandIsShown = false;
    imageMode = false;
    
    setPlotSettings(QtPlotSettings());
    setMarkMode(false);
    autoScale = 2;
    
}

void QtCanvas::setPlotSettings(const QtPlotSettings &settings)
{
    zoomStack.resize(1);
    zoomStack[0] = settings;
    curZoom = 0;
    
    curMarker = 0;
    refreshPixmap();
}

void QtCanvas::zoomOut()
{
    if (curZoom > 0)
    {
        --curZoom;        
        refreshPixmap();
        emit zoomChanged();
    }
}

void QtCanvas::zoomIn()
{
    if (curZoom < (int)zoomStack.size() - 1)
    {
        ++curZoom;        
        refreshPixmap();
        emit zoomChanged();
    }
}
void QtCanvas::markNext()
{
    
    //cout << " curMarker=" << curMarker 
    //<< " size=" << (int)markerStack.size() << endl;
   if (markMode && curMarker < (int)markerStack.size() - 1)
    {               
        ++curMarker;	
    }
    refreshPixmap();
}
void QtCanvas::markPrev()
{    
   //cout << " --------------curMarker=" << curMarker 
   // << " size=" << (int)markerStack.size() << endl;
   if (markMode && curMarker > 0)
    {
        --curMarker; 
        refreshPixmap();
    }
}
int QtCanvas::getLineCount()
{
    return curveMap.size();
}
CurveData* QtCanvas::getCurveData(int id)
{
    return &curveMap[id];
}
QString QtCanvas::getCurveName(int id)
{
    return legend[id];
}
void QtCanvas::setCurveData(int id, const CurveData &data, 
                                    const QString& lbl)
{
    curveMap[id] = data;
    legend[id] = lbl;
    refreshPixmap();
}
void QtCanvas::setDataRange()
{
    if (autoScale == 0)
       return;

    double xmin = 1000000000000000000000000.;
    double xmax = -xmin;
    double ymin = 1000000000000000000000000.;
    double ymax = -ymin;
    std::map<int, CurveData>::const_iterator it = curveMap.begin();
    while (it != curveMap.end())
    {
        const CurveData &data = (*it).second;
        int maxPoints = data.size() / 2;

        for (int i = 0; i < maxPoints; ++i)
        {
            double dx = data[2 * i];
            double dy = data[2 * i + 1];
            xmin = (xmin > dx) ? dx : xmin;
            xmax = (xmax < dx) ? dx : xmax;
            ymin = (ymin > dy) ? dy : ymin;
            ymax = (ymax < dy) ? dy : ymax;
        }
        ++it;
    }

    QtPlotSettings settings;
    if (xmax == xmin)
    {
        xmax = xmax + 0.5;
        xmin = xmin - 0.5;
    }
    if (ymax == ymin)
    {
        ymax = ymax + 0.5;
        ymin = ymin - 0.5;
    }
    settings.minX = xmin;
    settings.maxX = xmax;
    settings.minY = ymin;
    settings.maxY = ymax;
    settings.adjust();
    setPlotSettings(settings);

}

void QtCanvas::clearData()
{
    curveMap.clear();
    legend.clear();
}

void QtCanvas::clearCurve(int id)
{
    if (id == -1) {
       curveMap.clear();
       legend.clear();
    }
    else {
       curveMap.erase(id);
       legend.erase(id);
    }
    refreshPixmap();
}

QSize QtCanvas::minimumSizeHint() const
{
    return QSize(4 * Margin, 4 * Margin);
}

QSize QtCanvas::sizeHint() const
{
    return QSize(8 * Margin, 6 * Margin);
}

void QtCanvas::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    QVector<QRect> rects = event->region().rects();
    for (int i = 0; i < (int)rects.size(); ++i)
        painter.drawPixmap(rects[i], pixmap, rects[i]);

    //painter.drawPixmap(0, 0, pixmap);

    if (rubberBandIsShown)
    {
        painter.setPen(Qt::yellow);
        painter.drawRect(rubberBandRect.normalized());
    }
    if (hasFocus())
    {
        QStyleOptionFocusRect option;
        option.init(this);
        option.backgroundColor = palette().color(QPalette::Background);
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter,
                               this);
    }

}

void QtCanvas::resizeEvent(QResizeEvent *)
{       
    refreshPixmap();
}

void QtCanvas::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        rubberBandIsShown = true;
        rubberBandRect.setTopLeft(event->pos());
        rubberBandRect.setBottomRight(event->pos());
        updateRubberBandRegion();
        setCursor(Qt::CrossCursor);
    }
    if (event->button() == Qt::RightButton)
    {
      int x = event->pos().x() - Margin;
      int y = event->pos().y() - Margin;
      //cout << "x=" << x << " y=" << y << endl;
     QtPlotSettings prevSettings = zoomStack[curZoom];
        QtPlotSettings settings;

        double dx = prevSettings.spanX() / (width() - 2 * Margin);
        double dy = prevSettings.spanY() / (height() - 2 * Margin);
        x = (int)(prevSettings.minX + dx * x);        
        y = (int)(prevSettings.maxY - dy * y);
        //cout << "x=" << x << " y=" << y << endl;
		
    std::map<int, CurveData>::const_iterator it = markerStack.begin();

    while (it != markerStack.end())
    {
        int id = (*it).first;
        const CurveData &data = (*it).second;
        //cout << " " << data[0] << " " << data[2] 
	//        << " " << data[1] << " " << data[3] << endl;
	if ( x >= data[0] && x < data[2] && 
	     y <= data[1] && y > data[3]) {
	     markerStack[id] = markerStack[markerStack.size() - 1];
            markerStack.erase(markerStack.size() - 1);
	    refreshPixmap();
	    break;
	}
        ++it;
    }
}
}
void QtCanvas::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        updateRubberBandRegion();
        rubberBandRect.setBottomRight(event->pos());
        updateRubberBandRegion();
    }
}

void QtCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    //qDebug() << "mouse release" << event->button()
    //         << Qt::LeftButton;
    if (event->button() == Qt::LeftButton)
    {
        rubberBandIsShown = false;
        updateRubberBandRegion();
        unsetCursor();

        QRect rect = rubberBandRect.normalized();
        
        //qDebug() << "rect: left" << rect.left()
        //         << "right" << rect.right()
        //         << "top" << rect.top()
        //         << "bottom" << rect.bottom()
        //         << "width" << width()
        //         << "height" << height()
        //         << "margin" << Margin;
        //zoom only if zoom box is in the plot region
        if (rect.left() < Margin || rect.top() < Margin ||
            rect.right() > width() - Margin ||
            rect.bottom() > height() -  Margin)
            return;
        //qDebug() << "inside";
        
        if (rect.width() < 4 || rect.height() < 4)
            return;
        //qDebug() << "big enough";    
        
        //cout << "numCurves " << curveMap.size() << endl;
        //if (curveMap.size() == 0)
        //    return;
 
        rect.translate(-Margin, -Margin);

        QtPlotSettings prevSettings = zoomStack[curZoom];
        QtPlotSettings settings;

        double dx = prevSettings.spanX() / (width() - 2 * Margin);
        double dy = prevSettings.spanY() / (height() - 2 * Margin);
        settings.minX = prevSettings.minX + dx * rect.left();
        settings.maxX = prevSettings.minX + dx * rect.right();
        settings.minY = prevSettings.maxY - dy * rect.bottom();
        settings.maxY = prevSettings.maxY - dy * rect.top();
        settings.adjust();
        if (!markMode)
        {
            if (curveMap.size() != 0) {
            //qDebug() << "zoomin ";
            zoomStack.resize(curZoom + 1);
            zoomStack.push_back(settings);
            zoomIn();
            }
        }
        else
        {
            //qDebug() << "mark ";
            CurveData data;
            data.push_back(settings.minX);
            data.push_back(settings.maxY);
            data.push_back(settings.maxX);
            data.push_back(settings.minY);

            //cout << " minX=" << (int)settings.minX
            //<< " maxX=" << (int)settings.maxX
            //<< " minY=" <<  (int)settings.minY
            //<< " maxY=" << (int)settings.maxY << endl;
            markerStack[markerStack.size()] = data;
	    markNext();
        }
    }
}

void QtCanvas::keyPressEvent(QKeyEvent *event)
{
    if (!imageMode)
    switch (event->key())
    {
    case Qt::Key_Plus:
        zoomIn();
        break;
    case Qt::Key_Minus:
        zoomOut();
        break;
    case Qt::Key_Left:
        zoomStack[curZoom].scroll(-1, 0);
        refreshPixmap();
        break;
    case Qt::Key_Right:
        zoomStack[curZoom].scroll(+1, 0);
        refreshPixmap();
        break;
    case Qt::Key_Down:
        zoomStack[curZoom].scroll(0, -1);
        refreshPixmap();
        break;
    case Qt::Key_Up:
        zoomStack[curZoom].scroll(0, +1);
        refreshPixmap();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void QtCanvas::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    int numTicks = numDegrees / 15;

    if (event->orientation() == Qt::Horizontal)
        zoomStack[curZoom].scroll(numTicks, 0);
    else
        zoomStack[curZoom].scroll(0, numTicks);
    refreshPixmap();
}

void QtCanvas::updateRubberBandRegion()
{
    // QRect rect = rubberBandRect.normalized();
    // update(rect.left(), rect.top(), rect.width(), 1);
    // update(rect.left(), rect.top(), 1, rect.height());
    // update(rect.left(), rect.bottom(), rect.width(), 1);
    // update(rect.right(), rect.top(), 1, rect.height());
    update();
}

void QtCanvas::refreshPixmap()
{
    pixmap = QPixmap(size());
    pixmap.fill(this, 0, 0);
    QPainter painter(&pixmap);
    
    drawLabels(&painter);
    if (!imageMode)
    {
        drawGrid(&painter);
        drawCurves(&painter);
    }
    else
    {
        drawBackBuffer(&painter);
        drawTicks(&painter);
        if (markMode)
            drawRects(&painter);
    }
    if (welcome.text != "") 
       drawWelcome(&painter);
    update();
}
void QtCanvas::drawBackBuffer(QPainter *painter)
{
    QRect rect(Margin, Margin,  width() - 2 * Margin, height() - 2 * Margin);
    QtPlotSettings settings = zoomStack[curZoom];
    QRect src((int)settings.minX, (int)settings.minY,
              (int)settings.maxX, (int)settings.maxY);
    painter->drawPixmap(rect, backBuffer, src);
}

void QtCanvas::drawGrid(QPainter *painter)
{
    QRect rect(Margin, Margin,
               width() - 2 * Margin, height() - 2 * Margin);
    QtPlotSettings settings = zoomStack[curZoom];
    QPen quiteDark(QPalette::Dark);
    QPen light(QPalette::Highlight);

    for (int i = 0; i <= settings.numXTicks; ++i)
    {
        int x = rect.left() + (i * (rect.width() - 1)
                               / settings.numXTicks);
        double label = settings.minX + (i * settings.spanX()
                                        / settings.numXTicks);
        if (abs(label) < 0.00000005) label = 0.f;                                
        painter->setPen(quiteDark);
        painter->drawLine(x, rect.top(), x, rect.bottom());
        painter->setPen(light);
        painter->drawLine(x, rect.bottom(), x, rect.bottom() + 5);
        painter->drawText(x - 50, rect.bottom() + 5, 100, 15,
                          Qt::AlignHCenter | Qt::AlignTop,
                          QString::number(label));
    }
    for (int j = 0; j <= settings.numYTicks; ++j)
    {
        int y = rect.bottom() - (j * (rect.height() - 1)
                                 / settings.numYTicks);
        double label = settings.minY + (j * settings.spanY()
                                        / settings.numYTicks);

        if (abs(label) < 0.00000005) label = 0.f;                                 
        painter->setPen(quiteDark);
        painter->drawLine(rect.left(), y, rect.right(), y);
        painter->setPen(light);
        painter->drawLine(rect.left() - 5, y, rect.left(), y);
        painter->drawText(rect.left() - Margin, y - 10,
                          Margin - 5, 20,
                          Qt::AlignRight | Qt::AlignVCenter,
                          QString::number(label));
    }
    painter->drawRect(rect);
    //painter->setPen(Qt::red);
    //painter->drawRect(rubberBandRect);
}
void QtCanvas::drawTicks(QPainter *painter)
{
    QRect rect(Margin, Margin,
               width() - 2 * Margin, height() - 2 * Margin);
    QtPlotSettings settings = zoomStack[curZoom];
    QPen quiteDark(QPalette::Dark);
    QPen light(QPalette::Highlight);

    for (int i = 0; i <= settings.numXTicks; ++i)
    {
        int x = rect.left() + (i * (rect.width() - 1)
                               / settings.numXTicks);
        double label = settings.minX + (i * settings.spanX()
                                        / settings.numXTicks);
        if (abs(label) < 0.00000005) label = 0.f;                                
        painter->setPen(quiteDark);
        painter->drawLine(x, rect.top(), x, rect.top() + 5);
        painter->setPen(light);
        painter->drawLine(x, rect.bottom() - 5, x, rect.bottom());
        painter->drawText(x - 50, rect.bottom() + 5, 100, 15,
                          Qt::AlignHCenter | Qt::AlignTop,
                          QString::number(label));
    }
    for (int j = 0; j <= settings.numYTicks; ++j)
    {
        int y = rect.bottom() - (j * (rect.height() - 1)
                                 / settings.numYTicks);
        double label = settings.minY + (j * settings.spanY()
                                        / settings.numYTicks);
        if (abs(label) < 0.00000005) label = 0.f;                                  
        painter->setPen(quiteDark);
        painter->drawLine(rect.right() - 5, y, rect.right(), y);
        painter->setPen(light);
        painter->drawLine(rect.left(), y, rect.left() + 5, y);
        painter->drawText(rect.left() - Margin / 2, y - 10,
                          Margin - 5, 20,
                          Qt::AlignRight | Qt::AlignVCenter,
                          QString::number(label));
    }
    painter->drawRect(rect);
    //painter->setPen(Qt::red);
    //painter->drawRect(rubberBandRect);
}
void QtCanvas::drawLabels(QPainter *painter)
{
    QFont ft(painter->font());
    QPen pen(painter->pen());
    
    painter->setPen(title.color);
    painter->setFont(QFont(title.fontName, title.fontSize));
    painter->drawText(Margin, 8, width() - 2 * Margin, Margin / 2,
                          Qt::AlignHCenter | Qt::AlignTop, title.text);
                          
    painter->setPen(xLabel.color);                  
    painter->setFont(QFont(xLabel.fontName, xLabel.fontSize));                          
    painter->drawText(Margin, height() - Margin / 2, width() - 2 * Margin, Margin / 2,
                          Qt::AlignHCenter | Qt::AlignTop, xLabel.text);
                                          
    QPainterPath text;     
    QFont font(yLabel.fontName, yLabel.fontSize); 
    QRect fontBoundingRect = QFontMetrics(font).boundingRect(yLabel.text); 
    text.addText(-QPointF(fontBoundingRect.center()), font, yLabel.text);                   
    font.setPixelSize(50);
    painter->rotate(-90);
    painter->translate(- height() / 2, Margin / 6);  
    painter->fillPath(text, yLabel.color);
    painter->translate(height() / 2, - Margin / 4);
    painter->rotate(90);
    
    painter->setPen(pen);                   
    painter->setFont(ft);
    

}

void QtCanvas::drawWelcome(QPainter *painter)
{
    QFont ft(painter->font());
    QPen pen(painter->pen());
    
    painter->setPen(welcome.color);
    painter->setFont(QFont(title.fontName, welcome.fontSize));
    painter->drawText(Margin, Margin, 
                      width() - 2 * Margin, height() - Margin * 2,
                      Qt::AlignHCenter | Qt::AlignVCenter, 
                      welcome.text);
    painter->setPen(pen);                   
    painter->setFont(ft);
    

}
void QtCanvas::drawRects(QPainter *painter)
{
    QtPlotSettings settings = zoomStack[curZoom];
    QRect rect(Margin, Margin,
               width() - 2 * Margin, height() - 2 * Margin);

    painter->setClipRect(rect.x() + 1, rect.y() + 1,
                         rect.width() - 2, rect.height() - 2);

    std::map<int, CurveData>::const_iterator it = markerStack.begin();

    int h = (*pMask).ncolumn();
    int w = (*pMask).nrow();
    for (int i = 0; i < w; i++) 
        for (int j = 0; j < h; j++) {
	      (*pMask)(i, j) = 1;
	}
    while (it != markerStack.end())
    {
        int id = (*it).first;
        const CurveData &data = (*it).second;

        QPainterPath points;
        //cout << " id=" << id << endl; 
	//cout << " " << data[0] << " " << data[2] 
	//        << " " << data[1] << " " << data[3] << endl;
	int h = (*pMask).ncolumn();
	for (int i = (int)data[0]; i < (int)data[2]; i++) 
        for (int j = (int)data[3]; j < (int)data[1]; j++) {
	      (*pMask)(i, h - j) = 0;
	}
	       
	double dx1 = data[0] - settings.minX;
        double dy1 = data[1] - settings.minY;
        double dx2 = data[2] - settings.minX;
        double dy2 = data[3] - settings.minY;
	
        double x1 = rect.left() + (dx1 * (rect.width() - 1)
                                   / settings.spanX());
        double y1 = rect.bottom() - (dy1 * (rect.height() - 1)
                                     / settings.spanY());
        double x2 = rect.left() + (dx2 * (rect.width() - 1)
                                   / settings.spanX());
        double y2 = rect.bottom() - (dy2 * (rect.height() - 1)
                                     / settings.spanY());

        points.addRect((int)x1, (int)y1, (int)(x2 - x1), (int)(y2 - y1));
       

        painter->setPen(Qt::green);
	//cout << "curMarker=" << curMarker << " id=" << id << endl;
	if (id == curMarker)
	painter->setPen(Qt::white);
        painter->drawPath(points);
        ++it;
    }
}

QColor QtCanvas::getLinearColor(double d)
{
    double middle = 0.4;
    if (d > 1)
        return QColor(0, 0, 255);
    if (d < 0)
        return QColor(255, 0, 0);
    double r, g, b;
    if (d < 0.4)
    {
        r = 255 * (middle - d) / middle;
        g = 255 * d / middle;
        b = 0;
    }
    else
    {
        r = 0;
        g = 255 * ( 1 - d) / (1 - middle);
        b = (d - middle) * 255 / (1 - middle);
    }
    //std::cout << "r=" << r << " g=" << g << " b=" << b << std::endl;
    return QColor((int)r, (int)g, (int)b);
}
void QtCanvas::drawCurves(QPainter *painter)
{

    QFont ft(painter->font());
    QPen pen(painter->pen());
    
    QtPlotSettings settings = zoomStack[curZoom];
    QRect rect(Margin, Margin,
               width() - 2 * Margin, height() - 2 * Margin);

    painter->setClipRect(rect.x() + 1, rect.y() + 1,
                         rect.width() - 2, rect.height() - 2);

    std::map<int, CurveData>::const_iterator it = curveMap.begin();

    int siz = curveMap.size();
    int sz = siz > 1 ? siz - 1 : 1;
    QColor colorFolds[sz];

    while (it != curveMap.end())
    {
        int id = (*it).first;
        const CurveData &data = (*it).second;
        int maxPoints = data.size() / 2;
        QPainterPath points;
        double n = (double)id / sz;

        colorFolds[id] = getLinearColor(n);

        for (int i = 0; i < maxPoints; ++i)
        {
            double dx = data[2 * i] - settings.minX;
            double dy = data[2 * i + 1] - settings.minY;
            double x = rect.left() + (dx * (rect.width() - 1)
                                      / settings.spanX());
            double y = rect.bottom() - (dy * (rect.height() - 1)
                                        / settings.spanY());
            if (fabs(x) < 32768 && fabs(y) < 32768)
            {
                if (i == 0)
                    points.moveTo((int)x, (int)y);
                points.lineTo((int)x, (int)y);
            }
        }
        painter->setPen(colorFolds[(uint)id % 6]);
        painter->drawPath(points);
   
        if (siz > 1) {
           painter->setFont(QFont(xLabel.fontName, xLabel.fontSize));  
           painter->drawText(Margin + 4, Margin + (5 + id * 15), 
                          width() - 2 * Margin, Margin / 2,
                          Qt::AlignLeft | Qt::AlignTop, legend[id]);
        }                                  
        ++it;
    }

    painter->setPen(pen);                   
    painter->setFont(ft);
}


void QtCanvas::setMarkMode(bool b)
{
    markMode = b;
}

void QtCanvas::addPolyLine(const Vector<Float> &x,
                           const Vector<Float> &y, 
                           const QString& lb)
{
    //qDebug() << "add poly line float";
    Int xl, yl;
    x.shape(xl);
    y.shape(yl);
    CurveData data;
    for (uInt i = 0; i < (uInt)min(xl, yl); i++)
    {
        data.push_back(x[i]);
        data.push_back(y[i]);
    }
    int j = curveMap.size();
    //qDebug() << "j:" << j;
    setCurveData(j, data, lb);

    setDataRange();
    return;
}

void QtCanvas::plotPolyLines(QString path)
{
    QFile file(path);

    if (file.open(QIODevice::ReadOnly))
    {
        QTextStream in(&file);
        int lineCount = 0;
        int pointCount = 0;
        CurveData data;
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (!line.isNull() && !line.isEmpty())
            {
                if (line.startsWith('#')) {
                   if (line.contains("title"))
                      setTitle(line.mid(7)); 
                   if (line.contains("xLabel"))
                      setXLabel(line.mid(8)); 
                   if (line.contains("yLabel"))
                      setYLabel(line.mid(8)); 
                }
                else {
                   QStringList coords = line.trimmed().split(QRegExp("\\s+"));
                   if (coords.size() >= 1)
                   {
                       double x = coords[0].toDouble();
                       double y = coords[1].toDouble();
                       data.push_back(x);
                       data.push_back(y);
                       pointCount++;
                   }
                   if (in.atEnd() && pointCount > 0)
                   {
                       setCurveData(lineCount, data);
                   }
                }
            }
            else
            {
                if (pointCount > 0)
                {
                    setCurveData(lineCount, data);
                    pointCount = 0;
                    lineCount++;
                    data.clear();
                }
            }

        }
    }

    setDataRange();
    return;
}
void QtCanvas::plotPolyLine(const Vector<Float> &x, const Vector<Float> &y,
                            const QString& lb)
{

    //qDebug() << "plot poly line float";
    Int xl, yl;
    x.shape(xl);
    y.shape(yl);
    CurveData data;
    for (uInt i = 0; i < (uInt)min(xl, yl); i++)
    {
        data.push_back(x[i]);
        data.push_back(y[i]);
    }
    setCurveData(0, data, lb);

    setDataRange();
    return;
}
void QtCanvas::plotPolyLine(const Vector<Int> &x, const Vector<Int> &y)
{
    //qDebug() << "plot poly line int";
    Int xl, yl;
    x.shape(xl);
    y.shape(yl);
    CurveData data;
    for (int i = 0; i < min(xl, yl); i++)
    {
        data.push_back(x[i]);
        data.push_back(y[i]);
    }
    setCurveData(0, data);

    setDataRange();
    return;
}
void QtCanvas::plotPolyLine(
        const Vector<Double> &x, const Vector<Double>&y)
{
    //qDebug() << "plot poly line double";
    Int xl, yl;
    x.shape(xl);
    y.shape(yl);
    CurveData data;
    for (Int i = 0; i < min(xl, yl); i++)
    {
        data.push_back(x[i]);
        data.push_back(y[i]);
    }
    setCurveData(0, data);

    setDataRange();
    return;
}
void QtCanvas::plotPolyLine(const Matrix<Double> &x)
{

    Int xl, yl;
    x.shape(xl, yl);
    int nr = xl / 2;
    int nc = yl / 2;
    int n = min (nr, nc);
    if (n > 0)
    {
        CurveData data[n];
        if (n < nr)
        {
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < xl; j++)
                {
                    data[i].push_back(x(uInt(j), uInt(2 * i)));
                    data[i].push_back(x(uInt(j), uInt(2 * i + 1)));
                }
                setCurveData(i, data[i]);
            }
        }
        else
        {
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < yl; j++)
                {
                    data[i].push_back(x(2 * i, j));
                    data[i].push_back(x(2 * i + 1, j));
                }
                setCurveData(i, data[i]);
            }
        }


    }
    setDataRange();
    return;
}
void QtCanvas::plotPolyLine(const Matrix<Int> &x)
{

    Int xl, yl;
    x.shape(xl, yl);
    int nr = xl / 2;
    int nc = yl / 2;
    int n = min (nr, nc);
    if (n > 0)
    {
        CurveData data[n];
        if (n < nr)
        {
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < xl; j++)
                {
                    data[i].push_back(x(j, 2 * i));
                    data[i].push_back(x(j, 2 * i + 1));
                }
                setCurveData(i, data[i]);
            }
        }
        else
        {
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < yl; j++)
                {
                    data[i].push_back(x(2 * i, j));
                    data[i].push_back(x(2 * i + 1, j));
                }
                setCurveData(i, data[i]);
            }
        }

    }

    setDataRange();
    return;
}
void QtCanvas::plotPolyLine(const Matrix<Float> &x)
{

    Int xl, yl;
    x.shape(xl, yl);
    int nr = xl / 2;
    int nc = yl / 2;
    int n = min (nr, nc);
    if (n > 0)
    {
        CurveData data[n];
        if (n < nr)
        {
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < xl; j++)
                {
                    data[i].push_back(x(j, 2 * i));
                    data[i].push_back(x(j, 2 * i + 1));
                }
                setCurveData(i, data[i]);
            }
        }
        else
        {
            for (int i = 0; i < n; i++)
            {
                for (int j = 0; j < yl; j++)
                {
                    data[i].push_back(x(2 * i, j));
                    data[i].push_back(x(2 * i + 1, j));
                }
                setCurveData(i, data[i]);
            }
        }
    }

    setDataRange();
    return;
}
//template<class T>
void QtCanvas::drawImage(const Matrix<uInt> &data, Matrix<uInt>
*mask)
{
    pMask = mask; 
    Int w = data.nrow();
    Int h = data.ncolumn();

    QImage im = QImage(w, h, QImage::Format_RGB32);
    uInt max = 0;
    uInt min = 100000000;
    for (int y=0; y<h; y++)
    {
        for (int x=0; x<w; x++)
        {
            if (min > data(x, y))
                min = data(x, y);
            if (max < data(x, y))
                max = data(x, y);
        }
    }
    uInt swap = 0;
    if (min > max)
    {
        swap = max;
        max = min;
        min = swap;
    }
    else if (min == max)
    {
        if (min > 0)
        {
            min--;
        }
        else
        {
            max++;
        }
    }
    swap = max - min;

    for (int y=0; y<h; y++)
    {
        for (int x=0; x<w; x++)
        {
            QRgb qrgb =   getLinearColor((double)data(x, y) / swap).rgb();
            im.setPixel(x, h - y - 1, qrgb);
            //std::cout << "r=" << qRed(qrgb) << " g=" << qGreen(qrgb) << " b="
            //<< qBlue(qrgb) << std::endl;
        }
    }

    setPixmap(im);
    //setDataRange();

    setImageMode(true);
    
    return;
}
void QtCanvas::drawImage(const Matrix<uInt> &data)
{
     pMask = 0;

    Int w = data.nrow();
    Int h = data.ncolumn();

    QImage im = QImage(w, h, QImage::Format_RGB32);
    uInt max = 0;
    uInt min = 100000000;
    for (int y=0; y<h; y++)
    {
        for (int x=0; x<w; x++)
        {
            if (min > data(x, y))
                min = data(x, y);
            if (max < data(x, y))
                max = data(x, y);
        }	
    }

    uInt swap = 0;
    if (min > max)
    {
        swap = max;
        max = min;
        min = swap;
    }
    else if (min == max)
    {
        if (min > 0)
        {
            min--;
        }
        else
        {
            max++;
        }
    }
    swap = max - min;
      
    for (int y=0; y<h; y++)
    {
        for (int x=0; x<w; x++)
        {
            QRgb qrgb =   getLinearColor((double)data(x, y) /
                                         swap).rgb();
            im.setPixel(x,  h - y - 1, qrgb);
            //std::cout << "r=" << qRed(qrgb) << " g=" << qGreen(qrgb) << "
            //b=" << qBlue(qrgb) << std::endl;
        }
    }

    setPixmap(im);
    //setDataRange();

    setImageMode(true);
    return;
}
void QtCanvas::setPixmap(const QImage &data)
{
    //std::cout << "pixmap w=" << pixmap.width() << " h=" << pixmap.height() <<
    //" pixmap isNull()=" << pixmap.isNull() << std::endl;
    backBuffer = QPixmap::fromImage(data);
    QtPlotSettings settings;
    settings.minX = 0;
    settings.maxX = backBuffer.width();
    settings.minY = 0;
    settings.maxY = backBuffer.height();
    settings.adjust();
    setPlotSettings(settings);
    //QImage img = backBuffer.toImage();
    //for (int i = 290; i < 300; i++) for (int j = 290; j < 300; j++)
    //std::cout << "x=" << i << " y=" << j << " pixmap data=" << hex <<
    //img.pixel(i, j) << dec << std::endl;
    refreshPixmap();
}
void QtCanvas::setImageMode(bool b)
{
    imageMode = b;
}

void QtCanvas::increaseCurZoom()
{
    curZoom++;
    
}
int QtCanvas::getCurZoom()
{
    return curZoom;
}
int QtCanvas::getZoomStackSize()
{
    return (int)zoomStack.size();
}
void QtCanvas::setTitle(const QString &text, int fontSize, double clr, const QString &font)
{ 
 	title.text = text;
 	title.fontName = font;
 	title.fontSize = fontSize;
 	title.color = getLinearColor(clr); 
}
void QtCanvas::setXLabel(const QString &text, int fontSize, double clr, const QString &font)
{ 
	xLabel.text = text;
 	xLabel.fontName = font;
 	xLabel.fontSize = fontSize;
 	xLabel.color = getLinearColor(clr); 
}
void QtCanvas::setYLabel(const QString &text, int fontSize, double clr, const QString &font)
{ 
	yLabel.text = text;
 	yLabel.fontName = font;
 	yLabel.fontSize = fontSize;
 	yLabel.color = getLinearColor(clr); 
} 
void QtCanvas::setWelcome(const QString &text, int fontSize, double clr, const QString &font)
{ 
 	welcome.text = text;
 	welcome.fontName = font;
 	welcome.fontSize = fontSize;
 	welcome.color = getLinearColor(clr); 
}
QPixmap* QtCanvas::graph()
{
   return &pixmap;
}

}
