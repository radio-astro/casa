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


#if     !defined(_WINDOWFUNCTION_H)

#include <CWindowFunction.h>
#define _WINDOWFUNCTION_H
#endif 

#if     !defined(_WINDOWFUNCTION_HH)

#include "Enum.hpp"

using namespace WindowFunctionMod;

template<>
 struct enum_set_traits<WindowFunction> : public enum_set_traiter<WindowFunction,7,WindowFunctionMod::WELCH> {};

template<>
class enum_map_traits<WindowFunction,void> : public enum_map_traiter<WindowFunction,void> {
public:
  static bool   init_;
  static string typeName_;
  static string enumerationDesc_;
  static string order_;
  static string xsdBaseType_;
  static bool   init(){
    EnumPar<void> ep;
    m_.insert(pair<WindowFunction,EnumPar<void> >
     (WindowFunctionMod::UNIFORM,ep((int)WindowFunctionMod::UNIFORM,"UNIFORM","No windowing")));
    m_.insert(pair<WindowFunction,EnumPar<void> >
     (WindowFunctionMod::HANNING,ep((int)WindowFunctionMod::HANNING,"HANNING","Raised cosine: 0.5*(1-cos(x)) where x = 2*pi*i/(N-1)")));
    m_.insert(pair<WindowFunction,EnumPar<void> >
     (WindowFunctionMod::HAMMING,ep((int)WindowFunctionMod::HAMMING,"HAMMING","The classic Hamming window is Wm(x) = 0.54 - 0.46*cos(x). This is generalized to Wm(x) = beta - (1-beta)*cos(x) where beta can take any value in the range [0,1]. Beta=0.5 corresponds to the Hanning window.")));
    m_.insert(pair<WindowFunction,EnumPar<void> >
     (WindowFunctionMod::BARTLETT,ep((int)WindowFunctionMod::BARTLETT,"BARTLETT","The Bartlett (triangular) window is 1 - |x/pi|, where x = 2*pi*i/(N-1)")));
    m_.insert(pair<WindowFunction,EnumPar<void> >
     (WindowFunctionMod::BLACKMANN,ep((int)WindowFunctionMod::BLACKMANN,"BLACKMANN","The window function is: Wb(x) = (0.5 - Beta) - 0.5*cos(Xj) + Beta*cos(2Xj), where Xj=2*pi*j/(N-1). The classic Blackman window is given by Beta=0.08")));
    m_.insert(pair<WindowFunction,EnumPar<void> >
     (WindowFunctionMod::BLACKMANN_HARRIS,ep((int)WindowFunctionMod::BLACKMANN_HARRIS,"BLACKMANN_HARRIS","The BLACKMANN_HARRIS window is 1.0 - 1.36109*cos(x) + 0.39381*cos(2x) - 0.032557*cos(3x), where x = 2*pi*i/(N-1)")));
    m_.insert(pair<WindowFunction,EnumPar<void> >
     (WindowFunctionMod::WELCH,ep((int)WindowFunctionMod::WELCH,"WELCH","The Welch window (parabolic) is 1 - (2*i/N)^2")));
    return true;
  }
  static map<WindowFunction,EnumPar<void> > m_;
};
#define _WINDOWFUNCTION_HH
#endif
