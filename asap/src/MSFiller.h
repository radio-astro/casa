//
// C++ Interface: MSFiller
//
// Description:
//
// This class is specific filler for MS format
//
// Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPMSFILLER_H
#define ASAPMSFILLER_H

// STL
#include <string>

// AIPS++
#include <casa/aips.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Logging/LogIO.h>
#include <casa/Containers/RecordField.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/Block.h>
#include <casa/Quanta/MVTime.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSPointing.h>

#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/TableRow.h>

#include <measures/TableMeasures/ScalarMeasColumn.h>
#include <measures/TableMeasures/ArrayMeasColumn.h>
#include <measures/TableMeasures/ScalarQuantColumn.h>
#include <measures/TableMeasures/ArrayQuantColumn.h>

#include "TableTraverse.h"
#include "Scantable.h"
#include "MathUtils.h"

using namespace casa;

namespace asap
{

class MSFillerUtils {
protected:
  template<class T> void getScalar( const String &name, 
                                    const uInt &idx, 
                                    const Table &tab, 
                                    T &val )
  {
    ROScalarColumn<T> col( tab, name ) ;
    val = col( idx ) ;
  }
  template<class T> void getArray( const String &name, 
                                   const uInt &idx, 
                                   const Table &tab, 
                                   Array<T> &val )
  {
    ROArrayColumn<T> col( tab, name ) ;
    val = col( idx ) ;
  }
  template<class T> void getScalarMeas( const String &name, 
                                        const uInt &idx, 
                                        const Table &tab, 
                                        T &val )
  {
    ROScalarMeasColumn<T> measCol( tab, name ) ;
    val = measCol( idx ) ;
  }
  template<class T> void getArrayMeas( const String &name, 
                                       const uInt &idx, 
                                       const Table &tab, 
                                       Array<T> &val )
  {
    ROArrayMeasColumn<T> measCol( tab, name ) ;
    val = measCol( idx ) ;
  }
  template<class T> void getScalarQuant( const String &name, 
                                         const uInt &idx, 
                                         const Table &tab, 
                                         Quantum<T> &val )
  {
    ROScalarQuantColumn<T> quantCol( tab, name ) ;
    val = quantCol( idx ) ;
  }
  template<class T> void getArrayQuant( const String &name, 
                                        const uInt &idx, 
                                        const Table &tab, 
                                        Array< Quantum<T> > &val )
  {
    ROArrayQuantColumn<T> quantCol( tab, name ) ;
    val = quantCol( idx ) ;
  }
  template<class T> T interp( Double x0, Double x1, Double x, T y0, T y1 )
  {
    Double dx0 = x - x0 ;
    Double dx1 = x1 - x ;
    return ( y0 * dx1 + y1 * dx0 ) / ( x1 - x0 ) ;
  }
  String keyTcal( const Int &feedid, const Int &spwid, const Double &time )
  {
    String stime = MVTime( Quantity(time,Unit("s")) ).string( MVTime::YMD ) ;
    String sfeed = "FEED" + String::toString( feedid ) ;
    String sspw = "SPW" + String::toString( spwid ) ;
    return sfeed+":"+sspw+":"+stime ;
  }
  String keyTcal( const Int &feedid, const Int &spwid, const String &stime )
  {
    String sfeed = "FEED" + String::toString( feedid ) ;
    String sspw = "SPW" + String::toString( spwid ) ;
    return sfeed+":"+sspw+":"+stime ;
  }
};

class MSFiller : public MSFillerUtils
{
public:
  explicit MSFiller(CountedPtr<Scantable> stable) ;
  virtual ~MSFiller() ;
  
  virtual bool open(const std::string& filename, const Record& rec) ;
  virtual void fill() ;
  virtual void close() ;
  
protected:
  
  
private:
  
  MSFiller();
  MSFiller(const MSFiller&);
  MSFiller& operator=(const MSFiller&);

  // fill subtables
  //void fillFrequencies() ;
  //void fillMolecules() ;
  void fillWeather() ;
  void fillFocus() ;
  //void fillHistory() ;
  //void fillFit() ;
  void fillTcal() ;
  void infillTcal() ;

  // get frequency frame
  std::string getFrame() ;

  // initialize header
  void initHeader( STHeader &header ) ;

  // get base frame from SPECTRAL_WINDOW table
  std::string frameFromSpwTable();

  CountedPtr<Scantable> table_ ;
  MeasurementSet mstable_ ;
  String tablename_ ;
  Int antenna_ ;
  String antennaStr_ ;
  Bool getPt_ ;
  Bool freqToLsr_ ;

  Bool isFloatData_ ;
  Bool isData_ ;

  Bool isDoppler_ ;
  Bool isFlagCmd_ ;
  Bool isFreqOffset_ ;
  Bool isHistory_ ;
  Bool isProcessor_ ;
  Bool isSysCal_ ;
  Bool isWeather_ ;

  String colTsys_ ;
  String colTcal_ ;

  LogIO os_ ;
  
  Vector<Double> mwTime_ ;
  Vector<Double> mwInterval_ ;
  Vector<uInt> mwIndex_ ;

  // Record for TCAL_ID
  // "FIELD0": "SPW0": Vector<uInt>
  //           "SPW1": Vector<uInt>
  //  ...
  Record tcalrec_ ;
  //map< String,Vector<uInt> > tcalrec_ ;
};


};
#endif
