#include <synthesis/MSVis/ViColumns2.h>
#include <ms/MeasurementSets.h>

namespace casa {

namespace vi {

ViColumns2::ViColumns2 (Bool writable)
: antenna1_p (writable),
  antenna2_p (writable),
  array_p (writable),
  corrVis_p (writable),
  dataDescription_p (writable),
  exposure_p (writable),
  feed1_p (writable),
  feed2_p (writable),
  field_p (writable),
  flagCategory_p (writable),
  flagRow_p (writable),
  flag_p (writable),
  floatVis_p (writable),
  modelVis_p (writable),
  observation_p (writable),
  processor_p (writable),
  scan_p (writable),
  sigma_p (writable),
  state_p (writable),
  timeCentroid_p (writable),
  timeInterval_p (writable),
  time_p (writable),
  uvw_p (writable),
  vis_p (writable),
  weightSpectrum_p (writable),
  weight_p (writable),
  floatDataFound_p (False)
{}


void
ViColumns2::attachColumns (const Table & t, bool attachSpecialColumns)
{
    const ColumnDescSet & cds = t.tableDesc ().columnDescSet ();

    antenna1_p.attach (t, MS::columnName (MS::ANTENNA1));
    antenna2_p.attach (t, MS::columnName (MS::ANTENNA2));

    if (cds.isDefined ("CORRECTED_DATA")) {
        corrVis_p.attach (t, "CORRECTED_DATA");
    }

    exposure_p.attach (t, MS::columnName (MS::EXPOSURE));
    feed1_p.attach (t, MS::columnName (MS::FEED1));
    feed2_p.attach (t, MS::columnName (MS::FEED2));
    flag_p.attach (t, MS::columnName (MS::FLAG));
    flagCategory_p.attach (t, MS::columnName (MS::FLAG_CATEGORY));
    flagRow_p.attach (t, MS::columnName (MS::FLAG_ROW));

    if (cds.isDefined (MS::columnName (MS::FLOAT_DATA))) {
        floatVis_p.attach (t, MS::columnName (MS::FLOAT_DATA));
        floatDataFound_p = True;
    } else {
        floatDataFound_p = False;
    }

    if (cds.isDefined ("MODEL_DATA")) {
        modelVis_p.attach (t, "MODEL_DATA");
    }

    observation_p.attach (t, MS::columnName (MS::OBSERVATION_ID));
    processor_p.attach (t, MS::columnName (MS::PROCESSOR_ID));
    scan_p.attach (t, MS::columnName (MS::SCAN_NUMBER));
    sigma_p.attach (t, MS::columnName (MS::SIGMA));
    state_p.attach (t, MS::columnName (MS::STATE_ID));
    time_p.attach (t, MS::columnName (MS::TIME));
    timeCentroid_p.attach (t, MS::columnName (MS::TIME_CENTROID));
    timeInterval_p.attach (t, MS::columnName (MS::INTERVAL));
    uvw_p.attach (t, MS::columnName (MS::UVW));

    if (cds.isDefined (MS::columnName (MS::DATA))) {
        vis_p.attach (t, MS::columnName (MS::DATA));
    }

    weight_p.attach (t, MS::columnName (MS::WEIGHT));

    if (cds.isDefined ("WEIGHT_SPECTRUM")) {
        weightSpectrum_p.attach (t, "WEIGHT_SPECTRUM");
    }

    if (attachSpecialColumns){

        array_p.attach (t, MS::columnName (MS::ARRAY_ID));

        dataDescription_p.attach (t, MS::columnName (MS::DATA_DESC_ID));

        field_p.attach (t, MS::columnName (MS::FIELD_ID));
    }
}

Bool
ViColumns2::isFloatDataPresent () const
{
    return floatDataFound_p;
}

} // end namespace vi

} // end namespace casa
