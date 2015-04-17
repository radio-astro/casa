#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <map>

//#include <libsakura/sakura.h>
//#include <libsakura/config.h>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Utilities/Assert.h>
#include <casa/Arrays/ArrayMath.h>

#include <ms/MSSel/MSSelectionTools.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisSetUtil.h>

#include <casa_sakura/SakuraUtils.h>
#include <singledish/SingleDish/SingleDishMS.h>
#include <singledish/SingleDish/BLParameterParser.h>

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
  if (in_column_name=="float_data") {
    in_column_ = MS::FLOAT_DATA;
  }
  else if (in_column_name=="corrected_data") {
    in_column_ = MS::CORRECTED_DATA;
  } else if (in_column_name=="data") {
    in_column_ = MS::DATA;
  } else {
    throw(AipsError("Invalid data column name"));
  }
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
  // set large timebin
  Double timeBin;
  int exists = configure_param.fieldNumber("timebin");
  if (exists<0){
    timeBin = 1.0e8;
  } else {
    configure_param.get(exists, timeBin);
  }
  // set sort column
  sdh_->setSortColumns(sortColumns, addDefaultSortCols, timeBin);
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

void SingleDishMS::parse_spw(string const &in_spw, 
			     Vector<Int> &rec_spw,
			     Matrix<Int> &rec_chan,
			     Vector<size_t> &nchan, 
			     Vector<Vector<Bool> > &mask,
			     Vector<bool> &nchan_set)
{
  Record selrec = sdh_->getSelRec(in_spw);
  rec_spw = selrec.asArrayInt("spw");
  rec_chan = selrec.asArrayInt("channel");
  nchan.resize(rec_spw.nelements());
  mask.resize(rec_spw.nelements());
  nchan_set.resize(rec_spw.nelements());
  for (size_t i = 0; i < nchan_set.nelements(); ++i) {
    nchan_set(i) = false;
  }
}

void SingleDishMS::get_nchan_and_mask(Vector<Int> const &rec_spw,
				      Vector<Int> const &data_spw,
				      Matrix<Int> const &rec_chan,
				      size_t const num_chan,
			      	      Vector<size_t> &nchan, 
			              Vector<Vector<Bool> > &mask,
				      Vector<bool> &nchan_set,
				      bool &new_nchan)
{
  new_nchan = false;
  for (size_t i = 0; i < rec_spw.nelements(); ++i) {
    //get nchan by spwid and set to nchan[]
    for (size_t j = 0; j < data_spw.nelements(); ++j) {
      if ((!nchan_set(i))&&(data_spw(j) == rec_spw(i))) {
	bool found = false;
	for (size_t k = 0; k < nchan.nelements(); ++k) {
	  if (!nchan_set(k)) continue;
	  if (nchan(k) == num_chan) found = true;
	}
	if (!found) {
	  new_nchan = true;
	}
	nchan(i) = num_chan;
	nchan_set(i) = true;
	break;
      }
    }
    if (!nchan_set(i)) continue;
    mask(i).resize(nchan(i));
    // generate mask
    get_mask_from_rec(rec_spw(i), rec_chan, mask(i),true);
//     for (size_t j = 0; j < mask(i).nelements(); ++j) {
//       mask(i)(j) = False;
//     }
//     std::vector<uInt> edge; // start,end,stride,start,...
//     edge.clear();
//     for (size_t j = 0; j < rec_chan.nrow(); ++j) {
//       if (rec_chan.row(j)(0) == rec_spw(i)) {
// 	edge.push_back(rec_chan.row(j)(1));
// 	edge.push_back(rec_chan.row(j)(2));
// 	edge.push_back(rec_chan.row(j)(3))
//       }
//     }
//     //generate mask
//     for (size_t j = 0; j < edge.size(); j+=3) {
//       for (size_t k = edge[j]; k <= edge[j+1]; k+=edge[j+2]) {
// 	mask(i)(k) = True;
//       }
//     }
  }
}

void SingleDishMS::get_mask_from_rec(Int spwid,
				     Matrix<Int> const &rec_chan,
				     Vector<Bool> &mask, bool initialize)
{
  if (initialize) {
    for (size_t j = 0; j < mask.nelements(); ++j) {
      mask(j) = False;
    }
  }
  //construct a list of (start, end, stride, start, end, stride, ...)
  //from rec_chan for the spwid
  std::vector<uInt> edge;
  edge.clear();
  for (size_t j = 0; j < rec_chan.nrow(); ++j) {
    if (rec_chan.row(j)(0) == spwid) {
      edge.push_back(rec_chan.row(j)(1));
      edge.push_back(rec_chan.row(j)(2));
      edge.push_back(rec_chan.row(j)(3));
    }
  }
  //generate mask
  for (size_t j = 0; j < edge.size(); j+=3) {
    for (size_t k = edge[j]; k <= edge[j+1]; k+=edge[j+2]) {
      mask(k) = True;
    }
  }
}

void SingleDishMS::get_masklist_from_mask(size_t const num_chan, 
					  bool const *mask, 
					  Vector<uInt> &masklist)
{
  std::vector<int> mlist;
  mlist.clear();

  for (uInt i = 0; i < num_chan; ++i) {
    if (mask[i]) {
      if ((i == 0)||(i == num_chan-1)) {
	mlist.push_back(i);
      } else {
	if ((mask[i])&&(!mask[i-1])) {
	  mlist.push_back(i);
	}
	if ((mask[i])&&(!mask[i+1])) {
	  mlist.push_back(i);
	}
      }
    }
  }

  masklist.resize(masklist.size());
  for (size_t i = 0; i < masklist.size(); ++i) {
    masklist[i] = (uInt)mlist[i];
  }
}

void SingleDishMS::get_pol_selection(string const &in_pol,
				     size_t const num_pol,
				     Vector<bool> &pol)
{
  pol.resize(num_pol);
  bool pol_val = (in_pol == "")||(in_pol == "*");
  for (size_t i = 0; i < pol.nelements(); ++i) {
    pol(i) = pol_val;
  }
  if (!pol_val) {
    //parse_inpol
    istringstream iss(in_pol);
    string tmp;
    std::vector<int> in_pol_list;
    while (getline(iss, tmp, ',')) {
      istringstream iss2(tmp);
      size_t itmp;
      iss2 >> itmp;
      in_pol_list.push_back(itmp);
    }
    //set True for pol specified
    for (size_t i = 0; i < pol.nelements(); ++i) {
      for (size_t j = 0; j < in_pol_list.size(); ++j) {
	if (in_pol_list[j] == (int)i) {
	  pol(i) = true;
	  break;
	}
      }
    }
  }
}

void SingleDishMS::get_baseline_context(LIBSAKURA_SYMBOL(BaselineType) const baseline_type, 
		 	 		uint16_t order, 
					size_t num_chan, 
				        Vector<size_t> const &nchan, 
					Vector<bool> const &nchan_set,
					Vector<size_t> &ctx_indices, 
				        std::vector<LIBSAKURA_SYMBOL(BaselineContext) *> &bl_contexts)
{
  size_t idx = 0;
  bool found = false;
  for (size_t i = 0; i < nchan.nelements(); ++i) {
    if ((nchan_set[i])&&(nchan[i] == num_chan)) {
      idx = bl_contexts.size();
      found = true;
      break;
    }
  }
  if (found) {
    for (size_t i = 0; i < nchan.nelements(); ++i) {
      if ((nchan_set[i])&&(nchan[i] == num_chan)) {
	ctx_indices[i] = idx;
      }

    }

    LIBSAKURA_SYMBOL(BaselineContext) *context;
    LIBSAKURA_SYMBOL(Status) status; 
    status = LIBSAKURA_SYMBOL(CreateBaselineContext)(baseline_type, 
						     static_cast<uint16_t>(order), 
						     num_chan, &context);
    check_sakura_status("sakura_CreateBaselineContext", status);
    bl_contexts.push_back(context);
  }
}

