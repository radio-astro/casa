#ifndef _CASA_SINGLEDISH_MS2_H_
#define _CASA_SINGLEDISH_MS2_H_

#include <iostream>
#include <string>

#include <libsakura/sakura.h>

#include <casa/aipstype.h>
#include <casa/Containers/Record.h>
#include <casa_sakura/SakuraAlignedArray.h>
#include <ms/MeasurementSets/MeasurementSet.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <singledish/SingleDish/SDMSManager.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class SingleDishMS2 {
public:
  // Default constructor
  SingleDishMS2();
  // Construct from MS name string
  SingleDishMS2(string const& ms_name);

  // Destructor
  ~SingleDishMS2();
  
  // Return the name of the MeasurementSet
  string name() const { return msname_; };
  
  bool close();

  // Select data to process (verbose=T will print summary to logger)
  void set_selection(Record const& selection, bool const verbose=true);

  // Multiply a scale factor to selected spectra
  void scale(float const factor, string const& in_column_name,
	     string const& out_ms_name);

  // Set channel mask to process

  // Invoke baseline subtraction
  // (polynomial, write results in CORRECTED_DATA column)
  void subtract_baseline(Vector<Bool> const &in_mask,
                      int const order,
                      float const clip_threshold_sigma,
                      int const num_fitting_max);
  void subtract_baseline2(string const& in_column_name,
			  string const& out_ms_name,
			  string const &spwch,
			  int const order, 
			  float const clip_threshold_sigma=3.0, 
			  int const num_fitting_max=1);
  void subtract_baseline_new(string const& spwch, 
			 int const order, 
			 float const clip_threshold_sigma=3.0, 
			 int const num_fitting_max=1);

private:
  /////////////////////////
  /// Utility functions ///
  /////////////////////////
  // initialize member variables
  void initialize();
  // format selection parameters for single dish processing
  void parse_selection(Record &selection);

  // retrieve a field by name from Record as casa::String.
  String get_field_as_casa_string(Record const &in_data,
				  string const &field_name);
  // unset MS selection
  void reset_selection();

  bool prepare_for_process(string const& in_column_name,
			   string const& out_ms_name);
  void finalize_process();

  // check column 'in' is in input MS and set to 'out' if it exists.
  // if not, out is set to MS::UNDEFINED_COLUMN
  bool set_column(MSMainEnums::PredefinedColumns const &in,
		  MSMainEnums::PredefinedColumns &out);
  // Convert a Complex Array to Float Array
  void convertArrayC2F(Array<Float> &from, Array<Complex> const &to);
  // Split a string with given delimiter
  std::vector<string> split_string(string const &s, char delim);
  // Parse a string output by sdutil.get_spwchs().
  void parse_spwch(string const &spwch, 
		   Vector<Int> &spw, 
		   Vector<size_t> &nchan, 
		   Vector<Vector<Bool> > &mask);
  // Create a set of baseline contexts
  void create_baseline_contexts(LIBSAKURA_SYMBOL(BaselineType) const baseline_type, 
			        uint16_t order, 
			        Vector<size_t> const &nchan, 
			        Vector<size_t> ctx_indices, 
			        Vector<LIBSAKURA_SYMBOL(BaselineContext) *> &bl_contexts);
  // Destroy a set of baseline contexts
  void destroy_baseline_contexts(Vector<LIBSAKURA_SYMBOL(BaselineContext) *> &bl_contexts);

  /////////////////////////////
  /// MS handling functions ///
  /////////////////////////////
  // retrieve a spectrum at the row and plane (polarization) from data cube
  void get_spectrum_from_cube(Cube<Float> &data_cube,
			      size_t const row,
			      size_t const plane,
			      size_t const num_data,
			      SakuraAlignedArray<float> &out_data);
  // set a spectrum at the row and plane (polarization) from data cube
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
  // retrieve a flag at the row and plane (polarization) from data cube
  void get_flag_from_cube(Cube<Bool> &flag_cube,
			  size_t const row,
			  size_t const plane,
			  size_t const num_flag,
			  SakuraAlignedArray<bool> &out_flag);
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

}; // class SingleDishMS2 -END


} //# NAMESPACE CASA - END
  
#endif /* _CASA_SINGLEDISH_MS2_H_ */
