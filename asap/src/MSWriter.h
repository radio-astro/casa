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

  // fill empty rows
  void infillSpectralWindow() ;

  // add rows to subtables
  void addFeed( casa::Int id ) ;
  void addSpectralWindow( casa::Int spwid, casa::Int freqid ) ;
  void addField( casa::Int fid, casa::String fieldname, casa::String srcname, casa::Double t, casa::Vector<casa::Double> scanrate ) ;
  void addPointing( casa::String &name, casa::Double &me, casa::Double &interval, casa::Matrix<casa::Double> &dir ) ;
  casa::Int addPolarization( casa::Vector<casa::Int> polnos ) ;
  casa::Int addDataDescription( casa::Int polid, casa::Int spwid ) ;
  casa::Int addState( casa::Int type, casa::Int &subscan ) ;

  // utility
  casa::Vector<casa::Int> toCorrType( casa::Vector<casa::Int> polnos ) ;
  void getValidTimeRange( casa::Double &me, casa::Double &interval, casa::Table &tab ) ;
  void getValidTimeRange( casa::Double &me, casa::Double &interval, casa::Vector<casa::Double> &atime, casa::Vector<casa::Double> &ainterval ) ;
  //void queryType( casa::Int type, casa::String &stype, casa::Bool &b ) ; 
  void queryType( casa::Int type, casa::String &stype, casa::Bool &b, casa::Double &t, Double &l ) ; 
  casa::Double getDishDiameter( casa::String antname ) ;

  // tool for HPC
  double gettimeofday_sec() ;

  casa::CountedPtr<Scantable> table_ ;
  STHeader header_ ;
  //casa::CountedPtr<casa::MeasurementSet> mstable_ ;
  casa::MeasurementSet *mstable_ ;

  casa::Bool isTcal_ ;
  casa::Bool isWeather_ ;

  casa::Bool useFloatData_ ;
  casa::Bool useData_ ;
  casa::Bool tcalSpec_ ;
  casa::Bool tsysSpec_ ;

  casa::String ptTabName_ ;

  casa::String polType_ ;

  casa::String filename_ ;

  casa::LogIO os_ ;

  casa::Record tcalIdRec_ ;
  casa::Record tcalRowRec_ ;
  
  MSWriter();
  MSWriter(const MSWriter&);
  MSWriter& operator=(const MSWriter&);

};


};
#endif
