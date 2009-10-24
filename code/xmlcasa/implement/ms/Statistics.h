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

     static casa::Record
     get_statistics(const casa::Table &table,
                    const std::string &column,
                    const std::string &complex_value,
                    casa::LogIO *itsLog);

   static casa::Record
     get_stats_complex(casa::Vector<casa::Complex> v,
                       const std::string &column,
                       bool &supported,
                       const std::string complex_value);

   static casa::Record
     get_stats(casa::Vector<T> v,
               const std::string &column,
               bool &supported);

   static casa::Record
     get_stats_array(casa::Matrix<T> v,
                     const std::string &column,
                     bool &supported);


};

}

#ifndef AIPS_NO_TEMPLATE_SRC
#include <xmlcasa/ms/Statistics.tcc>
#endif //# AIPS_NO_TEMPLATE_SRC

#endif
