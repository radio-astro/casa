//#---------------------------------------------------------------------------
//# SDPol.cc: Polarimetric functionality
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

#include <singledish/SDPol.h>
#include <singledish/SDDefs.h>

#include <casa/Arrays/Array.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayLogical.h>
#include <casa/Arrays/VectorIter.h>
#include <casa/Containers/Record.h>
#include <casa/BasicSL/Constants.h>
#include <casa/BasicSL/String.h>
#include <casa/Utilities/Assert.h>

#include <tables/Tables/Table.h>
#include <tables/Tables/ScalarColumn.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/ColumnDesc.h>
#include <tables/Tables/TableRecord.h>
#include <tables/Tables/DataManError.h>


using namespace casa;
using namespace asap;



SDStokesEngine::SDStokesEngine (const String& outputColumnName,
			   const String& inputColumnName)
: BaseMappedArrayEngine<Float,Float> (outputColumnName, inputColumnName)
{
   setWritable(False);
}


SDStokesEngine::SDStokesEngine (const Record& spec)
: BaseMappedArrayEngine<Float,Float> ()
{
    setWritable(False);
    if (spec.isDefined("OUTPUTNAME")  &&  spec.isDefined("INPUTNAME")) {
        setNames (spec.asString("OUTPUTNAME"), spec.asString("INPUTNAME"));
    }
}

SDStokesEngine::SDStokesEngine (const SDStokesEngine& that)
: BaseMappedArrayEngine<Float,Float> (that)
{}

SDStokesEngine::~SDStokesEngine()
{}


DataManager* SDStokesEngine::clone() const
{
    DataManager* dmPtr = new SDStokesEngine (*this);
    return dmPtr;
}


String SDStokesEngine::dataManagerType() const
{
    return className();
}

String SDStokesEngine::className()
{
    return "SDStokesEngine";
}

String SDStokesEngine::dataManagerName() const
{
    return virtualName();
}

Record SDStokesEngine::dataManagerSpec() const
{
    Record spec;
    spec.define ("OUTPUTNAME", virtualName());
    spec.define ("INPUTNAME", storedName());
    return spec;
}

DataManager* SDStokesEngine::makeObject (const String&, const Record& spec)
{
    DataManager* dmPtr = new SDStokesEngine(spec);
    return dmPtr;
}


void SDStokesEngine::registerClass()
{
    DataManager::registerCtor (className(), makeObject);
}


void SDStokesEngine::create (uInt initialNrrow)
{
    BaseMappedArrayEngine<Float,Float>::create (initialNrrow);
}

void SDStokesEngine::prepare()
{
    BaseMappedArrayEngine<Float,Float>::prepare();
}

Bool SDStokesEngine::canAccessArrayColumnCells (Bool& reask) const
{
    reask = False;
    return True;
}


void SDStokesEngine::getArray (uInt rownr, Array<Float>& output)
{
    Array<Float> input;
    roColumn().get(rownr, input);
//
    computeOnGet (output, input);
}

void SDStokesEngine::putArray (uInt rownr, const Array<Float>& input)
{
    throw(AipsError("This Virtual Column is not writable"));
}

    
IPosition SDStokesEngine::shape (uInt rownr)
{
   IPosition inputShape = roColumn().shape (rownr);
   return findOutputShape(inputShape);
}



void SDStokesEngine::computeOnGet(Array<Float>& output,
                   		 const Array<Float>& input)
//
// array of shape (nBeam,nIF,nPol,nChan)
//
// We use the scaling convention I=(XX+YY)/2

