#if ! defined (MSVIS_VisBufferComponents_H)
#define MSVIS_VisBufferComponents_H

namespace casa {

class VisBufferComponents {

public:

    typedef enum {
        Unknown=-1,
        Ant1,
        Ant2,
        ArrayId,
        Channel,
        Cjones,
        CorrType,
        Corrected,
        CorrectedCube,
        Direction1,
        Direction2,
        Exposure,
        Feed1,
        Feed1_pa,
        Feed2,
        Feed2_pa,
        FieldId,
        Flag,
        FlagCategory,
        FlagCube,
        FlagRow,
        Freq,
        ImagingWeight,
        Model,
        ModelCube,
        NChannel,
        NCorr,
        NRow,
        ObservationId,
        Observed,
        ObservedCube,
        PhaseCenter,
        PolFrame,
        ProcessorId,
        Scan,
        Sigma,
        SigmaMat,
        SpW,
        StateId,
        Time,
        TimeCentroid,
        TimeInterval,
        Weight,
        WeightMat,
        WeightSpectrum,
        Uvw,
        UvwMat,
        N_VisBufferComponents // must be last
    } EnumType;


};

}
#endif // ! defined (MSVIS_VisBufferComponents_H)

