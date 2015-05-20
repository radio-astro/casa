#ifndef _CASA_SINGLEDISH_MS_H_
#define _CASA_SINGLEDISH_MS_H_

#include <iostream>
#include <string>

#include <casa/aipstype.h>
#include <casa/Containers/Record.h>
#include <casa_sakura/SakuraAlignedArray.h>
#include <libsakura/sakura.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <singledish/SingleDish/SDMSManager.h>

#include "BaselineTable.h"

namespace casa { //# NAMESPACE CASA - BEGIN

class SingleDishMS {
public:
  // Default constructor
  SingleDishMS();
  // Construct from MS name string
  SingleDishMS(string const& ms_name);

  // Destructor
  ~SingleDishMS();
  
  /* 
   * Return the name of the MeasurementSet
   */
  string name() const { return msname_; };
  
  bool close();

   /*
   * Formats selection parameters for single dish processing.
   * @param [in] selection A Record consists of selection key and values
   * @param [in] verbose If true, print summary of selection logger
   */ 
  void setSelection(Record const& selection, bool const verbose=true);

  // Multiply a scale factor to selected spectra
  void scale(float const factor, string const& in_column_name,
	     string const& out_ms_name);

  // Set channel mask to process

  // Invoke baseline subtraction
  // (polynomial, write results in new MS)
  void subtractBaseline(string const& in_column_name,
			string const& out_ms_name,
			string const& out_bltable_name,
			bool const& do_subtract,
			string const& in_spw, 
			string const& in_ppp, 
			string const& blfunc,
			int const order, 
			float const clip_threshold_sigma=3.0, 
			int const num_fitting_max=1);

  //Cubicspline  
  void subtractBaselineCspline(string const& in_column_name,
			       string const& out_ms_name,
			       string const& out_bltable_name,
			       bool const& do_subtract,
			       string const& in_spw, 
			       string const& in_ppp, 
			       int const npiece, 
			       float const clip_threshold_sigma=3.0, 
			       int const num_fitting_max=1);

  // variable fitting parameters stored in a text file
  void subtractBaselineVariable(string const& in_column_name,
				string const& out_ms_name,
				string const& out_bltable_name,
				bool const& do_subtract,
				string const& in_spw, 
				string const& in_ppp, 
				string const& param_file);

private:
  /////////////////////////
  /// Utility functions ///
  /////////////////////////
  /*
   *  Initializes member variables: in_column_
   */
  void initialize();
   /*
   * Formats selection parameters for single dish processing.
   * @param [in] selection A Record consists of selection key and values
   */ 
  void format_selection(Record &selection);

  // retrieve a field by name from Record as casa::String.
  String get_field_as_casa_string(Record const &in_data,
				  string const &field_name);
  // unset MS selection
  void reset_selection();

  bool prepare_for_process(string const &in_column_name,
			   string const&out_ms_name);

  bool prepare_for_process(string const &in_column_name,
			   string const&out_ms_name,
			   Block<Int> const &sortColumns,
			   bool const addDefaultSortCols=false);
  void finalize_process();

  // check column 'in' is in input MS and set to 'out' if it exists.
  // if not, out is set to MS::UNDEFINED_COLUMN
  bool set_column(MSMainEnums::PredefinedColumns const &in,
		  MSMainEnums::PredefinedColumns &out);
  // Convert a Complex Array to Float Array
  void convertArrayC2F(Array<Float> &from, Array<Complex> const &to);
  // Split a string with given delimiter
  std::vector<string> split_string(string const &s, char delim);
  // Parse msseltoindex output
  void parse_spw(string const &in_spw, 
		 Vector<Int> &spw, 
		 Matrix<Int> &chan,
		 Vector<size_t> &nchan, 
		 Vector<Vector<Bool> > &mask,
		 Vector<bool> &nchan_set);
  void get_nchan_and_mask(Vector<Int> const &rec_spw,
			  Vector<Int> const &data_spw,
			  Matrix<Int> const &rec_chan,
			  size_t const num_chan,
			  Vector<size_t> &nchan, 
			  Vector<Vector<Bool> > &mask,
			  Vector<bool> &nchan_set,
			  bool &new_nchan);
  void get_mask_from_rec(Int spwid, Matrix<Int> const &rec_chan,
			 Vector<Bool> &mask, bool initialize=true);
  void get_masklist_from_mask(size_t const num_chan, bool const *mask, Vector<uInt> &masklist);
  void get_pol_selection(string const &in_pol,
			 size_t const num_pol,
			 Vector<bool> &pol);
  // Create a set of baseline contexts
  void get_baseline_context(LIBSAKURA_SYMBOL(BaselineType) const baseline_type, 
			    uint16_t order, 
			    size_t num_chan,
			    Vector<size_t> const &nchan,
			    Vector<bool> const &nchan_set,
		            Vector<size_t> &ctx_indices, 
			    std::vector<LIBSAKURA_SYMBOL(BaselineContext) *> &bl_contexts);
  // Destroy a set of baseline contexts
  void destroy_baseline_contexts(std::vector<LIBSAKURA_SYMBOL(BaselineContext) *> &bl_contexts);
  void check_sakura_status(string const &name, LIBSAKURA_SYMBOL(Status) const status);
  template<typename T, typename U>
    void set_matrix_for_bltable(size_t const num_pol,
			        size_t const num_data_max, 
			        std::vector<std::vector<T> > const &in_data, 
			        Array<U> &out_data) {
    for (size_t ipol = 0; ipol < num_pol; ++ipol) {
      for (size_t i = 0; i < num_data_max; ++i) {
	out_data[i][ipol] = static_cast<U>(0);
      }
      size_t num_data = in_data[ipol].size();
      for (size_t i = 0; i < num_data; ++i) {
	out_data[i][ipol] = static_cast<U>(in_data[ipol][i]);
      }
    }
  }
  template<typename T, typename U>
    void set_array_for_bltable(size_t const ipol,
			       size_t const num_data,
			       T const *in_data, 
			       Array<U> &out_data) {
    for (size_t i = 0; i < num_data; ++i) {
      out_data[i][ipol] = static_cast<U>(in_data[i]);
    }
  }

