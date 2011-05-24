
#ifndef DD3_H
#define DD3_H

#include <graphics/X11/X_enter.h>
#include <QGLWidget>
#include <graphics/X11/X_exit.h>

#include <display/DisplayDatas/DisplayData.h>
#include <images/Images/ImageInterface.h>
#include <casa/Arrays/Cube.h>

namespace casa {


class DisplayData3 : public QGLWidget
{
    Q_OBJECT

public:
    DisplayData3(const String& = "");
    ~DisplayData3();

    void getCube(Cube<Float>&, int& pol);
    static void getDefaultCube(Cube<Float>&);

    void getAxes();
    void getStartEnd(IPosition& start, IPosition& end, 
                     IPosition& stride,
                     Int freq = 0, Int pol = 0);
private:

    ImageInterface<Float>* im_;
    ImageInterface<Complex>* cim_;
    //DisplayData* dd_;
    IPosition ip;

    Cube<Float> fileCube;
    Vector<Int> axes;
    Vector<String> tAxes;

};

}
#endif
