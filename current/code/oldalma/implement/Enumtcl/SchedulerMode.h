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


#if     !defined(_SCHEDULERMODE_H)

#include <CSchedulerMode.h>
#define _SCHEDULERMODE_H
#endif 

#if     !defined(_SCHEDULERMODE_HH)

#include "Enum.hpp"

using namespace SchedulerModeMod;

template<>
 struct enum_set_traits<SchedulerMode> : public enum_set_traiter<SchedulerMode,4,SchedulerModeMod::QUEUED> {};

template<>
class enum_map_traits<SchedulerMode,void> : public enum_map_traiter<SchedulerMode,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<SchedulerMode,EnumPar<void> >
     (SchedulerModeMod::DYNAMIC,ep((int)SchedulerModeMod::DYNAMIC,"DYNAMIC","Dynamic scheduling")));
    m_.insert(pair<SchedulerMode,EnumPar<void> >
     (SchedulerModeMod::INTERACTIVE,ep((int)SchedulerModeMod::INTERACTIVE,"INTERACTIVE","Interactive scheduling")));
    m_.insert(pair<SchedulerMode,EnumPar<void> >
     (SchedulerModeMod::MANUAL,ep((int)SchedulerModeMod::MANUAL,"MANUAL","Manual scheduling")));
    m_.insert(pair<SchedulerMode,EnumPar<void> >
     (SchedulerModeMod::QUEUED,ep((int)SchedulerModeMod::QUEUED,"QUEUED","Queued scheduling")));
    return true;
  }
  static map<SchedulerMode,EnumPar<void> > m_;
};
#define _SCHEDULERMODE_HH
#endif
