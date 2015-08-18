//
// C++ Implementation: STGrid
//
// Description:
//
//
// Author: Takeshi Nakazato <takeshi.nakazato@nao.ac.jp>, (C) 2011
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <casa/BasicSL/String.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Quanta/Quantum.h>
#include <casa/Quanta/QuantumHolder.h>
#include <casa/Utilities/CountedPtr.h>
#include <casa/Logging/LogIO.h>

#include <coordinates/Coordinates/DirectionCoordinate.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/TableRow.h>
#include <tables/Tables/ExprNode.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/TableCopy.h>

#include <measures/Measures/MDirection.h>

#include "MathUtils.h"
#include <atnf/PKSIO/SrcType.h>

#include "STGrid.h"

using namespace std ;
using namespace concurrent ;
using namespace casa ;
using namespace asap ;

namespace asap {

// for performance check
double eToInt = 0.0 ;
double eGetWeight = 0.0 ;

// constructor
STGrid::STGrid()
  : vshape_( 1 ), wshape_( 2 ), dshape_( 2 )
{
  init() ;
}

STGrid::STGrid( const string infile )
  : vshape_( 1 ), wshape_( 2 ), dshape_( 2 )
{
  init() ;

  setFileIn( infile ) ;
}

STGrid::STGrid( const vector<string> infile )
{
  init() ;

  setFileList( infile ) ;
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
  truncate_ = "";
  gwidth_ = "";
  jwidth_ = "";
  convSampling_ = 100 ;
  nprocessed_ = 0 ;
  nchunk_ = 0 ;

  // initialize user input 
  nxUI_ = -1 ;
  nyUI_ = -1 ;
  cellxUI_ = "" ;
  cellyUI_ = "" ;
  centerUI_ = "" ;
  doclip_ = False ;
}

void STGrid::setFileIn( const string infile )
{
  nfile_ = 1 ;
  String name( infile ) ;
  infileList_.resize( nfile_ ) ;
  infileList_[0] = String(infile) ;
}

void STGrid::setFileList( const vector<string> infile )
{
  nfile_ = infile.size() ;
  infileList_.resize( nfile_ ) ;
  for ( uInt i = 0 ; i < nfile_ ; i++ ) {
    infileList_[i] = infile[i] ;
  }
}

void STGrid::setPolList( vector<unsigned int> pols )
{
  pollist_.assign( Vector<uInt>( pols ) ) ;
}

void STGrid::setScanList( vector<unsigned int> scans )
{
  scanlist_.assign( Vector<uInt>( scans ) ) ;
}

void STGrid::setWeight( const string wType )
{
  wtype_ = String( wType ) ;
  wtype_.upcase() ;
}

void STGrid::defineImage( int nx,
                          int ny,
                          string scellx,
                          string scelly,
                          string scenter ) 
{
  nxUI_ = (Int)nx ;
  nyUI_ = (Int)ny ;
  cellxUI_ = String( scellx ) ;
  cellyUI_ = String( scelly ) ;
  centerUI_ = String( scenter ) ;
}
  
void STGrid::setFunc( string convType,
                      int convSupport,
                      string truncate,
                      string gwidth,
                      string jwidth ) 
{
  convType_ = String( convType ) ;
  convType_.upcase() ;
  userSupport_ = (Int)convSupport ;
  truncate_ = String( truncate );
  gwidth_ = String( gwidth );
  jwidth_ = String( jwidth );
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
void STGrid::call_ggridsd( Array<Double> &xypos,
                           Array<Complex> &spectra,
                           Int &nvispol,
                           Int &nvischan,
                           Array<Int> &flagtra,
                           Array<Int> &flagrow,
                           Array<Float> &weight,
                           Int &nrow,
                           Int &irow,
                           Array<Complex> &gdata,
                           Array<Float> &gwgt,
                           Int &nx,
                           Int &ny,
                           Int &npol,
                           Int &nchan,
                           Int &support,
                           Int &sampling,
                           Vector<Float> &convFunc,
                           Int *chanMap,
                           Int *polMap ) 
{
  // parameters for gridding
  Int idopsf = 0 ;
  Int len = npol*nchan ;
  Double *sumw_p = new Double[len] ;
  {
    Double *work_p = sumw_p ;
    for ( Int i = 0 ; i < len ; i++ ) {
      *work_p = 0.0 ;
      work_p++ ;
    }
  }

  // prepare pointer
  Bool deletePos, deleteData, deleteWgt, deleteFlag, deleteFlagR, deleteConv, deleteDataG, deleteWgtG ;
  Double *xy_p = xypos.getStorage( deletePos ) ;
  const Complex *values_p = spectra.getStorage( deleteData ) ;
  const Int *flag_p = flagtra.getStorage( deleteFlag ) ;
  const Int *rflag_p = flagrow.getStorage( deleteFlagR ) ;
  const Float *wgt_p = weight.getStorage( deleteWgt ) ;
  Complex *grid_p = gdata.getStorage( deleteDataG ) ;
  Float *wgrid_p = gwgt.getStorage( deleteWgtG ) ;
  Float *conv_p = convFunc.getStorage( deleteConv ) ;

  // pass copy of irow to ggridsd since it will be modified in theroutine
  Int irowCopy = irow ;
      
  // call ggridsd
  ggridsd( xy_p,
           values_p,
           &nvispol,
           &nvischan,
           &idopsf,
           flag_p,
           rflag_p,
           wgt_p,
           &nrow,
           &irowCopy,
           grid_p,
           wgrid_p,
           &nx,
           &ny,
           &npol,
           &nchan,
           &support,
           &sampling,
           conv_p,
           chanMap,
           polMap,
           sumw_p ) ;

  // finalization
  xypos.putStorage( xy_p, deletePos ) ;
  spectra.freeStorage( values_p, deleteData ) ;
  flagtra.freeStorage( flag_p, deleteFlag ) ;
  flagrow.freeStorage( rflag_p, deleteFlagR ) ;
  weight.freeStorage( wgt_p, deleteWgt ) ;
  gdata.putStorage( grid_p, deleteDataG ) ;
  gwgt.putStorage( wgrid_p, deleteWgtG ) ;
  convFunc.putStorage( conv_p, deleteConv ) ;
  delete sumw_p ;
}

#define NEED_UNDERSCORES
#if defined(NEED_UNDERSCORES)
#define ggridsd2 ggridsd2_
#endif
extern "C" { 
   void ggridsd2(Double*,
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
                 Complex*,
                 Float*,
                 Float*,
                 Complex*,
                 Float*,
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
void STGrid::call_ggridsd2( Array<Double> &xypos,
                            Array<Complex> &spectra,
                            Int &nvispol,
                            Int &nvischan,
                            Array<Int> &flagtra,
                            Array<Int> &flagrow,
                            Array<Float> &weight,
                            Int &nrow,
                            Int &irow,
                            Array<Complex> &gdata,
                            Array<Float> &gwgt,
                            Array<Int> &npoints,
                            Array<Complex> &clipmin,
                            Array<Float> &clipwmin,
                            Array<Float> &clipcmin,
                            Array<Complex> &clipmax,
                            Array<Float> &clipwmax,
                            Array<Float> &clipcmax,
                            Int &nx,
                            Int &ny,
                            Int &npol,
                            Int &nchan,
                            Int &support,
                            Int &sampling,
                            Vector<Float> &convFunc,
                            Int *chanMap,
                            Int *polMap ) 
{
  // parameters for gridding
  Int idopsf = 0 ;
  Int len = npol*nchan ;
  Double *sumw_p = new Double[len] ;
  {
    Double *work_p = sumw_p ;
    for ( Int i = 0 ; i < len ; i++ ) {
      *work_p = 0.0 ;
      work_p++ ;
    }
  }

  // prepare pointer
  Bool deletePos, deleteData, deleteWgt, deleteFlag, deleteFlagR, deleteConv, deleteDataG, deleteWgtG, deleteNpts, deleteCMin, deleteCWMin, deleteCCMin, deleteCMax, deleteCWMax, deleteCCMax ;
  Double *xy_p = xypos.getStorage( deletePos ) ;
  const Complex *values_p = spectra.getStorage( deleteData ) ;
  const Int *flag_p = flagtra.getStorage( deleteFlag ) ;
  const Int *rflag_p = flagrow.getStorage( deleteFlagR ) ;
  const Float *wgt_p = weight.getStorage( deleteWgt ) ;
  Complex *grid_p = gdata.getStorage( deleteDataG ) ;
  Float *wgrid_p = gwgt.getStorage( deleteWgtG ) ;
  Float *conv_p = convFunc.getStorage( deleteConv ) ;
  Int *npts_p = npoints.getStorage( deleteNpts ) ;
  Complex *cmin_p = clipmin.getStorage( deleteCMin ) ;
  Float *cwmin_p = clipwmin.getStorage( deleteCWMin ) ;
  Float *ccmin_p = clipcmin.getStorage( deleteCCMin ) ;
  Complex *cmax_p = clipmax.getStorage( deleteCMax ) ;
  Float *cwmax_p = clipwmax.getStorage( deleteCWMax ) ;
  Float *ccmax_p = clipcmax.getStorage( deleteCCMax ) ;

  // pass copy of irow to ggridsd since it will be modified in theroutine
  Int irowCopy = irow ;
      
  // call ggridsd
  ggridsd2( xy_p,
            values_p,
            &nvispol,
            &nvischan,
            &idopsf,
            flag_p,
            rflag_p,
            wgt_p,
            &nrow,
            &irowCopy,
            grid_p,
            wgrid_p,
            npts_p,
            cmin_p,
            cwmin_p,
            ccmin_p,
            cmax_p,
            cwmax_p,
            ccmax_p,
            &nx,
            &ny,
            &npol,
            &nchan,
            &support,
            &sampling,
            conv_p,
            chanMap,
            polMap,
            sumw_p ) ;

  // finalization
  xypos.putStorage( xy_p, deletePos ) ;
  spectra.freeStorage( values_p, deleteData ) ;
  flagtra.freeStorage( flag_p, deleteFlag ) ;
  flagrow.freeStorage( rflag_p, deleteFlagR ) ;
  weight.freeStorage( wgt_p, deleteWgt ) ;
  gdata.putStorage( grid_p, deleteDataG ) ;
  gwgt.putStorage( wgrid_p, deleteWgtG ) ;
  convFunc.putStorage( conv_p, deleteConv ) ;
  clipmin.putStorage( cmin_p, deleteCMin ) ;
  clipwmin.putStorage( cwmin_p, deleteCWMin ) ;
  clipcmin.putStorage( ccmin_p, deleteCCMin ) ;
  clipmax.putStorage( cmax_p, deleteCMax ) ;
  clipwmax.putStorage( cwmax_p, deleteCWMax ) ;
  clipcmax.putStorage( ccmax_p, deleteCCMax ) ;
  delete sumw_p ;
}

void STGrid::grid()
{
  LogIO os( LogOrigin("STGrid", "grid", WHERE) ) ;
  double t0,t1 ;

  // data selection
  t0 = mathutil::gettimeofday_sec() ;
  selectData() ;
  t1 = mathutil::gettimeofday_sec() ;
  os << LogIO::DEBUGGING << "selectData: elapsed time is " << t1-t0 << " sec." << LogIO::POST ;

  setupGrid() ;
  setupArray() ;

  if ( wtype_.compare("UNIFORM") != 0 &&
       wtype_.compare("TINT") != 0 && 
       wtype_.compare("TSYS") != 0 &&
       wtype_.compare("TINTSYS") != 0 ) {
    LogIO os( LogOrigin("STGrid", "grid", WHERE) ) ;
    os << LogIO::WARN << "Unsupported weight type '" << wtype_ << "', apply UNIFORM weight" << LogIO::POST ;
    wtype_ = "UNIFORM" ;
  }

  // Warn if gauss or gjinc gridding with non-square cells
  if ((cellx_ != celly_) && (convType_=="GAUSS"||convType_=="GJINC")) {
    os << LogIO::WARN 
       << "The " << convType_ << " gridding doesn't support non-square grid." << endl
       << "Result may be wrong." << LogIO::POST;
  }

  // grid parameter
  os << LogIO::DEBUGGING ;
  os << "----------" << endl ;
  os << "Data selection summary" << endl ;
  os << "   ifno = " << ifno_ << endl ;
  os << "   pollist = " << pollist_ << endl ;
  os << "   scanlist = " << scanlist_ << endl ;
  os << "----------" << endl ;
  os << "Grid parameter summary" << endl ;
  os << "   (nx,ny) = (" << nx_ << "," << ny_ << ")" << endl ;
  os << "   (cellx,celly) = (" << cellx_ << "," << celly_ << ")" << endl ;
  os << "   center = " << center_ << endl ;
  os << "   weighting = " << wtype_ << endl ;
  os << "   convfunc = " << convType_ << endl;
  if (convType_ == "GAUSS") {
    os << "      gwidth = " << gwidth_ << endl;
    os << "      truncate = " << truncate_ << endl;
  }
  else if (convType_ == "GJINC") {
    os << "      gwidth = " << gwidth_ << endl;
    os << "      jwidth = " << jwidth_ << endl;
    os << "      truncate = " << truncate_ << endl;
  }
  else {
    os << "      support = " << userSupport_ << endl;
  }
  os << "   doclip = " << (doclip_?"True":"False") << endl ;
  os << "----------" << LogIO::POST ;
  os << LogIO::NORMAL ;

  if ( doclip_ )
    gridPerRowWithClipping() ;
  else 
    gridPerRow() ;
}

void STGrid::updateChunkShape()
{
  // TODO: nchunk_ must be determined from nchan_, npol_, and (nx_,ny_) 
  //       by considering data size to be allocated for ggridsd input/output
  nchunk_ = 400 ;
  nchunk_ = min( nchunk_, nrow_ ) ;
  vshape_ = IPosition( 1, nchunk_ ) ;
  wshape_ = IPosition( 2, nchan_, nchunk_ ) ;
  dshape_ = IPosition( 2, 2, nchunk_ ) ;
}

struct STGChunk {
  Int nrow ;
  Array<Complex> spectra;
  Array<Int> flagtra;
  Array<Int> rflag;
  Array<Float> weight;
  Array<Double> direction;
  STGChunk(IPosition const &wshape, IPosition const &vshape,
	   IPosition const &dshape)
    : spectra(wshape), flagtra(wshape), rflag(vshape), weight(wshape),
      direction(dshape)
  { }
};

struct STCommonData {
  Int gnx;
  Int gny;
  Int *chanMap;
  Vector<Float> convFunc ;
  Array<Complex> gdataArrC;
  Array<Float> gwgtArr;
  STCommonData(IPosition const &gshape, Array<Float> const &data)
    : gdataArrC(gshape, 0.0), gwgtArr(data) {}
};

struct STCommonDataWithClipping {
  Int gnx;
  Int gny;
  Int *chanMap;
  Vector<Float> convFunc ;
  Array<Complex> gdataArrC;
  Array<Float> gwgtArr;
  Array<Int> npoints ;
  Array<Complex> clipMin ;
  Array<Float> clipWMin ;
  Array<Float> clipCMin ;
  Array<Complex> clipMax ;
  Array<Float> clipWMax ;
  Array<Float> clipCMax ;  
  STCommonDataWithClipping(IPosition const &gshape, 
                           IPosition const &pshape, 
                           Array<Float> const &data)
    : gdataArrC(gshape, 0.0), 
      gwgtArr(data), 
      npoints(pshape, 0),
      clipMin(gshape, Complex(FLT_MAX,0.0)),
      clipWMin(gshape, 0.0),
      clipCMin(gshape, 0.0),
      clipMax(gshape, Complex(-FLT_MAX,0.0)),
      clipWMax(gshape, 0.0),
      clipCMax(gshape, 0.0)
  {}
};

#define DO_AHEAD 3

struct STContext {
  STCommonData &common;
  FIFO<STGChunk *, DO_AHEAD> queue;
  STGrid *const self;
  const Int pol;
  STContext(STGrid *obj, STCommonData &common, Int pol)
    : common(common), self(obj), pol(pol) {}
};

struct STContextWithClipping {
  STCommonDataWithClipping &common;
  FIFO<STGChunk *, DO_AHEAD> queue;
  STGrid *const self;
  const Int pol;
  STContextWithClipping(STGrid *obj, STCommonDataWithClipping &common, Int pol)
    : common(common), self(obj), pol(pol) {}
};


bool STGrid::produceChunk(void *ctx) throw(PCException)
{
  STContext &context = *(STContext *)ctx;
  if ( context.self->nprocessed_ >= context.self->nrow_ ) {
    return false;
  }
  STGChunk *chunk = new STGChunk(context.self->wshape_,
				 context.self->vshape_,
				 context.self->dshape_);

  double t0 = mathutil::gettimeofday_sec() ;
  chunk->nrow = context.self->getDataChunk(
	context.self->wshape_, context.self->vshape_, context.self->dshape_,
	chunk->spectra, chunk->direction,
	chunk->flagtra, chunk->rflag, chunk->weight);
  double t1 = mathutil::gettimeofday_sec() ;
  context.self->eGetData_ += t1-t0 ;

  context.queue.lock();
  context.queue.put(chunk);
  context.queue.unlock();
  return true;
}

void STGrid::consumeChunk(void *ctx) throw(PCException)
{
  STContext &context = *(STContext *)ctx;
  STGChunk *chunk = NULL;
  try {
    context.queue.lock();
    chunk = context.queue.get();
    context.queue.unlock();
  } catch (FullException &e) {
    context.queue.unlock();
    // TODO: log error
    throw PCException();
  }

  double t0, t1 ;
  // world -> pixel
  Array<Double> xypos( context.self->dshape_ ) ;
  t0 = mathutil::gettimeofday_sec() ;
  context.self->toPixel( chunk->direction, xypos ) ;
  t1 = mathutil::gettimeofday_sec() ;
  context.self->eToPixel_ += t1-t0 ;
   
  // call ggridsd
  Int nvispol = 1 ;
  Int irow = -1 ;
  t0 = mathutil::gettimeofday_sec() ;
  context.self->call_ggridsd( xypos,
		chunk->spectra,
		nvispol,
		context.self->nchan_,
		chunk->flagtra,
		chunk->rflag,
		chunk->weight,
		chunk->nrow,
		irow,
		context.common.gdataArrC,
		context.common.gwgtArr,
		context.common.gnx,
		context.common.gny,
		context.self->npol_,
		context.self->nchan_,
		context.self->convSupport_,
		context.self->convSampling_,
		context.common.convFunc,
		context.common.chanMap,
		(Int*)&context.pol ) ;
  t1 = mathutil::gettimeofday_sec() ;
  context.self->eGGridSD_ += t1-t0 ;
  
  delete chunk;
}

void STGrid::gridPerRow()
{
  LogIO os( LogOrigin("STGrid", "gridPerRow", WHERE) ) ;
  double t0, t1 ;


  // grid data
  // Extend grid plane with convSupport_
  //   Int gnx = nx_+convSupport_*2 ;
  //   Int gny = ny_+convSupport_*2 ;
  Int gnx = nx_;
  Int gny = ny_;

  IPosition gshape( 4, gnx, gny, npol_, nchan_ ) ;
  // 2011/12/20 TN
  // data_ and gwgtArr share storage
  data_.resize( gshape ) ;
  data_ = 0.0 ;
  flag_.resize( gshape ) ;
  flag_ = (uChar)0;
  STCommonData common = STCommonData(gshape, data_);
  common.gnx = gnx ;
  common.gny = gny ;

  // parameters for gridding
  Int *chanMap = new Int[nchan_] ;
  for ( Int i = 0 ; i < nchan_ ; i++ ) {
    chanMap[i] = i ;
  }
  common.chanMap = chanMap;

  // convolution kernel
  t0 = mathutil::gettimeofday_sec() ;
  setConvFunc( common.convFunc ) ;
  t1 = mathutil::gettimeofday_sec() ;
  os << LogIO::DEBUGGING << "setConvFunc: elapsed time is " << t1-t0 << " sec." << LogIO::POST ; 

  // for performance check
  eGetData_ = 0.0 ;
  eToPixel_ = 0.0 ;
  eGGridSD_ = 0.0 ;
  double eInitPol = 0.0 ;

  for ( uInt ifile = 0 ; ifile < nfile_ ; ifile++ ) {
    initTable( ifile ) ;

    os << "start table " << ifile << ": " << infileList_[ifile] << LogIO::POST ;   
    Broker broker = Broker(produceChunk, consumeChunk);
    for ( Int ipol = 0 ; ipol < npol_ ; ipol++ ) {
      t0 = mathutil::gettimeofday_sec() ;
      initPol( ipol ) ; // set ptab_ and attach()
      t1 = mathutil::gettimeofday_sec() ;
      eInitPol += t1-t0 ;
      
      STContext context(this, common, ipol);
      
      os << "start pol " << ipol << LogIO::POST ;
      
      nprocessed_ = 0 ;
#if 1
      broker.runProducerAsMasterThread(&context, DO_AHEAD);
#else
      for (;;) {
        bool produced = produceChunk(&context);
        if (! produced) {
          break;
        }
        consumeChunk(&context);
      }
#endif

      os << "end pol " << ipol << LogIO::POST ;

    }
    os << "end table " << ifile << LogIO::POST ;   
  }
  os << LogIO::DEBUGGING << "initPol: elapsed time is " << eInitPol << " sec." << LogIO::POST ; 
  os << LogIO::DEBUGGING << "getData: elapsed time is " << eGetData_-eToInt-eGetWeight << " sec." << LogIO::POST ; 
  os << LogIO::DEBUGGING << "toPixel: elapsed time is " << eToPixel_ << " sec." << LogIO::POST ; 
  os << LogIO::DEBUGGING << "ggridsd: elapsed time is " << eGGridSD_ << " sec." << LogIO::POST ; 
  os << LogIO::DEBUGGING << "toInt: elapsed time is " << eToInt << " sec." << LogIO::POST ;
  os << LogIO::DEBUGGING << "getWeight: elapsed time is " << eGetWeight << " sec." << LogIO::POST ;
  
  delete chanMap ;

  // set data
  setData( common.gdataArrC, common.gwgtArr ) ;

}

void STGrid::consumeChunkWithClipping(void *ctx) throw(PCException)
{
  STContextWithClipping &context = *(STContextWithClipping *)ctx;
  STGChunk *chunk = NULL;
  try {
    context.queue.lock();
    chunk = context.queue.get();
    context.queue.unlock();
  } catch (FullException &e) {
    context.queue.unlock();
    // TODO: log error
    throw PCException();
  }

  double t0, t1 ;
  // world -> pixel
  Array<Double> xypos( context.self->dshape_ ) ;
  t0 = mathutil::gettimeofday_sec() ;
  context.self->toPixel( chunk->direction, xypos ) ;
  t1 = mathutil::gettimeofday_sec() ;
  context.self->eToPixel_ += t1-t0 ;
   
  // call ggridsd
  Int nvispol = 1 ;
  Int irow = -1 ;
  t0 = mathutil::gettimeofday_sec() ;
  context.self->call_ggridsd2( xypos,
		chunk->spectra,
		nvispol,
		context.self->nchan_,
		chunk->flagtra,
		chunk->rflag,
		chunk->weight,
		chunk->nrow,
		irow,
		context.common.gdataArrC,
		context.common.gwgtArr,
                context.common.npoints,
                context.common.clipMin,
                context.common.clipWMin,
                context.common.clipCMin,
                context.common.clipMax,
                context.common.clipWMax,
                context.common.clipCMax,
		context.common.gnx,
		context.common.gny,
		context.self->npol_,
		context.self->nchan_,
		context.self->convSupport_,
		context.self->convSampling_,
		context.common.convFunc,
		context.common.chanMap,
		(Int*)&context.pol ) ;
  t1 = mathutil::gettimeofday_sec() ;
  context.self->eGGridSD_ += t1-t0 ;
  
  delete chunk;
}

void STGrid::gridPerRowWithClipping()
{
  LogIO os( LogOrigin("STGrid", "gridPerRowWithClipping", WHERE) ) ;
  double t0, t1 ;


  // grid data
  // Extend grid plane with convSupport_
  //   Int gnx = nx_+convSupport_*2 ;
  //   Int gny = ny_+convSupport_*2 ;
  Int gnx = nx_;
  Int gny = ny_;

  IPosition gshape( 4, gnx, gny, npol_, nchan_ ) ;
  IPosition pshape( 3, gnx, gny, npol_ ) ;
  // 2011/12/20 TN
  // data_ and gwgtArr share storage
  data_.resize( gshape ) ;
  data_ = 0.0 ;
  flag_.resize( gshape ) ;
  flag_ = (uChar)0;
  STCommonDataWithClipping common = STCommonDataWithClipping( gshape,
                                                              pshape, 
                                                              data_ ) ;
  common.gnx = gnx ;
  common.gny = gny ;

  // parameters for gridding
  Int *chanMap = new Int[nchan_] ;
  for ( Int i = 0 ; i < nchan_ ; i++ ) {
    chanMap[i] = i ;
  }
  common.chanMap = chanMap;

  // convolution kernel
  t0 = mathutil::gettimeofday_sec() ;
  setConvFunc( common.convFunc ) ;
  t1 = mathutil::gettimeofday_sec() ;
  os << LogIO::DEBUGGING << "setConvFunc: elapsed time is " << t1-t0 << " sec." << LogIO::POST ; 

  // for performance check
  eGetData_ = 0.0 ;
  eToPixel_ = 0.0 ;
  eGGridSD_ = 0.0 ;
  double eInitPol = 0.0 ;

  for ( uInt ifile = 0 ; ifile < nfile_ ; ifile++ ) {
    initTable( ifile ) ;

    os << "start table " << ifile << ": " << infileList_[ifile] << LogIO::POST ;   
    Broker broker = Broker(produceChunk, consumeChunkWithClipping);
    for ( Int ipol = 0 ; ipol < npol_ ; ipol++ ) {
      t0 = mathutil::gettimeofday_sec() ;
      initPol( ipol ) ; // set ptab_ and attach()
      t1 = mathutil::gettimeofday_sec() ;
      eInitPol += t1-t0 ;
      
      STContextWithClipping context(this, common, ipol);
      
      os << "start pol " << ipol << LogIO::POST ;
      
      nprocessed_ = 0 ;
#if 1
      broker.runProducerAsMasterThread(&context, DO_AHEAD);
#else
      for (;;) {
        bool produced = produceChunk(&context);
        if (! produced) {
          break;
        }
        consumeChunkWithClipping(&context);
      }
#endif

      os << "end pol " << ipol << LogIO::POST ;

    }
    os << "end table " << ifile << LogIO::POST ;   
  }
  os << LogIO::DEBUGGING << "initPol: elapsed time is " << eInitPol << " sec." << LogIO::POST ; 
  os << LogIO::DEBUGGING << "getData: elapsed time is " << eGetData_-eToInt-eGetWeight << " sec." << LogIO::POST ; 
  os << LogIO::DEBUGGING << "toPixel: elapsed time is " << eToPixel_ << " sec." << LogIO::POST ; 
  os << LogIO::DEBUGGING << "ggridsd2: elapsed time is " << eGGridSD_ << " sec." << LogIO::POST ; 
  os << LogIO::DEBUGGING << "toInt: elapsed time is " << eToInt << " sec." << LogIO::POST ;
  os << LogIO::DEBUGGING << "getWeight: elapsed time is " << eGetWeight << " sec." << LogIO::POST ;
  
  delete chanMap ;

  // clip min and max in each grid
//   os << "BEFORE CLIPPING" << LogIO::POST ;
//   os << "gdataArrC=" << common.gdataArrC << LogIO::POST ;
//   os << "gwgtArr=" << common.gwgtArr << LogIO::POST ;
  t0 = mathutil::gettimeofday_sec() ;
  clipMinMax( common.gdataArrC, 
              common.gwgtArr,
              common.npoints,
              common.clipMin,
              common.clipWMin,
              common.clipCMin,
              common.clipMax,
              common.clipWMax,
              common.clipCMax ) ;
  t1 = mathutil::gettimeofday_sec() ;
  os << LogIO::DEBUGGING << "clipMinMax: elapsed time is " << t1-t0 << " sec." << LogIO::POST ;
//   os << "AFTER CLIPPING" << LogIO::POST ;
//   os << "gdataArrC=" << common.gdataArrC << LogIO::POST ;
//   os << "gwgtArr=" << common.gwgtArr << LogIO::POST ;

  // set data
  setData( common.gdataArrC, common.gwgtArr ) ;

}

void STGrid::clipMinMax( Array<Complex> &grid,
                         Array<Float> &weight,
                         Array<Int> &npoints,
                         Array<Complex> &clipmin,
                         Array<Float> &clipwmin,
                         Array<Float> &clipcmin,
                         Array<Complex> &clipmax,
                         Array<Float> &clipwmax,
                         Array<Float> &clipcmax )
{
  //LogIO os( LogOrigin("STGrid","clipMinMax",WHERE) ) ;

  // prepare pointers
  Bool delG, delW, delNP, delCMin, delCWMin, delCCMin, delCMax, delCWMax, delCCMax ;
  Complex *grid_p = grid.getStorage( delG ) ;
  Float *wgt_p = weight.getStorage( delW ) ;
  const Int *npts_p = npoints.getStorage( delNP ) ;
  const Complex *cmin_p = clipmin.getStorage( delCMin ) ;
  const Float *cwmin_p = clipwmin.getStorage( delCWMin ) ;
  const Float *ccmin_p = clipcmin.getStorage( delCCMin ) ;
  const Complex *cmax_p = clipmax.getStorage( delCMax ) ;
  const Float *cwmax_p = clipwmax.getStorage( delCWMax ) ;
  const Float *ccmax_p = clipcmax.getStorage( delCCMax ) ;

  const IPosition &gshape = grid.shape() ;
  long offset = gshape[0] * gshape[1] * gshape[2] ; // nx * ny * npol
  Int nchan = gshape[3] ;
  long origin = nchan * offset ;
  for ( long i = 0 ; i < offset ; i++ ) {
    if ( *npts_p > 2 ) {
      for ( Int ichan = 0 ; ichan < nchan ; ichan++ ) {
        // clip minimum and maximum
        *grid_p -= (*cmin_p)*(*cwmin_p)*(*ccmin_p)
          + (*cmax_p)*(*cwmax_p)*(*ccmax_p) ;
        *wgt_p -= (*cwmin_p)*(*ccmin_p)
          + (*cwmax_p)*(*ccmax_p) ;
        
        grid_p += offset ;
        wgt_p += offset ;
        cmin_p += offset ;
        cwmin_p += offset ;
        ccmin_p += offset ;
        cmax_p += offset ;
        cwmax_p += offset ;
        ccmax_p += offset ;
      }
      grid_p -= origin ;
      wgt_p -= origin ;
      cmin_p -= origin ;
      cwmin_p -= origin ;
      ccmin_p -= origin ;
      cmax_p -= origin ;
      cwmax_p -= origin ;
      ccmax_p -= origin ;
    }
    grid_p++ ;
    wgt_p++ ;
    npts_p++ ;
    cmin_p++ ;
    cwmin_p++ ;
    ccmin_p++ ;
    cmax_p++ ;
    cwmax_p++ ;
    ccmax_p++ ;
  }
  grid_p -= offset ;
  wgt_p -= offset ;
  npts_p -= offset ;
  cmin_p -= offset ;
  cwmin_p -= offset ;
  ccmin_p -= offset ;
  cmax_p -= offset ;
  cwmax_p -= offset ;
  ccmax_p -= offset ;  

  // finalization
  grid.putStorage( grid_p, delG ) ;
  weight.putStorage( wgt_p, delW ) ;
  npoints.freeStorage( npts_p, delNP ) ;
  clipmin.freeStorage( cmin_p, delCMin ) ;
  clipwmin.freeStorage( cwmin_p, delCWMin ) ;
  clipcmin.freeStorage( ccmin_p, delCCMin ) ;
  clipmax.freeStorage( cmax_p, delCMax ) ;
  clipwmax.freeStorage( cwmax_p, delCWMax ) ;
  clipcmax.freeStorage( ccmax_p, delCCMax ) ;
}

void STGrid::initPol( Int ipol ) 
{
  LogIO os( LogOrigin("STGrid","initPol",WHERE) ) ;
  if ( npolOrg_ == 1 ) {
    os << "single polarization data." << LogIO::POST ;
    ptab_ = tab_ ;
  }
  else 
    ptab_ = tab_( tab_.col("POLNO") == pollist_[ipol] ) ;

  attach( ptab_ ) ;
}

void STGrid::initTable( uInt idx ) 
{
  tab_ = tableList_[idx] ;
  nrow_ = rows_[idx] ;
  updateChunkShape() ;
}

void STGrid::setData( Array<Complex> &gdata,
                      Array<Float> &gwgt )
{
  // 2011/12/20 TN
  // gwgt and data_ share storage
  LogIO os( LogOrigin("STGrid","setData",WHERE) ) ;
  double t0, t1 ;
  t0 = mathutil::gettimeofday_sec() ;
  uInt len = data_.nelements() ;
  Bool b1, b2, b3 ;
  const Complex *gdata_p = gdata.getStorage( b1 ) ;
  Float *gwgt_p = gwgt.getStorage( b2 ) ; // storage shared with data_
  uChar *gflg_p = flag_.getStorage( b3 ) ;
  for ( uInt i = 0 ; i < len ; i++ ) {
    if (gwgt_p[i] > 0.0) {
      gwgt_p[i] = (gdata_p[i]).real() / gwgt_p[i];
      gflg_p[i] = (uChar)0;
    }
    else {
      gflg_p[i] = (uChar)1;
    }
  }
  gdata.freeStorage( gdata_p, b1 ) ;
  data_.putStorage( gwgt_p, b2 ) ;
  flag_.putStorage( gflg_p, b3 ) ;
  t1 = mathutil::gettimeofday_sec() ;
  os << LogIO::DEBUGGING << "setData: elapsed time is " << t1-t0 << " sec." << LogIO::POST ; 
}

void STGrid::setupGrid() 
{
  Double xmin,xmax,ymin,ymax ;
  mapExtent( xmin, xmax, ymin, ymax ) ;
  
  setupGrid( nxUI_, nyUI_, cellxUI_, cellyUI_, 
             xmin, xmax, ymin, ymax, centerUI_ ) ;
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
  LogIO os( LogOrigin("STGrid","setupGrid",WHERE) ) ;
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
    double base = 0.5 * (xmin + xmax) ;
    int maxrotate = 1 ;
    int nelem = 2 * maxrotate + 1 ;
    double *sep = new double[nelem] ;
    for ( int i = 0 ; i < nelem ; i++ )
      sep[i] = abs(base - center_[0] - (i-maxrotate) * C::_2pi) ;
//     os << "sep[0]=" << sep[0] << endl  
//        << "sep[1]=" << sep[1] << endl
//        << "sep[2]=" << sep[2] << LogIO::POST ;
    int idx = 0 ;
    base = sep[0] ;
    int nrotate = 0 ;
    while ( idx < nelem ) {
      if ( base > sep[idx] ) {
        base = sep[idx] ;
        nrotate = idx ;
      }
      idx++ ;
    }
    delete sep ;
    nrotate -= maxrotate ;
//     os << "nrotate = " << nrotate << LogIO::POST ;
    center_[0] += nrotate * C::_2pi ;
  }
//   os << "xmin=" << xmin << LogIO::POST ;
//   os << "center_=" << center_ << LogIO::POST ;

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

  //Double wx = xmax - xmin ;
  //Double wy = ymax - ymin ;
  Double wx = max( abs(xmax-center_(0)), abs(xmin-center_(0)) ) * 2 ;
  Double wy = max( abs(ymax-center_(1)), abs(ymin-center_(1)) ) * 2 ;

  Quantum<Double> qcellx ;
  Quantum<Double> qcelly ;
  //cout << "nx_ = " << nx_ << ",  ny_ = " << ny_ << endl ;
  if ( cellx.size() != 0 && celly.size() != 0 ) {
    readQuantity( qcellx, cellx ) ;
    readQuantity( qcelly, celly ) ;
  }
  else if ( celly.size() != 0 ) {
    os << "Using celly to x-axis..." << LogIO::POST ;
    readQuantity( qcelly, celly ) ;
    qcellx = qcelly ;
  }
  else if ( cellx.size() != 0 ) {
    os << "Using cellx to y-axis..." << LogIO::POST ;
    readQuantity( qcellx, cellx ) ;
    qcelly = qcellx ;
  }
  else {
    if ( nx_ < 0 ) {
      os << "No user preference in grid setting. Using default..." << LogIO::POST ;
      readQuantity( qcellx, "1.0arcmin" ) ;
      qcelly = qcellx ;
    }
    else {
      if ( wx == 0.0 ) {
        os << "Using default spatial extent (10arcmin) in x" << LogIO::POST ;
        wx = 0.00290888 ;
      }
      if ( wy == 0.0 ) {
        os << "Using default spatial extent (10arcmin) in y" << LogIO::POST ;
        wy = 0.00290888 ;
      }
      if (nx_ > 1) {
	qcellx = Quantum<Double>(wx / (nx_ - 1) * cos(center_[1]), "rad");
      }
      else {
	qcellx = Quantum<Double>( 1.1f * wx / nx_ * cos(center_[1]), "rad" );
      }
      if (ny_ > 1) {
	qcelly = Quantum<Double>(wy / (ny_ - 1), "rad");
      }
      else {
	qcelly = Quantum<Double>( 1.1f * wy / ny_, "rad" );
      }
    }
  }
  cellx_ = qcellx.getValue( "rad" ) ;
  celly_ = qcelly.getValue( "rad" ) ;
  //os << "cellx_=" << cellx_ << ", celly_=" << celly_ << ", cos("<<center_(1)<<")=" << cos(center_(1)) << LogIO::POST ;
  if ( nx_ < 0 ) {
    if ( wx == 0.0 ) {
      os << "Using default spatial extent (10arcmin) in x" << LogIO::POST ;
      wx = 0.00290888 ;
    }
    if ( wy == 0.0 ) {
      os << "Using default spatial extent (10arcmin) in y" << LogIO::POST ;
      wy = 0.00290888 ;
    }
    nx_ = Int( ceil( wx/(cellx_/cos(center_[1])) ) ) ;
    ny_ = Int( ceil( wy/celly_ ) ) ;
  }

  // create DirectionCoordinate
  Matrix<Double> xform(2,2) ;
  xform = 0.0 ;
  xform.diagonal() = 1.0 ;
  dircoord_ = new DirectionCoordinate(MDirection::J2000,
                                      Projection( Projection::SIN ),
                                      center_[0], center_[1],
                                      -cellx_, celly_,
                                      xform,
                                      0.5*Double(nx_-1), 
                                      0.5*Double(ny_-1)) ;
}

void STGrid::mapExtent( Double &xmin, Double &xmax, 
                        Double &ymin, Double &ymax ) 
{
  //LogIO os( LogOrigin("STGrid","mapExtent",WHERE) ) ;
  directionCol_.attach( tableList_[0], "DIRECTION" ) ;
  Matrix<Double> direction = directionCol_.getColumn() ;
  //os << "dirCol.nrow() = " << dirCol.nrow() << LogIO::POST ;
  Vector<Double> ra( direction.row(0) ) ;
  mathutil::rotateRA( ra ) ;
  minMax( xmin, xmax, direction.row( 0 ) ) ;
  minMax( ymin, ymax, direction.row( 1 ) ) ;
  Double amin, amax, bmin, bmax ;
  for ( uInt i = 1 ; i < nfile_ ; i++ ) {
    directionCol_.attach( tableList_[i], "DIRECTION" ) ;
    direction.assign( directionCol_.getColumn() ) ;
    //os << "dirCol.nrow() = " << dirCol.nrow() << LogIO::POST ;
    // to make contiguous RA distribution (no 2pi jump)
    Vector<Double> ra( direction.row(0) ) ;
    mathutil::rotateRA( ra ) ;
    minMax( amin, amax, direction.row( 0 ) ) ;
    minMax( bmin, bmax, direction.row( 1 ) ) ;
    xmin = min( xmin, amin ) ;
    xmax = max( xmax, amax ) ;
    ymin = min( ymin, bmin ) ;
    ymax = max( ymax, bmax ) ;
  }
  //os << "(xmin,xmax)=(" << xmin << "," << xmax << ")" << LogIO::POST ; 
  //os << "(ymin,ymax)=(" << ymin << "," << ymax << ")" << LogIO::POST ; 
}

void STGrid::table( Table &tab, uInt i )
{
  if ( i < nfile_ )
    tab = Table( infileList_[i] ) ;
}

void STGrid::selectData()
{
  LogIO os( LogOrigin("STGrid","selectData",WHERE) ) ;    
  Int ifno = ifno_ ;
  tableList_.resize( nfile_ ) ;
  if ( ifno_ == -1 ) {
    //Table taborg( infileList_[0] ) ;
    Table taborg ;
    table( taborg, 0 ) ;
    ROScalarColumn<uInt> ifnoCol( taborg, "IFNO" ) ;
    ifno_ = ifnoCol( 0 ) ;
    os << LogIO::WARN
       << "IFNO is not given. Using default IFNO: " << ifno_ << LogIO::POST ;
  }
  for ( uInt i = 0 ; i < nfile_ ; i++ ) {
    //Table taborg( infileList_[i] ) ;
    Table taborg ;
    table( taborg, i ) ;
    TableExprNode node ;
    if ( ifno != -1 || isMultiIF( taborg ) ) {
      os << "apply selection on IFNO" << LogIO::POST ;
      node = taborg.col("IFNO") == ifno_ ;
    }
    if ( scanlist_.size() > 0 ) {
      os << "apply selection on SCANNO" << LogIO::POST ;
      node = node && taborg.col("SCANNO").in( scanlist_ ) ;
    }
    if ( node.isNull() ) {
      tableList_[i] = taborg ;
    }
    else {
      tableList_[i] = taborg( node ) ;
    }
    os << LogIO::DEBUGGING << "tableList_[" << i << "].nrow()=" << tableList_[i].nrow() << LogIO::POST ;
    if ( tableList_[i].nrow() == 0 ) {
      os << LogIO::SEVERE
         << "No corresponding rows for given selection: IFNO " << ifno_ ;
      if ( scanlist_.size() > 0 )
        os << " SCANNO " << scanlist_ ;
      os << LogIO::EXCEPTION ;
    }
  }
}

Bool STGrid::isMultiIF( Table &tab ) 
{
  ROScalarColumn<uInt> ifnoCol( tab, "IFNO" ) ;
  Vector<uInt> ifnos = ifnoCol.getColumn() ;
  return anyNE( ifnos, ifnos[0] ) ;
}

void STGrid::attach( Table &tab )
{
  // attach to table
  spectraCol_.attach( tab, "SPECTRA" ) ;
  flagtraCol_.attach( tab, "FLAGTRA" ) ;
  directionCol_.attach( tab, "DIRECTION" ) ;
  flagRowCol_.attach( tab, "FLAGROW" ) ;
  tsysCol_.attach( tab, "TSYS" ) ;
  intervalCol_.attach( tab, "INTERVAL" ) ;
}

Int STGrid::getDataChunk(
			 IPosition const &wshape,
			 IPosition const &vshape,
			 IPosition const &/* dshape */,
			 Array<Complex> &spectra,
			 Array<Double> &direction,
			 Array<Int> &flagtra,
			 Array<Int> &rflag,
			 Array<Float> &weight ) 
{
  LogIO os( LogOrigin("STGrid","getDataChunk",WHERE) ) ;

  Array<Float> spectraF_(wshape);
  Array<uChar> flagtraUC_(wshape);
  Array<uInt> rflagUI_(vshape);
  Int nrow = getDataChunk( spectraF_, direction, flagtraUC_, rflagUI_, weight ) ;
  if ( nrow < nchunk_ ) {
    spectra.resize( spectraF_.shape() ) ;
    flagtra.resize( flagtraUC_.shape() ) ;
    rflag.resize( rflagUI_.shape() ) ;
  }
  double t0, t1 ;
  t0 = mathutil::gettimeofday_sec() ;
  convertArray( spectra, spectraF_ ) ;
  toInt( flagtraUC_, flagtra ) ;
  toInt( rflagUI_, rflag ) ;
  t1 = mathutil::gettimeofday_sec() ;
  eToInt = t1 - t0 ;
  
  return nrow ;
}

#if 0
Int STGrid::getDataChunk( Array<Complex> &spectra,
                          Array<Double> &direction,
                          Array<Int> &flagtra,
                          Array<Int> &rflag,
                          Array<Float> &weight ) 
{
  LogIO os( LogOrigin("STGrid","getDataChunk",WHERE) ) ;
  Int nrow = getDataChunk( spectraF_, direction, flagtraUC_, rflagUI_, weight ) ;
  if ( nrow < nchunk_ ) {
    spectra.resize( spectraF_.shape() ) ;
    flagtra.resize( flagtraUC_.shape() ) ;
    rflag.resize( rflagUI_.shape() ) ;
  }
  double t0, t1 ;
  t0 = mathutil::gettimeofday_sec() ;
  convertArray( spectra, spectraF_ ) ;
  toInt( flagtraUC_, flagtra ) ;
  toInt( rflagUI_, rflag ) ;
  t1 = mathutil::gettimeofday_sec() ;
  eToInt = t1 - t0 ;
  
  return nrow ;
}
#endif

Int STGrid::getDataChunk( Array<Float> &spectra,
                          Array<Double> &direction,
                          Array<uChar> &flagtra,
                          Array<uInt> &rflag,
                          Array<Float> &weight ) 
{
  LogIO os( LogOrigin("STGrid","getDataChunk",WHERE) ) ;
  Int nrow = spectra.shape()[1] ;
  Int remainingRow = nrow_ - nprocessed_ ;
  if ( remainingRow < nrow ) {
    nrow = remainingRow ;
    IPosition mshape( 2, nchan_, nrow ) ;
    IPosition vshape( 1, nrow ) ;
    spectra.resize( mshape ) ;
    flagtra.resize( mshape ) ;
    direction.resize( IPosition(2,2,nrow) ) ;
    rflag.resize( vshape ) ;
    weight.resize( mshape ) ;
  }
  // 2011/12/22 TN
  // tsys shares its storage with weight
  Array<Float> tsys( weight ) ;
  Array<Double> tint( rflag.shape() ) ;

  Vector<uInt> rflagVec( rflag ) ;
  Vector<Double> tintVec( tint ) ;

  RefRows rows( nprocessed_, nprocessed_+nrow-1, 1 ) ;
  //os<<LogIO::DEBUGGING<<"nprocessed_="<<nprocessed_<<": rows.nrows()="<<rows.nrows()<<LogIO::POST ;
  spectraCol_.getColumnCells( rows, spectra ) ;
  flagtraCol_.getColumnCells( rows, flagtra ) ;
  directionCol_.getColumnCells( rows, direction ) ;
  // to make contiguous RA distribution (no 2pi jump)
  Vector<Double> v( Matrix<Double>(direction).row(0) ) ;
  mathutil::rotateRA( v ) ;
  flagRowCol_.getColumnCells( rows, rflagVec ) ;
  intervalCol_.getColumnCells( rows, tintVec ) ;
  Vector<Float> tsysTemp = tsysCol_( nprocessed_ ) ;
  if ( tsysTemp.nelements() == (uInt)nchan_ )
    tsysCol_.getColumnCells( rows, tsys ) ;
  else
    tsys = tsysTemp[0] ;

  double t0,t1 ;
  t0 = mathutil::gettimeofday_sec() ;
  getWeight( weight, tsys, tint ) ;
  t1 = mathutil::gettimeofday_sec() ;
  eGetWeight += t1-t0 ;

  nprocessed_ += nrow ;
  
  return nrow ;
}

void STGrid::setupArray() 
{
  LogIO os( LogOrigin("STGrid","setupArray",WHERE) ) ;
  ROScalarColumn<uInt> polnoCol( tableList_[0], "POLNO" ) ;
  Vector<uInt> pols = polnoCol.getColumn() ;
  //os << pols << LogIO::POST ;
  Vector<uInt> pollistOrg ;
  npolOrg_ = 0 ;
  uInt polno ;
  for ( uInt i = 0 ; i < polnoCol.nrow() ; i++ ) {
    //polno = polnoCol( i ) ; 
    polno = pols( i ) ; 
    if ( allNE( pollistOrg, polno ) ) {
      pollistOrg.resize( npolOrg_+1, True ) ;
      pollistOrg[npolOrg_] = polno ;
      npolOrg_++ ;
    }
  }
  if ( pollist_.size() == 0 )
    pollist_ = pollistOrg ;
  else {
    Vector<uInt> newlist ;
    uInt newsize = 0 ;
    for ( uInt i = 0 ; i < pollist_.size() ; i++ ) {
      if ( anyEQ( pollistOrg, pollist_[i] ) ) {
        newlist.resize( newsize+1, True ) ;
        newlist[newsize] = pollist_[i] ;
        newsize++ ;
      }
    }
    pollist_.assign( newlist ) ;
  }
  npol_ = pollist_.size() ;
  if ( npol_ == 0 ) {
    os << LogIO::SEVERE << "Empty pollist" << LogIO::EXCEPTION ;
  }
  rows_.resize( nfile_ ) ;
  for ( uInt i = 0 ; i < nfile_ ; i++ ) {
    rows_[i] = tableList_[i].nrow() / npolOrg_ ;
    //if ( nrow_ < rows_[i] ) 
    //  nrow_ = rows_[i] ;
  }
  flagtraCol_.attach( tableList_[0], "FLAGTRA" ) ;
  nchan_ = flagtraCol_( 0 ).nelements() ;
//   os << "npol_ = " << npol_ << "(" << pollist_ << ")" << endl 
//      << "nchan_ = " << nchan_ << endl 
//      << "nrow_ = " << nrow_ << LogIO::POST ;
}

void STGrid::getWeight( Array<Float> &w,
                              Array<Float> &tsys,
                              Array<Double> &tint ) 
{
  LogIO os( LogOrigin("STGrid","getWeight",WHERE) ) ;

  // 2011/12/22 TN
  // w (weight) and tsys share storage
  IPosition refShape = tsys.shape() ;
  Int nchan = refShape[0] ;
  Int nrow = refShape[1] ;
//   os << "nchan=" << nchan << ", nrow=" << nrow << LogIO::POST ;
//   os << "w.shape()=" << w.shape() << endl
//      << "tsys.shape()=" << tsys.shape() << endl
//      << "tint.shape()=" << tint.shape() << LogIO::POST ;

  // set weight
  if ( wtype_.compare( "UNIFORM" ) == 0 ) {
    w = 1.0 ;
  }
  else if ( wtype_.compare( "TINT" ) == 0 ) {
    Bool b0, b1 ;
    Float *w_p = w.getStorage( b0 ) ;
    Float *w0_p = w_p ;
    const Double *ti_p = tint.getStorage( b1 ) ;
    const Double *w1_p = ti_p ;
    for ( Int irow = 0 ; irow < nrow ; irow++ ) {
      for ( Int ichan = 0 ; ichan < nchan ; ichan++ ) {
        *w0_p = *w1_p ;
        w0_p++ ;
      }
      w1_p++ ;
    }
    w.putStorage( w_p, b0 ) ;
    tint.freeStorage( ti_p, b1 ) ;
  }
  else if ( wtype_.compare( "TSYS" ) == 0 ) {
    Bool b0 ;
    Float *w_p = w.getStorage( b0 ) ;
    Float *w0_p = w_p ;
    for ( Int irow = 0 ; irow < nrow ; irow++ ) {
      for ( Int ichan = 0 ; ichan < nchan ; ichan++ ) {
        Float temp = *w0_p ;
        *w0_p = 1.0 / ( temp * temp ) ;
        w0_p++ ;
      }
    }
    w.putStorage( w_p, b0 ) ;
  }
  else if ( wtype_.compare( "TINTSYS" ) == 0 ) {
    Bool b0, b1 ;
    Float *w_p = w.getStorage( b0 ) ;
    Float *w0_p = w_p ;
    const Double *ti_p = tint.getStorage( b1 ) ;
    const Double *w1_p = ti_p ;
    for ( Int irow = 0 ; irow < nrow ; irow++ ) {
      Float interval = *w1_p ;
      for ( Int ichan = 0 ; ichan < nchan ; ichan++ ) {
        Float temp = *w0_p ;
        *w0_p = interval / ( temp * temp ) ;
        w0_p++ ;
      }
      w1_p++ ;
    }
    w.putStorage( w_p, b0 ) ;
    tint.freeStorage( ti_p, b1 ) ;
  }
  else {
    //LogIO os( LogOrigin("STGrid", "getWeight", WHERE) ) ;
    //os << LogIO::WARN << "Unsupported weight type '" << wtype_ << "', apply UNIFORM weight" << LogIO::POST ;
    w = 1.0 ;
  }
}

void STGrid::toInt( Array<uChar> &u, Array<Int> &v ) 
{
  uInt len = u.nelements() ;
  Int *int_p = new Int[len] ;
  Bool deleteIt ;
  const uChar *data_p = u.getStorage( deleteIt ) ;
  Int *i_p = int_p ;
  const uChar *u_p = data_p ;
  for ( uInt i = 0 ; i < len ; i++ ) {
    *i_p = ( *u_p == 0 ) ? 0 : 1 ;
    i_p++ ;
    u_p++ ;
  }
  u.freeStorage( data_p, deleteIt ) ;
  v.takeStorage( u.shape(), int_p, TAKE_OVER ) ;
}

void STGrid::toInt( Array<uInt> &u, Array<Int> &v ) 
{
  uInt len = u.nelements() ;
  Int *int_p = new Int[len] ;
  Bool deleteIt ;
  const uInt *data_p = u.getStorage( deleteIt ) ;
  Int *i_p = int_p ;
  const uInt *u_p = data_p ;
  for ( uInt i = 0 ; i < len ; i++ ) {
    *i_p = ( *u_p == 0 ) ? 0 : 1 ;
    i_p++ ;
    u_p++ ;
  }
  u.freeStorage( data_p, deleteIt ) ;
  v.takeStorage( u.shape(), int_p, TAKE_OVER ) ;
}

void STGrid::toPixel( Array<Double> &world, Array<Double> &pixel )
{
  uInt nrow = world.shape()[1] ;
  Bool bw, bp ;
  Double *w_p = world.getStorage( bw ) ;
  Double *p_p = pixel.getStorage( bp ) ;
  Double *ww_p = w_p ;
  Double *wp_p = p_p ;
  IPosition vshape( 1, 2 ) ;
  Vector<Double> _world, _pixel ;
  for ( uInt i = 0 ; i < nrow ; i++ ) {
    _world.takeStorage( vshape, ww_p, SHARE ) ;
    _pixel.takeStorage( vshape, wp_p, SHARE ) ;
    dircoord_->toPixel( _pixel, _world ) ;
    ww_p += 2 ;
    wp_p += 2 ;
  }
  world.putStorage( w_p, bw ) ;
  pixel.putStorage( p_p, bp ) ;
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
#define grdgauss grdgauss_
#define grdjinc1 grdjinc1_
#endif
#if defined(USE_CASAPY)
extern "C" { 
  void grdsf(Double*, Double*);
  void grdgauss(Double*, Double*, Double*); 
  void grdjinc1(Double*, Double*, Int*, Double*);
}
#else
extern "C" {
  void grdsf(Double*, Double*);
}
void grdgauss(Double *hwhm, Double *val, Double *out)
{
  *out = exp(-log(2.0) * (*val / *hwhm) * (*val / *hwhm));
}
void grdjinc1(Double *c, Double *val, Int *normalize, Double *out)
{
  // Calculate J_1(x) using approximate formula
  Double x = C::pi * *val / *c;
  Double ax = fabs(x);
  Double ans;
  if ( ax < 8.0 ) {
    Double y = x * x;
    Double ans1 = x * (72362614232.0 + y * (-7895059235.0 
                       + y * (242396853.1 + y * (-2972611.439 
                       + y * (15704.48260 + y * (-30.16036606))))));
    Double ans2 = 144725228442.0 + y * (2300535178.0
                       + y * (18583304.74 + y * (99447.43394
                       + y * (376.9991397 + y * 1.0))));
    ans = ans1 / ans2;
  }
  else {
    Double z = 8.0 / ax;
    Double y = z * z;
    Double xx = ax - 2.356194491;
    Double ans1 = 1.0 + y * (0.183105e-2 + y * (-0.3516396496e-4
                      + y * (0.2457520174e-5 + y * (-0.240337019e-6))));
    Double ans2 = 0.04687499995 + y * (-0.2002690873e-3
                      + y * (0.8449199096e-5 + y * (-0.88228987e-6 
                      + y * (0.105787412e-6))));
    ans = sqrt(0.636619772 / ax) * (cos(xx) * ans1
                                    - z * sin(xx) * ans2);
    if (x < 0.0)
      ans = -ans;
  }
  
  // Then, calculate Jinc
  if (x == 0.0) {
    *out = 0.5;
  }
  else {
    *out = ans / x;
  }

  if (*normalize == 1)
    *out = *out / 0.5;  
}
#endif
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

void STGrid::gaussFunc( Vector<Float> &convFunc, Double hwhm, Double truncate ) 
{
  convFunc = 0.0 ;
  Int len = (Int)(truncate*Double(convSampling_)+0.5);
  Double out, val;
  for ( Int i = 0 ; i < len ; i++ ) {
    val = Double(i) / Double(convSampling_) ;
    grdgauss(&hwhm, &val, &out);
    convFunc(i) = out;
  }
}

void STGrid::gjincFunc( Vector<Float> &convFunc, Double hwhm, Double c, Double truncate )
{
  convFunc = 0.0;
  Double out1, out2, val;
  Int normalize = 1;
  if  (truncate >= 0.0) { 
    Int len = (Int)(truncate*Double(convSampling_)+0.5);
    for (Int i = 0 ; i < len ; i++) {
      val = Double(i) / Double(convSampling_);
      grdgauss(&hwhm, &val, &out1);
      grdjinc1(&c, &val, &normalize, &out2);
      convFunc(i) = out1 * out2;
    }
  }
  else {
    Int len = convFunc.nelements();
    for (Int i = 0 ; i < len ; i++) {
      val = Double(i) / Double(convSampling_);
      grdjinc1(&c, &val, &normalize, &out2);
      if (out2 <= 0.0) {
        LogIO os(LogOrigin("STGrid","gjincFunc",WHERE));
        os << LogIO::DEBUG1 << "convFunc is automatically truncated at radius " << val << LogIO::POST;
        break;
      }
      grdgauss(&hwhm, &val, &out1);
      convFunc(i) = out1 * out2;
    }
  }
}

void STGrid::pbFunc( Vector<Float> &convFunc ) 
{
  convFunc = 0.0 ;
}

vector<float> STGrid::getConvFunc()
{
  LogIO os(LogOrigin("STGrid","getConvFunc",WHERE));
  Vector<Float> convFunc;
  vector<float> out;

  if (cellx_ <= 0.0 || celly_ <= 0.0) {
    selectData();
    setupGrid();
  }

  if (convType_ == "BOX" || convType_ == "SF") {
    setConvFunc(convFunc);
  }
  else if (convType_ == "GAUSS") {
    Quantum<Double> q1,q2;
    readQuantity(q1,gwidth_);
    readQuantity(q2,truncate_);
//     if (celly_ <= 0.0 
//         && ((!q1.getUnit().empty()&&q1.getUnit()!="pixel") ||
//             (!q2.getUnit().empty()&&q2.getUnit()!="pixel"))) {
//       throw AipsError("You have to call defineImage to get correct convFunc");
//     }
    setConvFunc(convFunc);
  }
  else if (convType_ == "GJINC") {
    Quantum<Double> q1,q2,q3;
    readQuantity(q1,gwidth_);
    readQuantity(q2,truncate_);
    readQuantity(q3,jwidth_);
//     if (celly_ <= 0.0 
//         && ((!q1.getUnit().empty()&&q1.getUnit()!="pixel") ||
//             (!q2.getUnit().empty()&&q2.getUnit()!="pixel") ||
//             (!q3.getUnit().empty()&&q3.getUnit()!="pixel"))) {
//       throw AipsError("You have to call defineImage to get correct convFunc");
//     }
    setConvFunc(convFunc);
  }
  else if (convType_ == "PB") {
    throw AipsError("Grid function PB is not available");
  }
  else {
    throw AipsError("Unknown grid function: "+convType_);
  }

  convFunc.tovector(out);
  return out;
}

void STGrid::setConvFunc( Vector<Float> &convFunc )
{
  LogIO os(LogOrigin("STGrid","setConvFunc",WHERE));
  convSupport_ = userSupport_ ;
  if ( convType_ == "BOX" ) {
    if ( convSupport_ < 0 )
      convSupport_ = 0 ;
    Int convSize = convSampling_ * ( 2 * convSupport_ + 2 )  ;
    convFunc.resize( convSize ) ;
    boxFunc( convFunc, convSize ) ;
    os << LogIO::DEBUGGING
       << "convType_ = " << convType_ << endl
       << "convSupport_ = " << convSupport_ << LogIO::POST;
  }
  else if ( convType_ == "SF" ) {
    if ( convSupport_ < 0 )
      convSupport_ = 3 ;
    Int convSize = convSampling_ * ( 2 * convSupport_ + 2 )  ;
    convFunc.resize( convSize ) ;
    spheroidalFunc( convFunc ) ;
    os << LogIO::DEBUGGING
       << "convType_ = " << convType_ << endl
       << "convSupport_ = " << convSupport_ << LogIO::POST;
  }
  else if ( convType_ == "GAUSS" ) {
    // determine pixel gwidth
    // default is HWHM corresponding to b = 1.0 (Mangum et al. 2007)
    Double pixelGW = -1.0;
    Quantum<Double> q ;
    if (!gwidth_.empty()) {
      readQuantity( q, gwidth_ );
      if ( q.getUnit().empty() || q.getUnit()=="pixel" ) {
        pixelGW = q.getValue();
      }
      else {
        pixelGW = q.getValue("rad")/celly_;
      }
    }
    pixelGW = (pixelGW >= 0.0) ? pixelGW : sqrt(log(2.0)); 
    if (pixelGW < 0.0) {
      os << LogIO::SEVERE
         << "Negative width is specified for gaussian" << LogIO::EXCEPTION;
    }
    // determine truncation radius
    // default is 3 * HWHM
    Double truncate = -1.0;
    if (!truncate_.empty()) {
      readQuantity( q, truncate_ );
      if ( q.getUnit().empty() || q.getUnit()=="pixel" ) {
        truncate = q.getValue();
      }
      else {
        truncate = q.getValue("rad")/celly_;
      }
    }      
    //convSupport_ = (Int)(truncate+0.5);
    truncate = (truncate >= 0.0) ? truncate : 3.0 * pixelGW;
    convSupport_ = Int(truncate);
    convSupport_ += (((truncate-(Double)convSupport_) > 0.0) ? 1 : 0);
    Int convSize = convSampling_ * ( 2*convSupport_ + 2 ) ;
    convFunc.resize( convSize ) ;
    gaussFunc( convFunc, pixelGW, truncate ) ;
    os << LogIO::DEBUGGING
       << "convType_ = " << convType_ << endl
       << "convSupport_ = " << convSupport_ << endl
       << "truncate_ = " << truncate << "pixel" << endl
       << "gwidth_ = " << pixelGW << "pixel" << LogIO::POST;
  }
  else if ( convType_ == "GJINC" ) {
    // determine pixel gwidth
    // default is HWHM corresponding to b = 2.52 (Mangum et al. 2007)
    Double pixelGW = -1.0;
    Quantum<Double> q ;
    if (!gwidth_.empty()) {
      readQuantity( q, gwidth_ );
      if ( q.getUnit().empty() || q.getUnit()=="pixel" ) {
        pixelGW = q.getValue();
      }
      else {
        pixelGW = q.getValue("rad")/celly_;
      }
    }
    pixelGW = (pixelGW >= 0.0) ? pixelGW : sqrt(log(2.0)) * 2.52; 
    if (pixelGW < 0.0) {
      os << LogIO::SEVERE
         << "Negative width is specified for gaussian" << LogIO::EXCEPTION;
    }
    // determine pixel c
    // default is c = 1.55 (Mangum et al. 2007)
    Double pixelJW = -1.0;
    if (!jwidth_.empty()) {
      readQuantity( q, jwidth_ );
      if ( q.getUnit().empty() || q.getUnit()=="pixel" ) {
        pixelJW = q.getValue();
      }
      else {
        pixelJW = q.getValue("rad")/celly_;
      }
    }
    pixelJW = (pixelJW >= 0.0) ? pixelJW : 1.55; 
    if (pixelJW < 0.0) {
      os << LogIO::SEVERE
         << "Negative width is specified for jinc" << LogIO::EXCEPTION;
    }
    // determine truncation radius
    // default is -1.0 (truncate at first null)
    Double truncate = -1.0;
    if (!truncate_.empty()) {
      readQuantity( q, truncate_ );
      if ( q.getUnit().empty() || q.getUnit()=="pixel" ) {
        truncate = q.getValue();
      }
      else {
        truncate = q.getValue("rad")/celly_;
      }
    }      
    //convSupport_ = (truncate >= 0.0) ? (Int)(truncate+0.5) : (Int)(2*pixelJW+0.5);
    Double convSupportF = (truncate >= 0.0) ? truncate : (2*pixelJW);
    convSupport_ = (Int)convSupportF;
    convSupport_ += (((convSupportF-(Double)convSupport_) > 0.0) ? 1 : 0);
    Int convSize = convSampling_ * ( 2*convSupport_ + 2 ) ;
    convFunc.resize( convSize ) ;
    gjincFunc( convFunc, pixelGW, pixelJW, truncate ) ;
    os << LogIO::DEBUGGING
       << "convType_ = " << convType_ << endl
       << "convSupport_ = " << convSupport_ << endl
       << "truncate_ = " << truncate << "pixel" << endl
       << "gwidth_ = " << pixelGW << "pixel" << endl
       << "jwidth_ = " << pixelJW << "pixel" << LogIO::POST;
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
  LogIO os( LogOrigin("STGrid", "saveData", WHERE) ) ;
  double t0, t1 ;
  t0 = mathutil::gettimeofday_sec() ;

  //Int polno = 0 ;
  String outfile_ ;
  if ( outfile.size() == 0 ) {
    if ( infileList_[0].lastchar() == '/' ) {
      outfile_ = infileList_[0].substr( 0, infileList_[0].size()-1 ) ;
    }
    else {
      outfile_ = infileList_[0] ;
    }
    outfile_ += ".grid" ;
  }
  else {
    outfile_ = outfile ;
  }
  Table tab ;
  prepareTable( tab, outfile_ ) ;
  fillTable( tab ) ;

  t1 = mathutil::gettimeofday_sec() ;
  os << LogIO::DEBUGGING << "saveData: elapsed time is " << t1-t0 << " sec." << LogIO::POST ; 

  return outfile_ ;
}

void STGrid::prepareTable( Table &tab, String &name ) 
{
  Table t( infileList_[0], Table::Old ) ;
  t.deepCopy( name, Table::New, False, t.endianFormat(), True ) ;
  tab = Table( name, Table::Update ) ;
  // 2012/02/13 TN
  // explicitly copy subtables since no rows including subtables are 
  // copied by Table::deepCopy with noRows=True
  //TableCopy::copySubTables( tab, t ) ;
  const TableRecord &inrec = t.keywordSet();
  TableRecord &outrec = tab.rwKeywordSet();
  for (uInt i = 0 ; i < inrec.nfields() ; i++) {
    if (inrec.type(i) == TpTable) {
      String name = inrec.name(i);
      Table intable = inrec.asTable(name);
      Table outtable = outrec.asTable(name);
      TableCopy::copyRows(outtable, intable);
    }
  }
}

void STGrid::fillTable( Table &tab )
{
  //IPosition dshape = data_.shape() ;
  Int nrow = nx_ * ny_ * npol_ ;
  tab.rwKeywordSet().define( "nPol", npol_ ) ;
  tab.addRow( nrow ) ;
  Vector<Double> cpix( 2 ) ;
  cpix(0) = Double( nx_ - 1 ) * 0.5 ;
  cpix(1) = Double( ny_ - 1 ) * 0.5 ;
  Vector<Double> dir( 2 ) ;
  Vector<Double> pix( 2 );
  ArrayColumn<Double> directionCol( tab, "DIRECTION" ) ;
  ArrayColumn<Float> spectraCol( tab, "SPECTRA" ) ;
  ArrayColumn<uChar> flagtraCol( tab, "FLAGTRA" ) ;
  ScalarColumn<uInt> flagRowCol( tab, "FLAGROW" );
  ScalarColumn<uInt> polnoCol( tab, "POLNO" ) ;
  ScalarColumn<uInt> scannoCol( tab, "SCANNO" ) ;
  Int irow = 0 ;
  Vector<Float> sp( nchan_ ) ;
  Vector<uChar> flag( nchan_ ) ;
  Bool bsp, bdata, bflag ;
  const Float *data_p = data_.getStorage( bdata ) ;
  const uChar *flag_p = flag_.getStorage( bflag ) ;
  Float *wsp_p, *sp_p ;
  const Float *wdata_p = data_p ;
  const uChar *wflag_p = flag_p ;
  long step = nx_ * ny_ * npol_ ;
  long offset ;
  uInt scanno = 0 ;
  uChar rflag;
  for ( Int iy = 0 ; iy < ny_ ; iy++ ) {
    pix(1) = (Double)(iy);
    for ( Int ix = 0 ; ix < nx_ ; ix++ ) {
      pix(0) = (Double)(ix);
      dircoord_->toWorld(dir,pix);
      //os << "dir[" << ix << "," << iy << "]=" << dir << LogIO::POST;
      for ( Int ipol = 0 ; ipol < npol_ ; ipol++ ) {
        offset = ix + nx_ * (iy + ipol * ny_) ;
        //os << "offset = " << offset << LogIO::POST ;
        sp_p = sp.getStorage( bsp ) ;
        wsp_p = sp_p ;
        wdata_p = data_p + offset ;
        wflag_p = flag_p + offset ;
	rflag = ~0 ; //11111111
        for ( Int ichan = 0 ; ichan < nchan_ ; ichan++ ) {
          *wsp_p = *wdata_p ;
          wsp_p++ ;
          wdata_p += step ;
	  flag[ichan] = *wflag_p ;
	  rflag &= flag[ichan] ;
	  wflag_p += step ;
        }
        sp.putStorage( sp_p, bsp ) ;
        spectraCol.put( irow, sp ) ;
        flagtraCol.put( irow, flag ) ;
        flagRowCol.put( irow, ((rflag>0) ? 1 : 0) ) ;
        directionCol.put( irow, dir ) ;
        polnoCol.put( irow, pollist_[ipol] ) ;
        scannoCol.put( irow, scanno ) ;
        irow++ ;
      }
      scanno++ ;
    }
  }
  data_.freeStorage( data_p, bdata ) ;
  flag_.freeStorage( flag_p, bflag ) ;

  fillMainColumns( tab ) ;
}

void STGrid::fillMainColumns( Table &tab ) 
{
  // values for fill
  //Table t( infileList_[0], Table::Old ) ;
  Table t ;
  table( t, 0 ) ;
  Table tsel = t( t.col( "IFNO" ) == (uInt)ifno_, 1 ) ;
  ROTableRow row( tsel ) ;
  row.get( 0 ) ;
  const TableRecord &rec = row.record() ;
  uInt freqId = rec.asuInt( "FREQ_ID" ) ;
  uInt molId = rec.asuInt( "MOLECULE_ID" ) ;
  uInt tcalId = rec.asuInt( "TCAL_ID" ) ;
  uInt focusId = rec.asuInt( "FOCUS_ID" ) ;
  uInt weatherId = rec.asuInt( "WEATHER_ID" ) ;
  String srcname = rec.asString( "SRCNAME" ) ;
  String fieldname = rec.asString( "FIELDNAME" ) ;
  Vector<Float> defaultTsys( 1, 1.0 ) ;
  // @todo how to set flagtra for gridded spectra?
  Vector<uChar> flagtra = rec.asArrayuChar( "FLAGTRA" ) ;
  flagtra = (uChar)0 ;
  Float opacity = rec.asFloat( "OPACITY" ) ;
  Double srcvel = rec.asDouble( "SRCVELOCITY" ) ;
  Vector<Double> srcpm = rec.asArrayDouble( "SRCPROPERMOTION" ) ;
  Vector<Double> srcdir = rec.asArrayDouble( "SRCDIRECTION" ) ;
  Vector<Double> scanrate = rec.asArrayDouble( "SCANRATE" ) ;
  Double time = rec.asDouble( "TIME" ) ;
  Double interval = rec.asDouble( "INTERVAL" ) ;

  // fill columns
  Int nrow = tab.nrow() ;
  ScalarColumn<uInt> ifnoCol( tab, "IFNO" ) ;
  ScalarColumn<uInt> beamnoCol(tab, "BEAMNO");
  ScalarColumn<uInt> freqIdCol( tab, "FREQ_ID" ) ;
  ScalarColumn<uInt> molIdCol( tab, "MOLECULE_ID" ) ;
  ScalarColumn<uInt> tcalidCol( tab, "TCAL_ID" ) ;
  ScalarColumn<Int> fitidCol( tab, "FIT_ID" ) ;
  ScalarColumn<uInt> focusidCol( tab, "FOCUS_ID" ) ;
  ScalarColumn<uInt> weatheridCol( tab, "WEATHER_ID" ) ;
  ArrayColumn<uChar> flagtraCol( tab, "FLAGTRA" ) ;
  ScalarColumn<uInt> rflagCol( tab, "FLAGROW" ) ;
  ArrayColumn<Float> tsysCol( tab, "TSYS" ) ;
  ScalarColumn<String> srcnameCol( tab, "SRCNAME" ) ;
  ScalarColumn<String> fieldnameCol( tab, "FIELDNAME" ) ;
  ScalarColumn<Int> srctypeCol( tab, "SRCTYPE" ) ;
  ScalarColumn<Float> opacityCol( tab, "OPACITY" ) ;
  ScalarColumn<Double> srcvelCol( tab, "SRCVELOCITY" ) ;
  ArrayColumn<Double> srcpmCol( tab, "SRCPROPERMOTION" ) ;
  ArrayColumn<Double> srcdirCol( tab, "SRCDIRECTION" ) ;
  ArrayColumn<Double> scanrateCol( tab, "SCANRATE" ) ;
  ScalarColumn<Double> timeCol( tab, "TIME" ) ;
  ScalarColumn<Double> intervalCol( tab, "INTERVAL" ) ;
  for ( Int i = 0 ; i < nrow ; i++ ) {
    ifnoCol.put( i, (uInt)ifno_ ) ;
    beamnoCol.put(i, 0);
    freqIdCol.put( i, freqId ) ;
    molIdCol.put( i, molId ) ;
    tcalidCol.put( i, tcalId ) ;
    fitidCol.put( i, -1 ) ;
    focusidCol.put( i, focusId ) ;
    weatheridCol.put( i, weatherId ) ;
    //flagtraCol.put( i, flagtra ) ;
    //rflagCol.put( i, 0 ) ;
    tsysCol.put( i, defaultTsys ) ;
    srcnameCol.put( i, srcname ) ;
    fieldnameCol.put( i, fieldname ) ;
    srctypeCol.put( i, (Int)SrcType::PSON ) ;
    opacityCol.put( i, opacity ) ;
    srcvelCol.put( i, srcvel ) ;
    srcpmCol.put( i, srcpm ) ;
    srcdirCol.put( i, srcdir ) ;
    scanrateCol.put( i, scanrate ) ;
    timeCol.put( i, time ) ;
    intervalCol.put( i, interval ) ;
    if ( (i + 1) % npol_ == 0 ) {
      time += interval / 86400.0;
    }
  }
}

vector<int> STGrid::getResultantMapSize()
{
  vector<int> r(2);
  r[0] = nx_;
  r[1] = ny_;
  return r;
}

vector<double> STGrid::getResultantCellSize()
{
  vector<double> r(2);
  r[0] = cellx_;
  r[1] = celly_;
  return r;
}

// STGrid2
STGrid2::STGrid2()
  : STGrid()
{
}

STGrid2::STGrid2( const ScantableWrapper &s )
  : STGrid()
{
  setScantable( s ) ;
}

STGrid2::STGrid2( const vector<ScantableWrapper> &v )
  : STGrid()
{
  setScantableList( v ) ;
}

void STGrid2::setScantable( const ScantableWrapper &s )
{
  nfile_ = 1 ;
  dataList_.resize( nfile_ ) ;
  dataList_[0] = s ;
  infileList_.resize( nfile_ ) ;
  infileList_[0] = s.getCP()->table().tableName() ;
}

void STGrid2::setScantableList( const vector<ScantableWrapper> &v )
{
  nfile_ = v.size() ;
  dataList_.resize( nfile_ ) ;
  infileList_.resize( nfile_ ) ;
  for ( uInt i = 0 ; i < nfile_ ; i++ ) {
    dataList_[i] = v[i] ;
    infileList_[i] = v[i].getCP()->table().tableName() ;
  }
}

ScantableWrapper STGrid2::getResultAsScantable( int tp )
{
  ScantableWrapper sw( tp ) ;
  CountedPtr<Scantable> s = sw.getCP() ;
  s->setHeader( dataList_[0].getCP()->getHeader() ) ;
  Table tout, tin ;
  String subt[] = { "FREQUENCIES", "FOCUS", "WEATHER", 
                    "TCAL", "MOLECULES", "HISTORY", "FIT" } ;
  for ( uInt i = 0 ; i < 7 ; i++ ) {
    tout = s->table().rwKeywordSet().asTable(subt[i]) ;
    tin = dataList_[0].getCP()->table().rwKeywordSet().asTable(subt[i]) ;
    TableCopy::copyRows( tout, tin ) ;
    tout.rwKeywordSet() = tin.rwKeywordSet();
  }
  fillTable( s->table() ) ;
  return sw ;
}

void STGrid2::table( Table &tab, uInt i ) 
{
  if ( i < nfile_ )
    tab = dataList_[i].getCP()->table() ;
}

}
