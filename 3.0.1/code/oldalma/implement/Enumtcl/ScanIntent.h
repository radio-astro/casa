/*
 *
 * /////////////////////////////////////////////////////////////////
 * // WARNING!  DO NOT MODIFY THIS FILE!                          //
 * //  ---------------------------------------------------------  //
 * // | This is generated code using a C++ template function!   | //
 * // ! Do not modify this file.                                | //
 * // | Any changes will be lost when the file is re-generated. | //
 * //  ---------------------------------------------------------  //
 * /////////////////////////////////////////////////////////////////
 *
 */


#if     !defined(_SCANINTENT_H)

#include <CScanIntent.h>
#define _SCANINTENT_H
#endif 

#if     !defined(_SCANINTENT_HH)

#include "Enum.hpp"

using namespace ScanIntentMod;

template<>
 struct enum_set_traits<ScanIntent> : public enum_set_traiter<ScanIntent,26,ScanIntentMod::UNSPECIFIED> {};

template<>
class enum_map_traits<ScanIntent,void> : public enum_map_traiter<ScanIntent,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::AMPLI_CAL,ep((int)ScanIntentMod::AMPLI_CAL,"AMPLI_CAL","Amplitude calibration scan")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::AMPLI_CURVE,ep((int)ScanIntentMod::AMPLI_CURVE,"AMPLI_CURVE","Amplitude calibration scan, calibration curve to be derived")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::ANTENNA_POSITIONS,ep((int)ScanIntentMod::ANTENNA_POSITIONS,"ANTENNA_POSITIONS","Antenna positions measurement")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::ATMOSPHERE,ep((int)ScanIntentMod::ATMOSPHERE,"ATMOSPHERE","Atmosphere calibration scan")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::BANDPASS,ep((int)ScanIntentMod::BANDPASS,"BANDPASS","Bandpass calibration scan")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::DELAY,ep((int)ScanIntentMod::DELAY,"DELAY","Delay calibration scan")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::FOCUS,ep((int)ScanIntentMod::FOCUS,"FOCUS","Focus calibration scan. Z coordinate to be derived")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::HOLOGRAPHY,ep((int)ScanIntentMod::HOLOGRAPHY,"HOLOGRAPHY","Holography calibration scan")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::PHASE_CAL,ep((int)ScanIntentMod::PHASE_CAL,"PHASE_CAL","Phase calibration scan")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::PHASE_CURVE,ep((int)ScanIntentMod::PHASE_CURVE,"PHASE_CURVE","Phase calibration scan; phase calibration curve to be derived")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::POINTING,ep((int)ScanIntentMod::POINTING,"POINTING","Pointing calibration scan")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::POINTING_MODEL,ep((int)ScanIntentMod::POINTING_MODEL,"POINTING_MODEL","Pointing calibration scan; pointing model to be derived")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::POLARIZATION,ep((int)ScanIntentMod::POLARIZATION,"POLARIZATION","Polarization calibration scan")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::SKYDIP,ep((int)ScanIntentMod::SKYDIP,"SKYDIP","Skydip calibration scan")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::TARGET,ep((int)ScanIntentMod::TARGET,"TARGET","Target source scan")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::CALIBRATION,ep((int)ScanIntentMod::CALIBRATION,"CALIBRATION","Generic calibration scan")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::LAST,ep((int)ScanIntentMod::LAST,"LAST","Last calibration scan of a group")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::OFFLINE_PROCESSING,ep((int)ScanIntentMod::OFFLINE_PROCESSING,"OFFLINE_PROCESSING","Calibration to be processed off-line")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::FOCUS_X,ep((int)ScanIntentMod::FOCUS_X,"FOCUS_X","Focus calibration scan; X focus coordinate to be derived")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::FOCUS_Y,ep((int)ScanIntentMod::FOCUS_Y,"FOCUS_Y","Focus calibration scan; Y focus coordinate to be derived")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::SIDEBAND_RATIO,ep((int)ScanIntentMod::SIDEBAND_RATIO,"SIDEBAND_RATIO","")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::OPTICAL_POINTING,ep((int)ScanIntentMod::OPTICAL_POINTING,"OPTICAL_POINTING","Data from the optical telescope are used to measure poitning offsets.")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::WVR_CAL,ep((int)ScanIntentMod::WVR_CAL,"WVR_CAL","Data from the water vapor radiometers (and correlation data) are used to derive their calibration parameters.")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::ASTRO_HOLOGRAPHY,ep((int)ScanIntentMod::ASTRO_HOLOGRAPHY,"ASTRO_HOLOGRAPHY","Correlation data on a celestial calibration source are used to derive antenna surface parameters.")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::BEAM_MAP,ep((int)ScanIntentMod::BEAM_MAP,"BEAM_MAP","Data on a celestial calibration source are used to derive a map of the primary beam.")));
    m_.insert(pair<ScanIntent,EnumPar<void> >
     (ScanIntentMod::UNSPECIFIED,ep((int)ScanIntentMod::UNSPECIFIED,"UNSPECIFIED","Unspecified scan intent")));
    return true;
  }
  static map<ScanIntent,EnumPar<void> > m_;
};
#define _SCANINTENT_HH
#endif
