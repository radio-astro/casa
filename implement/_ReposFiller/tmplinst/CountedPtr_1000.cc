// CountedPtr_1000.cc -- Sun Oct 15 21:42:44 BST 2006 -- wyoung
#include <casa/Utilities/CountedPtr.cc>
#include <synthesis/MeasurementComponents/PBMathInterface.h>
namespace casa { //# NAMESPACE - BEGIN
template class CountedConstPtr<PBMathInterface>;
template class CountedPtr<PBMathInterface>;
template class PtrRep<PBMathInterface>;
template class SimpleCountedConstPtr<PBMathInterface>;
template class SimpleCountedPtr<PBMathInterface>;
} //# NAMESPACE - END
