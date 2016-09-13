
#ifndef _ASDMTABLES_H_
#define _ASDMTABLES_H_
#include "ASDMTableBase.h"
/*
 * ALMA - Atacama Large Millimeter Array
 * (c) European Southern Observatory, 2002
 * (c) Associated Universities Inc., 2002
 * Copyright by ESO (in the framework of the ALMA collaboration),
 * Copyright by AUI (in the framework of the ALMA collaboration),
 * All rights reserved.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307  USA
 *
 * Warning!
 *  -------------------------------------------------------------------- 
 * | This is generated code!  Do not modify this file.                  |
 * | If you do, all changes will be lost when the file is re-generated. |
 *  --------------------------------------------------------------------
 *
 * casacore::File ASDMTables.h
 */
 
	
class ASDM_MAIN : public ASDM_TABLE_SINGLETON<ASDM_MAIN>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_MAIN>;

 private:
  ASDM_MAIN();
  ~ASDM_MAIN();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_ALMARADIOMETER : public ASDM_TABLE_SINGLETON<ASDM_ALMARADIOMETER>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_ALMARADIOMETER>;

 private:
  ASDM_ALMARADIOMETER();
  ~ASDM_ALMARADIOMETER();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_ANNOTATION : public ASDM_TABLE_SINGLETON<ASDM_ANNOTATION>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_ANNOTATION>;

 private:
  ASDM_ANNOTATION();
  ~ASDM_ANNOTATION();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_ANTENNA : public ASDM_TABLE_SINGLETON<ASDM_ANTENNA>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_ANTENNA>;

 private:
  ASDM_ANTENNA();
  ~ASDM_ANTENNA();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALAMPLI : public ASDM_TABLE_SINGLETON<ASDM_CALAMPLI>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALAMPLI>;

 private:
  ASDM_CALAMPLI();
  ~ASDM_CALAMPLI();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALAPPPHASE : public ASDM_TABLE_SINGLETON<ASDM_CALAPPPHASE>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALAPPPHASE>;

 private:
  ASDM_CALAPPPHASE();
  ~ASDM_CALAPPPHASE();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALATMOSPHERE : public ASDM_TABLE_SINGLETON<ASDM_CALATMOSPHERE>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALATMOSPHERE>;

 private:
  ASDM_CALATMOSPHERE();
  ~ASDM_CALATMOSPHERE();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALBANDPASS : public ASDM_TABLE_SINGLETON<ASDM_CALBANDPASS>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALBANDPASS>;

 private:
  ASDM_CALBANDPASS();
  ~ASDM_CALBANDPASS();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALCURVE : public ASDM_TABLE_SINGLETON<ASDM_CALCURVE>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALCURVE>;

 private:
  ASDM_CALCURVE();
  ~ASDM_CALCURVE();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALDATA : public ASDM_TABLE_SINGLETON<ASDM_CALDATA>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALDATA>;

 private:
  ASDM_CALDATA();
  ~ASDM_CALDATA();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALDELAY : public ASDM_TABLE_SINGLETON<ASDM_CALDELAY>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALDELAY>;

 private:
  ASDM_CALDELAY();
  ~ASDM_CALDELAY();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALDEVICE : public ASDM_TABLE_SINGLETON<ASDM_CALDEVICE>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALDEVICE>;

 private:
  ASDM_CALDEVICE();
  ~ASDM_CALDEVICE();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALFLUX : public ASDM_TABLE_SINGLETON<ASDM_CALFLUX>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALFLUX>;

 private:
  ASDM_CALFLUX();
  ~ASDM_CALFLUX();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALFOCUS : public ASDM_TABLE_SINGLETON<ASDM_CALFOCUS>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALFOCUS>;

 private:
  ASDM_CALFOCUS();
  ~ASDM_CALFOCUS();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALFOCUSMODEL : public ASDM_TABLE_SINGLETON<ASDM_CALFOCUSMODEL>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALFOCUSMODEL>;

 private:
  ASDM_CALFOCUSMODEL();
  ~ASDM_CALFOCUSMODEL();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALGAIN : public ASDM_TABLE_SINGLETON<ASDM_CALGAIN>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALGAIN>;

 private:
  ASDM_CALGAIN();
  ~ASDM_CALGAIN();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALHOLOGRAPHY : public ASDM_TABLE_SINGLETON<ASDM_CALHOLOGRAPHY>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALHOLOGRAPHY>;

 private:
  ASDM_CALHOLOGRAPHY();
  ~ASDM_CALHOLOGRAPHY();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALPHASE : public ASDM_TABLE_SINGLETON<ASDM_CALPHASE>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALPHASE>;

 private:
  ASDM_CALPHASE();
  ~ASDM_CALPHASE();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALPOINTING : public ASDM_TABLE_SINGLETON<ASDM_CALPOINTING>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALPOINTING>;

 private:
  ASDM_CALPOINTING();
  ~ASDM_CALPOINTING();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALPOINTINGMODEL : public ASDM_TABLE_SINGLETON<ASDM_CALPOINTINGMODEL>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALPOINTINGMODEL>;

 private:
  ASDM_CALPOINTINGMODEL();
  ~ASDM_CALPOINTINGMODEL();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALPOSITION : public ASDM_TABLE_SINGLETON<ASDM_CALPOSITION>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALPOSITION>;

 private:
  ASDM_CALPOSITION();
  ~ASDM_CALPOSITION();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALPRIMARYBEAM : public ASDM_TABLE_SINGLETON<ASDM_CALPRIMARYBEAM>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALPRIMARYBEAM>;

 private:
  ASDM_CALPRIMARYBEAM();
  ~ASDM_CALPRIMARYBEAM();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALREDUCTION : public ASDM_TABLE_SINGLETON<ASDM_CALREDUCTION>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALREDUCTION>;

 private:
  ASDM_CALREDUCTION();
  ~ASDM_CALREDUCTION();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALSEEING : public ASDM_TABLE_SINGLETON<ASDM_CALSEEING>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALSEEING>;

 private:
  ASDM_CALSEEING();
  ~ASDM_CALSEEING();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CALWVR : public ASDM_TABLE_SINGLETON<ASDM_CALWVR>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CALWVR>;

 private:
  ASDM_CALWVR();
  ~ASDM_CALWVR();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CONFIGDESCRIPTION : public ASDM_TABLE_SINGLETON<ASDM_CONFIGDESCRIPTION>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CONFIGDESCRIPTION>;

 private:
  ASDM_CONFIGDESCRIPTION();
  ~ASDM_CONFIGDESCRIPTION();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_CORRELATORMODE : public ASDM_TABLE_SINGLETON<ASDM_CORRELATORMODE>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_CORRELATORMODE>;

 private:
  ASDM_CORRELATORMODE();
  ~ASDM_CORRELATORMODE();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_DATADESCRIPTION : public ASDM_TABLE_SINGLETON<ASDM_DATADESCRIPTION>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_DATADESCRIPTION>;

 private:
  ASDM_DATADESCRIPTION();
  ~ASDM_DATADESCRIPTION();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_DELAYMODEL : public ASDM_TABLE_SINGLETON<ASDM_DELAYMODEL>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_DELAYMODEL>;

 private:
  ASDM_DELAYMODEL();
  ~ASDM_DELAYMODEL();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_DELAYMODELFIXEDPARAMETERS : public ASDM_TABLE_SINGLETON<ASDM_DELAYMODELFIXEDPARAMETERS>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_DELAYMODELFIXEDPARAMETERS>;

 private:
  ASDM_DELAYMODELFIXEDPARAMETERS();
  ~ASDM_DELAYMODELFIXEDPARAMETERS();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_DELAYMODELVARIABLEPARAMETERS : public ASDM_TABLE_SINGLETON<ASDM_DELAYMODELVARIABLEPARAMETERS>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_DELAYMODELVARIABLEPARAMETERS>;

 private:
  ASDM_DELAYMODELVARIABLEPARAMETERS();
  ~ASDM_DELAYMODELVARIABLEPARAMETERS();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_DOPPLER : public ASDM_TABLE_SINGLETON<ASDM_DOPPLER>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_DOPPLER>;

 private:
  ASDM_DOPPLER();
  ~ASDM_DOPPLER();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_EPHEMERIS : public ASDM_TABLE_SINGLETON<ASDM_EPHEMERIS>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_EPHEMERIS>;

 private:
  ASDM_EPHEMERIS();
  ~ASDM_EPHEMERIS();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_EXECBLOCK : public ASDM_TABLE_SINGLETON<ASDM_EXECBLOCK>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_EXECBLOCK>;

 private:
  ASDM_EXECBLOCK();
  ~ASDM_EXECBLOCK();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_FEED : public ASDM_TABLE_SINGLETON<ASDM_FEED>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_FEED>;

 private:
  ASDM_FEED();
  ~ASDM_FEED();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_FIELD : public ASDM_TABLE_SINGLETON<ASDM_FIELD>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_FIELD>;

 private:
  ASDM_FIELD();
  ~ASDM_FIELD();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_FLAG : public ASDM_TABLE_SINGLETON<ASDM_FLAG>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_FLAG>;

 private:
  ASDM_FLAG();
  ~ASDM_FLAG();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_FLAGCMD : public ASDM_TABLE_SINGLETON<ASDM_FLAGCMD>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_FLAGCMD>;

 private:
  ASDM_FLAGCMD();
  ~ASDM_FLAGCMD();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_FOCUS : public ASDM_TABLE_SINGLETON<ASDM_FOCUS>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_FOCUS>;

 private:
  ASDM_FOCUS();
  ~ASDM_FOCUS();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_FOCUSMODEL : public ASDM_TABLE_SINGLETON<ASDM_FOCUSMODEL>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_FOCUSMODEL>;

 private:
  ASDM_FOCUSMODEL();
  ~ASDM_FOCUSMODEL();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_FREQOFFSET : public ASDM_TABLE_SINGLETON<ASDM_FREQOFFSET>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_FREQOFFSET>;

 private:
  ASDM_FREQOFFSET();
  ~ASDM_FREQOFFSET();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_GAINTRACKING : public ASDM_TABLE_SINGLETON<ASDM_GAINTRACKING>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_GAINTRACKING>;

 private:
  ASDM_GAINTRACKING();
  ~ASDM_GAINTRACKING();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_HISTORY : public ASDM_TABLE_SINGLETON<ASDM_HISTORY>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_HISTORY>;

 private:
  ASDM_HISTORY();
  ~ASDM_HISTORY();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_HOLOGRAPHY : public ASDM_TABLE_SINGLETON<ASDM_HOLOGRAPHY>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_HOLOGRAPHY>;

 private:
  ASDM_HOLOGRAPHY();
  ~ASDM_HOLOGRAPHY();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_OBSERVATION : public ASDM_TABLE_SINGLETON<ASDM_OBSERVATION>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_OBSERVATION>;

 private:
  ASDM_OBSERVATION();
  ~ASDM_OBSERVATION();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_POINTING : public ASDM_TABLE_SINGLETON<ASDM_POINTING>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_POINTING>;

 private:
  ASDM_POINTING();
  ~ASDM_POINTING();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_POINTINGMODEL : public ASDM_TABLE_SINGLETON<ASDM_POINTINGMODEL>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_POINTINGMODEL>;

 private:
  ASDM_POINTINGMODEL();
  ~ASDM_POINTINGMODEL();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_POLARIZATION : public ASDM_TABLE_SINGLETON<ASDM_POLARIZATION>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_POLARIZATION>;

 private:
  ASDM_POLARIZATION();
  ~ASDM_POLARIZATION();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_PROCESSOR : public ASDM_TABLE_SINGLETON<ASDM_PROCESSOR>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_PROCESSOR>;

 private:
  ASDM_PROCESSOR();
  ~ASDM_PROCESSOR();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_RECEIVER : public ASDM_TABLE_SINGLETON<ASDM_RECEIVER>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_RECEIVER>;

 private:
  ASDM_RECEIVER();
  ~ASDM_RECEIVER();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_SBSUMMARY : public ASDM_TABLE_SINGLETON<ASDM_SBSUMMARY>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_SBSUMMARY>;

 private:
  ASDM_SBSUMMARY();
  ~ASDM_SBSUMMARY();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_SCALE : public ASDM_TABLE_SINGLETON<ASDM_SCALE>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_SCALE>;

 private:
  ASDM_SCALE();
  ~ASDM_SCALE();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_SCAN : public ASDM_TABLE_SINGLETON<ASDM_SCAN>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_SCAN>;

 private:
  ASDM_SCAN();
  ~ASDM_SCAN();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_SEEING : public ASDM_TABLE_SINGLETON<ASDM_SEEING>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_SEEING>;

 private:
  ASDM_SEEING();
  ~ASDM_SEEING();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_SOURCE : public ASDM_TABLE_SINGLETON<ASDM_SOURCE>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_SOURCE>;

 private:
  ASDM_SOURCE();
  ~ASDM_SOURCE();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_SPECTRALWINDOW : public ASDM_TABLE_SINGLETON<ASDM_SPECTRALWINDOW>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_SPECTRALWINDOW>;

 private:
  ASDM_SPECTRALWINDOW();
  ~ASDM_SPECTRALWINDOW();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_SQUARELAWDETECTOR : public ASDM_TABLE_SINGLETON<ASDM_SQUARELAWDETECTOR>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_SQUARELAWDETECTOR>;

 private:
  ASDM_SQUARELAWDETECTOR();
  ~ASDM_SQUARELAWDETECTOR();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_STATE : public ASDM_TABLE_SINGLETON<ASDM_STATE>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_STATE>;

 private:
  ASDM_STATE();
  ~ASDM_STATE();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_STATION : public ASDM_TABLE_SINGLETON<ASDM_STATION>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_STATION>;

 private:
  ASDM_STATION();
  ~ASDM_STATION();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_SUBSCAN : public ASDM_TABLE_SINGLETON<ASDM_SUBSCAN>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_SUBSCAN>;

 private:
  ASDM_SUBSCAN();
  ~ASDM_SUBSCAN();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_SWITCHCYCLE : public ASDM_TABLE_SINGLETON<ASDM_SWITCHCYCLE>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_SWITCHCYCLE>;

 private:
  ASDM_SWITCHCYCLE();
  ~ASDM_SWITCHCYCLE();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_SYSCAL : public ASDM_TABLE_SINGLETON<ASDM_SYSCAL>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_SYSCAL>;

 private:
  ASDM_SYSCAL();
  ~ASDM_SYSCAL();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_SYSPOWER : public ASDM_TABLE_SINGLETON<ASDM_SYSPOWER>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_SYSPOWER>;

 private:
  ASDM_SYSPOWER();
  ~ASDM_SYSPOWER();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_TOTALPOWER : public ASDM_TABLE_SINGLETON<ASDM_TOTALPOWER>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_TOTALPOWER>;

 private:
  ASDM_TOTALPOWER();
  ~ASDM_TOTALPOWER();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_WVMCAL : public ASDM_TABLE_SINGLETON<ASDM_WVMCAL>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_WVMCAL>;

 private:
  ASDM_WVMCAL();
  ~ASDM_WVMCAL();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	
class ASDM_WEATHER : public ASDM_TABLE_SINGLETON<ASDM_WEATHER>, public ASDM_TABLE_BASE {
  friend class ASDM_TABLE_SINGLETON<ASDM_WEATHER>;

 private:
  ASDM_WEATHER();
  ~ASDM_WEATHER();
  casacore::TableDesc tableDesc_;

 public:
  const casacore::TableDesc& tableDesc() const;
  void fill(const ASDM& asdm);
};
	

#endif // _ASDMTABLES_H_ 
