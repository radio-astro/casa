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
#include <casa/Utilities/GenSort.h>
#include <casa/Arrays/ArrayMath.h>
#include <ms/MeasurementSets/MSSpectralWindow.h>
#include <ms/MSSel/MSSelection.h>
#include <ms/MSSel/MSSelectionTools.h>
#include <msvis/MSVis/VisibilityIterator2.h>
#include <msvis/MSVis/VisSetUtil.h>
#include <scimath/Mathematics/Convolver.h>
#include <scimath/Mathematics/VectorKernel.h>
#include <singledish/SingleDish/SingleDishMS.h>
#include <singledish/SingleDish/BaselineTable.h>
#include <singledish/SingleDish/BLParameterParser.h>
#include <singledish/SingleDish/LineFinder.h>
#include <stdcasa/StdCasa/CasacSupport.h>

#include <casa/Arrays/Vector.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/Allocator.h>
#include <scimath/Fitting/GenericL2Fit.h>
#include <scimath/Fitting/NonLinearFitLM.h>
#include <scimath/Functionals/Function.h>
#include <scimath/Functionals/CompiledFunction.h>
#include <scimath/Functionals/CompoundFunction.h>
#include <scimath/Functionals/Gaussian1D.h>

#include <tables/Tables/ScalarColumn.h>
#include <casa/Quanta/MVTime.h>

#define _ORIGIN LogOrigin("SingleDishMS", __func__, WHERE)

namespace {
//---for measuring elapse time------------------------
#include <sys/time.h>
double gettimeofday_sec() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec + (double) tv.tv_usec * 1.0e-6;
}
//----------------------------------------------------

using casa::vi::VisBuffer2;
using casa::Matrix;
using casa::Cube;
using casa::Float;
using casa::Complex;
using casa::AipsError;

template<class CUBE_ACCESSOR>
struct DataAccessorInterface {
  static void GetCube(VisBuffer2 const &vb, Cube<Float> &cube) {
    real(cube, CUBE_ACCESSOR::GetVisCube(vb));
  }
  static void GetSlice(VisBuffer2 const &vb, size_t const iPol,
      Matrix<Float> &cubeSlice) {
    real(cubeSlice, CUBE_ACCESSOR::GetVisCube(vb).yzPlane(iPol));
  }
};

struct DataAccessor: public DataAccessorInterface<DataAccessor> {
  static Cube<Complex> GetVisCube(VisBuffer2 const &vb) {
    return vb.visCube();
  }
};

struct CorrectedDataAccessor: public DataAccessorInterface<CorrectedDataAccessor> {
  static Cube<Complex> GetVisCube(VisBuffer2 const &vb) {
    return vb.visCubeCorrected();
  }
};

struct FloatDataAccessor {
  static void GetCube(VisBuffer2 const &vb, Cube<Float> &cube) {
    cube = GetVisCube(vb);
  }
  static void GetSlice(VisBuffer2 const &vb, size_t const iPol,
      Matrix<Float> &cubeSlice) {
    cubeSlice = GetVisCube(vb).yzPlane(iPol);
  }
private:
  static Cube<Float> GetVisCube(VisBuffer2 const &vb) {
    return vb.visCubeFloat();
  }
};

inline void GetCubeFromData(VisBuffer2 const &vb, Cube<Float> &cube) {
  DataAccessor::GetCube(vb, cube);
}

inline void GetCubeFromCorrected(VisBuffer2 const &vb, Cube<Float> &cube) {
  CorrectedDataAccessor::GetCube(vb, cube);
}

inline void GetCubeFromFloat(VisBuffer2 const &vb, Cube<Float> &cube) {
  FloatDataAccessor::GetCube(vb, cube);
}

inline void GetCubeDefault(VisBuffer2 const &vb, Cube<Float> &cube) {
  throw AipsError("Data accessor for VB2 is not properly configured.");
}
} // anonymous namespace

namespace casa {

SingleDishMS::SingleDishMS() :
    msname_(""), sdh_(0) {
  initialize();
}

SingleDishMS::SingleDishMS(string const& ms_name) :
    msname_(ms_name), sdh_(0) {
  LogIO os(_ORIGIN);
  initialize();
}

SingleDishMS::~SingleDishMS() {
  if (sdh_) {
    delete sdh_;
    sdh_ = 0;
  }
  msname_ = "";
}

void SingleDishMS::initialize() {
  in_column_ = MS::UNDEFINED_COLUMN;
  //  out_column_ = MS::UNDEFINED_COLUMN;
  doSmoothing_ = False;
  visCubeAccessor_ = GetCubeDefault;
}

bool SingleDishMS::close() {
  LogIO os(_ORIGIN);
  os << "Detaching from SingleDishMS" << LogIO::POST;

  if (sdh_) {
    delete sdh_;
    sdh_ = 0;
  }
  msname_ = "";

  return True;
}

////////////////////////////////////////////////////////////////////////
///// Common utility functions
////////////////////////////////////////////////////////////////////////
void SingleDishMS::setSelection(Record const &selection, bool const verbose) {
  LogIO os(_ORIGIN);
  if (!selection_.empty()) // selection is set before
    os << "Discard old selection and setting new one." << LogIO::POST;
  if (selection.empty()) // empty selection is passed
    os << "Resetting selection." << LogIO::POST;

  selection_ = selection;
  // Verbose output
  bool any_selection(false);
  if (verbose && !selection_.empty()) {
    String timeExpr(""), antennaExpr(""), fieldExpr(""), spwExpr(""),
        uvDistExpr(""), taQLExpr(""), polnExpr(""), scanExpr(""), arrayExpr(""),
        obsExpr(""), intentExpr("");
    timeExpr = get_field_as_casa_string(selection_, "timerange");
    antennaExpr = get_field_as_casa_string(selection_, "baseline");
    fieldExpr = get_field_as_casa_string(selection_, "field");
    spwExpr = get_field_as_casa_string(selection_, "spw");
    uvDistExpr = get_field_as_casa_string(selection_, "uvdist");
    taQLExpr = get_field_as_casa_string(selection_, "taql");
    polnExpr = get_field_as_casa_string(selection_, "correlation");
    scanExpr = get_field_as_casa_string(selection_, "scan");
    arrayExpr = get_field_as_casa_string(selection_, "array");
    intentExpr = get_field_as_casa_string(selection_, "intent");
    obsExpr = get_field_as_casa_string(selection_, "observation");
    // selection Summary
    os << "[Selection Summary]" << LogIO::POST;
    if (obsExpr != "") {
      any_selection = true;
      os << "- Observation: " << obsExpr << LogIO::POST;
    }
    if (antennaExpr != "") {
      any_selection = true;
      os << "- Antenna: " << antennaExpr << LogIO::POST;
    }
    if (fieldExpr != "") {
      any_selection = true;
      os << "- Field: " << fieldExpr << LogIO::POST;
    }
    if (spwExpr != "") {
      any_selection = true;
      os << "- SPW: " << spwExpr << LogIO::POST;
    }
    if (polnExpr != "") {
      any_selection = true;
      os << "- Pol: " << polnExpr << LogIO::POST;
    }
    if (scanExpr != "") {
      any_selection = true;
      os << "- Scan: " << scanExpr << LogIO::POST;
    }
    if (timeExpr != "") {
      any_selection = true;
      os << "- Time: " << timeExpr << LogIO::POST;
    }
    if (intentExpr != "") {
      any_selection = true;
      os << "- Intent: " << intentExpr << LogIO::POST;
    }
    if (arrayExpr != "") {
      any_selection = true;
      os << "- Array: " << arrayExpr << LogIO::POST;
    }
    if (uvDistExpr != "") {
      any_selection = true;
      os << "- UVDist: " << uvDistExpr << LogIO::POST;
    }
    if (taQLExpr != "") {
      any_selection = true;
      os << "- TaQL: " << taQLExpr << LogIO::POST;
    }
    if (!any_selection)
      os << "No valid selection parameter is set." << LogIO::WARN;
  }
}

void SingleDishMS::setAverage(Record const &average, bool const verbose) {
  LogIO os(_ORIGIN);
  if (!average_.empty()) // average is set before
    os << "Discard old average and setting new one." << LogIO::POST;
  if (average.empty()) // empty average is passed
    os << "Resetting average." << LogIO::POST;

  average_ = average;

  if (verbose && !average_.empty()) {
    LogIO os(_ORIGIN);
    Int ifield;
    ifield = average_.fieldNumber(String("timeaverage"));
    os << "[Averaging Settings]" << LogIO::POST;
    if (ifield < 0 || !average_.asBool(ifield)) {
      os << "No averaging will be done" << LogIO::POST;
      return;
    }

    String timebinExpr(""), timespanExpr(""), tweightExpr("");
    timebinExpr = get_field_as_casa_string(average_, "timebin");
    timespanExpr = get_field_as_casa_string(average_, "timespan");
    tweightExpr = get_field_as_casa_string(average_, "tweight");
    if (timebinExpr != "") {
      os << "- Time bin: " << timebinExpr << LogIO::POST;
    }
    if (timespanExpr != "") {
      os << "- Time span: " << timespanExpr << LogIO::POST;
    }
    if (tweightExpr != "") {
      os << "- Averaging weight: " << tweightExpr << LogIO::POST;
    }

  }
}

String SingleDishMS::get_field_as_casa_string(Record const &in_data,
    string const &field_name) {
  Int ifield;
  ifield = in_data.fieldNumber(String(field_name));
  if (ifield > -1)
    return in_data.asString(ifield);
  return "";
}

bool SingleDishMS::prepare_for_process(string const &in_column_name,
    string const &out_ms_name) {
  // Sort by single dish default
  return prepare_for_process(in_column_name, out_ms_name, Block<Int>(), true);
}

bool SingleDishMS::prepare_for_process(string const &in_column_name,
    string const &out_ms_name, Block<Int> const &sortColumns,
    bool const addDefaultSortCols) {
  LogIO os(_ORIGIN);
  AlwaysAssert(msname_ != "", AipsError);
  // define a column to read data from
  if (in_column_name == "float_data") {
    in_column_ = MS::FLOAT_DATA;
    visCubeAccessor_ = GetCubeFromFloat;
  } else if (in_column_name == "corrected") {
    in_column_ = MS::CORRECTED_DATA;
    visCubeAccessor_ = GetCubeFromCorrected;
  } else if (in_column_name == "data") {
    in_column_ = MS::DATA;
    visCubeAccessor_ = GetCubeFromData;
  } else {
    throw(AipsError("Invalid data column name"));
  }
  // destroy SDMSManager
  if (sdh_)
    delete sdh_;
  // Configure record
  Record configure_param(selection_);
  format_selection(configure_param);
  configure_param.define("inputms", msname_);
  configure_param.define("outputms", out_ms_name);
  String in_name(in_column_name);
  in_name.upcase();
  configure_param.define("datacolumn", in_name);
  // merge averaging parameters
  configure_param.merge(average_);
  // The other available keys
  // - buffermode, realmodelcol, usewtspectrum, tileshape,
  // - chanaverage, chanbin, useweights, 
  // - combinespws, ddistart, hanning
  // - regridms, phasecenter, restfreq, outframe, interpolation, nspw,
  // - mode, nchan, start, width, veltype,
  // - timeaverage, timebin, timespan, maxuvwdistance

  // smoothing
  configure_param.define("smoothFourier", doSmoothing_);

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
  // set large timebin if not averaging
  Double timeBin;
  int exists = configure_param.fieldNumber("timebin");
  if (exists < 0) {
    timeBin = 1.0e8;
  } else {
    String timebin_string;
    configure_param.get(exists, timebin_string);
    timeBin = casaQuantity(timebin_string).get("s").getValue();

    Int ifield;
    ifield = configure_param.fieldNumber(String("timeaverage"));
    Bool average_time = ifield < 0 ? False : configure_param.asBool(ifield);
    if (timeBin < 0 || (average_time && timeBin == 0.0))
      throw(AipsError("time bin should be positive"));
  }
  // set sort column
  sdh_->setSortColumns(sortColumns, addDefaultSortCols, timeBin);
  // Set up the Data Handler
  sdh_->setup();
  return true;
}

void SingleDishMS::finalize_process() {
  initialize();
  if (sdh_) {
    sdh_->close();
    delete sdh_;
    sdh_ = 0;
  }
}

void SingleDishMS::format_selection(Record &selection) {
  // At this moment sdh_ is not supposed to be generated yet.
  LogIO os(_ORIGIN);
  // format spw
  String const spwSel(get_field_as_casa_string(selection, "spw"));
  selection.define("spw", spwSel == "" ? "*" : spwSel);

  // Select only auto-correlation
  String autoCorrSel("");
  os << "Formatting antenna selection to select only auto-correlation"
      << LogIO::POST;
  String const antennaSel(get_field_as_casa_string(selection, "baseline"));
  os << LogIO::DEBUG1 << "Input antenna expression = " << antennaSel
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
    for (uInt i = 0; i < ant1Vec.nelements(); ++i) {
      os << ant1Vec[i] << ", ";
      if (autoCorrSel != "")
        autoCorrSel += ";";
      autoCorrSel += String::toString(ant1Vec[i]) + "&&&";
    }
    os << LogIO::POST;
  }
  os << LogIO::DEBUG1 << "Auto-correlation selection string: " << autoCorrSel
      << LogIO::POST;
  selection.define("baseline", autoCorrSel);

}

void SingleDishMS::get_data_cube_float(vi::VisBuffer2 const &vb,
    Cube<Float> &data_cube) {
//  if (in_column_ == MS::FLOAT_DATA) {
//    data_cube = vb.visCubeFloat();
//  } else { //need to convert Complex cube to Float
//    Cube<Complex> cdata_cube(data_cube.shape());
//    if (in_column_ == MS::DATA) {
//      cdata_cube = vb.visCube();
//    } else { //MS::CORRECTED_DATA
//      cdata_cube = vb.visCubeCorrected();
//    }
//    // convert Complext to Float
//    convertArrayC2F(data_cube, cdata_cube);
//  }
  (*visCubeAccessor_)(vb, data_cube);
}

void SingleDishMS::convertArrayC2F(Array<Float> &to,
    Array<Complex> const &from) {
  if (to.nelements() == 0 && from.nelements() == 0) {
    return;
  }
  if (to.shape() != from.shape()) {
    throw(ArrayConformanceError("Array shape differs"));
  }
  Array<Complex>::const_iterator endFrom = from.end();
  Array<Complex>::const_iterator iterFrom = from.begin();
  for (Array<Float>::iterator iterTo = to.begin(); iterFrom != endFrom;
      ++iterFrom, ++iterTo) {
    *iterTo = iterFrom->real();
  }
}

