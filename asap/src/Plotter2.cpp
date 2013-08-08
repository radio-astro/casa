#include "Plotter2.h"

namespace asap {

Plotter2RectInfo::Plotter2RectInfo() {
    xmin = 0.0;
    xmax = 1.0;
    ymin = 0.0;
    ymax = 1.0;
    color = 15; // gray
    fill  = 4;  // hatch
    width = 1;
}

Plotter2RectInfo::~Plotter2RectInfo() {
}

Plotter2DataInfo::Plotter2DataInfo() {
    xData.clear();
    yData.clear();

    drawLine  = true;
    lineStyle = 1;       // solid line
    lineWidth = 1;
    lineColor = -1;      // undefined (default color should be assigned)

    drawMarker  = false;
    markerType  = 20;    // small circle
    markerSize  = 1.0;
    markerColor = 1;     // default foreground color (black)

    hasData = false;     // has no data
}

Plotter2DataInfo::~Plotter2DataInfo() {
}

Plotter2ViewportInfo::Plotter2ViewportInfo() {
    showViewport = true;

    vpPosXMin = 0.1;
    vpPosXMax = 0.9;
    vpPosYMin = 0.1;
    vpPosYMax = 0.9;

    vpRangeXMin = 0.0;
    vpRangeXMax = 1.0;
    vpRangeYMin = 0.0;
    vpRangeYMax = 1.0;
    isAutoRangeX = true;
    isAutoRangeY = true;
    autoRangeMarginX = 0.0;
    autoRangeMarginY = 0.1;

    hasDataRange = false;

    nMajorTickWithinTickNumsX = 2;
    nMajorTickWithinTickNumsY = 2;
    isAutoTickIntervalX = true;
    isAutoTickIntervalY = true;

    numLocationX = "l";
    numLocationY = "b";

    fontSizeDef = 1.0;

    vpBColor = -1; // transparent (<0)

    vData.clear();
}

Plotter2ViewportInfo::~Plotter2ViewportInfo() {
    vData.clear();
}

void Plotter2ViewportInfo::adjustRange() {
    if (hasDataRange) {
        if (isAutoRangeX) {
	    adjustRangeX(&vpRangeXMin, &vpRangeXMax);
	}
	if (isAutoRangeY) {
	    adjustRangeY(&vpRangeYMin, &vpRangeYMax);
	}
    }
}

void Plotter2ViewportInfo::adjustRangeX(float* xmin, float* xmax) {
    float xmargin = (maxXData - minXData) * autoRangeMarginX;
    *xmin = minXData - xmargin;
    *xmax = maxXData + xmargin;
}

void Plotter2ViewportInfo::adjustRangeY(float* ymin, float* ymax) {
    float ymargin = (maxYData - minYData) * autoRangeMarginY;
    *ymin = minYData - ymargin;
    *ymax = maxYData + ymargin;
}

std::vector<float> Plotter2ViewportInfo::getRangeX() {
    float minX, maxX;

    if (isAutoRangeX) {
        adjustRangeX(&minX, &maxX);
    } else {
        minX = vpRangeXMin;
	maxX = vpRangeXMax;
    }

    std::vector<float> res;
    res.clear();
    res.push_back(minX);
    res.push_back(maxX);

    return res;
}

std::vector<float> Plotter2ViewportInfo::getRangeY() {
    float minY, maxY;

    if (isAutoRangeY) {
        adjustRangeY(&minY, &maxY);
    } else {
        minY = vpRangeYMin;
	maxY = vpRangeYMax;
    }

    std::vector<float> res;
    res.clear();
    res.push_back(minY);
    res.push_back(maxY);

    return res;
}

void Plotter2ViewportInfo::adjustTickInterval() {
    if (hasDataRange) {
	if (isAutoTickIntervalX) {
	    adjustTickIntervalX();
	}
	if (isAutoTickIntervalY) {
	    adjustTickIntervalY();
	}
    }
}

void Plotter2ViewportInfo::adjustTickIntervalX() {
    adjustTickIntervalX(vpRangeXMin, vpRangeXMax);
}

void Plotter2ViewportInfo::adjustTickIntervalX(const float xmin, const float xmax) {
    majorTickIntervalX = (float)pow(10.0, ceil(log10((xmax - xmin)/10.0)));
    if ((xmax - xmin) / majorTickIntervalX < 2.0) {
        majorTickIntervalX /= 10.0;
    }
    nMinorTickWithinMajorTicksX = 5;
}

void Plotter2ViewportInfo::adjustTickIntervalY() {
    adjustTickIntervalY(vpRangeYMin, vpRangeYMax);
}

void Plotter2ViewportInfo::adjustTickIntervalY(const float ymin, const float ymax) {
    majorTickIntervalY = (float)pow(10.0, ceil(log10((ymax - ymin)/10.0)));
    if ((ymax - ymin) / majorTickIntervalY < 2.0) {
        majorTickIntervalY /= 10.0;
    }
    nMinorTickWithinMajorTicksY = 5;
}

void Plotter2ViewportInfo::setData(const std::vector<float>& inXData, const std::vector<float>& inYData, const int id) {
    if (!hasDataRange) {
        minXData = inXData[0];
	maxXData = inXData[0];
	minYData = inYData[0];
	maxYData = inYData[0];

        hasDataRange = true;
    }

    Plotter2DataInfo* info = &vData[id];

    info->xData.clear();
    info->xData.reserve(inXData.size());
    for (unsigned int i = 0; i < inXData.size(); ++i) {
        info->xData.push_back(inXData[i]);

	if (!info->hasData) {
	    updateXDataRange(inXData[i]);
	}
    }

    info->yData.clear();
    info->yData.reserve(inYData.size());
    for (unsigned int i = 0; i < inYData.size(); ++i) {
        info->yData.push_back(inYData[i]);

	if (!info->hasData) {
  	    updateYDataRange(inYData[i]);
	}
    }

    if (info->hasData) {
        updateAllDataRanges();
    } else {
        info->hasData = true;
    }
}

void Plotter2ViewportInfo::updateXDataRange(const float data) {
    if (data < minXData) {
  	minXData = data;
    }
    if (maxXData < data) {
	maxXData = data;
    }
}

void Plotter2ViewportInfo::updateYDataRange(const float data) {
    if (data < minYData) {
  	minYData = data;
    }
    if (maxYData < data) {
  	maxYData = data;
    }
}

void Plotter2ViewportInfo::updateAllDataRanges() {
    minXData = vData[0].xData[0];
    maxXData = minXData;
    minYData = vData[0].yData[0];
    maxYData = minYData;

    for (unsigned int i = 0; i < vData.size(); ++i) {
        for (unsigned int j = 0; j < vData[i].xData.size(); ++j) {
            updateXDataRange(vData[i].xData[j]);
            updateYDataRange(vData[i].yData[j]);
	}
    }
}

void Plotter2ViewportInfo::getWorldCoordByWindowCoord(const float winX, const float winY, float* worldX, float* worldY) {
    float xratio = (winX - vpPosXMin) / (vpPosXMax - vpPosXMin);
    if (winX < 0.0) {
        xratio = 0.5;
    }
    *worldX = vpRangeXMin + xratio * (vpRangeXMax - vpRangeXMin);
    float yratio = (winY - vpPosYMin) / (vpPosYMax - vpPosYMin);
    if (winY < 0.0) {
        yratio = 0.5;
    }
    *worldY = vpRangeYMin + yratio * (vpRangeYMax - vpRangeYMin);
}

Plotter2::Plotter2() {
    filename = "";
    device = "xwindow";
    hasDevice = false;

    vInfo.clear();
    Plotter2ViewportInfo vi;
    vInfo.push_back(vi);

    hasDefaultViewport = true;
    currentViewportId = 0;
}

Plotter2::~Plotter2() {
    close();
    vInfo.clear();
}

std::string Plotter2::getFileName() {
    return filename;
}

void Plotter2::setFileName(const std::string& inFilename) {
    filename = inFilename;
}

std::string Plotter2::getDevice() {
    return device;
}

void Plotter2::setDevice(const std::string& inDevice) {
    device = inDevice;
}

void Plotter2::open() {
    cpgopen((filename + "/" + device).c_str());
    hasDevice = true;
}

int Plotter2::addViewport(const float xmin, const float xmax, const float ymin, const float ymax) {
    Plotter2ViewportInfo vi;

    vi.vpPosXMin = xmin;
    vi.vpPosXMax = xmax;
    vi.vpPosYMin = ymin;
    vi.vpPosYMax = ymax;

    vInfo.push_back(vi);
    currentViewportId = vInfo.size() - 1;

    return currentViewportId;
}

void Plotter2::setViewport(const float xmin, const float xmax, const float ymin, const float ymax, const int id) {
    Plotter2ViewportInfo* vi = &vInfo[id];

    vi->vpPosXMin = xmin;
    vi->vpPosXMax = xmax;
    vi->vpPosYMin = ymin;
    vi->vpPosYMax = ymax;

    hasDefaultViewport = false;
}

void Plotter2::showViewport(const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        exit(0);
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->showViewport = true;
}

void Plotter2::hideViewport(const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        exit(0);
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->showViewport = false;
}

bool Plotter2::getHasDefaultViewport() {
    return hasDefaultViewport;
}

int Plotter2::getCurrentViewportId() {
  return currentViewportId;
}

void Plotter2::getViewInfo() {
    std::cout << "===================" << std::endl << std::flush;
    for (unsigned int i = 0; i < vInfo.size(); ++i) {
        std::cout << "[" << i << "]  " << std::endl;
        std::cout << "vpPos:  ";
	std::cout << "xmin=" << vInfo[i].vpPosXMin << ", ";
	std::cout << "xmax=" << vInfo[i].vpPosXMax << ", ";
	std::cout << "ymin=" << vInfo[i].vpPosYMin << ", ";
	std::cout << "ymax=" << vInfo[i].vpPosYMax << std::endl;

        std::cout << "vpRange:  ";
	std::cout << "xmin=" << vInfo[i].vpRangeXMin << ", ";
	std::cout << "xmax=" << vInfo[i].vpRangeXMax << ", ";
	std::cout << "ymin=" << vInfo[i].vpRangeYMin << ", ";
	std::cout << "ymax=" << vInfo[i].vpRangeYMax << std::endl;;

	std::cout << "vdatasize=" << vInfo[i].vData.size() << std::endl;
        for (unsigned int j = 0; j < vInfo[i].vData.size(); ++j) {
	    std::cout << "vdataxdatasize=" << vInfo[i].vData[j].xData.size() << ", ";
  	    for (unsigned int k = 0; k < vInfo[i].vData[j].xData.size(); ++k) {
	        std::cout << "(" << vInfo[i].vData[j].xData[k] << ", ";
	        std::cout << vInfo[i].vData[j].yData[k] << ") ";
	    }
            std::cout << std::endl;
	}
        std::cout << "===================" << std::endl << std::flush;
    }
}

void Plotter2::setRange(const float xmin, const float xmax, const float ymin, const float ymax, const int inVpid) {
    setRangeX(xmin, xmax, inVpid);
    setRangeY(ymin, ymax, inVpid);
}

void Plotter2::setRangeX(const float xmin, const float xmax, const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->vpRangeXMin = xmin;
    vi->vpRangeXMax = xmax;
    vi->isAutoRangeX = false;
}

void Plotter2::setRangeY(const float ymin, const float ymax, const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->vpRangeYMin = ymin;
    vi->vpRangeYMax = ymax;
    vi->isAutoRangeY = false;
}

std::vector<float> Plotter2::getRangeX(const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        exit(0);
    }

