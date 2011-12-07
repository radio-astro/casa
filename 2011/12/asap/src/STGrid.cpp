#include <iostream>
#include <fstream>

#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Cube.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayPartMath.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Logging/LogIO.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>

#include <measures/Measures/MDirection.h>

#include <Scantable.h>
#include "STGrid.h"

using namespace std ;
using namespace casa ;
using namespace asap ;

namespace asap {

// constructor
STGrid::STGrid()
{
  init() ;
}

STGrid::STGrid( const string infile )
{
  init() ;

  setFileIn( infile ) ;
}

void  STGrid::init() 
{
  ifno_ = -1 ;
  nx_ = -1 ;
  ny_ = -1 ;
  npol_ = 0 ;
  nchan_ = 0 ;
  nrow_ = 0 ;
  cellx_ = 0.0 ;
  celly_ = 0.0 ;
  center_ = Vector<Double> ( 2, 0.0 ) ;
  convType_ = "BOX" ;
  wtype_ = "UNIFORM" ;
  convSupport_ = -1 ;
  userSupport_ = -1 ;
  convSampling_ = 100 ;
}

void STGrid::setFileIn( const string infile )
{
  String name( infile ) ;
  if ( infile_.compare( name ) != 0 ) {
    infile_ = String( infile ) ;
    tab_ = Table( infile_ ) ;
  }
}

void STGrid::setPolList( vector<unsigned int> pols )
{
  //pollist_ = Vector<uInt>( pols ) ;
  pollist_.assign( Vector<uInt>( pols ) ) ;
  cout << "pollist_ = " << pollist_ << endl ;
}

void STGrid::setWeight( const string wType )
{
  wtype_ = String( wType ) ;
  wtype_.upcase() ;
  cout << "wtype_ = " << wtype_ << endl ; 
}

void STGrid::defineImage( int nx,
                          int ny,
                          string scellx,
                          string scelly,
                          string scenter ) 
{
  ROArrayColumn<Double> dirCol( tab_, "DIRECTION" ) ;
  Matrix<Double> direction = dirCol.getColumn() ;
  Double rmax, rmin, dmax, dmin ;
  minMax( rmin, rmax, direction.row( 0 ) ) ;
  minMax( dmin, dmax, direction.row( 1 ) ) ;

  Int npx = (Int)nx ;
  Int npy = (Int)ny ;
  String cellx( scellx ) ;
  String celly( scelly ) ;
  String center( scenter ) ;
  setupGrid( npx, npy, 
             cellx, celly, 
             rmin, rmax, 
             dmin, dmax, 
             center ) ;
}
  
void STGrid::setOption( string convType,
                        int convSupport ) 
{
  convType_ = String( convType ) ;
  convType_.upcase() ;
  userSupport_ = (Int)convSupport ;
}

#define NEED_UNDERSCORES
#if defined(NEED_UNDERSCORES)
#define ggridsd ggridsd_
#endif
extern "C" { 
   void ggridsd(Double*,
		const Complex*,
                Int*,
                Int*,
                Int*,
		const Int*,
		const Int*,
		const Float*,
		Int*,
		Int*,
		Complex*,
		Float*,
                Int*,
		Int*,
		Int *,
		Int *,
                Int*,
		Int*,
		Float*,
		Int*,
		Int*,
		Double*);
}
void STGrid::grid() 
{
  LogIO os( LogOrigin("STGrid", "grid", WHERE) ) ;

  // retrieve data
  Cube<Float> spectra ;
  Matrix<Double> direction ;
  Cube<uChar> flagtra ;
  Matrix<uInt> rflag ;
  Matrix<Float> weight ;
  getData( spectra, direction, flagtra, rflag, weight ) ;
  IPosition sshape = spectra.shape() ;
  //os << "spectra.shape()=" << spectra.shape() << LogIO::POST ;
  //os << "max(spectra) = " << max(spectra) << LogIO::POST ;
  //os << "weight = " << weight << LogIO::POST ;

  // flagtra: uChar -> Int
  // rflag: uInt -> Int
  Cube<Int> flagI ;
  Matrix<Int> rflagI ;
  toInt( &flagtra, &flagI ) ;
  toInt( &rflag, &rflagI ) ;
  
  // grid parameter
  os << LogIO::DEBUGGING ;
  os << "----------" << endl ;
  os << "Grid parameter summary" << endl ;
  os << "   (nx,ny) = (" << nx_ << "," << ny_ << ")" << endl ;
  os << "   (cellx,celly) = (" << cellx_ << "," << celly_ << ")" << endl ;
  os << "   center = " << center_ << endl ;
  os << "----------" << LogIO::POST ;
  os << LogIO::NORMAL ;

  // convolution kernel
  Vector<Float> convFunc ;
  setConvFunc( convFunc ) ;
  //cout << "convSupport=" << convSupport_ << endl ;
  //cout << "convFunc=" << convFunc << endl ;

  // world -> pixel
  Matrix<Double> xypos( direction.shape(), 0.0 ) ;
  toPixel( direction, xypos ) ;  
  
  // call ggridsd
  Bool deletePos, deleteData, deleteWgt, deleteFlag, deleteFlagR, deleteConv, deleteDataG, deleteWgtG ;
  Double *xypos_p = xypos.getStorage( deletePos ) ;
  Cube<Complex> dataC( spectra.shape(), 0.0 ) ;
  setReal( dataC, spectra ) ;
  const Complex *data_p = dataC.getStorage( deleteData ) ;
  const Float *wgt_p = weight.getStorage( deleteWgt ) ;
  const Int *flag_p = flagI.getStorage( deleteFlag ) ;
  const Int *rflag_p = rflagI.getStorage( deleteFlagR ) ;
  Float *conv_p = convFunc.getStorage( deleteConv ) ;
  // Extend grid plane with convSupport_
  //IPosition gshape( 4, nx_, ny_, npol, nchan ) ;
  Int gnx = nx_+convSupport_*2 ;
  Int gny = ny_+convSupport_*2 ;
  IPosition gshape( 4, gnx, gny, npol_, nchan_ ) ;
  Array<Complex> gdataArrC( gshape, 0.0 ) ;
  Array<Float> gwgtArr( gshape, 0.0 ) ;
  Complex *gdata_p = gdataArrC.getStorage( deleteDataG ) ;
  Float *wdata_p = gwgtArr.getStorage( deleteWgtG ) ;
  Int idopsf = 0 ;
  Int irow = -1 ;
  Int *chanMap = new Int[nchan_] ;
  {
    Int *work_p = chanMap ;
    for ( Int i = 0 ; i < nchan_ ; i++ ) {
      *work_p = i ;
      work_p++ ;
    }
  }
  Int *polMap = new Int[npol_] ;
  {
    Int *work_p = polMap ;
    for ( Int i = 0 ; i < npol_ ; i++ ) {
      *work_p = i ;
      work_p++ ;
    }
  }
  Double *sumw_p = new Double[npol_*nchan_] ;
  {
    Double *work_p = sumw_p ;
    for ( Int i = 0 ; i < npol_*nchan_ ; i++ ) {
      *work_p = 0.0 ;
      work_p++ ;
    }
  }
  ggridsd( xypos_p,
           data_p,
           &npol_,
           &nchan_,
           &idopsf,
           flag_p,
           rflag_p,
           wgt_p,
           &nrow_,
           &irow,
           gdata_p,
           wdata_p, 
           &gnx,
           &gny,
           &npol_,
           &nchan_,
           &convSupport_,
           &convSampling_,
           conv_p,
           chanMap,
           polMap,
           sumw_p ) ;
  xypos.putStorage( xypos_p, deletePos ) ;
  dataC.freeStorage( data_p, deleteData ) ;
  weight.freeStorage( wgt_p, deleteWgt ) ;
  flagI.freeStorage( flag_p, deleteFlag ) ;
  rflagI.freeStorage( rflag_p, deleteFlagR ) ;
  convFunc.putStorage( conv_p, deleteConv ) ;
  delete polMap ;
  delete chanMap ;
  gdataArrC.putStorage( gdata_p, deleteDataG ) ;
  gwgtArr.putStorage( wdata_p, deleteWgtG ) ;
  Array<Float> gdataArr = real( gdataArrC ) ;
  data_.resize( gdataArr.shape() ) ;
  data_ = 0.0 ;
  for ( Int ix = 0 ; ix < nx_ ; ix++ ) {
    for ( Int iy = 0 ; iy < ny_ ; iy++ ) {
      for ( Int ip = 0 ; ip < npol_ ; ip++ ) {
        for ( Int ic = 0 ; ic < nchan_ ; ic++ ) {
          IPosition pos( 4, ix, iy, ip, ic ) ;
          IPosition gpos( 4, ix+convSupport_, iy+convSupport_, ip, ic ) ;
          if ( gwgtArr( gpos ) > 0.0 ) 
            data_( pos ) = gdataArr( gpos ) / gwgtArr( gpos ) ;
        }
      }
    }
  }
  //Matrix<Double> sumWeight( IPosition( 2, npol_, nchan_ ), sumw_p, TAKE_OVER ) ;
  delete sumw_p ;
  //cout << "sumWeight = " << sumWeight << endl ;
  //cout << "gdataArr = " << gdataArr << endl ;
  //cout << "gwgtArr = " << gwgtArr << endl ;
  //cout << "data_ " << data_ << endl ;
}

void STGrid::setupGrid( Int &nx, 
                        Int &ny, 
                        String &cellx, 
                        String &celly, 
                        Double &xmin,
                        Double &xmax,
                        Double &ymin,
                        Double &ymax,
                        String &center )
{
  //cout << "nx=" << nx << ", ny=" << ny << endl ;

  // center position
  if ( center.size() == 0 ) {
    center_(0) = 0.5 * ( xmin + xmax ) ;
    center_(1) = 0.5 * ( ymin + ymax ) ;
  }
  else {
    String::size_type pos0 = center.find( " " ) ;
    if ( pos0 == String::npos ) {
      throw AipsError( "bad string format in parameter center" ) ;
    }
    String::size_type pos1 = center.find( " ", pos0+1 ) ;
    String typestr, xstr, ystr ;
    if ( pos1 != String::npos ) {
      typestr = center.substr( 0, pos0 ) ;
      xstr = center.substr( pos0+1, pos1-pos0 ) ;
      ystr = center.substr( pos1+1 ) ;
      // todo: convert to J2000 (or direction ref for DIRECTION column)
    }
    else {
      typestr = "J2000" ;
      xstr = center.substr( 0, pos0 ) ;
      ystr = center.substr( pos0+1 ) ;
    }
    QuantumHolder qh ;
    String err ;
    qh.fromString( err, xstr ) ;
    Quantum<Double> xcen = qh.asQuantumDouble() ;
    qh.fromString( err, ystr ) ;
    Quantum<Double> ycen = qh.asQuantumDouble() ;
    center_(0) = xcen.getValue( "rad" ) ;
    center_(1) = ycen.getValue( "rad" ) ;
  }


  //Double wx = xmax - xmin ;
  //Double wy = ymax - ymin ;
  Double wx = max( abs(xmax-center_(0)), abs(xmin-center_(0)) ) * 2 ;
  Double wy = max( abs(ymax-center_(1)), abs(ymin-center_(1)) ) * 2 ;
  // take 10% margin
  wx *= 1.10 ;
  wy *= 1.10 ;
  Quantum<Double> qcellx ;
  Quantum<Double> qcelly ;
  nx_ = nx ;
  ny_ = ny ;
  if ( nx < 0 && ny > 0 ) {
    nx_ = ny ;
    ny_ = ny ;
  }
  if ( ny < 0 && nx > 0 ) {
    nx_ = nx ;
    ny_ = nx ;
  }
  //cout << "nx_ = " << nx_ << ",  ny_ = " << ny_ << endl ;
  if ( cellx.size() != 0 && celly.size() != 0 ) {
    readQuantity( qcellx, cellx ) ;
    readQuantity( qcelly, celly ) ;
  }
  else if ( celly.size() != 0 ) {
    cout << "Using celly to x-axis..." << endl ;
    readQuantity( qcelly, celly ) ;
    qcellx = qcelly ;
  }
  else if ( cellx.size() != 0 ) {
    cout << "Using cellx to y-axis..." << endl ;
    readQuantity( qcellx, cellx ) ;
    qcelly = qcellx ;
  }
  else {
    if ( nx_ < 0 ) {
      cout << "No user preference in grid setting. Using default..." << endl ;
      readQuantity( qcellx, "1.0arcmin" ) ;
      qcelly = qcellx ;
    }
    else {
      qcellx = Quantum<Double>( wx/nx_, "rad" ) ;
      qcelly = Quantum<Double>( wy/ny_, "rad" ) ;
    }
  }
  cellx_ = qcellx.getValue( "rad" ) ;
  celly_ = qcelly.getValue( "rad" ) ;
  if ( nx_ < 0 ) {
    nx_ = Int( ceil( wx/cellx_ ) ) ;
    ny_ = Int( ceil( wy/celly_ ) ) ;
  }
}

void STGrid::selectData( Table &tab )
{
  Int ifno = ifno_ ;
  Table taborg( infile_ ) ;
  if ( ifno == -1 ) {
    LogIO os( LogOrigin("STGrid","getData",WHERE) ) ;
//     os << LogIO::SEVERE
//        << "Please set IFNO before actual gridding" 
//        << LogIO::EXCEPTION ;
    ROScalarColumn<uInt> ifnoCol( taborg, "IFNO" ) ;
    ifno = ifnoCol( 0 ) ;
    os << LogIO::WARN
       << "IFNO is not given. Using default IFNO: " << ifno << LogIO::POST ;
  }
  tab = taborg( taborg.col("IFNO") == ifno ) ;
  if ( tab.nrow() == 0 ) {
    LogIO os( LogOrigin("STGrid","getData",WHERE) ) ;
    os << LogIO::SEVERE
       << "No corresponding rows for given IFNO: " << ifno 
       << LogIO::EXCEPTION ;
  }
}

void STGrid::getData( Cube<Float> &spectra,
                      Matrix<Double> &direction,
                      Cube<uChar> &flagtra,
                      Matrix<uInt> &rflag,
                      Matrix<Float> &weight ) 
{
  Table tab ;
  selectData( tab ) ;
  ROScalarColumn<uInt> polnoCol( tab, "POLNO" ) ;
  Vector<uInt> pols = polnoCol.getColumn() ;
  Vector<uInt> pollistOrg ;
  uInt npolOrg = 0 ;
  for ( uInt i = 0 ; i < pols.size() ; i++ ) {
    if ( allNE( pollistOrg, pols[i] ) ) {
      pollistOrg.resize( npolOrg+1, True ) ;
      pollistOrg[npolOrg] = pols[i] ;
      npolOrg++ ;
    }
  }
  if ( pollist_.size() == 0 )
    pollist_ = pollistOrg ;
  else {
    Vector<uInt> newlist ;
    uInt newsize = 0 ;
    for ( uInt i = 0 ; i < pollist_.size() ; i++ ) {
      if ( anyEQ( pols, pollist_[i] ) ) {
        newlist.resize( newsize+1, True ) ;
        newlist[newsize] = pollist_[i] ;
        newsize++ ;
      }
    }
    pollist_ = newlist ;
  }
  npol_ = pollist_.size() ;
  ROArrayColumn<uChar> tmpCol( tab, "FLAGTRA" ) ;
  nchan_ = tmpCol( 0 ).nelements() ;
  nrow_ = tab.nrow() / npolOrg ;
//   cout << "npol_ = " << npol_ << endl ;
//   cout << "nchan_ = " << nchan_ << endl ;
//   cout << "nrow_ = " << nrow_ << endl ;
  spectra.resize( npol_, nchan_, nrow_ ) ;
  flagtra.resize( npol_, nchan_, nrow_ ) ;
  rflag.resize( npol_, nrow_ ) ;
  Cube<Float> tsys( npol_, nchan_, nrow_ ) ;
  Matrix<Double> tint( npol_, nrow_ ) ;
  for ( Int ipol = 0 ; ipol < npol_ ; ipol++ ) {
    Table subt = tab( tab.col("POLNO") == pollist_[ipol] ) ;
    ROArrayColumn<Float> spectraCol( subt, "SPECTRA" ) ;
    ROArrayColumn<Double> directionCol( subt, "DIRECTION" ) ;
    ROArrayColumn<uChar> flagtraCol( subt, "FLAGTRA" ) ;
    ROScalarColumn<uInt> rflagCol( subt, "FLAGROW" ) ;
    ROArrayColumn<Float> tsysCol( subt, "TSYS" ) ;
    ROScalarColumn<Double> tintCol( subt, "INTERVAL" ) ;
    Matrix<Float> tmpF = spectra.yzPlane( ipol ) ;
    Matrix<uChar> tmpUC = flagtra.yzPlane( ipol ) ;
    Vector<uInt> tmpUI = rflag.row( ipol ) ;
    spectraCol.getColumn( tmpF ) ;
    flagtraCol.getColumn( tmpUC ) ;
    rflagCol.getColumn( tmpUI ) ;
    if ( ipol == 0 )
      directionCol.getColumn( direction ) ;
    Matrix<Float> tmpF2 = tsysCol.getColumn() ;
    Vector<Double> tmpD = tint.row( ipol ) ;
    if ( tmpF2.shape()(0) == nchan_ ) {
      tsys.yzPlane( ipol ) = tmpF2 ;
    }
    else {
      tsys.yzPlane( ipol ) = tmpF2(0,0) ;
    }
    tintCol.getColumn( tmpD ) ;
  }

  getWeight( weight, tsys, tint ) ;
}

void STGrid::getWeight( Matrix<Float> &w,
                        Cube<Float> &tsys,
                        Matrix<Double> &tint ) 
{
  // resize
  w.resize( nchan_, nrow_ ) ;

  // set weight
  w = 1.0 ;
  Bool warn = False ;
  if ( wtype_.compare( "UNIFORM" ) == 0 ) {
    // do nothing
  }
  else if ( wtype_.compare( "TINT" ) == 0 ) {
    if ( npol_ > 1 ) warn = True ;
    for ( Int irow = 0 ; irow < nrow_ ; irow++ ) {
      Float val = mean( tint.column( irow ) ) ;
      w.column( irow ) = w.column( irow ) *  val ;
    }
  }
  else if ( wtype_.compare( "TSYS" ) == 0 ) {
    if ( npol_ > 1 ) warn = True ;
    for ( Int irow = 0 ; irow < nrow_ ; irow++ ) {
      Matrix<Float> arr = tsys.xyPlane( irow ) ;
      for ( Int ichan = 0 ; ichan < nchan_ ; ichan++ ) {
        Float val = mean( arr.column( ichan ) ) ;
        w(ichan,irow) = w(ichan,irow) / ( val * val ) ;
      }
    }
  }
  else if ( wtype_.compare( "TINTSYS" ) == 0 ) {
    if ( npol_ > 1 ) warn = True ;
    for ( Int irow = 0 ; irow < nrow_ ; irow++ ) {
      Float interval = mean( tint.column( irow ) ) ;
      Matrix<Float> arr = tsys.xyPlane( irow ) ;
      for ( Int ichan = 0 ; ichan < nchan_ ; ichan++ ) {
        Float temp = mean( arr.column( ichan ) ) ;
        w(ichan,irow) = w(ichan,irow) * interval / ( temp * temp ) ;
      }
    }
  }
  else {
    LogIO os( LogOrigin("STGrid", "getWeight", WHERE) ) ;
    os << LogIO::WARN << "Unsupported weight type '" << wtype_ << "', apply UNIFORM weight" << LogIO::POST ; 
  }

  if ( npol_ > 1 ) {
    LogIO os( LogOrigin("STGrid", "getWeight", WHERE) ) ;
    os << LogIO::WARN << "STGrid doesn't support assigning polarization-dependent weight. Use averaged weight over polarization." << LogIO::POST ;
  }
}

void STGrid::toInt( Array<uChar> *u, Array<Int> *v ) 
{
  uInt len = u->nelements() ;
  Int *int_p = new Int[len] ;
  Bool deleteIt ;
  const uChar *data_p = u->getStorage( deleteIt ) ;
  Int *i_p = int_p ;
  const uChar *u_p = data_p ;
  for ( uInt i = 0 ; i < len ; i++ ) {
    *i_p = ( *u_p == 0 ) ? 0 : 1 ;
    i_p++ ;
    u_p++ ;
  }
  u->freeStorage( data_p, deleteIt ) ;
  v->takeStorage( u->shape(), int_p, TAKE_OVER ) ;
}

void STGrid::toInt( Array<uInt> *u, Array<Int> *v ) 
{
  uInt len = u->nelements() ;
  Int *int_p = new Int[len] ;
  Bool deleteIt ;
  const uInt *data_p = u->getStorage( deleteIt ) ;
  Int *i_p = int_p ;
  const uInt *u_p = data_p ;
  for ( uInt i = 0 ; i < len ; i++ ) {
    *i_p = ( *u_p == 0 ) ? 0 : 1 ;
    i_p++ ;
    u_p++ ;
  }
  u->freeStorage( data_p, deleteIt ) ;
  v->takeStorage( u->shape(), int_p, TAKE_OVER ) ;
}

void STGrid::toPixel( Matrix<Double> &world, Matrix<Double> &pixel )
{
  // gridding will be done on (nx_+2*convSupport_) x (ny_+2*convSupport_) 
  // grid plane to avoid unexpected behavior on grid edge
  Vector<Double> pixc( 2 ) ;
  //pixc(0) = Double( nx_-1 ) * 0.5 ;
  //pixc(1) = Double( ny_-1 ) * 0.5 ;
  pixc(0) = Double( nx_+2*convSupport_-1 ) * 0.5 ;
  pixc(1) = Double( ny_+2*convSupport_-1 ) * 0.5 ;
  uInt nrow = world.shape()[1] ;
  Vector<Double> cell( 2 ) ;
  cell(0) = cellx_ ;
  cell(1) = celly_ ;
  //ofstream ofs( "grid.dat", ios::out ) ; 
  for ( uInt irow = 0 ; irow < nrow ; irow++ ) {
    //ofs << irow ;
    for ( uInt i = 0 ; i < 2 ; i++ ) {
      pixel( i, irow ) = pixc(i) + ( world(i, irow) - center_(i) ) / cell(i) ;
      //ofs << " " << world(i, irow) << " " << pixel(i, irow) ;
    }
    //ofs << endl ;
  }
  //ofs.close() ;
}

void STGrid::boxFunc( Vector<Float> &convFunc, Int &convSize ) 
{
  convFunc = 0.0 ;
  for ( Int i = 0 ; i < convSize/2 ; i++ )
    convFunc(i) = 1.0 ;
}

#define NEED_UNDERSCORES
#if defined(NEED_UNDERSCORES)
#define grdsf grdsf_
#endif
extern "C" { 
   void grdsf(Double*, Double*);
}
void STGrid::spheroidalFunc( Vector<Float> &convFunc ) 
{
  convFunc = 0.0 ;
  for ( Int i = 0 ; i < convSampling_*convSupport_ ; i++ ) {
    Double nu = Double(i) / Double(convSupport_*convSampling_) ;
    Double val ;
    grdsf( &nu, &val ) ;
    convFunc(i) = ( 1.0 - nu * nu ) * val ;
  }
}

void STGrid::gaussFunc( Vector<Float> &convFunc ) 
{
  convFunc = 0.0 ;
  // HWHM of the Gaussian is convSupport_ / 4
  // To take into account Gaussian tail, kernel cutoff is set to 4 * HWHM
  Int len = convSampling_ * convSupport_ ;
  Double hwhm = len * 0.25 ;
  for ( Int i = 0 ; i < len ; i++ ) {
    Double val = Double(i) / hwhm ;
    convFunc(i) = exp( -log(2)*val*val ) ;
  }
}

void STGrid::pbFunc( Vector<Float> &convFunc ) 
{
  convFunc = 0.0 ;
}

void STGrid::setConvFunc( Vector<Float> &convFunc )
{
  convSupport_ = userSupport_ ;
  if ( convType_ == "BOX" ) {
    if ( convSupport_ < 0 )
      convSupport_ = 0 ;
    Int convSize = convSampling_ * ( 2 * convSupport_ + 2 )  ;
    convFunc.resize( convSize ) ;
    boxFunc( convFunc, convSize ) ;
  }
  else if ( convType_ == "SF" ) {
    if ( convSupport_ < 0 )
      convSupport_ = 3 ;
    Int convSize = convSampling_ * ( 2 * convSupport_ + 2 )  ;
    convFunc.resize( convSize ) ;
    spheroidalFunc( convFunc ) ;
  }
  else if ( convType_ == "GAUSS" ) {
    // to take into account Gaussian tail
    if ( convSupport_ < 0 )
      convSupport_ = 12 ; // 3 * 4
    else {
      convSupport_ = userSupport_ * 4 ;
    }
    Int convSize = convSampling_ * ( 2 * convSupport_ + 2 ) ;
    convFunc.resize( convSize ) ;
    gaussFunc( convFunc ) ;
  }
  else if ( convType_ == "PB" ) {
    if ( convSupport_ < 0 ) 
      convSupport_ = 0 ;
    pbFunc( convFunc ) ;
  }
  else {
    throw AipsError( "Unsupported convolution function" ) ;
  }
} 

string STGrid::saveData( string outfile )
{
  //Int polno = 0 ;
  string outfile_ ;
  if ( outfile.size() == 0 ) {
    if ( infile_.lastchar() == '/' ) {
      outfile_ = infile_.substr( 0, infile_.size()-1 ) ;
    }
    else {
      outfile_ = infile_ ;
    }
    outfile_ += ".grid" ;
  }
  else {
    outfile_ = outfile ;
  }
  CountedPtr<Scantable> ref( new Scantable( infile_, Table::Memory ) ) ;
  //cout << "ref->nchan()=" << ref->nchan() << endl ;
  CountedPtr<Scantable> out( new Scantable( *ref, True ) ) ;
  Table tab = out->table() ;
  IPosition dshape = data_.shape() ;
  Int nrow = nx_ * ny_ * npol_ ;
  tab.rwKeywordSet().define( "nPol", npol_ ) ;
  tab.addRow( nrow ) ;
  Vector<Double> cpix( 2 ) ;
  cpix(0) = Double( nx_ - 1 ) * 0.5 ;
  cpix(1) = Double( ny_ - 1 ) * 0.5 ;
  Vector<Double> dir( 2 ) ;
  ArrayColumn<Double> directionCol( tab, "DIRECTION" ) ;
  ArrayColumn<Float> spectraCol( tab, "SPECTRA" ) ;
  ScalarColumn<uInt> polnoCol( tab, "POLNO" ) ;
  Int irow = 0 ;
  for ( Int iy = 0 ; iy < ny_ ; iy++ ) {
    for ( Int ix = 0 ; ix < nx_ ; ix++ ) {
      for ( Int ipol = 0 ; ipol < npol_ ; ipol++ ) {
        IPosition start( 4, ix, iy, ipol, 0 ) ;
        IPosition end( 4, ix, iy, ipol, nchan_-1 ) ;
        IPosition inc( 4, 1, 1, 1, 1 ) ;
        Vector<Float> sp = data_( start, end, inc ) ;
        dir(0) = center_(0) - ( cpix(0) - (Double)ix ) * cellx_ ;
        dir(1) = center_(1) - ( cpix(1) - (Double)iy ) * celly_ ;
        spectraCol.put( irow, sp ) ;
        directionCol.put( irow, dir ) ;
        polnoCol.put( irow, pollist_[ipol] ) ;
        irow++ ;
      }
    }
  }
  //cout << "outfile_=" << outfile_ << endl ;
  out->makePersistent( outfile_ ) ;
  
  return outfile_ ;
}

}
