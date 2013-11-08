//# PolOuterProduct.h: Definition for PolOuterProduct
//# Copyright (C) 2007
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#
//# $Id$

#ifndef SYNTHESIS_POLOUTERPRODUCT_H
#define SYNTHESIS_POLOUTERPRODUCT_H


#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <scimath/Mathematics/RigidVector.h>
#include <coordinates/Coordinates/StokesCoordinate.h>
#include <ms/MeasurementSets/MSIter.h> // Just to get MSIter::{Linear,Circular}!
#include <synthesis/TransformMachines/CFDefs.h>
namespace casa{
  // <summary>  
  //  Class to encapsulate the A-Term outer product (the Mueller Matrix in feed-polarization basis)
  // </summary>
  
  // <use visibility=export>
  // <prerequisite>
  // </prerequisite>
  // <etymology>
  //   
  // </etymology>
  //
  // <synopsis> 
  // 
  //</synopsis>
  using namespace CFDefs;
  class PolOuterProduct 
  {
    public:
    enum GenericVBPol {GPP, GPQ, GQP, GQQ, JUSTWRONGVBPOL};
    enum CrossPolGeneric{PP,     PP2Q,   P2QP,   P2QP2Q,
			 PQ2P,   PQ,     P2QQ2P, P2QQ,
			 Q2PP,   Q2PP2Q, QP,     QP2Q,
			 Q2PQ2P, Q2PQ,   QQ2P,   QQ,
                         JUSTWRONGGENERIC};
    enum CrossPolCircular{RR,     RR2L,   R2LR,   R2LR2L,
			  RL2R,   RL,     R2LL2R, R2LL,
			  L2RR,   L2RR2L, LR,     LR2L,
			  L2RL2R, L2RL,   LL2R,   LL,
                          JUSTWRONGCIRCULAR};
    enum CrossPolLinear{XX,     XX2Y,   X2YX,   X2YX2Y,
			XY2X,   XY,     X2YY2X, X2YY,
			Y2XX,   Y2XX2Y, YX,     YX2Y,
			Y2XY2X, Y2XY,   YY2X,   YY,
                        JUSTWRONGLINEAR};
    enum MuellerType {DIAGONAL=0, HYBRID, FULL};
    //
    //========================= Administrative Parts ==========================
    //------------------------------------------------------------------
    //
    PolOuterProduct (MuellerType type=DIAGONAL) 
    {
      polMap_p.resize(4,4);polMap_p=-1;
      init(type);
    };
    //
    //------------------------------------------------------------------
    //
    PolOuterProduct (const Matrix<Int>& polMap) 
    {
      //      assert(polMap.shape() == IPosition(2,4,4));

      polMap_p.assign(polMap); 
      init();
    };
    //
    //------------------------------------------------------------------
    //
    virtual ~PolOuterProduct () {};
    //
    //============================= Functional Parts ============================
    //------------------------------------------------------------------
    //
    void init(MuellerType type=DIAGONAL)
    {
      muellerType_p = type;
      makeMap();
      nelement_p=countNElements();
    };
    //
    //------------------------------------------------------------------
    //
    void makePolMap(const Vector<CrossPolCircular>& pols);
    //
    //------------------------------------------------------------------
    //
    inline void setElement(const RigidVector<Int,2>& element,const Int& pos)   
    {polMap_p(element(0),element(1)) = pos;}
    //
    //------------------------------------------------------------------
    //
    inline void setElement(const CrossPolCircular& element, const Int& pos)             
    {RigidVector<Int,2> loc=getMuellerElement(element); setElement(loc,pos);}
    //
    //------------------------------------------------------------------
    //
    inline Int getPolEnum(const RigidVector<Int, 2>& element)   
    {return invIndexMap_p(element(0), element(1));};
    //
    //------------------------------------------------------------------
    //
    inline RigidVector<Int,2> getMuellerElement(const CrossPolCircular& element) 
    {return indexMap_p(element);}
    //
    //------------------------------------------------------------------
    //
    inline Bool isSet(const RigidVector<Int,2> element)
    {return (polMap_p(element(0), element(1)) >= 0);};
    //
    //------------------------------------------------------------------
    //
    inline Bool isSet(const CrossPolCircular& element) 
    {return isSet(getMuellerElement(element));}
    //
    //------------------------------------------------------------------
    //
    inline Int countNElements() 
    {
      Int n=0;
      for (Int i=0;i<4;i++)
	for (Int j=0;j<4;j++)
	  if (polMap_p(i,j) >= 0) n++;
      return n;
    }
    //
    //------------------------------------------------------------------
    //
    inline Int nelements() {return nelement_p;};
    //
    //------------------------------------------------------------------
    //
    const Vector<RigidVector<Int,2> >& getIndexMap() {return indexMap_p;};
    //
    //------------------------------------------------------------------
    //
    const Matrix<Int>& getPolMap() {return polMap_p;}
    PolMapType& makePolMat(const Vector<Int>& vbPol, const Vector<Int>& vbPol2ImMap);
    PolMapType& makePol2CFMat(const Vector<Int>& vbPol, const Vector<Int>& vbPol2ImMap);
    PolMapType& makeConjPolMat(const Vector<Int>& vbPol, const Vector<Int>& vbPol2ImMap);
    PolMapType& makeConjPol2CFMat(const Vector<Int>& vbPol, const Vector<Int>& vbPol2ImMap);
    void initCFMaps(const Vector<Int>& visPol, const Vector<Int>& visPolsUsed);

