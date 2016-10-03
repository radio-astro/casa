#include <iosfwd>

namespace casacore{

class String;
}

namespace casa {


namespace cdo {

class Subtable;

class Subtables {

public:


    Subtables ();
    ~Subtables ();

    void add (Subtable * subtable);
    const Subtable * get (const casacore::String & name) const;
    Subtable * get (Subtable::ST subtableType);
    bool isPresent (Subtable::ST subtableType) const;
    void set (Subtable::ST subtableType, Subtable * subtable);
    casacore::uInt size () const;


protected:



private:

    class Impl;

    Impl * impl_p;
};


} // end namespace cdo

} // end namespace casa

