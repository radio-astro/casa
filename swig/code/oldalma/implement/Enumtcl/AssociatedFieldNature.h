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


#if     !defined(_ASSOCIATEDFIELDNATURE_H)

#include <CAssociatedFieldNature.h>
#define _ASSOCIATEDFIELDNATURE_H
#endif 

#if     !defined(_ASSOCIATEDFIELDNATURE_HH)

#include "Enum.hpp"

using namespace AssociatedFieldNatureMod;

template<>
 struct enum_set_traits<AssociatedFieldNature> : public enum_set_traiter<AssociatedFieldNature,3,AssociatedFieldNatureMod::PHASE_REFERENCE> {};

template<>
class enum_map_traits<AssociatedFieldNature,void> : public enum_map_traiter<AssociatedFieldNature,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<AssociatedFieldNature,EnumPar<void> >
     (AssociatedFieldNatureMod::ON,ep((int)AssociatedFieldNatureMod::ON,"ON","The associated field is used as ON source data")));
    m_.insert(pair<AssociatedFieldNature,EnumPar<void> >
     (AssociatedFieldNatureMod::OFF,ep((int)AssociatedFieldNatureMod::OFF,"OFF","The associated field is used as OFF source data")));
    m_.insert(pair<AssociatedFieldNature,EnumPar<void> >
     (AssociatedFieldNatureMod::PHASE_REFERENCE,ep((int)AssociatedFieldNatureMod::PHASE_REFERENCE,"PHASE_REFERENCE","The associated field is used as Phase reference data")));
    return true;
  }
  static map<AssociatedFieldNature,EnumPar<void> > m_;
};
#define _ASSOCIATEDFIELDNATURE_HH
#endif
