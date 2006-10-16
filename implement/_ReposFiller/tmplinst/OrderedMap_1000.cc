// OrderedMap_1000.cc -- Sun Oct 15 21:42:44 BST 2006 -- wyoung
#include <casa/Containers/OrderedMap.cc>
#include <synthesis/Parallel/Algorithm.h>
namespace casa { //# NAMESPACE - BEGIN
template class OrderedMapRep<Int, Algorithm *>;
template class OrderedMapIterRep<Int, Algorithm *>;
template class OrderedMap<Int, Algorithm *>;
template class OrderedMapNotice<Int, Algorithm *>;
} //# NAMESPACE - END