{

// Checks

   const uInt nDim = input.ndim();
   AlwaysAssert(nDim==4,AipsError);
   AlwaysAssert(output.ndim()==4,AipsError);
//
   const IPosition inputShape = input.shape();
   const uInt polAxis = asap::PolAxis;
   const uInt nPol = inputShape(polAxis);
   AlwaysAssert(nPol==1 || nPol==2 || nPol==4, AipsError);

// The silly Array slice operator does not give me back
// a const reference so have to caste it away

   Array<Float>& input2 = const_cast<Array<Float>&>(input);

// Slice coordnates

   IPosition start(nDim,0);
   IPosition end(input.shape()-1);

// Generate Slices

   start(polAxis) = 0;
   end(polAxis) = 0;
   Array<Float> C1 = input2(start,end);          // Input : C1
//
   start(polAxis) = 0;
   end(polAxis) = 0;
   Array<Float> I = output(start,end);           // Output : I
//
   if (nPol==1) {
      I = C1;
      return;
   }
//
   start(polAxis) = 1;
   end(polAxis) = 1;
   Array<Float> C2 = input2(start,end);          // Input : C1
//
   I = Float(0.5) * (C1 + C2);
   if (nPol <= 2) return;
//
   start(polAxis) = 2;
   end(polAxis) = 2;
   Array<Float> C3 = input2(start,end);          // Input : C3
//
   start(polAxis) = 3;
   end(polAxis) = 3;
   Array<Float> C4 = input2(start,end);          // Input : C4
//
   start(polAxis) = 1;
   end(polAxis) = 1;
   Array<Float> Q = output(start,end);           // Output : Q
   Q = Float(0.5) * (C1 - C2);
//
   start(polAxis) = 2;
   end(polAxis) = 2;
   Array<Float> U = output(start,end);           // Output : U
   U = C3;
//
   start(polAxis) = 3;
   end(polAxis) = 3;
   Array<Float> V = output(start,end);           // Output : V
   V = C4;
}



IPosition SDStokesEngine::findOutputShape (const IPosition& inputShape) const
{
   uInt axis = 2;
   uInt nPol = inputShape(axis);
   IPosition outputShape = inputShape;
   if (nPol==1) {
      outputShape(axis) = 1;            // XX -> I
   } else if (nPol==2) {
      outputShape(axis) = 1;            // XX YY -> I
   } else if (nPol==4) {
      outputShape(axis) = 4;            // XX YY R(XY) I(XY) -> I Q U V
   }
   return outputShape;
}



// SDPolUtil

Array<Float> SDPolUtil::polarizedIntensity (const Array<Float>& Q,
                                            const Array<Float>& U)
{
   Array<Float> t1 = pow(Q,Double(2.0));
   Array<Float> t2 = pow(U,Double(2.0));
   return sqrt(t1+t2);
}


Array<Float> SDPolUtil::positionAngle (const Array<Float>& Q,
                                       const Array<Float>& U)
{
   return Float(180.0/C::pi/2.0)*atan2(U,Q);       // Degrees
}


void SDPolUtil::rotatePhase (Array<Float>& R,
                             Array<Float>& I,
                             Float phase)
//
// Apply phase rotation to Z = (R + iI)
//
{
   Float cosVal = cos(C::pi/180.0*phase);
   Float sinVal = sin(C::pi/180.0*phase);
//
   Array<Float> R2 = R*cosVal - I*sinVal;
   I =  R*sinVal + I*cosVal;
   R = R2;
}


void SDPolUtil::rotateLinPolPhase (Array<Float>& C1,
                                   Array<Float>& C2,
                                   Array<Float>& C3,
                                   Array<Float>& I,
                                   Array<Float>& Q,
                                   Array<Float>& U,
                                   Float phase)
//
// Rotate P = Q + iU but do it directly on the  linear
// correlations.
//
// We are using I=(XX+YY)/2 convention
// C1 = XX; C2 = YY, C3 = Real(XY)
//
{
// Rotate Q & U (factor of 2 for polarization)

   rotatePhase(Q, U, 2.0*phase);

// Now recompute C1,C2,C3 
// C4 unchanged 

   C1 = I + Q;
   C2 = I - Q;
   C3 = U;
}





Array<Float> SDPolUtil::getStokesSlice (Array<Float>& in, const IPosition& start,
                                        const IPosition& end, const String& stokes)
{
   IPosition s(start);
   IPosition e(end);
//
   if (stokes=="I") {
      s(asap::PolAxis) = 0;
      e(asap::PolAxis) = 0;
   } else if (stokes=="Q") {
      s(asap::PolAxis) = 1;
      e(asap::PolAxis) = 1;
   } else if (stokes=="U") {
      s(asap::PolAxis) = 2;
      e(asap::PolAxis) = 2;
   } else if (stokes=="V") {
      s(asap::PolAxis) = 3;
      e(asap::PolAxis) = 3;
   }
//
   return in(s,e);
}
 

Array<Float> SDPolUtil::circularPolarizationFromStokes (Array<Float>& I,
                                                        Array<Float>& V, 
                                                        Bool doRR)
//
// We use the convention
//  I = (RR+LL)/2
//
{
   if (doRR) {
      return I + V;
   } else {
      return I - V;
   }
}

