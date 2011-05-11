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


#if     !defined(_ASSOCIATEDCALNATURE_H)

#include <CAssociatedCalNature.h>
#define _ASSOCIATEDCALNATURE_H
#endif 

#if     !defined(_ASSOCIATEDCALNATURE_HH)

#include "Enum.hpp"

using namespace AssociatedCalNatureMod;

template<>
 struct enum_set_traits<AssociatedCalNature> : public enum_set_traiter<AssociatedCalNature,1,AssociatedCalNatureMod::ASSOCIATED_EXECBLOCK> {};

template<>
class enum_map_traits<AssociatedCalNature,void> : public enum_map_traiter<AssociatedCalNature,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<AssociatedCalNature,EnumPar<void> >
     (AssociatedCalNatureMod::ASSOCIATED_EXECBLOCK,ep((int)AssociatedCalNatureMod::ASSOCIATED_EXECBLOCK,"ASSOCIATED_EXECBLOCK","The associated execblock id concatenated to produce the data set")));
    return true;
  }
  static map<AssociatedCalNature,EnumPar<void> > m_;
};
#define _ASSOCIATEDCALNATURE_HH
#endif
