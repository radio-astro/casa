#include <string>
#include <vector>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>
#include <casa/Quanta/Quantum.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MFrequency.h>
#include <measures/Measures/MRadialVelocity.h>
#include <measures/Measures/MEpoch.h>
#include <casac/Quantity.h>
#include <xmlcasa/record.h>

// Conversions utilities

namespace casa {

class Record;
class ValueHolder;

Vector<String>    toVectorString(const std::vector<std::string> &);
std::vector<std::string> fromVectorString(const Vector<String> &);
 String toCasaString(const casac::variant &);

Quantity          casaQuantity(const casac::Quantity &);

// Constructs and returns a Quantity from its input.  It will throw an
// exception if given a malformed string or string vector, but otherwise will
// generate a default Quantity (0.0) for unhandled variants.
Quantity          casaQuantity(const casac::variant &);

::casac::Quantity casacQuantity(const Quantity &);

// See note in implementation about possible memory leak.
::casac::record  *fromRecord(const Record &);
// makes a Vector of Quntity out of strings or vector of such or later records
 Bool toCasaVectorQuantity(const ::casac::variant& theval, 
			   casa::Vector<casa::Quantity>& theQuants);

::casac::record* recordFromQuantity(const Quantity q);
::casac::record* recordFromQuantity(const Quantum<Vector<Double> >& q);

Record           *toRecord(const casac::record &);
ValueHolder      *toValueHolder(const casac::variant&);
::casac::variant *fromValueHolder(const ValueHolder &);
Bool             casaMDirection(const ::casac::variant& theVar, 
				            MDirection& theMeas);
Bool             ang_as_formatted_str(string& out, const casa::Quantity& qang,
                                      const std::string& format, const Int precision);
Bool             MDirection2str(const MDirection& in, std::string& out);
Bool             casaMFrequency(const ::casac::variant& theVar, 
                                MFrequency& theMeas);
Bool             casaMPosition(const ::casac::variant& theVar, 
                               MPosition& theMeas);
Bool             casaMRadialVelocity(const ::casac::variant& theVar, 
				            MRadialVelocity& theMeas);
Bool             casaMEpoch(const ::casac::variant& theVar, 
                            MEpoch& theMeas);
 //utility to split a single string to multiple if they are , or empty space 
 //seperated
 Int sepCommaEmptyToVectorStrings(Vector<String>& retStr, 
				  const std::string& str); 

}
