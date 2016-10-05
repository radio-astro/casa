
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
    DisplayData3(const casacore::String& = "");
    ~DisplayData3();

    void getCube(casacore::Cube<float>&, int& pol);
    static void getDefaultCube(casacore::Cube<float>&);

    void getAxes();
    void getStartEnd(casacore::IPosition& start, casacore::IPosition& end, 
                     casacore::IPosition& stride,
                     int freq = 0, int pol = 0);
private:

    casacore::ImageInterface<float>* im_;
    casacore::ImageInterface<casacore::Complex>* cim_;
    //DisplayData* dd_;
    casacore::IPosition ip;

    casacore::Cube<float> fileCube;
    casacore::Vector<int> axes;
    casacore::Vector<casacore::String> tAxes;

};

}
#endif
