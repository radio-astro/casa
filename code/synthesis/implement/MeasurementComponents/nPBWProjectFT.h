//# nPBWProjectFT.h: Definition for nPBWProjectFT
//# Copyright (C) 1996,1997,1998,1999,2000,2002
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

#ifndef SYNTHESIS_PBWPROJECTFT_H
#define SYNTHESIS_PBWPROJECTFT_H

#include <synthesis/MeasurementComponents/FTMachine.h>
#include <casa/Arrays/Matrix.h>
#include <scimath/Mathematics/FFTServer.h>
#include <msvis/MSVis/VisBuffer.h>
#include <images/Images/ImageInterface.h>
#include <casa/Containers/Block.h>
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <scimath/Mathematics/ConvolveGridder.h>
#include <lattices/Lattices/LatticeCache.h>
#include <lattices/Lattices/ArrayLattice.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <measures/Measures/Measure.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MPosition.h>
#include <coordinates/Coordinates/DirectionCoordinate.h>
#include <synthesis/MeasurementComponents/VPSkyJones.h>
#include <synthesis/MeasurementComponents/VLACalcIlluminationConvFunc.h>
#include <synthesis/MeasurementComponents/VLAIlluminationConvFunc.h>
#include <synthesis/MeasurementComponents/Utils.h>