Stokes::StokesTypes SDPolUtil::convertStokes(Int val, Bool toStokes, Bool linear)
{   
   Stokes::StokesTypes stokes = Stokes::Undefined;
   if (toStokes) {
      if (val==0) {
          stokes = Stokes::I;
      } else if (val==1) {
         stokes = Stokes::Q;
      } else if (val==2) {
         stokes = Stokes::U;
      } else if (val==3) {
         stokes = Stokes::V;
      }   
   } else if (linear) {
      if (val==0) {
         stokes = Stokes::XX;
      } else if (val==1) {
         stokes = Stokes::YY;
      } else if (val==2) {
         stokes = Stokes::XY;         // Real(XY)
      } else if (val==3) {
         stokes = Stokes::XY;         // Imag(XY)
      }
   } else {
      if (val==0) {
         stokes = Stokes::RR;
      } else if (val==1) {
         stokes = Stokes::LL;
      } else if (val==2) {
         stokes = Stokes::RL;
      } else if (val==3) {
         stokes = Stokes::RL;
      }
   }
//
   return stokes;
}



String SDPolUtil::polarizationLabel (uInt polIdx, Bool linear, Bool stokes, Bool linPol)
{   
   Stokes::StokesTypes type = Stokes::Undefined;
   if (stokes) {
      switch (polIdx) {
         case 0:
           {
              type = Stokes::I;
           }
           break;
         case 1:
           {
              if (linPol) {
                 type = Stokes::Plinear;
              } else {
                 type = Stokes::Q;
              }
           }
           break;
         case 2:
           {
              if (linPol) {
                 type = Stokes::Pangle;
              } else {
                 type = Stokes::U;
              }
           }
           break;
         case 3:
           {
              type = Stokes::V;
           }
           break;
         default:  
           {
               throw(AipsError("Unknown Stokes type"));
           }
      }
   } else {
      if (linear) {
         switch (polIdx) {
            case 0:
              {
                 type = Stokes::XX;
              }
              break;
            case 1:
              {
                 type = Stokes::YY;
              }
              break;
            case 2:
              {
                 type = Stokes::XY;              // Really Real(XY)
                 return String("Real(XY)");
              }
              break;
            case 3:
              {
                 type = Stokes::YX;              // Really Imag(XY)
                 return String("Imag(XY)");
              }
              break;
            default:  
              {
                  throw(AipsError("Unknown linear polarization type"));
              }
         }
      } else {  
         switch (polIdx) {
            case 0:
              {
                 type = Stokes::RR;
              }
              break;
            case 1:
              {
                 type = Stokes::LL;
              }
	      break;
            case 2:
              {
                 type = Stokes::RL;               // Really Real(RL)
                 return String("Real(RL)");
              }
              break;
            case 3:
              {
                 type = Stokes::LR;               // Really Imag(RL)
                 return String("Imag(RL)");
              }
              break;
            default:  
              {
                  throw(AipsError("Unknown circular polarization type"));
              }
         }
      }
   }
//
   return SDPolUtil::stokesString(type);
} 




// private

String SDPolUtil::stokesString (Stokes::StokesTypes type)
{
  return Stokes::name (type);
}


Array<casa::uChar> SDPolUtil::andArrays (const Array<casa::uChar>& in1,
                                         const Array<casa::uChar>& in2)
{
   Array<uChar> out(in1.shape());
//
   Array<uChar>::const_iterator in1Iter;
   Array<uChar>::const_iterator in2Iter;
   Array<uChar>::iterator outIter;
//
   for (in1Iter=in1.begin(),in2Iter=in2.begin(),outIter=out.begin();
        in1Iter!=in1.end(); ++in1Iter,++in2Iter,++outIter) {  
      *outIter = *in1Iter & *in2Iter;
   }
   return out;
}


Array<Float> SDPolUtil::extractStokesForWriter (Array<Float>& in, const IPosition& start, const IPosition& end)
//
// start/end must already have applied the cursor selection of beam and IF
// Extract specified Stokes for beam/IF and flip nChan and nPol for bloody SDwriter
//
{
   IPosition shapeIn = in.shape();
   uInt nChan = shapeIn(asap::ChanAxis);
   uInt nPol = shapeIn(asap::PolAxis);
//
   IPosition shapeOut(2,nChan,nPol);
   Array<Float> out(shapeOut);
//
   Array<Float> sliceRef = in(start,end);                        // Beam and IF now degenerate axes
   ReadOnlyVectorIterator<Float> itIn(sliceRef, asap::ChanAxis);
   VectorIterator<Float> itOut(out,0);
   while (!itIn.pastEnd()) {
      itOut.vector() = itIn.vector();
// 
      itIn.next();
      itOut.next();
   }
//
   return out;
}





