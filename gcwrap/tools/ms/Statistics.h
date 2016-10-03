#ifndef CASA_STATISTICS_H
#define CASA_STATISTICS_H

//
// Get statistics on table/ms columns
//

#include <casa/Containers/Record.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Array.h>
#include <casa/aipstype.h>

namespace casac {

template <class T>
class Statistics {
   public :

   /*
     Converts data to casacore::Float, computes statistics
     
     v:           data
     flags:       flagged data is not used, shape must be
                  compatible with v's shape
     column:      column name
     supported:   set to true if column type is supported

     return: output dictionary

   */
   static casacore::Record
     get_stats(const casacore::Vector<T> v,
               const casacore::Vector<casacore::Bool> flags,
               const std::string &column,
               bool &supported);

   /*
     As above, but all values are considered unflagged
    */
   static casacore::Record
     get_stats(const casacore::Vector<T> v,
               const std::string &column,
               bool &supported);

   /*
     As get_stats().
     
     The conversion from complex to casacore::Float depends on the parameter
     complex_value. Eg complex_value="imag" picks out the imaginary part.
   */
   static casacore::Record
     get_stats_complex(const casacore::Vector<casacore::Complex> v,
                       const casacore::Vector<casacore::Bool> flags,
                       const std::string &column,
                       bool &supported,
                       const std::string complex_value);

   /*
     As above, but all values are considered unflagged
    */
   static casacore::Record
     get_stats_complex(const casacore::Vector<casacore::Complex> v,
                       const std::string &column,
                       bool &supported,
                       const std::string complex_value);  

   /*
     Converts data to casacore::Float, computes statistics
     
     v:           data
     flags:       indicating which data is flagged
     column:      column whose values are 1d arrays of length N
     supported:   set to true if column type is supported

     return: output dictionary, which will contain
             N sets of statistical information, one for each index in the array
   */

   static casacore::Record
     get_stats_array(const casacore::Matrix<T> v,
                     const casacore::Vector<casacore::Bool> flags,
                     const std::string &column,
                     bool &supported);


     /* 
        Compute statistics on table column
        table:         table
        column:        column name
        complex_value: see get_stats_complex()
        itsLog:        where to send log messages
     */
     static casacore::Record
     get_statistics(const casacore::Table &table,
                    const std::string &column,
                    const std::string &complex_value,
                    casacore::LogIO *itsLog);



};

}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <tools/ms/Statistics.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC

#endif