std::vector<string> SingleDishMS::split_string(string const &s, char delim) {
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

bool SingleDishMS::file_exists(string const &filename) {
  FILE *fp;
  if ((fp = fopen(filename.c_str(), "r")) == NULL)
    return false;
  fclose(fp);
  return true;
}

void SingleDishMS::parse_spw(string const &in_spw, Vector<Int> &rec_spw,
    Matrix<Int> &rec_chan, Vector<size_t> &nchan, Vector<Vector<Bool> > &mask,
    Vector<bool> &nchan_set) {
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
    Vector<Int> const &data_spw, Matrix<Int> const &rec_chan,
    size_t const num_chan, Vector<size_t> &nchan, Vector<Vector<Bool> > &mask,
    Vector<bool> &nchan_set, bool &new_nchan) {
  new_nchan = false;
  for (size_t i = 0; i < rec_spw.nelements(); ++i) {
    //get nchan by spwid and set to nchan[]
    for (size_t j = 0; j < data_spw.nelements(); ++j) {
      if ((!nchan_set(i)) && (data_spw(j) == rec_spw(i))) {
        bool found = false;
        for (size_t k = 0; k < nchan.nelements(); ++k) {
          if (!nchan_set(k))
            continue;
          if (nchan(k) == num_chan)
            found = true;
        }
        if (!found) {
          new_nchan = true;
        }
        nchan(i) = num_chan;
        nchan_set(i) = true;
        break;
      }
    }
    if (!nchan_set(i))
      continue;
    mask(i).resize(nchan(i));
    // generate mask
    get_mask_from_rec(rec_spw(i), rec_chan, mask(i), true);
  }
}

void SingleDishMS::get_mask_from_rec(Int spwid, Matrix<Int> const &rec_chan,
    Vector<Bool> &mask, bool initialize) {
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
  for (size_t j = 0; j < edge.size(); j += 3) {
    for (size_t k = edge[j]; k <= edge[j + 1]; k += edge[j + 2]) {
      mask(k) = True;
    }
  }
}

void SingleDishMS::get_masklist_from_mask(size_t const num_chan,
    bool const *mask, Vector<uInt> &masklist) {
  std::vector<int> mlist;
  mlist.clear();

  for (uInt i = 0; i < num_chan; ++i) {
    if (mask[i]) {
      if ((i == 0) || (i == num_chan - 1)) {
        mlist.push_back(i);
      } else {
        if ((mask[i]) && (!mask[i - 1])) {
          mlist.push_back(i);
        }
        if ((mask[i]) && (!mask[i + 1])) {
          mlist.push_back(i);
        }
      }
    }
  }

  masklist.resize(mlist.size());
  for (size_t i = 0; i < masklist.size(); ++i) {
    masklist[i] = (uInt) mlist[i];
  }
}

void SingleDishMS::get_pol_selection(string const &in_pol, size_t const num_pol,
    Vector<bool> &pol) {
  pol.resize(num_pol);
  bool pol_val = (in_pol == "") || (in_pol == "*");
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
        if (in_pol_list[j] == (int) i) {
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
        static_cast<uint16_t>(order),
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
    size_t const row, size_t const plane, size_t const num_data,
    float out_data[]) {
  AlwaysAssert(static_cast<size_t>(data_cube.ncolumn()) == num_data, AipsError);
  for (size_t i = 0; i < num_data; ++i)
    out_data[i] = static_cast<float>(data_cube(plane, i, row));
}

void SingleDishMS::set_spectrum_to_cube(Cube<Float> &data_cube,
    size_t const row, size_t const plane, size_t const num_data,
    float *in_data) {
  AlwaysAssert(static_cast<size_t>(data_cube.ncolumn()) == num_data, AipsError);
  for (size_t i = 0; i < num_data; ++i)
    data_cube(plane, i, row) = static_cast<Float>(in_data[i]);
}

void SingleDishMS::get_flag_cube(vi::VisBuffer2 const &vb,
    Cube<Bool> &flag_cube) {
  flag_cube = vb.flagCube();
}

void SingleDishMS::get_flag_from_cube(Cube<Bool> &flag_cube, size_t const row,
    size_t const plane, size_t const num_flag,
    bool out_flag[]) {
  AlwaysAssert(static_cast<size_t>(flag_cube.ncolumn()) == num_flag, AipsError);
  for (size_t i = 0; i < num_flag; ++i)
    out_flag[i] = static_cast<bool>(flag_cube(plane, i, row));
}

void SingleDishMS::set_flag_to_cube(Cube<Bool> &flag_cube, size_t const row,
    size_t const plane, size_t const num_flag, bool *in_flag) {
  AlwaysAssert(static_cast<size_t>(flag_cube.ncolumn()) == num_flag, AipsError);
  for (size_t i = 0; i < num_flag; ++i)
    flag_cube(plane, i, row) = static_cast<Bool>(in_flag[i]);
}

void SingleDishMS::flag_spectrum_in_cube(Cube<Bool> &flag_cube,
    size_t const row, size_t const plane) {
  uInt const num_flag = flag_cube.ncolumn();
  for (uInt ichan = 0; ichan < num_flag; ++ichan)
    flag_cube(plane, ichan, row) = True;
}

bool SingleDishMS::allchannels_flagged(size_t const num_flag,
    bool const* flag) {
  bool res = true;
  for (size_t i = 0; i < num_flag; ++i) {
    if (!flag[i]) {
      res = false;
      break;
    }
  }
  return res;
}

size_t SingleDishMS::NValidMask(size_t const num_mask, bool const* mask) {
  std::size_t nvalid = 0;
  // the assertion lines had better be replaced with static_assert when c++11 is supported
  AlwaysAssert(static_cast<std::size_t>(true) == 1, AipsError);
  AlwaysAssert(static_cast<std::size_t>(false) == 0, AipsError);
  for (size_t i = 0; i < num_mask; ++i) {
    nvalid += static_cast<std::size_t>(mask[i]);
  }
  return nvalid;
}

//void SingleDishMS::extract_outputname(string blform, string blout, string* name0, string* name1, string* name2)
//{

//}

void SingleDishMS::split_bloutputname(string str) {

  char key = ',';
  vector<size_t> v;
  for (size_t i = 0; i < str.size(); ++i) {
    char target = str[i];
    if (key == target) {
      v.push_back(i);
    }
  }

  //cout << "comma " << v.size() << endl;
  //cout << "v[1] " << v[1] << endl;
  //cout <<  "v.size()-1 " <<  v.size()-1 << endl;
  //cout << "v[1]+1 " << v[1]+1 << endl;
  //cout << "str.size()-v[1]-1 " << str.size()-v[1]-1 << endl;
  //cout << "str.substr(v[1]+1, str.size()-v[1]-1) " << str.substr(v[1]+1, str.size()-v[1]-1) << endl;

  string ss;

  if (0 != v[0]) {
    bloutputname_csv = str.substr(0, v[0]);
    ss = str.substr(0, v[0]);
    //cout << "csv " << bloutputname_csv << endl;
  }
  if (v[0] + 1 != v[1]) {
    bloutputname_text = str.substr(v[0] + 1, v[1] - v[0] - 1);
    //cout << "text " << bloutputname_text << endl;
  }
  //if(v[1] != v.size()-1 ){
  if (v[1] != str.size() - 1) {    //////////////////////////////////////////
  //bloutputname_table  = str.substr(v[1]+1, v.size()-v[1]-1);
    bloutputname_table = str.substr(v[1] + 1, str.size() - v[1] - 1); ////////////////////////////////
    //cout << "table " << bloutputname_table  << endl;
  }

}

/*
 void SingleDishMS::MJDtoYMDhms(int* Y, int* M, int* D, int* h, int* m, double* s){
 double mjd = 4.64435e+09/86400.0;
 *Y = int((mjd-15078.2)/365.25);
 *M   = int((mjd-14956.1- int(*Y * 365.25))/30.6001);
 double dd=mjd - 14956 -int(*Y * 365.25)-int(*M * 30.6001);
 int k =0;
 if(*M==14 || *M==15){
 k=1;
 }else{
 k=0;
 }

 *Y = *Y + k + 1900;
 *M=*M-1-k*12;
 *D = int(dd);

 double hh = (dd - int(dd)) * 24.0;
 double mm = (hh - int(hh)) * 60.0;
 double ss = (mm - int(mm)) * 60.0;

 *h = int(hh);
 *m = int(mm);
 *s = ss;


 }
 */

////////////////////////////////////////////////////////////////////////
///// Atcual processing functions
////////////////////////////////////////////////////////////////////////
void SingleDishMS::subtractBaseline(string const& in_column_name,
    string const& out_ms_name, string const& out_bltable_name,
    string const& out_blformat_name, string const& out_bloutput_name,
    bool const& do_subtract, string const& in_spw, string const& in_ppp,
    string const& blfunc, int const order, float const clip_threshold_sigma,
    int const num_fitting_max, bool const linefinding, float const threshold,
    int const avg_limit, int const minwidth, vector<int> const& edge) {
//cout << "out_bloutput_name" << out_bloutput_name << flush << endl;
  split_bloutputname(out_bloutput_name);
//cout << "bloutputname_text" << bloutputname_text << flush << endl;
//cout << "bloutputname_csv" << bloutputname_csv << flush << endl;
//cout << "bloutputname_table" << bloutputname_table << flush << endl;

  LogIO os(_ORIGIN);
  os << "Fitting and subtracting polynomial baseline order = " << order
      << LogIO::POST;
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
  LIBSAKURA_SYMBOL (Status)
  status;
  LIBSAKURA_SYMBOL (BaselineStatus)
  bl_status;

  prepare_for_process(in_column_name, out_ms_name, columns, false);
  vi::VisibilityIterator2 *vi = sdh_->getVisIter();
  vi::VisBuffer2 *vb = vi->getVisBuffer();

  BaselineTable *bt = 0;
  ofstream ofs_csv;
  ofstream ofs_txt;

  //bool write_baseline_csv   = (out_bltable_name != "");
  bool write_baseline_csv = (bloutputname_csv != "");

  //bool write_baseline_text  = (out_bltable_name != "");
  bool write_baseline_text = (bloutputname_text != "");

  //bool write_baseline_table = (out_bltable_name != "");
  bool write_baseline_table = (bloutputname_table != "");

  if (write_baseline_csv) {
    ofs_csv.open(bloutputname_csv.c_str());
  }

  if (write_baseline_text) {
    ofs_txt.open(bloutputname_text.c_str(), std::ios::app);
  }

  if (write_baseline_table)
    bt = new BaselineTable(vi->ms());

  Vector<Int> recspw;
  Matrix<Int> recchan;
  Vector<size_t> nchan;
  Vector<Vector<Bool> > in_mask;
  Vector<bool> nchan_set;
  parse_spw(in_spw, recspw, recchan, nchan, in_mask, nchan_set);

  Vector<size_t> ctx_indices(nchan.nelements(), 0ul);
  std::vector<LIBSAKURA_SYMBOL(BaselineContext) *> bl_contexts;
  bl_contexts.clear();
  LIBSAKURA_SYMBOL (BaselineType)
  bltype = LIBSAKURA_SYMBOL(BaselineType_kPolynomial);
  if (blfunc == "chebyshev") {
    bltype = LIBSAKURA_SYMBOL(BaselineType_kChebyshev);
  }
  Vector<bool> pol;
  bool pol_set = false;

  for (vi->originChunks(); vi->moreChunks(); vi->nextChunk()) {
    for (vi->origin(); vi->more(); vi->next()) {
      Vector<Int> scans = vb->scan();
      Vector<Double> times = vb->time();
      Vector<Int> beams = vb->feed1();
      Vector<Int> antennas = vb->antenna1();

      Vector<Int> data_spw = vb->spectralWindows();
      size_t const num_chan = static_cast<size_t>(vb->nChannels());
      if ((int) num_chan < order + 1) {
        throw(AipsError(
            "subtract_baseline: nchan must be greater than order value."));
      }
      size_t const num_pol = static_cast<size_t>(vb->nCorrelations());
      size_t const num_row = static_cast<size_t>(vb->nRows());
      Cube<Float> data_chunk(num_pol, num_chan, num_row, ArrayInitPolicy::NO_INIT);
      Vector<float> spec(num_chan, ArrayInitPolicy::NO_INIT);
      Cube<Bool> flag_chunk(num_pol, num_chan, num_row, ArrayInitPolicy::NO_INIT);
      Vector<bool> mask(num_chan, ArrayInitPolicy::NO_INIT);
      Vector<bool> mask2(num_chan, ArrayInitPolicy::NO_INIT); //---------------------------------------------------------------------
      // CAUTION!!!
      // data() method must be used with special care!!!
      float *spec_data = spec.data();
      bool *mask_data = mask.data();
      bool *mask2_data = mask2.data();

      uInt final_mask[num_pol]; //---------------------------------------------------------------------
      uInt final_mask2[num_pol]; //---------------------------------------------------------------------

      final_mask[0] = 0;  //-------------------------------------------------
      final_mask[1] = 0;  //------------------------------------------------
      final_mask2[0] = 0;  //------------------------------------------------
      final_mask2[1] = 0;  //------------------------------------------------

      bool new_nchan = false;
      get_nchan_and_mask(recspw, data_spw, recchan, num_chan, nchan, in_mask,
          nchan_set, new_nchan);
      if (new_nchan) {
        get_baseline_context(
            bltype,  //LIBSAKURA_SYMBOL(BaselineType_kPolynomial),
            static_cast<uint16_t>(order), num_chan, nchan, nchan_set,
            ctx_indices, bl_contexts);
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
        Array<Bool> apply_mtx(IPosition(2, num_pol, 1), ArrayInitPolicy::NO_INIT);
        size_t num_apply_true = 0;
        Array<uInt> bltype_mtx(IPosition(2, num_pol, 1), ArrayInitPolicy::NO_INIT);
        Array<Int> fpar_mtx(IPosition(2, num_pol, 1), ArrayInitPolicy::NO_INIT);
        Array<Float> ffpar_mtx(IPosition(2, num_pol, 0), ArrayInitPolicy::NO_INIT);  //1));
        for (size_t ipol = 0; ipol < num_pol; ++ipol) {
          bltype_mtx[0][ipol] = (uInt) bltype; //LIBSAKURA_SYMBOL(BaselineType_kPolynomial);
          fpar_mtx[0][ipol] = (Int) order;
        }
        size_t num_masklist_max = 0;
        std::vector<std::vector<uInt> > masklist_mtx_tmp(num_pol);
        size_t num_coeff;
        status = LIBSAKURA_SYMBOL(GetNumberOfCoefficients)(
            bl_contexts[ctx_indices[idx]], order, &num_coeff);
        check_sakura_status("sakura_GetNumberOfCoefficients", status);
        Array<Float> coeff_mtx(IPosition(2, num_pol, num_coeff), 0.0);

        Array<Float> rms_mtx(IPosition(2, num_pol, 1), ArrayInitPolicy::NO_INIT);
        Array<Float> cthres_mtx(IPosition(2, num_pol, 1), ArrayInitPolicy::NO_INIT);
        Array<uInt> citer_mtx(IPosition(2, num_pol, 1), ArrayInitPolicy::NO_INIT);
        Array<Bool> uself_mtx(IPosition(2, num_pol, 1), ArrayInitPolicy::NO_INIT);
        Array<Float> lfthres_mtx(IPosition(2, num_pol, 1), ArrayInitPolicy::NO_INIT);
        Array<uInt> lfavg_mtx(IPosition(2, num_pol, 1), ArrayInitPolicy::NO_INIT);
        Array<uInt> lfedge_mtx(IPosition(2, num_pol, 2), ArrayInitPolicy::NO_INIT);

        // loop over polarization
        for (size_t ipol = 0; ipol < num_pol; ++ipol) {
          apply_mtx[0][ipol] = True;
          // skip spectrum not selected by pol
          if (!pol(ipol)) {
            flag_spectrum_in_cube(flag_chunk, irow, ipol);
            apply_mtx[0][ipol] = False;
            continue;
          }
          // get a channel mask from data cube
          // (note that the variable 'mask' is flag in the next line
          // actually, then it will be converted to real mask when
          // taking AND with user-given mask info. this is just for
          // saving memory usage...)
          get_flag_from_cube(flag_chunk, irow, ipol, num_chan, mask_data);
          // skip spectrum if all channels flagged
          if (allchannels_flagged(num_chan, mask_data)) {
            apply_mtx[0][ipol] = False;
            continue;
          }

          // convert flag to mask by taking logical NOT of flag
          // and then operate logical AND with in_mask
          for (size_t ichan = 0; ichan < num_chan; ++ichan) {
            mask_data[ichan] = in_mask[idx][ichan] && (!(mask_data[ichan]));
          }
          // get a spectrum from data cube
          get_spectrum_from_cube(data_chunk, irow, ipol, num_chan, spec_data);
          // line finding. get baseline mask (invert=true)
          if (linefinding) {
            findLineAndGetMask(num_chan, spec_data, mask_data, threshold,
                avg_limit, minwidth, edge, true, mask_data);
          }
          // Final check of the valid number of channels
          if (NValidMask(num_chan, mask_data) < num_coeff) {
            flag_spectrum_in_cube(flag_chunk, irow, ipol);
            apply_mtx[0][ipol] = False;
            os << LogIO::WARN
                << "Too few valid channels to fit. Skipping Antenna "
                << antennas[irow] << ", Beam " << beams[irow] << ", SPW "
                << data_spw[irow] << ", Pol " << ipol << ", Time "
                << MVTime(times[irow] / 24. / 3600.).string(MVTime::YMD, 8)
                << LogIO::POST;
            continue;
          }
          // actual execution of single spectrum
          float rms;
          if (write_baseline_text == true || write_baseline_csv == true
              || write_baseline_table == true) {
            num_apply_true++;
            Vector<double> coeff(num_coeff);
            // CAUTION!!!
            // data() method must be used with special care!!!
            double *coeff_data = coeff.data();
            status = LIBSAKURA_SYMBOL(GetBestFitBaselineCoefficientsFloat)(
                bl_contexts[ctx_indices[idx]], num_chan, spec_data, mask_data,
                clip_threshold_sigma, num_fitting_max, num_coeff, coeff_data,
                mask2_data, //----------------------------------------------------
                &rms,
                &bl_status);

            for (size_t i = 0; i < num_chan; ++i) {
              if (mask_data[i] == false) {
                final_mask[ipol] += 1;
              }

              if (mask2_data[i] == false) {
                final_mask2[ipol] += 1;
              }
            }

            /*
             for(size_t i = 0; num_chan; ++i){//-------------------------------------------------------------------
             if((mask.data[i] ==1 &&  mask2.data[i]==0) || (mask.data[i] ==0 &&  mask2.data[i]==1)){//-------------
             final_mask[ipol] += 1;//------------------------------------------------------------------------
             }//------------------------------------------------------------------------
             }//------------------------------------------------------------------------
             */

            check_sakura_status("sakura_GetBestFitBaselineCoefficientsFloat",
                status);
            set_array_for_bltable<double, Float>(ipol, num_coeff, coeff_data,
                coeff_mtx);

            Vector<uInt> masklist;
            get_masklist_from_mask(num_chan, mask2_data, masklist); //----------------------------------
            if (masklist.size() > num_masklist_max) {
              num_masklist_max = masklist.size();
            }
            masklist_mtx_tmp[ipol].clear();
            for (size_t imask = 0; imask < masklist.size(); ++imask) {
              masklist_mtx_tmp[ipol].push_back(masklist[imask]);
            }

            status = LIBSAKURA_SYMBOL(SubtractBaselineUsingCoefficientsFloat)(
                bl_contexts[ctx_indices[idx]], num_chan, spec_data, num_coeff,
                coeff_data, spec_data);
            check_sakura_status("sakura_SubtractBaselineUsingCoefficientsFloat",
                status);
            rms_mtx[0][ipol] = rms;

            cthres_mtx[0][ipol] = clip_threshold_sigma;
            citer_mtx[0][ipol] = (uInt) num_fitting_max - 1;
            uself_mtx[0][ipol] = False;
            lfthres_mtx[0][ipol] = 0.0;
            lfavg_mtx[0][ipol] = 0;
            for (size_t iedge = 0; iedge < 2; ++iedge) {
              lfedge_mtx[iedge][ipol] = 0;
            }

          } else {
            status = LIBSAKURA_SYMBOL(SubtractBaselineFloat)(
                bl_contexts[ctx_indices[idx]], static_cast<uint16_t>(order),
                num_chan, spec_data, mask_data, clip_threshold_sigma,
                num_fitting_max, true, mask_data, spec_data, &rms,
                &bl_status);
            check_sakura_status("sakura_SubtractBaselineFloat", status);
          }
          // set back a spectrum to data cube
          if (do_subtract) {
            set_spectrum_to_cube(data_chunk, irow, ipol, num_chan, spec_data);
          }

        } // loop pol

        if (write_baseline_table) {
          // write to baseline table if there is apply=True spectrum.
          if (num_apply_true > 0) {
            Array<uInt> masklist_mtx(IPosition(2, num_pol, num_masklist_max), ArrayInitPolicy::NO_INIT);
            set_matrix_for_bltable<uInt, uInt>(num_pol, num_masklist_max,
                masklist_mtx_tmp, masklist_mtx);
            bt->appenddata((uInt) scans[irow], (uInt) beams[irow],
                (uInt) antennas[irow], (uInt) data_spw[irow], 0, times[irow],
                apply_mtx, bltype_mtx, fpar_mtx, ffpar_mtx, masklist_mtx,
                coeff_mtx, rms_mtx, (uInt) num_chan, cthres_mtx, citer_mtx,
                uself_mtx, lfthres_mtx, lfavg_mtx, lfedge_mtx);
          }
        }

        if (write_baseline_text) {
          if (num_apply_true > 0) {
            /*
             int year;
             int month;
             int day;
             int hour;
             int minute;
             double second;
             */

            //MJD(second)-> year,month,day,hour,minute,second
            //MJDtoYMDhms(&year, &month, &day, &hour, &minute, &second);
            Array<uInt> masklist_mtx(IPosition(2, num_pol, num_masklist_max), ArrayInitPolicy::NO_INIT);
            set_matrix_for_bltable<uInt, uInt>(num_pol, num_masklist_max,
                masklist_mtx_tmp, masklist_mtx);
            Matrix<uInt> masklist_mtx2 = masklist_mtx;
            //MVTime mvtime = MVTime(times[irow]);
            for (size_t ipol = 0; ipol < num_pol; ++ipol) {
              ofs_txt << "Scan" << '[' << (uInt) scans[irow] << ']' << ' '
                  << "Beam" << '[' << (uInt) beams[irow] << ']' << ' ' << "Spw"
                  << '[' << (uInt) data_spw[irow] << ']' << ' ' << "Pol" << '['
                  << ipol << ']' << ' '
                  //<< "Time" <<'[' <<  times[irow] << ']' << endl;
                  //<< "Time" <<'[' << year << '/' << month << '/' << day << '/' << hour << ':' << minute << ':' << second  << ']' << endl;
                  << "Time" << '['
                  << MVTime(times[irow] / 24. / 3600.).string(MVTime::YMD, 8)
                  << ']' << endl;
              /*        << "Time" <<'[' << year << '/'
               << setfill('0') << setw(2) << month << '/'
               << setfill('0') << setw(2) << day <<  '/'
               << setfill('0') << setw(2) << hour << ':'
               << setfill('0') << setw(2) << minute  << ':';
               ofs_txt.setf(ios::fixed);
               ofs_txt << setprecision(3) << second << ']'
               << endl;
               */

              ofs_txt << endl;
              ofs_txt << "Fitter range = " << '[';

              for (size_t imasklist = 0; imasklist < num_masklist_max / 2;
                  ++imasklist) {
                if (imasklist == 0) {
                  ofs_txt << '[' << masklist_mtx2(ipol, 2 * imasklist) << ';'
                      << masklist_mtx2(ipol, 2 * imasklist + 1) << ']';
                }
                if (imasklist >= 1
                    && (0 != masklist_mtx2(ipol, 2 * imasklist)
                        && 0 != masklist_mtx2(ipol, 2 * imasklist + 1))) {
                  ofs_txt << ",[" << masklist_mtx2(ipol, 2 * imasklist) << ','
                      << masklist_mtx2(ipol, 2 * imasklist + 1) << ']';
                }
              }

              ofs_txt << ']' << endl;
              ;
              ofs_txt << endl;
              Matrix<uInt> bltype_mtx2 = bltype_mtx[0][ipol];
              Matrix<Int> fpar_mtx2 = fpar_mtx[0][ipol];
              Matrix<Float> rms_mtx2 = rms_mtx[0][ipol];
              string bltype_name;
              if (bltype_mtx2(0, 0) == (uInt) 0) {
                bltype_name = "poly";
              } else if (bltype_mtx2(0, 0) == (uInt) 1) {
                bltype_name = "chebyshev";
              }

              ofs_txt << "Baseline parameters  Function = "
                  << bltype_name.c_str() << ' ' << " order = "
                  << fpar_mtx2(0, 0) << endl;
              ofs_txt << endl;
              ofs_txt << "Results of baseline fit" << endl;
              ofs_txt << endl;
              //for (size_t icoeff = 0; icoeff < num_coeff; ++icoeff) {
              //    ofs_txt << "p" << icoeff << "=" << coeff_mtx[icoeff][ipol] << ' ';
              //}

              Matrix<Float> coeff_mtx2 = coeff_mtx;
              for (size_t icoeff = 0; icoeff < num_coeff; ++icoeff) {
                ofs_txt << "p" << icoeff << " = ";
                ofs_txt.setf(ios::scientific);
                ofs_txt << setprecision(6) << coeff_mtx2(ipol, icoeff) << "  ";
              }

              ofs_txt << endl;
              ofs_txt << endl;
              //ofs_txt << "rms = " << rms_mtx2(0,0) << endl;
              ofs_txt << "rms = ";
              ofs_txt.setf(ios::fixed);
              ofs_txt << setprecision(6) << rms_mtx2(0, 0) << endl;
              ofs_txt << endl;
              //ofs_txt << "Number of clipped channels = " << (uInt)num_chan << endl;
              ofs_txt << "Number of clipped channels = "
                  << final_mask2[ipol] - final_mask[ipol] << endl; //-----------------------------------
              ofs_txt << endl;
              ofs_txt
                  << "------------------------------------------------------"
                  << endl;
              ofs_txt << endl;
            }
          }
        }

        if (write_baseline_csv) {
          if (num_apply_true > 0) {

            /*
             string fitting_func;
             if(bltype_mtx[0][ipol] =="[0]"){
             fittingfunc = "poly";
             }else if(bltype_mtx[0][ipol] =="[1]"){
             fittingfunc = "chebyshev";
             }else if(bltype_mtx[0][ipol] =="[2]"){
             fittingfunc = "cspline";
             }
             */

            Array<uInt> masklist_mtx(IPosition(2, num_pol, num_masklist_max), ArrayInitPolicy::NO_INIT);
            set_matrix_for_bltable<uInt, uInt>(num_pol, num_masklist_max,
                masklist_mtx_tmp, masklist_mtx);
            Matrix<uInt> masklist_mtx2 = masklist_mtx;
            for (size_t ipol = 0; ipol < num_pol; ++ipol) {
              ofs_csv << (uInt) scans[irow] << ',' << (uInt) beams[irow] << ','
                  << (uInt) data_spw[irow] << ',' << ipol << ',' << times[irow]
                  << ',';
              ofs_csv << '[';
              for (size_t imasklist = 0; imasklist < num_masklist_max / 2;
                  ++imasklist) {

                if (imasklist == 0) {
                  ofs_csv << '[' << masklist_mtx2(ipol, 2 * imasklist) << ';'
                      << masklist_mtx2(ipol, 2 * imasklist + 1) << ']';
                }

                if (imasklist >= 1
                    && (0 != masklist_mtx2(ipol, 2 * imasklist)
                        && 0 != masklist_mtx2(ipol, 2 * imasklist + 1))) {
                  ofs_csv << ";[" << masklist_mtx2(ipol, 2 * imasklist) << ';'
                      << masklist_mtx2(ipol, 2 * imasklist + 1) << ']';
                }
              }

              ofs_csv << ']' << ',';
              Matrix<uInt> bltype_mtx2 = bltype_mtx[0][ipol];
              string bltype_name;

              if (bltype_mtx2(0, 0) == (uInt) 0) {
                bltype_name = "poly";
              }
              if (bltype_mtx2(0, 0) == (uInt) 1) {
                bltype_name = "chebyshev";
              }

              Matrix<Int> fpar_mtx2 = fpar_mtx;
              Matrix<Float> coeff_mtx2 = coeff_mtx;
              ofs_csv << bltype_name.c_str() << ',' << fpar_mtx2(ipol, 0)
                  << ',';
              for (size_t icoeff = 0; icoeff < num_coeff; ++icoeff) {
                ofs_csv << coeff_mtx2(ipol, icoeff) << ',';
              }
              Matrix<Float> rms_mtx2 = rms_mtx;
              //ofs_csv << rms_mtx[0][ipol] << ',';
              ofs_csv << rms_mtx2(ipol, 0) << ',';
              //cout << "rms_mtx2(ipol,0) " << rms_mtx2(ipol,0) << endl;
              //cout << "rms_mtx2(0,0) " << rms_mtx2(0,0) << endl;

              //ofs_csv << (uInt)num_chan ;
              ofs_csv << final_mask2[ipol] - final_mask[ipol]; //------------------------------------------------------------------

              ofs_csv << endl;
              ofs_csv << endl;
            }
          }
        }

      } // end of chunk row loop
      // write back data cube to VisBuffer
      sdh_->fillCubeToOutputMs(vb, data_chunk, &flag_chunk);
    } // end of vi loop
  } // end of chunk loop

  if (write_baseline_csv) {
    ofs_csv.close();
  }
  if (write_baseline_text) {
    ofs_txt.close();
  }

  if (write_baseline_table) {
    bt->save(bloutputname_table);
    delete bt;
  }

  finalize_process();
  destroy_baseline_contexts (bl_contexts);

  //double tend = gettimeofday_sec();
  //std::cout << "Elapsed time = " << (tend - tstart) << " sec." << std::endl;
}

//Cubic Spline
void SingleDishMS::subtractBaselineCspline(string const& in_column_name,
    string const& out_ms_name, string const& out_bltable_name,
    string const& out_blformat_name, string const& out_bloutput_name,
    bool const& do_subtract, string const& in_spw, string const& in_ppp,
    int const npiece, float const clip_threshold_sigma,
    int const num_fitting_max, bool const linefinding, float const threshold,
    int const avg_limit, int const minwidth, vector<int> const& edge) {
  split_bloutputname(out_bloutput_name);
//cout << "SingleDishMS.cc 1" << flush << endl;

  LogIO os(_ORIGIN);
  os << "Fitting and subtracting cubic spline baseline npiece = " << npiece
      << LogIO::POST;
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
  LIBSAKURA_SYMBOL (Status)
  status;
  LIBSAKURA_SYMBOL (BaselineStatus)
  bl_status;

  prepare_for_process(in_column_name, out_ms_name, columns, false);
  vi::VisibilityIterator2 *vi = sdh_->getVisIter();
  vi::VisBuffer2 *vb = vi->getVisBuffer();
  BaselineTable *bt = 0;

  ofstream ofs_csv;
  ofstream ofs_txt;
  bool write_baseline_csv = (bloutputname_csv != "");
  bool write_baseline_text = (bloutputname_text != "");

  //bool write_baseline_table = (out_bltable_name != "");
  bool write_baseline_table = (bloutputname_table != "");

  if (write_baseline_csv) {
    ofs_csv.open(bloutputname_csv.c_str());
  }
  if (write_baseline_text) {
    ofs_txt.open(bloutputname_text.c_str(), std::ios::app);
  }

  if (write_baseline_table)
    bt = new BaselineTable(vi->ms());

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
      Vector<Int> antennas = vb->antenna1();

      Vector<Int> data_spw = vb->spectralWindows();
      size_t const num_chan = static_cast<size_t>(vb->nChannels());
      if ((int) num_chan < npiece * 4) {
        throw(AipsError(
            "subtract_baseline_cspline: nchan must be greater than 4*npiece."));
      }
      size_t const num_pol = static_cast<size_t>(vb->nCorrelations());
      size_t const num_row = static_cast<size_t>(vb->nRows());
      Cube<Float> data_chunk(num_pol, num_chan, num_row);
      Vector<float> spec(num_chan);
      Cube<Bool> flag_chunk(num_pol, num_chan, num_row);
      Vector<bool> mask(num_chan);

      Vector<bool> mask2(num_chan); //---------------------------------------------------------------------
      // CAUTION!!!
      // data() method must be used with special care!!!
      float *spec_data = spec.data();
      bool *mask_data = mask.data();
      bool *mask2_data = mask2.data();

      uInt final_mask[num_pol]; //---------------------------------------------------------------------
      uInt final_mask2[num_pol]; //---------------------------------------------------------------------

      final_mask[0] = 0;  //-------------------------------------------------
      final_mask[1] = 0;  //------------------------------------------------
      final_mask2[0] = 0;  //------------------------------------------------
      final_mask2[1] = 0;  //------------------------------------------------

      bool new_nchan = false;
      get_nchan_and_mask(recspw, data_spw, recchan, num_chan, nchan, in_mask,
          nchan_set, new_nchan);
      if (new_nchan) {
        get_baseline_context(LIBSAKURA_SYMBOL(BaselineType_kCubicSpline),
            static_cast<uint16_t>(npiece), num_chan, nchan, nchan_set,
            ctx_indices, bl_contexts);
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
        size_t num_apply_true = 0;
        Array<uInt> bltype_mtx(IPosition(2, num_pol, 1));
        Array<Int> fpar_mtx(IPosition(2, num_pol, 1));
        Array<Float> ffpar_mtx(IPosition(2, num_pol, npiece));
        for (size_t ipol = 0; ipol < num_pol; ++ipol) {
          bltype_mtx[0][ipol] = (uInt) LIBSAKURA_SYMBOL(
              BaselineType_kCubicSpline);
          fpar_mtx[0][ipol] = (Int) npiece;
          for (int ipiece = 0; ipiece < npiece; ++ipiece) {
            ffpar_mtx[ipiece][ipol] = 0.0;
          }
        }
        size_t num_masklist_max = 0;
        std::vector<std::vector<uInt> > masklist_mtx_tmp(num_pol);
        size_t num_coeff = 4 * npiece;
        Vector<double> coeff(num_coeff);
        // CAUTION!!!
        // data() method must be used with special care!!!
        double *coeff_data = coeff.data();
        Array<Float> coeff_mtx(IPosition(2, num_pol, num_coeff));
        for (size_t ipol = 0; ipol < num_pol; ++ipol) {
          for (size_t icoeff = 0; icoeff < num_coeff; ++icoeff) {
            coeff_mtx[icoeff][ipol] = 0.0;
          }
        }
        Array<Float> rms_mtx(IPosition(2, num_pol, 1));
        Array<Float> cthres_mtx(IPosition(2, num_pol, 1));
        Array<uInt> citer_mtx(IPosition(2, num_pol, 1));
        Array<Bool> uself_mtx(IPosition(2, num_pol, 1));
        Array<Float> lfthres_mtx(IPosition(2, num_pol, 1));
        Array<uInt> lfavg_mtx(IPosition(2, num_pol, 1));
        Array<uInt> lfedge_mtx(IPosition(2, num_pol, 2));

        // loop over polarization
        for (size_t ipol = 0; ipol < num_pol; ++ipol) {
          apply_mtx[0][ipol] = True;
          // skip spectrum not selected by pol
          if (!pol(ipol)) {
            flag_spectrum_in_cube(flag_chunk, irow, ipol);
            apply_mtx[0][ipol] = False;
            continue;
          }
          // get a channel mask from data cube
          // (note that the variable 'mask' is flag in the next line
          // actually, then it will be converted to real mask when
          // taking AND with user-given mask info. this is just for
          // saving memory usage...)
          get_flag_from_cube(flag_chunk, irow, ipol, num_chan, mask_data);
          // skip spectrum if all channels flagged
          if (allchannels_flagged(num_chan, mask_data)) {
            apply_mtx[0][ipol] = False;
            continue;
          }

          // convert flag to mask by taking logical NOT of flag
          // and then operate logical AND with in_mask
          for (size_t ichan = 0; ichan < num_chan; ++ichan) {
            mask_data[ichan] = in_mask[idx][ichan] && (!(mask_data[ichan]));
          }
          // get a spectrum from data cube
          get_spectrum_from_cube(data_chunk, irow, ipol, num_chan, spec_data);
          // line finding. get baseline mask (invert=true)
          if (linefinding) {
            findLineAndGetMask(num_chan, spec_data, mask_data, threshold,
                avg_limit, minwidth, edge, true, mask_data);
          }
          // Final check of the valid number of channels (For cubic spline, degree of freedom is npiece+3)
          if (NValidMask(num_chan, mask_data) < npiece + 3) {
            flag_spectrum_in_cube(flag_chunk, irow, ipol);
            apply_mtx[0][ipol] = False;
            os << LogIO::WARN
                << "Too few valid channels to fit. Skipping Antenna "
                << antennas[irow] << ", Beam " << beams[irow] << ", SPW "
                << data_spw[irow] << ", Pol " << ipol << ", Time "
                << MVTime(times[irow] / 24. / 3600.).string(MVTime::YMD, 8)
                << LogIO::POST;
            continue;
          }

          // actual execution of single spectrum
          float rms;
          Vector<double> boundary(npiece);
          // CAUTION!!!
          // data() method must be used with special care!!!
          double *boundary_data = boundary.data();

          if (write_baseline_text == true || write_baseline_csv == true
              || write_baseline_table == true) {
            //if(write_baseline_table==true) {
            num_apply_true++;
            status = LIBSAKURA_SYMBOL(
                GetBestFitBaselineCoefficientsCubicSplineFloat)(
                bl_contexts[ctx_indices[idx]], num_chan, spec_data, mask_data,
                clip_threshold_sigma, num_fitting_max, npiece, coeff_data,
                mask2_data, //------------------------------------------------------------
                &rms, boundary_data,
                &bl_status);

            for (size_t i = 0; i < num_chan; ++i) {
              if (mask_data[i] == false) {
                final_mask[ipol] += 1;
              }

              if (mask2_data[i] == false) {
                final_mask2[ipol] += 1;
              }
            }

            check_sakura_status(
                "sakura_GetBestFitBaselineCoefficientsCubicSplineFloat",
                status);
            set_array_for_bltable<double, Float>(ipol, num_coeff, coeff_data,
                coeff_mtx);
            Vector<uInt> masklist;
            get_masklist_from_mask(num_chan, mask2_data, masklist); //------------------------------------------
            if (masklist.size() > num_masklist_max) {
              num_masklist_max = masklist.size();
            }
            masklist_mtx_tmp[ipol].clear();
            for (size_t imask = 0; imask < masklist.size(); ++imask) {
              masklist_mtx_tmp[ipol].push_back(masklist[imask]);
            }
            set_array_for_bltable<double, Float>(ipol, npiece, boundary_data,
                ffpar_mtx);

            status = LIBSAKURA_SYMBOL(
                SubtractBaselineCubicSplineUsingCoefficientsFloat)(
                bl_contexts[ctx_indices[idx]], num_chan, spec_data, npiece,
                coeff_data, boundary_data, spec_data);
            check_sakura_status(
                "sakura_SubtractBaselineCubicSplineUsingCoefficientsFloat",
                status);
            rms_mtx[0][ipol] = rms;

            cthres_mtx[0][ipol] = clip_threshold_sigma;
            citer_mtx[0][ipol] = (uInt) num_fitting_max - 1;
            uself_mtx[0][ipol] = False;
            lfthres_mtx[0][ipol] = 0.0;
            lfavg_mtx[0][ipol] = 0;
            for (size_t iedge = 0; iedge < 2; ++iedge) {
              lfedge_mtx[iedge][ipol] = 0;
            }

          } else {
            status = LIBSAKURA_SYMBOL(SubtractBaselineCubicSplineFloat)(
                bl_contexts[ctx_indices[idx]], static_cast<uint16_t>(npiece),
                num_chan, spec_data, mask_data, clip_threshold_sigma,
                num_fitting_max, true, mask_data, spec_data, &rms,
                boundary_data,
                &bl_status);
            check_sakura_status("sakura_SubtractBaselineCubicSplineFloat",
                status);
          }
          // set back a spectrum to data cube
          if (do_subtract) {
            set_spectrum_to_cube(data_chunk, irow, ipol, num_chan, spec_data);
          }
        } // end of polarization loop

        if (write_baseline_table) {
          // write to baseline table if there is apply=True spectrum.
          if (num_apply_true > 0) {
            Array<uInt> masklist_mtx(IPosition(2, num_pol, num_masklist_max));
            set_matrix_for_bltable<uInt, uInt>(num_pol, num_masklist_max,
                masklist_mtx_tmp, masklist_mtx);
            bt->appenddata((uInt) scans[irow], (uInt) beams[irow],
                (uInt) antennas[irow], (uInt) data_spw[irow], 0, times[irow],
                apply_mtx, bltype_mtx, fpar_mtx, ffpar_mtx, masklist_mtx,
                coeff_mtx, rms_mtx, (uInt) num_chan, cthres_mtx, citer_mtx,
                uself_mtx, lfthres_mtx, lfavg_mtx, lfedge_mtx);
          }
        }

        if (write_baseline_text) {
          if (num_apply_true > 0) {
            /*
             int year;
             int month;
             int day;
             int hour;
             int minute;
             double second;
             */

            //MJD(second)-> year,month,day,hour,minute,second
            //MJDtoYMDhms(&year, &month, &day, &hour, &minute, &second);
            Array<uInt> masklist_mtx(IPosition(2, num_pol, num_masklist_max));
            set_matrix_for_bltable<uInt, uInt>(num_pol, num_masklist_max,
                masklist_mtx_tmp, masklist_mtx);
            Matrix<uInt> masklist_mtx2 = masklist_mtx;
            for (size_t ipol = 0; ipol < num_pol; ++ipol) {
              ofs_txt << "Scan" << '[' << (uInt) scans[irow] << ']' << ' '
                  << "Beam" << '[' << (uInt) beams[irow] << ']' << ' ' << "Spw"
                  << '[' << (uInt) data_spw[irow] << ']' << ' ' << "Pol" << '['
                  << ipol << ']' << ' '
                  //<< "Time" <<'[' <<  times[irow] << ']' << endl;
                  << "Time" << '['
                  << MVTime(times[irow] / 24. / 3600.).string(MVTime::YMD, 8)
                  << ']' << endl;
              /*         << "Time" <<'[' << year << '/'
               << setfill('0') << setw(2) << month << '/'
               << setfill('0') << setw(2) << day <<  '/'
               << setfill('0') << setw(2) << hour << ':'
               << setfill('0') << setw(2) << minute  << ':';
               ofs_txt.setf(ios::fixed);
               ofs_txt << setprecision(3) << second << ']'
               << endl;
               */
              ofs_txt << endl;
              ofs_txt << "Fitter range = " << '[';

              for (size_t imasklist = 0; imasklist < num_masklist_max / 2;
                  ++imasklist) {
                if (imasklist == 0) {
                  ofs_txt << '[' << masklist_mtx2(ipol, 2 * imasklist) << ';'
                      << masklist_mtx2(ipol, 2 * imasklist + 1) << ']';
                }
                if (imasklist >= 1
                    && (0 != masklist_mtx2(ipol, 2 * imasklist)
                        && 0 != masklist_mtx2(ipol, 2 * imasklist + 1))) {
                  ofs_txt << ",[" << masklist_mtx2(ipol, 2 * imasklist) << ','
                      << masklist_mtx2(ipol, 2 * imasklist + 1) << ']';
                }
              }

              //for(size_t imasklist = 0; imasklist < num_masklist_max/2; ++imasklist){
              //    ofs_txt << '[' << masklist_mtx2(ipol, 2*imasklist) << ',' << masklist_mtx2(ipol, 2*imasklist+1) << ']';
              //    if(imasklist < num_masklist_max/2 -1){
              //       ofs_txt  << ',';
              //    }
              //}

              ofs_txt << ']' << endl;
              ;
              ofs_txt << endl;
              Matrix<uInt> bltype_mtx2 = bltype_mtx[0][ipol];
              Matrix<Int> fpar_mtx2 = fpar_mtx[0][ipol];
              Matrix<Float> rms_mtx2 = rms_mtx[0][ipol];
              string bltype_name;
              if (bltype_mtx2(0, 0) == (uInt) 2) {
                bltype_name = "cspline";
              }

              ofs_txt << "Baseline parameters  Function = "
                  << bltype_name.c_str() << ' ' << " npiece = "
                  << fpar_mtx2(0, 0) << endl;
              ofs_txt << endl;
              ofs_txt << "Results of baseline fit" << endl;
              ofs_txt << endl;
              Matrix<Float> coeff_mtx2 = coeff_mtx;
              for (size_t icoeff = 0; icoeff < num_coeff; ++icoeff) {
                //ofs_txt << "p" << icoeff << " = " << coeff_mtx2(ipol,icoeff) << "  ";
                ofs_txt << "p" << icoeff << " = ";
                ofs_txt.setf(ios::scientific);
                ofs_txt << setprecision(6) << coeff_mtx2(ipol, icoeff) << "  ";
              }
              ofs_txt << endl;
              ofs_txt << endl;
              //ofs_txt << "rms = " << rms_mtx2(0,0) << endl;
              ofs_txt << "rms = ";
              ofs_txt.setf(ios::fixed);
              ofs_txt << setprecision(6) << rms_mtx2(0, 0) << endl;
              ofs_txt << endl;
              //ofs_txt << "Number of clipped channels = " << (uInt)num_chan << endl;
              ofs_txt << "Number of clipped channels = "
                  << final_mask2[ipol] - final_mask[ipol] << endl;
              ofs_txt << endl;
              ofs_txt
                  << "------------------------------------------------------"
                  << endl;
              ofs_txt << endl;
            }
          }
        }

        if (write_baseline_csv) {
          if (num_apply_true > 0) {

            //
            // string fitting_func;
            // if(bltype_mtx[0][ipol] =="[0]"){
            //      fittingfunc = "poly";
            //  }else if(bltype_mtx[0][ipol] =="[1]"){
            //      fittingfunc = "chebyshev";
            //  }else if(bltype_mtx[0][ipol] =="[2]"){
            //      fittingfunc = "cspline";
            //  }

            Array<uInt> masklist_mtx(IPosition(2, num_pol, num_masklist_max));
            set_matrix_for_bltable<uInt, uInt>(num_pol, num_masklist_max,
                masklist_mtx_tmp, masklist_mtx);
            Matrix<uInt> masklist_mtx2 = masklist_mtx;
            for (size_t ipol = 0; ipol < num_pol; ++ipol) {
              ofs_csv << (uInt) scans[irow] << ',' << (uInt) beams[irow] << ','
                  << (uInt) data_spw[irow] << ',' << ipol << ',' << times[irow]
                  << ',';
              ofs_csv << '[';

              for (size_t imasklist = 0; imasklist < num_masklist_max / 2;
                  ++imasklist) {
                if (imasklist == 0) {
                  ofs_csv << '[' << masklist_mtx2(ipol, 2 * imasklist) << ';'
                      << masklist_mtx2(ipol, 2 * imasklist + 1) << ']';
                }
                if (imasklist >= 1
                    && (0 != masklist_mtx2(ipol, 2 * imasklist)
                        && 0 != masklist_mtx2(ipol, 2 * imasklist + 1))) {
                  ofs_csv << ";[" << masklist_mtx2(ipol, 2 * imasklist) << ';'
                      << masklist_mtx2(ipol, 2 * imasklist + 1) << ']';
                }

                //ofs_csv << '[' << masklist_mtx2(ipol, 2*imasklist) << ',' << masklist_mtx2(ipol, 2*imasklist+1) << ']';
                //if(imasklist < num_masklist_max/2 -1){
                //   ofs_csv  << ',';
                //}
              }

              ofs_csv << ']' << ',';
              Matrix<uInt> bltype_mtx2 = bltype_mtx[0][ipol];
              string bltype_name;
              if (bltype_mtx2(0, 0) == (uInt) 2) {
                bltype_name = "cspline";
              }
              Matrix<Int> fpar_mtx2 = fpar_mtx;
              Matrix<Float> coeff_mtx2 = coeff_mtx;
              ofs_csv << bltype_name.c_str() << ',' << fpar_mtx2(ipol, 0)
                  << ',';

              for (size_t icoeff = 0; icoeff < num_coeff; ++icoeff) {
                //ofs_csv << coeff_mtx[icoeff][ipol] << ',';
                ofs_csv << coeff_mtx2(ipol, icoeff) << ',';
              }

              Matrix<Float> rms_mtx2 = rms_mtx;
              ofs_csv << rms_mtx2(ipol, 0) << ',';

              //ofs_csv << rms_mtx[0][ipol] << ',';
              //ofs_csv << (uInt)num_chan ;
              ofs_csv << final_mask2[ipol] - final_mask[ipol];

              ofs_csv << endl;
              ofs_csv << endl;
            }
          }
        }

      } // end of chunk row loop
      // write back data cube to VisBuffer
      sdh_->fillCubeToOutputMs(vb, data_chunk, &flag_chunk);
    } // end of vi loop
  } // end of chunk loop

  if (write_baseline_csv) {
    ofs_csv.close();
  }
  if (write_baseline_text) {
    ofs_txt.close();
  }

  if (write_baseline_table) {
    bt->save(bloutputname_table);
    delete bt;
  }

  finalize_process();
  destroy_baseline_contexts (bl_contexts);

  //double tend = gettimeofday_sec();
  //std::cout << "Elapsed time = " << (tend - tstart) << " sec." << std::endl;
}

