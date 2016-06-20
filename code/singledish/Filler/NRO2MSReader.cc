/*
 * NRO2MSReader.cc
 *
 *  Created on: May 9, 2016
 *      Author: wataru kawasaki
 */

#include <singledish/Filler/NRO2MSReader.h>

#include <iostream>
#include <string>

#include <casacore/casa/OS/File.h>
#include <casacore/casa/Containers/Record.h>
#include <casacore/casa/Utilities/Regex.h>
#include <casacore/tables/Tables/TableRecord.h>
#include <casacore/casa/Arrays/Vector.h>
#include <casacore/casa/Arrays/ArrayMath.h>
#include <casacore/casa/Arrays/ArrayIO.h>
#include <casacore/tables/Tables/Table.h>

using namespace casacore;
using namespace sdfiller;

namespace {
Double queryAntennaDiameter(String const &name) {
  String capitalized = name;
  capitalized.upcase();
  Double diameter = 0.0;
  if (capitalized.matches(Regex(".*(DV|DA|PM)[0-9]+$"))) {
    diameter = 12.0;
  } else if (capitalized.matches(Regex(".*CM[0-9]+$"))) {
    diameter = 7.0;
  } else if (capitalized.contains("GBT")) {
    diameter = 104.9;
  } else if (capitalized.contains("MOPRA")) {
    diameter = 22.0;
  } else if (capitalized.contains("PKS") || capitalized.contains("PARKS")) {
    diameter = 64.0;
  } else if (capitalized.contains("TIDBINBILLA")) {
    diameter = 70.0;
  } else if (capitalized.contains("CEDUNA")) {
    diameter = 30.0;
  } else if (capitalized.contains("HOBART")) {
    diameter = 26.0;
  } else if (capitalized.contains("APEX")) {
    diameter = 12.0;
  } else if (capitalized.contains("ASTE")) {
    diameter = 10.0;
  } else if (capitalized.contains("NRO")) {
    diameter = 45.0;
  }

  return diameter;
}

template<class T, class U>
U getMapValue(std::map<T, U> const mymap, T const key, U const default_value) {
  auto iter = mymap.find(key);
  if (iter != mymap.end()) {
    return iter->second;
  } else {
    return default_value;
  }
}

String getIntent(Int srctype) {
  static std::map<Int, String> intent_map;
  if (intent_map.size() == 0) {
    String sep1 = "#";
    String sep2 = ",";
    String target = "OBSERVE_TARGET";
    String atmcal = "CALIBRATE_ATMOSPHERE";
    String anycal = "CALIBRATE_SOMETHING";
    String onstr = "ON_SOURCE";
    String offstr = "OFF_SOURCE";
    String pswitch = "POSITION_SWITCH";
    String nod = "NOD";
    String fswitch = "FREQUENCY_SWITCH";
    String sigstr = "SIG";
    String refstr = "REF";
    String hot = "HOT";
    String warm = "WARM";
    String cold = "COLD";
    String unspecified = "UNSPECIFIED";
    String ftlow = "LOWER";
    String fthigh = "HIGHER";
    intent_map[0] = target + sep1 + onstr + sep2 + pswitch;
    intent_map[1] = target + sep1 + offstr + sep2 + pswitch;
    intent_map[2] = target + sep1 + onstr + sep2 + nod;
    intent_map[3] = target + sep1 + onstr + sep2 + fswitch + sep1 + sigstr;
    intent_map[4] = target + sep1 + onstr + sep2 + fswitch + sep1 + refstr;
    intent_map[6] = atmcal + sep1 + offstr + sep2 + unspecified;
    intent_map[7] = atmcal + sep1 + hot + sep2 + unspecified;
    intent_map[8] = atmcal + sep1 + warm + sep2 + unspecified;
    intent_map[9] = atmcal + sep1 + cold + sep2 + unspecified;
    intent_map[10] = atmcal + sep1 + onstr + sep2 + pswitch;
    intent_map[11] = atmcal + sep1 + offstr + sep2 + pswitch;
    intent_map[12] = atmcal + sep1 + onstr + sep2 + nod;
    intent_map[13] = atmcal + sep1 + onstr + sep2 + fswitch + sep1 + sigstr;
    intent_map[14] = atmcal + sep1 + offstr + sep2 + fswitch + sep1 + refstr;
    intent_map[20] = target + sep1 + onstr + sep2 + fswitch + sep1 + ftlow;
    intent_map[21] = target + sep1 + offstr + sep2 + fswitch + sep1 + ftlow;
    intent_map[26] = atmcal + sep1 + offstr + sep2 + fswitch + sep1 + ftlow;
    intent_map[27] = atmcal + sep1 + offstr + sep2 + fswitch + sep1 + ftlow;
    intent_map[28] = atmcal + sep1 + offstr + sep2 + fswitch + sep1 + ftlow;
    intent_map[29] = atmcal + sep1 + offstr + sep2 + fswitch + sep1 + ftlow;
    intent_map[30] = target + sep1 + onstr + sep2 + fswitch + sep1 + fthigh;
    intent_map[31] = target + sep1 + offstr + sep2 + fswitch + sep1 + fthigh;
    intent_map[36] = atmcal + sep1 + offstr + sep2 + fswitch + sep1 + fthigh;
    intent_map[37] = atmcal + sep1 + offstr + sep2 + fswitch + sep1 + fthigh;
    intent_map[38] = atmcal + sep1 + offstr + sep2 + fswitch + sep1 + fthigh;
    intent_map[39] = atmcal + sep1 + offstr + sep2 + fswitch + sep1 + fthigh;
    intent_map[90] = target + sep1 + onstr + sep2 + unspecified;
    intent_map[91] = target + sep1 + offstr + sep2 + unspecified;
    intent_map[92] = anycal + sep1 + offstr + sep2 + unspecified;
  }
  String default_type = "UNKNOWN_INTENT";
  return getMapValue(intent_map, srctype, default_type);
}

Int getSubscan(Int srctype) {
  static std::map<Int, Int> subscan_map;
  if (subscan_map.size() == 0) {
    subscan_map[0] = 1;
    subscan_map[1] = 2;
    subscan_map[2] = 1;
    subscan_map[3] = 1;
    subscan_map[4] = 2;
    subscan_map[6] = 1;
    subscan_map[7] = 2;
    subscan_map[8] = 3;
    subscan_map[9] = 4;
    subscan_map[10] = 5;
    subscan_map[11] = 6;
    subscan_map[12] = 7;
    subscan_map[13] = 8;
    subscan_map[14] = 9;
    subscan_map[20] = 1;
    subscan_map[21] = 2;
    subscan_map[26] = 1;
    subscan_map[27] = 2;
    subscan_map[28] = 3;
    subscan_map[29] = 4;
    subscan_map[30] = 3;
    subscan_map[31] = 4;
    subscan_map[36] = 5;
    subscan_map[37] = 6;
    subscan_map[38] = 7;
    subscan_map[39] = 8;
    subscan_map[90] = 1;
    subscan_map[91] = 2;
    subscan_map[92] = 1;
  }
  Int default_subscan = 1;
  return getMapValue(subscan_map, srctype, default_subscan);
}
}

