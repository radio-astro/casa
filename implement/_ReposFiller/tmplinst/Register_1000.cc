// Register_1000.cc -- Sun Oct 15 21:42:44 BST 2006 -- wyoung
#include <casa/Utilities/Register.cc>
#include <casa/Containers/OrderedMap.h>
#include <synthesis/Parallel/Algorithm.h>
namespace casa { //# NAMESPACE - BEGIN
template uInt Register(OrderedMapNotice<Int, Algorithm *> const *);
} //# NAMESPACE - END
