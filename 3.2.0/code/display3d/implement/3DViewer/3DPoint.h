
#ifndef DP3_H
#define DP3_H

#include <graphics/X11/X_enter.h>

#include <graphics/X11/X_exit.h>


namespace casa {

class DisplayPoint3
{

public:
    DisplayPoint3(int a, int b, int c, int d = 0) {
      xx = a;
      yy = b;
      zz = c;
      vv = d;
    }
    DisplayPoint3(): xx(0), yy(0), zz(0), vv(0){}
    ~DisplayPoint3(){}

    void setX(int a) {xx = a;}
    void setY(int a) {yy = a;}
    void setZ(int a) {zz = a;}
    void setV(int a) {vv = a;}
    int x() {return xx;}
    int y() {return yy;}
    int z() {return zz;}
    int v() {return vv;}

private:
    //all use int, can be faster
    int xx;
    int yy;
    int zz;
    //use positive for color and negative for grey
    int vv;

};

}
#endif