    return vInfo[vpid].getRangeX();
}

std::vector<float> Plotter2::getRangeY(const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        exit(0);
    }

    return vInfo[vpid].getRangeY();
}

void Plotter2::setAutoRange(const int inVpid) {
    setAutoRangeX(inVpid);
    setAutoRangeY(inVpid);
}

void Plotter2::setAutoRangeX(const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->isAutoRangeX = true;
}

void Plotter2::setAutoRangeY(const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->isAutoRangeY = true;
}

void Plotter2::setFontSizeDef(const float size, const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->fontSizeDef = size;
}

void Plotter2::setTicksX(const float interval, const int num, const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->majorTickIntervalX = interval;
    vi->nMinorTickWithinMajorTicksX = num;
    vi->isAutoTickIntervalX = false;
}

void Plotter2::setTicksY(const float interval, const int num, const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->majorTickIntervalY = interval;
    vi->nMinorTickWithinMajorTicksY = num;
    vi->isAutoTickIntervalY = false;
}

void Plotter2::setAutoTicks(const int inVpid) {
    setAutoTicksX(inVpid);
    setAutoTicksY(inVpid);
}

void Plotter2::setAutoTicksX(const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->isAutoTickIntervalX = true;
}

void Plotter2::setAutoTicksY(const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->isAutoTickIntervalY = true;
}