// Apply baseline table to MS
void SingleDishMS::applyBaselineTable(string const& in_column_name,
    string const& in_bltable_name, string const& in_spw,
    string const& out_ms_name) {
  LogIO os(_ORIGIN);
  os << "Apply baseline table " << in_bltable_name << " to MS. " << LogIO::POST;

  //checking in_bltable_name
  if (in_bltable_name == "") {
    throw(AipsError("baseline table is not given."));
  }

  // parse fitting parameters in the text file
  BLTableParser parser(in_bltable_name);
  std::vector<LIBSAKURA_SYMBOL(BaselineType)> baseline_types =
      parser.get_function_types();
  map<const LIBSAKURA_SYMBOL(BaselineType), uint16_t> max_orders;
  for (size_t i = 0; i < baseline_types.size(); ++i) {
    max_orders[baseline_types[i]] = parser.get_max_order(baseline_types[i]);
  }
  { //DEBUG ouput
    os << LogIO::DEBUG1 << "spw ID = " << in_spw << LogIO::POST;
    os << LogIO::DEBUG1 << "Baseline Types = " << baseline_types << LogIO::POST;
    os << LogIO::DEBUG1 << "Max Orders:" << LogIO::POST;
    map<const LIBSAKURA_SYMBOL(BaselineType), uint16_t>::iterator iter =
        max_orders.begin();
    while (iter != max_orders.end()) {
      os << LogIO::DEBUG1 << "- type " << (*iter).first << ": "
          << (*iter).second << LogIO::POST;
      ++iter;
    }
  }

  // Setup VisIter for input MS
  Block<Int> columns(1);
  columns[0] = MS::TIME;

  prepare_for_process(in_column_name, out_ms_name, columns, false);
  vi::VisibilityIterator2 *vi = sdh_->getVisIter();
  vi::VisBuffer2 *vb = vi->getVisBuffer();

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
    map<const LIBSAKURA_SYMBOL(BaselineType), uint16_t>::iterator iter =
        max_orders.begin();
    while (iter != max_orders.end()) {
      context_reservoir[(*iter).first] = std::vector<LIBSAKURA_SYMBOL(BaselineContext) *>();
      ++iter;
    }
  }

  LIBSAKURA_SYMBOL (Status)
  status;
  Vector<size_t> ctx_indices;
  ctx_indices.resize(nchan.nelements());
  for (size_t ictx = 0; ictx < ctx_indices.nelements(); ++ictx) {
    ctx_indices(ictx) = 0;
  }

  // Iterate over MS and subtract baseline
  for (vi->originChunks(); vi->moreChunks(); vi->nextChunk()) {
    //cout << "New chunk" << endl;
    for (vi->origin(); vi->more(); vi->next()) {
      Vector<Int> scans = vb->scan();
      Vector<Double> times = vb->time();
      Vector<Double> intervals = vb->timeInterval();
      Vector<Int> beams = vb->feed1();
      Vector<Int> antennas = vb->antenna1();
      Vector<Int> data_spw = vb->spectralWindows();
      size_t const num_chan = static_cast<size_t>(vb->nChannels());
      size_t const num_pol = static_cast<size_t>(vb->nCorrelations());
      size_t const num_row = static_cast<size_t>(vb->nRows());
      Vector<uInt> orig_rows = vb->rowIds();
      Cube<Float> data_chunk(num_pol, num_chan, num_row);
      Vector<float> spec(num_chan);
      Cube<Bool> flag_chunk(num_pol, num_chan, num_row);
      Vector<bool> mask(num_chan);
      //cout << "New iteration: num_row=" << num_row << ", num_chan=" << num_chan << ", num_pol=" << num_pol << ", spwid=" << data_spw << endl;
      // CAUTION!!!
      // data() method must be used with special care!!!
      float *spec_data = spec.data();
      bool *mask_data = mask.data();

      bool new_nchan = false;
      get_nchan_and_mask(recspw, data_spw, recchan, num_chan, nchan, in_mask,
          nchan_set, new_nchan);
      if (new_nchan) {
        // Generate context for all necessary baseline types
        map<const LIBSAKURA_SYMBOL(BaselineType), uint16_t>::iterator iter =
            max_orders.begin();
        while (iter != max_orders.end()) {
          get_baseline_context((*iter).first, (*iter).second, num_chan, nchan,
              nchan_set, ctx_indices, context_reservoir[(*iter).first]);
          ++iter;
        }
      }
      // get data/flag cubes (npol*nchan*nrow) from VisBuffer
      get_data_cube_float(*vb, data_chunk);
      get_flag_cube(*vb, flag_chunk);

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
        Array<uInt> bltype_mtx(IPosition(2, num_pol, 1));
        Array<Int> fpar_mtx(IPosition(2, num_pol, 1));
        std::vector<std::vector<double> > ffpar_mtx_tmp(num_pol);
        std::vector<std::vector<double> > coeff_mtx_tmp(num_pol);

        size_t idx_fit_param;
        if (!parser.GetFitParameterIdx(times[irow], intervals[irow],
            scans[irow], beams[irow], antennas[irow], data_spw[irow],
            idx_fit_param)) {
          for (size_t ipol = 0; ipol < num_pol; ++ipol) {
            flag_spectrum_in_cube(flag_chunk, irow, ipol); //flag
          }
          continue;
        }

        // loop over polarization
        for (size_t ipol = 0; ipol < num_pol; ++ipol) {
          bool apply;
          std::vector<float> bl_coeff;
          std::vector<double> bl_boundary;
          BLParameterSet fit_param;
          parser.GetFitParameterByIdx(idx_fit_param, ipol, apply, bl_coeff,
              bl_boundary, fit_param);
          if (!apply) {
            flag_spectrum_in_cube(flag_chunk, irow, ipol); //flag
            continue;
          }

          // get a channel mask from data cube
          // (note that the variable 'mask' is flag in the next line
          // actually, then it will be converted to real mask when
          // taking AND with user-given mask info. this is just for
          // saving memory usage...)
          get_flag_from_cube(flag_chunk, irow, ipol, num_chan, mask_data);
          // skip spectrum if all channels flagged
          if (allchannels_flagged(num_chan, mask_data)) {
            continue;
          }

          // get a spectrum from data cube
          get_spectrum_from_cube(data_chunk, irow, ipol, num_chan, spec_data);

          // actual execution of single spectrum
          map< const LIBSAKURA_SYMBOL(BaselineType),
          std::vector<LIBSAKURA_SYMBOL(BaselineContext) *> >::iterator
          iter = context_reservoir.find(fit_param.baseline_type);
          if (iter == context_reservoir.end())
            throw(AipsError("Invalid baseline type detected!"));
          LIBSAKURA_SYMBOL(BaselineContext) * context =
              (*iter).second[ctx_indices[idx]];
          //cout << "Got context for type " << (*iter).first << ": idx=" << ctx_indices[idx] << endl;

          size_t num_coeff = bl_coeff.size();
          Vector<double> coeff(num_coeff);
          // CAUTION!!!
          // data() method must be used with special care!!!
          double *coeff_data = coeff.data();
          for (size_t i = 0; i < num_coeff; ++i) {
            coeff_data[i] = bl_coeff[i];
          }
          size_t num_boundary = bl_boundary.size();
          Vector<double> boundary(num_boundary);
          // CAUTION!!!
          // data() method must be used with special care!!!
          double *boundary_data = boundary.data();
          for (size_t i = 0; i < num_boundary; ++i) {
            boundary_data[i] = bl_boundary[i];
          }

          string subtract_funcname;
          switch (fit_param.baseline_type) {
          case LIBSAKURA_SYMBOL(BaselineType_kPolynomial):
          case LIBSAKURA_SYMBOL(BaselineType_kChebyshev):
            //cout << (fit_param.baseline_type==0 ? "poly" : "chebyshev") << ": order=" << fit_param.order << ", row=" << orig_rows[irow] << ", pol=" << ipol << ", num_chan=" << num_chan << ", num_valid_chan = " << NValidMask(num_chan, mask.data) << endl;
            status = LIBSAKURA_SYMBOL(SubtractBaselineUsingCoefficientsFloat)(
                context, num_chan, spec_data, num_coeff, coeff_data, spec_data);
            subtract_funcname = "sakura_SubtractBaselineUsingCoefficientsFloat";
            break;
          case LIBSAKURA_SYMBOL(BaselineType_kCubicSpline):
            //cout << "cspline: npiece = " << fit_param.npiece << ", row=" << orig_rows[irow] << ", pol=" << ipol << ", num_chan=" << num_chan << ", num_valid_chan = " << NValidMask(num_chan, mask.data) << endl;
            status = LIBSAKURA_SYMBOL(
                SubtractBaselineCubicSplineUsingCoefficientsFloat)(context,
                num_chan, spec_data, num_boundary, coeff_data, boundary_data,
                spec_data);
            subtract_funcname =
                "sakura_SubtractBaselineCubicSplineUsingCoefficientsFloat";
            break;
          default:
            throw(AipsError("Unsupported baseline type."));
          }
          check_sakura_status(subtract_funcname, status);

          // set back a spectrum to data cube
          set_spectrum_to_cube(data_chunk, irow, ipol, num_chan, spec_data);
        } // end of polarization loop

      } // end of chunk row loop
      // write back data and flag cube to VisBuffer
      sdh_->fillCubeToOutputMs(vb, data_chunk, &flag_chunk);
    } // end of vi loop
  } // end of chunk loop

  finalize_process();
  // destroy baseline contexts
  map< const LIBSAKURA_SYMBOL(BaselineType), std::vector<LIBSAKURA_SYMBOL(BaselineContext) *> >::iterator ctxiter = context_reservoir.begin();
  while (ctxiter != context_reservoir.end()) {
    destroy_baseline_contexts (context_reservoir[(*ctxiter).first]);
    ++ctxiter;
  }
}

