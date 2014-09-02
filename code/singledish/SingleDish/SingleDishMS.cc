#include <iostream>

#include <libsakura/sakura.h>
//#include <libsakura/config.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Utilities/Assert.h>
#include <casa/Arrays/ArrayMath.h>

#include <ms/MeasurementSets/MSSelectionTools.h>
#include <synthesis/MSVis/VisibilityIterator2.h>
#include <synthesis/MSVis/VisSetUtil.h>

#include <casa_sakura/SakuraUtils.h>
#include <singledish/SingleDish/SingleDishMS.h>

namespace casa {

SingleDishMS::SingleDishMS()
  : msname_(""), ms_(0), mssel_(0),
    in_column_(MS::UNDEFINED_COLUMN), out_column_(MS::UNDEFINED_COLUMN)
{
}


SingleDishMS::SingleDishMS(string const& ms_name)
  : msname_(ms_name), ms_(0), mssel_(0),
    in_column_(MS::UNDEFINED_COLUMN), out_column_(MS::UNDEFINED_COLUMN)
{
  // Make a MeasurementSet object for the disk-base MeasurementSet
  String lems(ms_name);
  ms_ = new MeasurementSet(lems, TableLock(TableLock::AutoNoReadLocking), 
			    Table::Update);
  AlwaysAssert(ms_, AipsError);
}


SingleDishMS::SingleDishMS(MeasurementSet &ms)
  : msname_(""), ms_(0), mssel_(0),
    in_column_(MS::UNDEFINED_COLUMN), out_column_(MS::UNDEFINED_COLUMN)
{
  msname_ = static_cast<string>(ms.tableName());
  ms_ = new MeasurementSet(ms);
  AlwaysAssert(ms_, AipsError);
}

SingleDishMS::SingleDishMS(SingleDishMS const &other)
  : msname_(""), ms_(0),
    in_column_(MS::UNDEFINED_COLUMN), out_column_(MS::UNDEFINED_COLUMN)
{
  ms_ = new MeasurementSet(*other.ms_);
  if(other.mssel_) {
    mssel_ = new MeasurementSet(*other.mssel_);
  }
}

SingleDishMS &SingleDishMS::operator=(SingleDishMS const &other)
{
  msname_ = "";
  if (ms_ && this != &other) {
    *ms_ = *(other.ms_);
  }
  if (mssel_ && this != &other && other.mssel_) {
    *mssel_ = *(other.mssel_);
  }
  return *this;
}

SingleDishMS::~SingleDishMS()
{
  if (ms_) {
    ms_->relinquishAutoLocks();
    ms_->unlock();
    delete ms_;
  }
  ms_ = 0;
  if (mssel_) {
    mssel_->relinquishAutoLocks();
    mssel_->unlock();
    delete mssel_;
  }
  mssel_ = 0;
  msname_ = "";
}

void SingleDishMS::check_ms()
{
  AlwaysAssert(ms_, AipsError);
}

bool SingleDishMS::close()
{
  LogIO os(LogOrigin("SingleDishMS", "close()", WHERE));
  os << "Closing MeasurementSet and detaching from SingleDishMS"
     << LogIO::POST;

  ms_->unlock();
  if(mssel_) {
    mssel_->unlock();
    delete mssel_;
    mssel_ = 0;
  }
  if(ms_) delete ms_; ms_ = 0;
  msname_ = "";

  return True;
}

void SingleDishMS::prepare_for_process(string const& in_column_name,string const& out_ms_name)
{
  // make sure MS is set
  check_ms();
  // define a column to read data from
  if (in_column_name == "float_data"){
    if (!set_column(MS::FLOAT_DATA, in_column_))
      throw(AipsError("Input MS does not have FLOAT_DATA column"));
    cout << "Reading data from FLOAT_DATA column" << endl;
  } else if (in_column_name == "corrected_data") {
    if (!set_column(MS::CORRECTED_DATA, in_column_))
      throw(AipsError("Input MS does not have CORRECTED_DATA column"));
    cout << "Reading data from CORRECTED_DATA column" << endl;
  } else if (in_column_name == "data") {
    if (!set_column(MS::DATA, in_column_))
      throw(AipsError("Input MS does not have DATA column"));
    cout << "Reading data from DATA column" << endl;
  } else if (in_column_name != "") {
    throw(AipsError("Invalid data column name"));
  }
  if (set_column(MS::FLOAT_DATA, in_column_)) {
    cout << "Reading data from FLOAT_DATA column" << endl;
  } else if (set_column(MS::DATA, in_column_)) {
    cout << "Reading data from DATA column" << endl;
  } else {
    throw(AipsError("Unable to find input data column in input MS"));
  }
  // define a column to save data to
  if (out_ms_name != "") { // Creating a new MS.
    if (in_column_ == MS::CORRECTED_DATA)
      out_column_ = MS::DATA;
    else // DATA or FLOAT_DATA
      out_column_ = in_column_;
    cout << "Output is stored in a new MS" << endl;
  } else { // Storing results to input MS
    out_column_ = MS::CORRECTED_DATA;
    cout << "Output data to CORRECTED_DATA column" << endl;
  }
  //  create output CORRECTED_DATA if not exists.
  if (out_column_==MS::CORRECTED_DATA &&
      !ms_->tableDesc().isColumn("CORRECTED_DATA")) {
    Bool addModel(False), addScratch(True), alsoinit(True), compress(False);
    VisSetUtil::addScrCols(*ms_,addModel,addScratch,alsoinit,compress);
  }
  // handle no selection
  if (mssel_==0)
    mssel_ = new MeasurementSet(*ms_);
}

bool SingleDishMS::set_column(MSMainEnums::PredefinedColumns const &in,
			      MSMainEnums::PredefinedColumns &out){
  if (ms_!=0 && ms_->tableDesc().isColumn(MS::columnName(in))) {
    out = in;
    return true;
  } else {
    out = MS::UNDEFINED_COLUMN;
    return false;
  }
}


void SingleDishMS::reset_selection()
{
  if (mssel_) {
    delete mssel_;
    mssel_=0;
  };
}

// void SingleDishMS::scale(float const factor)
// {
//     Bool addModel(False), addScratch(True), alsoinit(True), compress(False);
//     VisSetUtil::addScrCols(*ms_,addModel,addScratch,alsoinit,compress);
//     //mssel_ = new MeasurementSet(*ms_);
//     Block<Int> columns(1);
//     columns[0] = MS::DATA_DESC_ID;
//     vi::SortColumns sc(columns,False);
//     vi::VisibilityIterator2 vi(*mssel_,sc,True);
//     vi::VisBuffer2 *vb = vi.getVisBuffer();
 
//     for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {
//       for (vi.origin(); vi.more(); vi.next()) {
// 	size_t const num_chan = static_cast<size_t>(vb->nChannels());
// 	size_t const num_pol = static_cast<size_t>(vb->nCorrelations());
// 	size_t const num_row = static_cast<size_t>(vb->nRows());
// 	Cube<Float> data_chunk(num_pol,num_chan,num_row);
// 	data_chunk = vb->visCubeFloat();
// 	Cube<Complex> cdata_chunk(data_chunk.shape(),Complex(3.0));
// 	vb->setVisCubeCorrected(cdata_chunk);
// 	vb->writeChangesBack();
//       } // end of vi loop
//     } // end of chunk loop
// }

void SingleDishMS::scale(float const factor)
{
  cout << "Got scaling factor = " << factor << endl;
  // in_ms = out_ms
  // in_column = [FLOAT_DATA|DATA] (auto-select), out_column=CORRECTED_DATA
  // no iteration is necessary for the processing.
  // procedure
  // 1. iterate over MS
  // 2. pick single spectrum from in_column (this is not actually necessary for simple scaling but for exibision purpose)
  // 3. multiply a scaling factor to each spectrum
  // 4. put single spectrum (or a block of spectra) to out_column
  prepare_for_process();
  Block<Int> columns(1);
  columns[0] = MS::DATA_DESC_ID;
  vi::SortColumns sc(columns,False);
  vi::VisibilityIterator2 vi(*mssel_,sc,True);
  vi::VisBuffer2 *vb = vi.getVisBuffer();

  for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {
    for (vi.origin(); vi.more(); vi.next()) {
      size_t const num_chan = static_cast<size_t>(vb->nChannels());
      size_t const num_pol = static_cast<size_t>(vb->nCorrelations());
      size_t const num_row = static_cast<size_t>(vb->nRows());
      Cube<Float> data_chunk(num_pol,num_chan,num_row);
      Matrix<Float> data_row(num_pol,num_chan);
      float spectrum[num_chan];
      // get a data cube (npol*nchan*nrow) from VisBuffer
      get_data_cube_float(*vb, data_chunk);
      // loop over MS rows
      for (size_t irow=0; irow < num_row; ++irow) {
	// loop over polarization
	for (size_t ipol=0; ipol < num_pol; ++ipol) {
	  // get a spectrum from data cube
	  get_spectrum_from_cube(data_chunk, irow, ipol, num_chan, spectrum);

	  // actual execution of single spectrum
	  do_scale(factor, num_chan, spectrum);

	  // set back a spectrum to data cube
	  set_spectrum_to_cube(data_chunk, irow, ipol, num_chan, spectrum);
	} // end of polarization loop
      } // end of MS row loop
      // write back data cube to VisBuffer
      set_data_cube_float(*vb, data_chunk);
      vb->writeChangesBack();
    } // end of vi loop
  } // end of chunk loop
}

void SingleDishMS::do_scale(float const factor,
			    size_t const num_data, float *data)
{
  for (size_t i=0; i < num_data; ++i) 
    data[i] *= factor;
}


void SingleDishMS::get_spectrum_from_cube(Cube<Float> &data_cube,
					  size_t const row,
					  size_t const plane,
					  size_t const num_data,
					  float *out_data)
{
  for (size_t i=0; i < num_data; ++i) 
    out_data[i] = static_cast<float>(data_cube(plane, i, row));
}


void SingleDishMS::set_spectrum_to_cube(Cube<Float> &data_cube,
					  size_t const row,
					  size_t const plane,
					  size_t const num_data,
					  float *in_data)
{
  for (size_t i=0; i < num_data; ++i) 
    data_cube(plane, i, row) = static_cast<Float>(in_data[i]);
}


void SingleDishMS::get_data_cube_float(vi::VisBuffer2 const &vb,
				       Cube<Float> &data_cube)
{
  if (in_column_==MS::FLOAT_DATA) {
    data_cube = vb.visCubeFloat();
  } else { //need to convert Complex cube to Float
    Cube<Complex> cdata_cube(data_cube.shape());
    if (in_column_==MS::DATA) {
      cdata_cube = vb.visCube();
    } else {//MS::CORRECTED_DATA
      cdata_cube = vb.visCubeCorrected();
    }
    // convert Complext to Float
    convertArrayC2F(data_cube, cdata_cube);
  }
  cout << "Got data cube shape = " << data_cube.shape() << endl;
}

void SingleDishMS::convertArrayC2F(Array<Float> &to,
				   Array<Complex> const &from){
    if (to.nelements() == 0 && from.nelements() == 0) {
        return;
    }
    if (to.shape() != from.shape()) {
        throw(ArrayConformanceError("Array shape differs"));
    }
    Array<Complex>::const_iterator endFrom = from.end();
    Array<Complex>::const_iterator iterFrom = from.begin();
    for (Array<Float>::iterator iterTo = to.begin();
	 iterFrom != endFrom;
	 ++iterFrom, ++iterTo) {
      *iterTo = iterFrom->real();
    }
}

void SingleDishMS::set_data_cube_float(vi::VisBuffer2 &vb,
				       Cube<Float> const &data_cube)
{
  if (out_column_==MS::FLOAT_DATA) {
    cout << "saving to FLOAT_DATA" << endl;
    vb.setVisCubeFloat(data_cube);
  } else { //need to convert Float cube to Complex
    Cube<Complex> cdata_cube(data_cube.shape());
    cout << "converting float cube to complex cube" << endl;
    convertArray(cdata_cube,data_cube);
    if (out_column_==MS::DATA) {
      cout << "saving to DATA" << endl;
      vb.setVisCube(cdata_cube);
    } else {//MS::CORRECTED_DATA
      if (!ms_->tableDesc().isColumn("CORRECTED_DATA"))
	throw(AipsError("CORRECTED_DATA column unexpectedly absent. Cannot correct."));
      cout << "saving to CORRECTED_DATA" << endl;
      cout << "data cube shape = " << cdata_cube.shape() << endl;
      vb.setVisCubeCorrected(cdata_cube);
    }
  }
}

void SingleDishMS::set_selection(Record const &selection)
{
  check_ms();
  reset_selection();

  //Parse selection
  String timeExpr(""), antennaExpr(""), fieldExpr(""),
    spwExpr(""), uvDistExpr(""), taQLExpr(""), polnExpr(""),
    scanExpr(""), arrayExpr(""), stateExpr(""), obsExpr("");
  timeExpr = get_field_as_casa_string(selection,"time");
  antennaExpr = get_field_as_casa_string(selection,"baseline");
  fieldExpr = get_field_as_casa_string(selection,"field");
  spwExpr = get_field_as_casa_string(selection,"spw");
  uvDistExpr = get_field_as_casa_string(selection,"uvdist");
  taQLExpr = get_field_as_casa_string(selection,"taql");
  polnExpr = get_field_as_casa_string(selection,"polarization");
  scanExpr = get_field_as_casa_string(selection,"scan");
  arrayExpr = get_field_as_casa_string(selection,"array");
  stateExpr = get_field_as_casa_string(selection,"state");
  obsExpr = get_field_as_casa_string(selection,"observation");
  //Now the actual selection.
  mssel_ = new MeasurementSet(*ms_);
  if (!mssSetData(*ms_,*mssel_,"",timeExpr,antennaExpr,fieldExpr,
		  spwExpr,uvDistExpr,taQLExpr,polnExpr,scanExpr,
		  arrayExpr,stateExpr,obsExpr)) { // no valid selection
    reset_selection();
    cout << "Reset selection" << endl;
  }
  if (mssel_!=0)
    cout << "Selected nrows = " << mssel_->nrow() << " from " << ms_->nrow() << "rows" << endl;
}

String SingleDishMS::get_field_as_casa_string(Record const &in_data, string const &field_name){
  Int ifield;
  ifield = in_data.fieldNumber(String(field_name));
  if (ifield>-1) return in_data.asString(ifield);
  return "";
}

}  // End of casa namespace.
