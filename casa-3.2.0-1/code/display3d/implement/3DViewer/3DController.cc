
#include <graphics/X11/X_enter.h>
#include <QtGui>
#include <QtOpenGL>
#include <QMessageBox>
#include <math.h>
#include <graphics/X11/X_exit.h>

#include <display3d/3DViewer/3DController.qo.h>
#include <display3d/3DViewer/3DPoint.h>

namespace casa {

GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(parent), dd3(0), pList(300)//, cmap(0)
{
    object = 0;
    box = 0;
    xRot = -1;
    yRot = -1;
    zRot = -1;

    cx = 0.f;
    cy = 0.f;
    cz = 0.f;
    cd = 1.f;
    pol = 0;

    near = -12345;
    depth = 24680;
    trans = 35 * 16; 

    frameOn = false;

    numColors = 20;
    setColorTable("");
    //setSizePolicy(QSizePolicy::Expanding, 
    //              QSizePolicy::Expanding);

    bg = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0);
    zoom = 0;

    tips = "";
    menu = new QMenu;
    helpAct = new QAction(QString("help"), this);
    momentAct = new QAction(QString("moment"), this);
    profileAct = new QAction(QString("profile"), this);
    menu->addAction(helpAct);
    //menu->addAction(momentAct);
    //menu->addAction(profileAct);
    connect(helpAct, SIGNAL(triggered()), this, SLOT(showHelp()));

}

GLWidget::~GLWidget()
{
    makeCurrent();
    glDeleteLists(object, 1);
    glDeleteLists(box, 2);
}

const GLuint GLWidget::fogMode[] =
         { GL_EXP, GL_EXP2, GL_LINEAR };
const GLfloat GLWidget::fogColor[] = 
         {0.9f, 0.9f, 0.9f, 1.0f};	
        // {0.1f, 0.1f, 0.1f, 1.0f}; darker	
        // {0.5f, 0.5f, 0.5f, 1.0f};	
          

QSize GLWidget::minimumSizeHint() const
{
    return QSize(200, 200);
}


QSize GLWidget::sizeHint() const
{
    return QSize(440, 440);
}