// Fit line profile
void SingleDishMS::fitLine(string const& in_column_name, string const& in_spw,
    string const& in_pol, string const& fitfunc, string const& in_nfit,
    string const& tempfile_name, string const& temp_out_ms_name) {
  /*
   std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << std::flush << std::endl;
   std::cout << "datacolumn :  " << in_column_name << std::flush << std::endl;
   std::cout << "spw :         " << in_spw << std::flush << std::endl;
   std::cout << "pol :         " << in_pol << std::flush << std::endl;
   std::cout << "fitfunc :     " << fitfunc << std::flush << std::endl;
   std::cout << "nfit :        " << in_nfit << std::flush << std::endl;
   std::cout << "tempfile :    " << tempfile_name << std::flush << std::endl;
   std::cout << "tempoutfile : " << temp_out_ms_name << std::flush << std::endl;
   std::cout << "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&" << std::flush << std::endl;
   */

  LogIO os(_ORIGIN);
  os << "Fitting line profile with " << fitfunc << LogIO::POST;
  // in_column = [FLOAT_DATA|DATA|CORRECTED_DATA]
  // no iteration is necessary for the processing.
  // procedure
  // 1. iterate over MS
  // 2. pick single spectrum from in_column (this is not actually necessary for simple scaling but for exibision purpose)
  // 3. fit Gaussian or Lorentzian profile to each spectrum
  // 4. write fitting results to outfile

  //double tstart = gettimeofday_sec();

  Block<Int> columns(1);
  columns[0] = MS::DATA_DESC_ID;
  LIBSAKURA_SYMBOL (Status)
  status;

  if (file_exists(tempfile_name)) {
    throw(AipsError("temporary file unexpectedly exists."));
  }
  ofstream ofs(tempfile_name);

  //string temp_out_ms_name = tempfile_name + "_temp_output_ms";
  if (file_exists(temp_out_ms_name)) {
    throw(AipsError("temporary ms file unexpectedly exists."));
  }
  prepare_for_process(in_column_name, temp_out_ms_name, columns, false);
  vi::VisibilityIterator2 *vi = sdh_->getVisIter();
  vi::VisBuffer2 *vb = vi->getVisBuffer();

  Vector<Int> recspw;
  Matrix<Int> recchan;
  Vector<size_t> nchan;
  Vector<Vector<Bool> > in_mask;
  Vector<bool> nchan_set;
  parse_spw(in_spw, recspw, recchan, nchan, in_mask, nchan_set);
  Vector<bool> pol;
  bool pol_set = false;
  std::vector<string> nfit_s = split_string(in_nfit, ',');
  std::vector<size_t> nfit;
  nfit.resize(nfit_s.size());
  for (size_t i = 0; i < nfit_s.size(); ++i) {
    nfit[i] = std::stoi(nfit_s[i]);
  }

  for (vi->originChunks(); vi->moreChunks(); vi->nextChunk()) {
    for (vi->origin(); vi->more(); vi->next()) {
      Vector<Int> scans = vb->scan();
      Vector<Double> times = vb->time();
      Vector<Int> beams = vb->feed1();
      Vector<Int> antennas = vb->antenna1();

      Vector<Int> data_spw = vb->spectralWindows();
      size_t const num_chan = static_cast<size_t>(vb->nChannels());
      size_t const num_pol = static_cast<size_t>(vb->nCorrelations());
      size_t const num_row = static_cast<size_t>(vb->nRows());
      Cube<Float> data_chunk(num_pol, num_chan, num_row);
      Vector<float> spec(num_chan);
      Cube<Bool> flag_chunk(num_pol, num_chan, num_row);
      Vector<bool> mask(num_chan);
      // CAUTION!!!
      // data() method must be used with special care!!!
      float *spec_data = spec.data();
      bool *mask_data = mask.data();

      bool new_nchan = false;
      get_nchan_and_mask(recspw, data_spw, recchan, num_chan, nchan, in_mask,
          nchan_set, new_nchan);

      // get data/flag cubes (npol*nchan*nrow) from VisBuffer
      get_data_cube_float(*vb, data_chunk);
      get_flag_cube(*vb, flag_chunk);

      if (!pol_set) {
        get_pol_selection(in_pol, num_pol, pol);
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

        std::vector<size_t> fitrange_start;
        fitrange_start.clear();
        std::vector<size_t> fitrange_end;
        fitrange_end.clear();
        for (size_t i = 0; i < recchan.nrow(); ++i) {
          if (recchan.row(i)(0) == data_spw[irow]) {
            fitrange_start.push_back(recchan.row(i)(1));
            fitrange_end.push_back(recchan.row(i)(2));
          }
        }
        if (nfit.size() != fitrange_start.size()) {
          throw(AipsError(
              "the number of elements of nfit and fitranges specified in spw must be identical."));
        }

        // loop over polarization
        for (size_t ipol = 0; ipol < num_pol; ++ipol) {
          // skip spectrum not selected by pol
          if (!pol(ipol)) {
            continue;
          }
          // get a channel mask from data cube
          // (note that the variable 'mask' is flag in the next line
          // actually, then it will be converted to real mask when
          // taking AND with user-given mask info. this is just for
          // saving memory usage...)
          get_flag_from_cube(flag_chunk, irow, ipol, num_chan, mask_data);
          // skip spectrum if all channels flagged
          if (allchannels_flagged(num_chan, mask_data)) {
            continue;
          }

          // convert flag to mask by taking logical NOT of flag
          // and then operate logical AND with in_mask
          for (size_t ichan = 0; ichan < num_chan; ++ichan) {
            mask_data[ichan] = in_mask[idx][ichan] && (!(mask_data[ichan]));
          }
          // get a spectrum from data cube
          get_spectrum_from_cube(data_chunk, irow, ipol, num_chan, spec_data);

          Vector<Float> x_;
          x_.resize(num_chan);
          Vector<Float> y_;
          y_.resize(num_chan);
          Vector<Bool> m_;
          m_.resize(num_chan);
          for (size_t ichan = 0; ichan < num_chan; ++ichan) {
            x_[ichan] = static_cast<Float>(ichan);
            y_[ichan] = spec_data[ichan];
          }
          Vector<Float> parameters_;
          Vector<Float> error_;

          PtrBlock<Function<Float>*> funcs_;
          std::vector<std::string> funcnames_;
          std::vector<int> funccomponents_;
          std::string expr;
          if (fitfunc == "gaussian") {
            expr = "gauss";
          } else if (fitfunc == "lorentzian") {
            expr = "lorentz";
          }

          for (size_t ifit = 0; ifit < nfit.size(); ++ifit) {
            if (nfit[ifit] == 0)
              continue;

            if (0 < ifit)
              ofs << ":";

            /*
             std::cout << "### [ ifit = " << ifit << "] ###" << std::flush << std::endl;
             std::cout << "   start(" << fitrange_start[ifit] << ") - end(" << fitrange_end[ifit]
             << ")" << std::flush << std::endl;
             */

            //extract spec/mask within fitrange
            for (size_t ichan = 0; ichan < num_chan; ++ichan) {
              if ((fitrange_start[ifit] <= ichan)
                  && (ichan <= fitrange_end[ifit])) {
                m_[ichan] = mask_data[ichan];
              } else {
                m_[ichan] = False;
              }
            }

            //initial guesss
            Vector<Float> peak;
            Vector<Float> cent;
            Vector<Float> fwhm;
            peak.resize(nfit[ifit]);
            cent.resize(nfit[ifit]);
            fwhm.resize(nfit[ifit]);
            if (nfit[ifit] == 1) {
              Float sum = 0.0;
              Float max_spec = y_[fitrange_start[ifit]];
              Float max_spec_x = x_[fitrange_start[ifit]];
              for (size_t ichan = fitrange_start[ifit];
                  ichan <= fitrange_end[ifit]; ++ichan) {
                sum += y_[ichan];
                if (max_spec < y_[ichan]) {
                  max_spec = y_[ichan];
                  max_spec_x = x_[ichan];
                }
              }
              peak[0] = max_spec;
              cent[0] = max_spec_x;
              fwhm[0] = sum / max_spec * 0.7;
            } else {
              size_t x_start = fitrange_start[ifit];
              size_t x_width = (fitrange_end[ifit] - fitrange_start[ifit])
                  / nfit[ifit];
              size_t x_end = x_start + x_width;
              for (size_t icomp = 0; icomp < nfit[ifit]; ++icomp) {
                if (icomp == nfit[ifit] - 1) {
                  x_end = fitrange_end[ifit] + 1;
                }

                Float sum = 0.0;
                Float max_spec = y_[x_start];
                Float max_spec_x = x_[x_start];
                for (size_t ichan = x_start; ichan < x_end; ++ichan) {
                  sum += y_[ichan];
                  if (max_spec < y_[ichan]) {
                    max_spec = y_[ichan];
                    max_spec_x = x_[ichan];
                  }
                }
                peak[icomp] = max_spec;
                cent[icomp] = max_spec_x;
                fwhm[icomp] = sum / max_spec * 0.7;

                x_start += x_width;
                x_end += x_width;
              }
            }

            //fitter setup
            funcs_.resize(nfit[ifit]);
            funcnames_.clear();
            funccomponents_.clear();
            for (size_t icomp = 0; icomp < funcs_.nelements(); ++icomp) {
              funcs_[icomp] = new Gaussian1D<Float>();
              (funcs_[icomp]->parameters())[0] = peak[icomp]; //initial guess (peak)
              (funcs_[icomp]->parameters())[1] = cent[icomp]; //initial guess (centre)
              (funcs_[icomp]->parameters())[2] = fwhm[icomp]; //initial guess (fwhm)
              funcnames_.push_back(expr);
              funccomponents_.push_back(3);
            }

            //actual fitting
            NonLinearFitLM < Float > fitter;
            CompoundFunction < Float > func;
            for (size_t icomp = 0; icomp < funcs_.nelements(); ++icomp) {
              func.addFunction(*funcs_[icomp]);
            }
            fitter.setFunction(func);
            fitter.setMaxIter(50 + 10 * funcs_.nelements());
            fitter.setCriteria(0.001);      // Convergence criterium

            parameters_.resize();
            parameters_ = fitter.fit(x_, y_, &m_);
            if (!fitter.converged()) {
              throw(AipsError("Failed in fitting. Fitter did not converged. "));
            }
            error_.resize();
            error_ = fitter.errors();

            //write best-fit parameters to tempfile/outfile
            for (size_t icomp = 0; icomp < funcs_.nelements(); ++icomp) {
              if (0 < icomp)
                ofs << ":";
              size_t offset = 3 * icomp;
              ofs.precision(4);
              ofs.setf(ios::fixed);
              ofs << scans[irow] << ","     // scanID
                  << times[irow] << ","     // time
                  << antennas[irow] << ","  // antennaID
                  << beams[irow] << ","     // beamID
                  << data_spw[irow] << ","  // spwID
                  << ipol << ",";           // polID
              ofs.precision(8);
              ofs << parameters_[offset + 1] << "," << error_[offset + 1] << "," // cent
                  << parameters_[offset + 0] << "," << error_[offset + 0] << "," // peak
                  << parameters_[offset + 2] << "," << error_[offset + 2]; // fwhm
            }
            /*
             for (size_t iparam = 0; iparam < parameters_.nelements(); ++iparam) {
             std::cout << "   result[" << iparam << "] = " << parameters_[iparam] << std::flush << std::endl;
             }
             for (size_t ierr = 0; ierr < error_.nelements(); ++ierr) {
             std::cout << "      err[" << ierr << "] = " << error_[ierr] << std::flush << std::endl;
             }
             std::cout << "###--------------------------###" << std::flush << std::endl;
             */
          }        //end of nfit loop

          ofs << "\n";

        }        //end of polarization loop
      }        // end of MS row loop
    }        //end of vi loop
  }        //end of chunk loop

  finalize_process();
  ofs.close();
  //double tend = gettimeofday_sec();
  //std::cout << "Elapsed time = " << (tend - tstart) << " sec." << std::endl;
}

