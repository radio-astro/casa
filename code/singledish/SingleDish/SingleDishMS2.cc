#include <iostream>

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
#include <singledish/SingleDish/SingleDishMS2.h>

//---for measuring elapse time------------------------
// #include <sys/time.h>
// double gettimeofday_sec() {
//   struct timeval tv;
//   gettimeofday(&tv, NULL);
//   return tv.tv_sec + (double)tv.tv_usec*1.0e-6;
// }
//----------------------------------------------------

#define _ORIGIN LogOrigin("SingleDishMS2", __func__, WHERE)

namespace casa {

SingleDishMS2::SingleDishMS2()
  : msname_(""), sdh_(0)
{
  initialize();
}

SingleDishMS2::SingleDishMS2(string const& ms_name)
  : msname_(ms_name), sdh_(0)
{
  LogIO os(_ORIGIN);
  initialize();
}

SingleDishMS2::~SingleDishMS2()
{
  if(sdh_){
    delete sdh_;
    sdh_ = 0;
  }
  msname_ = "";
}

void SingleDishMS2::initialize()
{
  in_column_ = MS::UNDEFINED_COLUMN;
  //  out_column_ = MS::UNDEFINED_COLUMN;
}

bool SingleDishMS2::close()
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
void SingleDishMS2::set_selection(Record const &selection, bool const verbose)
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

String SingleDishMS2::get_field_as_casa_string(Record const &in_data,
					      string const &field_name)
{
  Int ifield;
  ifield = in_data.fieldNumber(String(field_name));
  if (ifield>-1) return in_data.asString(ifield);
  return "";
}


bool SingleDishMS2::prepare_for_process(string const& in_column_name,
					string const& out_ms_name)
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
  // Configure record
  Record configure_param(selection_);
  parse_selection(configure_param);
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
  if (sdh_) delete sdh_;
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
  // Set up the Data Handler
  sdh_->setup();
  return true;
}

void SingleDishMS2::finalize_process()
{
  initialize();
  if(sdh_){
    sdh_->close();
    delete sdh_;
    sdh_ = 0;
  }
}

void SingleDishMS2::parse_selection(Record &selection)
{
  int exists = -1;
  // Select only auto-correlation
  exists = selection.fieldNumber ("baseline");
  if (exists >= 0)
    {
      //selection.define("antenna", )
    }
  // Select only SPW ID
  exists = selection.fieldNumber ("spw");
  if (exists >= 0)
    {
      //selection.define("antenna", )
    }
}

