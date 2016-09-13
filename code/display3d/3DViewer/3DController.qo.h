
#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <graphics/X11/X_enter.h>
#include <QGLWidget>
#include <QVector>
#include <QAction>
#include <QMenu>
#include <graphics/X11/X_exit.h>

#include <display3d/3DViewer/3DDisplayData.qo.h>
#include <display/Display/Colormap.h>
#include <display3d/3DViewer/3DPoint.h>

namespace casa {

class GLWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent = 0);
    ~GLWidget();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    void setDD(char*, int& pol);
    void getCubeSize(casacore::Int&, casacore::Int&, casacore::Int&);
    void getMinMax(casacore::Float&, casacore::Float&);
    casacore::Float getBound();

public slots:
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle); 

    void setZNear(int angle);
    void setZDepth(int angle);
    void setTrans(int angle); 

    void setColormap(const QString&);

    void updatePosition();

    void setFrame(bool);
    void setFog(int);
    void setLight(int);

    void setNumColors(int);
    void setPol(const QString&, int);

    void setBG(const QString&);
    void setZoom(int);

    void showHelp();

signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);

    void zNearChanged(int angle);
    void zDepthChanged(int angle);
    void transChanged(int angle);

    void resetZNear(double, double, double);
    void resetZDepth(double, double, double);
 
    void colorChanged(int, float, float, const QString&);

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

private:

    DisplayData3* dd3;
    casacore::Cube<casacore::Float> data;
    QVector<DisplayPoint3> pList;
    //Colormap* cmap;

    casacore::Vector<casacore::Float> rMap;
    casacore::Vector<casacore::Float> gMap;
    casacore::Vector<casacore::Float> bMap;
    int numColors;
    QString cmapname;

    void normalizeAngle(int *angle);
    void setColorTable(const QString& cmap = "");

    GLuint makeObject(bool transChanged = true);
    GLuint makeObject2();
    GLuint object;
    GLuint box;

    void calcLinearMaps(casacore::uInt numColors, casacore::Vector<casacore::Float>& r, 
                        casacore::Vector<casacore::Float>& g, casacore::Vector<casacore::Float>& b);

    void getLinearColor(casacore::Float& r, casacore::Float& g, casacore::Float& b, casacore::Float v,
                        casacore::Float min = 0., casacore::Float max = 1.);
    void getTableColor(casacore::Float& r, casacore::Float& g, casacore::Float& b, casacore::Float v, 
                       casacore::Float min = 0., casacore::Float max = 1.);
    GLfloat cx;
    GLfloat cy;
    GLfloat cz;
    GLfloat cd;
    int pol;

    GLfloat thresh;

    int near;
    int depth;
    int trans;

    int xRot;
    int yRot;
    int zRot;
    QPoint lastPos;

    bool frameOn;
    int zoom;
    QColor bg;

    GLuint filter;	

    static const GLuint fogMode[3];
    static const GLfloat fogColor[4];
    
    GLuint light;

    QString tips;

    QMenu* menu;
    QAction* helpAct;
    QAction* profileAct;
    QAction* momentAct;
   
};

}
#endif
