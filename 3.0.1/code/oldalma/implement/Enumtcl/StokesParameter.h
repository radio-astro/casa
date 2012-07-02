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


#if     !defined(_STOKESPARAMETER_H)

#include <CStokesParameter.h>
#define _STOKESPARAMETER_H
#endif 

#if     !defined(_STOKESPARAMETER_HH)

#include "Enum.hpp"

using namespace StokesParameterMod;

template<>
 struct enum_set_traits<StokesParameter> : public enum_set_traiter<StokesParameter,32,StokesParameterMod::PANGLE> {};

template<>
class enum_map_traits<StokesParameter,void> : public enum_map_traiter<StokesParameter,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::I,ep((int)StokesParameterMod::I,"I","")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::Q,ep((int)StokesParameterMod::Q,"Q","")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::U,ep((int)StokesParameterMod::U,"U","")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::V,ep((int)StokesParameterMod::V,"V","")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::RR,ep((int)StokesParameterMod::RR,"RR","")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::RL,ep((int)StokesParameterMod::RL,"RL","")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::LR,ep((int)StokesParameterMod::LR,"LR","")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::LL,ep((int)StokesParameterMod::LL,"LL","")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::XX,ep((int)StokesParameterMod::XX,"XX","Linear correlation product")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::XY,ep((int)StokesParameterMod::XY,"XY","")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::YX,ep((int)StokesParameterMod::YX,"YX","")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::YY,ep((int)StokesParameterMod::YY,"YY","")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::RX,ep((int)StokesParameterMod::RX,"RX","Mixed correlation product")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::RY,ep((int)StokesParameterMod::RY,"RY","Mixed correlation product")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::LX,ep((int)StokesParameterMod::LX,"LX","Mixed LX product")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::LY,ep((int)StokesParameterMod::LY,"LY","Mixed LY correlation product")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::XR,ep((int)StokesParameterMod::XR,"XR","Mixed XR correlation product")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::XL,ep((int)StokesParameterMod::XL,"XL","Mixed XL correlation product")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::YR,ep((int)StokesParameterMod::YR,"YR","Mixed YR correlation product")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::YL,ep((int)StokesParameterMod::YL,"YL","Mixel YL correlation product")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::PP,ep((int)StokesParameterMod::PP,"PP","")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::PQ,ep((int)StokesParameterMod::PQ,"PQ","")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::QP,ep((int)StokesParameterMod::QP,"QP","")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::QQ,ep((int)StokesParameterMod::QQ,"QQ","")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::RCIRCULAR,ep((int)StokesParameterMod::RCIRCULAR,"RCIRCULAR","")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::LCIRCULAR,ep((int)StokesParameterMod::LCIRCULAR,"LCIRCULAR","")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::LINEAR,ep((int)StokesParameterMod::LINEAR,"LINEAR","single dish polarization type")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::PTOTAL,ep((int)StokesParameterMod::PTOTAL,"PTOTAL","Polarized intensity ((Q^2+U^2+V^2)^(1/2))")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::PLINEAR,ep((int)StokesParameterMod::PLINEAR,"PLINEAR","Linearly Polarized intensity ((Q^2+U^2)^(1/2))")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::PFTOTAL,ep((int)StokesParameterMod::PFTOTAL,"PFTOTAL","Polarization Fraction (Ptotal/I)")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::PFLINEAR,ep((int)StokesParameterMod::PFLINEAR,"PFLINEAR","Linear Polarization Fraction (Plinear/I)")));
    m_.insert(pair<StokesParameter,EnumPar<void> >
     (StokesParameterMod::PANGLE,ep((int)StokesParameterMod::PANGLE,"PANGLE","Linear Polarization Angle (0.5 arctan(U/Q)) (in radians)")));
    return true;
  }
  static map<StokesParameter,EnumPar<void> > m_;
};
#define _STOKESPARAMETER_HH
#endif
