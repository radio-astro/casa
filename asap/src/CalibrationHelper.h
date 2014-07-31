#include <vector>

#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/CountedPtr.h>

#include "Scantable.h"
#include "STTcal.h"
#include "STIdxIter.h"
#include "STSelector.h"

using namespace casa;
using namespace asap;

namespace {
// Interpolation Helper
class TcalData
{
public:
  TcalData(CountedPtr<Scantable> s)
    : table_(s)
  {}
  ~TcalData() {}
  const String method_name() const {return "getTcalFromTime";}
  uInt nrow() const {return table_->nrow();}
  Vector<Float> GetEntry(int idx) const
  {
    String time;
    uInt tcalid = table_->getTcalId(idx);
    Vector<Float> return_value;
    table_->tcal().getEntry(time, return_value, tcalid);
    return return_value;
  }  
private:
  CountedPtr<Scantable> table_;
};
  
class TsysData
{
public:
  TsysData(CountedPtr<Scantable> s)
    : tsyscolumn_(s->table(), "TSYS")
  {}
  ~TsysData() {}
  const String method_name() const {return "getTsysFromTime";}
  uInt nrow() const {return tsyscolumn_.nrow();}
  Vector<Float> GetEntry(int idx) const {return tsyscolumn_(idx);}
private:
  ROArrayColumn<Float> tsyscolumn_;
};

class SpectralData
{
public:
  SpectralData(Matrix<Float> s)
    : data_(s)
  {}
  ~SpectralData() {}
  const String method_name() const {return "getSpectraFromTime";}
  uInt nrow() const {return data_.ncolumn();}
  Vector<Float> GetEntry(int idx) const {return data_.column(idx);}
private:
  Matrix<Float> data_;
};


vector<int> getRowIdFromTime(double reftime, const Vector<Double> &t)
{
  //   double reft = reftime ;
  double dtmin = 1.0e100 ;
  double dtmax = -1.0e100 ;
  //   vector<double> dt ;
  int just_before = -1 ;
  int just_after = -1 ;
  Vector<Double> dt = t - reftime ;
  for ( unsigned int i = 0 ; i < dt.size() ; i++ ) {
    if ( dt[i] > 0.0 ) {
      // after reftime
      if ( dt[i] < dtmin ) {
	just_after = i ;
	dtmin = dt[i] ;
      }
    }
    else if ( dt[i] < 0.0 ) {
      // before reftime
      if ( dt[i] > dtmax ) {
	just_before = i ;
	dtmax = dt[i] ;
      }
    }
    else {
      // just a reftime
      just_before = i ;
      just_after = i ;
      dtmax = 0 ;
      dtmin = 0 ;
      break ;
    }
  }
  
  vector<int> v(2) ;
  v[0] = just_before ;
  v[1] = just_after ;
  
  return v ;
}
  
template<class T>
class SimpleInterpolationHelper
{
 public:
  static Vector<Float> GetFromTime(double reftime,
				   const Vector<Double> &timeVec,
				   const vector<int> &idx,
				   const T &data,
				   const string mode)
  {
    Vector<Float> return_value;
    LogIO os_;
    LogIO os( LogOrigin( "STMath", data.method_name(), WHERE ) ) ;
    if ( data.nrow() == 0 ) {
      os << LogIO::SEVERE << "No row in the input scantable. Return empty tcal." << LogIO::POST ;
    }
    else if ( data.nrow() == 1 ) {
      return_value = data.GetEntry(0);
    }
    else {
      if ( mode == "before" ) {
	int id = -1 ;
	if ( idx[0] != -1 ) {
	  id = idx[0] ;
	}
	else if ( idx[1] != -1 ) {
	  os << LogIO::WARN << "Failed to find a scan before reftime. return a spectrum just after the reftime." << LogIO::POST ;
	  id = idx[1] ;
	}
	
	return_value = data.GetEntry(id);
      }
      else if ( mode == "after" ) {
	int id = -1 ;
	if ( idx[1] != -1 ) {
	  id = idx[1] ;
	}
	else if ( idx[0] != -1 ) {
	  os << LogIO::WARN << "Failed to find a scan after reftime. return a spectrum just before the reftime." << LogIO::POST ;
	  id = idx[1] ;
	}
	
	return_value = data.GetEntry(id);
      }
      else if ( mode == "nearest" ) {
	int id = -1 ;
	if ( idx[0] == -1 ) {
	  id = idx[1] ;
	}
	else if ( idx[1] == -1 ) {
	  id = idx[0] ;
	}
	else if ( idx[0] == idx[1] ) {
	  id = idx[0] ;
	}
	else {
	  double t0 = timeVec[idx[0]] ;
	  double t1 = timeVec[idx[1]] ;
	  if ( abs( t0 - reftime ) > abs( t1 - reftime ) ) {
	    id = idx[1] ;
	  }
	  else {
	    id = idx[0] ;
	  }
	}
	return_value = data.GetEntry(id);
      }
      else if ( mode == "linear" ) {
	if ( idx[0] == -1 ) {
	  // use after
	  os << LogIO::WARN << "Failed to interpolate. return a spectrum just after the reftime." << LogIO::POST ;
	  int id = idx[1] ;
	  return_value = data.GetEntry(id);
	}
	else if ( idx[1] == -1 ) {
	  // use before
	  os << LogIO::WARN << "Failed to interpolate. return a spectrum just before the reftime." << LogIO::POST ;
	  int id = idx[0] ;
	  return_value = data.GetEntry(id);
	}
	else if ( idx[0] == idx[1] ) {
	  // use before
	  //os << "No need to interporate." << LogIO::POST ;
	  int id = idx[0] ;
	  return_value = data.GetEntry(id);
	}
	else {
	  // do interpolation
	  double t0 = timeVec[idx[0]] ;
	  double t1 = timeVec[idx[1]] ;
	  Vector<Float> value0 = data.GetEntry(idx[0]);
	  Vector<Float> value1 = data.GetEntry(idx[1]);
	  double tfactor = (reftime - t0) / (t1 - t0) ;

	  /*     // commented out by WK for CAS-6787 (2014/07/31)
	  for ( unsigned int i = 0 ; i < value0.size() ; i++ ) {
	    value1[i] = ( value1[i] - value0[i] ) * tfactor + value0[i] ;
	  }
	  return_value = value1;
	  */
	  // following 4 lines added by WK for CAS-6787 (2014/07/31)
	  return_value.resize(value0.size());
	  for ( unsigned int i = 0 ; i < value0.size() ; i++ ) {
	    return_value[i] = ( value1[i] - value0[i] ) * tfactor + value0[i] ;
	  }

	}
      }
      else {
	os << LogIO::SEVERE << "Unknown mode" << LogIO::POST ;
      }
    }
    return return_value ;
  }
};
  
// Calibration Helper
class CalibrationHelper
{
public:
  static void CalibrateALMA( CountedPtr<Scantable>& out,
			     const CountedPtr<Scantable>& on,
			     const CountedPtr<Scantable>& off,
			     const Vector<uInt>& rows )
  {
    // 2012/05/22 TN
    // Assume that out has empty SPECTRA column

    // if rows is empty, just return
    if ( rows.nelements() == 0 )
      return ;

    ROArrayColumn<Float> in_spectra_column(on->table(), "SPECTRA");
    ROArrayColumn<Float> in_tsys_column(on->table(), "TSYS");
    ROArrayColumn<uChar> in_flagtra_column(on->table(), "FLAGTRA");
    ArrayColumn<Float> out_spectra_column(out->table(), "SPECTRA");
    ArrayColumn<uChar> out_flagtra_column(out->table(), "FLAGTRA");
    
    Vector<Double> timeVec = GetScalarColumn<Double>(off->table(), "TIME");
    Vector<Double> refTimeVec = GetScalarColumn<Double>(on->table(), "TIME");
    SpectralData offspectra(Matrix<Float>(GetArrayColumn<Float>(off->table(), "SPECTRA")));
    unsigned int spsize = on->nchan( on->getIF(rows[0]) ) ;
    vector<int> ids( 2 ) ;
    for ( int irow = 0 ; irow < rows.nelements() ; irow++ ) {
      uInt row = rows[irow];
      double reftime = refTimeVec[row];
      ids = getRowIdFromTime( reftime, timeVec ) ;
      Vector<Float> spoff = SimpleInterpolationHelper<SpectralData>::GetFromTime(reftime, timeVec, ids, offspectra, "linear");
      Vector<Float> spec = in_spectra_column(row);
      Vector<Float> tsys = in_tsys_column(row);
      Vector<uChar> flag = in_flagtra_column(row);
      // ALMA Calibration
      // 
      // Ta* = Tsys * ( ON - OFF ) / OFF
      //
      // 2010/01/07 Takeshi Nakazato
      unsigned int tsyssize = tsys.nelements() ;
      for ( unsigned int j = 0 ; j < spsize ; j++ ) {
	if ( spoff[j] == 0.0 ) {
	  spec[j] = 0.0 ;
	  flag[j] = (uChar)True;
	}
	else {
	  spec[j] = ( spec[j] - spoff[j] ) / spoff[j] ;
	}
	spec[j] *= (tsyssize == spsize) ? tsys[j] : tsys[0];
      }
      out_spectra_column.put(row, spec);
      out_flagtra_column.put(row, flag);
    }
  }
  static void CalibrateChopperWheel( CountedPtr<Scantable> &out,
				     const CountedPtr<Scantable>& on,
				     const CountedPtr<Scantable>& off,
				     const CountedPtr<Scantable>& sky,
				     const CountedPtr<Scantable>& hot,
				     const CountedPtr<Scantable>& cold,
				     const Vector<uInt> &rows )
  {
    // 2012/05/22 TN
    // Assume that out has empty SPECTRA column
    
    // if rows is empty, just return
    if ( rows.nelements() == 0 )
      return ;

    string antenna_name = out->getAntennaName();
    ROArrayColumn<Float> in_spectra_column(on->table(), "SPECTRA");
    ROArrayColumn<uChar> in_flagtra_column(on->table(), "FLAGTRA");
    ArrayColumn<Float> out_spectra_column(out->table(), "SPECTRA");
    ArrayColumn<uChar> out_flagtra_column(out->table(), "FLAGTRA");
    ArrayColumn<Float> out_tsys_column(out->table(), "TSYS");
        
    Vector<Double> timeOff = GetScalarColumn<Double>(off->table(), "TIME");
    Vector<Double> timeSky = GetScalarColumn<Double>(sky->table(), "TIME");
    Vector<Double> timeHot = GetScalarColumn<Double>(hot->table(), "TIME");
    Vector<Double> timeOn = GetScalarColumn<Double>(on->table(), "TIME");
    SpectralData offspectra(Matrix<Float>(GetArrayColumn<Float>(off->table(), "SPECTRA")));
    SpectralData skyspectra(Matrix<Float>(GetArrayColumn<Float>(sky->table(), "SPECTRA")));
    SpectralData hotspectra(Matrix<Float>(GetArrayColumn<Float>(hot->table(), "SPECTRA")));
    TcalData tcaldata(sky);
    TsysData tsysdata(sky);
    unsigned int spsize = on->nchan( on->getIF(rows[0]) ) ;
    vector<int> ids( 2 ) ;
    for ( int irow = 0 ; irow < rows.nelements() ; irow++ ) {
      uInt row = rows[irow];
      double reftime = timeOn[row];
      ids = getRowIdFromTime( reftime, timeOff ) ;
      Vector<Float> spoff = SimpleInterpolationHelper<SpectralData>::GetFromTime(reftime, timeOff, ids, offspectra, "linear");
      ids = getRowIdFromTime( reftime, timeSky ) ; 
      Vector<Float> spsky = SimpleInterpolationHelper<SpectralData>::GetFromTime(reftime, timeSky, ids, skyspectra, "linear");
      Vector<Float> tcal = SimpleInterpolationHelper<TcalData>::GetFromTime(reftime, timeSky, ids, tcaldata, "linear");
      Vector<Float> tsys = SimpleInterpolationHelper<TsysData>::GetFromTime(reftime, timeSky, ids, tsysdata, "linear");
      ids = getRowIdFromTime( reftime, timeHot ) ;
      Vector<Float> sphot = SimpleInterpolationHelper<SpectralData>::GetFromTime(reftime, timeHot, ids, hotspectra, "linear");
      Vector<Float> spec = in_spectra_column(row);
      Vector<uChar> flag = in_flagtra_column(row);
      if ( antenna_name.find( "APEX" ) != String::npos ) {
	// using gain array
	for ( unsigned int j = 0 ; j < tcal.size() ; j++ ) {
	  if ( spoff[j] == 0.0 || (sphot[j]-spsky[j]) == 0.0 ) {
	    spec[j] = 0.0 ;
	    flag[j] = (uChar)True;
	  }
	  else {
	    spec[j] = ( ( spec[j] - spoff[j] ) / spoff[j] )
	      * ( spsky[j] / ( sphot[j] - spsky[j] ) ) * tcal[j] ;
	  }
	}
      }
      else {
	// Chopper-Wheel calibration (Ulich & Haas 1976)
	for ( unsigned int j = 0 ; j < tcal.size() ; j++ ) {
	  if ( (sphot[j]-spsky[j]) == 0.0 ) {
	    spec[j] = 0.0 ;
	    flag[j] = (uChar)True;
	  }
	  else {
	    spec[j] = ( spec[j] - spoff[j] ) / ( sphot[j] - spsky[j] ) * tcal[j] ;
	  }
	}
      }
      out_spectra_column.put(row, spec);
      out_flagtra_column.put(row, flag);
      out_tsys_column.put(row, tsys);
    }
  }
  static void GetSelector(STSelector &sel, const vector<string> &names, const Record &values)
  {
    stringstream ss ;
    ss << "SELECT FROM $1 WHERE ";
    string separator = "";
    for (vector<string>::const_iterator i = names.begin(); i != names.end(); ++i) {
      ss << separator << *i << "==";
      switch (values.dataType(*i)) {
      case TpUInt:
	ss << values.asuInt(*i);
	break;
      case TpInt:
	ss << values.asInt(*i);
	break;
      case TpFloat:
	ss << values.asFloat(*i);
	break;
      case TpDouble:
	ss << values.asDouble(*i);
	break;
      case TpComplex:
	ss << values.asComplex(*i);
	break;
      case TpString:
	ss << values.asString(*i);
	break;
      default:
	break;
      }
      separator = "&&";
    }
    sel.setTaQL(ss.str());
  }
private:
  template<class T>
  static Vector<T> GetScalarColumn(const Table &table, const String &name)
  {
    ROScalarColumn<T> column(table, name);
    return column.getColumn();
  }
  template<class T>
  static Array<T> GetArrayColumn(const Table &table, const String &name)
  {
    ROArrayColumn<T> column(table, name);
    return column.getColumn();
  }
};
  
class AlmaCalibrator
{
public:
  AlmaCalibrator(CountedPtr<Scantable> &out,
		 const CountedPtr<Scantable> &on,
		 const CountedPtr<Scantable> &off)
    : target_(out),
      selector_(),
      on_(on),
      off_(off)
  {}
  ~AlmaCalibrator() {}
  CountedPtr<Scantable> target() const {return target_;}
  void Process(const vector<string> &cols, const Record &values, const Vector<uInt> &rows) {
    CalibrationHelper::GetSelector(selector_, cols, values);
    off_->setSelection(selector_);
    CalibrationHelper::CalibrateALMA(target_, on_, off_, rows);
    off_->unsetSelection();
  }
private:
  CountedPtr<Scantable> target_;
  STSelector selector_;
  const CountedPtr<Scantable> on_;
  const CountedPtr<Scantable> off_;
};

class ChopperWheelCalibrator
{
public:
  ChopperWheelCalibrator(CountedPtr<Scantable> &out,
			 const CountedPtr<Scantable> &on,
			 const CountedPtr<Scantable> &sky,
			 const CountedPtr<Scantable> &off,
			 const CountedPtr<Scantable> &hot,
			 const CountedPtr<Scantable> &cold)
    : target_(out),
      selector_(),
      on_(on),
      off_(off),
      sky_(sky),
      hot_(hot),
      cold_(cold)
  {}
  ~ChopperWheelCalibrator() {}
  CountedPtr<Scantable> target() const {return target_;}
  void Process(const vector<string> &cols, const Record &values, const Vector<uInt> &rows) {
    CalibrationHelper::GetSelector(selector_, cols, values);
    off_->setSelection(selector_);
    sky_->setSelection(selector_);
    hot_->setSelection(selector_);
    CalibrationHelper::CalibrateChopperWheel(target_, on_, off_, sky_, hot_, cold_, rows);
    off_->unsetSelection();
    sky_->unsetSelection();
    hot_->unsetSelection();
  }
private:
  CountedPtr<Scantable> target_;
  STSelector selector_;
  const CountedPtr<Scantable> on_;
  const CountedPtr<Scantable> off_;
  const CountedPtr<Scantable> sky_;
  const CountedPtr<Scantable> hot_;
  const CountedPtr<Scantable> cold_;
};
  
} // anonymous namespace
