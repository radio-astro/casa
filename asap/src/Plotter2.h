#include <cstdlib>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>

#include <cpgplot.h>

namespace asap {

class Plotter2TextInfo {
public:
    Plotter2TextInfo();
    ~Plotter2TextInfo();

    std::string text;
    float posx;
    float posy;
    float angle;
    float fjust;
    float size;
    int color;
    int bgcolor;
};

class Plotter2ArrowInfo {
public:
    Plotter2ArrowInfo();
    ~Plotter2ArrowInfo();

    float xhead;
    float xtail;
    float yhead;
    float ytail;

    int color;
    int width;
    int lineStyle;
    float headSize;
    int headFillStyle;
    float headAngle;
    float headVent;
};

class Plotter2RectInfo {
public: 
    Plotter2RectInfo();
    ~Plotter2RectInfo();

    float xmin;
    float xmax;
    float ymin;
    float ymax;

    int color;
    int fill;
    int width;
    float hsep;
};

class Plotter2DataInfo {
public:
    Plotter2DataInfo();
    ~Plotter2DataInfo();

    std::vector<float> xData;
    std::vector<float> yData;

    bool drawLine;
    int lineColor;
    int lineWidth;
    int lineStyle;

    bool drawMarker;
    int markerType;
    float markerSize;
    int markerColor;

    bool hasData;
};

class Plotter2ViewportInfo {
public:
    Plotter2ViewportInfo();
    ~Plotter2ViewportInfo();

    // show the whole viewport
    bool showViewport;

    // viewport position in window-coordinate
    float vpPosXMin;
    float vpPosXMax;
    float vpPosYMin;
    float vpPosYMax;

    // plotting range in world-coordinate
    float vpRangeXMin;
    float vpRangeXMax;
    float vpRangeYMin;
    float vpRangeYMax;

    // set plotting range automatic
    bool isAutoRangeX;
    bool isAutoRangeY;
    float autoRangeMarginX;
    float autoRangeMarginY;
    void adjustRange();
    std::vector<float> getRangeX();
    std::vector<float> getRangeY();

    // tick intervals
    bool isAutoTickIntervalX;
    bool isAutoTickIntervalY;
    float majorTickIntervalX;
    float majorTickIntervalY;
    int nMajorTickWithinTickNumsX;
    int nMajorTickWithinTickNumsY;
    int nMinorTickWithinMajorTicksX;
    int nMinorTickWithinMajorTicksY;
    void adjustTickInterval();

    // location of value strings along axes
    std::string numLocationX;
    std::string numLocationY;

    // default font size
    float fontSizeDef;

    // data to be plotted
    std::vector<Plotter2DataInfo> vData;
    void setData(const std::vector<float>& inXData, const std::vector<float>& inYData, const int id);

    // rectangles
    std::vector<Plotter2RectInfo> vRect;

    // arrows
    std::vector<Plotter2ArrowInfo> vArro;

    //annotations
    std::vector<Plotter2TextInfo> vText;

    // x-label
    std::string labelXString;
    float labelXPosX;
    float labelXPosY;
    float labelXAngle;
    float labelXFJust;
    float labelXSize;
    int labelXColor;
    int labelXBColor;

    // y-label
    std::string labelYString;
    float labelYPosX;
    float labelYPosY;
    float labelYAngle;
    float labelYFJust;
    float labelYSize;
    int labelYColor;
    int labelYBColor;

    // title
    std::string titleString;
    float titlePosX;
    float titlePosY;
    float titleAngle;
    float titleFJust;
    float titleSize;
    int titleColor;
    int titleBColor;

    // background colour
    int vpBColor;

