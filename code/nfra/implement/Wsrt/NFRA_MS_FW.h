//# NFRA_MS_FW.h: class definitions for NFRA Frequency objects
//# Copyright (C) 1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$
//
// Defines:
// - class NFRA_IVC
// - class NFRA_MFFE
// - class NFRA_FreqMos
// - class NFRA_FW
//
// Frequency information
//
//----------------------------------------------------------------------
// IVC information
//
class NFRA_IVC {
 private:
  String IVCName;
  Double CenterFreq;
  Double RestFreq;
  String ObsMode;
  String VelDef;
  String ConvType;
  Double VelValue;
  Double BandWidth;
  uInt NChan;
  Double ChanWidth;
  uInt NPol;
  vector<String> PolsUsed;
  String EF;
  Int SubArray;

 public:
  String getIVCName(){return IVCName;}
  Double getCenterFreq(){return CenterFreq;}
  Double getRestFreq(){return RestFreq;}
  String getObsMode(){return ObsMode;}
  String getVelDef(){return VelDef;}
  String getConvType(){return ConvType;}
  Double getVelValue(){return VelValue;}
  Double getBandWidth(){return BandWidth;}
  uInt getNChan(){return NChan;}
  Double getChanWidth(){return ChanWidth;}
  uInt getNPol(){return NPol;}
  vector<String> getPolsUsed(){return PolsUsed;}
  String getEF(){return EF;}
  Int getSubArray(){return SubArray;}

  String getPolsString();

  void setIVCName(String s){IVCName = s;}
  void setCenterFreq(Double d){CenterFreq = d;}
  void setRestFreq(Double d){RestFreq = d;}
  void setObsMode(String s) {ObsMode = s;}
  void setVelDef(String s){VelDef = s;}
  void setConvType(String s){ConvType = s;}
  void setVelValue(Double d){VelValue = d;}
  void setBandWidth(Double d){BandWidth = d;}
  void setNChan(uInt i){NChan = i;}
  void setChanWidth(Double d){ChanWidth = d;}
  void setNPol(uInt i){NPol = i;}
  void setPolsUsed(vector<String> vs){PolsUsed = vs;}
  void setEF(String s){EF = s;}
  void setSubArray(Int i){SubArray = i;}

  NFRA_IVC();
  void dump(String, uInt);
};


//----------------------------------------------------------------------
// Frontend information
//
class NFRA_MFFE {
 private:
  String MFFEBand;
  Double MFFESkyFreq;
  Double MFFEBandWidth;

 public:
  String getMFFEBand(){return MFFEBand;}
  Double getMFFESkyFreq(){return MFFESkyFreq;}
  Double getMFFEBandWidth(){return MFFEBandWidth;}
  
  void setMFFEBand(String s){MFFEBand = s;}
  void setMFFESkyFreq(Double d){MFFESkyFreq = d;}
  void setMFFEBandWidth(Double d){MFFEBandWidth = d;}

  void dump(String, uInt);

};

//----------------------------------------------------------------------
// Frequency Mosaic Position
//
class NFRA_FreqMos {
 private:
  uInt DwellTime;
  NFRA_MFFE MFFE;
  vector<NFRA_IVC> IVCs;

 public:
  uInt getDwellTime(){return DwellTime;}
  NFRA_MFFE getMFFE(){return MFFE;}
  vector<NFRA_IVC> getIVCs(){return IVCs;}

  void setDwellTime(uInt i){DwellTime = i;}
  void setMFFE(NFRA_MFFE M){MFFE = M;}
  void setIVCs(vector<NFRA_IVC> v){IVCs = v;}
  void addIVC(NFRA_IVC ivc){IVCs.push_back(ivc);}
  void addIVCs(NFRA_FreqMos);

  String getMFFEName(){return MFFE.getMFFEBand();}
  Int getNIVCBands(){return IVCs.size();}
  vector<Double> getCenterFreqs();
  Double getIFBandWidth(){return IVCs[0].getBandWidth();}
  uInt getNChan(){return IVCs[0].getNChan();}
  Double getChanWidth(){return IVCs[0].getChanWidth()*1000.0;}
  Int getNPol(){return IVCs[0].getNPol();}
  String getPolsString(){return IVCs[0].getPolsString();}

  void dump(String, uInt);
};

//----------------------------------------------------------------------
// Frequency Window - toplevel of Frequency information
//
class NFRA_FW{

 private:
  String Telescopes;
  vector<NFRA_FreqMos> FreqMoss;

 public:
  String getTelescopes(){return Telescopes;}
  vector<NFRA_FreqMos> getFreqMoss(){return FreqMoss;}

  void setTelescopes(String s){Telescopes = s;}
  void addFreqMoss(NFRA_FreqMos v){FreqMoss.push_back(v);}
  void setFreqMoss(vector<NFRA_FreqMos> v){FreqMoss = v;}
  // Copy IVCs from other Frequency Window to this one
  void addIVCs(NFRA_FW);

  Int getDwellTime(){return FreqMoss[0].getDwellTime();}
  String getMFFEName(){return FreqMoss[0].getMFFEName();}
  Int getNIVCBands(){return FreqMoss[0].getNIVCBands();}
  vector<Double> getCenterFreqs(){return FreqMoss[0].getCenterFreqs();}
  Double getIFBandWidth(){return FreqMoss[0].getIFBandWidth();}
  Int getNChan(){return FreqMoss[0].getNChan();}
  Double getChanWidth(){return FreqMoss[0].getChanWidth();}
  Int getNPol(){return FreqMoss[0].getNPol();}
  String getPolsString(){return FreqMoss[0].getPolsString();}


  uInt getNFreqMos(){return FreqMoss.size();}
  void dump(uInt);

};
