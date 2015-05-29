#include <iosfwd>

namespace casa {

class String;

namespace cdo {

class Subtable;

class Subtables {

public:


    Subtables ();
    ~Subtables ();

    void add (Subtable * subtable);
    const Subtable * get (const String & name) const;
    Subtable * get (Subtable::ST subtableType);
    bool isPresent (Subtable::ST subtableType) const;
    void set (Subtable::ST subtableType, Subtable * subtable);
    uInt size () const;


protected:



private:

    class Impl;

    Impl * impl_p;
};


} // end namespace cdo

} // end namespace casa
