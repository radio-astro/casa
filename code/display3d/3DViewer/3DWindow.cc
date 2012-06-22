
#include <graphics/X11/X_enter.h>
#include <QtGui>
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <graphics/X11/X_exit.h>

#include <display3d/3DViewer/3DController.qo.h>
#include <display3d/3DViewer/3DWindow.qo.h>
#include <display3d/3DViewer/3DDisplayData.qo.h>
#include <display3d/3DViewer/tqlayout.h>

namespace casa {



Window3D::Window3D(char* fn) : fname(fn), glWidget(0)
{
    //setFixedSize(600, 400);
    //3D display area
    glWidget = new GLWidget;
    //glWidget->setSizePolicy(QSizePolicy::MinimumExpanding,
    //                        QSizePolicy::MinimumExpanding);

    //create display data, grab the data cube
    int nPol = 0;
    glWidget->setDD(fn, nPol);

    //placeholder for control slides
    QWidget *pan = new QWidget;
    pan->setMinimumSize(160, 400);
    //pan->setSizePolicy(QSizePolicy::Minimum,
    //                   QSizePolicy::MinimumExpanding);
    QVBoxLayout *sideLayout = new QVBoxLayout;
    pan->setLayout(sideLayout);

    //anoth placeholder for control slides
    //QWidget *pot = new QWidget;
    //pot->setMinimumSize(130, 400);
    //pot->setSizePolicy(QSizePolicy::Minimum,
    //                   QSizePolicy::MinimumExpanding);
    //QVBoxLayout *leftLayout = new QVBoxLayout;
    //pot->setLayout(leftLayout);

    colorbar = new Colorbar;
    colorbar->setMaximumWidth(30);

    BorderLayout* mainLayout = new BorderLayout();
    mainLayout->addWidget(glWidget, BorderLayout::Center);
    mainLayout->addWidget(pan, BorderLayout::East);
    //mainLayout->addWidget(pot, BorderLayout::West);
    mainLayout->addWidget(colorbar, BorderLayout::West);
    //QHBoxLayout* mainLayout = new QHBoxLayout();
    //mainLayout->addWidget(glWidget);
    //mainLayout->addWidget(pan);
    //mainLayout->addWidget(pot);

    QVBoxLayout *upperLayout = new QVBoxLayout;
    sideLayout->addLayout(upperLayout);

    rotGroup = new QGroupBox("Rotation");
    QGridLayout *rotGrid = new QGridLayout;
    rotGrid->setSpacing(0);
    rotGroup->setLayout(rotGrid);

    xRa = new SlideEditor("xRa", 0, 360, 0);
    connect(xRa, SIGNAL(itemValueChanged(int)),
            glWidget, SLOT(setXRotation(int)));
    connect(glWidget, SIGNAL(xRotationChanged(int)), 
            xRa, SLOT(display2(int)));
    rotGrid->addWidget(new QLabel("X"), 0, 0, 1, 1);
    rotGrid->addWidget(xRa, 0, 1, 1, 11);

    xDec = new SlideEditor("xDec", 0, 360, 0);
    connect(xDec, SIGNAL(itemValueChanged(int)),
            glWidget, SLOT(setYRotation(int)));
    connect(glWidget, SIGNAL(yRotationChanged(int)), 
            xDec, SLOT(display2(int)));
    rotGrid->addWidget(new QLabel("Y"), 1, 0, 1, 1);
    rotGrid->addWidget(xDec, 1, 1, 1, 11);

    xVel = new SlideEditor("xVel", 0, 360, 15);
    connect(xVel, SIGNAL(itemValueChanged(int)),
            glWidget, SLOT(setZRotation(int)));
    connect(glWidget, SIGNAL(zRotationChanged(int)), 
            xVel, SLOT(display2(int)));
    rotGrid->addWidget(new QLabel("Z"), 2, 0, 1, 1);
    rotGrid->addWidget(xVel, 2, 1, 1, 11);

    upperLayout->addWidget(rotGroup);

    colorGroup = new QGroupBox("Look and Feel");
    QGridLayout *colorGrid = new QGridLayout;
    colorGrid->setSpacing(0);
    colorGroup->setLayout(colorGrid);

    numberC = new QLineEdit;
    numberC->setText(QString::number(20));
    numberC->setMaximumSize(30, 20);
    connect(numberC, SIGNAL(editingFinished()),
            this, SLOT(setNumColors()));
    //colorGrid->addWidget(new QLabel("Colors"));
    colorGrid->addWidget(numberC, 0, 0, 1, 1);
    colorGrid->addWidget(new QLabel("color"), 0, 1, 1, 1);

    cmap = new QComboBox;
    cmap->addItem("Simple");
    cmap->addItem("Hot Metal 1");
    cmap->addItem("Hot Metal 2");
    cmap->addItem("Greyscale 1");
    cmap->addItem("Greyscale 2");
    cmap->addItem("Rainbow 1");
    cmap->addItem("Rainbow 2");
    cmap->addItem("Rainbow 3");
    cmap->addItem("Rainbow 4");
    cmap->addItem("Smooth 1");
    cmap->addItem("Smooth 2");
    cmap->addItem("Smooth 3");
    cmap->addItem("Smooth 4");
    cmap->addItem("RGB 1");
    cmap->addItem("RGB 2");
    cmap->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    //cmap->addItem("Misc 1: isophotes");
    //cmap->addItem("Misc 2: Topograph");
    connect(cmap, SIGNAL(currentIndexChanged(const QString&)),
            glWidget, SLOT(setColormap(const QString&)));
    //colorGrid->addWidget(new QLabel("Colormap"));
    colorGrid->addWidget(cmap, 0, 2, 1, 4);

    fog = new QSpinBox;
    fog->setRange(0, 3);
    fog->setMaximumSize(30, 20);
    connect(fog, SIGNAL(valueChanged(int)),
            glWidget, SLOT(setFog(int)));
    colorGrid->addWidget(new QLabel("Fog"), 1, 0, 1, 1);
    colorGrid->addWidget(fog, 1, 1, 1, 1);

    light = new QSpinBox;
    light->setRange(0, 3);
    light->setMaximumSize(30, 20);
    connect(light, SIGNAL(valueChanged(int)),
            glWidget, SLOT(setLight(int)));
    colorGrid->addWidget(new QLabel("Light"), 1, 2, 1, 1);
    colorGrid->addWidget(light, 1, 3, 1, 1);

    frame = new QRadioButton("Box");
    frame->setMaximumSize(50, 20);
    connect(frame, SIGNAL(clicked(bool)),
            glWidget, SLOT(setFrame(bool)));
    colorGrid->addWidget(frame, 1, 4, 1, 2);

    bg = new QComboBox;
    bg->addItem("Blue");
    bg->addItem("Black");
    bg->addItem("White");
    connect(bg, SIGNAL(currentIndexChanged(const QString&)),
            glWidget, SLOT(setBG(const QString&)));
    colorGrid->addWidget(new QLabel("BG"), 2, 0, 1, 1);
    colorGrid->addWidget(bg, 2, 1, 1, 2);

    zoom = new QSpinBox;
    zoom->setMaximumSize(40, 20);
    zoom->setRange(0, 10);
    connect(zoom, SIGNAL(valueChanged(int)),
            glWidget, SLOT(setZoom(int)));
    colorGrid->addWidget(new QLabel("Zoom"), 2, 3, 1, 1);
    colorGrid->addWidget(zoom, 2, 4, 1, 1);

    upperLayout->addWidget(colorGroup);

    Int xs;
    Int ys;
    Int zs;
    glWidget->getCubeSize(xs, ys, zs);
    //qDebug() << "cube size:" << xs << ys << zs;

    Float bound = glWidget->getBound();
    //qDebug() << "bound:" << bound;

    Float min;
    Float max;
    glWidget->getMinMax(min, max);
    //qDebug() << "min:" << min << "max:" << max;
    
    dataGroup = new QGroupBox("Pol. and Transparency");
    QGridLayout *dataGrid = new QGridLayout;
    dataGrid->setSpacing(0);
    dataGroup->setLayout(dataGrid);

    pol = new QSpinBox;
    //qDebug() << "nPol:" << nPol;
    pol->setRange(0, nPol - 1);
    pol->setMaximumSize(30, 20);
    connect(pol, SIGNAL(valueChanged(int)),
            this, SLOT(setPol(int)));
    trans = new SlideEditor("trans", 0, 100, 35);
    connect(trans, SIGNAL(itemValueChanged(int)),
            glWidget, SLOT(setTrans(int)));
    connect(glWidget, SIGNAL(transChanged(int)), 
            trans, SLOT(display2(int)));
    //upperLayout->addWidget(new QLabel("Transparency"));
    dataGrid->addWidget(pol, 0, 0, 1, 1);
    dataGrid->addWidget(trans, 0, 1, 1, 4);
    upperLayout->addWidget(dataGroup);

    zNear = new SlideEditor("zNear", -bound, bound, -bound);
    connect(zNear, SIGNAL(itemValueChanged(int)),
            glWidget, SLOT(setZNear(int)));
    connect(glWidget, SIGNAL(zNearChanged(int)), 
            zNear, SLOT(display2(int)));
    dataGrid->addWidget(new QLabel("Near"), 1, 0, 1, 1);
    dataGrid->addWidget(zNear, 1, 1, 1, 4);

    zDepth = new SlideEditor("zDepth", 0, 2 * bound, 2 * bound);
    connect(zDepth, SIGNAL(itemValueChanged(int)),
            glWidget, SLOT(setZDepth(int)));
    connect(glWidget, SIGNAL(zDepthChanged(int)), 
            zDepth, SLOT(display2(int)));
    dataGrid->addWidget(new QLabel("Depth"), 2, 0, 1, 1);
    dataGrid->addWidget(zDepth, 2, 1, 1, 4);


    QSpacerItem *spacerItem = new QSpacerItem(
        131, 31, QSizePolicy::Minimum, QSizePolicy::Expanding);
    upperLayout->addItem(spacerItem);

    /* set initial position this way does not work!
       it gave a black window of bare widget
       any of the following will cause trouble!
    glWidget->setXRotation(15 * 16);
    glWidget->setYRotation(345 * 16);
    glWidget->setZRotation(0 * 16);
    glWidget->setZNear(1 - (int)(bound * 16));
    glWidget->setZDepth((int)(2 * bound * 16) - 1);
    glWidget->setColormap("Hot Metal 1");
    */

    /*
    QVBoxLayout *lowerLayout = new QVBoxLayout;
    leftLayout->addLayout(lowerLayout);

    QSpacerItem *lspacerItem = new QSpacerItem(
        131, 31, QSizePolicy::Minimum, QSizePolicy::Expanding);
    lowerLayout->addItem(lspacerItem);
    */

    connect(glWidget, 
            SIGNAL(colorChanged(int, float, float, const QString&)),
            colorbar, 
            SLOT(changeColor(int, float, float, const QString&)));

    setLayout(mainLayout);
    setWindowTitle(QString("3D View - ").append(fname));

}

void Window3D::setNumColors() {
    QString nm = numberC->text();
    bool ok;
    int val = nm.toInt(&ok);
    if (!ok || val < 1) {
       val = 30;
       numberC->setText(QString::number(30));
    }
    glWidget->setNumColors(val);
   

}

void Window3D::setPol(int pol) {
    glWidget->setPol(fname, pol);
}

}
