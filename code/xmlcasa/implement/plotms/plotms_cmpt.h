
/***
 * Framework independent header file for plotms...
 *
 * Implement the plotms component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _plotms_cmpt__H__
#define _plotms_cmpt__H__

#include <vector>
#include <xmlcasa/casac.h>

#include <xmlcasa/conversions.h>
#include <xmlcasa/record.h>
#include <xmlcasa/plotms/plotms_forward.h>

// put includes here

namespace casac {

/**
 * plotms component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class plotms
{
  private:

	

  public:

    plotms();
    virtual ~plotms();

    void setLogLevel(const std::string& logLevel, const bool logDebug = false);

    std::string getLogLevel();

    bool getLogDebug();

    void setClearSelectionOnAxesChange(const bool clearSelection);

    bool getClearSelectionOnAxesChange();

    void setCachedImageSize(const int width, const int height);

    void setCachedImageSizeToScreenResolution();

    int getCachedImageWidth();

    int getCachedImageHeight();

    void setPlotMSFilename(const std::string& msFilename, const bool updateImmediately = true, const int plotIndex = 0);

    std::string getPlotMSFilename(const int plotIndex = 0);

    void setPlotMSSelection(const std::string& field = "", const std::string& spw = "", const std::string& timerange = "", const std::string& uvrange = "", const std::string& antenna = "", const std::string& scan = "", const std::string& corr = "", const std::string& array = "", const std::string& msselect = "", const bool updateImmediately = true, const int plotIndex = 0);

    void setPlotMSSelectionRec(const ::casac::record& selection, const bool updateImmediately = true, const int plotIndex = 0);

    ::casac::record* getPlotMSSelection(const int plotIndex = 0);

    void setPlotMSAveraging(const bool channel = false, const double channelValue = 0.0, const bool time = false, const double timeValue = 0.0, const bool scan = false, const bool field = false, const bool baseline = false, const bool antenna = false, const bool spw = false, const bool updateImmediately = true, const int plotIndex = 0);

    void setPlotMSAveragingRec(const ::casac::record& averaging, const bool updateImmediately = true, const int plotIndex = 0);

    ::casac::record* getPlotMSAveraging(const int plotIndex = 0);

    void setPlotXAxis(const std::string& xAxis, const std::string& xDataColumn = "", const bool updateImmediately = true, const int plotIndex = 0);

    void setPlotYAxis(const std::string& yAxis, const std::string& yDataColumn = "", const bool updateImmediately = true, const int plotIndex = 0);

    void setPlotAxes(const std::string& xAxis, const std::string& yAxis, const std::string& xDataColumn = "", const std::string& yDataColumn = "", const bool updateImmediately = true, const int plotIndex = 0);

    std::string getPlotXAxis(const int plotIndex = 0);

    std::string getPlotXDataColumn(const int plotIndex = 0);

    std::string getPlotYAxis(const int plotIndex = 0);

    std::string getPlotYDataColumn(const int plotIndex = 0);

    void update();

    int execLoop();

    private:

    #include <xmlcasa/plotms/plotms_private.h>
  

};

} // casac namespace
#endif