    inline PolMapType& getPolMat() {return outerProduct2VBPolMap_p;};
    inline PolMapType& getConjPolMat() {return conjOuterProduct2VBPolMap_p;};
    inline PolMapType& getPol2CFMat() {return outerProductIndex2VBPolMap_p;};
    inline PolMapType& getConjPol2CFMat() {return conjOuterProductIndex2VBPolMap_p;};
    //
    //============================= Protected Parts ============================
    //
  protected:
    // LogIO& logIO() {return logIO_p;}
    // LogIO logIO_p;
    RigidVector<RigidVector<Int, 4>, 4> muellerRows_p, conjMuellerRows_p;
    Matrix<Int> polMap_p, invIndexMap_p;
    Vector<RigidVector<Int,2> > indexMap_p;
    PolMapType outerProduct2VBPolMap_p, outerProductIndex2VBPolMap_p, 
      conjOuterProduct2VBPolMap_p, conjOuterProductIndex2VBPolMap_p, cfIndices_p;
    MuellerType muellerType_p;
    Int nelement_p;

    PolMapType& makePol2CFMat_p(const Vector<Int>& vbPol, 
				const Vector<Int>& vbPol2ImMap,
				PolMapType& outerProdNdx2VBPolMap);
    PolMapType& makePolMat_p(const Vector<Int>& vbPol, 
			     const Vector<Int>& vbPol2ImMap,
			     PolMapType& outerProd2VBPolMap,
			     RigidVector<RigidVector<Int, 4>, 4>& mRows);

    virtual void makeMap()
    {
      //
      // Make the (damn) mappings!  Phew.
      //

      RigidVector<Int,4> el;
      el(0)=PP;     el(1)=PP2Q;   el(2)=P2QP;    el(3)=P2QP2Q;  muellerRows_p(GPP) = el;
      el(0)=PQ2P;   el(1)=PQ;     el(2)=P2QQ2P;  el(3)=P2QQ;    muellerRows_p(GPQ) = el;
      el(0)=Q2PP;   el(1)=Q2PP2Q; el(2)=QP;      el(3)=QP2Q;    muellerRows_p(GQP) = el;
      el(0)=Q2PQ2P; el(1)=Q2PQ;   el(2)=QQ2P;    el(3)=QQ;      muellerRows_p(GQQ) = el;

      el(0)=QQ;     el(1)=QQ2P;   el(2)=Q2PQ;   el(3)=Q2PQ2P;   conjMuellerRows_p(GPP)=el;
      el(0)=QP2Q;   el(1)=QP;     el(2)=Q2PP2Q; el(3)=Q2PP;     conjMuellerRows_p(GPQ)=el;
      el(0)=P2QQ;   el(1)=P2QQ2P; el(2)=PQ;     el(3)=PQ2P;     conjMuellerRows_p(GQP)=el;
      el(0)=P2QP2Q; el(1)=P2QP;   el(2)=PP2Q;   el(3)=PP;       conjMuellerRows_p(GQQ)=el;



      // Pol-enum to 2-index
      indexMap_p.resize(16);

      indexMap_p(RR)     = RigidVector<Int,2>(0,0);      
      indexMap_p(RR2L)   = RigidVector<Int,2>(0,1);    
      indexMap_p(R2LR)   = RigidVector<Int,2>(0,2);    
      indexMap_p(R2LR2L) = RigidVector<Int,2>(0,3);

      indexMap_p(RL2R)   = RigidVector<Int,2>(1,0);      
      indexMap_p(RL)     = RigidVector<Int,2>(1,1);      
      indexMap_p(R2LL2R) = RigidVector<Int,2>(1,2);      
      indexMap_p(R2LL)   = RigidVector<Int,2>(1,3);      

      indexMap_p(L2RR)   = RigidVector<Int,2>(2,0);     
      indexMap_p(L2RR2L) = RigidVector<Int,2>(2,1);     
      indexMap_p(LR)     = RigidVector<Int,2>(2,2);     
      indexMap_p(LR2L)   = RigidVector<Int,2>(2,3);     

      indexMap_p(L2RL2R) = RigidVector<Int,2>(3,0); 
      indexMap_p(L2RL)   = RigidVector<Int,2>(3,1);
      indexMap_p(LL2R)   = RigidVector<Int,2>(3,2);
      indexMap_p(LL)     = RigidVector<Int,2>(3,3);


      // 2-Index to Pol-enum
      invIndexMap_p.resize(4,4);
      
      invIndexMap_p(0,0)=RR;     invIndexMap_p(0,1)=RR2L;   invIndexMap_p(0,2)=R2LR;   invIndexMap_p(0,3)=R2LR2L;
      invIndexMap_p(1,0)=RL2R;   invIndexMap_p(1,1)=RL;     invIndexMap_p(1,2)=R2LL2R; invIndexMap_p(1,3)=R2LL;
      invIndexMap_p(2,0)=L2RR;   invIndexMap_p(2,1)=L2RR2L; invIndexMap_p(2,2)=LR;     invIndexMap_p(2,3)=LR2L;
      invIndexMap_p(3,0)=L2RL2R; invIndexMap_p(3,1)=L2RL;   invIndexMap_p(3,2)=LL2R;   invIndexMap_p(3,3)=LL;
    };

  };
  //
  //------------------------------------------------------------------
  //
  PolOuterProduct::GenericVBPol translateStokesToGeneric(const Int& stokes) ;
  Int translateGenericToStokes(const PolOuterProduct::GenericVBPol& gPol, 
			       const MSIter::PolFrame& polFrame=MSIter::Circular);
};

#endif
