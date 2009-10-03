#ifndef CASA_STATISTICS_H
#define CASA_STATISTICS_H

//
// Get statistics on table/ms columns
//

#include <casa/Containers/Record.h>

namespace casac {

class Statistics {
   public :

     static casa::Record
     get_statistics(const casa::Table &table,
                    const std::string &column,
                    const std::string &complex_value,
                    casa::LogIO *itsLog);
   
};

}

#endif
