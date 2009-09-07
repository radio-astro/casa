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


#if     !defined(_ACAPOLARIZATION_H)

#include <CACAPolarization.h>
#define _ACAPOLARIZATION_H
#endif 

#if     !defined(_ACAPOLARIZATION_HH)

#include "Enum.hpp"

using namespace ACAPolarizationMod;

template<>
 struct enum_set_traits<ACAPolarization> : public enum_set_traiter<ACAPolarization,4,ACAPolarizationMod::ACA_YY_50> {};

template<>
class enum_map_traits<ACAPolarization,void> : public enum_map_traiter<ACAPolarization,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<ACAPolarization,EnumPar<void> >
     (ACAPolarizationMod::ACA_STANDARD,ep((int)ACAPolarizationMod::ACA_STANDARD,"ACA_STANDARD","un-documented")));
    m_.insert(pair<ACAPolarization,EnumPar<void> >
     (ACAPolarizationMod::ACA_XX_YY_SUM,ep((int)ACAPolarizationMod::ACA_XX_YY_SUM,"ACA_XX_YY_SUM","un-documented")));
    m_.insert(pair<ACAPolarization,EnumPar<void> >
     (ACAPolarizationMod::ACA_XX_50,ep((int)ACAPolarizationMod::ACA_XX_50,"ACA_XX_50","un-documented")));
    m_.insert(pair<ACAPolarization,EnumPar<void> >
     (ACAPolarizationMod::ACA_YY_50,ep((int)ACAPolarizationMod::ACA_YY_50,"ACA_YY_50","un-documented")));
    return true;
  }
  static map<ACAPolarization,EnumPar<void> > m_;
};
#define _ACAPOLARIZATION_HH
#endif