void SingleDishMS::destroy_baseline_contexts(std::vector<LIBSAKURA_SYMBOL(BaselineContext) *> &bl_contexts)
{
  LIBSAKURA_SYMBOL(Status) status;
  for (size_t i = 0; i < bl_contexts.size(); ++i) {
    status = LIBSAKURA_SYMBOL(DestroyBaselineContext)(bl_contexts[i]);
    check_sakura_status("sakura_DestoyBaselineContext", status);
  }
}

void SingleDishMS::check_sakura_status(string const &name, 
				       LIBSAKURA_SYMBOL(Status) const status)
{
  if (status == LIBSAKURA_SYMBOL(Status_kOK)) return;

  ostringstream oss;
  oss << name << "() failed -- ";
  if (status == LIBSAKURA_SYMBOL(Status_kNG)) {
    oss << "NG";
  } else if (status == LIBSAKURA_SYMBOL(Status_kInvalidArgument)) {
    oss << "InvalidArgument";
  } else if (status == LIBSAKURA_SYMBOL(Status_kNoMemory)) {
    oss << "NoMemory";
  } else if (status == LIBSAKURA_SYMBOL(Status_kUnknownError)) {
    oss << "UnknownError";
  }
  throw(AipsError(oss.str()));
}

void SingleDishMS::get_spectrum_from_cube(Cube<Float> &data_cube,
					  size_t const row,
					  size_t const plane,
					  size_t const num_data,
					  SakuraAlignedArray<float> &out_data)
{
  AlwaysAssert(static_cast<size_t>(data_cube.ncolumn())==num_data,
	       AipsError);
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
  AlwaysAssert(static_cast<size_t>(data_cube.ncolumn())==num_data,
	       AipsError);
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
  AlwaysAssert(static_cast<size_t>(flag_cube.ncolumn())==num_flag,
	       AipsError);
  bool *ptr = out_flag.data;
  for (size_t i=0; i < num_flag; ++i) 
    ptr[i] = static_cast<bool>(flag_cube(plane, i, row));
}

void SingleDishMS::set_flag_to_cube(Cube<Bool> &flag_cube,
				    size_t const row,
				    size_t const plane,
				    size_t const num_flag,
				    bool *in_flag)
{
  AlwaysAssert(static_cast<size_t>(flag_cube.ncolumn())==num_flag,
	       AipsError);
  for (size_t i=0; i < num_flag; ++i) 
    flag_cube(plane, i, row) = static_cast<Bool>(in_flag[i]);
}

void SingleDishMS::flag_spectrum_in_cube(Cube<Bool> &flag_cube,
					 size_t const row,
					 size_t const plane)
{
  uInt const num_flag = flag_cube.ncolumn();
  for (uInt ichan=0; ichan<num_flag; ++ichan)
    flag_cube(plane,ichan,row) = True;
}


bool SingleDishMS::allchannels_flagged(size_t const num_flag, bool const* flag)
{
  bool res = true;
  for (size_t i = 0; i < num_flag; ++i) {
    if (!flag[i]) {
      res = false;
      break;
    }
  }
  return res;
}

size_t SingleDishMS::NValidMask(size_t const num_mask, bool const* mask)
{
  std::size_t nvalid = 0;
  // the assertion lines had better be replaced with static_assert when c++11 is supported
  AlwaysAssert(static_cast<std::size_t>(true)==1, AipsError);
  AlwaysAssert(static_cast<std::size_t>(false)==0, AipsError);
  for (size_t i = 0; i < num_mask; ++i) {
    nvalid += static_cast<std::size_t>(mask[i]);
  }
  return nvalid;
}