  // *****---------------------------------------------------------------
  // the following function has temporarily been copied from sakura to 
  // get positions of cubic spline boundaries. it should be deleted once 
  // sakura_GetBestFitBaselineCoefficientsCubicSplineFloat is updated so 
  // that boundary information become available (2015/4/6 WK)
  // *****---------------------------------------------------------------
  void GetBoundariesOfPiecewiseData(size_t num_mask,
				    bool const *mask, 
				    size_t num_pieces, 
				    double *boundary);
  // *****---------------------------------------------------------------

  /////////////////////////////
  /// MS handling functions ///
  /////////////////////////////
  // retrieve a spectrum at the row and plane (polarization) from data cube
  void get_spectrum_from_cube(Cube<Float> &data_cube,
			      size_t const row,
			      size_t const plane,
			      size_t const num_data,
			      SakuraAlignedArray<float> &out_data);
  // set a spectrum at the row and plane (polarization) to data cube
  void set_spectrum_to_cube(Cube<Float> &data_cube,
			    size_t const row,
			    size_t const plane,
			    size_t const num_data,
			    float in_data[/*num_data*/]);
  // get data cube (npol*nchan*nvirow) in in_column_ from visbuffer
  // and convert it to float cube
  void get_data_cube_float(vi::VisBuffer2 const &vb,
			   Cube<Float> &data_cube);
  // get flag cube (npol*nchan*nvirow) from visbuffer
  void get_flag_cube(vi::VisBuffer2 const &vb,
		     Cube<Bool> &flag_cube);
  // retrieve a flag at the row and plane (polarization) from flag cube
  void get_flag_from_cube(Cube<Bool> &flag_cube,
			  size_t const row,
			  size_t const plane,
			  size_t const num_flag,
			  SakuraAlignedArray<bool> &out_flag);
  // set a flag at the row and plane (polarization) to flag cube
  void set_flag_to_cube(Cube<Bool> &flag_cube,
			size_t const row,
			size_t const plane,
			size_t const num_flag,
			bool in_flag[/*num_data*/]);

  // flag all channels in a supectrum in cube at the row and plane (polarization)
  void flag_spectrum_in_cube(Cube<Bool> &flag_cube,
			     size_t const row,
			     size_t const plane);

  // return true if all channels are flagged
  bool allchannels_flagged(size_t const num_flag, 
			   bool const* flag);
  // returns the number of channels with true in input mask
  size_t NValidMask(size_t const num_mask, bool const* mask);

  /////////////////////////////////
  /// Array execution functions ///
  /////////////////////////////////
  // multiply a scaling factor to a float array
  void do_scale(float const factor,
		size_t const num_data, float data[/*num_data*/]);

  ////////////////////////
  /// Member vairables ///
  ////////////////////////
  // the name of input MS
  string msname_;
  // columns to read and save data
  MSMainEnums::PredefinedColumns in_column_;//, out_column_;
  // Record of selection
  Record selection_;
  // SDMSManager
  SDMSManager *sdh_;

}; // class SingleDishMS -END


} //# NAMESPACE CASA - END
  
#endif /* _CASA_SINGLEDISH_MS_H_ */
