
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

#include <casaqt/QtUtilities/QtDBusApp.h>

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
class plotms : public QtDBusApp
{
  private:

	

  public:

    plotms();
    virtual ~plotms();

    void setLogFilename(const std::string& logFilename = "casapy.log");

    std::string getLogFilename();

    void setLogFilter(const std::string& priority);

    std::string getLogFilter();

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

    void setPlotMSAveraging(const std::string& channel = "", const std::string& time = "", const bool scan = false, const bool field = false, const bool baseline = false, const bool antenna = false, const bool spw = false, const bool updateImmediately = true, const int plotIndex = 0);

    void setPlotMSAveragingRec(const ::casac::record& averaging, const bool updateImmediately = true, const int plotIndex = 0);

    ::casac::record* getPlotMSAveraging(const int plotIndex = 0);

    void setPlotXAxis(const std::string& xAxis, const std::string& xDataColumn = "", const bool updateImmediately = true, const int plotIndex = 0);

    void setPlotYAxis(const std::string& yAxis, const std::string& yDataColumn = "", const bool updateImmediately = true, const int plotIndex = 0);

    void setPlotAxes(const std::string& xAxis = "", const std::string& yAxis = "", const std::string& xDataColumn = "", const std::string& yDataColumn = "", const bool updateImmediately = true, const int plotIndex = 0);

    std::string getPlotXAxis(const int plotIndex = 0);

    std::string getPlotXDataColumn(const int plotIndex = 0);

    std::string getPlotYAxis(const int plotIndex = 0);

    std::string getPlotYDataColumn(const int plotIndex = 0);

    ::casac::record* getPlotParams(const int plotIndex = 0);

    void setFlagExtension(const bool extend = false, const std::string& correlation = "", const bool channel = false, const bool spw = false, const std::string& antenna = "", const bool time = false, const bool scans = false, const bool field = false, const ::casac::record& alternateSelection = ::casac::initialize_record(""));

    void setFlagExtensionRec(const ::casac::record& flagExtension);

    ::casac::record* getFlagExtension();

    void update();

    void show();

    void hide();

    private:

    #include <xmlcasa/plotms/plotms_private.h>
  

};

} // casac namespace
#endif

