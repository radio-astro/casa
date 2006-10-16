// Map_1000.cc -- Sun Oct 15 21:42:44 BST 2006 -- wyoung
#include <casa/Containers/Map.cc>
#include <synthesis/Parallel/Algorithm.h>
namespace casa { //# NAMESPACE - BEGIN
template class ConstMapIter<Int, Algorithm *>;
template class MapIter<Int, Algorithm *>;
template class MapIterRep<Int, Algorithm *>;
template class MapRep<Int, Algorithm *>;
template class Map<Int, Algorithm *>;
} //# NAMESPACE - END
