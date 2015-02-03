#include <iostream>
#include <string>
#include <vector>

//#include <libsakura/sakura.h>
//#include <libsakura/config.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Utilities/Assert.h>
#include <casa/Arrays/ArrayMath.h>

#include <ms/MeasurementSets/MSSelectionTools.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisSetUtil.h>

#include <casa_sakura/SakuraUtils.h>
#include <singledish/SingleDish/SingleDishMS.h>

//---for measuring elapse time------------------------
#include <sys/time.h>
double gettimeofday_sec() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double)tv.tv_usec*1.0e-6;
}
//----------------------------------------------------

#define _ORIGIN LogOrigin("SingleDishMS", __func__, WHERE)

namespace casa {

SingleDishMS::SingleDishMS()
  : msname_(""), sdh_(0)
{
  initialize();
}

SingleDishMS::SingleDishMS(string const& ms_name)
  : msname_(ms_name), sdh_(0)
{
  LogIO os(_ORIGIN);
  initialize();
}

SingleDishMS::~SingleDishMS()
{
  if(sdh_){
    delete sdh_;
    sdh_ = 0;
  }
  msname_ = "";
}

void SingleDishMS::initialize()
{
  in_column_ = MS::UNDEFINED_COLUMN;
  //  out_column_ = MS::UNDEFINED_COLUMN;
}

bool SingleDishMS::close()
{
  LogIO os(_ORIGIN);
  os << "Detaching from SingleDishMS"
     << LogIO::POST;

  if(sdh_){
    delete sdh_;
    sdh_ = 0;
  }
  msname_ = "";

  return True;
}

////////////////////////////////////////////////////////////////////////
///// Common utility functions
////////////////////////////////////////////////////////////////////////
void SingleDishMS::set_selection(Record const &selection, bool const verbose)
{
  LogIO os(_ORIGIN);
  if (!selection_.empty()) // selection is set before
    os << "Discard old selection and setting new one." << LogIO::POST;
  if (selection.empty()) // empty selection is passed
    os << "Resetting selection." << LogIO::POST;

  selection_ = selection;
  // Verbose output
  bool any_selection(false);
  if (verbose && !selection.empty()) {
    String timeExpr(""), antennaExpr(""), fieldExpr(""),
      spwExpr(""), uvDistExpr(""), taQLExpr(""), polnExpr(""),
      scanExpr(""), arrayExpr(""), obsExpr(""), intentExpr("");
    timeExpr = get_field_as_casa_string(selection,"timerange");
    antennaExpr = get_field_as_casa_string(selection,"baseline");
    fieldExpr = get_field_as_casa_string(selection,"field");
    spwExpr = get_field_as_casa_string(selection,"spw");
    uvDistExpr = get_field_as_casa_string(selection,"uvdist");
    taQLExpr = get_field_as_casa_string(selection,"taql");
    polnExpr = get_field_as_casa_string(selection,"correlation");
    scanExpr = get_field_as_casa_string(selection,"scan");
    arrayExpr = get_field_as_casa_string(selection,"array");
    intentExpr = get_field_as_casa_string(selection,"intent");
    obsExpr = get_field_as_casa_string(selection,"observation");
    // selection Summary
    os << "[Selection Summary]" << LogIO::POST;
    if (obsExpr != "")
      {any_selection = true; os << "- Observation: " << obsExpr << LogIO::POST;}
    if (antennaExpr != "")
      {any_selection = true; os << "- Antenna: " << antennaExpr << LogIO::POST;}
    if (fieldExpr != "")
      {any_selection = true; os << "- Field: " << fieldExpr << LogIO::POST;}
    if (spwExpr != "")
      {any_selection = true; os << "- SPW: " << spwExpr << LogIO::POST;}
    if (polnExpr != "")
      {any_selection = true; os << "- Pol: " << polnExpr << LogIO::POST;}
    if (scanExpr != "")
      {any_selection = true; os << "- Scan: " << scanExpr << LogIO::POST;}
    if (timeExpr != "")
      {any_selection = true; os << "- Time: " << timeExpr << LogIO::POST;}
    if (intentExpr != "")
      {any_selection = true; os << "- Intent: " << intentExpr << LogIO::POST;}
    if (arrayExpr != "")
      {any_selection = true; os << "- Array: " << arrayExpr << LogIO::POST;}
    if (uvDistExpr != "")
      {any_selection = true; os << "- UVDist: " << uvDistExpr << LogIO::POST;}
    if (taQLExpr != "")
      {any_selection = true; os << "- TaQL: " << taQLExpr << LogIO::POST;}
    if (!any_selection)
      os << "No valid selection parameter is set." << LogIO::WARN;
  }
}

String SingleDishMS::get_field_as_casa_string(Record const &in_data,
					       string const &field_name)
{
  Int ifield;
  ifield = in_data.fieldNumber(String(field_name));
  if (ifield>-1) return in_data.asString(ifield);
  return "";
}


bool SingleDishMS::prepare_for_process(string const &in_column_name,
				       string const &out_ms_name)
{
  // Sort by single dish default
  return prepare_for_process(in_column_name, out_ms_name, Block<Int>(),true);
}

bool SingleDishMS::prepare_for_process(string const &in_column_name,
				       string const &out_ms_name,
				       Block<Int> const &sortColumns,
				       bool const addDefaultSortCols)
{
  LogIO os(_ORIGIN);
  AlwaysAssert(msname_!="", AipsError);
  // define a column to read data from
  if (in_column_name == "float_data")
    in_column_ = MS::FLOAT_DATA;
  else if (in_column_name == "corrected_data")
    in_column_ = MS::CORRECTED_DATA;
  else if (in_column_name == "data")
    in_column_ = MS::DATA;
  else
    throw(AipsError("Invalid data column name"));
  // destroy SDMSManager
  if (sdh_) delete sdh_;
  // Configure record
  Record configure_param(selection_);
  format_selection(configure_param);
  configure_param.define("inputms", msname_);
  configure_param.define("outputms", out_ms_name);
  String in_name(in_column_name);
  in_name.upcase();
  configure_param.define("datacolumn", in_name);
  // The other available keys
  // - buffermode, realmodelcol, usewtspectrum, tileshape,
  // - chanaverage, chanbin, useweights, 
  // - combinespws, ddistart, hanning
  // - regridms, phasecenter, restfreq, outframe, interpolation, nspw,
  // - mode, nchan, start, width, veltype,
  // - timeaverage, timebin, timespan, maxuvwdistance

  // Generate SDMSManager
  sdh_ = new SDMSManager();

  // Configure SDMSManager
  sdh_->configure(configure_param);
  
  ostringstream oss;
  configure_param.print(oss);
  String str(oss.str());
  os << LogIO::DEBUG1 << " Configuration Record " << LogIO::POST;
  os << LogIO::DEBUG1 << str << LogIO::POST;
  // Open the MS and select data
  sdh_->open();
  // set sort column
  sdh_->setSortColumns(sortColumns, addDefaultSortCols);
  // Set up the Data Handler
  sdh_->setup();
  return true;
}

void SingleDishMS::finalize_process()
{
  initialize();
  if(sdh_){
    sdh_->close();
    delete sdh_;
    sdh_ = 0;
  }
}

void SingleDishMS::format_selection(Record &selection)
{
  // At this moment sdh_ is not supposed to be generated yet.
  LogIO os(_ORIGIN);
  int exists = -1;
  // format spw
  String const spwSel(get_field_as_casa_string(selection,"spw"));
  selection.define("spw", spwSel=="" ? "*" : spwSel);

  // Select only auto-correlation
  String autoCorrSel("");
  os << "Formatting antenna selection to select only auto-correlation"
     << LogIO::POST;
  String const antennaSel(get_field_as_casa_string(selection,"baseline"));
  os << LogIO::DEBUG1 << "Input antenna expression = "<< antennaSel
     << LogIO::POST;
  if (antennaSel == "") { //Antenna selection is NOT set
    autoCorrSel = String("*&&&");
  } else { //User defined antenna selection
    MeasurementSet MSobj = MeasurementSet(msname_);
    MeasurementSet* theMS = &MSobj;
    MSSelection theSelection;
    theSelection.setAntennaExpr(antennaSel);
    TableExprNode exprNode = theSelection.toTableExprNode(theMS);
    Vector<Int> ant1Vec = theSelection.getAntenna1List();
    os << LogIO::DEBUG1 << ant1Vec.nelements()
       << " antenna(s) are selected. ID = ";
    for (uInt i=0; i < ant1Vec.nelements(); ++i){
      os << ant1Vec[i] << ", ";
      if (autoCorrSel != "") autoCorrSel += ";";
      autoCorrSel += String::toString(ant1Vec[i]) + "&&&";
    }
  os << LogIO::POST;
  }
  os << LogIO::DEBUG1 << "Auto-correlation selection string: "
     << autoCorrSel << LogIO::POST;
  selection.define("baseline", autoCorrSel);

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
}

void SingleDishMS::convertArrayC2F(Array<Float> &to,
				   Array<Complex> const &from)
{
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

std::vector<string> SingleDishMS::split_string(string const &s, 
					       char delim)
{
  std::vector<string> elems;
  string item;
  for (size_t i = 0; i < s.size(); ++i) {
    char ch = s.at(i);
    if (ch == delim) {
      if (!item.empty()) {
	elems.push_back(item);
      }
      item.clear();
    } else {
      item += ch;
    }
  }
  if (!item.empty()) {
    elems.push_back(item);
  }
  return elems;
}

void SingleDishMS::parse_spwch(string const &spwch, 
			       Vector<Int> &spw, 
			       Vector<size_t> &nchan, 
			       Vector<Vector<Bool> > &mask)
{
  std::vector<string> elems = split_string(spwch, ',');
  size_t length = elems.size();
  spw.resize(length);
  nchan.resize(length);
  mask.resize(length);
  Vector<Vector<size_t> > edge(length);

  for (size_t i = 0; i < length; ++i) {
    std::vector<string> elems_spw = split_string(elems[i], ':');
    std::istringstream iss0(elems_spw[0]);
    iss0 >> spw[i];
    std::istringstream iss1(elems_spw[1]);
    iss1 >> nchan[i];
    std::istringstream iss2(elems_spw[2]);
    string edges;
    iss2 >> edges;
    std::vector<string> elems_edge = split_string(edges, ';');
    size_t length_edge = elems_edge.size();
    edge[i].resize(length_edge);
    for (size_t j = 0; j < length_edge; ++j) {
      std::istringstream iss(elems_edge[j]);
      iss >> edge[i][j];
    }
    mask[i].resize(nchan[i]);
    for (size_t j = 0; j < nchan[i]; ++j) {
      mask[i][j] = False;
    }
    for (size_t j = 0; j < length_edge; j+=2) {
      for (size_t k = edge[i][j]; k <= edge[i][j+1]; ++k) {
	mask[i][k] = True;
      }
    }
  }
}

void SingleDishMS::create_baseline_contexts(LIBSAKURA_SYMBOL(BaselineType) const baseline_type, 
		 	 		    uint16_t order, 
					    Vector<size_t> const &nchan, 
					    Vector<size_t> &ctx_indices, 
					    Vector<LIBSAKURA_SYMBOL(BaselineContext) *> &bl_contexts)
{
  std::vector<size_t> uniq_nchan;
  uniq_nchan.clear();
  ctx_indices.resize(nchan.nelements());
  for (size_t i = 0; i < nchan.nelements(); ++i) {
    size_t idx = 0;
    bool found = false;
    for (size_t j = 0; j < uniq_nchan.size(); ++j) {
      if (uniq_nchan[j] == nchan[i]) {
	idx = j;
	found = true;
	break;
      }
    }
    if (found) {
      ctx_indices[i] = idx;
    } else {
      uniq_nchan.push_back(nchan[i]);
      ctx_indices[i] = uniq_nchan.size() - 1;
     }
  }

  bl_contexts.resize(uniq_nchan.size());
  LIBSAKURA_SYMBOL(Status) status; 

  for (size_t i = 0; i < uniq_nchan.size(); ++i) {
    status = LIBSAKURA_SYMBOL(CreateBaselineContext)(baseline_type, 
						     static_cast<uint16_t>(order), 
						     uniq_nchan[i], 
						     &bl_contexts[i]);
    if (status != LIBSAKURA_SYMBOL(Status_kOK)) {
      ostringstream oss;
      oss << "sakura_CreateBaselineContext() failure -- ";
      if (status == LIBSAKURA_SYMBOL(Status_kNoMemory)) {
	oss << "memory allocation failed.";
      } else if (status == LIBSAKURA_SYMBOL(Status_kInvalidArgument)) {
	oss << "order (" << order << ") must be smaller than the minimum number of channels.";
      } else if (status == LIBSAKURA_SYMBOL(Status_kNG)) {
	oss << "runtime error occured.";
      }
      throw(AipsError(oss.str()));
    }
  }
}

void SingleDishMS::destroy_baseline_contexts(Vector<LIBSAKURA_SYMBOL(BaselineContext) *> &bl_contexts)
{
  LIBSAKURA_SYMBOL(Status) status;
  for (size_t i = 0; i < bl_contexts.nelements(); ++i) {
    status = LIBSAKURA_SYMBOL(DestroyBaselineContext)(bl_contexts[i]);
    if (status != LIBSAKURA_SYMBOL(Status_kOK)) {
      throw(AipsError("DestoyBaselineContext() failed."));
    }
  }
}

void SingleDishMS::get_spectrum_from_cube(Cube<Float> &data_cube,
					  size_t const row,
					  size_t const plane,
					  size_t const num_data,
					  SakuraAlignedArray<float> &out_data)
{
  float *ptr = out_data.data;
  for (size_t i=0; i < num_data; ++i) 
    ptr[i] = static_cast<float>(data_cube(plane, i, row));
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

void SingleDishMS::get_flag_cube(vi::VisBuffer2 const &vb,
				 Cube<Bool> &flag_cube)
{
  flag_cube = vb.flagCube();
}

void SingleDishMS::get_flag_from_cube(Cube<Bool> &flag_cube,
					  size_t const row,
					  size_t const plane,
					  size_t const num_flag,
					  SakuraAlignedArray<bool> &out_flag)
{
  bool *ptr = out_flag.data;
  for (size_t i=0; i < num_flag; ++i) 
    ptr[i] = static_cast<bool>(flag_cube(plane, i, row));
}

////////////////////////////////////////////////////////////////////////
///// Atcual processing functions
////////////////////////////////////////////////////////////////////////
void SingleDishMS::subtract_baseline(string const& in_column_name,
				     string const& out_ms_name,
				     string const &spwch,
				     int const order, 
				     float const clip_threshold_sigma, 
				     int const num_fitting_max)
{
  LogIO os(_ORIGIN);
  os << "Fitting and subtracting polynomial baseline order = " << order << LogIO::POST;
  // in_ms = out_ms
  // in_column = [FLOAT_DATA|DATA|CORRECTED_DATA], out_column=new MS
  // no iteration is necessary for the processing.
  // procedure
  // 1. iterate over MS
  // 2. pick single spectrum from in_column (this is not actually necessary for simple scaling but for exibision purpose)
  // 3. fit a polynomial to each spectrum and subtract it
  // 4. put single spectrum (or a block of spectra) to out_column

  //double tstart = gettimeofday_sec();

  //checking order
  if (order < 0) {
    throw(AipsError("order must be positive or zero."));
  }

  Block<Int> columns(1);
  columns[0] = MS::DATA_DESC_ID;
  LIBSAKURA_SYMBOL(Status) status;
  LIBSAKURA_SYMBOL(BaselineStatus) bl_status;

  prepare_for_process(in_column_name, out_ms_name, columns, false);
  vi::VisibilityIterator2 *vi = sdh_->getVisIter();
  vi::VisBuffer2 *vb = vi->getVisBuffer();

  Vector<Int> spw;
  Vector<size_t> nchan;
  Vector<Vector<Bool> > in_mask;
  parse_spwch(spwch, spw, nchan, in_mask);
  // checking nchan
  int min_nchan = static_cast<int>(nchan(0));
  for (size_t i = 0; i < nchan.nelements(); ++i) {
    int nch = static_cast<int>(nchan(i));
    if (nch < min_nchan) min_nchan = nch;
  }
  if (min_nchan < order + 1) { // for poly and/or chebyshev
    ostringstream oss;
    oss << "Order (=" << order << " given) must be smaller than " 
	<< "the minimum number of channels in the input data (" 
	<< min_nchan << ").";
    throw(AipsError(oss.str()));
  }

  Vector<size_t> ctx_indices;
  Vector<LIBSAKURA_SYMBOL(BaselineContext) *> bl_contexts;
  create_baseline_contexts(LIBSAKURA_SYMBOL(BaselineType_kPolynomial), 
  			   static_cast<uint16_t>(order), 
  			   nchan, ctx_indices, bl_contexts);
  for (vi->originChunks(); vi->moreChunks(); vi->nextChunk()) {
    for (vi->origin(); vi->more(); vi->next()) {
      Vector<Int> data_spw = vb->spectralWindows();
      size_t const num_chan = static_cast<size_t>(vb->nChannels());
      size_t const num_pol = static_cast<size_t>(vb->nCorrelations());
      size_t const num_row = static_cast<size_t>(vb->nRows());
      Cube<Float> data_chunk(num_pol,num_chan,num_row);
      SakuraAlignedArray<float> spec(num_chan);
      Cube<Bool> flag_chunk(num_pol,num_chan,num_row);
      SakuraAlignedArray<bool> mask(num_chan);

      // get a data cube (npol*nchan*nrow) from VisBuffer
      get_data_cube_float(*vb, data_chunk);
      // get a flag cube (npol*nchan*nrow) from VisBuffer
      get_flag_cube(*vb, flag_chunk);
      // get a flagrow vector (nrow) from VisBuffer
      Vector<Bool> flagrow_chunk = vb->flagRow();
      // loop over MS rows
      for (size_t irow=0; irow < num_row; ++irow) {
	// skip row-flagged spectra
	if (flagrow_chunk(irow)) continue;

  	size_t idx = 0;
  	for (size_t ispw=0; ispw < spw.nelements(); ++ispw) {
  	  if (data_spw[irow] == spw[ispw]) {
  	    idx = ispw;
  	    break;
  	  }
  	}
  	assert(num_chan == nchan[idx]);

  	// loop over polarization
  	for (size_t ipol=0; ipol < num_pol; ++ipol) {
  	  // get a spectrum from data cube
  	  get_spectrum_from_cube(data_chunk, irow, ipol, num_chan, spec);
  	  // get a channel mask from data cube
  	  // (note that mask used here is actually a flag)
  	  get_flag_from_cube(flag_chunk, irow, ipol, num_chan, mask);
  	  // convert flag to mask by taking logical NOT of flag
  	  // and then operate logical AND with in_mask
  	  for (size_t ichan=0; ichan < num_chan; ++ichan) {
  	    mask.data[ichan] = in_mask[idx][ichan] && (!(mask.data[ichan]));
  	  }
  	  // actual execution of single spectrum
  	  status = 
  	    LIBSAKURA_SYMBOL(SubtractBaselineFloat)(bl_contexts[ctx_indices[idx]], 
  						    static_cast<uint16_t>(order), 
						    num_chan, 
  						    spec.data, 
  						    mask.data, 
  						    clip_threshold_sigma, 
  						    num_fitting_max,
  						    true, 
  						    mask.data, 
  						    spec.data, 
  						    &bl_status);
  	  if (status != LIBSAKURA_SYMBOL(Status_kOK)) {
	    throw(AipsError("SubtractBaselineFloat() failed."));
  	  }
  	  // set back a spectrum to data cube
  	  set_spectrum_to_cube(data_chunk, irow, ipol, num_chan, spec.data);
  	} // end of polarization loop
      } // end of chunk row loop
      // write back data cube to VisBuffer
      sdh_->fillCubeToOutputMs(vb, data_chunk);
    } // end of vi loop
  } // end of chunk loop
  finalize_process();

  // destroy baselint contexts
  destroy_baseline_contexts(bl_contexts);

  //double tend = gettimeofday_sec();
  //std::cout << "Elapsed time = " << (tend - tstart) << " sec." << std::endl;
}

void SingleDishMS::scale(float const factor,
			  string const& in_column_name,
			  string const& out_ms_name)
{
  LogIO os(_ORIGIN);
  os << "Multiplying scaling factor = " << factor << LogIO::POST;
  // in_ms = out_ms
  // in_column = [FLOAT_DATA|DATA|CORRECTED_DATA], out_column=new MS
  // no iteration is necessary for the processing.
  // procedure
  // 1. iterate over MS
  // 2. pick single spectrum from in_column
  // 3. multiply a scaling factor to each spectrum
  // 4. put single spectrum (or a block of spectra) to out_column
  prepare_for_process(in_column_name, out_ms_name);
  vi::VisibilityIterator2 *vi = sdh_->getVisIter();
  vi::VisBuffer2 *vb = vi->getVisBuffer();

  // //DEBUG
  // Block<Int> scol = vi->getSortColumns().getColumnIds();
  // cout << "sort columns of iterator = ";
  // for (size_t i = 0; i < scol.nelements(); ++i)
  //   cout << scol[i] << ", ";
  // cout << endl;
  // cout << "default added = " << (vi->getSortColumns().shouldAddDefaultColumns()? 'T' : 'F') << endl;

  for (vi->originChunks(); vi->moreChunks(); vi->nextChunk()) {
    for (vi->origin(); vi->more(); vi->next()) {
      size_t const num_chan = static_cast<size_t>(vb->nChannels());
      size_t const num_pol = static_cast<size_t>(vb->nCorrelations());
      size_t const num_row = static_cast<size_t>(vb->nRows());
      Cube<Float> data_chunk(num_pol,num_chan,num_row);
      Matrix<Float> data_row(num_pol,num_chan);
      SakuraAlignedArray<float> spectrum(num_chan);
      // get a data cube (npol*nchan*nrow) from VisBuffer
      get_data_cube_float(*vb, data_chunk);
      // loop over MS rows
      for (size_t irow=0; irow < num_row; ++irow) {
	// loop over polarization
	for (size_t ipol=0; ipol < num_pol; ++ipol) {
	  // get a spectrum from data cube
	  get_spectrum_from_cube(data_chunk, irow, ipol, num_chan, spectrum);

	  // actual execution of single spectrum
	  do_scale(factor, num_chan, spectrum.data);

	  // set back a spectrum to data cube
	  set_spectrum_to_cube(data_chunk, irow, ipol, num_chan, spectrum.data);
	} // end of polarization loop
      } // end of chunk row loop
      // write back data cube to Output MS
      sdh_->fillCubeToOutputMs(vb, data_chunk);
    } // end of vi loop
  } // end of chunk loop
  finalize_process();
}

void SingleDishMS::do_scale(float const factor,
			    size_t const num_data, float *data)
{
  for (size_t i=0; i < num_data; ++i) 
    data[i] *= factor;
}

}  // End of casa namespace.
