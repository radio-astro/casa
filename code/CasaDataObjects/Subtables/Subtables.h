#if ! defined (CasaDataObjects_Subtables_H_160222_1045)
#define CasaDataObjects_Subtables_H_160222_1045

#include <iosfwd>

namespace casa {

class String;

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
    const Subtable * get (SubtableType) const;
    bool isPresent (SubtableType subtableType) const;
    void set (SubtableType subtableType, Subtable * subtable);
    unsigned int size () const;

   static String typeToName (SubtableType);
   static SubtableType nameToType (String);

protected:



private:

    class Impl;

    Impl * impl_p;
};


} // end namespace cdo

} // end namespace casa

#endif // ! defined (CasaDataObjects_Subtables_H_160222_1045)