void Plotter2::setNumIntervalX(const float interval, const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->nMajorTickWithinTickNumsX = (int)(interval / vi->majorTickIntervalX);
}

void Plotter2::setNumIntervalY(const float interval, const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->nMajorTickWithinTickNumsY = (int)(interval / vi->majorTickIntervalY);
}

void Plotter2::setNumLocationX(const std::string& side, const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->numLocationX = side;
}

void Plotter2::setNumLocationY(const std::string& side, const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->numLocationY = side;
}

void Plotter2::setData(const std::vector<float>& xdata, const std::vector<float>& ydata, const int inVpid, const int inDataid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];

    int dataid = inDataid;
    if (dataid < 0) {
        Plotter2DataInfo di;
        vi->vData.push_back(di);
        dataid = vi->vData.size() - 1;
    }

    vi->setData(xdata, ydata, dataid);
}

void Plotter2::setLine(const int color, const int width, const int style, const int inVpid, const int inDataid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    int dataid = inDataid;
    if (dataid < 0) {
	dataid = vInfo[vpid].vData.size() - 1;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->vData[dataid].drawLine  = true;
    vi->vData[dataid].lineColor = color;
    vi->vData[dataid].lineWidth = width;
    vi->vData[dataid].lineStyle = style;
}

void Plotter2::showLine(const int inVpid, const int inDataid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        exit(0);
    }

    int dataid = inDataid;
    if (dataid < 0) {
	dataid = vInfo[vpid].vData.size() - 1;
    }
    if (dataid < 0) {
        exit(0);
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->vData[dataid].drawLine  = true;
}

void Plotter2::hideLine(const int inVpid, const int inDataid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        exit(0);
    }

    int dataid = inDataid;
    if (dataid < 0) {
	dataid = vInfo[vpid].vData.size() - 1;
    }
    if (dataid < 0) {
        exit(0);
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->vData[dataid].drawLine  = false;
}

void Plotter2::setPoint(const int type, const float size, const int color, const int inVpid, const int inDataid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    int dataid = inDataid;
    if (dataid < 0) {
	dataid = vInfo[vpid].vData.size() - 1;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->vData[dataid].drawMarker  = true;
    vi->vData[dataid].markerType  = type;
    vi->vData[dataid].markerSize  = size;
    vi->vData[dataid].markerColor = color;
}

void Plotter2::showPoint(const int inVpid, const int inDataid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        exit(0);
    }

    int dataid = inDataid;
    if (dataid < 0) {
	dataid = vInfo[vpid].vData.size() - 1;
    }
    if (dataid < 0) {
        exit(0);
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->vData[dataid].drawMarker  = true;
}

void Plotter2::hidePoint(const int inVpid, const int inDataid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        exit(0);
    }

    int dataid = inDataid;
    if (dataid < 0) {
	dataid = vInfo[vpid].vData.size() - 1;
    }
    if (dataid < 0) {
        exit(0);
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->vData[dataid].drawMarker  = false;
}

void Plotter2::setMaskX(const float xmin, const float xmax, const int color, const int fill, const int width, const float hsep, const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];

    Plotter2RectInfo ri;
    ri.xmin  = xmin;
    ri.xmax  = xmax;
    std::vector<float> yrange = vi->getRangeY();
    float yexcess = 0.1*(yrange[1] - yrange[0]);
    ri.ymin  = yrange[0] - yexcess;
    ri.ymax  = yrange[1] + yexcess;
    ri.color = color;
    ri.fill  = fill;
    ri.width = width;
    ri.hsep  = hsep;

    vi->vRect.push_back(ri);
}

void Plotter2::setLabelX(const std::string& label, const float inPosx, const float inPosy, const float size, const std::string& style, const int color, const int bgcolor, const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];

    std::string styleString;
    if (style == "") {
      styleString = "";
    } else if (style == "roman") {
      styleString = "\\fr";
    } else if (style == "italic") {
      styleString = "\\fi";
    } else if (style == "script") {
      styleString = "\\fs";
    }
    vi->labelXString = styleString + label;

    float posx = inPosx;
    if (posx < 0.0) {
        posx = 0.5*(vi->vpPosXMin + vi->vpPosXMax);
    }
    vi->labelXPosX   = posx;

    float posy = inPosy;
    if (posy < 0.0) {
        posy = 0.35*vi->vpPosYMin;
    }
    vi->labelXPosY   = posy;

    vi->labelXAngle  = 0.0;
    vi->labelXFJust  = 0.5;
    vi->labelXSize   = size;
    vi->labelXColor  = color;
    vi->labelXBColor = bgcolor;
}