void SingleDishMS2::get_data_cube_float(vi::VisBuffer2 const &vb,
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

void SingleDishMS2::convertArrayC2F(Array<Float> &to,
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

void SingleDishMS2::get_spectrum_from_cube(Cube<Float> &data_cube,
					  size_t const row,
					  size_t const plane,
					  size_t const num_data,
					  SakuraAlignedArray<float> &out_data)
{
  float *ptr = out_data.data;
  for (size_t i=0; i < num_data; ++i) 
    ptr[i] = static_cast<float>(data_cube(plane, i, row));
}

void SingleDishMS2::set_spectrum_to_cube(Cube<Float> &data_cube,
					  size_t const row,
					  size_t const plane,
					  size_t const num_data,
					  float *in_data)
{
  for (size_t i=0; i < num_data; ++i) 
    data_cube(plane, i, row) = static_cast<Float>(in_data[i]);
}

void SingleDishMS2::get_flag_cube(vi::VisBuffer2 const &vb,
				 Cube<Bool> &flag_cube)
{
  flag_cube = vb.flagCube();
}

void SingleDishMS2::get_flag_from_cube(Cube<Bool> &flag_cube,
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
// void SingleDishMS2::subtract_baseline(Vector<Bool> const &in_mask,
//                       int const order, 
//                       float const clip_threshold_sigma, 
// 				      int const num_fitting_max){}

void SingleDishMS2::subtract_baseline2(string const& in_column_name,
				      string const& out_ms_name,
				      string const &sp,
				      int const order,
				      float const clip_threshold_sigma, 
				      int const num_fitting_max)
{
  LogIO os(_ORIGIN);
  os << "Fitting and subtracting polynomial baseline order = " << order << LogIO::POST;
//   // in_ms = out_ms
//   //  in_column = [FLOAT_DATA|DATA|CORRECTED_DATA], out_column=new MS
//   // no iteration is necessary for the processing.
//   // procedure
//   // 1. iterate over MS
//   // 2. pick single spectrum from in_column (this is not actually necessary for simple scaling but for exibision purpose)
//   // 3. fit a polynomial to each spectrum and subtract it
//   // 4. put single spectrum (or a block of spectra) to out_column

//   // initializing Sakura -- temporarily placed here, though this should be done in start-up of casapy eventually
//   SakuraUtils::InitializeSakura();

//   double tstart = gettimeofday_sec();

//   prepare_for_process();
//   Block<Int> columns(1);
//   columns[0] = MS::DATA_DESC_ID;
//   vi::SortColumns sc(columns,False);
//   vi::VisibilityIterator2 vi(*mssel_,sc,True);
//   vi::VisBuffer2 *vb = vi.getVisBuffer();
//   LIBSAKURA_SYMBOL(Status) status;
//   LIBSAKURA_SYMBOL(BaselineStatus) bl_status;

//   for (vi.originChunks(); vi.moreChunks(); vi.nextChunk()) {
//     for (vi.origin(); vi.more(); vi.next()) {
//       size_t const num_chan = static_cast<size_t>(vb->nChannels());
//       size_t const num_pol = static_cast<size_t>(vb->nCorrelations());
//       size_t const num_row = static_cast<size_t>(vb->nRows());
//       Cube<Float> data_chunk(num_pol,num_chan,num_row);
//       Matrix<Float> data_row(num_pol,num_chan);
//       SakuraAlignedArray<float> spec(num_chan);
//       Cube<Bool> flag_chunk(num_pol,num_chan,num_row);
//       Matrix<Bool> flag_row(num_pol,num_chan);
//       SakuraAlignedArray<bool> mask(num_chan);
//       // set the given channel mask into aligned mask
//       /*
//       for (size_t ichan=0; ichan < num_chan; ++ichan) {
// 	//mask.data[ichan] = true;
// 	mask.data[ichan] = in_mask[ichan];
//       }
//       */
//       // create baseline context
//       LIBSAKURA_SYMBOL(BaselineContext) *bl_context;
//       status = 
// 	LIBSAKURA_SYMBOL(CreateBaselineContext)(LIBSAKURA_SYMBOL(BaselineType_kPolynomial), 
// 						static_cast<uint16_t>(order), 
// 						num_chan, 
// 						&bl_context);
//       if (status != LIBSAKURA_SYMBOL(Status_kOK)) {
// 	std::cout << "   -- error occured in CreateBaselineContext()." << std::flush;
//       }
//       // get a data cube (npol*nchan*nrow) from VisBuffer
//       get_data_cube_float(*vb, data_chunk);
//       // get a flag cube (npol*nchan*nrow) from VisBuffer
//       get_flag_cube(*vb, flag_chunk);
//       // loop over MS rows
//       for (size_t irow=0; irow < num_row; ++irow) {
// 	// loop over polarization
// 	for (size_t ipol=0; ipol < num_pol; ++ipol) {
// 	  // get a spectrum from data cube
// 	  get_spectrum_from_cube(data_chunk, irow, ipol, num_chan, spec);
// 	  // get a channel mask from data cube
// 	  // (note that mask used here is actually a flag)
// 	  get_flag_from_cube(flag_chunk, irow, ipol, num_chan, mask);
// 	  // convert flag to mask by taking logical NOT of flag
// 	  // and then operate logical AND with in_mask
// 	  for (size_t ichan=0; ichan < num_chan; ++ichan) {
// 	    mask.data[ichan] = in_mask[ichan] && (!(mask.data[ichan]));
// 	  }
// 	  // actual execution of single spectrum
// 	  status = 
// 	    LIBSAKURA_SYMBOL(SubtractBaselineFloat)(num_chan, spec.data, mask.data, bl_context, 
// 					       static_cast<uint16_t>(order), clip_threshold_sigma, num_fitting_max, 
// 					       true, mask.data, spec.data, &bl_status);
// 	  if (status != LIBSAKURA_SYMBOL(Status_kOK)) {
// 	    //raise exception?
// 	    std::cout << "   -- error occured in SubtractBaselineFloat()." << std::flush;
// 	  }
// 	  // set back a spectrum to data cube
// 	  set_spectrum_to_cube(data_chunk, irow, ipol, num_chan, spec.data);
// 	} // end of polarization loop
//       } // end of MS row loop
//       // write back data cube to VisBuffer
//       set_data_cube_float(*vb, data_chunk);
//       vb->writeChangesBack();
//       // destroy baseline context
//       status =
//         LIBSAKURA_SYMBOL(DestroyBaselineContext)(bl_context);
//       if (status != LIBSAKURA_SYMBOL(Status_kOK)) {
// 	//raise exception?
// 	std::cout << "   -- error occured in DestroyBaselineContext()." << std::flush;
//       }
//     } // end of vi loop
//   } // end of chunk loop

//   double tend = gettimeofday_sec();
//   std::cout << "Elapsed time = " << (tend - tstart) << " sec." << std::endl;
//   // clean-up Sakura -- temporarily placed here, though this should be done in casapy-side
//   SakuraUtils::CleanUpSakura();
}

void SingleDishMS2::scale(float const factor,
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
      } // end of MS row loop
      // write back data cube to Output MS
      sdh_->fillCubeToOutputMs(vb, data_chunk);
    } // end of vi loop
  } // end of chunk loop
  finalize_process();
}

void SingleDishMS2::do_scale(float const factor,
			    size_t const num_data, float *data)
{
  for (size_t i=0; i < num_data; ++i) 
    data[i] *= factor;
}

}  // End of casa namespace.
