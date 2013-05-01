#if ! defined (MeasurementSet2_H_121204_1015)
#define MeasurementSet2_H_121204_1015

#include <ms/MeasurementSets.h>

namespace casa {

class String;
class MSSelection;

class MeasurementSet2 : public MeasurementSet {

public:

    ~MeasurementSet2 ();

    MeasurementSet2 (const MeasurementSet2 & other);
    MeasurementSet2 & operator= (const MeasurementSet2 & other);

    const MSSelection * getMSSelection () const;
    Bool wasCreatedWithSelection () const;

    static MeasurementSet2 create (const String & filename, const MSSelection * msSelection);

protected:

    MeasurementSet2 (const Table & table, MSSelection * msSelection);

private:

    MSSelection * msSelection_p;     // [own]
    Bool wasCreatedWithSelection_p;

};

} // end namespace casa

#endif // ! defined (MeasurementSet2_H_121204_1015)
