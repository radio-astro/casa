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
 struct enum_set_traits<ScanIntentMod::ScanIntent> : public enum_set_traiter<ScanIntentMod::ScanIntent,21,ScanIntentMod::UNSPECIFIED> {};

template<>
class enum_map_traits<ScanIntentMod::ScanIntent,void> : public enum_map_traiter<ScanIntentMod::ScanIntent,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::CALIBRATE_AMPLI,ep((int)ScanIntentMod::CALIBRATE_AMPLI,"CALIBRATE_AMPLI","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::CALIBRATE_ATMOSPHERE,ep((int)ScanIntentMod::CALIBRATE_ATMOSPHERE,"CALIBRATE_ATMOSPHERE","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::CALIBRATE_BANDPASS,ep((int)ScanIntentMod::CALIBRATE_BANDPASS,"CALIBRATE_BANDPASS","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::CALIBRATE_DELAY,ep((int)ScanIntentMod::CALIBRATE_DELAY,"CALIBRATE_DELAY","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::CALIBRATE_FLUX,ep((int)ScanIntentMod::CALIBRATE_FLUX,"CALIBRATE_FLUX","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::CALIBRATE_FOCUS,ep((int)ScanIntentMod::CALIBRATE_FOCUS,"CALIBRATE_FOCUS","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::CALIBRATE_FOCUS_X,ep((int)ScanIntentMod::CALIBRATE_FOCUS_X,"CALIBRATE_FOCUS_X","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::CALIBRATE_FOCUS_Y,ep((int)ScanIntentMod::CALIBRATE_FOCUS_Y,"CALIBRATE_FOCUS_Y","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::CALIBRATE_PHASE,ep((int)ScanIntentMod::CALIBRATE_PHASE,"CALIBRATE_PHASE","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::CALIBRATE_POINTING,ep((int)ScanIntentMod::CALIBRATE_POINTING,"CALIBRATE_POINTING","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::CALIBRATE_POLARIZATION,ep((int)ScanIntentMod::CALIBRATE_POLARIZATION,"CALIBRATE_POLARIZATION","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::CALIBRATE_SIDEBAND_RATIO,ep((int)ScanIntentMod::CALIBRATE_SIDEBAND_RATIO,"CALIBRATE_SIDEBAND_RATIO","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::CALIBRATE_WVR,ep((int)ScanIntentMod::CALIBRATE_WVR,"CALIBRATE_WVR","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::DO_SKYDIP,ep((int)ScanIntentMod::DO_SKYDIP,"DO_SKYDIP","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::MAP_ANTENNA_SURFACE,ep((int)ScanIntentMod::MAP_ANTENNA_SURFACE,"MAP_ANTENNA_SURFACE","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::MAP_PRIMARY_BEAM,ep((int)ScanIntentMod::MAP_PRIMARY_BEAM,"MAP_PRIMARY_BEAM","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::OBSERVE_TARGET,ep((int)ScanIntentMod::OBSERVE_TARGET,"OBSERVE_TARGET","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::CALIBRATE_POL_LEAKAGE,ep((int)ScanIntentMod::CALIBRATE_POL_LEAKAGE,"CALIBRATE_POL_LEAKAGE","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::CALIBRATE_POL_ANGLE,ep((int)ScanIntentMod::CALIBRATE_POL_ANGLE,"CALIBRATE_POL_ANGLE","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::TEST,ep((int)ScanIntentMod::TEST,"TEST","un-documented")));
    m_.insert(pair<ScanIntentMod::ScanIntent,EnumPar<void> >
     (ScanIntentMod::UNSPECIFIED,ep((int)ScanIntentMod::UNSPECIFIED,"UNSPECIFIED","un-documented")));
    return true;
  }
  static map<ScanIntentMod::ScanIntent,EnumPar<void> > m_;
};
#define _SCANINTENT_HH
#endif
