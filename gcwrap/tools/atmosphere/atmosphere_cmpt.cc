/***
 * Framework independent implementation file for atmosphere...
 *
 * Implement the atmosphere component here.
 * 
 * // Interface to the ALMA TELCAL C++ API for Juan Padro's FORTRAN
 * // Atmospheric Model library atmlib.
 * // Implemented 12 Oct 2006, Raymond Rusk
 *
 * @author
 * @version 
 ***/

#include <string>
#include <vector>
#include <iostream>

#include<casa/Utilities/Assert.h>
#include <casa/Exceptions/Error.h>
#include <casa/Logging/LogIO.h>
#include <atmosphere_cmpt.h>
//#include <stdcasa/StdCasa/CasacSupport.h>

using namespace atm;
using namespace std;
using namespace casacore;
using namespace casa;

using namespace casacore;
namespace casac {

///// helper functions /////
// Assert ATM type is in permissive range of enum, typeAtm_t
inline void atmosphere::check_atmtype_enum(int atmtype) {
  typeAtm_t typeEnd = typeATM_end;
  ThrowIf((atmtype<1 || atmtype>=typeEnd), "atmType not in permissive range.");
}
// Assert int value is positive or zero.
inline void atmosphere::assert_unsigned_int(int value)
{
  AlwaysAssert(value>=0, casacore::AipsError);
}
// Assert Spw ID and channel ID are in proper range
inline void atmosphere::assert_spwid(int spwid) {
  ThrowIf(pSpectralGrid == 0, "Spectral window is not defined yet");
  ThrowIf(spwid < 0 || static_cast<unsigned int>(spwid) >= pSpectralGrid->getNumSpectralWindow(),
	  "Spw ID out of range.");
}
inline void atmosphere::assert_spwid_and_channel(int spwid, int chan) {
  ThrowIf(pSpectralGrid == 0, "Spectral window is not defined yet");
  assert_spwid(spwid);
  ThrowIf(chan < 0 || static_cast<unsigned int>(chan) >= pSpectralGrid->getNumChan(static_cast<unsigned int>(spwid)),
	  "Channel ID out of range.");
}
///// end of helper functions /////

atmosphere::atmosphere()
  : pAtmProfile(0),
    pSpectralGrid(0),
    pRefractiveIndexProfile(0),
    pSkyStatus(0)
{
  try {
    itsLog = new LogIO();
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
}

atmosphere::~atmosphere()
{
  try {
    if (pSkyStatus != 0) {
      delete pSkyStatus;
      pSkyStatus = 0;
    }
    if (pRefractiveIndexProfile != 0) {
      delete pRefractiveIndexProfile;
      pRefractiveIndexProfile = 0;
    }
    if (pSpectralGrid != 0) {
      delete pSpectralGrid;
      pSpectralGrid = 0;
    }
    if (pAtmProfile != 0) {
      delete pAtmProfile;
      pAtmProfile = 0;
    }
    delete itsLog;
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
}

std::string
atmosphere::getAtmVersion()
{
  std::string rtn("");
  try {
    if (ATM_VERSION == getVersion()) {
    rtn = getTag();
    } else {
      *itsLog << LogIO::SEVERE
	      << "Version mismatch between ATM library and header files:  "
	      << "library = v" << getVersion() << ", header = v" << ATM_VERSION
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return rtn;
}


std::vector<std::string>
atmosphere::listAtmosphereTypes()
{
  std::vector<std::string> rtn;
  try {
    typeAtm_t typeEnd = typeATM_end;
    for (unsigned int i = 1; i < typeEnd; i++) {
      ostringstream oss;
      oss << i << " - " << AtmProfile::getAtmosphereType(i);
      rtn.push_back(oss.str());
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return rtn;
}
  
std::string
atmosphere::initAtmProfile(const Quantity& altitude,
			   const Quantity& temperature,
			   const Quantity& pressure,
			   const Quantity& maxAltitude,
			   const double humidity, const Quantity& dTem_dh,
			   const Quantity& dP, const double dPm,
			   const Quantity& h0, int atmtype)
{
  string rtn;

  try {
    check_atmtype_enum(atmtype);
    Length       Alt((casaQuantity(altitude)).getValue("m"),"m");
    Pressure       P((casaQuantity(pressure)).getValue("mbar"),"mb");
    Temperature    T((casaQuantity(temperature)).getValue("K"),"K");
    double       TLR((casaQuantity(dTem_dh)).getValue("K/km"));
    Humidity       H(humidity, "%");
    Length       WVL((casaQuantity(h0)).getValue("km"),"km");
    Pressure   Pstep((casaQuantity(dP)).getValue("mbar"), "mb");
    double PstepFact(dPm);
    Length    topAtm((casaQuantity(maxAltitude)).getValue("m"), "m");
    unsigned int atmType = (unsigned int)atmtype;

    ostringstream oss;
    oss<<"BASIC ATMOSPHERIC PARAMETERS TO GENERATE REFERENCE ATMOSPHERIC PROFILE"<<endl;
    oss<<"  "<<endl;
    oss<<"Ground temperature T:         " << T.get("K")      << " K"    <<endl;
    oss<<"Ground pressure P:            " << P.get("mb")     << " mb"   <<endl;
    oss<<"Relative humidity rh:         " << H.get("%")      << " %"    <<endl;
    oss<<"Scale height h0:              " << WVL.get("km")   << " km"   <<endl;
    oss<<"Pressure step dp:             " << Pstep.get("mb") << " mb"   <<endl;
    oss<<"Altitude alti:                " << Alt.get("m")    << " m"    <<endl;
    oss<<"Attitude top atm profile:     " << topAtm.get("km")<< " km"   <<endl;
    oss<<"Pressure step factor:         " << PstepFact          << " "    <<endl;
    oss<<"Tropospheric lapse rate:      " << TLR                << " K/km" <<endl;

    // Reset all atmospheric and spectral settings for this function.
    if (pSpectralGrid != 0) {
      delete pSpectralGrid;
      pSpectralGrid = 0;
    }
    if (pRefractiveIndexProfile != 0) {
      delete pRefractiveIndexProfile;
      pRefractiveIndexProfile = 0;
    }
    if (pSkyStatus != 0) {
      delete pSkyStatus;
      pSkyStatus = 0;
    }
    if (pAtmProfile != 0) delete pAtmProfile;
    pAtmProfile = new AtmProfile( Alt, P, T, TLR, H, WVL, Pstep, PstepFact,
				  topAtm, atmType );

    oss<<"Atmospheric type:             " << pAtmProfile->getAtmosphereType() <<endl;
    oss<<endl;
    oss<<"Built atmospheric profile with " << pAtmProfile->getNumLayer() << " layers." << endl;
    oss<<endl;
    rtn = oss.str();
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return rtn;
}

std::string
atmosphere::updateAtmProfile(const Quantity& altitude,
			     const Quantity& temperature,
			     const Quantity& pressure, const double humidity,
			     const Quantity& dTem_dh, const Quantity& h0)
{
  string rtn;
  try {
    Length       Alt((casaQuantity(altitude)).getValue("m"),"m");
    Pressure       P((casaQuantity(pressure)).getValue("mbar"),"mb");
    Temperature    T((casaQuantity(temperature)).getValue("K"),"K");
    double       TLR((casaQuantity(dTem_dh)).getValue("K/km"));
    Humidity       H(humidity, "%");
    Length       WVL((casaQuantity(h0)).getValue("km"),"km");
    if (pAtmProfile) {
      if (! pAtmProfile->setBasicAtmosphericParameters(Alt,P,T,TLR,H,WVL) ) {
	*itsLog << LogIO::WARN
		<< "Atmospheric profile update failed!" << LogIO::POST;
      }
      if (pRefractiveIndexProfile) {
	if (! pRefractiveIndexProfile->setBasicAtmosphericParameters(Alt,P,T,TLR,H,WVL) ) {
	  *itsLog << LogIO::WARN
		  << "Refractive index profile update failed!" 
		  << LogIO::POST;
	}
      }
      if (pSkyStatus) {
	if (! pSkyStatus->setBasicAtmosphericParameters(Alt,P,T,TLR,H,WVL) ) {
	  *itsLog << LogIO::WARN
		  << "Skystatus update failed!" 
		  << LogIO::POST;
	}
	// WORK AROUND to set the 1st guess water column as a coefficient
	pSkyStatus->setUserWH2O(pSkyStatus->getGroundWH2O());
      }
    } else {
      *itsLog << LogIO::WARN
	      << "Please initialize atmospheric profile with initAtmProfile."
	      << LogIO::POST;
    }

    ostringstream oss;
    oss<<"UPDATED BASIC ATMOSPHERIC PARAMETERS TO GENERATE REFERENCE ATMOSPHERIC PROFILE"<<endl;
    oss<<"  "<<endl;
    oss<<"Ground temperature T:         " << T.get("K")      << " K"    <<endl;
    oss<<"Ground pressure P:            " << P.get("mb")     << " mb"   <<endl;
    oss<<"Relative humidity rh:         " << H.get("%")      << " %"    <<endl;
    oss<<"Scale height h0:              " << WVL.get("km")   << " km"   <<endl;
    oss<<"Altitude alti:                " << Alt.get("m")    << " m"    <<endl;
    oss<<"Tropospheric lapse rate:      " << TLR                << " K/km" <<endl;
    oss<<endl;
    rtn = oss.str();
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return rtn;
}

std::string
atmosphere::getBasicAtmParms(Quantity& altitude, Quantity& temperature,
			     Quantity& pressure, Quantity& maxAltitude,
			     double& humidity, Quantity& dTem_dh,
			     Quantity& dP, double& dPm, Quantity& h0,
			     std::string& atmType)
{
  string rtn("");
  try {
    if (pAtmProfile) {
      altitude.value.resize(1);
      temperature.value.resize(1);
      pressure.value.resize(1);
      maxAltitude.value.resize(1);
      dTem_dh.value.resize(1);
      dP.value.resize(1);
      h0.value.resize(1);
      Length Alt = pAtmProfile->getAltitude();
      altitude.value[0] = Alt.get("m"); altitude.units = "m";
      Temperature T = pAtmProfile->getGroundTemperature();
      temperature.value[0] = T.get("K"); temperature.units = "K";
      Pressure P = pAtmProfile->getGroundPressure();
      pressure.value[0] = P.get("mb"); pressure.units = "mbar";
      Length topAtm = pAtmProfile->getTopAtmProfile();
      maxAltitude.value[0] = topAtm.get("km"); maxAltitude.units = "km";
      Humidity H = pAtmProfile->getRelativeHumidity();
      humidity = H.get("%");
      double TLR = pAtmProfile->getTropoLapseRate();
      dTem_dh.value[0] = TLR; dTem_dh.units ="K/km";
      Pressure Pstep = pAtmProfile->getPressureStep();
      dP.value[0] = Pstep.get("mb");dP.units = "mbar";
      Pressure PstepFact = pAtmProfile->getPressureStepFactor();
      dPm = PstepFact.get("Pa");
      Length WVL = pAtmProfile->getWvScaleHeight();
      h0.value[0] = WVL.get("km"); h0.units = "km";
      atmType = pAtmProfile->getAtmosphereType();

      ostringstream oss;
      oss<<"CURRENT ATMOSPHERIC PARAMETERS OF REFERENCE ATMOSPHERIC PROFILE"<<endl;
      oss<<"  "<<endl;
      oss<<"Ground temperature T:         " << temperature.value[0]  << " " << temperature.units  <<endl;
      oss<<"Ground pressure P:            " << pressure.value[0]     << " " << pressure.units <<endl;
      oss<<"Relative humidity rh:         " << humidity              << " %"    <<endl;
      oss<<"Scale height h0:              " << h0.value[0]           << " " << h0.units <<endl;
      oss<<"Pressure step dp:             " << dP.value[0]           << " " << dP.units <<endl;
      oss<<"Altitude alti:                " << altitude.value[0]     << " " << altitude.units <<endl;
      oss<<"Attitude top atm profile      " << maxAltitude.value[0]  << " " << maxAltitude.units <<endl;
      oss<<"Pressure step factor          " << dPm                   << " "    <<endl;
      oss<<"Tropospheric lapse rate       " << dTem_dh.value[0]      << " " << dTem_dh.units <<endl;
      oss<<"Atmospheric type:             " << atmType <<endl;
      oss<<endl;
      oss<<"Atmospheric profile has " << pAtmProfile->getNumLayer() << " layers." << endl;
      rtn = oss.str();
    } else {
      *itsLog << LogIO::WARN
	      << "Please initialize atmospheric profile with initAtmProfile."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return rtn;
}

int
atmosphere::getNumLayers()
{
  int rtn(-1);
  try {
    if (pAtmProfile) {
      rtn = pAtmProfile->getNumLayer();
    } else {
      *itsLog << LogIO::WARN
	      << "Please initialize atmospheric profile with initAtmProfile."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return rtn;
}


// int
// atmosphere::getAtmTypeHPT(Quantity& Hx, Quantity& Px, Quantity& Tx)
// {
//   int rtn(-1);
//   try {
//     if (pAtmProfile) {
//       rtn = pAtmProfile->getArraySize();
//       (Hx.value).resize(rtn); Hx.units="km";
//       (Px.value).resize(rtn); Px.units="mbar";
//       (Tx.value).resize(rtn); Tx.units="K";
//       for (int i=0; i < rtn; i++) {
// 	(Hx.value)[i] = pAtmProfile->getHx(i);
// 	(Px.value)[i] = pAtmProfile->getPx(i);
// 	(Tx.value)[i] = pAtmProfile->getTx(i);
//       }
//     } else {
//       *itsLog << LogIO::WARN
// 	      << "Please initialize atmospheric profile with initAtmProfile."
// 	      << LogIO::POST;
//     }
//   } catch (AipsError x) {
//     *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
// 	    << LogIO::POST;
//     RETHROW(x);
//   }
//   return rtn;
// }

Quantity
atmosphere::getGroundWH2O()
{
  ::casac::Quantity q;
  try {
    if (pAtmProfile) {
      atm::Length gw = pAtmProfile->getGroundWH2O();
      std::vector<double> qvalue(1);
      qvalue[0] = gw.get("mm");
      q.value = qvalue;
      q.units = "mm";
    } else {
      *itsLog << LogIO::WARN
	      << "Please initialize atmospheric profile with initAtmProfile."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return q;
}

std::string
atmosphere::getProfile(Quantity& thickness, Quantity& temperature,
		       Quantity& watermassdensity,
		       Quantity& water, Quantity& pressure, Quantity& O3,
		       Quantity& CO, Quantity& N2O)
{
  std::string rtn("");
  try {
    if (pAtmProfile) {
      int nl = pAtmProfile->getNumLayer();
      thickness.value.resize(nl);        thickness.units="m";
      temperature.value.resize(nl);      temperature.units="K";
      watermassdensity.value.resize(nl); watermassdensity.units="kg m-3";
      water.value.resize(nl);            water.units="m-3";
      pressure.value.resize(nl);         pressure.units="mb";
      O3.value.resize(nl);               O3.units="m-3";
      CO.value.resize(nl);               CO.units="m-3";
      N2O.value.resize(nl);              N2O.units="m-3";

      ostringstream oss;
      oss << "Number of layers returned: " << nl << endl;
      oss << "Layer parameters: " << endl;

      for(int i=0; i < nl; i++){
	pressure.value[i] = pAtmProfile->getLayerPressure(i).get(pressure.units);
	temperature.value[i] =
	  pAtmProfile->getLayerTemperature(i).get(temperature.units);
        thickness.value[i] =
	  pAtmProfile->getLayerThickness(i).get(thickness.units);
	watermassdensity.value[i]=
	  pAtmProfile->
	  getLayerWaterVaporMassDensity(i).get(watermassdensity.units);
	water.value[i] = pAtmProfile->getLayerWaterVaporNumberDensity(i).get(water.units);
	O3.value[i] = pAtmProfile->getLayerO3(i).get(O3.units);
	CO.value[i] = pAtmProfile->getLayerCO(i).get(CO.units);
	N2O.value[i] = pAtmProfile->getLayerN2O(i).get(N2O.units);
	oss << " P: "         << pressure.value[i]   << " " << pressure.units
	    << "  T: "         << temperature.value[i]<< " " << temperature.units
	    << "  Thickness: " << thickness.value[i]  << " " << thickness.units
	    << "  WaterVapor: "<< watermassdensity.value[i] << " " << watermassdensity.units
	    << "  WaterVapor: "<< water.value[i]      << " " << water.units
	    << "  CO: "        << CO.value[i]         << " " << CO.units
	    << "  O3: "        << O3.value[i]         << " " << O3.units
	    << "  N2O: "       << N2O.value[i]        << " " << N2O.units
	    << endl;
      }
      rtn.append(oss.str());
    } else {
      *itsLog << LogIO::WARN
	      << "Please initialize atmospheric profile with initAtmProfile."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return rtn;
}

int
atmosphere::initSpectralWindow(int nbands, const Quantity& fCenter,
		       const Quantity& fWidth, const Quantity& fRes)
{
  int rstat(-1);
  try {
    ThrowIf(nbands<1, "nbands should be > 0.");
    if (pAtmProfile) {
      vector<double> fC = fCenter.value;
      vector<double> fW = fWidth.value;
      vector<double> fR = fRes.value;
      if ( ((int)fC.size() != nbands) || ((int)fW.size() != nbands) ||
	   ((int)fR.size() != nbands) ) {
	*itsLog << LogIO::WARN
		<< "Dimensions of fCenter, fWidth and fRes != nbands!"
		<< LogIO::POST;
	return rstat;
      }
      vector<int> numChan(nbands);
      vector<int> refChan(nbands);
      vector<Frequency> refFreq(nbands);
      vector<Frequency> chanSep(nbands);
      Unit ufC(fCenter.units);
      Unit ufW(fWidth.units);
      Unit ufR(fRes.units);
      // Use BW = nchan * resolution = nchan * channel_separation
      for (int i = 0; i < nbands; i++) {
	if (fR[i] == 0) {
	  numChan[i] = 1;
	} else {
	  numChan[i] = (int)ceil((casacore::Quantity(fW[i],ufW) /
				  casacore::Quantity(fR[i],ufR)).getValue());
	}
	refChan[i] = numChan[i]/2;  // assume center channel is ref chan
	refFreq[i] = Frequency(fC[i],fCenter.units);
	chanSep[i] = Frequency(fR[i],fRes.units);
      }
      if (pSpectralGrid != 0) delete pSpectralGrid;
      pSpectralGrid = new SpectralGrid(numChan[0],refChan[0],
				       refFreq[0],chanSep[0]);
      for (int i = 1; i < nbands; i++) {
	pSpectralGrid->add(numChan[i],refChan[i],refFreq[i],chanSep[i]);
      }
      if (pRefractiveIndexProfile != 0) delete pRefractiveIndexProfile;
      pRefractiveIndexProfile = new RefractiveIndexProfile(*pSpectralGrid,
							   *pAtmProfile);
      if (pSkyStatus != 0) delete pSkyStatus;
      pSkyStatus = new SkyStatus(*pRefractiveIndexProfile);
      // WORK AROUND to set the 1st guess water column as a coefficient
      pSkyStatus->setUserWH2O(pSkyStatus->getGroundWH2O());
      rstat = numChan[0];
    } else {
      *itsLog << LogIO::WARN
	      << "Initialize atmospheric profile with initAtmProfile first."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

int
atmosphere::addSpectralWindow(const Quantity& fCenter,
		       const Quantity& fWidth, const Quantity& fRes)
{
  int rstat(-1);
  try {
    if (pSpectralGrid) {
      Unit ufC(fCenter.units);
      Unit ufW(fWidth.units);
      Unit ufR(fRes.units);
      // Use BW = nchan * resolution = nchan * channel_separation
      if (fRes.value[0] == 0) {
	*itsLog << LogIO::WARN << "Resolution of band cannot be 0,0 GHz!" << LogIO::POST;
	return rstat;
      }	
      int numChan = (int)ceil((casacore::Quantity(fWidth.value[0],ufW) /
			       casacore::Quantity(fRes.value[0],ufR)).getValue());
      int refChan = numChan/2;  // assume center channel is ref chan
      Frequency refFreq = Frequency(fCenter.value[0],fCenter.units);
      Frequency chanSep = Frequency(fRes.value[0],fRes.units);
      pSpectralGrid->add(numChan,refChan,refFreq,chanSep);
      if (pRefractiveIndexProfile != 0)	delete pRefractiveIndexProfile;
      pRefractiveIndexProfile = new RefractiveIndexProfile(*pSpectralGrid,*pAtmProfile);
      if (pSkyStatus != 0) delete pSkyStatus;
      pSkyStatus = new SkyStatus(*pRefractiveIndexProfile);
      // WORK AROUND to set the 1st guess water column as a coefficient
      pSkyStatus->setUserWH2O(pSkyStatus->getGroundWH2O());
      rstat = numChan;
    } else {
      *itsLog << LogIO::WARN
	      << "Initialize spectral window with initSpectralWindow first."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
//	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

int
atmosphere::getNumSpectralWindows()
{
  int rstat(-1);
  try {
    if (pSpectralGrid) {
      rstat = pSpectralGrid->getNumSpectralWindow();
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

int
atmosphere::getNumChan(int spwid)
{
  auto myfunc = (unsigned int(SpectralGrid::*)(unsigned int) const)&SpectralGrid::getNumChan;
  return DoSpGridSingleIdFuncInt(myfunc, spwid);
}

int
atmosphere::getRefChan(int spwid)
{
  auto myfunc = (unsigned int(SpectralGrid::*)(unsigned int) const)&SpectralGrid::getRefChan;
  return DoSpGridSingleIdFuncInt(myfunc, spwid);
}

/// a private helper function
int atmosphere::DoSpGridSingleIdFuncInt(SpGridSingleIdFuncInt func, int spwid)
{
	 int rstat(-1);
	  try {
	    if (pSpectralGrid) {
	      assert_spwid(spwid);
	      rstat = (pSpectralGrid->*func)(static_cast<unsigned int>(spwid));
	    } else {
	      *itsLog << LogIO::WARN
		      << "Please set spectral window(s) with initSpectralWindow."
		      << LogIO::POST;
	    }
	  } catch (AipsError x) {
	    //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	//	    << LogIO::POST;
	    RETHROW(x);
	  }
	  return rstat;
}

Quantity
atmosphere::getRefFreq(int spwid)
{
  std::string qunits("GHz");
  auto myfunc = (Frequency(SpectralGrid::*)(unsigned int) const)&SpectralGrid::getRefFreq;
  return DoSpGridSingleIdFuncQuantum(myfunc, spwid, qunits);
}


Quantity
atmosphere::getChanSep(int spwid)
{
  std::string qunits("MHz");
  auto myfunc = (Frequency(SpectralGrid::*)(unsigned int) const)&SpectralGrid::getChanSep;
  return DoSpGridSingleIdFuncQuantum(myfunc, spwid, qunits);
}

Quantity
atmosphere::getBandwidth(int spwid)
{
  std::string qunits("GHz");
  auto myfunc = (Frequency(SpectralGrid::*)(unsigned int) const)&SpectralGrid::getBandwidth;
  return DoSpGridSingleIdFuncQuantum(myfunc, spwid, qunits);
}

Quantity
atmosphere::getMinFreq(int spwid)
{
  std::string qunits("GHz");
  auto myfunc = (Frequency(SpectralGrid::*)(unsigned int) const)&SpectralGrid::getMinFreq;
  return DoSpGridSingleIdFuncQuantum(myfunc, spwid, qunits);
}

Quantity
atmosphere::getMaxFreq(int spwid)
{
  std::string qunits("GHz");
  auto myfunc = (Frequency(SpectralGrid::*)(unsigned int) const)&SpectralGrid::getMaxFreq;
  return DoSpGridSingleIdFuncQuantum(myfunc, spwid, qunits);
}

/// a private helper function
Quantity atmosphere::DoSpGridSingleIdFuncQuantum(SpGridSingleIdFuncFreq func, int spwid, string qunits)
{
  ::casac::Quantity q;
  try {
    if (pSpectralGrid) {
      assert_spwid(spwid);
      std::vector<double> qvalue(1);
      qvalue[0] = (pSpectralGrid->*func)(static_cast<unsigned int>(spwid)).get(qunits);
      q.value = qvalue;
      q.units = qunits;
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
//	    << LogIO::POST;
    RETHROW(x);
  }
  return q;
}

// This can't be merged now since SpectralGrid::getSpectralWindow returns vector
Quantity
atmosphere::getSpectralWindow(int spwid)
{
  std::string qunits("Hz");

  Quantity q;
  try {
    if (pSpectralGrid) {
      assert_spwid(spwid);
      q.value = pSpectralGrid->getSpectralWindow(static_cast<unsigned int>(spwid));
      q.units = "Hz";
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return q;
}

double
atmosphere::getChanNum(const Quantity& freq, int spwid)
{
  double rstat(-1.0);
  try {
    if (pSpectralGrid) {
      assert_spwid(spwid);
      rstat = pSpectralGrid->getChanNum(static_cast<unsigned int>(spwid),
					casaQuantity(freq).getValue("Hz"));
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
//	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

Quantity
atmosphere::getChanFreq(int chanNum, int spwid)
{
  ::casac::Quantity q;
  try {
    if (pSpectralGrid) {
      assert_spwid_and_channel(spwid, chanNum);
      std::vector<double> qvalue(1);
      std::string qunits("GHz");
      qvalue[0] = pSpectralGrid->getChanFreq(static_cast<unsigned int>(spwid),
					     static_cast<unsigned int>(chanNum)).get(qunits);
      q.value = qvalue;
      q.units = qunits;
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return q;
}


// a helper function to invoke ATM functions in RefractiveIndexProfile and SkyStatus classes
// for atmosphere functions which take two integer ids as paramters
template<typename Func, typename ClassType>
double atmosphere::doTwoIdATMFuncDouble(Func func, ClassType obj, int nc, int spwid)
{
  double out_data(-1.0);
  try {
    assert_spwid(spwid);
    if (obj) {//TODO: add check for obj class (allow only RIP and SS) or check for pSpectralGrid
      unsigned int chan;
      unsigned int spw = static_cast<unsigned int>(spwid);
      if (nc < 0) {
	chan = pSpectralGrid->getRefChan(spw);
	*itsLog << "Using reference channel " << chan << LogIO::POST;
      } else {
	chan = static_cast<unsigned int>(nc);
      }
      out_data = func(obj, spw,chan).get("neper");
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow first."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return out_data;
}

// a helper function to invoke ATM functions in RefractiveIndexProfile and SkyStatus classes
// for atmosphere functions which take two integer ids as paramters
template<typename Func, typename ClassType>
Quantity atmosphere::doTwoIdATMFuncQuantum(Func func, ClassType obj, int nc, int spwid, string units)
{
  ::casac::Quantity rtn;
  try {
    assert_spwid(spwid);
    if (obj) {//TODO: add check for obj class (allow only RIP and SS) or check for pSpectralGrid
      unsigned int chan;
      unsigned int spw = static_cast<unsigned int>(spwid);
      if (nc < 0) {
	chan = pSpectralGrid->getRefChan(spw);
	*itsLog << "Using reference channel " << chan << LogIO::POST;
      } else {
	chan = static_cast<unsigned int>(nc);
      }
      rtn.value.resize(1);
      rtn.units = units;
      rtn.value[0] = func(obj,spw,chan).get(rtn.units);
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow first."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return rtn;
}

double
atmosphere::getDryOpacity(int nc, int spwid)
{
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx) {
    return RIP->getDryOpacity(spw_idx, chan_idx);
  };
  return doTwoIdATMFuncDouble(myfunc, pRefractiveIndexProfile, nc, spwid);
}

double
atmosphere::getDryContOpacity(int nc, int spwid)
{
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx) {
    return RIP->getDryContOpacity(spw_idx, chan_idx);
  };
  return doTwoIdATMFuncDouble(myfunc, pRefractiveIndexProfile, nc, spwid);
}

double
atmosphere::getO2LinesOpacity(int nc, int spwid)
{
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx) {
    return RIP->getO2LinesOpacity(spw_idx, chan_idx);
  };
  return doTwoIdATMFuncDouble(myfunc, pRefractiveIndexProfile, nc, spwid);
}

double
atmosphere::getCOLinesOpacity(int nc, int spwid)
{
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx) {
    return RIP->getCOLinesOpacity(spw_idx, chan_idx);
  };
  return doTwoIdATMFuncDouble(myfunc, pRefractiveIndexProfile, nc, spwid);
}

double
atmosphere::getO3LinesOpacity(int nc, int spwid)
{
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx) {
    return RIP->getO3LinesOpacity(spw_idx, chan_idx);
  };
  return doTwoIdATMFuncDouble(myfunc, pRefractiveIndexProfile, nc, spwid);
}

double
atmosphere::getN2OLinesOpacity(int nc, int spwid)
{
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx) {
    return RIP->getN2OLinesOpacity(spw_idx, chan_idx);
  };
  return doTwoIdATMFuncDouble(myfunc, pRefractiveIndexProfile, nc, spwid);
}

Quantity
atmosphere::getWetOpacity(int nc, int spwid)
{
  std::string units("neper");
  auto myfunc = [](SkyStatus *SS, unsigned int spw_idx, unsigned int chan_idx) {
    return SS->getWetOpacity(spw_idx, chan_idx);
  };
  return doTwoIdATMFuncQuantum(myfunc, pSkyStatus, nc, spwid, units);
}

double
atmosphere::getH2OLinesOpacity(int nc, int spwid)
{
  auto myfunc = [](SkyStatus *SS, unsigned int spw_idx, unsigned int chan_idx) {
    return SS->getH2OLinesOpacity(spw_idx, chan_idx);
  };
  return doTwoIdATMFuncDouble(myfunc, pSkyStatus, nc, spwid);
}


double
atmosphere::getH2OContOpacity(int nc, int spwid)
{
  auto myfunc = [](SkyStatus *SS, unsigned int spw_idx, unsigned int chan_idx) {
    return SS->getH2OContOpacity(spw_idx, chan_idx);
  };
  return doTwoIdATMFuncDouble(myfunc, pSkyStatus, nc, spwid);
}

int
atmosphere::getDryOpacitySpec(int spwid, std::vector<double>& dryOpacity)
{
  int nchan(-1);
  try {
    assert_spwid(spwid);
    if (pRefractiveIndexProfile) {
      unsigned int num_chan = pSpectralGrid->getNumChan(spwid);
      nchan = static_cast<int>(num_chan);
      dryOpacity.resize(num_chan);
      unsigned int spw = static_cast<unsigned int>(spwid);
      for (unsigned int i = 0; i < num_chan; i++) {
	dryOpacity[i] =
	  pRefractiveIndexProfile->getDryOpacity(spw,i).get("neper");
      }
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow first."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return nchan;
}

int
atmosphere::getWetOpacitySpec(int spwid, Quantity& wetOpacity)
{
  int nchan(-1);
  try {
    assert_spwid(spwid);
    if (pSkyStatus) {
      unsigned int num_chan = pSpectralGrid->getNumChan(spwid);
      nchan = static_cast<int>(num_chan);
      (wetOpacity.value).resize(num_chan);
      wetOpacity.units="mm-1";
      unsigned int spw = static_cast<unsigned int>(spwid);
      for (int i = 0; i < nchan; i++) {
	(wetOpacity.value)[i] =
          pSkyStatus->getWetOpacity(spw,i).get(wetOpacity.units);
      }
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow first."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return nchan;
}

Quantity
atmosphere::getDispersivePhaseDelay(int nc, int spwid)
{
  std::string units("deg");
  auto myfunc = [](SkyStatus *SS, unsigned int spw_idx, unsigned int chan_idx) {
    return SS->getDispersiveH2OPhaseDelay(spw_idx, chan_idx);
  };
  return doTwoIdATMFuncQuantum(myfunc, pSkyStatus, nc, spwid, units);
}

Quantity
atmosphere::getDispersiveWetPhaseDelay(int nc, int spwid)
{
  std::string units("deg");
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx) {
    return RIP->getDispersiveH2OPhaseDelay(RIP->getGroundWH2O(), spw_idx, chan_idx);
  };
  return doTwoIdATMFuncQuantum(myfunc, pRefractiveIndexProfile, nc, spwid, units);
}

Quantity
atmosphere::getNonDispersiveWetPhaseDelay(int nc, int spwid)
{
  std::string units("deg");
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx) {
    return RIP->getNonDispersiveH2OPhaseDelay(RIP->getGroundWH2O(), spw_idx, chan_idx);
  };
  return doTwoIdATMFuncQuantum(myfunc, pRefractiveIndexProfile, nc, spwid, units);
}

Quantity
atmosphere::getNonDispersiveDryPhaseDelay(int nc, int spwid)
{
  std::string units("deg");
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx) {
    return RIP->getNonDispersiveDryPhaseDelay(spw_idx, chan_idx);
  };
  return doTwoIdATMFuncQuantum(myfunc, pRefractiveIndexProfile, nc, spwid, units);
}

Quantity
atmosphere::getDispersiveWetPathLength(int nc, int spwid)
{
  std::string units("m");
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx) {
    return RIP->getDispersiveH2OPathLength(RIP->getGroundWH2O(), spw_idx, chan_idx);
  };
  return doTwoIdATMFuncQuantum(myfunc, pRefractiveIndexProfile, nc, spwid, units);
}

Quantity
atmosphere::getNonDispersiveWetPathLength(int nc, int spwid)
{
  std::string units("m");
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx) {
    return RIP->getNonDispersiveH2OPathLength(RIP->getGroundWH2O(), spw_idx, chan_idx);
  };
  return doTwoIdATMFuncQuantum(myfunc, pRefractiveIndexProfile, nc, spwid, units);
}

Quantity
atmosphere::getNonDispersiveDryPathLength(int nc, int spwid)
{
  std::string units("m");
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx) {
    return RIP->getNonDispersiveDryPathLength(spw_idx, chan_idx);
  };
  return doTwoIdATMFuncQuantum(myfunc, pRefractiveIndexProfile, nc, spwid, units);
}

Quantity
atmosphere::getO2LinesPathLength(int nc, int spwid)
{
  std::string units("m");
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx) {
    return RIP->getO2LinesPathLength(spw_idx, chan_idx);
  };
  return doTwoIdATMFuncQuantum(myfunc, pRefractiveIndexProfile, nc, spwid, units);
}

Quantity
atmosphere::getO3LinesPathLength(int nc, int spwid)
{
  std::string units("m");
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx) {
    return RIP->getO3LinesPathLength(spw_idx, chan_idx);
  };
  return doTwoIdATMFuncQuantum(myfunc, pRefractiveIndexProfile, nc, spwid, units);
}

Quantity
atmosphere::getCOLinesPathLength(int nc, int spwid)
{
  std::string units("m");
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx) {
    return RIP->getCOLinesPathLength(spw_idx, chan_idx);
  };
  return doTwoIdATMFuncQuantum(myfunc, pRefractiveIndexProfile, nc, spwid, units);
}

Quantity
atmosphere::getN2OLinesPathLength(int nc, int spwid)
{
  std::string units("m");
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx) {
    return RIP->getN2OLinesPathLength(spw_idx, chan_idx);
  };
  return doTwoIdATMFuncQuantum(myfunc, pRefractiveIndexProfile, nc, spwid, units);
}

Quantity
atmosphere::getNonDispersivePhaseDelay(int nc, int spwid)
{
  std::string units("deg");
  auto myfunc = [](SkyStatus *SS, unsigned int spw_idx, unsigned int chan_idx) {
    return SS->getNonDispersiveH2OPhaseDelay(spw_idx, chan_idx);
  };
  return doTwoIdATMFuncQuantum(myfunc, pSkyStatus, nc, spwid, units);
}

Quantity
atmosphere::getDispersivePathLength(int nc, int spwid)
{
  std::string units("m");
  auto myfunc = [](SkyStatus *SS, unsigned int spw_idx, unsigned int chan_idx) {
    return SS->getDispersiveH2OPathLength(spw_idx, chan_idx);
  };
  return doTwoIdATMFuncQuantum(myfunc, pSkyStatus, nc, spwid, units);
}

Quantity
atmosphere::getNonDispersivePathLength(int nc, int spwid)
{
  std::string units("m");
  auto myfunc = [](SkyStatus *SS, unsigned int spw_idx, unsigned int chan_idx) {
    return SS->getNonDispersiveH2OPathLength(spw_idx, chan_idx);
  };
  return doTwoIdATMFuncQuantum(myfunc, pSkyStatus, nc, spwid, units);
}


Quantity
atmosphere::getAbsH2OLines(int nl, int nf, int spwid)
{
  std::string units = "m-1";
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx, unsigned int layer_idx) {
    return RIP->getAbsH2OLines(spw_idx, chan_idx, layer_idx);
  };
  return doRIPThreeIdFuncQuantum(myfunc, nl, nf, spwid, units);
}
  
Quantity
atmosphere::getAbsH2OCont(int nl, int nf, int spwid)
{
  std::string units = "m-1";
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx, unsigned int layer_idx) {
    return RIP->getAbsH2OCont(spw_idx, chan_idx, layer_idx);
  };
  return doRIPThreeIdFuncQuantum(myfunc, nl, nf, spwid, units);
}
  
Quantity
atmosphere::getAbsO2Lines(int nl, int nf, int spwid)
{
  std::string units = "m-1";
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx, unsigned int layer_idx) {
    return RIP->getAbsO2Lines(spw_idx, chan_idx, layer_idx);
  };
  return doRIPThreeIdFuncQuantum(myfunc, nl, nf, spwid, units);
}

Quantity
atmosphere::getAbsDryCont(int nl, int nf, int spwid)
{
  std::string units = "m-1";
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx, unsigned int layer_idx) {
    return RIP->getAbsDryCont(spw_idx, chan_idx, layer_idx);
  };
  return doRIPThreeIdFuncQuantum(myfunc, nl, nf, spwid, units);
}
  
Quantity
atmosphere::getAbsO3Lines(int nl, int nf, int spwid)
{
  std::string units = "m-1";
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx, unsigned int layer_idx) {
    return RIP->getAbsO3Lines(spw_idx, chan_idx, layer_idx);
  };
  return doRIPThreeIdFuncQuantum(myfunc, nl, nf, spwid, units);
}
  
Quantity
atmosphere::getAbsCOLines(int nl, int nf, int spwid)
{
  std::string units = "m-1";
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx, unsigned int layer_idx) {
    return RIP->getAbsCOLines(spw_idx, chan_idx, layer_idx);
  };
  return doRIPThreeIdFuncQuantum(myfunc, nl, nf, spwid, units);
}
  
Quantity
atmosphere::getAbsN2OLines(int nl, int nf, int spwid)
{
  std::string units = "m-1";
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx, unsigned int layer_idx) {
    return RIP->getAbsN2OLines(spw_idx, chan_idx, layer_idx);
  };
  return doRIPThreeIdFuncQuantum(myfunc, nl, nf, spwid, units);
}

Quantity
atmosphere::getAbsTotalDry(int nl, int nf, int spwid)
{
  std::string units = "m-1";
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx, unsigned int layer_idx) {
    return RIP->getAbsTotalDry(spw_idx, chan_idx, layer_idx);
  };
  return doRIPThreeIdFuncQuantum(myfunc, nl, nf, spwid, units);
}
 
Quantity
atmosphere::getAbsTotalWet(int nl, int nf, int spwid)
{
  std::string units = "m-1";
  auto myfunc = [](RefractiveIndexProfile *RIP, unsigned int spw_idx, unsigned int chan_idx, unsigned int layer_idx) {
    return RIP->getAbsTotalWet(spw_idx, chan_idx, layer_idx);
  };
  return doRIPThreeIdFuncQuantum(myfunc, nl, nf, spwid, units);
}

// a helper function
template<typename Func>
Quantity atmosphere::doRIPThreeIdFuncQuantum(Func func, int nl, int nf, int spwid, string units)
{
  Quantity rtn(std::vector<double> (1,-1.0), "");
  try {
    assert_unsigned_int(nl);
    assert_spwid_and_channel(spwid, nf);
    if (pRefractiveIndexProfile) {
      rtn.units = units;
      (rtn.value)[0] = func(pRefractiveIndexProfile,
			    static_cast<unsigned int>(spwid),
			    static_cast<unsigned int>(nf),
			    static_cast<unsigned int>(nl)).get(rtn.units);
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow first."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return rtn;
}


bool
atmosphere::setUserWH2O(const Quantity& wh2o)
{
  bool rstat(false);
  try {
    if (pSkyStatus) {
      Length new_wh2o(wh2o.value[0],wh2o.units);
      pSkyStatus->setUserWH2O(new_wh2o);
      rstat = true;
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow first."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
//	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

Quantity
atmosphere::getUserWH2O()
{
  ::casac::Quantity q;
  try {
    if (pSkyStatus) {
      q.value.resize(1);
      std::string qunits("mm");
      q.value[0]=pSkyStatus->getUserWH2O().get(qunits);
      q.units = qunits;
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow first."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return q;
}

bool
atmosphere::setAirMass(const double airmass)
{
  bool rstat(false);
  try {
    if (pSkyStatus) {
      pSkyStatus->setAirMass(airmass);
      rstat = true;
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow first."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

double
atmosphere::getAirMass()
{
  double m(-1.0);
  try {
    if (pSkyStatus) {
      m=pSkyStatus->getAirMass();
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow first."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return m;
}

bool
atmosphere::setSkyBackgroundTemperature(const Quantity& tbgr)
{
  bool rstat(false);
  try {
    if (pSkyStatus) {
      Temperature new_tbgr(tbgr.value[0],tbgr.units);
      pSkyStatus->setSkyBackgroundTemperature(new_tbgr);
      rstat = true;
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow first."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
//	    << LogIO::POST;
    RETHROW(x);
  }
  return rstat;
}

Quantity
atmosphere::getSkyBackgroundTemperature()
{
  ::casac::Quantity q;
  try {
    if (pSkyStatus) {
      q.value.resize(1);
      std::string qunits("K");
      q.value[0]=pSkyStatus->getSkyBackgroundTemperature().get(qunits);
      q.units = qunits;
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow first."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return q;
}

Quantity
atmosphere::getAverageTebbSky(int spwid, const Quantity& wh2o)
{
  ::casac::Quantity q;
  try {
    assert_spwid(spwid);
    if (pSkyStatus) {
      q.value.resize(1);
      std::string qunits("K");
      if (wh2o.value[0] == -1) {
	q.value[0]=pSkyStatus->getAverageTebbSky(static_cast<unsigned int>(spwid)).get(qunits);
      } else {
	Length new_wh2o(wh2o.value[0],wh2o.units);
	q.value[0]=pSkyStatus->getAverageTebbSky(static_cast<unsigned int>(spwid),
						 new_wh2o).get(qunits);
      }
      q.units = qunits;
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow first."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    //*itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
//	    << LogIO::POST;
    RETHROW(x);
  }
  return q;
}

Quantity
atmosphere::getTebbSky(int nc, int spwid, const Quantity& wh2o)
{
  ::casac::Quantity q;
  try {
    assert_spwid(spwid);
    if (pSkyStatus) {
      q.value.resize(1);
      std::string qunits("K");
      unsigned int chan;
      unsigned int spw = static_cast<unsigned int>(spwid);
      if (nc < 0) {
	chan = pSpectralGrid->getRefChan(spw);
	*itsLog << "Using reference channel " << chan << LogIO::POST;
      } else {
	chan = static_cast<unsigned int>(nc);
      }
      if (wh2o.value[0] == -1) {
	q.value[0]=pSkyStatus->getTebbSky(spw,chan).get(qunits);
      } else {
	Length new_wh2o(wh2o.value[0],wh2o.units);
	q.value[0]=pSkyStatus->getTebbSky(spw,chan,new_wh2o).get(qunits);
      }
      q.units = qunits;
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow first."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return q;
}

int
atmosphere::getTebbSkySpec(const int spwid, const Quantity& wh2o, Quantity& tebbSky)
{
  int nchan(0);
  try {
    assert_spwid(spwid);
    if (pSkyStatus) {
      bool userwh2o(false);
      unsigned int spw = static_cast<unsigned int>(spwid);
      unsigned int num_chan = pSpectralGrid->getNumChan(spw);
      nchan = static_cast<int>(num_chan);
      (tebbSky.value).resize(num_chan);
      tebbSky.units="K";
      Length new_wh2o;
      if (wh2o.value[0] != -1) {
	new_wh2o = Length(wh2o.value[0],wh2o.units);
	userwh2o = true;
      }
      for (unsigned int i = 0; i < num_chan; i++) {
	if (userwh2o) {
	(tebbSky.value)[i] =
	  pSkyStatus->getTebbSky(spw,i,new_wh2o).get(tebbSky.units);
	} else {
	(tebbSky.value)[i] =
	  pSkyStatus->getTebbSky(spw,i).get(tebbSky.units);
	}
      }
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow first."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return nchan;
}

Quantity
atmosphere::getAverageTrjSky(int spwid, const Quantity& wh2o)
{
  ::casac::Quantity q;
  try {
    assert_spwid(spwid);
    if (pSkyStatus) {
      q.value.resize(1);
      std::string qunits("K");
      if (wh2o.value[0] == -1) {
	q.value[0]=pSkyStatus->getAverageTrjSky(static_cast<unsigned int>(spwid)).get(qunits);
      } else {
	Length new_wh2o(wh2o.value[0],wh2o.units);
	q.value[0]=pSkyStatus->getAverageTrjSky(static_cast<unsigned int>(spwid),
						new_wh2o).get(qunits);
      }
      q.units = qunits;
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow first."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return q;
}

Quantity
atmosphere::getTrjSky(int nc, int spwid, const Quantity& wh2o)
{
  ::casac::Quantity q;
  try {
    assert_spwid(spwid);
    if (pSkyStatus) {
      q.value.resize(1);
      std::string qunits("K");
      unsigned int chan;
      unsigned int spw = static_cast<unsigned int>(spwid);
      if (nc < 0) {
	chan = pSpectralGrid->getRefChan(spw);
	*itsLog << "Using reference channel " << chan << LogIO::POST;
      } else {
	chan = static_cast<unsigned int>(nc);
      }
      if (wh2o.value[0] == -1) {
	q.value[0]=pSkyStatus->getTrjSky(spw,chan).get(qunits);
      } else {
	Length new_wh2o(wh2o.value[0],wh2o.units);
	q.value[0]=pSkyStatus->getTrjSky(spw,chan,new_wh2o).get(qunits);
      }
      q.units = qunits;
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow first."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return q;
}

int
atmosphere::getTrjSkySpec(const int spwid, const Quantity& wh2o, Quantity& trjSky)
{
  int nchan(0);
  try {
    assert_spwid(spwid);
    if (pSkyStatus) {
      bool userwh2o(false);
      unsigned int spw = static_cast<unsigned int>(spwid);
      unsigned int num_chan = pSpectralGrid->getNumChan(spw);
      nchan = static_cast<int>(num_chan);
      (trjSky.value).resize(num_chan);
      trjSky.units="K";
      Length new_wh2o;
      if (wh2o.value[0] != -1) {
	new_wh2o = Length(wh2o.value[0],wh2o.units);
	userwh2o = true;
      }
      for (unsigned int i = 0; i < num_chan; i++) {
	if (userwh2o) {
	(trjSky.value)[i] =
	  pSkyStatus->getTrjSky(spw,i,new_wh2o).get(trjSky.units);
	} else {
	(trjSky.value)[i] =
	  pSkyStatus->getTrjSky(spw,i).get(trjSky.units);
	}
      }
    } else {
      *itsLog << LogIO::WARN
	      << "Please set spectral window(s) with initSpectralWindow first."
	      << LogIO::POST;
    }
  } catch (AipsError x) {
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
    RETHROW(x);
  }
  return nchan;
}


} // casac namespace
