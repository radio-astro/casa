
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

    void getCube(casacore::Cube<casacore::Float>&, int& pol);
    static void getDefaultCube(casacore::Cube<casacore::Float>&);

    void getAxes();
    void getStartEnd(casacore::IPosition& start, casacore::IPosition& end, 
                     casacore::IPosition& stride,
                     casacore::Int freq = 0, casacore::Int pol = 0);
private:

    casacore::ImageInterface<casacore::Float>* im_;
    casacore::ImageInterface<casacore::Complex>* cim_;
    //DisplayData* dd_;
    casacore::IPosition ip;

    casacore::Cube<casacore::Float> fileCube;
    casacore::Vector<casacore::Int> axes;
    casacore::Vector<casacore::String> tAxes;

};

}
#endif
