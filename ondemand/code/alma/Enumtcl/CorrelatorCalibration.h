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
 struct enum_set_traits<CorrelatorCalibrationMod::CorrelatorCalibration> : public enum_set_traiter<CorrelatorCalibrationMod::CorrelatorCalibration,3,CorrelatorCalibrationMod::REAL_OBSERVATION> {};

template<>
class enum_map_traits<CorrelatorCalibrationMod::CorrelatorCalibration,void> : public enum_map_traiter<CorrelatorCalibrationMod::CorrelatorCalibration,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<CorrelatorCalibrationMod::CorrelatorCalibration,EnumPar<void> >
     (CorrelatorCalibrationMod::NONE,ep((int)CorrelatorCalibrationMod::NONE,"NONE","un-documented")));
    m_.insert(pair<CorrelatorCalibrationMod::CorrelatorCalibration,EnumPar<void> >
     (CorrelatorCalibrationMod::CORRELATOR_CALIBRATION,ep((int)CorrelatorCalibrationMod::CORRELATOR_CALIBRATION,"CORRELATOR_CALIBRATION","un-documented")));
    m_.insert(pair<CorrelatorCalibrationMod::CorrelatorCalibration,EnumPar<void> >
     (CorrelatorCalibrationMod::REAL_OBSERVATION,ep((int)CorrelatorCalibrationMod::REAL_OBSERVATION,"REAL_OBSERVATION","un-documented")));
    return true;
  }
  static map<CorrelatorCalibrationMod::CorrelatorCalibration,EnumPar<void> > m_;
};
#define _CORRELATORCALIBRATION_HH
#endif
