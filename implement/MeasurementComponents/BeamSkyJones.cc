//# BeamSkyJones.cc: Implementation for BeamSkyJones
//# Copyright (C) 1996,1997,1998,1999,2000,2001,2002,2003
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

#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>

#include <ms/MeasurementSets/MeasurementSet.h>
#include <ms/MeasurementSets/MSColumns.h>
#include <ms/MeasurementSets/MSObsColumns.h>
#include <ms/MeasurementSets/MSSpWindowColumns.h>
#include <tables/Tables.h>
#include <measures/Measures/Stokes.h>
#include <measures/Measures/MeasConvert.h>

#include <casa/BasicSL/Constants.h>

#include <components/ComponentModels/Flux.h>
#include <components/ComponentModels/ComponentShape.h>

#include <synthesis/MeasurementComponents/SkyModel.h>
#include <synthesis/MeasurementComponents/BeamSkyJones.h>
#include <synthesis/MeasurementComponents/PBMath.h>

#include <msvis/MSVis/VisBuffer.h>

#include <images/Images/ImageInterface.h>
#include <images/Images/ImageRegion.h>

#include <casa/Utilities/Assert.h>

/*
// temporary, for debugging
#include <casa/Quanta/MVAngle.h>
void printDirection(std::ostream &os,const casa::MDirection &dir) throw (casa::AipsError) {
  double lngbuf=dir.getValue().getLong("deg").getValue();
  if (lngbuf<0) lngbuf+=360.;
  os<<(dir.getRefString()!="GALACTIC"?casa::MVAngle::Format(casa::MVAngle::TIME):
  casa::MVAngle::Format(casa::MVAngle::ANGLE))<<casa::MVAngle(casa::Quantity(lngbuf,"deg"))<<" "
    <<casa::MVAngle(dir.getValue().getLat("deg"))<<
    " ("<<dir.getRefString()<<")";
}
//
*/

