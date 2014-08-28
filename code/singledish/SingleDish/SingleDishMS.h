#ifndef _CASA_SINGLEDISH_MS_H_
#define _CASA_SINGLEDISH_MS_H_

#include <iostream>
#include <string>

//#include <libsakura/sakura.h>

#include <casa/aipstype.h>
#include <casa/Containers/Record.h>
#include <ms/MeasurementSets/MeasurementSet.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class SingleDishMS {
public:
  // Default constructor
  SingleDishMS();
  // Construct from MS name string
  SingleDishMS(string const& ms_name);
  // Construct from MS instance
  SingleDishMS(MeasurementSet &ms);
  // Copy constructor
  SingleDishMS(SingleDishMS const &other);

  SingleDishMS &operator=(SingleDishMS const &other);
  // Destructor
  ~SingleDishMS();
  
  // Return the name of the MeasurementSet
  string name() const { return msname_; };
  
  bool close();

  // Select data to process
  void set_selection(Record const& selection);

  // Multiply a scale factor to selected spectra
  void scale(double const factor);

  // Set channel mask to process

  // Invoke baseline subtraction


private:
  // assert MS is set
  void check_MS();
  // unset MS selection
  void reset_selection();
  // retrieve a field by name from Record as casa::String.
  String get_field_as_casa_string(Record const &in_data,
				  string const &field_name);
  // the name of input MS
  string msname_;
  // input MS instance (full MS without selection)
  MeasurementSet* ms_;
  // a selected portion of input MS
  MeasurementSet* mssel_;

}; // class SingleDishMS -END


} //# NAMESPACE CASA - END
  
#endif /* _CASA_SINGLEDISH_MS_H_ */