void Plotter2::setLabelY(const std::string& label, const float inPosx, const float inPosy, const float size, const std::string& style, const int color, const int bgcolor, const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];

    std::string styleString;
    if (style == "") {
      styleString = "";
    } else if (style == "roman") {
      styleString = "\\fr";
    } else if (style == "italic") {
      styleString = "\\fi";
    } else if (style == "script") {
      styleString = "\\fs";
    }
    vi->labelYString = styleString + label;

    float posx = inPosx;
    if (posx < 0.0) {
        posx = 0.35*vi->vpPosXMin;
    }
    vi->labelYPosX   = posx;

    float posy = inPosy;
    if (posy < 0.0) {
        posy = 0.5*(vi->vpPosYMin + vi->vpPosYMax);
    }
    vi->labelYPosY   = posy;

    vi->labelYAngle  = 90.0;
    vi->labelYFJust  = 0.5;
    vi->labelYSize   = size;
    vi->labelYColor  = color;
    vi->labelYBColor = bgcolor;
}

void Plotter2::setTitle(const std::string& label, const float inPosx, const float inPosy, const float size, const std::string& style, const int color, const int bgcolor, const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];

    std::string styleString;
    if (style == "") {
      styleString = "";
    } else if (style == "roman") {
      styleString = "\\fr";
    } else if (style == "italic") {
      styleString = "\\fi";
    } else if (style == "script") {
      styleString = "\\fs";
    }
    vi->titleString = styleString + label;

    float posx = inPosx;
    if (posx < 0.0) {
        posx = 0.5*(vi->vpPosXMin + vi->vpPosXMax);
    }
    vi->titlePosX   = posx;

    float posy = inPosy;
    if (posy < 0.0) {
        posy = vi->vpPosYMax + 0.25*(1.0 - vi->vpPosYMax);
    }
    vi->titlePosY   = posy;

    vi->titleAngle  = 0.0;
    vi->titleFJust  = 0.5;
    vi->titleSize   = size;
    vi->titleColor  = color;
    vi->titleBColor = bgcolor;
}

