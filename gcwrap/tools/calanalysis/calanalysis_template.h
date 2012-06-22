
// --- //

template <typename T>
Bool calanalysis::writeInput( const CalAnalysis::OUTPUT<T>& oOutput,
    const uInt& row, const uInt& col, ::casac::record& oRecIter ) {

  // Write the field

  unsigned int uiField = oOutput.uiField;
  std::string oFieldString( String::toString(uiField).c_str() );
  oRecIter.insert( std::string("field"), oFieldString );


  // Write the antenna 1

  unsigned int uiAntenna1 = oOutput.uiAntenna1;
  std::string oAntenna1String( String::toString(uiAntenna1).c_str() );
  oRecIter.insert( std::string("antenna1"), oAntenna1String );


  // Write the antenna 2

  int iAntenna2 = oOutput.iAntenna2;
  std::string oAntenna2String( String::toString(iAntenna2).c_str() );
  oRecIter.insert( std::string("antenna2"), oAntenna2String );


  // Write the feed

  std::string oFeedKey( "feed" );
  std::string oFeedValue( oOutput.oOut(row,col).oAxes.sFeed.c_str() );

  oRecIter.insert( oFeedKey, oFeedValue );


  // Write the user-defined iteration axis

  std::string oAxisIterKey;
  CalStats::AXIS eAxis = oOutput.oOut(row,col).oAxes.eAxisIterUserID;

  if ( eAxis == CalStats::FREQUENCY ) {
    oAxisIterKey = std::string( "frequency" );
  } else {
    oAxisIterKey = std::string( "time" );
  }

  double dAxisIterValue = oOutput.oOut(row,col).oAxes.dAxisIterUser;
  oRecIter.insert( oAxisIterKey, dAxisIterValue );


  // Write the RAP parameter

  std::string oRAPString;

  if ( oOutput.eRAP == CalAnalysis::REAL ) {
    oRAPString = std::string( "REAL" );
  } else if ( oOutput.eRAP == CalAnalysis::AMPLITUDE ) {
    oRAPString = std::string( "AMPLITUDE" );
  } else {
    oRAPString = std::string( "PHASE" );
  }

  oRecIter.insert( std::string("rap"), oRAPString );


  // Write the amplitude normalization boolean

  if ( oOutput.eRAP == CalAnalysis::AMPLITUDE ) {
    oRecIter.insert( std::string("norm"), (bool) oOutput.bNorm );
  }


  // Write the phase unwrapping boolean and the maximum phase jump parameter

  if ( oOutput.eRAP == CalAnalysis::PHASE ) {
    oRecIter.insert( std::string("unwrap"), (bool) oOutput.bUnwrap );
    oRecIter.insert( std::string("jumpMax"), (double) oOutput.dJumpMax );
  }


  // Return True

  return( True );

}

// --- //

template <typename T>
Bool calanalysis::writeData( const CalAnalysis::OUTPUT<T>& oOutput,
    const uInt& row, const uInt& col, ::casac::record& oRecIter ) {

  // Write the non-iteration axis

  std::string oAxisNonIterKey;
  CalStats::AXIS eAxisNon = oOutput.oOut(row,col).oAxes.eAxisNonIterID;

  if ( eAxisNon == CalStats::FREQUENCY ) {
    oAxisNonIterKey = std::string( "frequency" );
  } else {
    oAxisNonIterKey = std::string( "time" );
  }

  oRecIter.insert( std::string("abscissa"), oAxisNonIterKey );


  // Get the number of abscissae

  uInt uiNumAbs = oOutput.oOut(row,col).oData.oAbs.nelements();


  // Write the abscissae from the non-iteration axis (either times or
  // frequencies)

  std::vector<double> oAbs( uiNumAbs );

  for ( uInt a=0; a<uiNumAbs; a++ ) {
    oAbs[a] = oOutput.oOut(row,col).oData.oAbs[a];
  }
  
  oRecIter.insert( oAxisNonIterKey, oAbs );


  // Write the values (either reals, amplitudes, or phases)

  std::vector<double> oValue( uiNumAbs );

  for ( uInt a=0; a<uiNumAbs; a++ ) {
    oValue[a] = oOutput.oOut(row,col).oData.oValue[a];
  }

  oRecIter.insert( string("value"), oValue );


  // Write the value errors (either real, amplitude, or phase errors)

  std::vector<double> oValueErr( uiNumAbs );

  for ( uInt a=0; a<uiNumAbs; a++ ) {
    oValueErr[a] = oOutput.oOut(row,col).oData.oValueErr[a];
  }

  oRecIter.insert( string("valueErr"), oValueErr );


  // Write the flags

  std::vector<bool> oFlag( uiNumAbs );

  for ( uInt a=0; a<uiNumAbs; a++ ) {
    oFlag[a] = oOutput.oOut(row,col).oData.oFlag[a];
  }

  oRecIter.insert( string("flag"), oFlag );


  // Return True

  return( True );

}

