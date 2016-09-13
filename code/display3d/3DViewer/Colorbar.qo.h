
#ifndef COLORBAR_H
#define COLORBAR_H

#include <graphics/X11/X_enter.h>
#include <QVector>
#include <graphics/X11/X_exit.h>

#include <display3d/3DViewer/3DDisplayData.qo.h>
#include <display/Display/Colormap.h>
#include <display3d/3DViewer/3DPoint.h>

namespace casa {

class Colorbar : public QWidget
{
    Q_OBJECT

public:
    Colorbar(QWidget *parent = 0);
    ~Colorbar();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

public slots:
    void setRange(float min, float max);
    void setMinimum(float min);
    void setMaximum(float min);
    void setNumColors(int nc);
    void setColormap(const QString& nm);
    void changeColor(int, float, float, const QString&);

signals:

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *);
    void paintEvent(QPaintEvent *event);
    void refreshBar();

    int numColors;
    QString cmapname;

    Vector<Float> rMap;
    Vector<Float> gMap;
    Vector<Float> bMap;

    void setColorTable();
    void calcLinearMaps();
    void getLinearColor(Float& r, Float& g, Float& b, Float v);
    void getTableColor(Float& r, Float& g, Float& b, Float v);
   
    float min;
    float max;
    
};

}
#endif