void Plotter2::setViewportBackgroundColor(const int bgcolor, const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        exit(0);
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    vi->vpBColor = bgcolor;
}

  /*
void Plotter2::setAnnotation(const std::string& label, const float posx, const float posy, const float angle, const float fjust, const float size, const std::string& style, const int color, const int bgcolor, const int inVpid) {
    int vpid = inVpid;
    if (vpid < 0) {
        vpid = vInfo.size() - 1;
    }
    if (vpid < 0) {
        Plotter2ViewportInfo vi;
        vInfo.push_back(vi);
	vpid = 0;
    }

    std::string styleString;
    if (style == "") {
      styleString = "";
    } else if (style == "roman") {
      styleString = "\\fr";
    } else if (style == "italic") {
      styleString = "\\fi";
    } else if (style == "script") {
      styleString = "\\fs";
    }

    Plotter2ViewportInfo* vi = &vInfo[vpid];
    //vi->titleString = styleString + label;
    //vi->titlePosX   = posx;
    //vi->titlePosY   = posy;
    //vi->titleAngle  = angle;
    //vi->titleFJust  = fjust;
    //vi->titleSize   = size;
    //vi->titleColor  = color;
    //vi->titleBColor = bgcolor;
}
  */

void Plotter2::close() {
    if (hasDevice) {
        cpgclos();
        hasDevice = false;
    }
}