namespace casa { //# NAMESPACE CASA - BEGIN

NRO2MSReader::NRO2MSReader(std::string const &scantable_name) :
  ReaderInterface(scantable_name), fp_(NULL), obs_header_(),
  beam_id_counter_(0), source_spw_id_counter_(0), spw_id_counter_(0),
  time_range_sec_(),
  get_antenna_row_(&NRO2MSReader::getAntennaRowImpl),
  get_field_row_(&NRO2MSReader::getFieldRowImpl),
  get_observation_row_(&NRO2MSReader::getObservationRowImpl),
  get_processor_row_(&NRO2MSReader::getProcessorRowImpl),
  get_source_row_(&NRO2MSReader::getSourceRowImpl),
  get_spw_row_(&NRO2MSReader::getSpectralWindowRowImpl)
  {
//  std::cout << "NRO2MSReader::NRO2MSReader" << std::endl;
}

NRO2MSReader::~NRO2MSReader() {
//  std::cout << "NRO2MSReader::~NRO2MSReader" << std::endl;
}

void NRO2MSReader::checkEndian() {
  fseek(fp_, 144, SEEK_SET) ;
  int tmp ;
  if (fread(&tmp, 1, sizeof(int), fp_) != sizeof(int)) {
    return ;
  }
  if ((0 < tmp)&&(tmp <= NRO_ARYMAX)) {
    same_endian_ = true;
  } else {
    same_endian_ = false;
  }
}
  
void NRO2MSReader::readObsHeader() {
  fseek(fp_, 0, SEEK_SET);

  readHeader(obs_header_.LOFIL0, 8);
  readHeader(obs_header_.VER0,   8);
  readHeader(obs_header_.GROUP0, 16);
  readHeader(obs_header_.PROJ0,  16);
  readHeader(obs_header_.SCHED0, 24);
  readHeader(obs_header_.OBSVR0, 40);
  readHeader(obs_header_.LOSTM0, 16);
  readHeader(obs_header_.LOETM0, 16);
  readHeader(obs_header_.ARYNM0);
  readHeader(obs_header_.NSCAN0);
  readHeader(obs_header_.TITLE0, 120);
  readHeader(obs_header_.OBJ0, 16);
  readHeader(obs_header_.EPOCH0, 8);
  readHeader(obs_header_.RA00);
  readHeader(obs_header_.DEC00);
  readHeader(obs_header_.GL00);
  readHeader(obs_header_.GB00);
  readHeader(obs_header_.NCALB0);
  readHeader(obs_header_.SCNCD0);
  readHeader(obs_header_.SCMOD0, 120);
  readHeader<double>(obs_header_.VEL0);
  readHeader(obs_header_.VREF0,  4);
  readHeader(obs_header_.VDEF0,  4);
  readHeader(obs_header_.SWMOD0, 8);
  readHeader<double>(obs_header_.FRQSW0);
  readHeader<double>(obs_header_.DBEAM0);
  readHeader<double>(obs_header_.MLTOF0);
  readHeader<double>(obs_header_.CMTQ0);
  readHeader<double>(obs_header_.CMTE0);
  readHeader<double>(obs_header_.CMTSOM0);
  readHeader<double>(obs_header_.CMTNODE0);
  readHeader<double>(obs_header_.CMTI0);
  readHeader(obs_header_.CMTTMO0, 24);
  readHeader<double>(obs_header_.SBDX0);
  readHeader<double>(obs_header_.SBDY0);
  readHeader<double>(obs_header_.SBDZ10);
  readHeader<double>(obs_header_.SBDZ20);
  readHeader<double>(obs_header_.DAZP0);
  readHeader<double>(obs_header_.DELP0);
  readHeader<int>(obs_header_.CBIND0);
  readHeader<int>(obs_header_.NCH0);
  readHeader<int>(obs_header_.CHRANGE0, 2);
  readHeader<double>(obs_header_.ALCTM0);
  readHeader<double>(obs_header_.IPTIM0);
  readHeader<double>(obs_header_.PA0);
  readHeader(obs_header_.RX0, 16, NRO_ARYMAX);
  readHeader<double>(obs_header_.HPBW0, NRO_ARYMAX);
  readHeader<double>(obs_header_.EFFA0, NRO_ARYMAX);
  readHeader<double>(obs_header_.EFFB0, NRO_ARYMAX);
  readHeader<double>(obs_header_.EFFL0, NRO_ARYMAX);
  readHeader<double>(obs_header_.EFSS0, NRO_ARYMAX);
  readHeader<double>(obs_header_.GAIN0, NRO_ARYMAX);
  readHeader(obs_header_.HORN0, 4, NRO_ARYMAX);
  readHeader(obs_header_.POLTP0, 4, NRO_ARYMAX);
  readHeader<double>(obs_header_.POLDR0, NRO_ARYMAX);
  readHeader<double>(obs_header_.POLAN0, NRO_ARYMAX);
  readHeader<double>(obs_header_.DFRQ0, NRO_ARYMAX);
  readHeader(obs_header_.SIDBD0, 4, NRO_ARYMAX);
  readHeader<int>(obs_header_.REFN0, NRO_ARYMAX);
  readHeader<int>(obs_header_.IPINT0, NRO_ARYMAX);
  readHeader<int>(obs_header_.MULTN0, NRO_ARYMAX);
  readHeader<double>(obs_header_.MLTSCF0, NRO_ARYMAX);
  readHeader(obs_header_.LAGWIN0, 8, NRO_ARYMAX);
  readHeader<double>(obs_header_.BEBW0, NRO_ARYMAX);
  readHeader<double>(obs_header_.BERES0, NRO_ARYMAX);
  readHeader<double>(obs_header_.CHWID0, NRO_ARYMAX);
  readHeader<int>(obs_header_.ARRY0, NRO_ARYMAX);
  readHeader<int>(obs_header_.NFCAL0, NRO_ARYMAX);
  readHeader<double>(obs_header_.F0CAL0, NRO_ARYMAX);
  for (size_t i = 0; i < NRO_ARYMAX; ++i) {
    readHeader<double>(obs_header_.FQCAL0[i], 10);
  }
  for (size_t i = 0; i < NRO_ARYMAX; ++i) {
    readHeader<double>(obs_header_.CHCAL0[i], 10);
  }
  for (size_t i = 0; i < NRO_ARYMAX; ++i) {
    readHeader<double>(obs_header_.CWCAL0[i], 10);
  }
  readHeader<int>(obs_header_.SCNLEN0);
  readHeader<int>(obs_header_.SBIND0);
  readHeader<int>(obs_header_.IBIT0);
  readHeader(obs_header_.SITE0, 8);
  readHeader(obs_header_.TRK_TYPE, 8);
  readHeader(obs_header_.SCAN_COORD, 8);
  readHeader<int>(obs_header_.NBEAM);
  readHeader<int>(obs_header_.NPOL);
  readHeader<int>(obs_header_.NSPWIN);
  readHeader<int>(obs_header_.CHMAX);
  readHeader(obs_header_.VERSION, 40);
  readHeader(obs_header_.CDMY1, 108);
}

void NRO2MSReader::readScanData(int const irow, sdfiller::NRODataScanData &data) {
  fseek(fp_, len_obs_header_ + irow * obs_header_.SCNLEN0, SEEK_SET);

  readHeader(data.LSFIL0, 4);
  readHeader(data.ISCN0);
  readHeader(data.LAVST0, 24);
  readHeader(data.SCNTP0, 8);
  readHeader(data.DSCX0);
  readHeader(data.DSCY0);
  readHeader(data.SCX0);
  readHeader(data.SCY0);
  readHeader(data.PAZ0);
  readHeader(data.PEL0);
  readHeader(data.RAZ0);
  readHeader(data.REL0);
  readHeader(data.XX0);
  readHeader(data.YY0);
  readHeader(data.ARRYT0, 4);
  readHeader(data.TEMP0);
  readHeader(data.PATM0);
  readHeader(data.PH200);
  readHeader(data.VWIND0);
  readHeader(data.DWIND0);
  readHeader(data.TAU0);
  readHeader(data.TSYS0);
  readHeader(data.BATM0);
  readHeader(data.LINE0);
  readHeader(data.IDMY1, 4);
  readHeader(data.VRAD0);
  readHeader(data.FRQ00);
  readHeader(data.FQTRK0);
  readHeader(data.FQIF10);
  readHeader(data.ALCV0);
  for (size_t i = 0; i < 2; ++i) {
    readHeader(data.OFFCD0[i], 2);
  }
  readHeader(data.IDMY0);
  readHeader(data.IDMY2);
  readHeader(data.DPFRQ0);
  readHeader(data.CDMY1, 144);
  readHeader(data.SFCTR0);
  readHeader(data.ADOFF0);
  readHeader(data.LDATA, obs_header_.CHMAX * obs_header_.IBIT0 / 8);
}
  
double NRO2MSReader::getMJD(string const &strtime) {
  // TODO: should be checked which time zone the time depends on
  // 2008/11/14 Takeshi Nakazato
  string strYear = strtime.substr(0, 4);
  string strMonth = strtime.substr(4, 2);
  string strDay = strtime.substr(6, 2);
  string strHour = strtime.substr(8, 2);
  string strMinute = strtime.substr(10, 2);
  string strSecond = strtime.substr(12, strtime.size() - 12);
  unsigned int year = atoi(strYear.c_str());
  unsigned int month = atoi(strMonth.c_str());
  unsigned int day = atoi(strDay.c_str());
  unsigned int hour = atoi(strHour.c_str());
  unsigned int minute = atoi(strMinute.c_str());
  double second = atof(strSecond.c_str());
  Time t(year, month, day, hour, minute, second);

  return t.modifiedJulianDay() ;
}

double NRO2MSReader::getIntMiddleTimeSec(sdfiller::NRODataScanData const &data) {
  return getMJD(data.LAVST0) * kDay2Sec + 0.5 * obs_header_.IPTIM0;
}

double NRO2MSReader::getIntStartTimeSec(int const scanno) {
  fseek(fp_, len_obs_header_ + scanno * obs_header_.ARYNM0 * obs_header_.SCNLEN0 + 8, SEEK_SET);
  string time_header;
  readHeader(time_header, 24);
  return getMJD(time_header) * kDay2Sec;
}

double NRO2MSReader::getIntEndTimeSec(int const scanno) {
  double interval = obs_header_.IPTIM0;
  return getIntStartTimeSec(scanno) + interval;
}

void NRO2MSReader::getFullTimeRange() {
  if (time_range_sec_.size() != 2) {
    time_range_sec_.resize(2);
  }
  time_range_sec_[0] = getIntStartTimeSec(0);
  time_range_sec_[1] = getIntEndTimeSec(obs_header_.NSCAN0 - 1);
}

double NRO2MSReader::getMiddleOfTimeRangeSec() {
  return 0.5 * (time_range_sec_[0] + time_range_sec_[1]);
}

double NRO2MSReader::getRestFrequency(int const spwno) {
  fseek(fp_, len_obs_header_ + spwno * obs_header_.SCNLEN0 + 184, SEEK_SET);
  double restfreq_header;
  readHeader(restfreq_header);
  return restfreq_header;
}

std::vector<double> NRO2MSReader::getSpectrum(int const irow, sdfiller::NRODataScanData const &data) {
  // size of spectrum is not (SCNLEN-HEADER_SIZE)*8/IBIT0
  // but obs_header_.NCH0 after binding
  int const nchan = obs_header_.NCH0;
  int const chmax_ = (obs_header_.SCNLEN0 - SCNLEN_HEADER_SIZE) * 8 / obs_header_.IBIT0;
  vector<double> spec( chmax_ ) ;  // spectrum "before" binding
  // DEBUG
  //cout << "NRODataset::getSpectrum()  nchan = " << nchan << " chmax_ = " << chmax_ << endl ;

  const int bit = obs_header_.IBIT0;   // fixed to 12 bit
  double scale = data.SFCTR0;
  // DEBUG
  //cout << "NRODataset::getSpectrum()  scale = " << scale << endl ;
  double offset = data.ADOFF0;
  // DEBUG
  //cout << "NRODataset::getSpectrum()  offset = " << offset << endl ;

  if ((scale == 0.0) && (offset == 0.0)) {
    LogIO os( LogOrigin("NRODataset","getSpectrum",WHERE) );
    os << LogIO::WARN << "zero spectrum for row " << irow << LogIO::POST;
    if (spec.size() != (unsigned int)nchan) {
      spec.resize(nchan);
    }
    for (vector<double>::iterator i = spec.begin(); i != spec.end(); ++i) {
      *i = 0.0;
    }
    return spec;
  }
  
  unsigned char *cdata = reinterpret_cast<unsigned char *>(const_cast<char *>(data.LDATA.c_str()));
  vector<double> mscale;
  mscale.resize(NRO_ARYMAX);
  for (size_t i = 0; i < NRO_ARYMAX; ++i) {
    mscale[i] = obs_header_.MLTSCF0[i];
  }
  string sbeamno = data.ARRYT0.substr(1, data.ARRYT0.size() - 1);
  int index = atoi(sbeamno.c_str()) - 1;
  double dscale = mscale[index];
  int cbind = obs_header_.CBIND0;
  int chmin = obs_header_.CHRANGE0[0];

  // char -> int -> double
  vector<double>::iterator iter = spec.begin();

  static const int shift_right[] = { 4, 0 };
  static const int start_pos[]   = { 0, 1 };
  static const int incr[]        = { 0, 3 };
  int j = 0;
  for (int i = 0; i < chmax_; ++i) {
    // char -> int
    int ivalue = 0;
    if (bit == 12) {  // 12 bit qunatization
      const int ialt = i & 1;
      const int idx = j + start_pos[ialt];
      const unsigned tmp = unsigned(cdata[idx]) << 8 | cdata[idx + 1];
      ivalue = int((tmp >> shift_right[ialt]) & 0xFFF);
      j += incr[ialt];
    }

    if ((ivalue < 0) || (ivalue > 4096)) {
      //cerr << "NRODataset::getSpectrum()  ispec[" << i << "] is out of range" << endl ;
      LogIO os( LogOrigin( "NRODataset", "getSpectrum", WHERE));
      os << LogIO::SEVERE << "ivalue for row " << i << " is out of range" << LogIO::EXCEPTION;
      if (spec.size() != (unsigned int)nchan) {
        spec.resize(nchan);
      }
      for (vector<double>::iterator i = spec.begin(); i != spec.end(); ++i) {
        *i = 0.0;
      }
      return spec;
    }
    
    // int -> double
    *iter = (double)(ivalue * scale + offset) * dscale; 
    // DEBUG
    //cout << "NRODataset::getSpectrum()  spec[" << i << "] = " << *iter << endl ;
    iter++;
  }

  // channel binding if necessary
  if (cbind != 1) {
    iter = spec.begin();
    advance(iter, chmin);
    vector<double>::iterator iter2 = spec.begin();
    for (int i = 0; i < nchan; ++i) {
      double sum0 = 0;
      double sum1 = 0;
      for (int j = 0; j < cbind; ++j) {
        sum0 += *iter;
        sum1 += 1.0;
        iter++;
      }
      *iter2 = sum0 / sum1;
      iter2++;
      // DEBUG
      //cout << "NRODataset::getSpectrum()  bspec[" << i << "] = " << bspec[i] << endl ;
    }
    spec.resize(nchan);
  }

  // DEBUG
  //cout << "NRODataset::getSpectrum() end process" << endl ;
  return spec;
}

Int NRO2MSReader::getPolNo(string const &rx) {
  Int polno = 0;
  // 2013/01/23 TN
  // In NRO 45m telescope, naming convension for dual-polarization 
  // receiver is as follows:
  // 
  //    xxxH for horizontal component,
  //    xxxV for vertical component.
  // 
  // Exception is H20ch1/ch2.
  // Here, POLNO is assigned as follows:
  // 
  //    POLNO=0: xxxH or H20ch1
  //          1: xxxV or H20ch2
  //
  // For others, POLNO is always 0.
  string last_letter = rx.substr(rx.size()-1, 1);
  if ((last_letter == "V") || (rx == "H20ch2")) {
    polno = 1;
  }

  return polno;
}

void NRO2MSReader::initializeSpecific() {
//  std::cout << "NRO2MSReader::initialize" << std::endl;
  fp_ = fopen(STRING2CHAR(name_), "rb");
  if (fp_ == NULL) {
    throw AipsError("Input data doesn't exist or is invalid");
  }
  checkEndian();
  
  readObsHeader();
  getFullTimeRange();
}

void NRO2MSReader::finalizeSpecific() {
//  std::cout << "NRO2MSReader::finalize" << std::endl;
  if (fp_ != NULL) {
    fclose(fp_);
  }
  fp_ = NULL;
}

size_t NRO2MSReader::getNumberOfRows() {
  int nrows = obs_header_.ARYNM0 * obs_header_.NSCAN0;
  return (nrows >= 0) ? nrows : 0;
}

MDirection::Types NRO2MSReader::getDirectionFrame() {
  string epoch = obs_header_.EPOCH0;
  MDirection::Types res;
  if (epoch == "J2000") {
    res = MDirection::J2000;
  } else if (epoch == "B1950") {
    res = MDirection::B1950;
  } else {
    throw AipsError("Unsupported epoch.");
  }
  return res;
}

Bool NRO2MSReader::getAntennaRowImpl(AntennaRecord &record) {
//  std::cout << "NRO2MSReader::getAntennaRowImpl" << std::endl;

  record.station = "";
  String header_antenna_name = obs_header_.SITE0;
  ostringstream oss;
  oss << header_antenna_name << "-BEAM" << beam_id_counter_;
  record.name = String(oss);
  record.mount = "ALT-AZ";
  record.dish_diameter = queryAntennaDiameter(header_antenna_name);
  record.type = "GROUND-BASED";
  record.position = MPosition(MVPosition(posx_, posy_, posz_), MPosition::ITRF);

  beam_id_counter_++;
  if (obs_header_.NBEAM <= beam_id_counter_) {
    get_antenna_row_ = &NRO2MSReader::noMoreRowImpl<AntennaRecord>;
  }

  return True;
}

Bool NRO2MSReader::getObservationRowImpl(ObservationRecord &record) {
//  std::cout << "NRO2MSReader::getObservationRowImpl" << std::endl;

  if (record.time_range.size() != 2) {
    record.time_range.resize(2);
  }
  for (int i = 0; i < 2; ++i) {
    record.time_range[i] = time_range_sec_[i] * kSec2Day;
  }

  record.observer = obs_header_.OBSVR0;
  record.project = obs_header_.PROJ0;
  record.telescope_name = obs_header_.SITE0;

  // only one entry so redirect function pointer to noMoreRowImpl
  get_observation_row_ = &NRO2MSReader::noMoreRowImpl<ObservationRecord>;

  return True;
}


Bool NRO2MSReader::getProcessorRowImpl(ProcessorRecord &/*record*/) {
//  std::cout << "NRO2MSReader::getProcessorRowImpl" << std::endl;

  // just add empty row once

  // only one entry so redirect function pointer to noMoreRowImpl
  get_processor_row_ = &NRO2MSReader::noMoreRowImpl<ProcessorRecord>;

  return True;
}

Bool NRO2MSReader::getSourceRowImpl(SourceRecord &record) {
  record.name = obs_header_.OBJ0;
  record.source_id = 0;
  record.spw_id = source_spw_id_counter_;
  record.direction = MDirection(
      Quantity(obs_header_.RA00, "rad"),
      Quantity(obs_header_.DEC00, "rad"),
      getDirectionFrame());
  record.proper_motion = Vector<Double>(0.0, 0.0);
  record.rest_frequency = getRestFrequency(source_spw_id_counter_);
  //record.transition
  record.num_lines = 1;
  record.sysvel = obs_header_.VEL0;
  double time_sec = getMiddleOfTimeRangeSec();
  record.time = time_sec;
  record.interval = time_sec - time_range_sec_[0];

  source_spw_id_counter_++;
  if (obs_header_.NSPWIN <= source_spw_id_counter_) {
    get_source_row_ = &NRO2MSReader::noMoreRowImpl<SourceRecord>;
  }

  return True;
}

Bool NRO2MSReader::getFieldRowImpl(FieldRecord &record) {
  record.name = obs_header_.OBJ0;
  record.field_id = 0;
  record.time = getMiddleOfTimeRangeSec();
  record.source_name = obs_header_.OBJ0;
  record.frame = getDirectionFrame();
  Matrix<Double> direction0(2, 2, 0.0);
  Matrix<Double> direction(direction0(IPosition(2, 0, 0), IPosition(2, 1, 0)));
  direction(0, 0) = obs_header_.RA00;
  direction(1, 0) = obs_header_.DEC00;
  record.direction = direction;

  // only one entry so redirect function pointer to noMoreRowImpl
  get_field_row_ = &NRO2MSReader::noMoreRowImpl<FieldRecord>;

  return True;
}

Bool NRO2MSReader::getSpectralWindowRowImpl(
    SpectralWindowRecord &record) {
  record.spw_id = spw_id_counter_;
  record.num_chan = obs_header_.NCH0;
  MFrequency::Types frame_type;
  Bool status = MFrequency::getType(frame_type, obs_header_.VREF0);
  if (!status) {
    frame_type = MFrequency::N_Types;
  }
  record.meas_freq_ref = frame_type;
  record.refpix = obs_header_.CHCAL0[spw_id_counter_][0];
  record.refval = obs_header_.FQCAL0[spw_id_counter_][0];
  record.increment = obs_header_.CHWID0[spw_id_counter_];
  
  spw_id_counter_++;
  if (obs_header_.NSPWIN <= spw_id_counter_) {
    get_spw_row_ = &NRO2MSReader::noMoreRowImpl<SpectralWindowRecord>;
  }

  return True;
}

Bool NRO2MSReader::getData(size_t irow, DataRecord &record) {
//  std::cout << "NRO2MSReader::getData(irow=" << irow << ")" << std::endl;

  if (irow >= getNumberOfRows()) {
    return False;
  }

//  std::cout << "Accessing row " << irow << std::endl;
  NRODataScanData scan_data;
  readScanData(irow, scan_data);

  record.time = getIntMiddleTimeSec(scan_data);
  record.interval = obs_header_.IPTIM0;
//  std::cout << "TIME=" << record.time << " INTERVAL=" << record.interval
//      << std::endl;

  Int srctype = (scan_data.SCNTP0 == "ON") ? 90 : 91;
  record.intent = getIntent(srctype);
  record.scan = (Int)scan_data.ISCN0;
  record.subscan = getSubscan(srctype); 
  record.field_id = 0;
  Int ndata_per_ant = obs_header_.NPOL * obs_header_.NSPWIN;
  record.antenna_id = (Int)(irow / ndata_per_ant % obs_header_.NBEAM);
  record.direction_vector(0) = scan_data.SCX0;
  record.direction_vector(1) = scan_data.SCY0;
  record.scan_rate = 0.0;
  record.feed_id = (Int)(irow / ndata_per_ant % obs_header_.NBEAM);
  record.spw_id = (Int)(irow % obs_header_.NSPWIN);
  Int ndata_per_scan = obs_header_.NBEAM * obs_header_.NPOL * obs_header_.NSPWIN;
  record.polno = getPolNo(obs_header_.RX0[irow % ndata_per_scan]);
  record.pol_type = "linear";

//  std::cout << "set data size to " << num_chan_map_[record.spw_id] << " shape "
//      << record.data.shape() << std::endl;
  record.setDataSize(obs_header_.NCH0);
  record.data = getSpectrum(irow, scan_data);
  size_t flag_len = obs_header_.NCH0;
  for (size_t i = 0; i < flag_len; ++i) {
    record.flag(i) = False;
  }
  record.flag_row = False;

//    std::cout << "set tsys size to " << tsys_column_.shape(index)[0]
//        << " shape " << record.tsys.shape() << std::endl;
  record.setTsysSize(1);
  record.tsys(0) = scan_data.TSYS0;

  record.temperature = scan_data.TEMP0;
  record.pressure = scan_data.PATM0;
  record.rel_humidity = scan_data.PH200;
  record.wind_speed = scan_data.VWIND0;
  record.wind_direction = scan_data.DWIND0;

  return True;
}

} //# NAMESPACE CASA - END