// Baseline subtraction by per spectrum fitting parameters
void SingleDishMS::subtractBaselineVariable(string const& in_column_name,
    string const& out_ms_name, string const& out_bltable_name,
    string const& out_blformat_name, string const& out_bloutput_name,
    bool const& do_subtract, string const& in_spw, string const& in_ppp,
    string const& param_file) {

  split_bloutputname(out_bloutput_name);

  LogIO os(_ORIGIN);
  os << "Fitting and subtracting baseline using parameters in file, "
      << param_file << LogIO::POST;

  // parse fitting parameters in the text file
  BLParameterParser parser(param_file);
  std::vector<LIBSAKURA_SYMBOL(BaselineType)> baseline_types =
      parser.get_function_types();
  map<const LIBSAKURA_SYMBOL(BaselineType), uint16_t> max_orders;
  for (size_t i = 0; i < baseline_types.size(); ++i) {
    max_orders[baseline_types[i]] = parser.get_max_order(baseline_types[i]);
  }
  { //DEBUG ouput
    os << LogIO::DEBUG1 << "Baseline Types = " << baseline_types << LogIO::POST;
    os << LogIO::DEBUG1 << "Max Orders:" << LogIO::POST;
    map<const LIBSAKURA_SYMBOL(BaselineType), uint16_t>::iterator iter =
        max_orders.begin();
    while (iter != max_orders.end()) {
      os << LogIO::DEBUG1 << "- type " << (*iter).first << ": "
          << (*iter).second << LogIO::POST;
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

  ofstream ofs_csv;
  ofstream ofs_txt;
  bool write_baseline_csv = (bloutputname_csv != "");
  bool write_baseline_text = (bloutputname_text != "");
  bool write_baseline_table = (bloutputname_table != "");

  if (write_baseline_csv) {
    ofs_csv.open(bloutputname_csv.c_str());
  }
  if (write_baseline_text) {
    ofs_txt.open(bloutputname_text.c_str(), std::ios::app);
  }

  //bool write_baseline_table = (out_bltable_name != "");
  if (write_baseline_table)
    bt = new BaselineTable(vi->ms());

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
    map<const LIBSAKURA_SYMBOL(BaselineType), uint16_t>::iterator iter =
        max_orders.begin();
    while (iter != max_orders.end()) {
      context_reservoir[(*iter).first] = std::vector<LIBSAKURA_SYMBOL(BaselineContext) *>();
      ++iter;
    }
  }

  LIBSAKURA_SYMBOL (Status)
  status;
  LIBSAKURA_SYMBOL (BaselineStatus)
  bl_status;
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
      Vector<Int> antennas = vb->antenna1();
      Vector<Int> data_spw = vb->spectralWindows();
      size_t const num_chan = static_cast<size_t>(vb->nChannels());
      size_t const num_pol = static_cast<size_t>(vb->nCorrelations());
      size_t const num_row = static_cast<size_t>(vb->nRows());
      Vector<uInt> orig_rows = vb->rowIds();
      Cube<Float> data_chunk(num_pol, num_chan, num_row);
      Vector<float> spec(num_chan);
      Cube<Bool> flag_chunk(num_pol, num_chan, num_row);
      Vector<bool> mask(num_chan);
      //cout << "New iteration: num_row=" << num_row << ", num_chan=" << num_chan << ", num_pol=" << num_pol << ", spwid=" << data_spw << endl;

      Vector<bool> mask2(num_chan);
      // CAUTION!!!
      // data() method must be used with special care!!!
      float *spec_data = spec.data();
      bool *mask_data = mask.data();
      bool *mask2_data = mask2.data();

      uInt final_mask[num_pol];
      uInt final_mask2[num_pol];

      final_mask[0] = 0;
      final_mask[1] = 0;
      final_mask2[0] = 0;
      final_mask2[1] = 0;

      bool new_nchan = false;
      get_nchan_and_mask(recspw, data_spw, recchan, num_chan, nchan, in_mask,
          nchan_set, new_nchan);
      if (new_nchan) {
        // Generate context for all necessary baseline types
        map<const LIBSAKURA_SYMBOL(BaselineType), uint16_t>::iterator iter =
            max_orders.begin();
        while (iter != max_orders.end()) {
          get_baseline_context((*iter).first, (*iter).second, num_chan, nchan,
              nchan_set, ctx_indices, context_reservoir[(*iter).first]);
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
        size_t num_apply_true = 0;
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
        Array<Bool> uself_mtx(IPosition(2, num_pol, 1));
        Array<Float> lfthres_mtx(IPosition(2, num_pol, 1));
        Array<uInt> lfavg_mtx(IPosition(2, num_pol, 1));
        Array<uInt> lfedge_mtx(IPosition(2, num_pol, 2));

        // loop over polarization
        for (size_t ipol = 0; ipol < num_pol; ++ipol) {
          apply_mtx[0][ipol] = True;
          // skip spectrum not selected by pol
          if (!pol(ipol)) {
            flag_spectrum_in_cube(flag_chunk, irow, ipol); //flag
            apply_mtx[0][ipol] = False;
            continue;
          }
          // get a channel mask from data cube
          // (note that the variable 'mask' is flag in the next line
          // actually, then it will be converted to real mask when
          // taking AND with user-given mask info. this is just for
          // saving memory usage...)
          get_flag_from_cube(flag_chunk, irow, ipol, num_chan, mask_data);
          // skip spectrum if all channels flagged
          if (allchannels_flagged(num_chan, mask_data)) {
            apply_mtx[0][ipol] = False;
            continue;
          }

          // convert flag to mask by taking logical NOT of flag
          // and then operate logical AND with in_mask
          for (size_t ichan = 0; ichan < num_chan; ++ichan) {
            mask_data[ichan] = in_mask[idx][ichan] && (!(mask_data[ichan]));
          }
          // get fitting parameter
          BLParameterSet fit_param;
          if (!parser.GetFitParameter(orig_rows[irow], ipol, fit_param)) { //no fit requrested
            flag_spectrum_in_cube(flag_chunk, irow, ipol);
            os << LogIO::DEBUG1 << "Row " << orig_rows[irow] << ", Pol " << ipol
                << ": Fit not requested. Skipping." << LogIO::POST;
            apply_mtx[0][ipol] = False;
            continue;
          }
          if (true) {
            os << "Fitting Parameter" << LogIO::POST;
            os << "[ROW" << orig_rows[irow] << ", POL" << ipol << "]"
                << LogIO::POST;
            fit_param.PrintSummary();
          }
          // get mask from BLParameterset and create composit mask
          if (fit_param.baseline_mask != "") {
            stringstream local_spw;
            local_spw << data_spw[irow] << ":" << fit_param.baseline_mask;
            //cout << "row " << orig_rows[irow] << ": Generating local mask with selection " << local_spw.str() << endl;
            Record selrec = sdh_->getSelRec(local_spw.str());
            Matrix<Int> local_rec_chan = selrec.asArrayInt("channel");
            Vector<Bool> local_mask(num_chan, False);
            get_mask_from_rec(data_spw[irow], local_rec_chan, local_mask,
                false);
            for (size_t ichan = 0; ichan < num_chan; ++ichan) {
              mask_data[ichan] = mask_data[ichan] && local_mask[ichan];
            }
          }
          // check for composit mask and flag if no valid channel to fit
          if (NValidMask(num_chan, mask_data) == 0) {
            flag_spectrum_in_cube(flag_chunk, irow, ipol);
            os << LogIO::DEBUG1 << "Row " << orig_rows[irow] << ", Pol " << ipol
                << ": No valid channel to fit. Skipping" << LogIO::POST;
            apply_mtx[0][ipol] = False;
            continue;
          }
          // get a spectrum from data cube
          get_spectrum_from_cube(data_chunk, irow, ipol, num_chan, spec_data);

          // actual execution of single spectrum
          map< const LIBSAKURA_SYMBOL(BaselineType),
          std::vector<LIBSAKURA_SYMBOL(BaselineContext) *> >::iterator
          iter = context_reservoir.find(fit_param.baseline_type);
          if (iter == context_reservoir.end())
            throw(AipsError("Invalid baseline type detected!"));
          LIBSAKURA_SYMBOL(BaselineContext) * context =
              (*iter).second[ctx_indices[idx]];
          //cout << "Got context for type " << (*iter).first << ": idx=" << ctx_indices[idx] << endl;

          // Number of coefficients to fit this spectrum
          size_t num_coeff;
          switch (fit_param.baseline_type) {
          case LIBSAKURA_SYMBOL(BaselineType_kPolynomial):
          case LIBSAKURA_SYMBOL(BaselineType_kChebyshev):
            status = LIBSAKURA_SYMBOL(GetNumberOfCoefficients)(context,
                fit_param.order, &num_coeff);
            check_sakura_status("sakura_GetNumberOfCoefficients", status);
            break;
          case LIBSAKURA_SYMBOL(BaselineType_kCubicSpline):
            num_coeff = 4 * fit_param.npiece;
            break;
          default:
            throw(AipsError("Unsupported baseline type."));
          }
          // Final check of the valid number of channels
          size_t num_min =
              fit_param.baseline_type
                  == LIBSAKURA_SYMBOL(BaselineType_kCubicSpline) ?
                  fit_param.npiece + 3 : num_coeff;
          if (NValidMask(num_chan, mask_data) < num_min) {
            flag_spectrum_in_cube(flag_chunk, irow, ipol);
            apply_mtx[0][ipol] = False;
            os << LogIO::WARN
                << "Too few valid channels to fit. Skipping Antenna "
                << antennas[irow] << ", Beam " << beams[irow] << ", SPW "
                << data_spw[irow] << ", Pol " << ipol << ", Time "
                << MVTime(times[irow] / 24. / 3600.).string(MVTime::YMD, 8)
                << LogIO::POST;
            continue;
          }

          float rms;
          size_t num_boundary = 0;
          if (fit_param.baseline_type
              == LIBSAKURA_SYMBOL(BaselineType_kCubicSpline)) {
            num_boundary = fit_param.npiece;
          }
          Vector<double> boundary(num_boundary);
          // CAUTION!!!
          // data() method must be used with special care!!!
          double *boundary_data = boundary.data();

          if (write_baseline_text == true || write_baseline_csv == true
              || write_baseline_table == true) {
            num_apply_true++;
            bltype_mtx[0][ipol] = (uInt) fit_param.baseline_type;
            Int fpar_tmp;
            switch (fit_param.baseline_type) {
            case LIBSAKURA_SYMBOL(BaselineType_kPolynomial):
            case LIBSAKURA_SYMBOL(BaselineType_kChebyshev):
              fpar_tmp = (Int) fit_param.order;
              break;
            case LIBSAKURA_SYMBOL(BaselineType_kCubicSpline):
              fpar_tmp = (Int) fit_param.npiece;
              break;
            default:
              throw(AipsError("Unsupported baseline type."));
            }
            fpar_mtx[0][ipol] = fpar_tmp;

            if (num_coeff > num_coeff_max) {
              num_coeff_max = num_coeff;
            }
            Vector<double> coeff(num_coeff);
            // CAUTION!!!
            // data() method must be used with special care!!!
            double *coeff_data = coeff.data();
            string get_coeff_funcname;
            switch (fit_param.baseline_type) {
            case LIBSAKURA_SYMBOL(BaselineType_kPolynomial):
            case LIBSAKURA_SYMBOL(BaselineType_kChebyshev):
              status = LIBSAKURA_SYMBOL(GetBestFitBaselineCoefficientsFloat)(
                  context, num_chan, spec_data, mask_data,
                  fit_param.clip_threshold_sigma, fit_param.num_fitting_max,
                  num_coeff, coeff_data, mask2_data, &rms, &bl_status);

              for (size_t i = 0; i < num_chan; ++i) {
                if (mask_data[i] == false) {
                  final_mask[ipol] += 1;
                }
                if (mask2_data[i] == false) {
                  final_mask2[ipol] += 1;
                }
              }

              get_coeff_funcname = "sakura_GetBestFitBaselineCoefficientsFloat";
              break;
            case LIBSAKURA_SYMBOL(BaselineType_kCubicSpline):
              status = LIBSAKURA_SYMBOL(
                  GetBestFitBaselineCoefficientsCubicSplineFloat)(context,
                  num_chan, spec_data, mask_data,
                  fit_param.clip_threshold_sigma, fit_param.num_fitting_max,
                  fit_param.npiece, coeff_data, mask2_data, &rms, boundary_data,
                  &bl_status);

              for (size_t i = 0; i < num_chan; ++i) {
                if (mask_data[i] == false) {
                  final_mask[ipol] += 1;
                }
                if (mask2_data[i] == false) {
                  final_mask2[ipol] += 1;
                }
              }

              get_coeff_funcname =
                  "sakura_GetBestFitBaselineCoefficientsCubicSplineFloat";
              break;
            default:
              throw(AipsError("Unsupported baseline type."));
            }
            check_sakura_status(get_coeff_funcname, status);
            for (size_t icoeff = 0; icoeff < num_coeff; ++icoeff) {
              coeff_mtx_tmp[ipol].push_back(coeff_data[icoeff]);
            }
            Vector<uInt> masklist;
            get_masklist_from_mask(num_chan, mask2_data, masklist);
            if (masklist.size() > num_masklist_max) {
              num_masklist_max = masklist.size();
            }
            masklist_mtx_tmp[ipol].clear();
            for (size_t imask = 0; imask < masklist.size(); ++imask) {
              masklist_mtx_tmp[ipol].push_back(masklist[imask]);
            }

            string subtract_funcname;
            switch (fit_param.baseline_type) {
            case LIBSAKURA_SYMBOL(BaselineType_kPolynomial):
            case LIBSAKURA_SYMBOL(BaselineType_kChebyshev):
              status = LIBSAKURA_SYMBOL(SubtractBaselineUsingCoefficientsFloat)(
                  context, num_chan, spec_data, num_coeff, coeff_data,
                  spec_data);
              subtract_funcname =
                  "sakura_SubtractBaselineUsingCoefficientsFloat";
              break;
            case LIBSAKURA_SYMBOL(BaselineType_kCubicSpline):
              status = LIBSAKURA_SYMBOL(
                  SubtractBaselineCubicSplineUsingCoefficientsFloat)(context,
                  num_chan, spec_data, fit_param.npiece, coeff_data,
                  boundary_data, spec_data);
              subtract_funcname =
                  "sakura_SubtractBaselineCubicSplineUsingCoefficientsFloat";
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
            if (fit_param.baseline_type
                == LIBSAKURA_SYMBOL(BaselineType_kCubicSpline)) {
              for (size_t ipiece = 0; ipiece < fit_param.npiece; ++ipiece) {
                ffpar_mtx_tmp[ipol].push_back(boundary_data[ipiece]);
              }
            }

            rms_mtx[0][ipol] = rms;

            cthres_mtx[0][ipol] = fit_param.clip_threshold_sigma;
            citer_mtx[0][ipol] = (uInt) fit_param.num_fitting_max - 1;
            uself_mtx[0][ipol] = (Bool) fit_param.line_finder.use_line_finder;
            lfthres_mtx[0][ipol] = fit_param.line_finder.threshold;
            lfavg_mtx[0][ipol] = fit_param.line_finder.chan_avg_limit;
            for (size_t iedge = 0; iedge < 2; ++iedge) {
              lfedge_mtx[iedge][ipol] = fit_param.line_finder.edge[iedge];
            }

          } else {
            string subtract_funcname;
            switch (fit_param.baseline_type) {
            case LIBSAKURA_SYMBOL(BaselineType_kPolynomial):
            case LIBSAKURA_SYMBOL(BaselineType_kChebyshev):
              //cout << (fit_param.baseline_type==0 ? "poly" : "chebyshev") << ": order=" << fit_param.order << ", row=" << orig_rows[irow] << ", pol=" << ipol << ", num_chan=" << num_chan << ", num_valid_chan = " << NValidMask(num_chan, mask_data) << endl;
              status = LIBSAKURA_SYMBOL(SubtractBaselineFloat)(context,
                  fit_param.order, num_chan, spec_data, mask_data,
                  fit_param.clip_threshold_sigma, fit_param.num_fitting_max,
                  true, mask_data, spec_data, &rms,
                  &bl_status);
              subtract_funcname = "sakura_SubtractBaselineFloat";
              break;
            case LIBSAKURA_SYMBOL(BaselineType_kCubicSpline):
              //cout << "cspline: npiece = " << fit_param.npiece << ", row=" << orig_rows[irow] << ", pol=" << ipol << ", num_chan=" << num_chan << ", num_valid_chan = " << NValidMask(num_chan, mask_data) << endl;
              status = LIBSAKURA_SYMBOL(SubtractBaselineCubicSplineFloat)(
                  context, fit_param.npiece, num_chan, spec_data, mask_data,
                  fit_param.clip_threshold_sigma, fit_param.num_fitting_max,
                  true, mask_data, spec_data, &rms, boundary_data,
                  &bl_status);
              subtract_funcname = "sakura_SubtractBaselineCubicSplineFloat";
              break;
            default:
              throw(AipsError("Unsupported baseline type."));
            }
            check_sakura_status(subtract_funcname, status);
          }
          // set back a spectrum to data cube
          if (do_subtract) {
            set_spectrum_to_cube(data_chunk, irow, ipol, num_chan, spec_data);
          }
        } // end of polarization loop

        if (write_baseline_table) {
          //write to baseline table if there is apply=True spectrum.
          if (num_apply_true > 0) {
            Array<Float> ffpar_mtx(IPosition(2, num_pol, num_ffpar_max));
            set_matrix_for_bltable<double, Float>(num_pol, num_ffpar_max,
                ffpar_mtx_tmp, ffpar_mtx);
            Array<uInt> masklist_mtx(IPosition(2, num_pol, num_masklist_max));
            set_matrix_for_bltable<uInt, uInt>(num_pol, num_masklist_max,
                masklist_mtx_tmp, masklist_mtx);
            Array<Float> coeff_mtx(IPosition(2, num_pol, num_coeff_max));
            set_matrix_for_bltable<double, Float>(num_pol, num_coeff_max,
                coeff_mtx_tmp, coeff_mtx);
            bt->appenddata((uInt) scans[irow], (uInt) beams[irow],
                (uInt) antennas[irow], (uInt) data_spw[irow], 0, times[irow],
                apply_mtx, bltype_mtx, fpar_mtx, ffpar_mtx, masklist_mtx,
                coeff_mtx, rms_mtx, (uInt) num_chan, cthres_mtx, citer_mtx,
                uself_mtx, lfthres_mtx, lfavg_mtx, lfedge_mtx);
          }
        }

        if (write_baseline_text) {
          if (num_apply_true > 0) {

            /*
             int year;
             int month;
             int day;
             int hour;
             int minute;
             double second;
             */

            //MJD(second)-> year,month,day,hour,minute,second
            ///MJDtoYMDhms(&year, &month, &day, &hour, &minute, &second);

            Array<Float> ffpar_mtx(IPosition(2, num_pol, num_ffpar_max));
            set_matrix_for_bltable<double, Float>(num_pol, num_ffpar_max,
                ffpar_mtx_tmp, ffpar_mtx);

            Array<uInt> masklist_mtx(IPosition(2, num_pol, num_masklist_max));
            set_matrix_for_bltable<uInt, uInt>(num_pol, num_masklist_max,
                masklist_mtx_tmp, masklist_mtx);

            Array<Float> coeff_mtx(IPosition(2, num_pol, num_coeff_max));
            set_matrix_for_bltable<double, Float>(num_pol, num_coeff_max,
                coeff_mtx_tmp, coeff_mtx);

            Matrix<uInt> masklist_mtx2 = masklist_mtx;
            for (size_t ipol = 0; ipol < num_pol; ++ipol) {
              Matrix<Bool> apply_mtx2 = apply_mtx[0][ipol];
              if (apply_mtx2(0, 0) == True) {
                ofs_txt << "Scan" << '[' << (uInt) scans[irow] << ']' << ' '
                    << "Beam" << '[' << (uInt) beams[irow] << ']' << ' '
                    << "Spw" << '[' << (uInt) data_spw[irow] << ']' << ' '
                    << "Pol" << '[' << ipol << ']' << ' '
                    //<< "Time" <<'[' <<  times[irow] << ']' << endl;
                    //<< "Time" <<'[' << year << '/' << month << '/' << day << '/' << hour << ':' << minute << ':' << second  << ']' << endl;
                    << "Time" << '['
                    << MVTime(times[irow] / 24. / 3600.).string(MVTime::YMD, 8)
                    << ']' << endl;
                /*         << "Time" <<'[' << year << '/'
                 << setfill('0') << setw(2) << month << '/'
                 << setfill('0') << setw(2) << day <<  '/'
                 << setfill('0') << setw(2) << hour << ':'
                 << setfill('0') << setw(2) << minute  << ':';
                 ofs_txt.setf(ios::fixed);
                 ofs_txt << setprecision(3) << second << ']'
                 << endl;
                 */

                ofs_txt << endl;
                ofs_txt << "Fitter range = " << '[';

                for (size_t imasklist = 0; imasklist < num_masklist_max / 2;
                    ++imasklist) {
                  if (imasklist == 0) {
                    ofs_txt << '[' << masklist_mtx2(ipol, 2 * imasklist) << ';'
                        << masklist_mtx2(ipol, 2 * imasklist + 1) << ']';
                  }
                  if (imasklist >= 1
                      && (0 != masklist_mtx2(ipol, 2 * imasklist)
                          && 0 != masklist_mtx2(ipol, 2 * imasklist + 1))) {
                    ofs_txt << ",[" << masklist_mtx2(ipol, 2 * imasklist) << ','
                        << masklist_mtx2(ipol, 2 * imasklist + 1) << ']';
                  }
                }

                ofs_txt << ']' << endl;
                ;
                ofs_txt << endl;
                Matrix<uInt> bltype_mtx2 = bltype_mtx[0][ipol];
                Matrix<Int> fpar_mtx2 = fpar_mtx[0][ipol];
                Matrix<Float> rms_mtx2 = rms_mtx[0][ipol];
                string bltype_name;
                if (bltype_mtx2(0, 0) == (uInt) 0) {
                  bltype_name = "poly";
                }
                if (bltype_mtx2(0, 0) == (uInt) 1) {
                  bltype_name = "chebyshev";
                }

                if (bltype_mtx2(0, 0) == (uInt) 2) {
                  bltype_name = "cspline";
                }

                ofs_txt << "Baseline parameters  Function = "
                    << bltype_name.c_str() << ' ' << " npiece = "
                    << fpar_mtx2(0, 0) << endl;
                ofs_txt << endl;
                ofs_txt << "Results of baseline fit" << endl;
                ofs_txt << endl;
                Matrix<Float> coeff_mtx2 = coeff_mtx;
                for (size_t icoeff = 0; icoeff < num_coeff_max; ++icoeff) {
                  //ofs_txt << "p" << icoeff << " = " << coeff_mtx2(ipol,icoeff) << "  ";
                  ofs_txt << "p" << icoeff << " = ";
                  ofs_txt.setf(ios::scientific);
                  ofs_txt << setprecision(6) << coeff_mtx2(ipol, icoeff)
                      << "  ";
                }
                ofs_txt << endl;
                ofs_txt << endl;
                ofs_txt << "rms = ";
                ofs_txt.setf(ios::fixed);
                ofs_txt << setprecision(6) << rms_mtx2(0, 0) << endl;
                ofs_txt << endl;
                //ofs_txt << "Number of clipped channels = " << (uInt)num_chan << endl;
                ofs_txt << "Number of clipped channels = "
                    << final_mask2[ipol] - final_mask[ipol] << endl;
                ofs_txt << endl;
                ofs_txt
                    << "------------------------------------------------------"
                    << endl;
                ofs_txt << endl;
              }                            //if apply_mtx = true
            }                            //loop ipol
          }
        }

        if (write_baseline_csv) {
          if (num_apply_true > 0) {

            Array<Float> ffpar_mtx(IPosition(2, num_pol, num_ffpar_max));
            set_matrix_for_bltable<double, Float>(num_pol, num_ffpar_max,
                ffpar_mtx_tmp, ffpar_mtx);

            Array<uInt> masklist_mtx(IPosition(2, num_pol, num_masklist_max));
            set_matrix_for_bltable<uInt, uInt>(num_pol, num_masklist_max,
                masklist_mtx_tmp, masklist_mtx);

            Array<Float> coeff_mtx(IPosition(2, num_pol, num_coeff_max));
            set_matrix_for_bltable<double, Float>(num_pol, num_coeff_max,
                coeff_mtx_tmp, coeff_mtx);

            Matrix<uInt> masklist_mtx2 = masklist_mtx;
            for (size_t ipol = 0; ipol < num_pol; ++ipol) {
              Matrix<Bool> apply_mtx2 = apply_mtx[0][ipol];
              if (apply_mtx2(0, 0) == True) {
                ofs_csv << (uInt) scans[irow] << ',' << (uInt) beams[irow]
                    << ',' << (uInt) data_spw[irow] << ',' << ipol << ','
                    << times[irow] << ',';
                ofs_csv << '[';
                for (size_t imasklist = 0; imasklist < num_masklist_max / 2;
                    ++imasklist) {
                  if (imasklist == 0) {
                    ofs_csv << '[' << masklist_mtx2(ipol, 2 * imasklist) << ';'
                        << masklist_mtx2(ipol, 2 * imasklist + 1) << ']';
                  }
                  if (imasklist >= 1
                      && (0 != masklist_mtx2(ipol, 2 * imasklist)
                          && 0 != masklist_mtx2(ipol, 2 * imasklist + 1))) {
                    ofs_csv << ";[" << masklist_mtx2(ipol, 2 * imasklist) << ';'
                        << masklist_mtx2(ipol, 2 * imasklist + 1) << ']';
                  }
                }
                ofs_csv << ']' << ',';
                Matrix<uInt> bltype_mtx2 = bltype_mtx[0][ipol];
                string bltype_name;
                if (bltype_mtx2(0, 0) == (uInt) 0) {
                  bltype_name = "poly";
                }
                if (bltype_mtx2(0, 0) == (uInt) 1) {
                  bltype_name = "chebyshev";
                }
                if (bltype_mtx2(0, 0) == (uInt) 2) {
                  bltype_name = "cspline";
                }
                Matrix<Int> fpar_mtx2 = fpar_mtx;
                Matrix<Float> coeff_mtx2 = coeff_mtx;
                ofs_csv << bltype_name.c_str() << ',' << fpar_mtx2(ipol, 0)
                    << ',';

                for (size_t icoeff = 0; icoeff < num_coeff_max; ++icoeff) {
                  //ofs_csv << coeff_mtx[icoeff][ipol] << ',';
                  ofs_csv << coeff_mtx2(ipol, icoeff) << ',';
                }
                Matrix<Float> rms_mtx2 = rms_mtx;
                ofs_csv << rms_mtx2(ipol, 0) << ',';
                //ofs_csv << rms_mtx[0][ipol] << ',';
                //ofs_csv << (uInt)num_chan ;
                ofs_csv << final_mask2[ipol] - final_mask[ipol];
                ofs_csv << endl;
                ofs_csv << endl;
              }                    //if apply_mtx ==true
            }                    //loop ipol
          }
        }

      } // end of chunk row loop
      // write back data and flag cube to VisBuffer
      sdh_->fillCubeToOutputMs(vb, data_chunk, &flag_chunk);
    } // end of vi loop
  } // end of chunk loop

  if (write_baseline_csv) { //---------------
    ofs_csv.close(); //--------------------
  } //------------------------------------
  if (write_baseline_text) { //--------------
    ofs_txt.close(); //---------------------
  } //-----------------------------------

  if (write_baseline_table) {
    bt->save(bloutputname_table); //------------
    delete bt;
  }
  finalize_process();
  // destroy baseline contexts
  map< const LIBSAKURA_SYMBOL(BaselineType), std::vector<LIBSAKURA_SYMBOL(BaselineContext) *> >::iterator ctxiter = context_reservoir.begin();
  while (ctxiter != context_reservoir.end()) {
    destroy_baseline_contexts (context_reservoir[(*ctxiter).first]);
    ++ctxiter;
  }
}

void SingleDishMS::findLineAndGetMask(size_t const num_data, float const* data,
    bool const* in_mask, float const threshold, int const avg_limit,
    int const minwidth, vector<int> const& edge, bool const invert,
    bool* out_mask) {
  // inpu value check
  AlwaysAssert(minwidth > 0, AipsError);
  AlwaysAssert(avg_limit >= 0, AipsError);
  size_t max_iteration = 10;
  size_t maxwidth = num_data;
  AlwaysAssert(maxwidth > static_cast<size_t>(minwidth), AipsError);
  // edge handling
  pair<size_t, size_t> lf_edge;
  if (edge.size() == 0) {
    lf_edge = pair<size_t, size_t>(0, 0);
  } else if (edge.size() == 1) {
    AlwaysAssert(edge[0] >= 0, AipsError);
    lf_edge = pair<size_t, size_t>(static_cast<size_t>(edge[0]),
        static_cast<size_t>(edge[0]));
  } else {
    AlwaysAssert(edge[0] >= 0 && edge[1] >= 0, AipsError);
    lf_edge = pair<size_t, size_t>(static_cast<size_t>(edge[0]),
        static_cast<size_t>(edge[1]));
  }
  // copy input mask to output mask vector if necessary
  if (&in_mask != &out_mask) {
    for (size_t i = 0; i < num_data; ++i) {
      out_mask[i] = in_mask[i];
    }
  }
  // line detection
  list<pair<size_t, size_t>> line_ranges = linefinder::MADLineFinder(num_data,
      data, out_mask, threshold, max_iteration, static_cast<size_t>(minwidth),
      maxwidth, static_cast<size_t>(avg_limit), lf_edge);
  // debug output
  LogIO os(_ORIGIN);
  os << LogIO::DEBUG1 << line_ranges.size() << " lines found: ";
  for (list<pair<size_t, size_t>>::iterator iter = line_ranges.begin();
      iter != line_ranges.end(); ++iter) {
    os << "[" << (*iter).first << ", " << (*iter).second << "] ";
  }
  os << LogIO::POST;
  if (invert) { // eliminate edge channels from output mask
    if (lf_edge.first > 0)
      line_ranges.push_front(pair<size_t, size_t>(0, lf_edge.first - 1));
    if (lf_edge.second > 0)
      line_ranges.push_back(
          pair<size_t, size_t>(num_data - lf_edge.second, num_data - 1));
  }
  // line mask creation (do not initialize in case of baseline mask)
  linefinder::getMask(num_data, out_mask, line_ranges, invert, !invert);
}

void SingleDishMS::scale(float const factor, string const& in_column_name,
    string const& out_ms_name) {
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
      Cube<Float> data_chunk(num_pol, num_chan, num_row);
      Matrix<Float> data_row(num_pol, num_chan);
      Vector<float> spectrum(num_chan);
      // CAUTION!!!
      // data() method must be used with special care!!!
      float *spectrum_data = spectrum.data();

      // get a data cube (npol*nchan*nrow) from VisBuffer
      get_data_cube_float(*vb, data_chunk);
      // loop over MS rows
      for (size_t irow = 0; irow < num_row; ++irow) {
        // loop over polarization
        for (size_t ipol = 0; ipol < num_pol; ++ipol) {
          // get a spectrum from data cube
          get_spectrum_from_cube(data_chunk, irow, ipol, num_chan, spectrum_data);

          // actual execution of single spectrum
          do_scale(factor, num_chan, spectrum_data);

          // set back a spectrum to data cube
          set_spectrum_to_cube(data_chunk, irow, ipol, num_chan, spectrum_data);
        } // end of polarization loop
      } // end of chunk row loop
      // write back data cube to Output MS
      sdh_->fillCubeToOutputMs(vb, data_chunk);
    } // end of vi loop
  } // end of chunk loop
  finalize_process();
}

