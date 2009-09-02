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


#if     !defined(_FLUXCALIBRATIONMETHOD_H)

#include <CFluxCalibrationMethod.h>
#define _FLUXCALIBRATIONMETHOD_H
#endif 

#if     !defined(_FLUXCALIBRATIONMETHOD_HH)

#include "Enum.hpp"

using namespace FluxCalibrationMethodMod;

template<>
 struct enum_set_traits<FluxCalibrationMethod> : public enum_set_traiter<FluxCalibrationMethod,3,FluxCalibrationMethodMod::EFFICIENCY> {};

template<>
class enum_map_traits<FluxCalibrationMethod,void> : public enum_map_traiter<FluxCalibrationMethod,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<FluxCalibrationMethod,EnumPar<void> >
     (FluxCalibrationMethodMod::ABSOLUTE,ep((int)FluxCalibrationMethodMod::ABSOLUTE,"ABSOLUTE","Absolute flux calibration (based on standard antenna)")));
    m_.insert(pair<FluxCalibrationMethod,EnumPar<void> >
     (FluxCalibrationMethodMod::RELATIVE,ep((int)FluxCalibrationMethodMod::RELATIVE,"RELATIVE","Relative flux calibration (based on a primary calibrator)")));
    m_.insert(pair<FluxCalibrationMethod,EnumPar<void> >
     (FluxCalibrationMethodMod::EFFICIENCY,ep((int)FluxCalibrationMethodMod::EFFICIENCY,"EFFICIENCY","Flux calibrator based on tabulated antenna efficiciency")));
    return true;
  }
  static map<FluxCalibrationMethod,EnumPar<void> > m_;
};
#define _FLUXCALIBRATIONMETHOD_HH
#endif
