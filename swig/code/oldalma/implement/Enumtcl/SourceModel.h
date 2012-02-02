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


#if     !defined(_SOURCEMODEL_H)

#include <CSourceModel.h>
#define _SOURCEMODEL_H
#endif 

#if     !defined(_SOURCEMODEL_HH)

#include "Enum.hpp"

using namespace SourceModelMod;

template<>
 struct enum_set_traits<SourceModel> : public enum_set_traiter<SourceModel,3,SourceModelMod::DISK> {};

template<>
class enum_map_traits<SourceModel,void> : public enum_map_traiter<SourceModel,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<SourceModel,EnumPar<void> >
     (SourceModelMod::GAUSSIAN,ep((int)SourceModelMod::GAUSSIAN,"GAUSSIAN","Gaussian source")));
    m_.insert(pair<SourceModel,EnumPar<void> >
     (SourceModelMod::POINT,ep((int)SourceModelMod::POINT,"POINT","Point Source")));
    m_.insert(pair<SourceModel,EnumPar<void> >
     (SourceModelMod::DISK,ep((int)SourceModelMod::DISK,"DISK","Uniform Disk")));
    return true;
  }
  static map<SourceModel,EnumPar<void> > m_;
};
#define _SOURCEMODEL_HH
#endif