void SingleDishMS::do_scale(float const factor, size_t const num_data,
    float *data) {
  for (size_t i = 0; i < num_data; ++i)
    data[i] *= factor;
}

void SingleDishMS::smooth(string const &kernelType, float const kernelWidth,
    string const &columnName, string const &outMSName) {
  LogIO os(_ORIGIN);
  os << "Input parameter summary:" << endl << "   kernelType = " << kernelType
      << endl << "   kernelWidth = " << kernelWidth << endl
      << "   columnName = " << columnName << endl << "   outMSName = "
      << outMSName << LogIO::POST;

  // Initialization
  doSmoothing_ = True;
  prepare_for_process(columnName, outMSName);

  // configure smoothing
  sdh_->setSmoothing(kernelType, kernelWidth);
  sdh_->initializeSmoothing();

  // get VI/VB2 access
  vi::VisibilityIterator2 *visIter = sdh_->getVisIter();
  vi::VisBuffer2 *vb = visIter->getVisBuffer();

  double startTime = gettimeofday_sec();

  for (visIter->originChunks(); visIter->moreChunks(); visIter->nextChunk()) {
    for (visIter->origin(); visIter->more(); visIter->next()) {
      sdh_->fillOutputMs(vb);
    }
  }

  double endTime = gettimeofday_sec();
  os << LogIO::DEBUGGING
      << "Elapsed time for VI/VB loop: " << endTime - startTime << " sec"
      << LogIO::POST;

  // Finalization
  finalize_process();
}

}  // End of casa namespace.

