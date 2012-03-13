

/***
 * Framework independent implementation file for calanalysis...
 *
 * Implement the calanalysis component here.
 * 
 * // TODO: WRITE YOUR DESCRIPTION HERE! 
 *
 * @author
 * @version 
 ***/

#include <calanalysis_cmpt.h>

using namespace std;
using namespace casa;

namespace casac {

// --- ///

calanalysis::calanalysis() {

  // Initialize the CalAnalysis pointer and return

  oCalTable = string();
  poCA = NULL;

  return;

}

// --- ///

calanalysis::~calanalysis() {

  //  If the CalAnalysis instance is valid deallocate it and return

  if ( poCA != NULL ) {
    oCalTable = string();
    delete poCA;
  }

  return;

}

// --- ///

bool calanalysis::open( const std::string& caltable ) {

  // Allocate the CalAnalysis instance and return true

  oCalTable = String( caltable );
  poCA = new CalAnalysis( oCalTable );

  return( true );

}

// --- ///

bool calanalysis::close() {

  // Deallocate the CalAnalysis instance and return true

  oCalTable = string();

  delete poCA;
  poCA = NULL;

  return( true );

}

// --- ///

std::string calanalysis::caltable() {

  // If the CalAnalysis instance is valid return the caltable name otherwise
  // return a null string

  if ( poCA != NULL ) {
    return( oCalTable );
  } else {
    return( string() );
  }

}

// --- ///

std::string calanalysis::coltype() {

  // If the CalAnalysis instance is valid return the PARAM/PARAMERR column type
  // otherwise return a null string

  if ( poCA != NULL ) {
    return( poCA->tableType( oCalTable ) );
  } else {
    return( string() );
  }

}

// --- ///

std::string calanalysis::polbasis() {

  // If the CalAnalysis instance is valid return the polarization basis otherwise
  // return a null string

  if ( poCA != NULL ) {
    return( poCA->polBasis( oCalTable ) );
  } else {
    return( string() );
  }

  return( string() );

}

// --- ///

::casac::record* calanalysis::get( const std::vector<std::string>& field,
    const std::vector<std::string>& antenna1,
    const std::vector<std::string>& antenna2, const double starttime,
    const double stoptime, const std::vector<std::string>& feed,
    const std::vector<std::string>& spw, const std::vector<int>& startchan,
    const std::vector<int>& stopchan, const std::string& axis,
    const std::string& ap ) {

  // Initialize the field parameter

  uInt uiNumField = field.size();
  Vector<Int> oField( uiNumField );
  for ( uInt f=0; f<uiNumField; f++ ) oField[f] = atoi( field[f].c_str() );


  // Initialize the antenna parameters

  uInt uiNumAntenna1 = antenna1.size();
  uInt uiNumAntenna2 = antenna2.size();

  if ( uiNumAntenna1 != uiNumAntenna2 ) {
    throw( AipsError( "Number of antenna1 must be the same as antenna2" ) );
  }

  Vector<Int> oAntenna1( uiNumAntenna1 );
  for ( uInt a=0; a<uiNumAntenna1; a++ ) {
    oAntenna1[a] = atoi( antenna1[a].c_str() );
  }

  Vector<Int> oAntenna2( uiNumAntenna2 );
  for ( uInt a=0; a<uiNumAntenna2; a++ ) {
    oAntenna2[a] = atoi( antenna2[a].c_str() );
  }


  // Initialize the time parameters

  Double dStartTime = starttime;
  Double dStopTime = stoptime;


  // Initialize the feed parameter

  uInt uiNumFeed = feed.size();
  Vector<String> oFeed( uiNumFeed );
  for ( uInt f=0; f<uiNumFeed; f++ ) oFeed[f] = feed[f];


  // Initialize the spectral window and channel parameters

  uInt uiNumSPW = spw.size();
  Vector<Int> oSPW( uiNumSPW );
  for ( uInt s=0; s<uiNumSPW; s++ ) oSPW[s] = atoi( spw[s].c_str() );

  Vector<Int> aoChannel[uiNumSPW];
  for ( uInt s=0; s<uiNumSPW; s++ ) {
    uInt uiNumChannel = stopchan[s] - startchan[s] + 1;
    aoChannel[s].resize( uiNumChannel );
    for ( uInt c=(uInt)startchan[s]; c<=(uInt)stopchan[s]; c++ ) {
      aoChannel[s][c-startchan[s]] = c;
    }
  }


  // Initialize the user-defined iteration axis parameter

  CalStats::AXIS eAxisIterUserID = CalStats::INIT;
  if ( axis == "FREQ" ) {
    eAxisIterUserID = CalStats::FREQUENCY;
  } else if ( axis == "TIME" ) {
    eAxisIterUserID = CalStats::TIME;
  } else {
    throw( AipsError( "Invalid axis ('FREQ' or 'TIME')" ) );
  }


  // Initialize the real, amplitude, phase parameter

  CalAnalysis::RAP eRAP = CalAnalysis::INIT;
  if ( ap == "REAL" ) {
    eRAP = CalAnalysis::REAL;
  } else if ( ap == "AMPLITUDE" ) {
    eRAP = CalAnalysis::AMPLITUDE;
  } else if ( ap == "PHASE" ) {
    eRAP = CalAnalysis::PHASE;
  } else {
    throw( AipsError( "Invalid data type ('REAL', 'AMPLITUDE', or 'PHASE')" ) );
  }


  // Initialize the arguments (NONE)

  CalStats::ARG<CalStats::NONE> oArg;


  // Call the CalAnalysis::stats<T>() member function for getting

  Vector<CalAnalysis::OUTPUT<CalStats::NONE> > oOutput;

  try {
    oOutput = poCA->stats<CalStats::NONE>( oField, oAntenna1, oAntenna2,
        dStartTime, dStopTime, oFeed, oSPW, aoChannel, eAxisIterUserID, eRAP,
        oArg );
  }
  catch( AipsError oAipsError ) {
    throw( oAipsError );
  }

  uInt uiNumOutput = oOutput.nelements();


  // Initialize the iteration number and the output record.  The final iteration
  // number is the number of keys/elements in the output record.

  uInt uiNumIter = 0;
  ::casac::record* poRecOutput = new ::casac::record();


  // Load the output record with the caltable data.  Each iteration corresponds
  // to a single field, antenna1, antenna2, feed, and frequency/time.

  for ( uInt o=0; o<uiNumOutput; o++ ) {

    uInt uiNumRow = oOutput[o].oOut.shape()[0];
    uInt uiNumCol = oOutput[o].oOut.shape()[1];

    for ( uInt r=0; r<uiNumRow; r++ ) {
      for ( uInt c=0; c<uiNumCol; c++ ) {

        ::casac::record oRecIter;

        std::string oField( String::toString(oOutput[o].uiField).c_str() );
        oRecIter.insert( std::string("field"), oField );

        std::string oAntenna1( String::toString(oOutput[o].uiAntenna1).c_str() );
        oRecIter.insert( std::string("antenna1"), oAntenna1 );

        std::string oAntenna2( String::toString(oOutput[o].uiAntenna2).c_str() );
        oRecIter.insert( std::string("antenna2"), oAntenna2 );

        std::string oFeedKey( "feed" );
	std::string oFeedValue( oOutput[o].oOut(r,c).oAxes.sFeed.c_str() );
	oRecIter.insert( oFeedKey, oFeedValue );

        std::string oAxisIterKey;
        CalStats::AXIS eAxisIter = oOutput[o].oOut(r,c).oAxes.eAxisIterUserID;
	if ( eAxisIter == CalStats::FREQUENCY ) {
          oAxisIterKey = std::string( "frequency" );
        } else {
          oAxisIterKey = std::string( "time" );
        }
        double dAxisIterValue = (double)oOutput[o].oOut(r,c).oAxes.dAxisIterUser;
        oRecIter.insert( oAxisIterKey, dAxisIterValue );

        uInt uiNumAbs = oOutput[o].oOut(r,c).oData.oAbs.nelements();

        std::string oAxisNonIterKey;
        CalStats::AXIS eAxisNonIter = oOutput[o].oOut(r,c).oAxes.eAxisNonIterID;
	if ( eAxisNonIter == CalStats::FREQUENCY ) {
          oAxisNonIterKey = std::string( "frequency" );
        } else {
          oAxisNonIterKey = std::string( "time" );
        }
        std::vector<double> oAbs( uiNumAbs );
        for ( uInt a=0; a<uiNumAbs; a++ ) {
	  oAbs[a] = oOutput[o].oOut(r,c).oData.oAbs[a];
	}
        oRecIter.insert( std::string("abscissa"), oAxisNonIterKey );
        oRecIter.insert( oAxisNonIterKey, oAbs );

        std::vector<double> oValue( uiNumAbs );
        for ( uInt a=0; a<uiNumAbs; a++ ) {
	  oValue[a] = oOutput[o].oOut(r,c).oData.oData[a];
	}
        oRecIter.insert( string("value"), oValue );

        std::vector<double> oValueErr( uiNumAbs );
        for ( uInt a=0; a<uiNumAbs; a++ ) {
	  oValueErr[a] = oOutput[o].oOut(r,c).oData.oDataErr[a];
	}
        oRecIter.insert( string("valueErr"), oValueErr );

        std::vector<bool> oFlag( uiNumAbs );
        for ( uInt a=0; a<uiNumAbs; a++ ) {
	  oFlag[a] = oOutput[o].oOut(r,c).oData.oFlag[a];
	}
        oRecIter.insert( string("flag"), oFlag );

        std::string oNumIter( String::toString(uiNumIter).c_str() );
        poRecOutput->insert( oNumIter, oRecIter );

        uiNumIter++;

      }
    }

  }

  return( poRecOutput );

}

// --- ///

::casac::record* calanalysis::fit(const std::vector<std::string>& field,
    const std::vector<std::string>& antenna1,
    const std::vector<std::string>& antenna2, const double starttime,
    const double stoptime, const std::vector<std::string>& feed,
    const std::vector<std::string>& spw, const std::vector<int>& startchan,
    const std::vector<int>& stopchan, const std::string& axis,
    const std::string& order, const std::string& type, const bool weight,
    const std::string& ap ) {

  // Initialize the field parameter

  uInt uiNumField = field.size();
  Vector<Int> oField( uiNumField );
  for ( uInt f=0; f<uiNumField; f++ ) oField[f] = atoi( field[f].c_str() );


  // Initialize the antenna parameters

  uInt uiNumAntenna1 = antenna1.size();
  uInt uiNumAntenna2 = antenna2.size();

  if ( uiNumAntenna1 != uiNumAntenna2 ) {
    throw( AipsError( "Number of antenna1 must be the same as antenna2" ) );
  }

  Vector<Int> oAntenna1( uiNumAntenna1 );
  for ( uInt a=0; a<uiNumAntenna1; a++ ) {
    oAntenna1[a] = atoi( antenna1[a].c_str() );
  }

  Vector<Int> oAntenna2( uiNumAntenna2 );
  for ( uInt a=0; a<uiNumAntenna2; a++ ) {
    oAntenna2[a] = atoi( antenna2[a].c_str() );
  }


  // Initialize the time parameters

  Double dStartTime = starttime;
  Double dStopTime = stoptime;


  // Initialize the feed parameter

  uInt uiNumFeed = feed.size();
  Vector<String> oFeed( uiNumFeed );
  for ( uInt f=0; f<uiNumFeed; f++ ) oFeed[f] = feed[f];


  // Initialize the spectral window and channel parameters

  uInt uiNumSPW = spw.size();
  Vector<Int> oSPW( uiNumSPW );
  for ( uInt s=0; s<uiNumSPW; s++ ) oSPW[s] = atoi( spw[s].c_str() );

  Vector<Int> aoChannel[uiNumSPW];
  for ( uInt s=0; s<uiNumSPW; s++ ) {
    uInt uiNumChannel = stopchan[s] - startchan[s] + 1;
    aoChannel[s].resize( uiNumChannel );
    for ( uInt c=(uInt)startchan[s]; c<=(uInt)stopchan[s]; c++ ) {
      aoChannel[s][c-startchan[s]] = c;
    }
  }


  // Initialize the user-defined iteration axis parameter

  CalStats::AXIS eAxisIterUserID = CalStats::INIT;
  if ( axis == "FREQ" ) {
    eAxisIterUserID = CalStats::FREQUENCY;
  } else if ( axis == "TIME" ) {
    eAxisIterUserID = CalStats::TIME;
  } else {
    throw( AipsError( "Invalid axis ('FREQ' or 'TIME')" ) );
  }


  // Initialize the order, type, and weight parameters for the arguments

  CalStats::ARG<CalStatsFitter::FIT> oArg;

  if ( order == "AVERAGE" ) {
    oArg.eOrder = CalStatsFitter::AVERAGE;
  } else if ( order == "LINEAR" ) {
    oArg.eOrder = CalStatsFitter::LINEAR;
  } else if ( order == "QUADRATIC" ) {
    oArg.eOrder = CalStatsFitter::QUADRATIC;
  } else {
    throw( AipsError( "Invalid order ('AVERAGE', 'LINEAR', or 'QUADRATIC')" ) );
  }

  if ( type == "LSQ" ) {
    oArg.eType = CalStatsFitter::LSQ;
  } else if ( type == "ROBUST" ) {
    oArg.eType = CalStatsFitter::ROBUST;
  } else {
    throw( AipsError( "Invalid type ('LSQ', 'ROBUST')" ) );
  }

  if ( order == "QUADRATIC" && type == "ROBUST" ) {
    throw( AipsError( "Robust quadratic fit unavailable" ) );
  }

  oArg.eWeight = (CalStatsFitter::WEIGHT) weight;


  // Initialize the real, amplitude, phase parameter

  CalAnalysis::RAP eRAP = CalAnalysis::INIT;
  if ( ap == "REAL" ) {
    eRAP = CalAnalysis::REAL;
  } else if ( ap == "AMPLITUDE" ) {
    eRAP = CalAnalysis::AMPLITUDE;
  } else if ( ap == "PHASE" ) {
    eRAP = CalAnalysis::PHASE;
  } else {
    throw( AipsError( "Invalid data type ('REAL', 'AMPLITUDE', or 'PHASE')" ) );
  }


  // Call the CalAnalysis::stats<T>() member function for getting and fitting

  Vector<CalAnalysis::OUTPUT<CalStatsFitter::FIT> > oOutput;

  try {
    oOutput = poCA->stats<CalStatsFitter::FIT>( oField, oAntenna1, oAntenna2,
        dStartTime, dStopTime, oFeed, oSPW, aoChannel, eAxisIterUserID, eRAP,
        oArg );
  }
  catch( AipsError oAipsError ) {
    throw( oAipsError );
  }

  uInt uiNumOutput = oOutput.nelements();


  // Initialize the iteration number and the output record.  The final iteration
  // number is the number of keys/elements in the output record.

  uInt uiNumIter = 0;
  ::casac::record* poRecOutput = new ::casac::record();


  // Load the output record with the caltable data.  Each iteration corresponds
  // to a single field, antenna1, antenna2, feed, and frequency/time.

  for ( uInt o=0; o<uiNumOutput; o++ ) {

    uInt uiNumRow = oOutput[o].oOut.shape()[0];
    uInt uiNumCol = oOutput[o].oOut.shape()[1];

    for ( uInt r=0; r<uiNumRow; r++ ) {
      for ( uInt c=0; c<uiNumCol; c++ ) {

        ::casac::record oRecIter;

        std::string oField( String::toString(oOutput[o].uiField).c_str() );
        oRecIter.insert( std::string("field"), oField );

        std::string oAntenna1( String::toString(oOutput[o].uiAntenna1).c_str() );
        oRecIter.insert( std::string("antenna1"), oAntenna1 );

        std::string oAntenna2( String::toString(oOutput[o].uiAntenna2).c_str() );
        oRecIter.insert( std::string("antenna2"), oAntenna2 );

        std::string oFeedKey( "feed" );
        std::string oFeedValue( oOutput[o].oOut(r,c).oAxes.sFeed.c_str() );
        oRecIter.insert( oFeedKey, oFeedValue );

        std::string oAxisIterKey;
        CalStats::AXIS eAxisIter = oOutput[o].oOut(r,c).oAxes.eAxisIterUserID;
        if ( eAxisIter == CalStats::FREQUENCY ) {
          oAxisIterKey = std::string( "frequency" );
        } else {
          oAxisIterKey = std::string( "time" );
        }
        double dAxisIterValue = (double)oOutput[o].oOut(r,c).oAxes.dAxisIterUser;
        oRecIter.insert( oAxisIterKey, dAxisIterValue );

        uInt uiNumAbs = oOutput[o].oOut(r,c).oData.oAbs.nelements();

        std::string oAxisNonIterKey;
        CalStats::AXIS eAxisNonIter = oOutput[o].oOut(r,c).oAxes.eAxisNonIterID;
        if ( eAxisNonIter == CalStats::FREQUENCY ) {
          oAxisNonIterKey = std::string( "frequency" );
        } else {
          oAxisNonIterKey = std::string( "time" );
        }
        std::vector<double> oAbs( uiNumAbs );
        for ( uInt a=0; a<uiNumAbs; a++ ) {
          oAbs[a] = oOutput[o].oOut(r,c).oData.oAbs[a];
        }
        oRecIter.insert( std::string("abscissa"), oAxisNonIterKey );
        oRecIter.insert( oAxisNonIterKey, oAbs );

        std::vector<double> oValue( uiNumAbs );
        for ( uInt a=0; a<uiNumAbs; a++ ) {
          oValue[a] = oOutput[o].oOut(r,c).oData.oData[a];
        }
        oRecIter.insert( std::string("value"), oValue );

        std::vector<double> oValueErr( uiNumAbs );
        for ( uInt a=0; a<uiNumAbs; a++ ) {
          oValueErr[a] = oOutput[o].oOut(r,c).oData.oDataErr[a];
        }
        oRecIter.insert( std::string("valueErr"), oValueErr );

        std::vector<bool> oFlag( uiNumAbs );
        for ( uInt a=0; a<uiNumAbs; a++ ) {
          oFlag[a] = oOutput[o].oOut(r,c).oData.oFlag[a];
        }
        oRecIter.insert( std::string("flag"), oFlag );

        oRecIter.insert( std::string("order"), order );
        oRecIter.insert( std::string("type"), type );
        oRecIter.insert( std::string("weight"), weight );

        bool valid = oOutput[o].oOut(r,c).oT.bValid;
        oRecIter.insert( std::string("validFit"), valid );

        double redchi2 = oOutput[o].oOut(r,c).oT.dRedChi2;
        oRecIter.insert( std::string("redChi2"), redchi2 );

        uInt uiNumPars = oOutput[o].oOut(r,c).oT.oPars.nelements();
        std::vector<double> oPars( uiNumPars );
        for ( uInt p=0; p<uiNumPars; p++ ) {
          oPars[p] = oOutput[o].oOut(r,c).oT.oPars[p];
        }
        oRecIter.insert( std::string("pars"), oPars );

        std::vector<double> oVars( uiNumPars );

        for ( uInt p=0; p<uiNumPars; p++ ) {
          oVars[p] = oOutput[o].oOut(r,c).oT.oCovars(p,p);
        }
        oRecIter.insert( std::string("vars"), oVars );

        std::vector<double> oCovars( uiNumPars * (uiNumPars-1) / 2 );
        for ( uInt pr=0,p=0; pr<uiNumPars; pr++ ) {
          for ( uInt pc=pr+1; pc<uiNumPars; pc++ ) {
            oCovars[p] = oOutput[o].oOut(r,c).oT.oCovars(pr,pc);
            p++;
          }
        }
        oRecIter.insert( std::string("covars"), oCovars );

        uInt uiNumData = oOutput[o].oOut(r,c).oT.oModel.nelements();

        std::vector<double> oModel( uiNumData );
        for ( uInt d=0; d<uiNumData; d++ ) {
          oModel[d] = oOutput[o].oOut(r,c).oT.oModel[d];
        }
        oRecIter.insert( std::string("model"), oModel );

        std::vector<double> oRes( uiNumData );
        for ( uInt d=0; d<uiNumData; d++ ) {
          oRes[d] = oOutput[o].oOut(r,c).oT.oRes[d];
        }
        oRecIter.insert( std::string("res"), oRes );

        std::string oNumIter( String::toString(uiNumIter).c_str() );
        poRecOutput->insert( oNumIter, oRecIter );

        uiNumIter++;

      }
    }

  }

  return( poRecOutput );

}

} // casac namespace
