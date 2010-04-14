
/***
 * Framework independent header file for atmosphere...
 *
 * Implement the atmosphere component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#ifndef _atmosphere_cmpt__H__
#define _atmosphere_cmpt__H__
#ifndef _atmosphere_cnpt__H__
#define _atmosphere_cnpt__H__

#include <vector>
#include <xmlcasa/casac.h>
#include <xmlcasa//Quantity.h>
#include <xmlcasa/atmosphere/atmosphere_forward.h>

// put includes here

namespace casac {

/**
 * atmosphere component class 
 *
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 **/
class atmosphere
{
  private:

	

  public:

    atmosphere();
    virtual ~atmosphere();

    std::vector<std::string> listAtmosphereTypes();

    std::string initAtmProfile(const Quantity& altitude = Quantity(std::vector<double> (1, 5000.),"m"), const Quantity& temperature = Quantity(std::vector<double> (1, 270.0),"K"), const Quantity& pressure = Quantity(std::vector<double> (1, 560.0),"mbar"), const Quantity& maxAltitude = Quantity(std::vector<double> (1, 48.0),"km"), const double humidity = 20.0, const Quantity& dTem_dh = Quantity(std::vector<double> (1, -5.6),"K/km"), const Quantity& dP = Quantity(std::vector<double> (1, 10.0),"mbar"), const double dPm = 1.2, const Quantity& h0 = Quantity(std::vector<double> (1, 2.0),"km"), const int atmType = 1);

    std::string updateAtmProfile(const Quantity& altitude = Quantity(std::vector<double> (1, 5000.),"m"), const Quantity& temperature = Quantity(std::vector<double> (1, 270.0),"K"), const Quantity& pressure = Quantity(std::vector<double> (1, 560.0),"mbar"), const double humidity = 20.0, const Quantity& dTem_dh = Quantity(std::vector<double> (1, -5.6),"K/km"), const Quantity& h0 = Quantity(std::vector<double> (1, 2.0),"km"));

    std::string getBasicAtmParms(Quantity& altitude, Quantity& temperature, Quantity& pressure, Quantity& maxAltitude, double& humidity, Quantity& dTem_dh, Quantity& dP, double& dPm, Quantity& h0, std::string& atmType);

    int getNumLayers();

    Quantity getGroundWH2O();

    std::string getProfile(Quantity& thickness, Quantity& temperature, Quantity& watermassdensity, Quantity& water, Quantity& pressure, Quantity& O3, Quantity& CO, Quantity& N2O);

    int initSpectralWindow(const int nbands = 1, const Quantity& fCenter = Quantity(std::vector<double> (1, 90),"GHz"), const Quantity& fWidth = Quantity(std::vector<double> (1, 0.64),"GHz"), const Quantity& fRes = Quantity(std::vector<double> (1, 0.0),"GHz"));

    int addSpectralWindow(const Quantity& fCenter = Quantity(std::vector<double> (1, 350),"GHz"), const Quantity& fWidth = Quantity(std::vector<double> (1, 0.008),"GHz"), const Quantity& fRes = Quantity(std::vector<double> (1, 0.002),"GHz"));

    int getNumSpectralWindows();

    int getNumChan(const int spwid = 0);

    int getRefChan(const int spwid = 0);

    Quantity getRefFreq(const int spwid = 0);

    Quantity getChanSep(const int spwid = 0);

    Quantity getChanFreq(const int chanNum = 0, const int spwid = 0);

    Quantity getSpectralWindow(const int spwid = 0);

    double getChanNum(const Quantity& freq, const int spwid = 0);

    Quantity getBandwidth(const int spwid = 0);

    Quantity getMinFreq(const int spwid = 0);

    Quantity getMaxFreq(const int spwid = 0);

    double getDryOpacity(const int nc = -1, const int spwid = 0);

    double getDryContOpacity(const int nc = -1, const int spwid = 0);

    double getO2LinesOpacity(const int nc = -1, const int spwid = 0);

    double getO3LinesOpacity(const int nc = -1, const int spwid = 0);

    double getCOLinesOpacity(const int nc = -1, const int spwid = 0);

    double getN2OLinesOpacity(const int nc = -1, const int spwid = 0);

    Quantity getWetOpacity(const int nc = -1, const int spwid = 0);

    double getH2OLinesOpacity(const int nc = -1, const int spwid = 0);

    double getH2OContOpacity(const int nc = -1, const int spwid = 0);

    int getDryOpacitySpec(std::vector<double>& dryOpacity, const int spwid = 0);

    int getWetOpacitySpec(Quantity& wetOpacity, const int spwid = 0);

    Quantity getDispersivePhaseDelay(const int nc = -1, const int spwid = 0);

    Quantity getDispersiveWetPhaseDelay(const int nc = -1, const int spwid = 0);

    Quantity getNonDispersiveWetPhaseDelay(const int nc = -1, const int spwid = 0);

    Quantity getNonDispersiveDryPhaseDelay(const int nc = -1, const int spwid = 0);

    Quantity getNonDispersivePhaseDelay(const int nc = -1, const int spwid = 0);

    Quantity getDispersivePathLength(const int nc = -1, const int spwid = 0);

    Quantity getDispersiveWetPathLength(const int nc = -1, const int spwid = 0);

    Quantity getNonDispersiveWetPathLength(const int nc = -1, const int spwid = 0);

    Quantity getNonDispersiveDryPathLength(const int nc = -1, const int spwid = 0);

    Quantity getO2LinesPathLength(const int nc = -1, const int spwid = 0);

    Quantity getO3LinesPathLength(const int nc = -1, const int spwid = 0);

    Quantity getCOLinesPathLength(const int nc = -1, const int spwid = 0);

    Quantity getN2OLinesPathLength(const int nc = -1, const int spwid = 0);

    Quantity getNonDispersivePathLength(const int nc = -1, const int spwid = 0);

    Quantity getAbsH2OLines(const int nl, const int nf = 0, const int spwid = 0);

    Quantity getAbsH2OCont(const int nl, const int nf = 0, const int spwid = 0);

    Quantity getAbsO2Lines(const int nl, const int nf = 0, const int spwid = 0);

    Quantity getAbsDryCont(const int nl, const int nf = 0, const int spwid = 0);

    Quantity getAbsO3Lines(const int nl, const int nf = 0, const int spwid = 0);

    Quantity getAbsCOLines(const int nl, const int nf = 0, const int spwid = 0);

    Quantity getAbsN2OLines(const int nl, const int nf = 0, const int spwid = 0);

    Quantity getAbsTotalDry(const int nl, const int nf = 0, const int spwid = 0);

    Quantity getAbsTotalWet(const int nl, const int nf = 0, const int spwid = 0);

    bool setUserWH2O(const Quantity& wh2o = Quantity(std::vector<double> (1, 0.0),"mm"));

    Quantity getUserWH2O();

    bool setAirMass(const double airmass);

    double getAirMass();

    bool setSkyBackgroundTemperature(const Quantity& tbgr = Quantity(std::vector<double> (1, 2.73),"K"));

    Quantity getSkyBackgroundTemperature();

    Quantity getAverageTebbSky(const int spwid = 0, const Quantity& wh2o = Quantity(std::vector<double> (1, -1),"mm"));

    Quantity getTebbSky(const int nc = -1, const int spwid = 0, const Quantity& wh2o = Quantity(std::vector<double> (1, -1),"mm"));

    private:

	#include <xmlcasa/atmosphere/atmosphere_private.h>

};

} // casac namespace
#endif
#endif

