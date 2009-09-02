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


#if     !defined(_POINTINGMETHOD_H)

#include <CPointingMethod.h>
#define _POINTINGMETHOD_H
#endif 

#if     !defined(_POINTINGMETHOD_HH)

#include "Enum.hpp"

using namespace PointingMethodMod;

template<>
 struct enum_set_traits<PointingMethod> : public enum_set_traiter<PointingMethod,5,PointingMethodMod::CIRCLE> {};

template<>
class enum_map_traits<PointingMethod,void> : public enum_map_traiter<PointingMethod,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<PointingMethod,EnumPar<void> >
     (PointingMethodMod::THREE_POINT,ep((int)PointingMethodMod::THREE_POINT,"THREE_POINT","Three-point scan")));
    m_.insert(pair<PointingMethod,EnumPar<void> >
     (PointingMethodMod::FOUR_POINT,ep((int)PointingMethodMod::FOUR_POINT,"FOUR_POINT","Four-point scan")));
    m_.insert(pair<PointingMethod,EnumPar<void> >
     (PointingMethodMod::FIVE_POINT,ep((int)PointingMethodMod::FIVE_POINT,"FIVE_POINT","Five-point scan")));
    m_.insert(pair<PointingMethod,EnumPar<void> >
     (PointingMethodMod::CROSS,ep((int)PointingMethodMod::CROSS,"CROSS","Cross scan")));
    m_.insert(pair<PointingMethod,EnumPar<void> >
     (PointingMethodMod::CIRCLE,ep((int)PointingMethodMod::CIRCLE,"CIRCLE","Circular scan")));
    return true;
  }
  static map<PointingMethod,EnumPar<void> > m_;
};
#define _POINTINGMETHOD_HH
#endif
