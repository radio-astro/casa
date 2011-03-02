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
using namespace std;

//#include <ATMSkyStatus.h>
//using namespace atm;

#include <casa/Logging/LogIO.h>
#include <casa/Exceptions/Error.h>
#include <atmosphere_cmpt.h>
#include <xmlcasa/StdCasa/CasacSupport.h>

using namespace atm;
using namespace std;
using namespace casa;

namespace casac {

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
			   const Quantity& h0, const int atmtype)
{
  string rtn;

  try {
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
atmosphere::initSpectralWindow(const int nbands, const Quantity& fCenter,
		       const Quantity& fWidth, const Quantity& fRes)
{
  int rstat(-1);
  try {
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
	  numChan[i] = (int)ceil((casa::Quantity(fW[i],ufW) /
				  casa::Quantity(fR[i],ufR)).getValue());
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
      int numChan = (int)ceil((casa::Quantity(fWidth.value[0],ufW) /
			       casa::Quantity(fRes.value[0],ufR)).getValue());
      int refChan = numChan/2;  // assume center channel is ref chan
      Frequency refFreq = Frequency(fCenter.value[0],fCenter.units);
      Frequency chanSep = Frequency(fRes.value[0],fRes.units);
      pSpectralGrid->add(numChan,refChan,refFreq,chanSep);
      if (pRefractiveIndexProfile != 0)	delete pRefractiveIndexProfile;
      pRefractiveIndexProfile = new RefractiveIndexProfile(*pSpectralGrid,*pAtmProfile);
      rstat = numChan;
    } else {
      *itsLog << LogIO::WARN
	      << "Initialize spectral window with initSpectralWindow first."
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
atmosphere::getNumChan(const int spwId)
{
  int rstat(-1);
  try {
    if (pSpectralGrid) {
      rstat = pSpectralGrid->getNumChan(spwId);
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
atmosphere::getRefChan(const int spwId)
{
  int rstat(-1);
  try {
    if (pSpectralGrid) {
      rstat = pSpectralGrid->getRefChan(spwId);
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

Quantity
atmosphere::getRefFreq(const int spwId)
{
  ::casac::Quantity q;
  try {
    if (pSpectralGrid) {
      std::vector<double> qvalue(1);
      std::string qunits("GHz");
      qvalue[0] = pSpectralGrid->getRefFreq(spwId).get(qunits);
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

Quantity
atmosphere::getChanSep(const int spwId)
{
  ::casac::Quantity q;
  try {
    if (pSpectralGrid) {
      std::vector<double> qvalue(1);
      std::string qunits("MHz");
      qvalue[0] = pSpectralGrid->getChanSep(spwId).get(qunits);
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

Quantity
atmosphere::getChanFreq(const int chanNum, const int spwId)
{
  ::casac::Quantity q;
  try {
    if (pSpectralGrid) {
      std::vector<double> qvalue(1);
      std::string qunits("GHz");
      qvalue[0] = pSpectralGrid->getChanFreq(spwId,chanNum).get(qunits);
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

Quantity
atmosphere::getSpectralWindow(const int spwId)
{
  Quantity q;
  try {
    if (pSpectralGrid) {
      q.value = pSpectralGrid->getSpectralWindow(spwId);
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
atmosphere::getChanNum(const Quantity& freq, const int spwId)
{
  double rstat(-1.0);
  try {
    if (pSpectralGrid) {
      rstat = pSpectralGrid->getChanNum(spwId,
					casaQuantity(freq).getValue("Hz"));
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

Quantity
atmosphere::getBandwidth(const int spwId)
{
  ::casac::Quantity q;
  try {
    if (pSpectralGrid) {
      std::vector<double> qvalue(1);
      std::string qunits("GHz");
      qvalue[0] = pSpectralGrid->getBandwidth(spwId).get(qunits);
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

Quantity
atmosphere::getMinFreq(const int spwId)
{
  ::casac::Quantity q;
  try {
    if (pSpectralGrid) {
      std::vector<double> qvalue(1);
      std::string qunits("GHz");
      qvalue[0] = pSpectralGrid->getMinFreq(spwId).get(qunits);
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

Quantity
atmosphere::getMaxFreq(const int spwId)
{
  ::casac::Quantity q;
  try {
    if (pSpectralGrid) {
      std::vector<double> qvalue(1);
      std::string qunits("GHz");
      qvalue[0] = pSpectralGrid->getMaxFreq(spwId).get(qunits);
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

double
atmosphere::getDryOpacity(const int nc, const int spwId)
{
  double dryOpacity(-1.0);
  try {
    if (pRefractiveIndexProfile) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      dryOpacity = pRefractiveIndexProfile->getDryOpacity(spwId,chan).get("neper");
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
  return dryOpacity;
}

double
atmosphere::getDryContOpacity(const int nc, const int spwId)
{
  double dryContOpacity(-1.0);
  try {
    if (pRefractiveIndexProfile) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      dryContOpacity = pRefractiveIndexProfile->getDryContOpacity(spwId,chan).get("neper");
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
  return dryContOpacity;
}


double
atmosphere::getO2LinesOpacity(const int nc, const int spwId)
{
  double o2LinesOpacity(-1.0);
  try {
    if (pRefractiveIndexProfile) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      o2LinesOpacity = pRefractiveIndexProfile->getO2LinesOpacity(spwId,chan).get("neper");
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
  return o2LinesOpacity;
}


double
atmosphere::getO3LinesOpacity(const int nc, const int spwId)
{
  double o3LinesOpacity(-1.0);
  try {
    if (pRefractiveIndexProfile) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      o3LinesOpacity = pRefractiveIndexProfile->getO3LinesOpacity(spwId,chan).get("neper");
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
  return o3LinesOpacity;
}


double
atmosphere::getCOLinesOpacity(const int nc, const int spwId)
{
  double coLinesOpacity(-1.0);
  try {
    if (pRefractiveIndexProfile) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      coLinesOpacity = pRefractiveIndexProfile->getCOLinesOpacity(spwId,chan).get("neper");
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
  return coLinesOpacity;
}


double
atmosphere::getN2OLinesOpacity(const int nc, const int spwId)
{
  double n2oLinesOpacity(-1.0);
  try {
    if (pRefractiveIndexProfile) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      n2oLinesOpacity = pRefractiveIndexProfile->getN2OLinesOpacity(spwId,chan).get("neper");
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
  return n2oLinesOpacity;
}


Quantity
atmosphere::getWetOpacity(const int nc, const int spwId)
{
  ::casac::Quantity wetOpacity;
  try {
    //if (pRefractiveIndexProfile) {
    if (pSkyStatus) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      wetOpacity.value.resize(1);
      wetOpacity.units = "neper";
      //wetOpacity.value[0] = pRefractiveIndexProfile->getWetOpacity(spwId,chan).get(wetOpacity.units);
      wetOpacity.value[0] = pSkyStatus->getWetOpacity(spwId,chan).get(wetOpacity.units);
      cout << "rip: " << wetOpacity.value[0] << " " << wetOpacity.units << " " << chan << endl;
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
  return wetOpacity;
}


double
atmosphere::getH2OLinesOpacity(const int nc, const int spwId)
{
  double h2oLinesOpacity(-1.0);
  try {
    if (pRefractiveIndexProfile) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      //h2oLinesOpacity = pRefractiveIndexProfile->getH2OLinesOpacity(spwId,chan).get("neper");
      h2oLinesOpacity = pSkyStatus->getH2OLinesOpacity(spwId,chan).get("neper");
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
  return h2oLinesOpacity;
}


double
atmosphere::getH2OContOpacity(const int nc, const int spwId)
{
  double h2oContOpacity(-1.0);
  try {
    if (pRefractiveIndexProfile) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      //h2oContOpacity = pRefractiveIndexProfile->getH2OContOpacity(spwId,chan).get("neper");
      h2oContOpacity = pSkyStatus->getH2OContOpacity(spwId,chan).get("neper");
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
  return h2oContOpacity;
}


int
atmosphere::getDryOpacitySpec(std::vector<double>& dryOpacity, const int spwId)
{
  int nchan(-1);
  try {
    if (pRefractiveIndexProfile) {
      nchan = pSpectralGrid->getNumChan(spwId);
      dryOpacity.resize(nchan);
      for (int i = 0; i < nchan; i++) {
	dryOpacity[i] =
	  pRefractiveIndexProfile->getDryOpacity(spwId,i).get("neper");
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
atmosphere::getWetOpacitySpec(Quantity& wetOpacity, const int spwId)
{
  int nchan(-1);
  try {
    //if (pRefractiveIndexProfile) {
    if (pSkyStatus) {
      nchan = pSpectralGrid->getNumChan(spwId);
      (wetOpacity.value).resize(nchan);
      wetOpacity.units="mm-1";
      for (int i = 0; i < nchan; i++) {
	(wetOpacity.value)[i] =
	  //pRefractiveIndexProfile->getWetOpacity(spwId,i).get(wetOpacity.units);
          pSkyStatus->getWetOpacity(spwId,i).get(wetOpacity.units);
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
atmosphere::getDispersivePhaseDelay(const int nc, const int spwId)
{
  ::casac::Quantity dpd;
  try {
    if (pSkyStatus) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      dpd.value.resize(1);
      std::string units("deg");
      dpd.value[0] = pSkyStatus->getDispersiveH2OPhaseDelay(spwId,chan).get(units);
      dpd.units = units;
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
  return dpd;
}

Quantity
atmosphere::getDispersiveWetPhaseDelay(const int nc, const int spwId)
{
  ::casac::Quantity dwpd;
  try {
    if (pRefractiveIndexProfile) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      dwpd.value.resize(1);
      std::string units("deg");
      //dwpd.value[0] = pRefractiveIndexProfile->getDispersiveH2OPhaseDelay(spwId,chan).get(units);
      dwpd.value[0] = pRefractiveIndexProfile->getDispersiveH2OPhaseDelay(pRefractiveIndexProfile->getGroundWH2O(),spwId,chan).get(units);
      dwpd.units = units;
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
  return dwpd;
}

Quantity
atmosphere::getNonDispersiveWetPhaseDelay(const int nc, const int spwId)
{
  ::casac::Quantity ndwpd;
  try {
    if (pRefractiveIndexProfile) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      ndwpd.value.resize(1);
      std::string units("deg");
      //ndwpd.value[0] = pRefractiveIndexProfile->getNonDispersiveH2OPhaseDelay(spwId,chan).get(units);
      ndwpd.value[0] = pRefractiveIndexProfile->getNonDispersiveH2OPhaseDelay(pRefractiveIndexProfile->getGroundWH2O(),spwId,chan).get(units);
      ndwpd.units = units;
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
  return ndwpd;
}

Quantity
atmosphere::getNonDispersiveDryPhaseDelay(const int nc, const int spwId)
{
  ::casac::Quantity nddpd;
  try {
    if (pRefractiveIndexProfile) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      nddpd.value.resize(1);
      std::string units("deg");
      nddpd.value[0] = pRefractiveIndexProfile->getNonDispersiveDryPhaseDelay(spwId,chan).get(units);
      nddpd.units = units;
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
  return nddpd;
}

Quantity
atmosphere::getNonDispersivePhaseDelay(const int nc, const int spwId)
{
  ::casac::Quantity ndpd;
  try {
    if (pSkyStatus) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      ndpd.value.resize(1);
      std::string units("deg");
      ndpd.value[0] = pSkyStatus->getNonDispersiveH2OPhaseDelay(spwId,chan).get(units);
      ndpd.units = units;
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
  return ndpd;
}


Quantity
atmosphere::getDispersivePathLength(const int nc, const int spwId)
{
  ::casac::Quantity dpl;
  try {
    if (pSkyStatus) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      dpl.value.resize(1);
      std::string units("m");      
      dpl.value[0] = pSkyStatus->getDispersiveH2OPathLength(spwId,chan).get(units);
      // Remy's test:
      //      std::string units("neper");      
      //      dpl.value[0] = pSkyStatus->getWetOpacity(chan).get(units);

      dpl.units = units;
      //      cout << "sst: " << dpl.value[0] << " " << dpl.units << " " << chan << endl;

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
  return dpl;
}

Quantity
atmosphere::getDispersiveWetPathLength(const int nc, const int spwId)
{
  ::casac::Quantity dwpl;
  try {
    if (pRefractiveIndexProfile) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      dwpl.value.resize(1);
      std::string units("m");
      //dwpl.value[0] = pRefractiveIndexProfile->getDispersiveH2OPathLength(spwId,chan).get(units);
      dwpl.value[0] = pRefractiveIndexProfile->getDispersiveH2OPathLength(pRefractiveIndexProfile->getGroundWH2O(),spwId,chan).get(units);
      dwpl.units = units;
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
  return dwpl;
}

Quantity
atmosphere::getNonDispersiveWetPathLength(const int nc, const int spwId)
{
  ::casac::Quantity ndwpl;
  try {
    if (pRefractiveIndexProfile) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      ndwpl.value.resize(1);
      std::string units("m");
      //ndwpl.value[0] = pRefractiveIndexProfile->getNonDispersiveH2OPathLength(spwId,chan).get(units);
      ndwpl.value[0] = pRefractiveIndexProfile->getNonDispersiveH2OPathLength(pRefractiveIndexProfile->getGroundWH2O(),spwId,chan).get(units);
      ndwpl.units = units;
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
  return ndwpl;
}

Quantity
atmosphere::getNonDispersiveDryPathLength(const int nc, const int spwId)
{
  ::casac::Quantity nddpl;
  try {
    if (pRefractiveIndexProfile) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      nddpl.value.resize(1);
      std::string units("m");
      nddpl.value[0] = pRefractiveIndexProfile->getNonDispersiveDryPathLength(spwId,chan).get(units);
      nddpl.units = units;
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
  return nddpl;
}


Quantity
atmosphere::getO2LinesPathLength(const int nc, const int spwId)
{
  ::casac::Quantity o2pl;
  try {
    if (pRefractiveIndexProfile) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      o2pl.value.resize(1);
      std::string units("m");
      o2pl.value[0] = pRefractiveIndexProfile->getO2LinesPathLength(spwId,chan).get(units);
      o2pl.units = units;
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
  return o2pl;
}

Quantity
atmosphere::getO3LinesPathLength(const int nc, const int spwId)
{
  ::casac::Quantity o3pl;
  try {
    if (pRefractiveIndexProfile) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      o3pl.value.resize(1);
      std::string units("m");
      o3pl.value[0] = pRefractiveIndexProfile->getO3LinesPathLength(spwId,chan).get(units);
      o3pl.units = units;
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
  return o3pl;
}

Quantity
atmosphere::getCOLinesPathLength(const int nc, const int spwId)
{
  ::casac::Quantity COpl;
  try {
    if (pRefractiveIndexProfile) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      COpl.value.resize(1);
      std::string units("m");
      COpl.value[0] = pRefractiveIndexProfile->getCOLinesPathLength(spwId,chan).get(units);
      COpl.units = units;
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
  return COpl;
}

Quantity
atmosphere::getN2OLinesPathLength(const int nc, const int spwId)
{
  ::casac::Quantity N2Opl;
  try {
    if (pRefractiveIndexProfile) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      N2Opl.value.resize(1);
      std::string units("m");
      N2Opl.value[0] = pRefractiveIndexProfile->getN2OLinesPathLength(spwId,chan).get(units);
      N2Opl.units = units;
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
  return N2Opl;
}


Quantity
atmosphere::getNonDispersivePathLength(const int nc, const int spwId)
{
  ::casac::Quantity ndpl;
  try {
    if (pSkyStatus) {
      int chan;
      if (nc < 0)
	chan = pSpectralGrid->getRefChan(spwId);
      else
	chan = nc;
      ndpl.value.resize(1);
      std::string units("m");
      ndpl.value[0] = pSkyStatus->getNonDispersiveH2OPathLength(spwId,chan).get(units);
      ndpl.units = units;
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
  return ndpl;
}


Quantity
atmosphere::getAbsH2OLines(int nl, int nf, const int spwid)
{
  Quantity rtn(std::vector<double> (1,-1.0), "");
  try {
    if (pRefractiveIndexProfile) {
      rtn.units = "m-1";
      (rtn.value)[0] = pRefractiveIndexProfile->getAbsH2OLines(spwid, nf, nl).get(rtn.units);
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
  

Quantity
atmosphere::getAbsH2OCont(int nl, int nf, const int spwid)
{
  Quantity rtn(std::vector<double> (1,-1.0), "");
  try {
    if (pRefractiveIndexProfile) {
      rtn.units = "m-1";
      (rtn.value)[0] = pRefractiveIndexProfile->getAbsH2OCont(spwid, nf, nl).get(rtn.units);
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
  

Quantity
atmosphere::getAbsO2Lines(int nl, int nf, const int spwid)
{
  Quantity rtn(std::vector<double> (1,-1.0), "");
  try {
    if (pRefractiveIndexProfile) {
      rtn.units = "m-1";
      (rtn.value)[0] = pRefractiveIndexProfile->getAbsO2Lines(spwid, nf, nl).get(rtn.units);
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
  

Quantity
atmosphere::getAbsDryCont(int nl, int nf, const int spwid)
{
  Quantity rtn(std::vector<double> (1,-1.0), "");
  try {
    if (pRefractiveIndexProfile) {
      rtn.units = "m-1";
      (rtn.value)[0] = pRefractiveIndexProfile->getAbsDryCont(spwid, nf, nl).get(rtn.units);
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
  

Quantity
atmosphere::getAbsO3Lines(int nl, int nf, const int spwid)
{
  Quantity rtn(std::vector<double> (1,-1.0), "");
  try {
    if (pRefractiveIndexProfile) {
      rtn.units = "m-1";
      (rtn.value)[0] = pRefractiveIndexProfile->getAbsO3Lines(spwid, nf, nl).get(rtn.units);
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
  

Quantity
atmosphere::getAbsCOLines(int nl, int nf, const int spwid)
{
  Quantity rtn(std::vector<double> (1,-1.0), "");
  try {
    if (pRefractiveIndexProfile) {
      rtn.units = "m-1";
      (rtn.value)[0] = pRefractiveIndexProfile->getAbsCOLines(spwid, nf, nl).get(rtn.units);
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
  

Quantity
atmosphere::getAbsN2OLines(int nl, int nf, const int spwid)
{
  Quantity rtn(std::vector<double> (1,-1.0), "");
  try {
    if (pRefractiveIndexProfile) {
      rtn.units = "m-1";
      (rtn.value)[0] = pRefractiveIndexProfile->getAbsN2OLines(spwid, nf, nl).get(rtn.units);
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
  

Quantity
atmosphere::getAbsTotalDry(int nl, int nf, const int spwid)
{
  Quantity rtn(std::vector<double> (1,-1.0), "");
  try {
    if (pRefractiveIndexProfile) {
      rtn.units = "m-1";
      (rtn.value)[0] = pRefractiveIndexProfile->getAbsTotalDry(spwid, nf, nl).get(rtn.units);
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
  

Quantity
atmosphere::getAbsTotalWet(int nl, int nf, const int spwid)
{
  Quantity rtn(std::vector<double> (1,-1.0), "");
  try {
    if (pRefractiveIndexProfile) {
      rtn.units = "m-1";
      (rtn.value)[0] = pRefractiveIndexProfile->getAbsTotalWet(spwid, nf, nl).get(rtn.units);
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
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
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
    *itsLog << LogIO::SEVERE << "Exception Reported: " << x.getMesg()
	    << LogIO::POST;
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
atmosphere::getAverageTebbSky(const int spwid, const Quantity& wh2o)
{
  ::casac::Quantity q;
  try {
    if (pSkyStatus) {
      q.value.resize(1);
      std::string qunits("K");
      if (wh2o.value[0] == -1) {
	q.value[0]=pSkyStatus->getAverageTebbSky((unsigned int)spwid).get(qunits);
      } else {
	Length new_wh2o(wh2o.value[0],wh2o.units);
	q.value[0]=pSkyStatus->getAverageTebbSky(spwid,new_wh2o).get(qunits);
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
atmosphere::getTebbSky(const int nc, const int spwid, const Quantity& wh2o)
{
  ::casac::Quantity q;
  try {
    if (pSkyStatus) {
      q.value.resize(1);
      std::string qunits("K");
      int chan;
      if (nc < 0) {
	chan = pSpectralGrid->getRefChan(spwid);
      } else {
	chan = nc;
      }
      if (wh2o.value[0] == -1) {
	q.value[0]=pSkyStatus->getTebbSky((unsigned int)spwid,(unsigned int)chan).get(qunits);
      } else {
	Length new_wh2o(wh2o.value[0],wh2o.units);
	q.value[0]=pSkyStatus->getTebbSky(spwid,chan,new_wh2o).get(qunits);
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

} // casac namespace
