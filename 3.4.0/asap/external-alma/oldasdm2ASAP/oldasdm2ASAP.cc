#include <iostream>
#include <fstream>
#include <casa/Utilities/Regex.h>
#include <casa/Inputs/Input.h>
#include <casa/BasicSL/String.h>
#include <casa/Containers/Record.h>
#include <casa/OS/Directory.h>
#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/StreamLogSink.h>
#include <casa/Logging/LogFilter.h>
#include "../../src/Scantable.h"
#include "OldASDMFiller.h"

using namespace std ;
using namespace asdm ;
using namespace casa ;
using namespace asap ;

int main( int argc, char *argv[] )
{
  // options
  Input inp ;
  String indent = "   " ;
  String versionInfo = "$Id:$\nConverts an ASDM dataset into Scantable.\nUsage:\n"+indent+argv[0]+" -antenna <antenna name or id> -asdm <ASDM directory> -asap <Scantable name> [-apc both|yes|no] [-corr-mode ca|ao|ca,ao] [-ocorr-mode ao] [-time-sampling all|integration|subintegration] [-srt fr|bw|ca|fr,bw|fr,ca|ca,bw|all]" ;
  Bool helpMode = False ;
  for ( int i = 1 ; i < argc ; i++ ) {
    if ( strncmp( argv[i], "-h", 2 ) == 0 
         || strncmp( argv[i], "--help", 6 ) == 0 
         || strncmp( argv[i], "-v", 2 ) == 0 
         || strncmp( argv[i], "--version", 9 ) == 0 ) {
      helpMode = True ;
      break ;
    }
  }
  if ( helpMode )
    inp.version( versionInfo ) ;
  else
    inp.version( "" ) ;

  inp.create( "antenna", "0", "antenna name or id", "String" ) ;
  inp.create( "asdm", "", "ASDM directory name", "String" ) ;
  inp.create( "asap", "", "Scantable name", "String" ) ;
  inp.create( "apc", "both", "Retrieve Atm Phase Corrected data or not: both|yes|no", "String" ) ;
  inp.create( "overwrite", "True", "Overwrite existing Scantable or not: True|False", "Bool" ) ;
  inp.create( "corr-mode", "ca,ao", "Input correlator mode: ca+ao|ca|ao", "String" ) ;
  inp.create( "ocorr-mode", "ao", "Output correlator mode: ao", "String" ) ;
  inp.create( "time-sampling", "all", "time sampling mode: all|integration|subintegration", "String" ) ;
  inp.create( "srt", "all", "spectral resolution mode: all|fr(full resolution)|ca(channel average)|bw(baseband wide)|fr+ca|fr+bw|ca+bw", "String" ) ;
  inp.create( "logfile", "", "logger output", "String" ) ;
  inp.readArguments( argc, argv ) ;

  string asdmname = inp.getString( "asdm" ) ;
  string antenna = inp.getString( "antenna" ) ;
  string asapname = inp.getString( "asap" ) ;
  string apc = inp.getString( "apc" ) ;
  Bool overwrite = inp.getBool( "overwrite" ) ;
  string corrMode = inp.getString( "corr-mode" ) ;
  string timeSampling = inp.getString( "time-sampling" ) ;
  string resolutionType = inp.getString( "srt" ) ;
  string logfile = inp.getString( "logfile" ) ;
    
  int numApc = 1 ;
  Vector<Bool> apcCorrected ;
  apcCorrected.resize( numApc ) ;
  if ( apc == "both" ) {
    numApc = 2 ;
    apcCorrected.resize( numApc ) ;
    apcCorrected[0] = True ;
    apcCorrected[1] = False ;
  }
  else if ( apc == "yes" ) {
    apcCorrected.resize( numApc ) ;
    apcCorrected[0] = True ;
  }
  else if ( apc == "no" ) {
    apcCorrected.resize( numApc ) ;
    apcCorrected[0] = False ;
  }
  else {
    throw AipsError( "Unrecognized value for -apc option" ) ;
  }
    

  ofstream ofs ;
  CountedPtr<LogSinkInterface> logsink_p ;
  String funcname( argv[0] ) ;
  if ( logfile.size() != 0 ) {
    ofs.open( logfile.c_str(), ios_base::app ) ;
    logsink_p = new StreamLogSink( &ofs ) ;
    logsink_p->cerrToo( false ) ;
  } 
  else {
    logsink_p = new StreamLogSink() ;
  }
  // create OldASDMFiller object
  //logsink_p->postLocally( LogMessage( "numApc = "+String::toString(numApc), LogOrigin(funcname,WHERE) ) ) ;
  for ( int iapc = 0 ; iapc < numApc ; iapc++ ) {
    CountedPtr<Scantable> stable( new Scantable() ) ;
    OldASDMFiller *filler = new OldASDMFiller( stable ) ;

    // set logger
    filler->setLogger( logsink_p ) ;

    // open data
    Record rec ;
    Record asdmRec ;
    Regex reg( "[0-9]+$" ) ;
    //asdmRec.define( "apc", apcCorrected ) ;
    asdmRec.define( "apc", apcCorrected[iapc] ) ;
    asdmRec.define( "corr", corrMode ) ;
    asdmRec.define( "sampling", timeSampling ) ;
    asdmRec.define( "srt", resolutionType ) ;
    if ( reg.match( antenna.c_str(), antenna.size() ) != String::npos ) {
      // antenna is specifiec as id
      int aid = atoi( antenna.c_str() ) ;
      asdmRec.define( "antenna", aid ) ;
    }
    else {
      // antenna is specified as name
      asdmRec.define( "antenna", antenna ) ;
    }
    rec.defineRecord( "asdm", asdmRec ) ;
    filler->open( asdmname, rec ) ;
    
    // output filename
    CountedPtr<OldASDMReader> reader = filler->getReader() ;
    string aname = reader->getAntennaName() ;
    string outname = asapname ;
    if ( asapname.size() == 0 ) {
      outname = asdmname + "." + aname + ".asap" ;
    }
    if ( apcCorrected[iapc] == True ) {
      outname += ".wvr-corrected" ;
    }
    
    //logsink_p->postLocally( LogMessage("specified option summary:",LogOrigin(funcname,WHERE)) ) ;
    //logsink_p->postLocally( LogMessage("   antenna = "+String(aname)+" (ID: "+String::toString(reader->getAntennaId())+")",LogOrigin(funcname,WHERE)) ) ;
    //logsink_p->postLocally( LogMessage("   asdmname = "+asdmname,LogOrigin(funcname,WHERE)) ) ;
    //logsink_p->postLocally( LogMessage("   asapname = "+outname,LogOrigin(funcname,WHERE)) ) ;
    //logsink_p->postLocally( LogMessage("   apcCorrected = "+String::toString(apcCorrected[iapc]),LogOrigin(funcname,WHERE) ) ) ;
    //logsink_p->postLocally( LogMessage("   timeSampling = "+timeSampling,LogOrigin(funcname,WHERE) ) ) ;
    //logsink_p->postLocally( LogMessage("   corrMode = "+corrMode,LogOrigin(funcname,WHERE) ) ) ;
    
    // save scantable on disk
    Directory dir( outname ) ;
    if ( dir.exists() ) {
      if ( overwrite ) {
        logsink_p->postLocally( LogMessage("Delete existing file "+outname+" ...",LogOrigin(funcname,WHERE)) ) ;
        dir.removeRecursive() ;
      }
      else {
        logsink_p->postLocally( LogMessage("Output file "+outname+" exists.",LogOrigin(funcname,WHERE),LogMessage::WARN) ) ;
        return 1 ;
      }
    }
    
    // fill data
    filler->fill() ;
    
    // close data
    filler->close() ;
    
    // save data only if nrow is not zero
    if ( stable->nrow() > 0 ) {
      logsink_p->postLocally( LogMessage("Creating "+outname+"...",LogOrigin(funcname,WHERE)) ) ;
      stable->makePersistent( outname ) ;
    }
    else {
      logsink_p->postLocally( LogMessage(outname+" will not be created since there are no data associate with the selection",LogOrigin(funcname,WHERE)) ) ;
    }
    
    // finalize
    reader = 0 ;
    delete filler ;
   
  }
 
  if ( logfile.size() != 0 )
    ofs.close() ;

  return 0 ;
}
