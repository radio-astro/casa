//#---------------------------------------------------------------------------
//# SDMathWrapper.cc: Wrapper classes to use CountedPtr
//#---------------------------------------------------------------------------
//# Copyright (C) 2004
//# ATNF
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but
//# WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: Malte.Marquarding@csiro.au
//#        Postal address: Malte Marquarding,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id:
//#---------------------------------------------------------------------------

#include <singledish/SDMathWrapper.h>

using namespace asap;
using namespace casa;


SDMemTableWrapper SDMathWrapper::quotient(const SDMemTableWrapper& on,
                                          const SDMemTableWrapper& off,
                                          Bool preserveContinuum)
{
    
    Bool doTSys = True;
    return SDMemTableWrapper(sdmath.binaryOperate(on.getCP(), off.getCP(), 
						  String("QUOTIENT"), 
						  preserveContinuum, doTSys));
}


SDMemTableWrapper SDMathWrapper::binaryOperate(const SDMemTableWrapper& left,
                                               const SDMemTableWrapper& right,
                                               const std::string& op, 
					       bool doTSys)
{
    
    return SDMemTableWrapper(sdmath.binaryOperate(left.getCP(), right.getCP(), 
						  String(op), False, 
						  Bool(doTSys)));
}


void SDMathWrapper::scaleInSitu(SDMemTableWrapper& in, float factor, 
				bool doAll, bool doTSys)
{
  SDMemTable* pIn = in.getPtr();
  const uInt what = 0;
  
  SDMemTable* pOut = sdmath.unaryOperate (*pIn, Float(factor), 
					Bool(doAll), what, Bool(doTSys));
  *pIn = *pOut;
   delete pOut;
}

SDMemTableWrapper SDMathWrapper::scale(const SDMemTableWrapper& in,
                                       float factor, bool doAll, bool doTSys)
{
  const CountedPtr<SDMemTable>& pIn = in.getCP();
  const uInt what = 0;
  
  return CountedPtr<SDMemTable>(sdmath.unaryOperate(*pIn, Float(factor), Bool(doAll), 
                                                 what, Bool (doTSys)));
}


void SDMathWrapper::addInSitu(SDMemTableWrapper& in, float offset, bool doAll)
{
  SDMemTable* pIn = in.getPtr();
  const uInt what = 1;
  
  Bool doTSys = False;
  SDMemTable* pOut = sdmath.unaryOperate (*pIn, Float(offset), 
					Bool(doAll), what, doTSys);
  *pIn = *pOut;
   delete pOut;
}

SDMemTableWrapper SDMathWrapper::add(const SDMemTableWrapper& in,
                                     float offset, bool doAll)
{
  const CountedPtr<SDMemTable>& pIn = in.getCP();
  const uInt what = 1;
  
  Bool doTSys = False;
  return CountedPtr<SDMemTable>(sdmath.unaryOperate(*pIn, Float(offset),
						  Bool(doAll), what, doTSys));
}


void SDMathWrapper::smoothInSitu(SDMemTableWrapper& in, 
				 const std::string& kernel, float width,
				 bool doAll)
{
  SDMemTable* pIn = in.getPtr();
  
  SDMemTable* pOut = sdmath.smooth(*pIn, String(kernel), 
				Float(width), Bool(doAll));
  *pIn = *pOut;
   delete pOut;
}


SDMemTableWrapper SDMathWrapper::smooth (const SDMemTableWrapper& in, 
					 const std::string& kernel, 
                                         float width, bool doAll)
{
  const CountedPtr<SDMemTable>& pIn = in.getCP();
  
  return CountedPtr<SDMemTable>(sdmath.smooth(*pIn, String(kernel), 
					   Float(width), Bool(doAll)));
}



void SDMathWrapper::binInSitu(SDMemTableWrapper& in, int width)
{
  SDMemTable* pIn = in.getPtr();
  
  SDMemTable* pOut = sdmath.bin (*pIn, Int(width));
  *pIn = *pOut;
   delete pOut;
}

SDMemTableWrapper SDMathWrapper::bin (const SDMemTableWrapper& in,
                                      int width)
{
  const CountedPtr<SDMemTable>& pIn = in.getCP();
  
  return CountedPtr<SDMemTable>(sdmath.bin(*pIn, Int(width)));
}

void SDMathWrapper::resampleInSitu(SDMemTableWrapper& in, const std::string& method,
                                   float width)
{
  SDMemTable* pIn = in.getPtr();
  
  SDMemTable* pOut = sdmath.resample(*pIn, String(method), Float(width));
  *pIn = *pOut;
   delete pOut;
}

SDMemTableWrapper SDMathWrapper::resample (const SDMemTableWrapper& in,
                                           const std::string& method, float width)
{
  const CountedPtr<SDMemTable>& pIn = in.getCP();
  
  return CountedPtr<SDMemTable>(sdmath.resample(*pIn, String(method), Float(width)));
}


void SDMathWrapper::averagePolInSitu(SDMemTableWrapper& in, 
				     const std::vector<bool>& mask,
                                     const std::string& weightStr)
{
  SDMemTable* pIn = in.getPtr();
  
  Vector<Bool> tMask(mask);
  SDMemTable* pOut = sdmath.averagePol (*pIn, tMask, String(weightStr));
  *pIn = *pOut;
   delete pOut;
}

