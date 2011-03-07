
#ifndef WINDOW3D_H
#define WINDOW3D_H

#include <graphics/X11/X_enter.h>
#include <QWidget>
#include <QSlider>
#include <QComboBox>
#include <QRadioButton>
#include <QSpinBox>
#include <QGroupBox>
#include <graphics/X11/X_exit.h>

#include <display3d/3DViewer/3DController.qo.h>
#include <display3d/3DViewer/SlideEditor.qo.h>
#include <display3d/3DViewer/Colorbar.qo.h>

namespace casa { 

class Window3D : public QWidget
{
    Q_OBJECT

public:

    Window3D(char* fn = 0);

public slots:
    void setNumColors();
    void setPol(int);

private:

    QGroupBox* rotGroup;
    SlideEditor* xRa;
    SlideEditor* xDec;
    SlideEditor* xVel;
    SlideEditor* zNear;
    SlideEditor* zDepth;

    QGroupBox* dataGroup;
    SlideEditor* trans;
    QSpinBox* pol;

    QGroupBox* colorGroup;
    QLineEdit* numberC;
    QComboBox* cmap;
    //QGroupBox* otherGroup;
    QRadioButton* frame;
    QSpinBox* fog;
    QSpinBox* light;

    QComboBox* bg;
    QSpinBox* zoom;

    QString fname;
    GLWidget *glWidget;

    Colorbar* colorbar; 

};

}
#endif