void Plotter2::plot() {
    open();

    cpgscr(0, 1.0, 1.0, 1.0); // set background color white
    cpgscr(1, 0.0, 0.0, 0.0); // set foreground color black

    for (unsigned int i = 0; i < vInfo.size(); ++i) {
        Plotter2ViewportInfo vi = vInfo[i];

	if (vi.showViewport) {
            cpgstbg(0); // reset background colour to the initial one (white)
            cpgsci(1);  // reset foreground colour to the initial one (black)
  	    cpgsls(1);  // reset line style to solid
	    cpgslw(1);  // reset line width to 1
	    cpgscf(1);  // reset font style to normal
	    cpgsch(vi.fontSizeDef);// reset font size
	    cpgsfs(1);  // reset fill style (solid)

	    // setup viewport
            cpgsvp(vi.vpPosXMin, vi.vpPosXMax, vi.vpPosYMin, vi.vpPosYMax);
  	    vi.adjustRange();
  	    vi.adjustTickInterval();

	    cpgswin(vi.vpRangeXMin, vi.vpRangeXMax, vi.vpRangeYMin, vi.vpRangeYMax);

	    // background color (default is transparent)
	    if (vi.vpBColor >= 0) {
	        cpgsci(vi.vpBColor);
	        cpgrect(vi.vpRangeXMin, vi.vpRangeXMax, vi.vpRangeYMin, vi.vpRangeYMax);
	        cpgsci(1);  // reset foreground colour to the initial one (black)
	    }

	    // data
	    for (unsigned int j = 0; j < vi.vData.size(); ++j) {
                cpgstbg(0); // reset background colour to the initial one (white)
                cpgsci(1);  // reset foreground colour to the initial one (black)
  	        cpgsls(1);  // reset line style to solid
	        cpgslw(1);  // reset line width to 1
	        cpgscf(1);  // reset font style to normal
	        cpgsch(vi.fontSizeDef);// reset font size

	        Plotter2DataInfo di = vi.vData[j];
	        std::vector<float> vxdata = di.xData;
                int ndata = vxdata.size();
	        float* pxdata = new float[ndata];
	        float* pydata = new float[ndata];
	        for (int k = 0; k < ndata; ++k) {
	            pxdata[k] = di.xData[k];
	            pydata[k] = di.yData[k];
	        }

	        if (di.drawLine) {
  	            cpgsls(di.lineStyle);
	            cpgslw(di.lineWidth);
		    int colorIdx = di.lineColor;
		    if (colorIdx < 0) {
		        colorIdx = (j + 1) % 15 + 1;
		    }
	            cpgsci(colorIdx);
	            cpgline(ndata, pxdata, pydata);
	        }

	        if (di.drawMarker) {
	            cpgsch(di.markerSize);
	            cpgsci(di.markerColor);
	            cpgpt(ndata, pxdata, pydata, di.markerType);
	        }

	        delete [] pxdata;
	        delete [] pydata;
	    }

	    // masks
	    for (unsigned int j = 0; j < vi.vRect.size(); ++j) {
                cpgstbg(0); // reset background colour to the initial one (white)
                cpgsci(1);  // reset foreground colour to the initial one (black)
  	        cpgsls(1);  // reset line style to solid
	        cpgslw(1);  // reset line width to 1
	        cpgscf(1);  // reset font style to normal
	        cpgsch(vi.fontSizeDef);// reset font size
	        cpgsfs(1);  // reset fill style (solid)

	        Plotter2RectInfo ri = vi.vRect[j];
                cpgsci(ri.color);
	        cpgsfs(ri.fill);
	        cpgslw(ri.width);
	        cpgshs(45.0, ri.hsep, 0.0);
	        float* mxdata = new float[4];
	        float* mydata = new float[4];
	        mxdata[0] = ri.xmin;
	        mxdata[1] = ri.xmax;
	        mxdata[2] = ri.xmax;
	        mxdata[3] = ri.xmin;
	        mydata[0] = ri.ymin;
	        mydata[1] = ri.ymin;
	        mydata[2] = ri.ymax;
	        mydata[3] = ri.ymax;
                cpgpoly(4, mxdata, mydata);
	    }

            cpgstbg(0); // reset background colour to the initial one (white)
            cpgsci(1);  // reset foreground colour to the initial one (black)
  	    cpgsls(1);  // reset line style to solid
	    cpgslw(1);  // reset line width to 1
	    cpgscf(1);  // reset font style to normal
	    cpgsch(vi.fontSizeDef);// reset font size
	    cpgsfs(1);  // reset fill style (solid)

            cpgbox("BCTS",  vi.majorTickIntervalX, vi.nMinorTickWithinMajorTicksX, 
	           "BCTSV", vi.majorTickIntervalY, vi.nMinorTickWithinMajorTicksY);

	    // viewport outline, ticks and number labels
	    std::string numformatx, numformaty;
	    if (vi.numLocationX == "l") {
	        numformatx = "N";
	    } else if (vi.numLocationX == "r") {
	        numformatx = "M";
	    } else if (vi.numLocationX == "") {
	        numformatx = "";
	    }
	    if (vi.numLocationY == "b") {
	        numformaty = "NV";
	    } else if (vi.numLocationY == "t") {
	        numformaty = "MV";
	    } else if (vi.numLocationY == "") {
	        numformaty = "";
	    }

            cpgbox(numformatx.c_str(), vi.majorTickIntervalX * vi.nMajorTickWithinTickNumsX, 0, 
	           numformaty.c_str(), vi.majorTickIntervalY * vi.nMajorTickWithinTickNumsY, 0);

	    float xpos, ypos;

	    // x-label
	    vi.getWorldCoordByWindowCoord(vi.labelXPosX, vi.labelXPosY, &xpos, &ypos);
	    cpgsch(vi.labelXSize);
            cpgsci(vi.labelXColor);
            cpgstbg(vi.labelXBColor); //outside viewports, works ONLY with /xwindow
            cpgptxt(xpos, ypos, vi.labelXAngle, vi.labelXFJust, vi.labelXString.c_str());

	    // y-label
	    vi.getWorldCoordByWindowCoord(vi.labelYPosX, vi.labelYPosY, &xpos, &ypos);
	    cpgsch(vi.labelYSize);
            cpgsci(vi.labelYColor);
            cpgstbg(vi.labelYBColor); //outside viewports, works ONLY with /xwindow
            cpgptxt(xpos, ypos, vi.labelYAngle, vi.labelYFJust, vi.labelYString.c_str());

	    // title
	    vi.getWorldCoordByWindowCoord(vi.titlePosX, vi.titlePosY, &xpos, &ypos);
	    cpgsch(vi.titleSize);
            cpgsci(vi.titleColor);
            cpgstbg(vi.titleBColor); //outside viewports, works ONLY with /xwindow
            cpgptxt(xpos, ypos, vi.titleAngle, vi.titleFJust, vi.titleString.c_str());
	}

    }

    close();
}

} // namespace asap
