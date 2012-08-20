//
// C++ Interface: MSWriter
//
// Description:
//
// This class is specific writer for MS format
//
// Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ASAPMSWRITER_H
#define ASAPMSWRITER_H

// STL
#include <string>
// AIPS++
#include <casa/aips.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Logging/LogIO.h>
#include <casa/Containers/Record.h>
#include <casa/Containers/RecordField.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/RefRows.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>

#include <measures/Measures/MEpoch.h>

#include <atnf/PKSIO/SrcType.h>

#include "Scantable.h"
#include "STHeader.h"

namespace asap
{
class MSWriterUtils
{
protected:
  template<class T> void putField( const String &name, 
                                   TableRecord &r, 
                                   T &val )
  {
    RecordFieldPtr<T> rf( r, name ) ;
    *rf = val ;
  }
  template<class T> void defineField( const String &name, 
                                      TableRecord &r, 
                                      T &val )
  {
    RecordFieldPtr<T> rf( r, name ) ;
    rf.define( val ) ;
  }
};

class MSWriter
{
public:
  explicit MSWriter(casa::CountedPtr<Scantable> stable) ;
  virtual ~MSWriter() ;
  
  virtual bool write(const std::string& filename, const casa::Record& rec) ;
  
protected:
  
  
private:

  // initialize writer from input Scantable
  void init() ;

  // set up MS
  void setupMS() ;
  
  // fill subtables
  void fillObservation() ;
  void fillAntenna() ;
  void fillProcessor() ;
  void fillSource() ;
  void fillWeather() ;
  void fillSysCal() ;

  // utility
  void getValidTimeRange( casa::Double &me, casa::Double &interval, casa::Table &tab ) ;
  void getValidTimeRange( casa::Double &me, casa::Double &interval, casa::Vector<casa::Double> &atime, casa::Vector<casa::Double> &ainterval ) ;
  void antennaProperty( casa::String &name, casa::String &mount, casa::String &type, casa::Double &diameter ) ;

  casa::CountedPtr<Scantable> table_ ;
  STHeader header_ ;
  casa::MeasurementSet *mstable_ ;

  casa::Bool isWeather_ ;

  casa::Bool useFloatData_ ;
  casa::Bool useData_ ;
  casa::Bool tcalSpec_ ;
  casa::Bool tsysSpec_ ;

  casa::String ptTabName_ ;

  casa::String polType_ ;

  casa::String filename_ ;

  casa::LogIO os_ ;

  casa::Record srcRec_ ;
  
  MSWriter();
  MSWriter(const MSWriter&);
  MSWriter& operator=(const MSWriter&);

};


};
#endif
