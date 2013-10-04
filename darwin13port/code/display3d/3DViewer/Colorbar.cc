
#include <graphics/X11/X_enter.h>
#include <QtGui>
#include <math.h>
#include <graphics/X11/X_exit.h>

#include <display3d/3DViewer/3DController.qo.h>
#include <display3d/3DViewer/3DPoint.h>
#include <display3d/3DViewer/Colorbar.qo.h>

namespace casa {

Colorbar::Colorbar(QWidget *parent)
{
    numColors = 20;
    cmapname = "Simple";
    setMouseTracking(true);
}

Colorbar::~Colorbar()
{
}

QSize Colorbar::minimumSizeHint() const
{
    return QSize(200, 200);
}


QSize Colorbar::sizeHint() const
{
    return QSize(440, 440);
}

void Colorbar::resizeEvent(QResizeEvent *)
{
    //qDebug() << "resize";
    repaint();
}

void Colorbar::refreshBar()
{
   QPainter painter(this);
   int Margin = 5;
   int w = width() - 2 * Margin;
   int h = height() - 2 * Margin; 
   QRect rect;

   int s = qMax((int)rMap.nelements(), 1);
   int t = (int)(h / (float)s);
   for (int i = 0; i < s; i++) {
      painter.setBrush(QColor((int)(255 * rMap(i)), (int)(255 * gMap(i)), 
                              (int)(255 * bMap(i))));
      int b = height() - Margin - (i + 1) * t;
      rect.setRect(Margin, b, w, t);
      painter.drawRect(rect); 
   } 
   //qDebug() << "refresh";
   //update();
}
void Colorbar::paintEvent(QPaintEvent *event)
{
    //qDebug() << "paint";
    refreshBar();
}

void Colorbar::changeColor(int nc, float mi, float ma, 
                           const QString& mapname)
{
    numColors = nc;
    cmapname = mapname;
    min = mi;
    max = ma;
    //qDebug() << "changeColor" << nc << min << max << mapname;
    setColorTable();
    repaint();
    //cout << rMap << gMap << bMap << endl;
}

void Colorbar::setNumColors(int ck)
{
    if (ck != numColors) {
       numColors = ck;
       qDebug() << cmapname << "numColors:" << numColors;
    }
}


void Colorbar::mousePressEvent(QMouseEvent *event)
{
    //qDebug() << "press---" ;
    int Margin = 5;
    int h = height() - 2 * Margin; 
    
    int s = qMax((int)rMap.nelements(), 1);
    int t = (int)(h / (float)s);
    int len = s * t;
    int pos = height() - Margin - event->y(); 
    if (pos < 0 || pos > len)
       return;
    float v = min + (max - min) * pos / len;
    //qDebug() << v; 
    QToolTip::showText(event->globalPos(), QString::number(v));
    //QToolTip::hideText();
}

void Colorbar::mouseDoubleClickEvent(QMouseEvent *event)
{
    //qDebug() << "double---" ;
}

void Colorbar::mouseMoveEvent(QMouseEvent *event)
{
    //if (event->buttons() & Qt::LeftButton) {
    //  ;
    //}
    int Margin = 5;
    int h = height() - 2 * Margin; 
    
    int s = qMax((int)rMap.nelements(), 1);
    int t = (int)(h / (float)s);
    int len = s * t;
    int pos = height() - Margin - event->y(); 
    if (pos < 0 || pos > len)
       return;
    float v = min + (max - min) * pos / len;
    //qDebug() << v; 
    QToolTip::showText(event->globalPos(), QString::number(v));
}

void Colorbar::calcLinearMaps()
{
   if (numColors < 1) {
      numColors = 1;
   }
   rMap.resize(numColors);
   gMap.resize(numColors);
   bMap.resize(numColors);
   if (numColors == 1) {
      getLinearColor(rMap(0), gMap(0), bMap(0), 0);
      return;
   }
   for (int k = 0; k < numColors; k++) {
      getLinearColor(rMap(k), gMap(k), bMap(k),
                     (float)k / (numColors - 1) );
   }
}

void Colorbar::getLinearColor(Float& r, Float& g, Float& b, Float a)
{
    if (a < 0) a = 0;
    if (a > 1) a = 1;
    static const Float mid = 0.4;
    if (a > mid) {
       r = 0.;
       g = (1. - a) / (1. - mid);
       b = (a - mid) / (1. - mid);
    }
    else {
       r = (mid - a) / mid;
       g = a / mid;
       b = 0.;
    }
}


void Colorbar::getTableColor(Float& r, Float& g, Float& b, Float val)
{
    //this function has too many temporaries that should
    //slow the process. rewrite it later to pick value from table
    double len = max - min;
    if (len == 0) {
       len = 1.;
    }
    double a = (val - min) / len * (numColors - 1);
    int c = (int)ceil(a);
    int d = (int)floor(a);
    double e = a - d;
    //qDebug() << a << d << c << e;
    if (c != d) {
       r = rMap(d) + e * (rMap(c) - rMap(d));
       r = rMap(d) + e * (rMap(c) - rMap(d));
       r = rMap(d) + e * (rMap(c) - rMap(d));
    }
    else {
       r = rMap(d);
       g = gMap(d);
       b = bMap(d);
    }
}

void Colorbar::setColorTable()
{

    try {
       Colormap cmap(cmapname.toStdString());
       //cout << "cmap.rigid=" << cmap.rigid() << endl;
       cmap.calcRGBMaps(numColors, rMap, gMap, bMap);
       //cout << "colormap " << *cmap << endl;
       //qDebug() << cmapname << numColors;
    }
    catch(...) {
       calcLinearMaps();
       cmapname = "Simple 1";
       //qDebug() << cmapname << numColors;
    }
    //cout << "rMap=" << rMap << " gMap=" << gMap
    //     << " bMap=" << bMap << endl;
}

void Colorbar::setRange(float mi, float ma)
{   
    min = mi;
    max = ma;
}

void Colorbar::setMinimum(float mi)
{
    min = mi;
}

void Colorbar::setMaximum(float ma)
{
    max = ma;
}

void Colorbar::setColormap(const QString& nm)
{

};







}
