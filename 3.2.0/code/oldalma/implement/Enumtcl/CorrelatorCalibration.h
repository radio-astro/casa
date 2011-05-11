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


#if     !defined(_CORRELATORCALIBRATION_H)

#include <CCorrelatorCalibration.h>
#define _CORRELATORCALIBRATION_H
#endif 

#if     !defined(_CORRELATORCALIBRATION_HH)

#include "Enum.hpp"

using namespace CorrelatorCalibrationMod;

template<>
 struct enum_set_traits<CorrelatorCalibration> : public enum_set_traiter<CorrelatorCalibration,10,CorrelatorCalibrationMod::BL_CALC_TFB_SCALING_FACTORS> {};

template<>
class enum_map_traits<CorrelatorCalibration,void> : public enum_map_traiter<CorrelatorCalibration,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<CorrelatorCalibration,EnumPar<void> >
     (CorrelatorCalibrationMod::NONE,ep((int)CorrelatorCalibrationMod::NONE,"NONE","No internal correlator calibration")));
    m_.insert(pair<CorrelatorCalibration,EnumPar<void> >
     (CorrelatorCalibrationMod::ACA_OBSERVE_CALIBATOR,ep((int)CorrelatorCalibrationMod::ACA_OBSERVE_CALIBATOR,"ACA_OBSERVE_CALIBATOR","Specific ACA Correlator calibration")));
    m_.insert(pair<CorrelatorCalibration,EnumPar<void> >
     (CorrelatorCalibrationMod::ACA_CALIBRATE_CALIBRATOR,ep((int)CorrelatorCalibrationMod::ACA_CALIBRATE_CALIBRATOR,"ACA_CALIBRATE_CALIBRATOR","Specific ACA Correlator calibration")));
    m_.insert(pair<CorrelatorCalibration,EnumPar<void> >
     (CorrelatorCalibrationMod::ACA_HFSC_REFRESH_CALIBRATOR,ep((int)CorrelatorCalibrationMod::ACA_HFSC_REFRESH_CALIBRATOR,"ACA_HFSC_REFRESH_CALIBRATOR","Specific ACA Correlator calibration")));
    m_.insert(pair<CorrelatorCalibration,EnumPar<void> >
     (CorrelatorCalibrationMod::ACA_OBSERVE_TARGET,ep((int)CorrelatorCalibrationMod::ACA_OBSERVE_TARGET,"ACA_OBSERVE_TARGET","Specific ACA Correlator calibration")));
    m_.insert(pair<CorrelatorCalibration,EnumPar<void> >
     (CorrelatorCalibrationMod::ACA_CALIBATE_TARGET,ep((int)CorrelatorCalibrationMod::ACA_CALIBATE_TARGET,"ACA_CALIBATE_TARGET","Specific ACA Correlator calibration")));
    m_.insert(pair<CorrelatorCalibration,EnumPar<void> >
     (CorrelatorCalibrationMod::ACA_HFSC_REFRESH_TARGET,ep((int)CorrelatorCalibrationMod::ACA_HFSC_REFRESH_TARGET,"ACA_HFSC_REFRESH_TARGET","Specific ACA Correlator calibration")));
    m_.insert(pair<CorrelatorCalibration,EnumPar<void> >
     (CorrelatorCalibrationMod::ACA_CORRELATOR_CALIBRATION,ep((int)CorrelatorCalibrationMod::ACA_CORRELATOR_CALIBRATION,"ACA_CORRELATOR_CALIBRATION","Specific ACA Correlator calibration")));
    m_.insert(pair<CorrelatorCalibration,EnumPar<void> >
     (CorrelatorCalibrationMod::ACA_REAL_OBSERVATION,ep((int)CorrelatorCalibrationMod::ACA_REAL_OBSERVATION,"ACA_REAL_OBSERVATION","Specific ACA Correlator calibration")));
    m_.insert(pair<CorrelatorCalibration,EnumPar<void> >
     (CorrelatorCalibrationMod::BL_CALC_TFB_SCALING_FACTORS,ep((int)CorrelatorCalibrationMod::BL_CALC_TFB_SCALING_FACTORS,"BL_CALC_TFB_SCALING_FACTORS","Specific ACA Correlator calibration")));
    return true;
  }
  static map<CorrelatorCalibration,EnumPar<void> > m_;
};
#define _CORRELATORCALIBRATION_HH
#endif
