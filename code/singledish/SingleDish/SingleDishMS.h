#ifndef _CASA_SINGLEDISH_MS_H_
#define _CASA_SINGLEDISH_MS_H_

#include <iostream>
#include <string>

//#include <libsakura/sakura.h>

#include <casa/aipstype.h>
#include <ms/MeasurementSets/MeasurementSet.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class SingleDishMS {
public:
  // Default constructor
  SingleDishMS();
  // Construct from MS name string
  SingleDishMS(const string& ms_name);
  // Construct from MS instance
  SingleDishMS(MeasurementSet &ms);
  // Copy constructor
  SingleDishMS(const SingleDishMS &other);

  SingleDishMS &operator=(const SingleDishMS &other);
  // Destructor
  ~SingleDishMS();
  
  // Return the name of the MeasurementSet
  string name() const { return msname_p; };
  
  bool close();

  // Select data to process

  // Set channel mask to process

  // Invoke baseline subtraction


private:
  string msname_p;
  MeasurementSet* ms_p;
  MeasurementSet* mssel_p;

}; // class SingleDishMS -END


} //# NAMESPACE CASA - END
  
#endif /* _CASA_SINGLEDISH_MS_H_ */
