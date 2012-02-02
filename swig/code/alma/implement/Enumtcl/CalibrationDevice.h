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


#if     !defined(_CALIBRATIONDEVICE_H)

#include <CCalibrationDevice.h>
#define _CALIBRATIONDEVICE_H
#endif 

#if     !defined(_CALIBRATIONDEVICE_HH)

#include "Enum.hpp"

using namespace CalibrationDeviceMod;

template<>
 struct enum_set_traits<CalibrationDevice> : public enum_set_traiter<CalibrationDevice,7,CalibrationDeviceMod::NONE> {};

template<>
class enum_map_traits<CalibrationDevice,void> : public enum_map_traiter<CalibrationDevice,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<CalibrationDevice,EnumPar<void> >
     (CalibrationDeviceMod::AMBIENT_LOAD,ep((int)CalibrationDeviceMod::AMBIENT_LOAD,"AMBIENT_LOAD","un-documented")));
    m_.insert(pair<CalibrationDevice,EnumPar<void> >
     (CalibrationDeviceMod::COLD_LOAD,ep((int)CalibrationDeviceMod::COLD_LOAD,"COLD_LOAD","un-documented")));
    m_.insert(pair<CalibrationDevice,EnumPar<void> >
     (CalibrationDeviceMod::HOT_LOAD,ep((int)CalibrationDeviceMod::HOT_LOAD,"HOT_LOAD","un-documented")));
    m_.insert(pair<CalibrationDevice,EnumPar<void> >
     (CalibrationDeviceMod::NOISE_TUBE_LOAD,ep((int)CalibrationDeviceMod::NOISE_TUBE_LOAD,"NOISE_TUBE_LOAD","un-documented")));
    m_.insert(pair<CalibrationDevice,EnumPar<void> >
     (CalibrationDeviceMod::QUARTER_WAVE_PLATE,ep((int)CalibrationDeviceMod::QUARTER_WAVE_PLATE,"QUARTER_WAVE_PLATE","un-documented")));
    m_.insert(pair<CalibrationDevice,EnumPar<void> >
     (CalibrationDeviceMod::SOLAR_FILTER,ep((int)CalibrationDeviceMod::SOLAR_FILTER,"SOLAR_FILTER","un-documented")));
    m_.insert(pair<CalibrationDevice,EnumPar<void> >
     (CalibrationDeviceMod::NONE,ep((int)CalibrationDeviceMod::NONE,"NONE","un-documented")));
    return true;
  }
  static map<CalibrationDevice,EnumPar<void> > m_;
};
#define _CALIBRATIONDEVICE_HH
#endif