#include <synthesis/MeasurementComponents/EPJones.h>
#include <synthesis/MeasurementComponents/ConvFuncDiskCache.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  
  // <summary>  An FTMachine for Gridded Fourier transforms including effects of primary beam and pointing offsets and the w-term</summary>
  
  // <use visibility=export>
  
  // <reviewed reviewer="" date="" tests="" demos="">
  
  // <prerequisite>
  //   <li> <linkto class=FTMachine>FTMachine</linkto> module
  //   <li> <linkto class=SkyEquation>SkyEquation</linkto> module
  //   <li> <linkto class=VisBuffer>VisBuffer</linkto> module
  //   <li> <linto class=EPJones>EPJones</linkto> module
  // </prerequisite>
  //
  // <etymology> 
  // FTMachine is a Machine for Fourier Transforms. Like
  // WProjectFT, nPBWProjectFT does Grid-based Fourier transforms but
  // also includes the effects of primary beam and antenna pointing
  // offsets. 
  // </etymology>
  //
  // <synopsis> 
  //
  // The <linkto class=SkyEquation>SkyEquation</linkto> needs to be
  // able to perform Fourier transforms on visibility
  // data. nPBWProjectFT allows efficient handling of direction
  // dependent effects due to the primary beam and antenna pointing
  // offsets using a <linkto class=VisBuffer>VisBuffer</linkto> which
  // encapsulates a chunk of visibility (typically all baselines for
  // one time) together with all the information needed for processing
  // (e.g. UVW coordinates).
  //
  // Using this FTMachine, errors due antenna pointing offsets can be
  // corrected during deconvolution.  One form of antenna pointing
  // error which is known a-priori is the VLA polarization squint
  // (about 6% of the Primary beam width at any frequency).  For
  // Stokes imaging, using this FTMachine, the VLA polarization squint
  // and beam polarization can also be corrected.  Also since the
  // effects of antenna pointing errors is strongest in the range of
  // 1-2GHz band (where the sky is not quite empty while the beams are
  // not too large either), this FTMachine can also be setup to
  // correct for the w-term.
  //
  // Switches are provided in the get() method to compute the
  // derivatives with respect to the parameters of the primary beam
  // (only pointing offsets for now).  This is used in the pointing
  // offset solver.
  //
  // See the documentation of other FTMachines for details about the
  // design of the FTMachines in general.
  //
  // </synopsis> 
  //
  // <example>
  // See the example for <linkto class=SkyModel>SkyModel</linkto>.
  // </example>
  //
  // <motivation>
  //
  // Encapsulate the correction of direction dependent effects via
  // visibility plane convolutions with a potentially different
  // convolution function for each baseline.
  //
  // </motivation>
  //
  // <todo asof="2005/07/21">
  //
  // <ul> Include the antenna based complex gain term as well since
  // that can interfere with the effects of pointing offsets.  
  //
  // <ul> Factor out the actual convolution functions as a separate
  // class making FTMachines for various direction dependent effects
  // generic.
  //
  // </todo>
  
  class EPJones;
  class nPBWProjectFT : public FTMachine {
  public:
    
    // Constructor: cachesize is the size of the cache in words
    // (e.g. a few million is a good number), tilesize is the
    // size of the tile used in gridding (cannot be less than
    // 12, 16 works in most cases). 
    // <group>
    nPBWProjectFT(Int nFacets, Long cachesize, String& cfCacheDirName,
		  Bool applyPointingOffset=True,
		  Bool doPBCorr=True,
		  Int tilesize=16, 
		  Float paSteps=5.0, Float pbLimit=5e-2,
		  Bool usezero=False);
    // </group>
    
    // Construct from a Record containing the nPBWProjectFT state
    nPBWProjectFT(const RecordInterface& stateRec);
    
    // Copy constructor
    nPBWProjectFT(const nPBWProjectFT &other);
    
    // Assignment operator
    nPBWProjectFT &operator=(const nPBWProjectFT &other);
    
    ~nPBWProjectFT();
    
    void setEPJones(EPJones* ep_j) {epJ = ep_j;}
    
    void setDOPBCorrection(Bool doit=True) {doPBCorrection=doit;};
    // Initialize transform to Visibility plane using the image
    // as a template. The image is loaded and Fourier transformed.
    
    virtual void initializeToVis(ImageInterface<Complex>& image,
			 const VisBuffer& vb);
    // This version returns the gridded vis...should be used in conjunction 
    // with the version of 'get' that needs the gridded visdata 
    virtual void initializeToVis(ImageInterface<Complex>& image,
			 const VisBuffer& vb, Array<Complex>& griddedVis,
			 Vector<Double>& uvscale);
    
    // Finalize transform to Visibility plane: flushes the image
    // cache and shows statistics if it is being used.
    virtual void finalizeToVis();
    
    // Initialize transform to Sky plane: initializes the image
    virtual void initializeToSky(ImageInterface<Complex>& image,  Matrix<Float>& weight,
			 const VisBuffer& vb);
    
    // Finalize transform to Sky plane: flushes the image
    // cache and shows statistics if it is being used. DOES NOT
    // DO THE FINAL TRANSFORM!
    virtual void finalizeToSky();
    
    virtual void initVisBuffer(VisBuffer& vb, Type whichVBColumn);
    void initVisBuffer(VisBuffer& vb, Type whichVBColumn, Int row);

    // Get actual coherence from grid by degridding
    void get(VisBuffer& vb, Int row=-1);
    
    // Get the coherence from grid return it in the degrid 
    // is used especially when scratch columns are not 
    // present in ms.
    void get(VisBuffer& vb, Cube<Complex>& degrid, 
	     Array<Complex>& griddedVis, Vector<Double>& scale, 
	     Int row=-1);
    
    void get(VisBuffer& vb, Cube<Float>& pointingOffsets, Int row=-1,
	     Type whichVBColumn=FTMachine::MODEL,Int Conj=0)
    {
      get(vb,vb,vb,pointingOffsets,row,whichVBColumn,whichVBColumn,Conj,0);
    }
    
    void get(VisBuffer& vb, VisBuffer& gradAzVB,VisBuffer& gradElVB,
	     Cube<Float>& pointingOffsets,Int row=-1,
	     Type whichVBColumn=FTMachine::MODEL,
	     Type whichGradVBColumn=FTMachine::MODEL,
	     Int Conj=0, Int doGrad=1) ;
  void nget(VisBuffer& vb,
	    // These offsets should be appropriate for the VB
	    Array<Float>& l_off, Array<Float>& m_off,
	    Cube<Complex>& Mout,
	    Cube<Complex>& dMout1,
	    Cube<Complex>& dMout2,
	    Int Conj=0, Int doGrad=1);
    // Get the coherence from grid return it in the degrid 
    // is used especially when scratch columns are not 
    // present in ms.
    void get(VisBuffer& vb, Cube<Complex>& degrid, 
	     Array<Complex>& griddedVis, Vector<Double>& scale, 
	     Cube<Float>& pointingOffsets,Int row=-1);
    
    
    
    
    
    
    // Put coherence to grid by gridding.
    void put(const VisBuffer&,
	     TempImage<Complex>&, Vector<Double>&, int,
	     UVWMachine*, Bool) 
    {
      //    throw(AipsError("nPBWProjectFT::put is not implemented"));
    }
    void put(const VisBuffer& vb, Int row=-1, Bool dopsf=False,
	     FTMachine::Type type=FTMachine::OBSERVED,
	     const Matrix<Float>& wgt=Matrix<Float>(0,0));
    
    // Make the entire image
    void makeImage(FTMachine::Type type,
		   VisSet& vs,
		   ImageInterface<Complex>& image,
		   Matrix<Float>& weight);
    
    // Get the final image: do the Fourier transform and
    // grid-correct, then optionally normalize by the summed weights
    virtual ImageInterface<Complex>& getImage(Matrix<Float>&, Bool normalize=True);
    
    // Get the final weights image
    void getWeightImage(ImageInterface<Float>&, Matrix<Float>&);
    
    // Save and restore the nPBWProjectFT to and from a record
    Bool toRecord(String& error, RecordInterface& outRec, 
		  Bool withImage=False);
    Bool fromRecord(String& error, const RecordInterface& inRec);
    
    // Can this FTMachine be represented by Fourier convolutions?
    Bool isFourier() {return True;}
    
    //  Bool changed(const VisBuffer& vb) {return vpSJ->changed(vb,1);};
    Bool changed(const VisBuffer& vb) {return False;}
    
    virtual Int findPointingOffsets(const VisBuffer&, Array<Float>&, Array<Float>&,
				    Bool Evaluate=True);
    virtual Int findPointingOffsets(const VisBuffer&, Cube<Float>&,
			    Array<Float>&, Array<Float>&,
			    Bool Evaluate=True);
    MDirection::Convert makeCoordinateMachine(const VisBuffer&,
					      const MDirection::Types&,
					      const MDirection::Types&,
					      MEpoch& last);
    /*
      void makePB(const VisBuffer& vb,
      TempImage<Float>& PB,
      IPosition& shape,CoordinateSystem& coord);
      
      void makeAveragePB(const VisBuffer& vb, 
      const ImageInterface<Complex>& image,
      Int& polInUse,
      TempImage<Float>& PB,
      TempImage<Float>& avgPB);
    */
    virtual Bool makeAveragePB0(const VisBuffer& vb, 
		       const ImageInterface<Complex>& image,
		       Int& polInUse,
		       TempImage<Float>& avgPB);
    /*
    void makeAveragePB(const VisBuffer& vb, 
		       const ImageInterface<Complex>& image,
		       Int& polInUse,
		       TempImage<Float>& avgPB);
    */
    void makeConjPolMap(const VisBuffer& vb, const Vector<Int> cfPolMap, Vector<Int>& conjPolMap);
    //    Vector<Int> makeConjPolMap(const VisBuffer& vb);
    void makeCFPolMap(const VisBuffer& vb, Vector<Int>& polM);
    //    void reset() {vpSJ->reset();}
    void reset() {paChangeDetector.reset();}

    void setPAIncrement(const Quantity &paIncrement);

    Vector<Int>& getPolMap() {return polMap;};
    virtual String name(){ return "PBWProjectFT";};
    virtual Bool verifyAvgPB(ImageInterface<Float>& pb, ImageInterface<Float>& sky)
    {return verifyShapes(pb.shape(),sky.shape());}
    virtual Bool verifyAvgPB(ImageInterface<Float>& pb, ImageInterface<Complex>& sky)
    {return verifyShapes(pb.shape(),sky.shape());}
    virtual Bool verifyShapes(IPosition shape0, IPosition shape1);
    Bool findSupport(Array<Complex>& func, Float& threshold, Int& origin, Int& R);
    void makeAntiAliasingOp(Vector<Complex>& val, const Int len);
    void makeAntiAliasingCorrection(Vector<Complex>& correction, 
				    const Vector<Complex>& op, 
				    const Int nx);
    void applyAntiAliasingOp(ImageInterface<Complex>& cf, 
			     Vector<IPosition>& offset,
			     Int op=0, 
			     Bool Square=False);
    void applyAntiAliasingOp(ImageInterface<Float>& cf, 
			     Vector<IPosition>& offset,
			     Int op=0, 
			     Bool Square=False);
    void correctAntiAliasing(Lattice<Complex>& cf);
  protected:
    
    // Padding in FFT
    Float padding_p;
    
    Int nint(Double val) {return Int(floor(val+0.5));};
    
    // Make the PB part of the convolution function
    Int makePBPolnCoords(//const ImageInterface<Complex>& image,
			 CoordinateSystem& coord, const VisBuffer& vb);
    // Locate convolution functions on the disk
    Int locateConvFunction(Int Nw, Int polInUse, const VisBuffer& vb, Float &pa);
    void cacheConvFunction(Int which, Array<Complex>& cf, CoordinateSystem& coord);
    // Find the convolution function
    void findConvFunction(const ImageInterface<Complex>& image,
			  const VisBuffer& vb);
    void makeConvFunction(const ImageInterface<Complex>& image,
			  const VisBuffer& vb, Float pa);
    
    Int nWPlanes_p;
    
    // Get the appropriate data pointer
    Array<Complex>* getDataPointer(const IPosition&, Bool);
    
    void ok();
    
    void init();
    //    Int getVisParams();
    Int getVisParams(const VisBuffer& vb);
    // Is this record on Grid? check both ends. This assumes that the
    // ends bracket the middle
    Bool recordOnGrid(const VisBuffer& vb, Int rownr) const;
    
    // Image cache
    LatticeCache<Complex> * imageCache;
    
    // Sizes
    Long cachesize;
    Int tilesize;
    
    // Gridder
    ConvolveGridder<Double, Complex>* gridder;
    
    // Is this tiled?
    Bool isTiled;
    
    // Array lattice
    //Lattice<Complex> * arrayLattice;
    CountedPtr<Lattice<Complex> > arrayLattice;
    
    // Lattice. For non-tiled gridding, this will point to arrayLattice,
    //  whereas for tiled gridding, this points to the image
    //Lattice<Complex>* lattice;
    CountedPtr<Lattice<Complex> > lattice;
    
    Float maxAbsData;
    
    // Useful IPositions
    IPosition centerLoc, offsetLoc;
    
    // Image Scaling and offset
    Vector<Double> uvScale, uvOffset;
    
    // Array for non-tiled gridding
    Array<Complex> griddedData;
    
    // Pointing columns
    MSPointingColumns* mspc;
    
    // Antenna columns
    MSAntennaColumns* msac;
    
    DirectionCoordinate directionCoord;
    
    MDirection::Convert* pointingToImage;
    
    Vector<Double> xyPos;
    
    MDirection worldPosMeas;
    
    Int priorCacheSize;
    
    // Grid/degrid zero spacing points?
    Bool usezero_p;
    
    Array<Complex> convFunc;
    Array<Complex> convWeights;
    CoordinateSystem convFuncCS_p;
    Int convSize;
    //
    // Vector to hold the support size info. for the convolution
    // functions pointed to by the elements of convFunctions_p.  The
    // co-ordinates of this array are (W-term, Poln, PA).
    //
    Int convSampling;
    Cube<Int> convSupport, convWtSupport;
    //
    // Holder for the pointers to the convolution functions. Each
    // convolution function itself is a complex 3D array (U,V,W) per
    // PA.
    //
    PtrBlock < Array<Complex> *> convFuncCache, convWeightsCache;
    //    Array<Complex>* convFunc_p;

    //
    // The index into the conv. func. cache for the current conv. func.
    // 
    Int PAIndex;
    //
    // If true, all convolution functions are in the cache.
    //
    Bool convFuncCacheReady;

    Int wConvSize;
    
    Int lastIndex_p;
    
    Int getIndex(const ROMSPointingColumns& mspc, const Double& time,
		 const Double& interval);
    
    Bool getXYPos(const VisBuffer& vb, Int row);
    //    VPSkyJones *vpSJ;
    //
    // The PA averaged (and potentially antenna averaged) PB for
    // normalization
    //
    TempImage<Float> avgPB;
    //
    // No. of vis. polarization planes used in making the user defined
    // Stokes images
    //
    Int polInUse, bandID_p;
    Int maxConvSupport;
    //
    // Percentage of the peak of the PB after which the image is set
    // to zero.
    //
    Float pbLimit_p;

    EPJones *epJ;
    Double HPBW, Diameter_p, sigma;
    Int Nant_p;
    Int doPointing;
    Bool doPBCorrection;
    Bool makingPSF;
    
    Unit Second, Radian, Day;
    Array<Float> l_offsets,m_offsets;
    Int noOfPASteps;
    Vector<Float> pbPeaks;
    Bool pbNormalized,resetPBs;
    Vector<Float> paList;
    ConvFuncDiskCache cfCache;
    Double currentCFPA;
    ParAngleChangeDetector paChangeDetector;
    Vector<Int> cfStokes;
    Vector<Complex> Area;
    Double cfRefFreq_p;
    Bool avgPBSaved;
    Bool avgPBReady;
    Vector<Complex> antiAliasingOp,antiAliasingCorrection;
    //    VLACalcIlluminationConvFunc vlaPB;
    //
    //----------------------------------------------------------------------
    //
    virtual void normalizeAvgPB();
    virtual void runFortranGet(Matrix<Double>& uvw,Vector<Double>& dphase,
		       Cube<Complex>& visdata,
		       IPosition& s,
		       //Cube<Complex>& gradVisAzData,
		       //Cube<Complex>& gradVisElData,
		       //IPosition& gradS,
		       Int& Conj,
		       Cube<Int>& flags,Vector<Int>& rowFlags,
		       Int& rownr,Vector<Double>& actualOffset,
		       Array<Complex>* dataPtr,
		       Int& aNx, Int& aNy, Int& npol, Int& nchan,
		       VisBuffer& vb,Int& Nant_p, Int& scanNo,
		       Double& sigma,
		       Array<Float>& raoffsets,
		       Array<Float>& decoffsets,
		       Double area,
		       Int& doGrad,Int paIndex);
    virtual void runFortranPut(Matrix<Double>& uvw,Vector<Double>& dphase,
		       const Complex& visdata_p,
		       IPosition& s,
		       //Cube<Complex>& gradVisAzData,
		       //Cube<Complex>& gradVisElData,
		       //IPosition& gradS,
		       Int& Conj,
		       Cube<Int>& flags,Vector<Int>& rowFlags,
		       const Matrix<Float>& weight,
		       Int& rownr,Vector<Double>& actualOffset,
		       Array<Complex>& dataPtr,
		       Int& aNx, Int& aNy, Int& npol, Int& nchan,
		       const VisBuffer& vb,Int& Nant_p, Int& scanNo,
		       Double& sigma,
		       Array<Float>& raoffsets,
		       Array<Float>& decoffsets,
		       Matrix<Double>& sumWeight,
		       Double& area,
		       Int& doGrad,
		       Int& doPSF,Int paIndex);
  void runFortranGetGrad(Matrix<Double>& uvw,Vector<Double>& dphase,
			 Cube<Complex>& visdata,
			 IPosition& s,
			 Cube<Complex>& gradVisAzData,
			 Cube<Complex>& gradVisElData,
			 //			 IPosition& gradS,
			 Int& Conj,
			 Cube<Int>& flags,Vector<Int>& rowFlags,
			 Int& rownr,Vector<Double>& actualOffset,
			 Array<Complex>* dataPtr,
			 Int& aNx, Int& aNy, Int& npol, Int& nchan,
			 VisBuffer& vb,Int& Nant_p, Int& scanNo,
			 Double& sigma,
			 Array<Float>& l_off,
			 Array<Float>& m_off,
			 Double area,
			 Int& doGrad,
			 Int paIndex);
  };
  
  //void saveImmage(TempImage<Complex>& convFunc, Int wConvSize);
} //# NAMESPACE CASA - END

#endif