SDMemTableWrapper SDMathWrapper::averagePol (const SDMemTableWrapper& in,
					     const std::vector<bool>& mask,
                                             const std::string& weightStr)
{
  const CountedPtr<SDMemTable>& pIn = in.getCP();
  
  Vector<Bool> tMask(mask);
  return CountedPtr<SDMemTable>(sdmath.averagePol(*pIn, tMask, String(weightStr)));
}


std::vector<float> SDMathWrapper::statistic(const SDMemTableWrapper& in,
                                            const std::vector<bool>& mask, 
                                            const std::string& which, int row) 
{
  
  Vector<Bool> tMask(mask);
  return sdmath.statistic(in.getCP(), tMask, String(which), Int(row));
}


void SDMathWrapper::convertFluxInSitu(SDMemTableWrapper& in, float D,
                                      float eta, float JyPerK, bool doAll)
{
  SDMemTable* pIn = in.getPtr();
  
  SDMemTable* pOut = sdmath.convertFlux (*pIn, Float(D), Float(eta), Float(JyPerK), Bool(doAll));
  *pIn = *pOut;
  delete pOut;
}


SDMemTableWrapper SDMathWrapper::convertFlux(const SDMemTableWrapper& in, float D, float eta, 
                                             float JyPerK, bool doAll)
{
  const CountedPtr<SDMemTable>& pIn = in.getCP();
  
  return CountedPtr<SDMemTable>(sdmath.convertFlux(*pIn, Float(D), Float(eta), Float(JyPerK), Bool(doAll)));
}


void SDMathWrapper::gainElevationInSitu(SDMemTableWrapper& in, 
                                        const std::vector<float>& coeffs,
                                        const string& fileName,
                                        const string& method, bool doAll)
{
  SDMemTable* pIn = in.getPtr();
  Vector<Float> tCoeffs(coeffs);
  
  SDMemTable* pOut = sdmath.gainElevation(*pIn, tCoeffs, String(fileName), 
                                       String(method), Bool(doAll));
  *pIn = *pOut;
  delete pOut;
}


SDMemTableWrapper SDMathWrapper::gainElevation(const SDMemTableWrapper& in, 
                                               const std::vector<float>& coeffs,
                                               const string& fileName, 
                                               const string& method, bool doAll)
{
  const CountedPtr<SDMemTable>& pIn = in.getCP();
  Vector<Float> tCoeffs(coeffs);
  
  return CountedPtr<SDMemTable>(sdmath.gainElevation(*pIn, tCoeffs, String(fileName), 
                                                  String(method), Bool(doAll)));
}

void SDMathWrapper::frequencyAlignmentInSitu (SDMemTableWrapper& in, const std::string& refTime,
                                              const std::string& method, bool perFreqID)
{
  SDMemTable* pIn = in.getPtr();
  
  SDMemTable* pOut = sdmath.frequencyAlignment(*pIn, String(refTime), String(method),
                                            Bool(perFreqID));
  *pIn = *pOut;
  delete pOut;
}


SDMemTableWrapper SDMathWrapper::frequencyAlignment (const SDMemTableWrapper& in,
                                                     const std::string& refTime,
                                                     const std::string& method,
                                                     bool perFreqID)
{
  const CountedPtr<SDMemTable>& pIn = in.getCP();
  
  return CountedPtr<SDMemTable>(sdmath.frequencyAlignment(*pIn, String(refTime), 
                                                       String(method), Bool(perFreqID)));
}

void SDMathWrapper::rotateXYPhaseInSitu(SDMemTableWrapper& in, float angle, bool doAll)
{
  SDMemTable* pIn = in.getPtr();
  
  sdmath.rotateXYPhase(*pIn, Float(angle), Bool(doAll));
}


void SDMathWrapper::rotateLinPolPhaseInSitu(SDMemTableWrapper& in, float angle, bool doAll)
{
  SDMemTable* pIn = in.getPtr();
  
  sdmath.rotateLinPolPhase(*pIn, Float(angle), Bool(doAll));
}




void SDMathWrapper::opacityInSitu(SDMemTableWrapper& in, float tau, bool doAll)
{
  SDMemTable* pIn = in.getPtr();
  
  SDMemTable* pOut = sdmath.opacity(*pIn, Float(tau), Bool(doAll));
  *pIn = *pOut;
  delete pOut;
}

SDMemTableWrapper SDMathWrapper::opacity(const SDMemTableWrapper& in, 
                                         float tau, bool doAll)
{
  const CountedPtr<SDMemTable>& pIn = in.getCP();
  
  return CountedPtr<SDMemTable>(sdmath.opacity(*pIn, Float(tau), Bool(doAll)));
}

void SDMathWrapper::frequencySwitchInSitu(SDMemTableWrapper& in)
{
  SDMemTable* pIn = in.getPtr();
  SDMath sdm;
  SDMemTable* pOut = sdm.frequencySwitch(*pIn);
  *pIn = *pOut;
  delete pOut;
}

SDMemTableWrapper SDMathWrapper::frequencySwitch(const SDMemTableWrapper& in)
{
  const CountedPtr<SDMemTable>& pIn = in.getCP();
  SDMath sdm;
  return CountedPtr<SDMemTable>(sdm.frequencySwitch(*pIn));
}
