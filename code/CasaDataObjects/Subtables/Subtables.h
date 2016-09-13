#if ! defined (CasaDataObjects_Subtables_H_160222_1045)
#define CasaDataObjects_Subtables_H_160222_1045

#include <iosfwd>

namespace casacore{

class String;
}

namespace casa {


namespace cdo {

    class Subtable;

enum class SubtableType : int {

        Unknown,
        Antenna,
        DataDescription,
        Feed,
        Field,
        Observation,
        Pointing,
        Polarization,
        Processor,
        SpectralWindow,
        State,
        Source,
        Syscal,

        N_Types
};

class Subtables {

public:


    Subtables ();
    ~Subtables ();

    void add (Subtable * subtable);
<<<<<<< .mine
    const Subtable * get (const casacore::String & name) const;
    Subtable * get (Subtable::ST subtableType);
    bool isPresent (Subtable::ST subtableType) const;
    void set (Subtable::ST subtableType, Subtable * subtable);
    casacore::uInt size () const;
=======
    const Subtable * get (SubtableType) const;
    bool isPresent (SubtableType subtableType) const;
    void set (SubtableType subtableType, Subtable * subtable);
    unsigned int size () const;
>>>>>>> .r38222

   static String typeToName (SubtableType);
   static SubtableType nameToType (String);

protected:



private:

    class Impl;

    Impl * impl_p;
};


} // end namespace cdo

} // end namespace casa
<<<<<<< .mine

=======

#endif // ! defined (CasaDataObjects_Subtables_H_160222_1045)
>>>>>>> .r38222