void GLWidget::setXRotation(int angle)
{
    normalizeAngle(&angle);
    //qDebug() << "setXRot" << "angle" << angle << "xRot" << xRot;
    if (angle != xRot) {
        xRot = angle;
        emit xRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setYRotation(int angle)
{
    normalizeAngle(&angle);
    //qDebug() << "setYRot" << "angle" << angle << "yRot" << yRot;
    if (angle != yRot) {
        yRot = angle;
        emit yRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setZRotation(int angle)
{
    normalizeAngle(&angle);
    //qDebug() << "setZRot" << "angle" << angle << "zRot" << zRot;
    if (angle != zRot) {
        zRot = angle;
        emit zRotationChanged(angle);
        updateGL();
    }
}

void GLWidget::setZNear(int angle)
{
    //qDebug() << "setZNear" << "angle" << angle << "near" << near;
    if (angle != near) {
       near = angle;
       updatePosition();
       emit zNearChanged(angle);
       updateGL();
    }
}

void GLWidget::setZDepth(int angle)
{
    //qDebug() << "setZDepth" << "angle" << angle << "depth" << depth;
    if (angle != depth) {
       depth = max(angle, 1);
       //qDebug() << "depth" << depth;
       updatePosition();
       emit zDepthChanged(angle);
       updateGL();
    }
}

void GLWidget::setTrans(int angle)
{
    //qDebug() << "setTrans" << "angle" << angle << "trans" << trans;
    if (trans != angle) {
       trans = angle;
       object = makeObject();
       updatePosition();
       emit transChanged(angle);
       updateGL();
    }
}

void GLWidget::setColormap(const QString& newmap)
{
    setColorTable(newmap);
    object = makeObject(false);
    updatePosition();
    updateGL();
}

void GLWidget::setFrame(bool ck)
{
    frameOn = ck;
    updatePosition();
    updateGL();
}

void GLWidget::setFog(int ck)
{
    if (ck > 0) {
       glFogi(GL_FOG_MODE, fogMode[ck - 1]);
       glFogfv(GL_FOG_COLOR, fogColor);	
       glFogf(GL_FOG_DENSITY, 0.15f);
       glHint(GL_FOG_HINT, GL_DONT_CARE);
       glFogf(GL_FOG_START, -cd);
       glFogf(GL_FOG_END, cd);
       glEnable(GL_FOG);	
    }
    else {
       glDisable(GL_FOG);
    }
    updatePosition();
    updateGL();
}

void GLWidget::setLight(int ck)
{
   
    if (ck > 0) {
       /*
       GLfloat lightAmbient[] = {0.0f, 0.0001f, 0.0f, 0.01f};
       if (ck == 2) {
          lightAmbient[0] = 0.0001f;
          lightAmbient[1] = 0.0f;
          lightAmbient[2] = 0.0f;
       }
       if (ck == 3) {
          lightAmbient[0] = 0.0f;
          lightAmbient[1] = 0.0f;
          lightAmbient[2] = 0.0001f;
       }
       glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
       */

       /*
       GLfloat specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
       glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
       glMaterialf(GL_FRONT, GL_SHININESS, 0.1f);
       glNormal3f(0.0f, 0.0f, -1.0f);
       glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 0);
       */

       GLfloat lightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};    
       glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
       //glLightfv(GL_LIGHT1, GL_DIFFUSE, lightDiffuse);
       
       GLfloat lp = cz / (4 - ck);
       GLfloat lightPosition[] = {0.0f, 0.0f, lp, 1.0f};
       glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
       //lightPosition[2] = lp / 2.0f;
       //glLightfv(GL_LIGHT1, GL_POSITION, lightPosition);
       
       glEnable(GL_LIGHT0);
       //glEnable(GL_LIGHT1);
       glEnable(GL_LIGHTING);
    }
    else {
       glDisable(GL_LIGHTING);
       glDisable(GL_LIGHT0);
       //glDisable(GL_LIGHT1);
    }
    updatePosition();
    updateGL();
}

void GLWidget::setNumColors(int ck)
{
    if (ck != numColors) {
       numColors = ck;
       //qDebug() << cmapname << "numColors:" << numColors;
       setColorTable(cmapname);
       makeObject(false);
       updatePosition();
       updateGL();
    }
}

void GLWidget::setPol(const QString& fn, int ck)
{
   //set polarization and reload data
   if (ck != pol) {
      pol = ck;
      char* n = (char*)(fn.data());
      //qDebug() << n;
      setDD(n, pol); 
      makeObject();
      updatePosition();
      updateGL();
   }
}

void GLWidget::setZoom(int ck)
{
   zoom = ck; 
   //object = makeObject(false);
   updatePosition();
   updateGL();
   //set scale factor
}

void GLWidget::setBG(const QString& cl)
{
   if (cl == "White")
     bg = QColor(255, 255, 255, 255); 
   else if (cl == "Blue")
     bg = QColor::fromCmykF(0.39, 0.39, 0.0, 0.0).dark();
   else
     bg = QColor(0, 0, 0, 0).dark(); 

   object = makeObject(false);
   updatePosition();
   updateGL();

}

void GLWidget::showHelp()
{
    QMessageBox::information(this, "3D Viewer",
    //if want to show instead of exec
    //QWidget *wgt = new QWidget;
    //wgt->resize(430, 700);
    //QLabel *label = new QLabel(wgt);
    //label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    //label->setAlignment(Qt::AlignBottom | Qt::AlignRight);
    //label->setText(
      "How to rotate the data cube?\n"
      "   The cube is to rotate about its geometric center. The origin of\n"
      "the coordinate system is on the screen at the center of the display\n"
      "window. The X-axis is horizontal, Y-axis is vertical and Z-axis \n"
      "points away from your eyes. There are four ways to rotate the cube:\n"
      "   1. use rotation slider. Click the slider, each step is 10 degrees;\n"
      "      or drag the slider with arbitrary step size.\n"
      "   2. use keyboard. (First activate the slider by clicking or by walking\n"
      "      through with tab key.) Press arrow keys for small step, 1 degree; \n"
      "      or press pgUp/Dn keys for big step, 10 degrees.\n"
      "   3. use mouse in display window. Drag left mouse button to rotate\n"
      "      about XY or drag right mouse button to rotate about XZ.\n"  
      "   4. use rotation edit box. Enter any value between 0 and 360. This\n"
      "      is useful if you need precise position.\n"
      "\n"
      "How many colors to use?\n"
      "   Probably 20 is enough. Using more has no performance penalty \n"
      "but has no real advantage either. Move mouse cursor over the \n"
      "colorbar to readout the brightness valuses. Press left mouse button\n"
      "on display to get data information.\n"
      "\n"
      "What is the 'box' for?\n"
      "   If you rotate too much and feel disorientated, you can activate \n"
      "the box to help you find the North.\n"
      "\n"
      "What is the 'zoom'?\n"
      "   It scales the object bigger in 3 directions.\n"
      "\n"
      "What do 'fog' and 'light' do?\n"
      "   They can make the object look fuzzy so to help finding feature \n"
      "in the object (possibly).\n"
      "\n"
      "What is transparency?\n"
      "   It is defined as: 100*(CutOffValue-MinValue)/(MaxValue-MinValue).\n"
      "If the brightness is below the CutOffValue, the materials become \n"
      "invisible, but do not block the view. If you set the transparency to 0,\n"
      "the cube becomes a solid block, you can only look at its outer \n"
      "surface, and it is 'heavy' to rotate!\n"
      "\n"
      "What is Near and Depth?\n"
      "   The cube is confined in a bounding sphere. It can be further\n"
      "confined by two clip planes, the Near and the Far. The depth is\n"
      "the distance between Near and Far. The maximum depth is the \n"
      "diameter of the bounding sphere, with Near and Far at ends. The \n"
      "cube can be viewed layer by layer by keeping Depth fixed and \n"
      "adjusting Near. The Near and Depth can be controlled by slider, \n"
      "edit box, keyboard and by double clicking on the object.\n"
    );
   
  
    //wgt->show();

}


void GLWidget::initializeGL()
{
    qglClearColor(bg);

    object = makeObject();
    box = makeObject2();
    //qDebug() << "object=" << object ;

    glShadeModel(GL_FLAT);
    //glShadeModel(GL_SMOOTH);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glEnable(GL_POINT_SMOOTH);

    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void GLWidget::paintGL()
{
    //qDebug() << "paintGL---" ;
   
    qglClearColor(bg);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glLoadIdentity();
    //glTranslated(x, y, z) should generally go before
    //glRotaited(...) - when direction identity is known
     
    glRotated(xRot / 16.0, 1.0, 0.0, 0.0);
    glRotated(yRot / 16.0, 0.0, 1.0, 0.0);
    glRotated(zRot / 16.0, 0.0, 0.0, 1.0);
   
    if (frameOn) {
       glCallList(box);
    }

    GLfloat s = 1.0f + zoom * 0.1f; 
    glScalef(s, s, s);

    glCallList(object);

    //qDebug() << "object:" << object;
    //qDebug() << "xRot=" << xRot / 16.0
    //     << " yRot=" << yRot / 16.0
    //     << " zRot=" << zRot / 16.0;
    //qDebug() << "paint c:" << cx << cy << cz ;
    

}

void GLWidget::resizeGL(int width, int height)
{
    int side = qMin(width, height);
    glViewport((width - side) / 2, (height - side) / 2, side, side);
    //glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    //look from negative to positive z
    //setZNear and zetZDepth must call this resize
    //to adjust clip planes
    //qDebug() << "resize" << cd 
    //         << near / 16.0 << (near + depth) / 16.0;
    glOrtho(-cd, cd, -cd, cd, near / 16.0, (near + depth) / 16.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void GLWidget::updatePosition()
{

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    //qDebug() << "updatePosition" << cd 
    //         << (double)near / 16.0 << (double)(near + depth) / 16.0;
    glOrtho(-cd, cd, -cd, cd, near / 16.0, (near + depth) / 16.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    //qDebug() << "press---" ;
    lastPos = event->pos();
    if (event->button() == Qt::LeftButton){
       Int s1, s2, s3;
       data.shape(s1, s2, s3);
       Float f1, f2;    
       getMinMax(f1, f2);
       f1 = thresh; 
       QToolTip::showText(event->globalPos(), 
               QString("Shape: [")
               .append(QString::number(s1)).append(QString(", "))
               .append(QString::number(s2)).append(QString(", "))
               .append(QString::number(s3)).append(QString("]\n"))
               .append(QString("min:"))
               .append(QString::number(thresh)).append(QString(" "))
               .append(QString("max:"))
               .append(QString::number(f2)).append(QString(" "))
               .append(QString("points:"))
               .append(QString::number(pList.size())));
    }
    
    if (event->button() == Qt::RightButton &&
       event->x() < 100 && event->y() < 100) {
       menu->exec(QCursor::pos());
    }
                         
}

void GLWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    //qDebug() << "double---" ;
    if (event->button() == Qt::LeftButton){
      near += 8;
    }
    if (event->button() == Qt::RightButton){
      near -= 8;
    }
    
    //updatePosition();
    updateGL();

    emit zNearChanged(near);
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    //qDebug() << "move---" ;
    int dx = event->x() - lastPos.x();
    int dy = event->y() - lastPos.y();

    if (event->buttons() & Qt::LeftButton) {
        setXRotation(xRot + 8 * dy);
        setYRotation(yRot + 8 * dx);
    } else if (event->buttons() & Qt::RightButton) {
        setXRotation(xRot + 8 * dy);
        setZRotation(zRot + 8 * dx);
    }
    lastPos = event->pos();
}

void GLWidget::setDD(char* fn, int& pol)
{
    Bool ddOk = False;
    if (fn == 0) {
       qDebug() << "empty image file name";
    }
    else {
       try {
          if (!dd3) {
             dd3 = new DisplayData3(fn);
          }
          if (dd3) {
             dd3->getCube(data, pol);
             ddOk = True;
          }
       }
       catch (AipsError e) {
          qDebug() << e.getMesg().chars();
       }
    }
    if (!ddOk) {
       DisplayData3::getDefaultCube(data);
    }
    
    cd = getBound(); 
    near = -(int)(cd * 16);
    depth = (int)(2 * 16 * cd);

}

void GLWidget::getCubeSize(Int& s1, Int& s2, Int& s3)
{
   data.shape(s1, s2, s3);
}

void GLWidget::getMinMax(Float& min, Float& max)
{
   min = data(0, 0, 0);
   max = data(0, 0, 0);
   Int s1, s2, s3;
   data.shape(s1, s2, s3);
   for (Int i = 0; i < s1; i++) {
      for (Int j = 0; j < s2; j++) {
         for (Int k = 0; k < s3; k++) {
            min = fmin(min, data(i, j, k));
            max = fmax(max, data(i, j, k));
         }
      }
   }
}

Float GLWidget::getBound()
{
    Int xc, yc, zc;
    getCubeSize(xc, yc, zc);
    cx = xc / 2.0;
    cy = yc / 2.0;
    cz = zc / 2.0; 
    cd = sqrt(cx * cx  + cy * cy + cz * cz); 
    return cd;
}

void GLWidget::normalizeAngle(int *angle)
{
    while (*angle < 0)
        *angle += 360 * 16;
    while (*angle > 360 * 16)
        *angle -= 360 * 16;
}

GLuint GLWidget::makeObject2()
{

    Int s1;
    Int s2;
    Int s3;
    getCubeSize(s1, s2, s3);

    Float cx = (s1 - 1.f) / 2.f;
    Float cy = (s2 - 1.f) / 2.f;
    Float cz = (s3 - 1.f) / 2.f;
    //qDebug() << "center:" << cx << cy << cz;

    GLuint list = glGenLists(2);
    glNewList(list, GL_COMPILE);

    glPointSize(3.0);
    
    glBegin(GL_LINE_LOOP);
      glColor3f(0.0, 0.0, 1.0);
      glVertex3f( -cx, -cy, -cz);
      glVertex3f(  cx, -cy, -cz);
      glVertex3f(  cx,  cy, -cz);
      glVertex3f( -cx,  cy, -cz);
    glEnd();

    glBegin(GL_LINE_LOOP);
      glColor3f(1.0, 0.0, 0.0);
      glVertex3f( -cx, -cy, cz);
      glVertex3f(  cx, -cy, cz);
      glVertex3f(  cx,  cy, cz);
      glVertex3f( -cx,  cy, cz);
    glEnd();

    glBegin(GL_LINES);
      glColor3f(0.0, 0.0, 1.0);
      glVertex3f( -cx, -cy, -cz);
      glColor3f(0.0, 1.0, 0.0);
      glVertex3f( -cx, -cy, cz);

      glColor3f(0.0, 0.0, 1.0);
      glVertex3f(  cx, -cy, -cz);
      glColor3f(0.0, 1.0, 0.0);
      glVertex3f(  cx, -cy, cz);

      glColor3f(0.0, 0.0, 1.0);
      glVertex3f(  cx,  cy, -cz);
      glColor3f(0.0, 1.0, 0.0);
      glVertex3f(  cx,  cy, cz);

      glColor3f(0.0, 0.0, 1.0);
      glVertex3f( -cx,  cy, -cz);
      glColor3f(0.0, 1.0, 0.0);
      glVertex3f( -cx,  cy, cz);

    glEnd();
    
    glEndList();

    return list;
}

void GLWidget::calcLinearMaps(uInt reqSize, 
                    Vector<Float> &rMap,
                    Vector<Float> &gMap, Vector<Float> &bMap)
{
   if (reqSize < 1) {
      reqSize = 1;
   }
   rMap.resize(reqSize);
   gMap.resize(reqSize);
   bMap.resize(reqSize);
   if (reqSize == 1) {
      getLinearColor(rMap(0), gMap(0), bMap(0), 0); 
      return;
   }
   for (uInt k = 0; k < reqSize; k++) {
      getLinearColor(rMap(k), gMap(k), bMap(k), 
                     (float)k / (reqSize - 1));
   }
}

void GLWidget::getLinearColor(Float& r, Float& g, Float& b,
                              Float val, Float min, Float max)
{
    static const Float mid = 0.4;
    double len = max - min;
    if (len == 0) {
       len = 1.;
    }   
    double a = (val - min) / len;
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

void GLWidget::getTableColor(Float& r, Float& g, Float& b,
                             Float val, Float min, Float max)
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

void GLWidget::setColorTable(const QString& newmap)
{
    
    try {   
       Colormap cmap(newmap.toStdString());
       //cout << "cmap.rigid=" << cmap.rigid() << endl;
       cmap.calcRGBMaps(numColors, rMap, gMap, bMap);
       //cout << "colormap " << *cmap << endl;
       cmapname = newmap;
    }
    catch(...) {
       calcLinearMaps(numColors, rMap, gMap, bMap);
       cmapname = "Simple 1"; 
    }
    //cout << "rMap=" << rMap << " gMap=" << gMap
    //     << " bMap=" << bMap << endl;
}

GLuint GLWidget::makeObject(bool transChanged)
{
    Int s1;
    Int s2;
    Int s3;
    getCubeSize(s1, s2, s3);
  
    Float cx = (s1 - 1.f) / 2.f;
    Float cy = (s2 - 1.f) / 2.f;
    Float cz = (s3 - 1.f) / 2.f;
    //qDebug() << "center:" << cx << cy << cz;
   
    Float min;
    Float max;
    getMinMax(min, max);
    Float len = max - min;
    if (len == 0) {
       len = 1.;
    }   
   
    if (transChanged) {
       thresh = len * trans / 1600.0 + min; 
       GLfloat u = max - thresh;
       pList.clear();
       pList.reserve(1000);
       if (u == 0) {
         for (int k = 0; k < s3; k++){
           for (int i = 0; i < s1; i++){
             for (int j = 0; j < s2; j++){
               GLfloat d = data(i, j, k);
               if (d > thresh - 0.001) { 
                 int v = (int)(numColors - 1);
                 DisplayPoint3 a(i, j, k, v);
                 pList.append(a);
               }
             }
           }
         }
       }
       else {
         for (int k = 0; k < s3; k++){
           for (int i = 0; i < s1; i++){
             for (int j = 0; j < s2; j++){
               GLfloat d = data(i, j, k);
               if (d >= thresh) { 
                 int v = (int)(0.5 + 
                            (d - thresh) / u * (numColors - 1));
                 DisplayPoint3 a(i, j, k, v);
                 pList.append(a);
               }
             }
           }
         }
       }
       //qDebug() << "threshold:" << thresh
       //         << "num points:" << pList.size();
       //int m = -1;
       //for (int i = 0; i < pList.size(); ++i) {
       //  m = qMax(m, pList[i].v()) ;
       //}
       //qDebug() << "max color=" << m;
    }

    GLuint list = glGenLists(1);
    glNewList(list, GL_COMPILE);
    glBegin(GL_POINTS);
    DisplayPoint3 pt;
    glPointSize(3.0);
    for (int i = 0; i < pList.size(); ++i) {
       pt = pList[i];
       glColor3f(rMap(pt.v()), gMap(pt.v()), bMap(pt.v()));
       glVertex3f((GLfloat)pt.x() - cx, 
                  (GLfloat)pt.y() - cy, 
                  (GLfloat)pt.z() - cz);
       //if (pt.v() > numColors - 3)
       //  cout << pt.v() << " " << endl;
    }
    glEnd();
    glEndList();
    //glDeleteLists(object, 1);

    //qDebug() << "colorChanged" << numColors << thresh 
    //         << max << cmapname;
    emit colorChanged(numColors, thresh, max, cmapname);

    return list;
}
}