    void getWorldCoordByWindowCoord(const float winX, const float winY, float* worldX, float* worldY);

private:
    float minXData;
    float maxXData;
    float minYData;
    float maxYData;
    bool hasDataRange;
    void updateXDataRange(const float data);
    void updateYDataRange(const float data);
    void updateAllDataRanges();
    void adjustRangeX(float* xmin, float* xmax);
    void adjustRangeY(float* ymin, float* ymax);
    void adjustTickIntervalX(const float xmin, const float xmax);
    void adjustTickIntervalY(const float ymin, const float ymax);
};

class Plotter2 {
public:
    Plotter2();
    ~Plotter2();

    std::string getFileName();
    void setFileName(const std::string& inFilename);
    std::string getDevice();
    void setDevice(const std::string& inDevice);

    float getViewSurfaceWidth();
    float getViewSurfaceAspect();
    void setViewSurface(const float width, const float aspect);
    int addViewport(const float xmin, const float xmax, const float ymin, const float ymax);
    void setViewport(const float xmin, const float xmax, const float ymin, const float ymax, const int id);
    void showViewport(const int inVpid);
    void hideViewport(const int inVpid);
    bool getHasDefaultViewport();
    int getCurrentViewportId();
    void getViewInfo();
    void setRange(const float xmin, const float xmax, const float ymin, const float ymax, const int inVpid);
    void setRangeX(const float xmin, const float xmax, const int inVpid);
    void setRangeY(const float ymin, const float ymax, const int inVpid);
    std::vector<float> getRangeX(const int inVpid);
    std::vector<float> getRangeY(const int inVpid);
    void setAutoRange(const int inVpid);
    void setAutoRangeX(const int inVpid);
    void setAutoRangeY(const int inVpid);
    void setFontSizeDef(const float size, const int inVpid);
    void setTicksX(const float interval, const int num, const int inVpid);
    void setTicksY(const float interval, const int num, const int inVpid);
    void setAutoTicks(const int inVpid);
    void setAutoTicksX(const int inVpid);
    void setAutoTicksY(const int inVpid);
    void setNumIntervalX(const float interval, const int inVpid);
    void setNumIntervalY(const float interval, const int inVpid);
    void setNumLocationX(const std::string& side, const int inVpid);
    void setNumLocationY(const std::string& side, const int inVpid);
    void setData(const std::vector<float>& xdata, const std::vector<float>& ydata, const int inVpid, const int inDataid);
    void setLine(const int color, const int width, const int style, const int inVpid, const int inDataid);
    void showLine(const int inVpid, const int inDataid);
    void hideLine(const int inVpid, const int inDataid);
    void setPoint(const int type, const float size, const int color, const int inVpid, const int inDataid);
    void showPoint(const int inVpid, const int inDataid);
    void hidePoint(const int inVpid, const int inDataid);
    void setMaskX(const float xmin, const float xmax, const int color, const int fill, const int width, const float hsep, const int inVpid);
    void setArrow(const float xtail, const float xhead, const float ytail, const float yhead, const int color, const int width, const int lineStyle, const float headSize, const int headFillStyle, const float headAngle, const float headVent, const int inVpid, const int inArrowid);
    void setText(const std::string& text, const float inPosx, const float inPosy, const float angle, const float fjust, const float size, const std::string& style, const int color, const int bgcolor, const int inVpid, const int inTextid);
    void setLabelX(const std::string& label, const float inPosx, const float inPosy, const float size, const std::string& style, const int color, const int bgcolor, const int inVpid);
    void setLabelY(const std::string& label, const float inPosx, const float inPosy, const float size, const std::string& style, const int color, const int bgcolor, const int inVpid);
    void setTitle(const std::string& label, const float inPosx, const float inPosy, const float size, const std::string& style, const int color, const int bgcolor, const int inVpid);
    void setViewportBackgroundColor(const int bgcolor, const int inVpid);
    void plot();
private:
    std::string filename;
    std::string device;
    bool hasDevice;
    std::vector<Plotter2ViewportInfo> vInfo;
    bool hasDefaultViewport;
    int currentViewportId;
    float width;
    float aspect;
    void open();
    void close();
    void resetAttributes(const Plotter2ViewportInfo& vi);
};

} // namespace asap