// --- //

template <typename T>
Bool calanalysis::writeFit( const CalStats::ARG<T>& oArg,
    const CalAnalysis::OUTPUT<T>& oOutput, const uInt& row, const uInt& col,
    ::casac::record& oRecIter ) {

  // Write the fit order, type, and weight parameters

  oRecIter.insert( std::string("order"),
      CalStatsFitter::orderName(oArg.eOrder) );

  oRecIter.insert( std::string("type"), CalStatsFitter::typeName(oArg.eType) );

  oRecIter.insert( std::string("weight"),
      CalStatsFitter::weightName(oArg.eWeight) );


  // Write the fit validity flag

  bool valid = oOutput.oOut(row,col).oT.bValid;

  oRecIter.insert( std::string("validFit"), valid );


  // Write the fit reduced chi2 value

  double redchi2 = oOutput.oOut(row,col).oT.dRedChi2;

  oRecIter.insert( std::string("redChi2"), redchi2 );


  // Write the fit parameters

  uInt uiNumPars = oOutput.oOut(row,col).oT.oPars.nelements();
  std::vector<double> oPars( uiNumPars );

  for ( uInt p=0; p<uiNumPars; p++ ) {
    oPars[p] = oOutput.oOut(row,col).oT.oPars[p];
  }

  oRecIter.insert( std::string("pars"), oPars );


  // Write the fit variances

  std::vector<double> oVars( uiNumPars );

  for ( uInt p=0; p<uiNumPars; p++ ) {
    oVars[p] = oOutput.oOut(row,col).oT.oCovars(p,p);
  }

  oRecIter.insert( std::string("vars"), oVars );


  // Write the fit covariances

  std::vector<double> oCovars( uiNumPars * (uiNumPars-1) / 2 );

  for ( uInt pr=0,p=0; pr<uiNumPars; pr++ ) {
    for ( uInt pc=pr+1; pc<uiNumPars; pc++,p++ ) {
      oCovars[p] = oOutput.oOut(row,col).oT.oCovars(pr,pc);
    }
  }

  oRecIter.insert( std::string("covars"), oCovars );


  // Write the fit model

  uInt uiNumData = oOutput.oOut(row,col).oT.oModel.nelements();
  std::vector<double> oModel( uiNumData );

  for ( uInt d=0; d<uiNumData; d++ ) {
    oModel[d] = oOutput.oOut(row,col).oT.oModel[d];
  }

  oRecIter.insert( std::string("model"), oModel );


  // Write the fit residuals

  std::vector<double> oRes( uiNumData );

  for ( uInt d=0; d<uiNumData; d++ ) {
    oRes[d] = oOutput.oOut(row,col).oT.oRes[d];
  }

  oRecIter.insert( std::string("res"), oRes );


  // Write the fit residual variance

  Double dResVar = oOutput.oOut(row,col).oT.dResVar;

  oRecIter.insert( std::string("resVar"), dResVar );


  // Write the fit residual mean

  Double dResMean = oOutput.oOut(row,col).oT.dResMean;

  oRecIter.insert( std::string("resMean"), dResMean );


  // Return True

  return( True );

}
