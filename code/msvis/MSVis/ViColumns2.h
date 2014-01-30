#if ! defined (ViColumns_H_121126_1506)
#define ViColumns_H_121126_1506

#include <casa/aips.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ScalarColumn.h>


namespace casa {

namespace vi {

class ViColumns2 {

public:

    ViColumns2 ();
    ViColumns2 & operator= (const ViColumns2 & other);

    void attachColumns (const Table & t, bool attachSpecialColumns = True);


    ScalarColumn<Int>    antenna1_p;
    ScalarColumn<Int>    antenna2_p;
    ScalarColumn<Int>    array_p;
    ArrayColumn<Complex> corrVis_p;
    ScalarColumn<Int>    dataDescription_p;
    ScalarColumn<Double> exposure_p;
    ScalarColumn<Int>    feed1_p;
    ScalarColumn<Int>    feed2_p;
    ScalarColumn<Int>    field_p;
    ArrayColumn<Bool>    flagCategory_p;
    ScalarColumn<Bool>   flagRow_p;
    ArrayColumn<Bool>    flag_p;
    ArrayColumn<Float>   floatVis_p;
    ArrayColumn<Complex> modelVis_p;
    ScalarColumn<Int>    observation_p;
    ScalarColumn<Int>    processor_p;
    ScalarColumn<Int>    scan_p;
    ArrayColumn<Float>   sigma_p;
    ScalarColumn<Int>    state_p;
    ScalarColumn<Double> timeCentroid_p;
    ScalarColumn<Double> timeInterval_p;
    ScalarColumn<Double> time_p;
    ArrayColumn<Double>  uvw_p;
    ArrayColumn<Complex> vis_p;
    ArrayColumn<Float>   weightSpectrum_p;
    ArrayColumn<Float>   weightSpectrumCorrected_p;
    ArrayColumn<Float>   weight_p;

    Bool isFloatDataPresent () const;

private:

    Bool floatDataFound_p;

};

} // namespace vi

} // end namespace casa

#endif // ! defined (ViColumns_H_121126_1506)