////////////////////////////////////////////////////////////////////////
///// Atcual processing functions
////////////////////////////////////////////////////////////////////////
void SingleDishMS::subtract_baseline(string const& in_column_name,
				     string const& out_ms_name,
				     string const& out_bltable_name,
				     bool const& do_subtract,
				     string const& in_spw,
				     string const& in_ppp,
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
  BaselineTable *bt = 0;
  bool write_baseline_table = (out_bltable_name != "");
  if (write_baseline_table) bt = new BaselineTable(sdh_->getMS());
  bool do_set_spectrum_to_cube = (do_subtract || !write_baseline_table);

  Vector<Int> recspw;
  Matrix<Int> recchan;
  Vector<size_t> nchan;
  Vector<Vector<Bool> > in_mask;
  Vector<bool> nchan_set;
  parse_spw(in_spw, recspw, recchan, nchan, in_mask, nchan_set);

  Vector<size_t> ctx_indices;
  ctx_indices.resize(nchan.nelements());
  for (size_t ictx = 0; ictx < ctx_indices.nelements(); ++ictx) {
    ctx_indices(ictx) = 0;
  }
  std::vector<LIBSAKURA_SYMBOL(BaselineContext) *> bl_contexts;
  bl_contexts.clear();
  Vector<bool> pol;
  bool pol_set = false;

  for (vi->originChunks(); vi->moreChunks(); vi->nextChunk()) {
    for (vi->origin(); vi->more(); vi->next()) {
      Vector<Int> scans = vb->scan();
      Vector<Double> times = vb->time();
      Vector<Int> beams = vb->feed1();

      Vector<Int> data_spw = vb->spectralWindows();
      size_t const num_chan = static_cast<size_t>(vb->nChannels());
      if ((int)num_chan < order + 1) {
	throw(AipsError("subtract_baseline: nchan must be greater than order value."));
      }
      size_t const num_pol = static_cast<size_t>(vb->nCorrelations());
      size_t const num_row = static_cast<size_t>(vb->nRows());
      Cube<Float> data_chunk(num_pol,num_chan,num_row);
      SakuraAlignedArray<float> spec(num_chan);
      Cube<Bool> flag_chunk(num_pol,num_chan,num_row);
      SakuraAlignedArray<bool> mask(num_chan);

      bool new_nchan = false;
      get_nchan_and_mask(recspw, data_spw, recchan, num_chan, nchan, in_mask, nchan_set, new_nchan);
      if (new_nchan) {
	get_baseline_context(LIBSAKURA_SYMBOL(BaselineType_kPolynomial), 
			     static_cast<uint16_t>(order), 
			     num_chan, nchan, nchan_set, ctx_indices, bl_contexts);
      }

      // get data/flag cubes (npol*nchan*nrow) from VisBuffer
      get_data_cube_float(*vb, data_chunk);
      get_flag_cube(*vb, flag_chunk);

      if (!pol_set) {
	get_pol_selection(in_ppp, num_pol, pol);
	pol_set = true;
      }
      // loop over MS rows
      for (size_t irow = 0; irow < num_row; ++irow) {
  	size_t idx = 0;
  	for (size_t ispw = 0; ispw < recspw.nelements(); ++ispw) {
  	  if (data_spw[irow] == recspw[ispw]) {
  	    idx = ispw;
  	    break;
  	  }
  	}

	//prepare varables for writing baseline table
	Array<Bool> apply_mtx(IPosition(2, num_pol, 1));
	Array<uInt> bltype_mtx(IPosition(2, num_pol, 1));
	Array<Int> fpar_mtx(IPosition(2, num_pol, 1));
	Array<Float> ffpar_mtx(IPosition(2, num_pol, 0));//1));
  	for (size_t ipol = 0; ipol < num_pol; ++ipol) {
	  bltype_mtx[0][ipol] = (uInt)LIBSAKURA_SYMBOL(BaselineType_kPolynomial);
	  fpar_mtx[0][ipol] = (Int)order;
	}
	size_t num_masklist_max = 0;
	std::vector<std::vector<uInt> > masklist_mtx_tmp(num_pol);
	size_t num_coeff;
	status = LIBSAKURA_SYMBOL(GetNumberOfCoefficients)(bl_contexts[ctx_indices[idx]], 
							   order,
							   &num_coeff);
	check_sakura_status("sakura_GetNumberOfCoefficients", status);
	SakuraAlignedArray<double> coeff(num_coeff);
	Array<Float> coeff_mtx(IPosition(2, num_pol, num_coeff));
  	for (size_t ipol = 0; ipol < num_pol; ++ipol) {
	  for (size_t icoeff = 0; icoeff < num_coeff; ++icoeff) {
	    coeff_mtx[icoeff][ipol] = 0.0;
	  }
	}
	Array<Float> rms_mtx(IPosition(2, num_pol, 1));
	Array<Float> cthres_mtx(IPosition(2, num_pol, 1));
	Array<uInt> citer_mtx(IPosition(2, num_pol, 1));
	Array<Float> lfthres_mtx(IPosition(2, num_pol, 1));
	Array<uInt> lfavg_mtx(IPosition(2, num_pol, 1));
	Array<uInt> lfedge_mtx(IPosition(2, num_pol, 1));

  	// loop over polarization
  	for (size_t ipol = 0; ipol < num_pol; ++ipol) {
	  apply_mtx[0][ipol] = True;
	  // skip spectrum not selected by pol
	  if (!pol(ipol)) {
	    flag_spectrum_in_cube(flag_chunk,irow,ipol);
	    apply_mtx[0][ipol] = False;
	    continue;
	  }
  	  // get a channel mask from data cube
  	  // (note that the variable 'mask' is flag in the next line 
	  // actually, then it will be converted to real mask when 
	  // taking AND with user-given mask info. this is just for 
	  // saving memory usage...)
  	  get_flag_from_cube(flag_chunk, irow, ipol, num_chan, mask);
	  // skip spectrum if all channels flagged
	  if (allchannels_flagged(num_chan, mask.data)) {
	    apply_mtx[0][ipol] = False;
	    continue;
	  }

  	  // convert flag to mask by taking logical NOT of flag
  	  // and then operate logical AND with in_mask
  	  for (size_t ichan = 0; ichan < num_chan; ++ichan) {
  	    mask.data[ichan] = in_mask[idx][ichan] && (!(mask.data[ichan]));
  	  }
  	  // get a spectrum from data cube
  	  get_spectrum_from_cube(data_chunk, irow, ipol, num_chan, spec);

  	  // actual execution of single spectrum
	  if (write_baseline_table) {
	    status = 
	    LIBSAKURA_SYMBOL(GetBestFitBaselineCoefficientsFloat)(bl_contexts[ctx_indices[idx]], 
								  num_chan,
								  spec.data, 
							          mask.data,
							          clip_threshold_sigma,
							          num_fitting_max, 
							          num_coeff, 
							          coeff.data,
							          mask.data,
							          &bl_status);
	    check_sakura_status("sakura_GetBestFitBaselineCoefficientsFloat", status);
	    set_array_for_bltable<double, Float>(ipol, num_coeff, coeff.data, coeff_mtx);
	    /*
	    for (size_t icoeff = 0; icoeff < num_coeff; ++icoeff) {
	      coeff_mtx[icoeff][ipol] = coeff.data[icoeff];
	    }
	    */
	    Vector<uInt> masklist;
	    get_masklist_from_mask(num_chan, mask.data, masklist);
	    if (masklist.size() > num_masklist_max) {
	      num_masklist_max = masklist.size();
	    }
	    masklist_mtx_tmp[ipol].clear();
	    for (size_t imask = 0; imask < masklist.size(); ++imask) {
	      masklist_mtx_tmp[ipol].push_back(masklist[imask]);
	    }

	    status = 
	    LIBSAKURA_SYMBOL(SubtractBaselineUsingCoefficientsFloat)(bl_contexts[ctx_indices[idx]],
								     num_chan,
								     spec.data,
								     num_coeff,
							             coeff.data,
							             spec.data);
	    check_sakura_status("sakura_SubtractBaselineUsingCoefficientsFloat", status);
	    LIBSAKURA_SYMBOL(StatisticsResultFloat) stat;
	    status = 
	    //LIBSAKURA_SYMBOL(ComputeAccurateStatisticsFloat)(num_chan,
	    LIBSAKURA_SYMBOL(ComputeStatisticsFloat)(num_chan,
							     spec.data,
							     mask.data,
							     &stat);
	    check_sakura_status("sakura_ComputeAccurateStatisticsFloat", status);
	    rms_mtx[0][ipol] = stat.stddev;

	    cthres_mtx[0][ipol] = clip_threshold_sigma;
	    citer_mtx[0][ipol] = (uInt)num_fitting_max;
	    lfthres_mtx[0][ipol] = 0.0;
	    lfavg_mtx[0][ipol] = 0;
	    lfedge_mtx[0][ipol] = 0;

	  } else {
	    status = LIBSAKURA_SYMBOL(SubtractBaselineFloat)(bl_contexts[ctx_indices[idx]], 
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
	    check_sakura_status("sakura_SubtractBaselineFloat", status);
	  }
  	  // set back a spectrum to data cube
	  if (do_set_spectrum_to_cube) {
	    set_spectrum_to_cube(data_chunk, irow, ipol, num_chan, spec.data);
	  }
  	} // end of polarization loop

	if (write_baseline_table) {
	  // write to baseline table
	  Array<uInt> masklist_mtx(IPosition(2, num_pol, num_masklist_max));
	  set_matrix_for_bltable<uInt, uInt>(num_pol, num_masklist_max, masklist_mtx_tmp, masklist_mtx);
	  /*
	  for (size_t ipol = 0; ipol < num_pol; ++ipol) {
	    for (size_t imask = 0; imask < num_masklist_max; ++imask) {
	      masklist_mtx[imask][ipol] = 0;
	    }
	    for (size_t imask = 0; imask < masklist_mtx_tmp[ipol].size(); ++imask) {
	      masklist_mtx[imask][ipol] = masklist_mtx_tmp[ipol][imask];
	    }
	  }
	  */
	  bt->appenddata((uInt)scans[irow], (uInt)beams[irow], (uInt)data_spw[irow],
			 0, times[irow], apply_mtx, bltype_mtx, 
			 fpar_mtx, ffpar_mtx, masklist_mtx,
			 coeff_mtx, rms_mtx, (uInt)num_chan, cthres_mtx,
			 citer_mtx, lfthres_mtx, lfavg_mtx, lfedge_mtx);
	}
      } // end of chunk row loop
      // write back data cube to VisBuffer
      sdh_->fillCubeToOutputMs(vb, data_chunk, &flag_chunk);
    } // end of vi loop
  } // end of chunk loop

  if (write_baseline_table) {
    bt->save(out_bltable_name);
    delete bt;
  }
  finalize_process();
  destroy_baseline_contexts(bl_contexts);

  //double tend = gettimeofday_sec();
  //std::cout << "Elapsed time = " << (tend - tstart) << " sec." << std::endl;
}

//Cubic Spline
void SingleDishMS::subtract_baseline_cspline(string const& in_column_name,
				     string const& out_ms_name,
				     string const& out_bltable_name,
				     bool const& do_subtract,
				     string const& in_spw,
				     string const& in_ppp,
				     int const npiece, 
				     float const clip_threshold_sigma, 
				     int const num_fitting_max)
{
  LogIO os(_ORIGIN);
  os << "Fitting and subtracting cubic spline baseline npiece = " << npiece << LogIO::POST;
  // in_ms = out_ms
  // in_column = [FLOAT_DATA|DATA|CORRECTED_DATA], out_column=new MS
  // no iteration is necessary for the processing.
  // procedure
  // 1. iterate over MS
  // 2. pick single spectrum from in_column (this is not actually necessary for simple scaling but for exibision purpose)
  // 3. fit a polynomial to each spectrum and subtract it
  // 4. put single spectrum (or a block of spectra) to out_column

  //double tstart = gettimeofday_sec();

  //checking npiece 
  if (npiece <= 0) {
    throw(AipsError("npiece must be positive."));
  }

  Block<Int> columns(1);
  columns[0] = MS::DATA_DESC_ID;
  LIBSAKURA_SYMBOL(Status) status;
  LIBSAKURA_SYMBOL(BaselineStatus) bl_status;

  prepare_for_process(in_column_name, out_ms_name, columns, false);
  vi::VisibilityIterator2 *vi = sdh_->getVisIter();
  vi::VisBuffer2 *vb = vi->getVisBuffer();
  BaselineTable *bt = 0;
  bool write_baseline_table = (out_bltable_name != "");
  if (write_baseline_table) bt = new BaselineTable(sdh_->getMS());
  bool do_set_spectrum_to_cube = (do_subtract || !write_baseline_table);

  Vector<Int> recspw;
  Matrix<Int> recchan;
  Vector<size_t> nchan;
  Vector<Vector<Bool> > in_mask;
  Vector<bool> nchan_set;
  parse_spw(in_spw, recspw, recchan, nchan, in_mask, nchan_set);

  Vector<size_t> ctx_indices;
  ctx_indices.resize(nchan.nelements());
  for (size_t ictx = 0; ictx < ctx_indices.nelements(); ++ictx) {
    ctx_indices(ictx) = 0;
  }
  std::vector<LIBSAKURA_SYMBOL(BaselineContext) *> bl_contexts;
  bl_contexts.clear();
  Vector<bool> pol;
  bool pol_set = false;

  for (vi->originChunks(); vi->moreChunks(); vi->nextChunk()) {
    for (vi->origin(); vi->more(); vi->next()) {
      Vector<Int> scans = vb->scan();
      Vector<Double> times = vb->time();
      Vector<Int> beams = vb->feed1();

      Vector<Int> data_spw = vb->spectralWindows();
      size_t const num_chan = static_cast<size_t>(vb->nChannels());
      if ((int)num_chan < npiece * 4) {
	throw(AipsError("subtract_baseline_cspline: nchan must be greater than 4*npiece."));
      }
      size_t const num_pol = static_cast<size_t>(vb->nCorrelations());
      size_t const num_row = static_cast<size_t>(vb->nRows());
      Cube<Float> data_chunk(num_pol,num_chan,num_row);
      SakuraAlignedArray<float> spec(num_chan);
      Cube<Bool> flag_chunk(num_pol,num_chan,num_row);
      SakuraAlignedArray<bool> mask(num_chan);

      bool new_nchan = false;
      get_nchan_and_mask(recspw, data_spw, recchan, num_chan, nchan, in_mask, nchan_set, new_nchan);
      if (new_nchan) {
	get_baseline_context(LIBSAKURA_SYMBOL(BaselineType_kCubicSpline), 
			     static_cast<uint16_t>(npiece), 
			     num_chan, nchan, nchan_set, ctx_indices, bl_contexts);
      }
      // get data/flag cubes (npol*nchan*nrow) from VisBuffer
      get_data_cube_float(*vb, data_chunk);
      get_flag_cube(*vb, flag_chunk);

      if (!pol_set) {
	get_pol_selection(in_ppp, num_pol, pol);
	pol_set = true;
      }
      // loop over MS rows
      for (size_t irow = 0; irow < num_row; ++irow) {
  	size_t idx = 0;
  	for (size_t ispw = 0; ispw < recspw.nelements(); ++ispw) {
  	  if (data_spw[irow] == recspw[ispw]) {
  	    idx = ispw;
  	    break;
  	  }
  	}

	//prepare varables for writing baseline table
	Array<Bool> apply_mtx(IPosition(2, num_pol, 1));
	Array<uInt> bltype_mtx(IPosition(2, num_pol, 1));
	Array<Int> fpar_mtx(IPosition(2, num_pol, 1));
	Array<Float> ffpar_mtx(IPosition(2, num_pol, npiece));
  	for (size_t ipol = 0; ipol < num_pol; ++ipol) {
	  bltype_mtx[0][ipol] = (uInt)LIBSAKURA_SYMBOL(BaselineType_kCubicSpline);
	  fpar_mtx[0][ipol] = (Int)npiece;
	  for (int ipiece = 0; ipiece < npiece; ++ipiece) {
	    ffpar_mtx[ipiece][ipol] = 0.0;
	  }
	}
	size_t num_masklist_max = 0;
	std::vector<std::vector<uInt> > masklist_mtx_tmp(num_pol);
	size_t num_coeff = 4 * npiece;
	SakuraAlignedArray<double> coeff(num_coeff);
	Array<Float> coeff_mtx(IPosition(2, num_pol, num_coeff));
  	for (size_t ipol = 0; ipol < num_pol; ++ipol) {
	  for (size_t icoeff = 0; icoeff < num_coeff; ++icoeff) {
	    coeff_mtx[icoeff][ipol] = 0.0;
	  }
	}
	Array<Float> rms_mtx(IPosition(2, num_pol, 1));
	Array<Float> cthres_mtx(IPosition(2, num_pol, 1));
	Array<uInt> citer_mtx(IPosition(2, num_pol, 1));
	Array<Float> lfthres_mtx(IPosition(2, num_pol, 1));
	Array<uInt> lfavg_mtx(IPosition(2, num_pol, 1));
	Array<uInt> lfedge_mtx(IPosition(2, num_pol, 1));

  	// loop over polarization
  	for (size_t ipol = 0; ipol < num_pol; ++ipol) {
	  apply_mtx[0][ipol] = True;
	  // skip spectrum not selected by pol
	  if (!pol(ipol)) {
	    flag_spectrum_in_cube(flag_chunk,irow,ipol);
	    apply_mtx[0][ipol] = False;
	    continue;
	  }
  	  // get a channel mask from data cube
  	  // (note that the variable 'mask' is flag in the next line 
	  // actually, then it will be converted to real mask when 
	  // taking AND with user-given mask info. this is just for 
	  // saving memory usage...)
  	  get_flag_from_cube(flag_chunk, irow, ipol, num_chan, mask);
	  // skip spectrum if all channels flagged
	  if (allchannels_flagged(num_chan, mask.data)) {
	    apply_mtx[0][ipol] = False;
	    continue;
	  }

  	  // convert flag to mask by taking logical NOT of flag
  	  // and then operate logical AND with in_mask
  	  for (size_t ichan = 0; ichan < num_chan; ++ichan) {
  	    mask.data[ichan] = in_mask[idx][ichan] && (!(mask.data[ichan]));
  	  }
  	  // get a spectrum from data cube
  	  get_spectrum_from_cube(data_chunk, irow, ipol, num_chan, spec);

  	  // actual execution of single spectrum
	  if (write_baseline_table) {
	    status = 
	    LIBSAKURA_SYMBOL(GetBestFitBaselineCoefficientsCubicSplineFloat)(bl_contexts[ctx_indices[idx]], 
									     num_chan,
									     spec.data, 
									     mask.data,
								             clip_threshold_sigma,
								             num_fitting_max, 
								             npiece, 
								             coeff.data,
								             mask.data,
								             &bl_status);
	    check_sakura_status("sakura_GetBestFitBaselineCoefficientsCubicSplineFloat", status);
	    set_array_for_bltable<double, Float>(ipol, num_coeff, coeff.data, coeff_mtx);
	    /*
	    for (size_t icoeff = 0; icoeff < num_coeff; ++icoeff) {
	      coeff_mtx[icoeff][ipol] = coeff.data[icoeff];
	    }
	    */
	    Vector<uInt> masklist;
	    get_masklist_from_mask(num_chan, mask.data, masklist);
	    if (masklist.size() > num_masklist_max) {
	      num_masklist_max = masklist.size();
	    }
	    masklist_mtx_tmp[ipol].clear();
	    for (size_t imask = 0; imask < masklist.size(); ++imask) {
	      masklist_mtx_tmp[ipol].push_back(masklist[imask]);
	    }
	    // ***workaround***
	    // get piece boundary positions
	    // as sakura_GetBestFitBaselineCoefficientsCubicSplineFloat 
	    // does not return boundary values now
	    SakuraAlignedArray<double> boundary(npiece);
	    GetBoundariesOfPiecewiseData(num_chan, mask.data, npiece, boundary.data);
	    // ***end workaround***
	    set_array_for_bltable<double, Float>(ipol, npiece, boundary.data, ffpar_mtx);
	    /*
	    for (int ipiece = 0; ipiece < npiece; ++ipiece) {
	      ffpar_mtx[ipiece][ipol] = boundary.data[ipiece];
	    }
	    */

	    status = 
	    LIBSAKURA_SYMBOL(SubtractBaselineCubicSplineUsingCoefficientsFloat)(bl_contexts[ctx_indices[idx]],
										num_chan,
									        spec.data,
									        npiece,
									        coeff.data,
									        boundary.data,
								                spec.data);
	    check_sakura_status("sakura_SubtractBaselineCubicSplineUsingCoefficientsFloat", status);
	    LIBSAKURA_SYMBOL(StatisticsResultFloat) stat;
	    status = 
	    //LIBSAKURA_SYMBOL(ComputeAccurateStatisticsFloat)(num_chan,
	    LIBSAKURA_SYMBOL(ComputeStatisticsFloat)(num_chan,
							     spec.data,
							     mask.data,
							     &stat);
	    check_sakura_status("sakura_ComputeAccurateStatisticsFloat", status);
	    rms_mtx[0][ipol] = stat.stddev;

	    cthres_mtx[0][ipol] = clip_threshold_sigma;
	    citer_mtx[0][ipol] = (uInt)num_fitting_max;
	    lfthres_mtx[0][ipol] = 0.0;
	    lfavg_mtx[0][ipol] = 0;
	    lfedge_mtx[0][ipol] = 0;

	  } else {
	    status = 
	    LIBSAKURA_SYMBOL(SubtractBaselineCubicSplineFloat)(bl_contexts[ctx_indices[idx]], 
							       static_cast<uint16_t>(npiece), 
							       num_chan, 
							       spec.data, 
							       mask.data, 
							       clip_threshold_sigma, 
							       num_fitting_max,
							       true, 
							       mask.data, 
							       spec.data, 
							       &bl_status);
	    check_sakura_status("sakura_SubtractBaselineCubicSplineFloat", status);
	  }
  	  // set back a spectrum to data cube
	  if (do_set_spectrum_to_cube) {
	    set_spectrum_to_cube(data_chunk, irow, ipol, num_chan, spec.data);
	  }
  	} // end of polarization loop

	if (write_baseline_table) {
	  // write to baseline table
	  Array<uInt> masklist_mtx(IPosition(2, num_pol, num_masklist_max));
	  set_matrix_for_bltable<uInt, uInt>(num_pol, num_masklist_max, masklist_mtx_tmp, masklist_mtx);
	  /*
	  for (size_t ipol = 0; ipol < num_pol; ++ipol) {
	    for (size_t imask = 0; imask < num_masklist_max; ++imask) {
	      masklist_mtx[imask][ipol] = 0;
	    }
	    for (size_t imask = 0; imask < masklist_mtx_tmp[ipol].size(); ++imask) {
	      masklist_mtx[imask][ipol] = masklist_mtx_tmp[ipol][imask];
	    }
	  }
	  */
	  bt->appenddata((uInt)scans[irow], (uInt)beams[irow], (uInt)data_spw[irow],
			 0, times[irow], apply_mtx, bltype_mtx, 
			 fpar_mtx, ffpar_mtx, masklist_mtx,
			 coeff_mtx, rms_mtx, (uInt)num_chan, cthres_mtx,
			 citer_mtx, lfthres_mtx, lfavg_mtx, lfedge_mtx);
	}
      } // end of chunk row loop
      // write back data cube to VisBuffer
      sdh_->fillCubeToOutputMs(vb, data_chunk, &flag_chunk);
    } // end of vi loop
  } // end of chunk loop

  if (write_baseline_table) {
    bt->save(out_bltable_name);
    delete bt;
  }
  finalize_process();
  destroy_baseline_contexts(bl_contexts);

  //double tend = gettimeofday_sec();
  //std::cout << "Elapsed time = " << (tend - tstart) << " sec." << std::endl;
}

// --------------------------------------------------------------------
// this function is temporarily copied from sakura code to get 
// positions of cubic spline boundaries. (2015/4/6 WK)
// --------------------------------------------------------------------
void SingleDishMS::GetBoundariesOfPiecewiseData(size_t num_mask,
						bool const *mask, 
						size_t num_pieces, 
						double *boundary) {
	assert(num_pieces > 0);

	size_t num_unmasked_data = 0;
	for (size_t i = 0; i < num_mask; ++i) {
		if (mask[i])
			++num_unmasked_data;
	}
	size_t idx = 0;
	size_t count_unmasked_data = 0;
	for (size_t i = 0; i < num_mask; ++i) {
		if (idx == num_pieces)
			break;
		if (mask[i]) {
			if (count_unmasked_data
					>= static_cast<double>(num_unmasked_data * idx)
							/ static_cast<double>(num_pieces)) {
				boundary[idx] = static_cast<double>(i);
				++idx;
			}
			++count_unmasked_data;
		}
	}
}
// --------------------------------------------------------------------

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


// Baseline subtraction by per spectrum fitting parameters
void SingleDishMS::subtract_baseline_variable(string const& in_column_name,
					      string const& out_ms_name,
					      string const& out_bltable_name,
					      bool const& do_subtract,
					      string const& in_spw,
					      string const& in_ppp,
					      string const& param_file)
{
  LogIO os(_ORIGIN);
  os << "Fitting and subtracting baseline using parameters in file, " << param_file << LogIO::POST;

  // parse fitting parameters in the text file
  BLParameterParser parser(param_file);
  std::vector<LIBSAKURA_SYMBOL(BaselineType)> 
    baseline_types = parser.get_function_types();
  map<const LIBSAKURA_SYMBOL(BaselineType), uint16_t> max_orders;
  for (size_t i = 0; i < baseline_types.size(); ++i) {
    max_orders[baseline_types[i]] 
      = parser.get_max_order(baseline_types[i]);
  }
  { //DEBUG ouput
    os << LogIO::DEBUG1 << "Baseline Types = " << baseline_types << LogIO::POST;
    os << LogIO::DEBUG1 << "Max Orders:"<< LogIO::POST;
    map<const LIBSAKURA_SYMBOL(BaselineType), uint16_t>::iterator iter=max_orders.begin();
    while (iter != max_orders.end() ) {
      os << LogIO::DEBUG1 << "- type " << (*iter).first << ": " << (*iter).second << LogIO::POST;
      ++iter;
    }
  }

  // Setup VisIter for input MS
  Block<Int> columns(1);
  columns[0] = MS::DATA_DESC_ID;
  prepare_for_process(in_column_name, out_ms_name, columns, false);
  vi::VisibilityIterator2 *vi = sdh_->getVisIter();
  vi::VisBuffer2 *vb = vi->getVisBuffer();
  BaselineTable *bt = 0;
  bool write_baseline_table = (out_bltable_name != "");
  if (write_baseline_table) bt= new BaselineTable(sdh_->getMS());
  bool do_set_spectrum_to_cube = (do_subtract || !write_baseline_table);
  // SPW and channelization reservoir
  Vector<Int> recspw;
  Matrix<Int> recchan;
  Vector<size_t> nchan;
  Vector<Vector<Bool> > in_mask;
  Vector<bool> nchan_set;
  parse_spw(in_spw, recspw, recchan, nchan, in_mask, nchan_set);
  // Baseline Contexts reservoir
  // key: Sakura_BaselineType enum,
  // value: a vector of Sakura_BaselineContext for various nchans
  map< const LIBSAKURA_SYMBOL(BaselineType),
    std::vector<LIBSAKURA_SYMBOL(BaselineContext) *> > context_reservoir;
  {
    map<const LIBSAKURA_SYMBOL(BaselineType), uint16_t>::iterator
      iter = max_orders.begin();
    while (iter != max_orders.end() ) {
      context_reservoir[(*iter).first] = std::vector<LIBSAKURA_SYMBOL(BaselineContext) *>() ;
      ++iter;
    }
  }

  LIBSAKURA_SYMBOL(Status) status;
  LIBSAKURA_SYMBOL(BaselineStatus) bl_status;
  Vector<size_t> ctx_indices;
  ctx_indices.resize(nchan.nelements());
  for (size_t ictx = 0; ictx < ctx_indices.nelements(); ++ictx) {
    ctx_indices(ictx) = 0;
  }

  Vector<bool> pol;
  bool pol_set = false;
  // Iterate over MS and subtract baseline
  for (vi->originChunks(); vi->moreChunks(); vi->nextChunk()) {
    //cout << "New chunk" << endl;
    for (vi->origin(); vi->more(); vi->next()) {
      Vector<Int> scans = vb->scan();
      Vector<Double> times = vb->time();
      Vector<Int> beams = vb->feed1();

      Vector<Int> data_spw = vb->spectralWindows();
      size_t const num_chan = static_cast<size_t>(vb->nChannels());
      size_t const num_pol = static_cast<size_t>(vb->nCorrelations());
      size_t const num_row = static_cast<size_t>(vb->nRows());
      Vector<uInt> orig_rows = vb->rowIds();
      Cube<Float> data_chunk(num_pol,num_chan,num_row);
      SakuraAlignedArray<float> spec(num_chan);
      Cube<Bool> flag_chunk(num_pol,num_chan,num_row);
      SakuraAlignedArray<bool> mask(num_chan);
      //cout << "New iteration: num_row=" << num_row << ", num_chan=" << num_chan << ", num_pol=" << num_pol << ", spwid=" << data_spw << endl;

      bool new_nchan=false;
      get_nchan_and_mask(recspw, data_spw, recchan, num_chan, nchan, in_mask, nchan_set, new_nchan);
      if (new_nchan) {
	// Generate context for all necessary baseline types
	map<const LIBSAKURA_SYMBOL(BaselineType), uint16_t>::iterator
	  iter = max_orders.begin();
	while (iter != max_orders.end() ) {
	  get_baseline_context((*iter).first,(*iter).second,
			       num_chan, nchan, nchan_set, ctx_indices,
			       context_reservoir[(*iter).first]);
	  ++iter;
	}
      }
      // get data/flag cubes (npol*nchan*nrow) from VisBuffer
      get_data_cube_float(*vb, data_chunk);
      get_flag_cube(*vb, flag_chunk);

      if (!pol_set) {
	get_pol_selection(in_ppp, num_pol, pol);
	pol_set = true;
      }
      // loop over MS rows
      for (size_t irow = 0; irow < num_row; ++irow) {
	//cout << "Processing original rowid=" << orig_rows[irow] << ", spwid=" << data_spw[irow] << endl;
  	size_t idx = 0;
  	for (size_t ispw = 0; ispw < recspw.nelements(); ++ispw) {
  	  if (data_spw[irow] == recspw[ispw]) {
  	    idx = ispw;
  	    break;
  	  }
  	}

	//prepare varables for writing baseline table
	Array<Bool> apply_mtx(IPosition(2, num_pol, 1));
	Array<uInt> bltype_mtx(IPosition(2, num_pol, 1));
	Array<Int> fpar_mtx(IPosition(2, num_pol, 1));
	size_t num_ffpar_max = 0;
	std::vector<std::vector<double> > ffpar_mtx_tmp(num_pol);
	size_t num_masklist_max = 0;
	std::vector<std::vector<uInt> > masklist_mtx_tmp(num_pol);
	size_t num_coeff_max = 0;
	std::vector<std::vector<double> > coeff_mtx_tmp(num_pol);
	Array<Float> rms_mtx(IPosition(2, num_pol, 1));
	Array<Float> cthres_mtx(IPosition(2, num_pol, 1));
	Array<uInt> citer_mtx(IPosition(2, num_pol, 1));
	Array<Float> lfthres_mtx(IPosition(2, num_pol, 1));
	Array<uInt> lfavg_mtx(IPosition(2, num_pol, 1));
	Array<uInt> lfedge_mtx(IPosition(2, num_pol, 1));

  	// loop over polarization
  	for (size_t ipol = 0; ipol < num_pol; ++ipol) {
	  apply_mtx[0][ipol] = True;
	  // skip spectrum not selected by pol
	  if (!pol(ipol)) {
	    flag_spectrum_in_cube(flag_chunk,irow,ipol); //flag
	    apply_mtx[0][ipol] = False;
	    continue;
	  }
  	  // get a channel mask from data cube
  	  // (note that the variable 'mask' is flag in the next line 
	  // actually, then it will be converted to real mask when 
	  // taking AND with user-given mask info. this is just for 
	  // saving memory usage...)
  	  get_flag_from_cube(flag_chunk, irow, ipol, num_chan, mask);
	  // skip spectrum if all channels flagged
	  if (allchannels_flagged(num_chan, mask.data)) {
	    apply_mtx[0][ipol] = False;
	    continue;
	  }

  	  // convert flag to mask by taking logical NOT of flag
  	  // and then operate logical AND with in_mask
  	  for (size_t ichan = 0; ichan < num_chan; ++ichan) {
  	    mask.data[ichan] = in_mask[idx][ichan] && (!(mask.data[ichan]));
  	  }
	  // get fitting parameter
	  BLParameterSet fit_param;
	  if (!parser.GetFitParameter(orig_rows[irow],ipol,fit_param))
	  { //no fit requrested
	    flag_spectrum_in_cube(flag_chunk,irow,ipol);
	    os << LogIO::DEBUG1 << "Row " << orig_rows[irow]
	       << ", Pol " << ipol
	       << ": Fit not requested. Skipping." << LogIO::POST;
	    continue;
	  }
	  if (true) {
	    os << "Fitting Parameter" << LogIO::POST;
	    os << "[ROW" << orig_rows[irow] << ", POL" << ipol << "]"
	       << LogIO::POST;
	    fit_param.PrintSummary();
	  }
	  // get mask from BLParameterset and create composit mask
	  if (fit_param.baseline_mask!="") {
	    stringstream local_spw;
	    local_spw << data_spw[irow] << ":" << fit_param.baseline_mask;
	    //cout << "row " << orig_rows[irow] << ": Generating local mask with selection " << local_spw.str() << endl;
	    Record selrec = sdh_->getSelRec(local_spw.str());
	    Matrix<Int> local_rec_chan = selrec.asArrayInt("channel");
	    Vector<Bool> local_mask(num_chan,False);
	    get_mask_from_rec(data_spw[irow], local_rec_chan, local_mask, false);
	    for (size_t ichan = 0; ichan < num_chan; ++ichan) {
	      mask.data[ichan] = mask.data[ichan] && local_mask[ichan];
	    }
	  }
	  // check for composit mask and flag if no valid channel to fit
	  if (NValidMask(num_chan, mask.data)==0) {
	    flag_spectrum_in_cube(flag_chunk,irow,ipol);
	    os << LogIO::DEBUG1 << "Row " << orig_rows[irow]
	       << ", Pol " << ipol
	       << ": No valid channel to fit. Skipping" << LogIO::POST;
	    continue;
	  }
  	  // get a spectrum from data cube
  	  get_spectrum_from_cube(data_chunk, irow, ipol, num_chan, spec);

  	  // actual execution of single spectrum
	  map< const LIBSAKURA_SYMBOL(BaselineType),
	    std::vector<LIBSAKURA_SYMBOL(BaselineContext) *> >::iterator
	    iter = context_reservoir.find(fit_param.baseline_type);
	  if (iter==context_reservoir.end())
	    throw(AipsError("Invalid baseline type detected!"));
	  LIBSAKURA_SYMBOL(BaselineContext)* context = (*iter).second[ctx_indices[idx]];
	  //cout << "Got context for type " << (*iter).first << ": idx=" << ctx_indices[idx] << endl;

	  if (write_baseline_table) {
	    bltype_mtx[0][ipol] = (uInt)fit_param.baseline_type;
	    Int fpar_tmp;
	    switch (fit_param.baseline_type) {
	    case LIBSAKURA_SYMBOL(BaselineType_kPolynomial):
	    case LIBSAKURA_SYMBOL(BaselineType_kChebyshev):
	      fpar_tmp = (Int)fit_param.order;
	      break;
	    case LIBSAKURA_SYMBOL(BaselineType_kCubicSpline):
	      fpar_tmp = (Int)fit_param.npiece;
	      break;
	    default:
	      throw(AipsError("Unsupported baseline type."));
	    }
	    fpar_mtx[0][ipol] = fpar_tmp;

	    size_t num_coeff;
	    switch (fit_param.baseline_type) {
	    case LIBSAKURA_SYMBOL(BaselineType_kPolynomial):
	    case LIBSAKURA_SYMBOL(BaselineType_kChebyshev):
	      status = 
	      LIBSAKURA_SYMBOL(GetNumberOfCoefficients)(context, fit_param.order, &num_coeff);
	      check_sakura_status("sakura_GetNumberOfCoefficients", status);
	      break;
	    case LIBSAKURA_SYMBOL(BaselineType_kCubicSpline):
	      num_coeff = 4 * fit_param.npiece;
	      break;
	    default:
	      throw(AipsError("Unsupported baseline type."));
	    }
	    if (num_coeff > num_coeff_max) {
	      num_coeff_max = num_coeff;
	    }
	    SakuraAlignedArray<double> coeff(num_coeff);
	    string get_coeff_funcname;
	    switch (fit_param.baseline_type) {
	    case LIBSAKURA_SYMBOL(BaselineType_kPolynomial):
	    case LIBSAKURA_SYMBOL(BaselineType_kChebyshev):
	      status = 
	      LIBSAKURA_SYMBOL(GetBestFitBaselineCoefficientsFloat)(context, 
								    num_chan,
								    spec.data, 
								    mask.data,
								    fit_param.clip_threshold_sigma,
							            fit_param.num_fitting_max, 
							            num_coeff, 
							            coeff.data,
							            mask.data,
							            &bl_status);
	      get_coeff_funcname = "sakura_GetBestFitBaselineCoefficientsFloat";
	      break;
	    case LIBSAKURA_SYMBOL(BaselineType_kCubicSpline):
	      status = 
	      LIBSAKURA_SYMBOL(GetBestFitBaselineCoefficientsCubicSplineFloat)(context, 
									       num_chan,
									       spec.data, 
									       mask.data,
									       fit_param.clip_threshold_sigma,
									       fit_param.num_fitting_max, 
									       fit_param.npiece, 
									       coeff.data,
									       mask.data,
									       &bl_status);
	      get_coeff_funcname = "sakura_GetBestFitBaselineCoefficientsCubicSplineFloat";
	      break;
	    default:
	      throw(AipsError("Unsupported baseline type."));
	    }
	    check_sakura_status(get_coeff_funcname, status);
	    for (size_t icoeff = 0; icoeff < num_coeff; ++icoeff) {
	      coeff_mtx_tmp[ipol].push_back(coeff.data[icoeff]);
	    }
	    Vector<uInt> masklist;
	    get_masklist_from_mask(num_chan, mask.data, masklist);
	    if (masklist.size() > num_masklist_max) {
	      num_masklist_max = masklist.size();
	    }
	    masklist_mtx_tmp[ipol].clear();
	    for (size_t imask = 0; imask < masklist.size(); ++imask) {
	      masklist_mtx_tmp[ipol].push_back(masklist[imask]);
	    }

	    // ***workaround***
	    // get piece boundary positions
	    // as sakura_GetBestFitBaselineCoefficientsCubicSplineFloat 
	    // does not return boundary values now
	    size_t num_boundary = 0;
	    if (fit_param.baseline_type == LIBSAKURA_SYMBOL(BaselineType_kCubicSpline)) {
	      num_boundary = fit_param.npiece;
	    }
	    SakuraAlignedArray<double> boundary(num_boundary);
	    if (fit_param.baseline_type == LIBSAKURA_SYMBOL(BaselineType_kCubicSpline)) {
	      GetBoundariesOfPiecewiseData(num_chan, mask.data, fit_param.npiece, boundary.data);
	    }
	    // ***end workaround***

	    string subtract_funcname;
	    switch (fit_param.baseline_type) {
	    case LIBSAKURA_SYMBOL(BaselineType_kPolynomial):
	    case LIBSAKURA_SYMBOL(BaselineType_kChebyshev):
	      status = 
	      LIBSAKURA_SYMBOL(SubtractBaselineUsingCoefficientsFloat)(context,
								       num_chan,
								       spec.data,
								       num_coeff,
								       coeff.data,
								       spec.data);
	      subtract_funcname = "sakura_SubtractBaselineUsingCoefficientsFloat";
	      break;
	    case LIBSAKURA_SYMBOL(BaselineType_kCubicSpline):
	      status = 
	      LIBSAKURA_SYMBOL(SubtractBaselineCubicSplineUsingCoefficientsFloat)(context,
										  num_chan,
										  spec.data,
										  fit_param.npiece,
										  coeff.data,
									          boundary.data,
									          spec.data);
	      subtract_funcname = "sakura_SubtractBaselineCubicSplineUsingCoefficientsFloat";
	      break;
	    default:
	      throw(AipsError("Unsupported baseline type."));
	    }
	    check_sakura_status(subtract_funcname, status);

	    size_t num_ffpar = 0;
	    switch (fit_param.baseline_type) {
	    case LIBSAKURA_SYMBOL(BaselineType_kPolynomial):
	    case LIBSAKURA_SYMBOL(BaselineType_kChebyshev):
	      break;
	    case LIBSAKURA_SYMBOL(BaselineType_kCubicSpline):
	      num_ffpar = fit_param.npiece;
	      break;
	    default:
	      throw(AipsError("Unsupported baseline type."));
	    }
	    if (num_ffpar > num_ffpar_max) {
	      num_ffpar_max = num_ffpar;
	    }
	    ffpar_mtx_tmp[ipol].clear();
	    if (fit_param.baseline_type == LIBSAKURA_SYMBOL(BaselineType_kCubicSpline)) {
	      for (size_t ipiece = 0; ipiece < fit_param.npiece; ++ipiece) {
		ffpar_mtx_tmp[ipol].push_back(boundary.data[ipiece]);
	      }
	    }

	    LIBSAKURA_SYMBOL(StatisticsResultFloat) stat;
	    status = 
	    //LIBSAKURA_SYMBOL(ComputeAccurateStatisticsFloat)(num_chan,
	    LIBSAKURA_SYMBOL(ComputeStatisticsFloat)(num_chan,
						             spec.data,
 							     mask.data,
 							     &stat);
	    check_sakura_status("sakura_ComputeAccurateStatisticsFloat", status);
	    rms_mtx[0][ipol] = stat.stddev;

	    cthres_mtx[0][ipol] = fit_param.clip_threshold_sigma;
	    citer_mtx[0][ipol] = (uInt)fit_param.num_fitting_max;
	    lfthres_mtx[0][ipol] = 0.0;
	    lfavg_mtx[0][ipol] = 0;
	    lfedge_mtx[0][ipol] = 0;

	  } else {
	    string subtract_funcname;
	    switch (fit_param.baseline_type) {
	    case LIBSAKURA_SYMBOL(BaselineType_kPolynomial):
	    case LIBSAKURA_SYMBOL(BaselineType_kChebyshev):
	      //cout << (fit_param.baseline_type==0 ? "poly" : "chebyshev") << ": order=" << fit_param.order << ", row=" << orig_rows[irow] << ", pol=" << ipol << ", num_chan=" << num_chan << ", num_valid_chan = " << NValidMask(num_chan, mask.data) << endl;
	      status = 
	      LIBSAKURA_SYMBOL(SubtractBaselineFloat)(context, 
		  				      fit_param.order, 
						      num_chan, 
						      spec.data, 
						      mask.data, 
						      fit_param.clip_threshold_sigma, 
						      fit_param.num_fitting_max,
						      true, 
						      mask.data, 
						      spec.data, 
						      &bl_status);
	      subtract_funcname = "sakura_SubtractBaselineFloat";
	      break;
	    case LIBSAKURA_SYMBOL(BaselineType_kCubicSpline):
	      //cout << "cspline: npiece = " << fit_param.npiece << ", row=" << orig_rows[irow] << ", pol=" << ipol << ", num_chan=" << num_chan << ", num_valid_chan = " << NValidMask(num_chan, mask.data) << endl;
	      status = 
	      LIBSAKURA_SYMBOL(SubtractBaselineCubicSplineFloat)(context, 
								 fit_param.npiece,
								 num_chan,
								 spec.data,
								 mask.data,
								 fit_param.clip_threshold_sigma, 
								 fit_param.num_fitting_max,
								 true,
								 mask.data,
								 spec.data,
								 &bl_status);
	      subtract_funcname = "sakura_SubtractBaselineCubicSplineFloat";
	      break;
	    default:
	      throw(AipsError("Unsupported baseline type."));
	    }
	    check_sakura_status(subtract_funcname, status);
	    /*
	    //cout << "statistics of baselined spetrum" << endl;
	    float maxval = -1000.;
	    float minval = 1000.;
	    for (size_t i = 0; i < num_chan; ++i){
	      maxval = max(maxval, spec.data[i]);
	      minval = min(minval, spec.data[i]);
	    }
	    //cout << "- max=" << maxval << ", min=" << minval << endl;
	    */
	  }
  	  // set back a spectrum to data cube
	  if (do_set_spectrum_to_cube) {
	    set_spectrum_to_cube(data_chunk, irow, ipol, num_chan, spec.data);
	  }
  	} // end of polarization loop

	if (write_baseline_table) {
	  //write to baseline table
	  Array<Float> ffpar_mtx(IPosition(2, num_pol, num_ffpar_max));
	  set_matrix_for_bltable<double, Float>(num_pol, num_ffpar_max, ffpar_mtx_tmp, ffpar_mtx);
	  /*
	  for (size_t ipol = 0; ipol < num_pol; ++ipol) {
	    for (size_t ipiece = 0; ipiece < num_ffpar_max; ++ipiece) {
	      ffpar_mtx[ipiece][ipol] = 0.0;
            }
	    for (size_t ipiece = 0; ipiece < ffpar_mtx_tmp[ipol].size(); ++ipiece) {
	      ffpar_mtx[ipiece][ipol] = ffpar_mtx_tmp[ipol][ipiece];
	    }
          }
	  */
	  Array<uInt> masklist_mtx(IPosition(2, num_pol, num_masklist_max));
	  set_matrix_for_bltable<uInt, uInt>(num_pol, num_masklist_max, masklist_mtx_tmp, masklist_mtx);
	  /*
	  for (size_t ipol = 0; ipol < num_pol; ++ipol) {
	    for (size_t imask = 0; imask < num_masklist_max; ++imask) {
	      masklist_mtx[imask][ipol] = 0;
	    }
	    for (size_t imask = 0; imask < masklist_mtx_tmp[ipol].size(); ++imask) {
	      masklist_mtx[imask][ipol] = masklist_mtx_tmp[ipol][imask];
	    }
	  }
	  */
	  Array<Float> coeff_mtx(IPosition(2, num_pol, num_coeff_max));
	  set_matrix_for_bltable<double, Float>(num_pol, num_coeff_max, coeff_mtx_tmp, coeff_mtx);
	  /*
  	  for (size_t ipol = 0; ipol < num_pol; ++ipol) {
	    for (size_t icoeff = 0; icoeff < num_coeff_max; ++icoeff) {
	      coeff_mtx[icoeff][ipol] = 0.0;
	    }
	    for (size_t icoeff = 0; icoeff < coeff_mtx_tmp[ipol].size(); ++icoeff) {
	      coeff_mtx[icoeff][ipol] = coeff_mtx_tmp[ipol][icoeff];
	    }
	  }
	  */
	  bt->appenddata((uInt)scans[irow], (uInt)beams[irow], (uInt)data_spw[irow],
			 0, times[irow], apply_mtx, bltype_mtx, 
			 fpar_mtx, ffpar_mtx, masklist_mtx,
			 coeff_mtx, rms_mtx, (uInt)num_chan, cthres_mtx,
			 citer_mtx, lfthres_mtx, lfavg_mtx, lfedge_mtx);
	}
      } // end of chunk row loop
      // write back data and flag cube to VisBuffer
      sdh_->fillCubeToOutputMs(vb, data_chunk, &flag_chunk);
    } // end of vi loop
  } // end of chunk loop

  if (write_baseline_table) {
    bt->save(out_bltable_name);
    delete bt;
  }
  finalize_process();
  // destroy baseline contexts
  map< const LIBSAKURA_SYMBOL(BaselineType), std::vector<LIBSAKURA_SYMBOL(BaselineContext) *> >::iterator ctxiter = context_reservoir.begin();
  while (ctxiter != context_reservoir.end() ) {
    destroy_baseline_contexts(context_reservoir[(*ctxiter).first]);
    ++ctxiter;
  }
}

}  // End of casa namespace.