namespace casa { //# NAMESPACE CASA - BEGIN

BeamSkyJones::BeamSkyJones(MeasurementSet& ms, 
			   const Quantity &parallacticAngleIncrement,
			   BeamSquint::SquintType doSquint,
			   const Quantity &skyPositionThreshold) :
     ms_p(&ms), doSquint_p(doSquint),
     parallacticAngleIncrement_p(parallacticAngleIncrement.getValue("rad")),
     skyPositionThreshold_p(skyPositionThreshold.getValue("rad")),
     lastUpdateVisBuffer_p(NULL), lastUpdateRow_p(-1),
     lastUpdateIndex1_p(-1), lastUpdateIndex2_p(-1), hasBeenApplied(False)
     
{  
  reset();
  setThreshold(0.01); // use this in apply to determine level of cutoff
};

void BeamSkyJones::reset()
{
  lastFieldId_p=-1;
  lastArrayId_p=-1;
  lastMSId_p=0;  
  telescope_p=" ";
}

BeamSkyJones::~BeamSkyJones()
{
};

void BeamSkyJones::showState(LogIO& os)
{
  os << "Field ID    = " << lastFieldId_p+1 << LogIO::POST;
  os << "Telescope   = " << telescope_p << LogIO::POST;
  for (uInt i=0;i<lastParallacticAngles_p.nelements();++i) {
       os << "ParAngle[d] ("<<i<<" model) = " <<
            lastParallacticAngles_p[i]/C::pi*180.<< LogIO::POST;
       os << "Pointing direction ("<<i<<" model) = "<<
             lastDirections_p[i].getAngle().getValue("deg")(0) <<
    ", " <<  lastDirections_p[i].getAngle().getValue("deg")(1) << LogIO::POST;
  }
  os << "delta PA[d] = " << Quantity(parallacticAngleIncrement_p,"rad").getValue("deg") << LogIO::POST;
  os << "skyPositionThreshold[d] = " << Quantity(skyPositionThreshold_p,"rad").getValue("deg") << LogIO::POST;
  os << "SquintType  = " << (Int)(doSquint_p) << LogIO::POST;
};

// update the indices cache. This method could be made protected in the
// future (need access functions for lastUpdateIndex?_p to benefit from it)
// Cache will be valid for a given VisBuffer and row
void BeamSkyJones::updatePBMathIndices(const VisBuffer &vb, Int row) const
{
  // for debug
  // cout<<endl<<"BeamSkyJones::updatePBMathIndices row="<<row<<endl<<endl;
  //

  // we will not check msId, arrayId and fieldID as they are supposed to be
  // checked before this method is called. However, if this method is to
  // be made protected, a change may be required here to avoid nasty
  // surprises in the future.

  if (&vb==lastUpdateVisBuffer_p && row==lastUpdateRow_p) return;

  lastUpdateVisBuffer_p=&vb;
  lastUpdateRow_p=row;

  // Getting the reference on antennae/feed IDs is a
  // fast operation as caching is implemented inside VisBuffer.
  DebugAssert(row<(Int)vb.antenna1().nelements(),AipsError);
  Int ant1=vb.antenna1()[row];
  Int ant2=vb.antenna2()[row];
  Int feed1=vb.feed1()[row];
  Int feed2=vb.feed2()[row];  

  // telescope_p should be valid at this stage because it is updated
  // after each ArrayID change. Care must be taken if the method is to be
  // made protected.
  lastUpdateIndex1_p=indexTelescope(telescope_p,ant1,feed1);
  lastUpdateIndex2_p=indexTelescope(telescope_p,ant2,feed2);
}

Bool BeamSkyJones::changed(const VisBuffer& vb, Int row)
{
  // for debug
  // cout<<endl<<"BeamSkyJones::changed row="<<row<<" lastUpdateRow_p="<<
  //         lastUpdateRow_p<<endl<<endl;  
  //
  
  if (row < 0) row = 0;
  
  if(vb.msId() != lastMSId_p || vb.arrayId()!=lastArrayId_p ||
     vb.fieldId()!=lastFieldId_p)  {
        lastUpdateVisBuffer_p=NULL; // invalidate index cache
        return True;
  }
  
  //if (lastUpdateIndex1_p<0 || lastUpdateIndex2_p<0) return True;
  
  updatePBMathIndices(vb,row); // lastUpdateIndex?_p are now valid

  //Unnecessary ...i believe and causes issues with PSF making
  //if (!hasBeenApplied) return True; // we shouldn't have such a flag in
                 // a well designed code

  if (!lastParallacticAngles_p.nelements() && myPBMaths_p.nelements())
       return True; // it's a first call of this method and setPBMath has
                    // definitely been called before
  
  // Obtaining a reference on parallactic angles is a fast operation as
  // caching is implemented inside VisBuffer.
  Float feed1_pa=vb.feed1_pa()[row];
  Float feed2_pa=vb.feed2_pa()[row];  

  // it may be good to check here whether an indexed beam model
  // depend on parallactic angle before returning True
  // An additional interface function may be required for PBMath classes

  if (lastUpdateIndex1_p!=-1)
      if (abs(feed1_pa-lastParallacticAngles_p[lastUpdateIndex1_p]) >
              parallacticAngleIncrement_p) return True;

  if (lastUpdateIndex2_p!=-1)
      if (abs(feed2_pa-lastParallacticAngles_p[lastUpdateIndex2_p]) >
              parallacticAngleIncrement_p) return True;
	      
  if (lastUpdateIndex1_p!=-1)
      if (!directionsCloseEnough(lastDirections_p[lastUpdateIndex1_p],
                     vb.direction1()[row])) return True;

  if (lastUpdateIndex2_p!=-1)
      if (!directionsCloseEnough(lastDirections_p[lastUpdateIndex2_p],
                     vb.direction2()[row])) return True;    
  return False;
};

// return True if two directions are close enough to consider the
// operator unchanged, False otherwise
Bool BeamSkyJones::directionsCloseEnough(const MDirection &dir1,
                           const MDirection &dir2) const throw(AipsError)
{
  Double sep; 
  if (dir1.getRef()!=dir2.getRef())
      sep=dir1.getValue().separation(MDirection::Convert(dir2.getRef(),
              dir1.getRef())(dir2).getValue());
  else sep=dir1.getValue().separation(dir2.getValue());
  return (fabs(sep)<skyPositionThreshold_p);
}

// Does this BeamSkyJones change during this buffer, starting from
// row1?  If row2 <0, row2 = nRow()-1
Bool BeamSkyJones::changedBuffer(const VisBuffer& vb, Int row1, Int& row2)
{
  Int irow = row1;
  if (irow < 0) irow = 0;
  Int jrow = row2;
  if (jrow < 0) jrow = vb.nRow()-1;
  DebugAssert(jrow<vb.nRow(),AipsError);

  // It is not important now to have a separate function for a "block"
  // operation. Because an appropriate caching is implemented inside
  // both VisBuffer and this class, changed(vb,row) can be called in the
  // loop without a perfomance penalty. We preserve this function to keep
  // the interface intact.

  for (Int ii=irow+1;ii<=jrow;++ii)
       if (changed(vb,ii)) {
           row2 = ii-1;
	   return True;
       }
  return False;
};

// as it is stated in BeamSkyJones.h this method may not be useful
// anymore. Implementing it just in case it is used somewhere.
// Because an appropriate caching is implemented in both VisBuffer and
// BeamSkyJones, this method can use BeamSkyJones::changed in a loop
Bool BeamSkyJones::change(const VisBuffer& vb)
{
  for (Int i=0;i<vb.nRow();++i)
       if (changed(vb,i)) return True;
  return False;
};

void BeamSkyJones::update(const VisBuffer& vb, Int row)
{
  // for debug
  //cout<<endl<<"BeamSkyJones::update nrow="<<vb.nRow()<<" row="<<row<<" feed1="<<vb.feed1()(0)<<" feed2="<<vb.feed2()(0)<<endl<<endl;
  //
  
  if (row<0) row=0;
  
  lastFieldId_p=vb.fieldId();
  lastArrayId_p=vb.arrayId();
  lastMSId_p=vb.msId();

  // The pointing direction depends on feed, antenna, pointing errors, etc  
  MDirection pointingDirection1_p = vb.direction1()(row);
  MDirection pointingDirection2_p = vb.direction2()(row);
    
  // Look up correct telescope
  const ROMSObservationColumns& msoc=vb.msColumns().observation();
  telescope_p = msoc.telescopeName()(vb.arrayId());

  updatePBMathIndices(vb,row); // lastUpdateIndex?_p are now valid

  if (!lastParallacticAngles_p.nelements() && myPBMaths_p.nelements()) {
       lastParallacticAngles_p.resize(myPBMaths_p.nelements());
       lastParallacticAngles_p.set(1000.); // to force recalculation
                           // it will recalculate directions as well
  }
  if (!lastDirections_p.nelements() && myPBMaths_p.nelements()) 
       lastDirections_p.resize(myPBMaths_p.nelements());
       
  if (lastUpdateIndex1_p == lastUpdateIndex2_p &&
      !directionsCloseEnough(pointingDirection1_p,pointingDirection2_p)) {
        // the case is inhomogeneous: pointing directions are slightly
	// different at different antennae
    //This check is an overkill for standard arrays...need to find a better one

    /*	LogIO os;
	os << LogIO::WARN << LogOrigin("BeamSkyJones","update")
	   << "The pointing directions differ for different stations."
	   << LogIO::POST << LogIO::WARN << LogOrigin("BeamSkyJones","update")
	   << "This case is not handled correctly. Continuing anyway."<<LogIO::POST;

    */
	// we could, in principle, clone a PBMath object for one of the
	// antennae and rebuild lastDirections_p.
	// For now, the value for the second antenna will be used
  }
  
  if (lastUpdateIndex1_p!=-1)
      lastDirections_p[lastUpdateIndex1_p]=pointingDirection1_p;

  if (lastUpdateIndex2_p!=-1)
      lastDirections_p[lastUpdateIndex2_p]=pointingDirection2_p;
  
  // Obtaining a reference on parallactic angles is a fast operation as
  // caching is implemented inside VisBuffer.
  Float feed1_pa=vb.feed1_pa()[row];
  Float feed2_pa=vb.feed2_pa()[row];  

  if (lastUpdateIndex1_p == lastUpdateIndex2_p &&
      abs(abs(feed1_pa-feed2_pa)-parallacticAngleIncrement_p)> 1e-5 ) {
      // the array is not compact: position angles are significantly
      // different at different antennae
      LogIO os;
      //Commenting out this message...more pest than  useful to have it at this low level
      //    os << LogIO::WARN << LogOrigin("BeamSkyJones","update")
      //       << "The array is not compact, position angles differ for different stations."
      //     << LogIO::POST << LogIO::WARN << LogOrigin("BeamSkyJones","update")
      //      << "Primary beams are not correctly handled if they are asymmetric. Continuing anyway."<<LogIO::POST;
    // we could, in principle, clone a PBMath object for one of the
    // antennae and rebuild lastParallacticAngles_p.
    // For now, the value for the second antenna will be used
  }
  if (lastUpdateIndex1_p!=-1)
      lastParallacticAngles_p[lastUpdateIndex1_p]=feed1_pa;
  if (lastUpdateIndex2_p!=-1)
      lastParallacticAngles_p[lastUpdateIndex2_p]=feed2_pa;
};

void BeamSkyJones::assure (const VisBuffer& vb, Int row)
{
  if(changed(vb, row)) update(vb, row);  
};


ImageInterface<Complex>& 
BeamSkyJones::apply(const ImageInterface<Complex>& in,
		    ImageInterface<Complex>& out,
		    const VisBuffer& vb, Int row,
		    Bool forward)
{
  if(changed(vb, row)) update(vb, row);
  hasBeenApplied=True;
  // now lastUpdateIndex?_p are valid
  
  if (lastUpdateIndex1_p!=lastUpdateIndex2_p) 
      throw(AipsError("BeamSkyJones::apply(Image...) - can only treat homogeneous PB case"));
  else {
    // for debug
    // cout<<endl<<"BeamSkyJones::apply(Image...) index="<<lastUpdateIndex1_p<<" feed="<<vb.feed1()(0)<<" direction=";
    // printDirection(cout,lastDirections_p[lastUpdateIndex1_p]); cout<<endl<<endl;
    //
    PBMath myPBMath;    
    if (getPBMath(lastUpdateIndex1_p, myPBMath)) 
      return myPBMath.applyPB(in, out, lastDirections_p[lastUpdateIndex1_p], 
	      Quantity(lastParallacticAngles_p[lastUpdateIndex1_p],"rad"),
              doSquint_p, False, threshold(), forward);
    else 
      throw(AipsError("BeamSkyJones::apply(Image...)!!! - PBMath not found"));
  }
}; 

ImageInterface<Float>& 
BeamSkyJones::applySquare(const ImageInterface<Float>& in,
			  ImageInterface<Float>& out,
			  const VisBuffer& vb, Int row)
{
  if(changed(vb, row)) update(vb, row);
  hasBeenApplied=True;
  // now lastUpdateIndex?_p are valid
  
  if (lastUpdateIndex1_p!=lastUpdateIndex2_p)   
    throw(AipsError("BeamSkyJones::applySquare(Image...) - can only treat homogeneous PB case"));
  else {
    // for debug
    // cout<<endl<<"BeamSkyJones::applySquare(Image...) index="<<lastUpdateIndex1_p<<" feed="<<vb.feed1()(0)<<" direction=";
    // printDirection(cout,lastDirections_p[lastUpdateIndex1_p]); cout<<endl<<endl;
    //
    PBMath myPBMath;
    if (getPBMath(lastUpdateIndex1_p, myPBMath)) 
      return myPBMath.applyPB2(in, out, lastDirections_p[lastUpdateIndex1_p],
           lastParallacticAngles_p[lastUpdateIndex1_p], doSquint_p, threshold()*threshold());
    else 
      throw(AipsError("BeamSkyJones::applySquare(Image...) - PBMath not found"));    
  }
}; 


SkyComponent& 
BeamSkyJones::apply(SkyComponent& in,
		    SkyComponent& out,
		    const VisBuffer& vb, Int row,
		    Bool forward)
{
  if(changed(vb, row)) update(vb, row);
  hasBeenApplied=True;
  // now lastUpdateIndex?_p are valid
  
  if (lastUpdateIndex1_p!=lastUpdateIndex2_p)
    throw(AipsError("BeamSkyJones::apply(SkyComp...) - can only treat homogeneous PB case"));
  else { 
    // for debug
    // cout<<endl<<"BeamSkyJones::apply(SkyComp...) index="<<lastUpdateIndex1_p<<" feed="<<vb.feed1()(0)<<" direction=";
    // printDirection(cout,lastDirections_p[lastUpdateIndex1_p]); cout<<endl<<endl;
    //
    PBMath myPBMath;
    if (getPBMath(lastUpdateIndex1_p, myPBMath)) 
      return myPBMath.applyPB(in, out, lastDirections_p[lastUpdateIndex1_p], 
			      Quantity(vb.frequency()(0), "Hz"), 
			      lastParallacticAngles_p[lastUpdateIndex1_p],
			      doSquint_p, False, threshold(), forward);
      else 
      throw(AipsError("BeamSkyJones::apply(SkyComponent,...) - PBMath not found"));    
  }
}; 


SkyComponent& 
BeamSkyJones::applySquare(SkyComponent& in,
		    SkyComponent& out,
		    const VisBuffer& vb, Int row)
{
  if(changed(vb, row)) update(vb, row);
  hasBeenApplied=True;
  // now lastUpdateIndex?_p are valid
  
  if (lastUpdateIndex1_p!=lastUpdateIndex2_p)   
    throw(AipsError("BeamSkyJones::applySquare(SkyComponent,...) - can only treat homogeneous PB case"));
  else { 
    PBMath myPBMath;
    if (getPBMath(lastUpdateIndex1_p, myPBMath))
      return myPBMath.applyPB2(in, out, lastDirections_p[lastUpdateIndex1_p], 
			       Quantity(vb.frequency()(0), "Hz"), 
			       lastParallacticAngles_p[lastUpdateIndex1_p],
			       doSquint_p);
    else 
      throw(AipsError("BeamSkyJones::applySquare(SkyComponent,...) - PBMath not found"));
  }
}; 

// Apply gradient
ImageInterface<Complex>&
BeamSkyJones::applyGradient(ImageInterface<Complex>& result,
			  const VisBuffer&,
			  Int)
{  
  return result;
};


void BeamSkyJones::initializeGradients()
{
};

void BeamSkyJones::finalizeGradients()
{
};


SkyComponent&
BeamSkyJones::applyGradient(SkyComponent& result, const VisBuffer&,
			  Int) 
{  
  return result;
};

void BeamSkyJones::addGradients(const VisBuffer& vb, Int row,
			      const Float sumwt, 
			      const Float chisq,
			      const Matrix<Complex>& c, 
			      const Matrix<Float>& f) 
{
  // Keep compiler quiet
  if(&vb) {};
  if(row) {};
  if(sumwt) {};
  if(chisq) {};
  if(&c) {};
  if(&f) {};
};

// Solve
Bool BeamSkyJones::solve (SkyEquation& se) 
{
  // Keep compiler quiet
  if(&se) {};
  return False;
};

// return index of compareTelescope, compareAntenna and compareFeed in
// myTelescopes_p; -1 if not found
// if compareAntenna or compareTelescope is -1, this means that the
// PBMath class is the same for all antennae/feeds. An exception will
// be raised, if separate PBMath objects have been assigned by setPBMath
// for different feeds/antennae but -1 is used for query.
//
// It would be good to rename this function to indexBeams as this name
// is more appropriate. 
//
Int BeamSkyJones::indexTelescope(const String &compareTelescope,
                     const Int &compareAntenna, const Int &compareFeed) const
{
  // for debugging
  //cout<<endl<<"BeamSkyJones::indexTelescope tel="<<compareTelescope<<" ant="<<compareAntenna<<" feed="<<compareFeed<<endl<<endl;
  //cout<<"Currently "<<myTelescopes_p.nelements()<<" models have been set"<<endl;
  //for (uInt i=0; i<myTelescopes_p.nelements(); ++i) 
  //     cout<<i<<" telescope: "<<myTelescopes_p[i]<<" ant:" <<
  //         myAntennaIDs_p[i]<<" feed: "<<myFeedIDs_p[i]<<endl;
  //	     
	     
  DebugAssert(myTelescopes_p.nelements()==myAntennaIDs_p.nelements(),
              AipsError);
  DebugAssert(myTelescopes_p.nelements()==myFeedIDs_p.nelements(),
              AipsError);	      
  for (uInt i=0; i<myTelescopes_p.nelements(); ++i) 
       if (myTelescopes_p[i] == compareTelescope) {
           if (compareAntenna==myAntennaIDs_p[i] &&
	       compareFeed==myFeedIDs_p[i]) return i; // -1 will also work
           if (compareAntenna==myAntennaIDs_p[i]) {
	       if (compareFeed==-1)
	           throw AipsError("BeamSkyJones::indexTelescope: separate beam models"
	              "have been set up for different feeds and a common model is requested");
               if (myFeedIDs_p[i]==-1) return i; // valid for all feeds and a given antenna		      
           }
           if (compareFeed==myFeedIDs_p[i]) {
	       if (compareAntenna==-1)
	           throw AipsError("BeamSkyJones::indexTelescope: separate beam models"
	               "have been set up for different antennae and a common model is requested");
	       if (myAntennaIDs_p[i]==-1) return i; // valid for all antennae and a given feed       
           }
	   if (myFeedIDs_p[i]==-1 && myAntennaIDs_p[i]==-1)
	       return i; // valid for all antennae and feeds
           
       }  
  return -1;
};

// get the PBMath object; returns False if that one doesn't exist,
// True if it does exist and is OK
// antennaID and feedID default to -1 to preserve the old interface
// TODO: change the interface and make antennaID and feedID the
// second and third parameter respectively to have a better looking code

Bool BeamSkyJones::getPBMath(const String &telescope, PBMath &myPBMath,
                 const Int &antennaID, const Int &feedID) const
{
  Int indTel = indexTelescope(telescope,antennaID,feedID);
  if (indTel >= 0) 
    return getPBMath((uInt)indTel, myPBMath);
   else 
    return False;  // PBMath not found for this telescope/antenna/feed combination
  
};

Bool BeamSkyJones::getPBMath(uInt whichAnt, PBMath &myPBMath) const
{
  if (whichAnt <  myPBMaths_p.nelements() && whichAnt>=0) {
    if (myPBMaths_p[whichAnt].ok()) {
      myPBMath = myPBMaths_p[whichAnt];
      return True;
    } else 
      return False;  // whichAnt's PBMath found but not valid    
  } else 
    return False;  // whichAnt's PBMath not found
  
};

// set the PB based on telescope name, antennaID and feedID
// If antennaID or feedID is -1, the PBMath object is set for
// all antennae or feeds, respectively. These are the default
// values to retain the previous interface.
//
// Note. It would be nice to change the interface and make antennaID
// and feedID the second and the third parameter, respectively.
void BeamSkyJones::setPBMath(const String &telescope, PBMath &myPBMath,
                const Int &antennaID, const Int &feedID)
{
   // for debug
   // cout<<endl<<"BeamSkyJones::setPBMath tel="<<telescope<<" ant="<<antennaID<<" feed="<<feedID<<endl<<endl;
   //
   
   DebugAssert(myTelescopes_p.nelements()==myAntennaIDs_p.nelements(),
               AipsError);
   DebugAssert(myTelescopes_p.nelements()==myFeedIDs_p.nelements(),
               AipsError);	      
   DebugAssert(myTelescopes_p.nelements()==myPBMaths_p.nelements(),
               AipsError);

   Bool doRemove=False;
   if (antennaID==-1 || feedID==-1) 
     // we have to remove PBMaths for individual antennae/feeds, if they     
     // were assigned earlier
     for (uInt i=0; i<myTelescopes_p.nelements(); ++i) {
          if (doRemove) {
	      // we have to step back because the previous element
	      // has been removed
	      --i;
	      doRemove=False;
	      DebugAssert(i<myTelescopes_p.nelements(), AipsError);
	  }
          if (myTelescopes_p[i] == telescope) {	      
	      if (myAntennaIDs_p[i]==-1 && antennaID==-1 &&
	          myFeedIDs_p[i]==-1 && feedID==-1)
		     continue;  // to speed things up
	      if (myAntennaIDs_p[i]!=-1 && antennaID==-1)
	          if (myFeedIDs_p[i]!=-1) myAntennaIDs_p[i]=-1;
		      // now it's valid for all antennae and a given feed
		      // and will be replaced later
		  else doRemove=True;
              if (myFeedIDs_p[i]!=-1 && feedID==-1)
	          if (myAntennaIDs_p[i]!=-1) myFeedIDs_p[i]=-1;
		      // now it's valid for all feeds at a given antenna
		      // and will be replaced later
                  else doRemove=True;
              if (doRemove) {
	          myTelescopes_p.remove(i,False);
	          myAntennaIDs_p.remove(i,False);
		  myFeedIDs_p.remove(i,False);
		  myPBMaths_p.remove(i,False);
		  if (lastParallacticAngles_p.nelements())
		      lastParallacticAngles_p.remove(i,False);
                  if (lastDirections_p.nelements())
		      lastDirections_p.remove(i,False);
	      }
	  }
     }
  // there should be no exception at this stage because all bad elements
  // should be removed by the code above
  Int ind = indexTelescope(telescope,antennaID,feedID);
  if (ind < 0) {
    ind = myPBMaths_p.nelements();
    myPBMaths_p.resize(ind+1);
    myTelescopes_p.resize(ind+1);
    myTelescopes_p[ind] = telescope;
    myAntennaIDs_p.resize(ind+1);
    myAntennaIDs_p[ind] = antennaID;
    myFeedIDs_p.resize(ind+1);
    myFeedIDs_p[ind] = feedID;
    if (lastParallacticAngles_p.nelements())
        lastParallacticAngles_p.resize(ind+1);
    if (lastDirections_p.nelements())
        lastDirections_p.resize(ind+1);
  }
  myPBMaths_p[ind] = myPBMath;
  if (lastParallacticAngles_p.nelements())
      lastParallacticAngles_p[ind]=1000.; // to force
                                          // recalculation (it is >> 2pi)
};


Bool BeamSkyJones::isHomogeneous() const
{
  // Hogwash!  our "myPBMath_p/myTelescope_p scheme only deals
  // with homogeneous pointings.  Need to fix this!
  // Wait for MS-II
  return True;
};


ImageRegion*  
BeamSkyJones::extent (const ImageInterface<Complex>& im, 
		      const VisBuffer& vb,  
		      const Int row,	
		      const Float fPad,  
		      const Int iChan, 
		      const SkyJones::SizeType sizeType)
{
  if(changed(vb, row))  update(vb, row);
  DebugAssert(lastUpdateIndex1_p>=0,AipsError); // should be checked in changed/update
  
  PBMath myPBMath;
  if (getPBMath(lastUpdateIndex1_p, myPBMath)) {
    return myPBMath.extent(im, lastDirections_p[lastUpdateIndex1_p], row, fPad, iChan, sizeType);
  } else {
    throw(AipsError("BeamSkyJones::extent() - PBMath not found"));
  }   
};

ImageRegion*  
BeamSkyJones::extent (const ImageInterface<Float>& im, 
		      const VisBuffer& vb,  
		      const Int row,
		      const Float fPad,  
		      const Int iChan,
		      const SkyJones::SizeType sizeType)
{ 
  if(changed(vb, row)) update(vb, row);
  DebugAssert(lastUpdateIndex1_p>=0,AipsError); // should be checked in changed/update
  
  PBMath myPBMath;
  if (getPBMath(lastUpdateIndex1_p, myPBMath)) {
    return myPBMath.extent(im, lastDirections_p[lastUpdateIndex1_p], row, fPad, iChan, sizeType);
  } else {
    throw(AipsError("BeamSkyJones::extent() - PBMath not found"));
  }   
};

Int BeamSkyJones::support(const VisBuffer& vb, const CoordinateSystem& cs){
  PBMath myPBMath;

  if(changed(vb, 0)) update(vb, 0);
  if (getPBMath(lastUpdateIndex1_p, myPBMath)) {
    return myPBMath.support(cs);
  } else {
    throw(AipsError("BeamSkyJones::support() - PBMath not found"));
  }

}

void BeamSkyJones::summary(Int n) 
{
  uInt nPBs = myPBMaths_p.nelements();
  LogIO os(LogOrigin("BeamSkyJones", "summary"));
  os << "Beam Summary: "<< LogIO::POST;
  for (uInt i=0; i< nPBs; ++i) {
    String pbclass;
    myPBMaths_p[i].namePBClass(pbclass);
    os << "Model " << i+1 << " for " << myTelescopes_p[i] <<" ant="<<
          myAntennaIDs_p[i]<<" feed="<<myFeedIDs_p[i]<< " uses PB: "
       << pbclass << LogIO::POST;
    if (n >=0) {
      myPBMaths_p[i].summary(n);
    }
  }
};

} //# NAMESPACE CASA - END

