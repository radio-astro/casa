#if     !defined(ATM_SKYSTATUS_H)

#include "ATMWaterVaporRadiometer.h"
#include "ATMWVRMeasurement.h"
#include "ATMRefractiveIndexProfile.h"
#include <math.h>

namespace atm {
  /*! \brief From the layerThickness and layerTemperature arrays (from AtmProfile),  
   *   the RefractiveIndexProfile array, and a brightness temperature measured to 
   *   the sky at the corresponding frequency, this Class retrieves the water vapor 
   *   column that corresponds to the measurement.
   *
   *   The strating point will therefore be an RefractiveIndexProfile object form 
   *   which the essential information will be inherited: layerThickness, 
   *   layerTemperature vectros (these two inherited themselves from AtmProfile), 
   *   and absTotalDry and absTotalWet arrays. The numerical methods  
   *   will be an iteration wh2o to match the measured brightness temperature
   */
  class SkyStatus: public RefractiveIndexProfile 
    {
    public:
      
      //@{
      
      /** The basic constructor. It will allow to perform forward radiative transfer enquires in 
       *  the spectral bands of refractiveIndexProfile, and to use some of these bands for water vapor retrievals. 
       *  The basic constructor needs at least an  RefractiveIndexProfile object (it brings the spectral information via the 
       *  SpectralGrid object inherited by it). Additional inputs can be the 
       *  Air mass (default 1.0), sky background temperature (default 2.73 K), and user water vapor column (default 1 mm). 
       *  Methods inside the class allow to retrive the water vapor column and eventually update the user value to the retrieved one.
       *  In order to perform such retrievals, the numerical methods will need as inputs the measured T_EBBs towards the sky in 
       *  some spectral bands that will be designed as water vapor radiometry channels. The sideband gains and coupling to the sky 
       *  of these vapor radiometry channels should also be inputs of the retrieval methods, otherwise they will be set to default 
       *  values. See documentation ef particular methods for more information.
       */
      SkyStatus(RefractiveIndexProfile refractiveIndexProfile);
      /** Class constructor with additional inputs */
      SkyStatus(RefractiveIndexProfile refractiveIndexProfile, double airMass);   
      /** Class constructor with additional inputs */
      SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Temperature temperatureBackground);   
      /** Class constructor with additional inputs */
      SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Length wh2o);   
      /** Class constructor with additional inputs */
      SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Temperature temperatureBackground, double airMass);   
      /** Class constructor with additional inputs */
      SkyStatus(RefractiveIndexProfile refractiveIndexProfile, double airMass, Temperature temperatureBackground);   
      /** Class constructor with additional inputs */
      SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Length wh2o, double airMass);   
      /** Class constructor with additional inputs */
      SkyStatus(RefractiveIndexProfile refractiveIndexProfile, double airMass, Length wh2o);   
      /** Class constructor with additional inputs */
      SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Length wh2o, Temperature temperatureBackground);   
      /** Class constructor with additional inputs */
      SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Temperature temperatureBackground, Length wh2o);   
      /** Class constructor with additional inputs */
      SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Temperature temperatureBackground, double airMass, Length wh2o);   
      /** Class constructor with additional inputs */
      SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Temperature temperatureBackground, Length wh2o, double airMass);
      /** Class constructor with additional inputs */
      SkyStatus(RefractiveIndexProfile refractiveIndexProfile, double airMass, Temperature temperatureBackground, Length wh2o);   
      /** Class constructor with additional inputs */
      SkyStatus(RefractiveIndexProfile refractiveIndexProfile, double airMass, Length wh2o, Temperature temperatureBackground);   
      /** Class constructor with additional inputs */
      SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Length wh2o, Temperature temperatureBackground, double airMass);   
      /** Class constructor with additional inputs */
      SkyStatus(RefractiveIndexProfile refractiveIndexProfile, Length wh2o, double airMass, Temperature temperatureBackground);   
      
      /** A copy constructor for deep copy   */
      SkyStatus( const SkyStatus &);
      
      
      virtual ~SkyStatus();
      
      
      //@}
      
      //@{
      
      
      
      
      /** Accessor to get the user water vapor column. This is the water vapor column used for forward 
	  radiative transfer calculations. It can be equal to wh2o_retrieved_ when this one is avalilable 
	  and the proper setter or update accessor (retrieveandupdateWaterVapor) has been used. */
      Length getUserWH2O() {return wh2o_user_;}
      /** Setter for user zenith water vapor column for forward radiative transfer calculations. The user 
	  zenith water vapor column equals the retrieved zenith water vapor column from H2O radiometers, 
	  every time the last one is derived with the retrieveandupdateWaterVapor accessor.*/
      void   setUserWH2O(Length wh2o){ if(wh2o.get()==wh2o_user_.get()){     }else{ wh2o_user_=wh2o; } }
      /** Alternative form of the setter for user zenith water vapor column for forward radiative transfer calculations. The user 
	  zenith water vapor column equals the retrieved zenith water vapor column from H2O radiometers, 
	  every time the last one is derived with the retrieveandupdateWaterVapor accessor.*/
      void   setUserWH2O(double dwh2o, string units){Length wh2o(dwh2o,units); if(wh2o.get()==wh2o_user_.get()){     }else{ wh2o_user_=wh2o; } }
      /** Accessor to get airmass */
      double getAirMass() {return airMass_;}
      /** Setter for air mass in SkyStatus without performing water vapor retrieval */
      void  setAirMass(double airMass){ if(airMass==airMass_){ }else{ airMass_ = airMass;} }
      /** Accessor to get sky background temperature */
      Temperature getSkyBackgroundTemperature() {return skyBackgroundTemperature_;}
      /** Setter for sky background temperature in SkyStatus without 
	  performing water vapor retrieval */
      void  setSkyBackgroundTemperature(Temperature skyBackgroundTemperature){     
	if(skyBackgroundTemperature.get("K")==skyBackgroundTemperature_.get("K")){
	}else{skyBackgroundTemperature_ = Temperature(skyBackgroundTemperature.get("K"),"K"); } }
      
   
      
    

      /** Accesor to the average Equivalent Blackbody Temperature in spectral window 0, for the current conditions 
       and a perfect sky coupling */
      Temperature getAverageTebbSky(){unsigned int n=0; return getAverageTebbSky(n);}
      /** Accesor to the average Equivalent Blackbody Temperature in spectral window spwid, for the current conditions 
       and a perfect sky coupling */
      Temperature getAverageTebbSky(unsigned int spwid){
	return getAverageTebbSky(spwid,getUserWH2O(),getAirMass(),1.0,getGroundTemperature());}
     /** Accesor to the average Equivalent Blackbody Temperature in spectral window 0, for the current 
	 conditions, except water column wh2o, and a perfect sky coupling */
      Temperature getAverageTebbSky(Length wh2o){return getAverageTebbSky(0,wh2o);}
     /** Accesor to the average Equivalent Blackbody Temperature in spectral window spwid, for the current 
	 conditions, except water column wh2o, and a perfect sky coupling */
      Temperature getAverageTebbSky(unsigned int spwid, Length wh2o){
	return getAverageTebbSky(spwid,wh2o,getAirMass(),1.0,getGroundTemperature());}
     /** Accesor to the average Equivalent Blackbody Temperature in spectral window 0, for the current 
	 conditions, except Air Mass airmass, and a perfect sky coupling */
      Temperature getAverageTebbSky(double airmass){return getAverageTebbSky(0,airmass);}
     /** Accesor to the average Equivalent Blackbody Temperature in spectral window spwid, for the current 
	 conditions, except Air Mass airmass, and a perfect sky coupling */
      Temperature getAverageTebbSky(unsigned int spwid, double airmass){
	return getAverageTebbSky(spwid,getUserWH2O(),airmass,1.0,getGroundTemperature());}
     /** Accesor to the average Equivalent Blackbody Temperature in spectral window 0, for the current 
	 conditions, except water column wh2o, introducing a Sky Coupling and Spill Over Temperature */
      Temperature getAverageTebbSky(Length wh2o, double skycoupling, Temperature Tspill){
	return getAverageTebbSky(0,wh2o,skycoupling,Tspill);}
     /** Accesor to the average Equivalent Blackbody Temperature in spectral window spwid, for the current 
	 conditions, except water column wh2o, introducing a Sky Coupling and Spill Over Temperature */
      Temperature getAverageTebbSky(unsigned int spwid, Length wh2o, double skycoupling, Temperature Tspill){
	return getAverageTebbSky(spwid,wh2o,getAirMass(),skycoupling,Tspill);}
     /** Accesor to the average Equivalent Blackbody Temperature in spectral window 0, for the current 
	 conditions, except Air Mass airmass, introducing a Sky Coupling and Spill Over Temperature */
      Temperature getAverageTebbSky(double airmass, double skycoupling, Temperature Tspill){
	return getAverageTebbSky(0,airmass,skycoupling,Tspill);}
     /** Accesor to the average Equivalent Blackbody Temperature in spectral window spwid, for the current 
	 conditions, except Air Mass airmass, introducing a Sky Coupling and Spill Over Temperature */
      Temperature getAverageTebbSky(unsigned int spwid, double airmass, double skycoupling, Temperature Tspill){
	return getAverageTebbSky(spwid,getUserWH2O(),airmass,skycoupling,Tspill);}
     /** Accesor to the average Equivalent Blackbody Temperature in spectral window 0 for the current 
	 conditions, introducing a Sky Coupling and Spill Over Temperature */
      Temperature getAverageTebbSky(double skycoupling, Temperature Tspill){unsigned int n=0; 
	return getAverageTebbSky(n,skycoupling,Tspill);}
     /** Accesor to the average Equivalent Blackbody Temperature in spectral window spwid for the current 
	 conditions, introducing a Sky Coupling and Spill Over Temperature */
      Temperature getAverageTebbSky(unsigned int spwid, double skycoupling, Temperature Tspill){
	return getAverageTebbSky(spwid,getUserWH2O(),getAirMass(),skycoupling,Tspill);}
     /** Accesor to the average Equivalent Blackbody Temperature in spectral window spwid, for Water Column 
	 wh2o, Air Mass airmass, Sky Coupling skycoupling, and Spill Over Temperature Tspill */
      Temperature getAverageTebbSky(unsigned int spwid, Length wh2o, double airmass, double skycoupling, Temperature Tspill);


      
     /** Accesor to the Equivalent Blackbody Temperature in spectral window 0 and channel 0, for the currnet 
	 (user) Water Vapor Column, the current Air Mass, and perfect Sky Coupling to the sky */
      Temperature getTebbSky() {unsigned int n=0; return getTebbSky(n); }
     /** Accesor to the Equivalent Blackbody Temperature in spectral window 0 and channel nc, for the currnet 
	 (user) Water Vapor Column, the current Air Mass, and perfect Sky Coupling to the sky */
      Temperature getTebbSky(unsigned int nc){unsigned int n=0; return getTebbSky(n, nc);}
     /** Accesor to the Equivalent Blackbody Temperature in spectral window spwid and channel nc, for the currnet 
	 (user) Water Vapor Column, the current Air Mass, and perfect Sky Coupling to the sky */
      Temperature getTebbSky(unsigned int spwid, unsigned int nc){return getTebbSky(spwid, nc,  getUserWH2O(), getAirMass(), 1.0, Temperature(100,"K"));}
      /** Accesor to the Equivalent Blackbody Temperature in spectral window 0 and channel 0, for the currnet 
	  (user) Water Vapor Column, Air Mass airmass, and perfect Sky Coupling to the sky */
      Temperature getTebbSky(double airmass){unsigned int n=0; return getTebbSky(n,airmass);}
      /** Accesor to the Equivalent Blackbody Temperature in spectral window 0 and channel nc, for the currnet 
	  (user) Water Vapor Column, Air Mass airmass, and perfect Sky Coupling to the sky */
      Temperature getTebbSky(unsigned int nc, double airmass){unsigned int n=0; return getTebbSky(n,nc,airmass);}
     /** Accesor to the Equivalent Blackbody Temperature in spectral window spwid and channel nc, for the currnet 
	 (user) Water Vapor Column, Air Mass airmass, and perfect Sky Coupling to the sky */
      Temperature getTebbSky(unsigned int spwid, unsigned int nc, double airmass){return getTebbSky(spwid, nc, getUserWH2O(), airmass, 1.0, Temperature(100,"K"));}
     /** Accesor to the Equivalent Blackbody Temperature in spectral window 0 and channel 0, for Water  
	 Vapor Column wh2o, the current Air Mass, and perfect Sky Coupling to the sky */
      Temperature getTebbSky(Length wh2o){unsigned int n=0; return getTebbSky(n, wh2o);}
     /** Accesor to the Equivalent Blackbody Temperature in spectral window 0 and channel nc, for Water  
	 Vapor Column wh2o, the current Air Mass, and perfect Sky Coupling to the sky */
      Temperature getTebbSky(unsigned int nc, Length wh2o){unsigned int n=0; return getTebbSky(n, nc, wh2o);}
     /** Accesor to the Equivalent Blackbody Temperature in spectral window spwid and channel nc, for Water  
	 Vapor Column wh2o, the current Air Mass, and perfect Sky Coupling to the sky */
      Temperature getTebbSky(unsigned int spwid, unsigned int nc, Length wh2o){return getTebbSky(spwid, nc, wh2o, getAirMass(), 1.0, Temperature(100,"K"));}
     /** Accesor to the Equivalent Blackbody Temperature in spectral window 0 and channel nc, for the current 
	 (user) Water Vapor Column and Air Mass, Sky Coupling skycoupling, and Spill Over Temperature Tspill */
      Temperature getTebbSky(unsigned int nc, double skycoupling, Temperature Tspill){unsigned int n=0; 
	return getTebbSky(n, nc, skycoupling, Tspill);}				
     /** Accesor to the Equivalent Blackbody Temperature in spectral window spwid and channel nc, for the current 
	 (user) Water Vapor Column and Air Mass, Sky Coupling skycoupling, and Spill Over Temperature Tspill */
      Temperature getTebbSky(unsigned int spwid, unsigned int nc, double skycoupling, Temperature Tspill){
	return getTebbSky(spwid, nc, getUserWH2O(), getAirMass(), skycoupling, Tspill);}						       
      /** Accesor to the Equivalent Blackbody Temperature in spectral window 0 and channel nc, for Water  
	 Vapor Column wh2o, Air Mass airmass, and perfect Sky Coupling to the sky */
      Temperature getTebbSky(unsigned int nc, Length wh2o, double airmass){unsigned int n=0; return getTebbSky(n, nc,  wh2o, airmass);}
      /** Accesor to the Equivalent Blackbody Temperature in spectral window spwid and channel nc, for Water  
	 Vapor Column wh2o, Air Mass airmass, and perfect Sky Coupling to the sky */
     Temperature getTebbSky(unsigned int spwid, unsigned int nc, Length wh2o, double airmass){
	return getTebbSky(spwid, nc,  wh2o, airmass, 1.0, Temperature(100,"K"));}
     /** Accesor to the Equivalent Blackbody Temperature in spectral window 0 and channel nc, for Water  
	 Vapor Column wh2o, the current Air Mass, Sky Coupling skycoupling, and Spill Over Temperature Tspill */
      Temperature getTebbSky(unsigned int nc, Length wh2o, double skycoupling, Temperature Tspill){unsigned int n=0; 
	return getTebbSky(n, nc, wh2o, skycoupling, Tspill);}
     /** Accesor to the Equivalent Blackbody Temperature in spectral window spwid and channel nc, for Water  
	 Vapor Column wh2o, the current Air Mass, Sky Coupling skycoupling, and Spill Over Temperature Tspill */
      Temperature getTebbSky(unsigned int spwid, unsigned int nc, Length wh2o, double skycoupling, Temperature Tspill){
	return getTebbSky(spwid, nc, wh2o, getAirMass(), skycoupling, Tspill);}
     /** Accesor to the Equivalent Blackbody Temperature in spectral window 0 and channel nc, for the current 
	 User Water Column, Air Mass airmass, Sky Coupling skycoupling, and Spill Over Temperature Tspill */
      Temperature getTebbSky(unsigned int nc, double airmass, double skycoupling, Temperature Tspill){unsigned int n=0;
	return getTebbSky(n, nc, airmass, skycoupling, Tspill);}
     /** Accesor to the Equivalent Blackbody Temperature in spectral window spwid and channel nc, for the current 
	 User Water Column, Air Mass airmass, Sky Coupling skycoupling, and Spill Over Temperature Tspill */
      Temperature getTebbSky(unsigned int spwid, unsigned int nc, double airmass, double skycoupling, Temperature Tspill){
	return getTebbSky(spwid, nc, getUserWH2O(), airmass, skycoupling, Tspill);}
     /** Accesor to the Equivalent Blackbody Temperature in spectral window 0 and channel nc, for Water Column 
	 wh2o, Air Mass airmass, Sky Coupling skycoupling, and Spill Over Temperature Tspill */
      Temperature getTebbSky(unsigned int nc, Length wh2o, double airmass, double skycoupling, Temperature Tspill){unsigned int n=0;
	return getTebbSky(n,nc,wh2o,airmass,skycoupling,Tspill);}
     /** Accesor to the Equivalent Blackbody Temperature in spectral window spwid and channel nc, for Water Column 
	 wh2o, Air Mass airmass, Sky Coupling skycoupling, and Spill Over Temperature Tspill */
      Temperature getTebbSky(unsigned int spwid, unsigned int nc, Length wh2o, double airmass, double skycoupling, Temperature Tspill);



      
      /** Accesor to get the integrated zenith Wet Opacity for the current conditions, for a single 
	  frequency RefractiveIndexProfile object or for the point 0 of spectral window 0 of a 
	  multi-window RefractiveIndexProfile object. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Opacity getWetOpacity(){unsigned int n=0; return getWetOpacity(n);}
      /** Accesor to get the integrated zenith Wet Opacity for the current conditions, 
	  for the point nc of spectral window 0. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Opacity getWetOpacity( unsigned int nc){return getH2OLinesOpacity(nc)+getH2OContOpacity(nc);}       
      /** Accesor to get the integrated zenith Wet Opacity for the current conditions, 
	  for the point nc of spectral window spwid. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Opacity getWetOpacity(unsigned int spwid, unsigned int nc){if(!spwidAndIndexAreValid(spwid,nc))return (double) -999.0; return getWetOpacity( v_transfertId_[spwid]+nc);}
      /** Accesor to get the average Wet Opacity for the current conditions, 
	  in spectral window spwid. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Opacity getAverageWetOpacity(unsigned int spwid){return RefractiveIndexProfile::getAverageWetOpacity(spwid);}



      Opacity getDryOpacity(){return RefractiveIndexProfile::getDryOpacity();}
      Opacity getDryOpacity(unsigned int nc){return RefractiveIndexProfile::getDryOpacity(nc);}
      Opacity getDryOpacity(unsigned int spwid, unsigned int nc){return RefractiveIndexProfile::getDryOpacity(spwid,nc);}
      Opacity getAverageDryOpacity(unsigned int spwid){return RefractiveIndexProfile::getAverageDryOpacity(spwid);}

      Opacity getTotalOpacity(){return getWetOpacity()+getDryOpacity();}
      Opacity getTotalOpacity(unsigned int nc){return getWetOpacity(nc)+getDryOpacity(nc);}
      Opacity getTotalOpacity(unsigned int spwid, unsigned int nc){return getWetOpacity(spwid,nc)+getDryOpacity(spwid,nc);}
      Opacity getAverageTotalOpacity(unsigned int spwid){return getAverageWetOpacity(spwid)+getAverageDryOpacity(spwid);}
     
      /** Accesor to get the integrated zenith H2O Lines Opacity for the current conditions, 
	  for a single frequency RefractiveIndexProfile object or for the point 0 of spectral 
	  window 0 of a multi-window RefractiveIndexProfile object. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Opacity getH2OLinesOpacity(){unsigned int n=0;  return getH2OLinesOpacity(n);}
      /** Accesor to get the integrated zenith H2O Lines Opacity for the current conditions, 
	  for the point nc of spectral window 0. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Opacity getH2OLinesOpacity(unsigned int nc);
      /** Accesor to get the integrated zenith H2O Lines Opacity for the current conditions, 
	  for the point nc of spectral window spwid. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Opacity getH2OLinesOpacity(unsigned int spwid, unsigned int nc){if(!spwidAndIndexAreValid(spwid,nc))return (double) -999.0; return getH2OLinesOpacity( v_transfertId_[spwid]+nc);}
      
      
      /** Accesor to get the integrated zenith H2O Continuum Opacity for the current conditions, 
	  for a single frequency RefractiveIndexProfile object or for the point 0 of spectral 
	  window 0 of a multi-window RefractiveIndexProfile object. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Opacity getH2OContOpacity(){unsigned int n=0;  return getH2OContOpacity(n);}
      /** Accesor to get the integrated zenith H2O Continuum Opacity for the current conditions, 
	  for the point nc of spectral window 0. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Opacity getH2OContOpacity(unsigned int nc);
      /** Accesor to get the integrated zenith H2O Continuum Opacity for the current conditions, 
	  for the point nc of spectral window spwid. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Opacity getH2OContOpacity(unsigned int spwid, unsigned int nc){if(!spwidAndIndexAreValid(spwid,nc))return (double) -999.0; return getH2OContOpacity( v_transfertId_[spwid]+nc);}
      
      
      /** Accesor to get the integrated zenith H2O Atmospheric Phase Delay (Dispersive part) 
	  for the current conditions, for a single frequency RefractiveIndexProfile object or 
	  for the point 0 of spectral window 0 of a multi-window RefractiveIndexProfile object. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Angle getDispersivePhaseDelay(){unsigned int n=0; return getDispersivePhaseDelay(n);}
      /** Accesor to get the integrated zenith H2O Atmospheric Phase Delay (Dispersive part) 
	  for the current conditions, for a single frequency RefractiveIndexProfile object or 
	  for the point nc of spectral window 0. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Angle getDispersivePhaseDelay(unsigned int nc);
      /** Accesor to get the integrated zenith H2O Atmospheric Phase Delay (Dispersive part) 
	  for the current conditions, for a single frequency RefractiveIndexProfile object or 
	  for the point nc of spectral window spwid. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Angle getDispersivePhaseDelay(unsigned int spwid, unsigned int nc);
      
      
      /** Accesor to get the integrated zenith H2O Atmospheric Path length (Dispersive part) 
	  for the current conditions, for a single frequency RefractiveIndexProfile object or 
	  for the point 0 of spectral window 0 of a multi-window RefractiveIndexProfile object. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Length getDispersivePathLength(){unsigned int n=0; return getDispersivePathLength(n);}
      /** Accesor to get the integrated zenith H2O Atmospheric Path length (Dispersive part) 
	  for the current conditions, for a single frequency RefractiveIndexProfile object or 
	  for the point nc of spectral window 0. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Length getDispersivePathLength(unsigned int nc);
      /** Accesor to get the integrated zenith H2O Atmospheric Path length (Dispersive part) 
	  for the current conditions, for a single frequency RefractiveIndexProfile object or 
	  for the point nc of spectral window spwid. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Length getDispersivePathLength(unsigned int spwid, unsigned int nc);
      
      
      /** Accesor to get the integrated zenith H2O Atmospheric Phase Delay (Non-Dispersive part) 
	  for the current conditions, for a single frequency RefractiveIndexProfile object or 
	  for the point 0 of spectral window 0 of a multi-window RefractiveIndexProfile object.
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Angle getNonDispersivePhaseDelay(){unsigned int n=0; return getNonDispersivePhaseDelay(n); }
      /** Accesor to get the integrated zenith H2O Atmospheric Phase Delay (Non-Dispersive part) 
	  for the current conditions, for a single frequency RefractiveIndexProfile object or 
	  for the point nc of spectral window 0. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Angle getNonDispersivePhaseDelay(unsigned int nc);
      /** Accesor to get the integrated zenith H2O Atmospheric Phase Delay (Non-Dispersive part) 
	  for the current conditions, for a single frequency RefractiveIndexProfile object or 
	  for the point nc of spectral window spwid. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Angle getNonDispersivePhaseDelay(unsigned int spwid, unsigned int nc);
      
      
      /** Accesor to get the integrated zenith H2O Atmospheric Path length (Non-Dispersive part) 
	  for the current conditions, for a single frequency RefractiveIndexProfile object or 
	  for the point 0 of spectral window 0 of a multi-window RefractiveIndexProfile object.
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Length getNonDispersivePathLength(){unsigned int n=0; return getNonDispersivePathLength(n); }
      /** Accesor to get the integrated zenith H2O Atmospheric Path length (Non-Dispersive part) 
	  for the current conditions, for a single frequency RefractiveIndexProfile object or 
	  for the point nc of spectral window 0. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Length getNonDispersivePathLength(unsigned int nc);
      /** Accesor to get the integrated zenith H2O Atmospheric Path length (Non-Dispersive part) 
	  for the current conditions, for a single frequency RefractiveIndexProfile object or 
	  for the point nc of spectral window spwid. 
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Length getNonDispersivePathLength(unsigned int spwid, unsigned int nc);
      
      
      /** Accessor to get the average integrated zenith Atmospheric Phase Delay (Dispersive part) 
	  for the current conditions in spectral Window spwid.
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Angle getAverageDispersivePhaseDelay(unsigned int spwid);
      /** Accessor to get the average integrated zenith Atmospheric Phase Delay (Dispersive part) 
	  for the current conditions in spectral Window 0.
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Angle getAverageDispersivePhaseDelay(){unsigned int n=0; return getAverageDispersivePhaseDelay(n);}
      /** Accessor to get the average integrated zenith Atmospheric Phase Delay (Non-Dispersive 
	  part) in spectral Window spwid.
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Angle getAverageNonDispersivePhaseDelay(unsigned int spwid);
      /** Accessor to get the average integrated zenith Atmospheric Phase Delay (Non-Dispersive 
	  part) in spectral Window 0.
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Angle getAverageNonDispersivePhaseDelay(){unsigned int n=0; return getAverageNonDispersivePhaseDelay(n);}
      
      
      
      /** Accessor to get the average integrated zenith Atmospheric Path Length (Dispersive part) 
	  in spectral Window spwid.
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Length getAverageDispersivePathLength(unsigned int spwid);
      /** Accessor to get the average integrated zenith Atmospheric Path Length (Dispersive part) 
	  in spectral Window 0.
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Length getAverageDispersivePathLength(){unsigned int n=0; return getAverageDispersivePathLength(n);}
      /** Accessor to get the average integrated zenith Atmospheric Path Length (Non-Dispersive 
	  part) in spectral Window spwid.
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Length getAverageNonDispersivePathLength(unsigned int spwid);
      /** Accessor to get the average integrated zenith Atmospheric Path Length (Non-Dispersive 
	  part) in spectral Window 0.
	  There is overloading. The same accessor exists in RefractiveIndexProfile but in that 
	  case the returned value corresponds to the zenith water vapor column of the AtmProfile object.*/
      Length getAverageNonDispersivePathLength(){unsigned int n=0; return getAverageNonDispersivePathLength(n);}
      
      
      /** Setter for a new set of basic atmospheric parameters. Automatically updates the AtmProfile 
	  (if generated from this basic atmospheric parameters), RefractiveIndexProfile, and SkyStatus  
	  objects if the one or more value differ from the previous ones (there is overloading) */
      bool setBasicAtmosphericParameters(Length altitude, Pressure groundPressure, Temperature groundTemperature, 
					 double tropoLapseRate, Humidity humidity, Length wvScaleHeight ); 
      /** The same setter is available for only one parameter each time (except for "Length wvScaleHeight" due to a type 
	  conflict with "Length altitude") */ 
      bool setBasicAtmosphericParameters(Length altitude);
      /** The same setter is available for only one parameter each time (except for "Length wvScaleHeight" due to a type 
	  conflict with "Length altitude") */ 
      bool setBasicAtmosphericParameters(Temperature groundTemperature);
      /** The same setter is available for only one parameter each time (except for "Length wvScaleHeight" due to a type 
	  conflict with "Length altitude") */ 
      bool setBasicAtmosphericParameters(Pressure groundPressure);
      /** The same setter is available for only one parameter each time (except for "Length wvScaleHeight" due to a type 
	  conflict with "Length altitude") */ 
      bool setBasicAtmosphericParameters(Humidity humidity);
      /** The same setter is available for only one parameter each time (except for "Length wvScaleHeight" due to a type 
	  conflict with "Length altitude") */ 
      bool setBasicAtmosphericParameters(double tropoLapseRate);
      /** The same setter is available for couples of parameters. The order does not matter except for (Length altitude) and 
	  (Length wvScaleHeight) because they are of the same type (Length). The convention is that if a type Length is in first 
	  position, it represents the altitude, and if it is in second position, it represents wvScaleHeight */ 
      bool setBasicAtmosphericParameters(Length altitude, Temperature groundTemperature); 
      /** The same setter is available for couples of parameters. The order does not matter except for (Length altitude) and 
	  (Length wvScaleHeight) because they are of the same type (Length). The convention is that if a type Length is in first 
	  position, it represents the altitude, and if it is in second position, it represents wvScaleHeight */ 
      bool setBasicAtmosphericParameters(Length altitude, Pressure groundPressure); 
      /** The same setter is available for couples of parameters. The order does not matter except for (Length altitude) and 
	  (Length wvScaleHeight) because they are of the same type (Length). The convention is that if a type Length is in first 
	  position, it represents the altitude, and if it is in second position, it represents wvScaleHeight */ 
      bool setBasicAtmosphericParameters(Length altitude, Humidity humidity); 
      /** The same setter is available for couples of parameters. The order does not matter except for (Length altitude) and 
	  (Length wvScaleHeight) because they are of the same type (Length). The convention is that if a type Length is in first 
	  position, it represents the altitude, and if it is in second position, it represents wvScaleHeight */ 
      bool setBasicAtmosphericParameters(Length altitude, double tropoLapseRate); 
      /** The same setter is available for couples of parameters. The order does not matter except for (Length altitude) and 
	  (Length wvScaleHeight) because they are of the same type (Length). The convention is that if a type Length is in first 
	  position, it represents the altitude, and if it is in second position, it represents wvScaleHeight */ 
      bool setBasicAtmosphericParameters(Length altitude, Length wvScaleHeight); 
      /** The same setter is available for couples of parameters. The order does not matter */ 
      bool setBasicAtmosphericParameters(Temperature groundTemperature, Pressure groundPressure);
      /** The same setter is available for couples of parameters. The order does not matter  */ 
      bool setBasicAtmosphericParameters(Pressure groundPressure, Temperature groundTemperature);
      /** The same setter is available for couples of parameters. The order does not matter  */ 
      bool setBasicAtmosphericParameters(Temperature groundTemperature, Humidity humidity);
      /** The same setter is available for couples of parameters. The order does not matter  */ 
      bool setBasicAtmosphericParameters(Humidity humidity, Temperature groundTemperature);
      /** The same setter is available for couples of parameters. The order does not matter  */ 
      bool setBasicAtmosphericParameters(Temperature groundTemperature, double tropoLapseRate);
      /** The same setter is available for couples of parameters. The order does not matter  */ 
      bool setBasicAtmosphericParameters(double tropoLapseRate, Temperature groundTemperature);
      /** The same setter is available for couples of parameters. The order does not matter except for (Length altitude) and 
	  (Length wvScaleHeight) because they are of the same type (Length). The convention is that if a type Length is in first 
	  position, it represents the altitude, and if it is in second position, it represents wvScaleHeight */ 
      bool setBasicAtmosphericParameters(Temperature groundTemperature, Length wvScaleHeight); 
      /** The same setter is available for couples of parameters. The order does not matter  */ 
      bool setBasicAtmosphericParameters(Pressure groundPressure, Humidity humidity);
      /** The same setter is available for couples of parameters. The order does not matter  */ 
      bool setBasicAtmosphericParameters(Humidity humidity, Pressure groundPressure);
      /** The same setter is available for couples of parameters. The order does not matter  */ 
      bool setBasicAtmosphericParameters(Pressure groundPressure, double tropoLapseRate);
      /** The same setter is available for couples of parameters. The order does not matter */ 
      bool setBasicAtmosphericParameters(double tropoLapseRate, Pressure groundPressure);
      /** The same setter is available for couples of parameters. The order does not matter except for (Length altitude) and 
	  (Length wvScaleHeight) because they are of the same type (Length). The convention is that if a type Length is in first 
	  position, it represents the altitude, and if it is in second position, it represents wvScaleHeight */ 
      bool setBasicAtmosphericParameters(Pressure groundPressure, Length wvScaleHeight);
      /** The same setter is available for couples of parameters. The order does not matter  */ 
      bool setBasicAtmosphericParameters(Humidity humidity, double tropoLapseRate);
      /** The same setter is available for couples of parameters. The order does not matter  */ 
      bool setBasicAtmosphericParameters(double tropoLapseRate, Humidity humidity);
      /** The same setter is available for couples of parameters. The order does not matter except for (Length altitude) and 
	  (Length wvScaleHeight) because they are of the same type (Length). The convention is that if a type Length is in first 
	  position, it represents the altitude, and if it is in second position, it represents wvScaleHeight */ 
      bool setBasicAtmosphericParameters(Humidity humidity, Length wvScaleHeight);
      /** The same setter is available for couples of parameters. The order does not matter except for (Length altitude) and 
	  (Length wvScaleHeight) because they are of the same type (Length). The convention is that if a type Length is in first 
	  position, it represents the altitude, and if it is in second position, it represents wvScaleHeight */ 
      bool setBasicAtmosphericParameters(double tropoLapseRate, Length wvScaleHeight);
      
      
            /** Setter of new spectral windows (adds frequencies to the SpectralGrid and calculates the corresponding absorption coefficients) */
      void addNewSpectralWindow( unsigned int numChan, unsigned int refChan, Frequency refFreq, Frequency chanSep){RefractiveIndexProfile::add(numChan, refChan, refFreq, chanSep);}
      /** Setter of new spectral windows (adds frequencies to the SpectralGrid and calculates the corresponding absorption coefficients) */
      void addNewSpectralWindow( unsigned int numChan, unsigned int refChan, double* chanFreq, string freqUnits){RefractiveIndexProfile::add(numChan, refChan, chanFreq, freqUnits);}
      /** Setter of new spectral windows (adds frequencies to the SpectralGrid and calculates the corresponding absorption coefficients) */
      void addNewSpectralWindow( unsigned int numChan, double refFreq, double* chanFreq, string freqUnits){RefractiveIndexProfile::add(numChan, refFreq, chanFreq, freqUnits);}
      /** Setter of new spectral windows (adds frequencies to the SpectralGrid and calculates the corresponding absorption coefficients) */
      void addNewSpectralWindow( unsigned int numChan, double refFreq, vector<double> chanFreq, string freqUnits){RefractiveIndexProfile::add(numChan, refFreq, chanFreq, freqUnits); }
      /** Setter of new spectral windows (adds frequencies to the SpectralGrid and calculates the corresponding absorption coefficients) */
      void addNewSpectralWindow( unsigned int numChan, unsigned int refChan, Frequency refFreq, Frequency chanSep, Frequency intermediateFreq, SidebandSide sbSide, SidebandType sbType){RefractiveIndexProfile::add(numChan, refChan, refFreq, chanSep, intermediateFreq, sbSide, sbType); }
      void addNewSpectralWindow( vector<Frequency> chanFreq){RefractiveIndexProfile::add(chanFreq); }

      
      
      
      /** Zenith Water Vapor column retrieval based on fitting the vector of zenith atmospheric transmission given as input. 
	  Requirements: A) The FTS individual frequencies must be in spectral window 0, and B) the input zenith atmospheric 
	  transmission vector should match those frequencies (i.e. both vectors should have the same size), 
	  C) the air mass for the retrieval is the current one. If a different one is desired, it should be 
	  changed using the setAirMass setter. */ 
      Length WaterVaporRetrieval_fromFTS(vector<double> v_transmission){ unsigned int spwId=0; Frequency f1(-999,"GHz"); Frequency f2(-999,"GHz"); return WaterVaporRetrieval_fromFTS(spwId, v_transmission, f1, f2); }
      /** Same as above but using for the retrieval only the measurements between frequencies f1 and f2>f1 */
      Length WaterVaporRetrieval_fromFTS(vector<double> v_transmission, Frequency f1, Frequency f2){unsigned int spwId=0; return WaterVaporRetrieval_fromFTS(spwId, v_transmission, f1, f2);}
      /** Zenith Water Vapor column retrieval based on fitting the vector of zenith atmospheric transmission given as input. 
	  Requirements: A) The FTS individual frequencies must be in spectral window spwId, and B) the input zenith atmospheric 
	  transmission vector should match those frequencies (i.e. both vectors should have the same size),
	  C) the air mass for the retrieval is the current one. If a different one is desired, it should be 
	  changed using the setAirMass setter. */ 
      Length WaterVaporRetrieval_fromFTS(unsigned int spwId, vector<double> v_transmission){Frequency f1(-999,"GHz"); Frequency f2(-999,"GHz"); return WaterVaporRetrieval_fromFTS(spwId, v_transmission, f1, f2);}
      /** Same as above but using for the retrieval only the measurements between frequencies f1 and f2>f1 */
      Length WaterVaporRetrieval_fromFTS(unsigned int spwId, vector<double> v_transmission, Frequency f1, Frequency f2);

      /** Zenith Water Vapor column retrieval based on fitting measured brightness temperatures of the atmosphere */
      Length WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Percent> signalGain, 
					  vector<vector<Temperature> > vv_tebb, vector<vector<double> > spwId_filters,   
					  double airmass, vector<double> skycoupling, vector<Temperature> tspill);

      Length WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Percent> signalGain, 
					  vector<Temperature> v_tebb,  vector<vector<double> > spwId_filters,       
					  double airmass, vector<double> skycoupling, vector<Temperature> tspill);


      Length WaterVaporRetrieval_fromTEBB(unsigned int spwId, Percent signalGain, vector<Temperature> v_tebb, 
					  vector<double> spwId_filter, double airmass, 
					  double skycoupling, Temperature tspill);
      Length WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, Percent signalGain, vector<Temperature> v_tebb, 
					  vector<vector<double> > spwId_filters, double airmass, double skycoupling, 
					  Temperature tspill);
      Length WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, Percent signalGain, vector<Temperature> v_tebb, 
					  double airmass, double skycoupling, Temperature tspill);





      Length WaterVaporRetrieval_fromTEBB(unsigned int spwId, vector<Temperature> v_tebb, 
					  double skycoupling, Temperature tspill){
	vector<double> spwId_filter; for(unsigned int n=0; n<v_numChan_[spwId]; n++){spwId_filter.push_back(1.0);}
	return WaterVaporRetrieval_fromTEBB(spwId, v_tebb, spwId_filter, skycoupling, tspill);
      }



      Length WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<vector<Temperature> > vv_tebb, 
					  vector<double> skycoupling, vector<Temperature> tspill){
	vector<vector<double> > spwId_filters; vector<double> spwId_filter;
	for(unsigned int i=0; i<spwId.size(); i++){
	  for(unsigned int n=0; n<v_numChan_[spwId[i]]; n++){spwId_filter.push_back(1.0);}
	  spwId_filters.push_back(spwId_filter); spwId_filter.clear();
	}
	return WaterVaporRetrieval_fromTEBB(spwId, vv_tebb, spwId_filters, skycoupling, tspill);
      }

      Length WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Temperature> v_tebb, 
					  vector<double> skycoupling, vector<Temperature> tspill){
	vector<vector<double> > spwId_filters; vector<double> spwId_filter;
	for(unsigned int i=0; i<spwId.size(); i++){
	  for(unsigned int n=0; n<v_numChan_[spwId[i]]; n++){spwId_filter.push_back(1.0);}
	  spwId_filters.push_back(spwId_filter); spwId_filter.clear();
	}
	return WaterVaporRetrieval_fromTEBB(spwId, v_tebb, spwId_filters, skycoupling, tspill);
      }

      Length WaterVaporRetrieval_fromTEBB(unsigned int spwId, vector<Temperature> v_tebb, 
					  vector<double> spwId_filter, double skycoupling, Temperature tspill);
      Length WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<vector<Temperature> > vv_tebb, 
					  vector<vector<double> > spwId_filters, vector<double> skycoupling, 
					  vector<Temperature> tspill);
      Length WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Temperature> v_tebb, 
					  vector<vector<double> > spwId_filters, vector<double> skycoupling, 
					  vector<Temperature> tspill);

      Length WaterVaporRetrieval_fromTEBB(unsigned int spwId, vector<Temperature> v_tebb, double airmass, 
					  double skycoupling, Temperature tspill);
      Length WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<vector<Temperature> > vv_tebb, 
					  double airmass, vector<double> skycoupling, vector<Temperature> tspill);
      Length WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Temperature> v_tebb, 
					  double airmass, vector<double> skycoupling, vector<Temperature> tspill);

      Length WaterVaporRetrieval_fromTEBB(unsigned int spwId, vector<Temperature> v_tebb, 
					  vector<double> spwId_filter, double airmass, 
					  double skycoupling, Temperature tspill);
      Length WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<vector<Temperature> > vv_tebb, 
					  vector<vector<double> > spwId_filters, double airmass, 
					  vector<double> skycoupling, vector<Temperature> tspill);
      Length WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Temperature> v_tebb, 
					  vector<vector<double> > spwId_filters, double airmass, 
					  vector<double> skycoupling, vector<Temperature> tspill);

      Length WaterVaporRetrieval_fromTEBB(unsigned int spwId, Percent signalGain, vector<Temperature> v_tebb, 
					  double skycoupling, Temperature tspill);
      Length WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Percent> signalGain, 
					  vector<vector<Temperature> >  vv_tebb, vector<double> skycoupling, 
					  vector<Temperature> tspill);
      Length WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Percent> signalGain, 
					  vector<Temperature> v_tebb, vector<double> skycoupling, 
					  vector<Temperature> tspill);

      Length WaterVaporRetrieval_fromTEBB(unsigned int spwId, Percent signalGain, vector<Temperature> v_tebb, 
					  vector<double> spwId_filter, double skycoupling, Temperature tspill);
      Length WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Percent> signalGain, 
					  vector<vector<Temperature> > vv_tebb, vector<vector<double> > spwId_filters, 
					  vector<double> skycoupling, vector<Temperature> tspill);
      Length WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Percent> signalGain, 
					  vector<Temperature> v_tebb, vector<vector<double> > spwId_filters, 
					  vector<double> skycoupling, vector<Temperature> tspill);

      Length WaterVaporRetrieval_fromTEBB(unsigned int spwId, Percent signalGain, vector<Temperature> v_tebb, 
					  double airmass, double skycoupling, Temperature tspill);
      Length WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId,  vector<Percent> signalGain, 
					  vector<vector<Temperature> > vv_tebb, double airmass, 
					  vector<double> skycoupling, vector<Temperature> tspill);
      Length WaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId,  vector<Percent> signalGain, 
					  vector<Temperature> v_tebb, double airmass, vector<double> skycoupling, 
					  vector<Temperature> tspill);


      double SkyCouplingRetrieval_fromTEBB(unsigned int spwId, vector<Temperature> v_tebb, double skycoupling, Temperature tspill);
      double SkyCouplingRetrieval_fromTEBB(unsigned int spwId, vector<Temperature> v_tebb, 
					   vector<double> spwId_filter, double skycoupling, Temperature tspill);
      double SkyCouplingRetrieval_fromTEBB(unsigned int spwId, vector<Temperature> v_tebb, double airmass, 
					   double skycoupling, Temperature tspill);
      double SkyCouplingRetrieval_fromTEBB(unsigned int spwId, vector<Temperature> v_tebb, vector<double> spwId_filter,
					   double airmass, double skycoupling, Temperature tspill);

      
      /** (Re)setter of water vapor radiometer channels */
      void setWaterVaporRadiometer(WaterVaporRadiometer waterVaporRadiometer){waterVaporRadiometer_ = waterVaporRadiometer;}
      /** Performs a sky coupling retrieval using WVR measurement sets between n and m 
	  (obtains the ratio between the current sky couplings of all WVR channels and the best fit that 
	  results from WVR measurement sets between n and m, keeping the other parameters unchanged) 
	  and uses the result to update the Skycoupling of all WVR channels */
      void  updateSkyCoupling_fromWVR(vector<WVRMeasurement> &RadiometerData, unsigned int n, unsigned int m);
      void  updateSkyCoupling_fromWVR(vector<WVRMeasurement> &RadiometerData, unsigned int n){updateSkyCoupling_fromWVR(RadiometerData,n,n+1);}
      void  updateSkyCoupling_fromWVR(vector<WVRMeasurement> &RadiometerData, unsigned int n, unsigned int m, WaterVaporRadiometer &external_wvr){updateSkyCoupling_fromWVR(RadiometerData,n,m); external_wvr=waterVaporRadiometer_;}
      void  updateSkyCoupling_fromWVR(vector<WVRMeasurement> &RadiometerData, unsigned int n, WaterVaporRadiometer &external_wvr){updateSkyCoupling_fromWVR(RadiometerData,n,n+1); external_wvr=waterVaporRadiometer_;}
      /** Accessor to the sky coupling of channel n of the Water Vapor Radiometer*/
      double getWaterVaporRadiometerSkyCoupling(int n){return waterVaporRadiometer_.getSkyCoupling()[n];}
      /** Accessor to the sky coupling of channel n of the Water Vapor Radiometer*/
      Percent getWaterVaporRadiometerSignalGain(int n){return waterVaporRadiometer_.getsignalGain()[n];}
      /** Performs water vapor retrieval for WVR measurement sets between n and m */
      void WaterVaporRetrieval_fromWVR(vector<WVRMeasurement> &RadiometerData, unsigned int n, unsigned int m);
      void WaterVaporRetrieval_fromWVR(vector<WVRMeasurement> &RadiometerData, unsigned int n){WaterVaporRetrieval_fromWVR(RadiometerData,n,n+1);}
      /** Performs water vapor retrieval for one WVR measurement */
      void WaterVaporRetrieval_fromWVR(WVRMeasurement &RadiometerData);
      /** Accessor to get or check the water vapor radiometer channels */
      WaterVaporRadiometer getWaterVaporRadiometer(){return waterVaporRadiometer_;}
      
      double sigmaSkyCouplingRetrieval_fromWVR(double par_fit, WaterVaporRadiometer wvr, 
					       vector<WVRMeasurement> &RadiometerData, unsigned int n, unsigned int m);
      double sigmaSkyCouplingRetrieval_fromWVR(double par_fit, WaterVaporRadiometer wvr, 
						 vector<WVRMeasurement> &RadiometerData, unsigned int n){return sigmaSkyCouplingRetrieval_fromWVR(par_fit,wvr,RadiometerData,n,n+1);}
      
      Temperature getWVRAverageSigmaTskyFit(vector<WVRMeasurement> RadiometerData, unsigned int n, unsigned int m);
      Temperature getWVRAverageSigmaTskyFit(vector<WVRMeasurement> RadiometerData, unsigned int n){return getWVRAverageSigmaTskyFit(RadiometerData,n,n+1);}

      double getSigmaTransmissionFit(unsigned int spwId, vector<double> v_transmission, double airm, Frequency f1, Frequency f2);
      //double getSigmaTransmissionFit(){}
      Temperature getSigmaFit(unsigned int spwId, vector<Temperature> v_tebbspec, double skyCoupling, Temperature Tspill){
	return getSigmaFit(spwId,v_tebbspec,getUserWH2O(),skyCoupling,Tspill);}
      Temperature getSigmaFit(unsigned int spwId, vector<Temperature> v_tebbspec, Length wh2o, double skyCoupling, Temperature Tspill){
	return getSigmaFit(spwId, v_tebbspec, wh2o, getAirMass(), skyCoupling, Tspill);
      }
      Temperature getSigmaFit(unsigned int spwId, vector<Temperature> v_tebbspec, double airmass, double skyCoupling, Temperature Tspill){
	return getSigmaFit(spwId, v_tebbspec, getUserWH2O(), airmass, skyCoupling, Tspill);}
      Temperature getSigmaFit(unsigned int spwId, vector<Temperature> v_tebbspec, Length wh2o, double airmass, double skyCoupling, Temperature Tspill);

      
      
      //@}
      
    protected:
      
      double         airMass_;                                        //!< Air Mass used for the radiative transfer 
      Temperature    skyBackgroundTemperature_;                       //!< Blackbody temperature of the sky background
      Length         wh2o_user_;                                      //!< Water vapor column used for radiative transfer calculations. If not provided, 
                                                                      //!< the one retrieved from the water vapor radiometer channels will be used.
      WaterVaporRadiometer              waterVaporRadiometer_;        // !< Identifiers, sky coupling, and sideband gain of channels corresponding to the water vapor radiometer.
      
      
      void iniSkyStatus();                          //!< Basic Method initialize the class when using the constructors.
      bool updateProfilesAndRadiance(Length altitude, Pressure    groundPressure, 
				     Temperature groundTemperature, double  tropoLapseRate, 
				     Humidity relativeHumidity, Length wvScaleHeight);    
      void rmSkyStatus();           //!< Resets retrieved water column to zero, or the default value.

      Length mkWaterVaporRetrieval_fromFTS(unsigned int spwId, vector<double> v_transmission, 
					   double airm, Frequency f1, Frequency f2);

      Length mkWaterVaporRetrieval_fromTEBB(unsigned int spwId, Percent signalGain, vector<Temperature> v_measuredSkyTEBB, 
					    double airm, vector<double> spwId_filter, double skycoupling, Temperature tspill){
	vector<unsigned int> spwIds;
	spwIds.push_back(spwId);
	vector<Percent> signalGains;
	signalGains.push_back(signalGain);
	vector<vector<Temperature> > v_measuredSkyTEBBs;
	v_measuredSkyTEBBs.push_back(v_measuredSkyTEBB);
	vector<vector<double> >  spwId_filters;
	spwId_filters.push_back(spwId_filter);
	vector<double> skycouplings;
	skycouplings.push_back(skycoupling);
	vector<Temperature> tspills;
	tspills.push_back(tspill);

	return mkWaterVaporRetrieval_fromTEBB(spwIds, signalGains, v_measuredSkyTEBBs, airm, spwId_filters, skycouplings, tspills);
      }

      Length mkWaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Percent> signalGain, 
					    vector<vector<Temperature> > measuredSkyTEBB, double airm, 
					    vector<vector<double> > spwId_filters, vector<double> skycoupling, 
					    vector<Temperature> tspill);

      Length mkWaterVaporRetrieval_fromTEBB(vector<unsigned int> spwId, vector<Percent> signalGain, 
					    vector<Temperature> measuredAverageSkyTEBB, double airm, 
					    vector<vector<double> > spwId_filters, vector<double> skycoupling, 
					    vector<Temperature> tspill);

      double mkSkyCouplingRetrieval_fromTEBB(unsigned int spwId, Percent signalGain, vector<Temperature> measuredSkyTEBB, 
					     double airm, vector<double> spwId_filter, double skycoupling, Temperature tspill);

      WVRMeasurement mkWaterVaporRetrieval_fromWVR(vector<Temperature> measuredSkyBrightnessVector, 
						   vector<unsigned int> radiometricChannels, vector<double> skyCoupling, 
						   vector<Percent> signalGain, Temperature spilloverTemperature, Angle elevation);
      
      
      double RT(double pfit_wh2o, double skycoupling, double tspill, unsigned int spwid, unsigned int nc){
	return RT(pfit_wh2o,skycoupling,tspill,airMass_,spwid,nc);}
      double RT(double pfit_wh2o, double skycoupling, double tspill, unsigned int spwid){
	return RT(pfit_wh2o,skycoupling,tspill,airMass_,spwid);}   
      double RT(double pfit_wh2o, double skycoupling, double tspill, double airmass, unsigned int spwId, unsigned int nc);
      double RT(double pfit_wh2o, double skycoupling, double tspill, double airmass, unsigned int spwid){
	double tebb_channel=0.0;
	for(unsigned int n=0; n<v_numChan_[spwid]; n++){
	  tebb_channel=tebb_channel+RT(pfit_wh2o,skycoupling,tspill,airmass,spwid,n)/(v_numChan_[spwid]);
	}
	return tebb_channel;  
      }

      double RT(double pfit_wh2o, double skycoupling, double tspill, unsigned int spwid, Percent signalgain){
	vector<double> spwId_filter; for(unsigned int n=0; n<v_numChan_[spwid]; n++){spwId_filter.push_back(1.0);}
	return RT(pfit_wh2o,skycoupling,tspill,airMass_,spwid,spwId_filter,signalgain);}   

      double RT(double pfit_wh2o, double skycoupling, double tspill, double airmass, unsigned int spwid, Percent signalgain){
	vector<double> spwId_filter; for(unsigned int n=0; n<v_numChan_[spwid]; n++){spwId_filter.push_back(1.0);}
	return RT(pfit_wh2o,skycoupling,tspill,airmass,spwid,spwId_filter,signalgain);}   

      double RT(double pfit_wh2o, double skycoupling, double tspill, unsigned int spwid, vector<double> spwId_filter){
	return RT(pfit_wh2o,skycoupling,tspill,airMass_,spwid,spwId_filter,Percent(100.0,"%"));}   

      double RT(double pfit_wh2o, double skycoupling, double tspill, unsigned int spwid, vector<double> spwId_filter, Percent signalgain){
	return RT(pfit_wh2o,skycoupling,tspill,airMass_,spwid,spwId_filter,signalgain);}   

      double RT(double pfit_wh2o, double skycoupling, double tspill, double airmass, unsigned int spwid, vector<double> spwId_filter, Percent signalgain){

	double tebb_channel=0.0;
	double rtr;
	double norm=0.0;

	for(unsigned int n=0; n<v_numChan_[spwid]; n++){
	  if(spwId_filter[n]>0){
	    norm=norm+spwId_filter[n];
	  }
	}

	if(norm==0.0){
	  return norm;
	}

	for(unsigned int n=0; n<v_numChan_[spwid]; n++){

	  if(spwId_filter[n]>0){

	    if(signalgain.get()<1.0){
	      rtr=RT(pfit_wh2o,skycoupling,tspill,airmass,spwid,n)*signalgain.get()+RT(pfit_wh2o,skycoupling,tspill,airmass,getAssocSpwId(spwid)[0],n)*(1.0-signalgain.get());
	    }else{
	      rtr=RT(pfit_wh2o,skycoupling,tspill,airmass,spwid,n);
	    }
	    tebb_channel=tebb_channel+rtr*spwId_filter[n]/norm;
	  }
	}



	return tebb_channel;  
      }





    private:
      
      // no accessors provided for these
      double sigma_transmission_FTSfit_;
      Temperature sigma_TEBBfit_;
      
    };
  
}

#define ATM_SKYSTATUS_H
#endif
