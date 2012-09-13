#if ! defined (VisibilityIterator_Test_H_20120912_1001
#define VisibilityIterator_Test_H_20120912_1001

namespace casa {
namespace vi {
namespace test {

class MsFactory {

public:

    MsFactory ();

    MeasurementSet * createMs ();

protected:

    void createSubtables ();
    void generateAntennaTable ();
    void generateDataDescriptionTable ();
    void generateFeedTable ();
    void generateFieldTable ();
    void generateObservationTable ();
    void generatePointingTable ();
    void generatePolarizationTable ();
    void generateProcessorTable ();
    void generateSpectralWindowTable ();
    void generateStateTable ();

private:



};

} // end namespace casa
} // end namespace vi
} // end namespace test

#endif // ! defined (VisibilityIterator_Test_H_20120912_1001
