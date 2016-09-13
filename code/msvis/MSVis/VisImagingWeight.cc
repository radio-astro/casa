//# VisImagingWeight.cc: imaging weight calculation for a give buffer
//# Copyright (C) 2009
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
#include <msvis/MSVis/VisibilityIterator.h>
#include <msvis/MSVis/VisBuffer.h>
#include <msvis/MSVis/VisBuffer2.h>
#include <msvis/MSVis/VisImagingWeight.h>
#include <casa/Quanta/MVAngle.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/Vector.h>



namespace casa { //# NAMESPACE CASA - BEGIN

  VisImagingWeight::VisImagingWeight() : multiFieldMap_p(-1), wgtType_p("none"), doFilter_p(False), robust_p(0.0), rmode_p("norm"), noise_p(Quantity(0.0, "Jy")) {

    }

  VisImagingWeight::VisImagingWeight(const String& type) : multiFieldMap_p(-1),doFilter_p(False),  robust_p(0.0), rmode_p("norm"), noise_p(Quantity(0.0, "Jy")) {

        wgtType_p=type;
        wgtType_p.downcase();
        if (wgtType_p != "natural" && wgtType_p != "radial"){

            throw(AipsError("Programmer error...wrong constructor used"));
        }
  
    }


  VisImagingWeight::VisImagingWeight(ROVisibilityIterator& vi, const String& rmode, const Quantity& noise,
                                     const Double robust, const Int nx, const Int ny,
                                     const Quantity& cellx, const Quantity& celly,
                                     const Int uBox, const Int vBox, const Bool multiField) : multiFieldMap_p(-1), doFilter_p(False), robust_p(robust), rmode_p(rmode), noise_p(noise) {

      LogIO os(LogOrigin("VisSetUtil", "VisImagingWeight()", WHERE));

      VisBufferAutoPtr vb (vi);
      wgtType_p="uniform";
      // Float uscale, vscale;
      //Int uorigin, vorigin;
      Vector<Double> deltas;
      uscale_p=(nx*cellx.get("rad").getValue());
      vscale_p=(ny*celly.get("rad").getValue());
      uorigin_p=nx/2;
      vorigin_p=ny/2;
      nx_p=nx;
      ny_p=ny;
      // Simply declare a big matrix
      //Matrix<Float> gwt(nx,ny);
      gwt_p.resize(1);
      multiFieldMap_p.clear();
      vi.originChunks();
      vi.origin();
      
      // Detect usable WEIGHT_SPECTRUM
      Bool doWtSp=(vb->weightSpectrum().nelements()>0);

      String mapid=String::toString(vb->msId())+String("_")+String::toString(vb->fieldId());
      multiFieldMap_p.define(mapid, 0);
      gwt_p[0].resize(nx, ny);
      gwt_p[0].set(0.0);

      Int fields=0;
      for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
          for (vi.origin();vi.more();vi++) {
              if(vb->newFieldId()){
                  mapid=String::toString(vb->msId())+String("_")+String::toString(vb->fieldId());
                  if(multiField){
                      if(!multiFieldMap_p.isDefined(mapid)){
                          fields+=1;
                          gwt_p.resize(fields+1);
                          gwt_p[fields].resize(nx,ny);
                          gwt_p[fields].set(0.0);
                      }
                  }
                  if(!multiFieldMap_p.isDefined(mapid))
                      multiFieldMap_p.define(mapid, fields);
              }
          }
      }

      Float u, v;
      Vector<Double> sumwt(fields+1,0.0);
      f2_p.resize(fields+1);
      d2_p.resize(fields+1);
      Int fid=0;
      for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
          for (vi.origin();vi.more();vi++) {
              if(vb->newFieldId())
                  mapid=String::toString(vb->msId())+String("_")+String::toString(vb->fieldId());
              fid=multiFieldMap_p(mapid);
              Int nRow=vb->nRow();
              Int nChan=vb->nChannel();

	      // Extract weights correctly (use WEIGHT_SPECTRUM, if available)
	      Matrix<Float> wtm;
	      if (doWtSp)                        
		// WS available, 
		unPolChanWeight(wtm,vb->weightSpectrum());               // collapse corr axis (parallel-hand average)
	      else                                          
		// WS UNavailable
		wtm.reference(vb->weight().reform(IPosition(2,1,nRow))); // use vb.weight() (corr-collapsed, w/ 1 channel)

	      // Use this to mod the channel indexing below
	      Int nChanWt=wtm.shape()(0);

              for (Int row=0; row<nRow; row++) {
                  for (Int chn=0; chn<nChan; chn++) {
                      if(!vb->flag()(chn,row)) {
			  Float currwt=wtm(chn%nChanWt,row);  // the weight for this chan,row
                          Float f=vb->frequency()(chn)/C::c;
                          u=vb->uvw()(row)(0)*f;
                          v=vb->uvw()(row)(1)*f;
                          Int ucell=Int(uscale_p*u+uorigin_p);
                          Int vcell=Int(vscale_p*v+vorigin_p);
			 
                          if(((ucell-uBox)>0)&&((ucell+uBox)<nx)&&((vcell-vBox)>0)&&((vcell+vBox)<ny)) {
                              for (Int iv=-vBox;iv<=vBox;iv++) {
                                  for (Int iu=-uBox;iu<=uBox;iu++) {
				      gwt_p[fid](ucell+iu,vcell+iv)+=currwt;
                                      sumwt[fid]+=currwt;
                                  }
                              }
                          }
                          ucell=Int(-uscale_p*u+uorigin_p);
                          vcell=Int(-vscale_p*v+vorigin_p);
                          if(((ucell-uBox)>0)&&((ucell+uBox)<nx)&&((vcell-vBox)>0)&&((vcell+vBox)<ny)) {
                              for (Int iv=-vBox;iv<=vBox;iv++) {
                                  for (Int iu=-uBox;iu<=uBox;iu++) {
                                      gwt_p[fid](ucell+iu,vcell+iv)+=currwt;
                                      sumwt[fid]+=currwt;
                                  }
                              }
                          }
                      }
                  }
              }
          }
      }

      // We use the approximation that all statistical weights are equal to
      // calculate the average summed weights (over visibilities, not bins!)
      // This is simply to try an ensure that the normalization of the robustness
      // parameter is similar to that of the ungridded case, but it doesn't have
      // to be exact, since any given case will require some experimentation.

      //Float f2, d2;
      for(fid=0; fid < Int(gwt_p.nelements()); ++fid){
	if (rmode=="norm") {
              os << "Normal robustness, robust = " << robust << LogIO::POST;
              Double sumlocwt = 0.;
              for(Int vgrid=0;vgrid<ny;vgrid++) {
                  for(Int ugrid=0;ugrid<nx;ugrid++) {
                      if(gwt_p[fid](ugrid, vgrid)>0.0) sumlocwt+=square(gwt_p[fid](ugrid,vgrid));
                  }
              }
              f2_p[fid] = square(5.0*pow(10.0,Double(-robust))) / (sumlocwt / sumwt[fid]);
              d2_p[fid] = 1.0;

          }
          else if (rmode=="abs") {
              os << "Absolute robustness, robust = " << robust << ", noise = "
                      << noise.get("Jy").getValue() << "Jy" << LogIO::POST;
              f2_p[fid] = square(robust);
              d2_p[fid] = 2.0 * square(noise.get("Jy").getValue());

          }
          else {
              f2_p[fid] = 1.0;
              d2_p[fid] = 0.0;
          }
      }

  }

  VisImagingWeight::VisImagingWeight(ROVisibilityIterator& vi, Block<Matrix<Float> >& grids, const String& rmode, const Quantity& noise,
                                     const Double robust, const Quantity& cellx, const Quantity& celly,
                                     const Bool multiField) : multiFieldMap_p(-1), doFilter_p(False), robust_p(robust), rmode_p(rmode), noise_p(noise) {

   LogIO os(LogOrigin("VisSetUtil", "VisImagingWeight()", WHERE));

      VisBufferAutoPtr vb (vi);
      wgtType_p="uniform";
      gwt_p.resize(grids.nelements(), True, False);
      for (uInt k =0; k < gwt_p.nelements(); ++k)
	gwt_p[k].assign(grids[k]);
      nx_p=gwt_p[0].shape()[0];
      ny_p=gwt_p[0].shape()[1];
      uscale_p=(nx_p*cellx.get("rad").getValue());
      vscale_p=(ny_p*celly.get("rad").getValue());
      uorigin_p=nx_p/2;
      vorigin_p=ny_p/2;
      
      multiFieldMap_p.clear();
      Int fields=0;
      String mapid;
      for (vi.originChunks();vi.moreChunks();vi.nextChunk()) {
	for (vi.origin();vi.more();vi++) {
	  if(vb->newFieldId()){
	    mapid=String::toString(vb->msId())+String("_")+String::toString(vb->fieldId());
	    if(multiField){
	      if(!multiFieldMap_p.isDefined(mapid)){
		fields+=1;
                   
	      }
	    }
	    if(!multiFieldMap_p.isDefined(mapid))
	      multiFieldMap_p.define(mapid, fields);
	  }
	}
      }
      f2_p.resize(gwt_p.nelements());
      d2_p.resize(gwt_p.nelements());
      for(uInt fid=0; fid < gwt_p.nelements(); ++fid){
	if (rmode_p=="norm") {
	  os << "Normal robustness, robust = " << robust_p << LogIO::POST;
	  Double sumlocwt = 0.;
	  for(Int vgrid=0;vgrid<ny_p;vgrid++) {
	    for(Int ugrid=0;ugrid<nx_p;ugrid++) {
	      if(gwt_p[fid](ugrid, vgrid)>0.0) sumlocwt+=square(gwt_p[fid](ugrid,vgrid));
	    }
	  }
	  Double sumwt_fid=sum(gwt_p[fid]);
	  f2_p[fid] = square(5.0*pow(10.0,Double(-robust_p))) / (sumlocwt / sumwt_fid);
	  d2_p[fid] = 1.0;

	}
          else if (rmode=="abs") {
              os << "Absolute robustness, robust = " << robust_p << ", noise = "
                      << noise_p.get("Jy").getValue() << "Jy" << LogIO::POST;
              f2_p[fid] = square(robust_p);
              d2_p[fid] = 2.0 * square(noise_p.get("Jy").getValue());

          }
          else {
              f2_p[fid] = 1.0;
              d2_p[fid] = 0.0;
          }
      }
}

  VisImagingWeight::VisImagingWeight(vi::VisibilityIterator2& visIter,
				     const String& rmode, const Quantity& noise,
                                     const Double robust, const Int nx, const Int ny,
                                     const Quantity& cellx, const Quantity& celly,
                                     const Int uBox, const Int vBox, const Bool multiField) : multiFieldMap_p(-1), doFilter_p(False), robust_p(robust), rmode_p(rmode), noise_p(noise) {

      LogIO os(LogOrigin("VisSetUtil", "VisImagingWeight()", WHERE));



      vi::VisBuffer2* vb=(visIter.getVisBuffer());
      wgtType_p="uniform";
      // Float uscale, vscale;
      //Int uorigin, vorigin;
      Vector<Double> deltas;
      uscale_p=(nx*cellx.get("rad").getValue());
      vscale_p=(ny*celly.get("rad").getValue());
      uorigin_p=nx/2;
      vorigin_p=ny/2;
      nx_p=nx;
      ny_p=ny;
      // Simply declare a big matrix
      //Matrix<Float> gwt(nx,ny);
      gwt_p.resize(1);
      multiFieldMap_p.clear();
      visIter.originChunks();
      visIter.origin();
      String mapid=String::toString(vb->msId())+String("_")+String::toString(vb->fieldId()[0]);
      multiFieldMap_p.define(mapid, 0);
      gwt_p[0].resize(nx, ny);
      gwt_p[0].set(0.0);

      // Discover if weightSpectrum non-trivially available
      Bool doWtSp=visIter.weightSpectrumExists();

      Int fields=0;
      for (visIter.originChunks();visIter.moreChunks();visIter.nextChunk()) {
          for (visIter.origin();visIter.more();visIter.next()) {
              if(vb->isNewFieldId()){
                  mapid=String::toString(vb->msId())+String("_")+String::toString(vb->fieldId()[0]);
                  if(multiField){
                      if(!multiFieldMap_p.isDefined(mapid)){
                          fields+=1;
                          gwt_p.resize(fields+1);
                          gwt_p[fields].resize(nx,ny);
                          gwt_p[fields].set(0.0);
                      }
                  }
                  if(!multiFieldMap_p.isDefined(mapid))
                      multiFieldMap_p.define(mapid, fields);
              }
          }
      }

      Float u, v;
      Vector<Double> sumwt(fields+1,0.0);
      f2_p.resize(fields+1);
      d2_p.resize(fields+1);
      Int fid=0;
      for (visIter.originChunks();visIter.moreChunks();visIter.nextChunk()) {
          for (visIter.origin();visIter.more();visIter.next()) {
              if(vb->isNewFieldId())
                  mapid=String::toString(vb->msId())+String("_")+String::toString(vb->fieldId()[0]);
              fid=multiFieldMap_p(mapid);
              Int nRow=vb->nRows();
              Int nChan=vb->nChannels();

	      // Extract weights correctly (use WEIGHT_SPECTRUM, if available)
	      Matrix<Float> wtm;
	      Cube<Float> wtc;
	      if (doWtSp)
		// WS available [nCorr,nChan,nRow]
		wtc.reference(vb->weightSpectrum());       
	      else {
		Int nCorr=vb->nCorrelations();
		// WS UNavailable weight()[nCorr,nRow] --> [nCorr,nChan,nRow]
	        wtc.reference(vb->weight().reform(IPosition(3,nCorr,1,nRow)));  // unchan'd weight as single-chan
	      }
	      unPolChanWeight(wtm,wtc);   // Collapse on corr axis

	      // Used for mod of chn index below
	      Int nChanWt=wtm.shape()(0);

	      //Oww !!! temporary implementation of old vb.flag just to see if things work
	      Matrix<Bool> flag;
	      cube2Matrix(vb->flagCube(), flag);
              for (Int row=0; row<nRow; row++) {
                  for (Int chn=0; chn<nChan; chn++) {
                	  
		    Float currwt=wtm(chn%nChanWt,row);  // the weight for this chan,row
			
                      if(!flag(chn,row)) {
                          Float f=vb->getFrequency(row, chn)/C::c;
                          u=vb->uvw()(0,row)*f;
                          v=vb->uvw()(1,row)*f;
                          Int ucell=Int(uscale_p*u+uorigin_p);
                          Int vcell=Int(vscale_p*v+vorigin_p);
                          if(((ucell-uBox)>0)&&((ucell+uBox)<nx)&&((vcell-vBox)>0)&&((vcell+vBox)<ny)) {
                              for (Int iv=-vBox;iv<=vBox;iv++) {
                                  for (Int iu=-uBox;iu<=uBox;iu++) {
				      gwt_p[fid](ucell+iu,vcell+iv)+=currwt;
                                      sumwt[fid]+=currwt;
                                  }
                              }
                          }
                          ucell=Int(-uscale_p*u+uorigin_p);
                          vcell=Int(-vscale_p*v+vorigin_p);
                          if(((ucell-uBox)>0)&&((ucell+uBox)<nx)&&((vcell-vBox)>0)&&((vcell+vBox)<ny)) {
                              for (Int iv=-vBox;iv<=vBox;iv++) {
                                  for (Int iu=-uBox;iu<=uBox;iu++) {
                                      gwt_p[fid](ucell+iu,vcell+iv)+=currwt;
                                      sumwt[fid]+=currwt;
                                  }
                              }
                          }
                      }
                  }
              }
          }
      }

      // We use the approximation that all statistical weights are equal to
      // calculate the average summed weights (over visibilities, not bins!)
      // This is simply to try an ensure that the normalization of the robustness
      // parameter is similar to that of the ungridded case, but it doesn't have
      // to be exact, since any given case will require some experimentation.

      //Float f2, d2;
      for(fid=0; fid < Int(gwt_p.nelements()); ++fid){
	if (rmode=="norm") {
              os << "Normal robustness, robust = " << robust << LogIO::POST;
              Double sumlocwt = 0.;
              for(Int vgrid=0;vgrid<ny;vgrid++) {
                  for(Int ugrid=0;ugrid<nx;ugrid++) {
                      if(gwt_p[fid](ugrid, vgrid)>0.0) sumlocwt+=square(gwt_p[fid](ugrid,vgrid));
                  }
              }
              f2_p[fid] = square(5.0*pow(10.0,Double(-robust))) / (sumlocwt / sumwt[fid]);
              d2_p[fid] = 1.0;

          }
          else if (rmode=="abs") {
              os << "Absolute robustness, robust = " << robust << ", noise = "
                      << noise.get("Jy").getValue() << "Jy" << LogIO::POST;
              f2_p[fid] = square(robust);
              d2_p[fid] = 2.0 * square(noise.get("Jy").getValue());

          }
          else {
              f2_p[fid] = 1.0;
              d2_p[fid] = 0.0;
          }
      }
  }

  VisImagingWeight::~VisImagingWeight(){
      for (uInt fid=0; fid < gwt_p.nelements(); ++fid){
          gwt_p[fid].resize();
      }
  }


  void VisImagingWeight::setFilter(const String& type, const Quantity& bmaj,
				   const Quantity& bmin, const Quantity& bpa)
  {

     LogIO os(LogOrigin("VisImagingWeight", "setFilter()", WHERE));

    if (type=="gaussian") {
      
      Bool lambdafilt=False;
      
      if( bmaj.getUnit().contains("lambda"))
	lambdafilt=True;
      if(lambdafilt){
	os << "Filtering for Gaussian of shape: " 
	   << bmaj.get("klambda").getValue() << " by " 
	   << bmin.get("klambda").getValue() << " (klambda) at p.a. "
	   << bpa.get("deg").getValue() << " (degrees)" << LogIO::POST;
	rbmaj_p=log(2.0)/square(bmaj.get("lambda").getValue());
	rbmin_p=log(2.0)/square(bmin.get("lambda").getValue());
      }
      else{
	os << "Filtering for Gaussian of shape: " 
	   << bmaj.get("arcsec").getValue() << " by " 
	   << bmin.get("arcsec").getValue() << " (arcsec) at p.a. "
	   << bpa.get("deg").getValue() << " (degrees)" << LogIO::POST;
	
	// Convert to values that we can use
	Double fact = 4.0*log(2.0);
	rbmaj_p = fact*square(bmaj.get("rad").getValue());
	rbmin_p = fact*square(bmin.get("rad").getValue());
      }
      Double rbpa  = MVAngle(bpa).get("rad").getValue();
      cospa_p = sin(rbpa);
      sinpa_p = cos(rbpa);
      doFilter_p=True;

    }
    else {
      os << "Unknown filtering " << type << LogIO::EXCEPTION;    
    }
  


  
  }


  Bool VisImagingWeight::doFilter() const{

    return doFilter_p;
  }


  void VisImagingWeight::filter(Matrix<Float>& imWeight, const Matrix<Bool>& flag, 
				const Matrix<Double>& uvw,
				const Vector<Double>& frequency, const Matrix<Float>& weight) const{

    // Expecting weight[nchan,nrow], where nchan=1 or nChan (data)
    // If nchan=1 (i.e., WEIGHT_SPECTRUM unavailable), then the
    //  following will be handle the channel degeneracy correctly, using %.


    Int nRow=imWeight.shape()(1);
    Int nChan=imWeight.shape()(0);
    Int nChanWt=weight.shape()(0);
    for (Int row=0; row<nRow; row++) {
      for (Int chn=0; chn<nChan; chn++) {
	Double invLambdaC=frequency(chn)/C::c;
	Double u = uvw(0,row);
	Double v = uvw(1,row);
	if(!flag(chn,row) && (weight(chn%nChanWt,row)>0.0) ) {
	  Double ru = invLambdaC*(  cospa_p * u + sinpa_p * v);
	  Double rv = invLambdaC*(- sinpa_p * u + cospa_p * v);
	  Double filter = exp(-rbmaj_p*square(ru) - rbmin_p*square(rv));
	  imWeight(chn,row)*=filter;
	}
	else {
	  imWeight(chn,row)=0.0;
	}
      }
    }


  }


    void VisImagingWeight::weightUniform(Matrix<Float>& imWeight, const Matrix<Bool>& flag, const Matrix<Double>& uvw,
                                         const Vector<Double>& frequency,
                                         const Matrix<Float>& weight, const Int msId, const Int fieldId) const{


      //cout << " WEIG " << nx_p << "  " << ny_p << "   " << gwt_p[0].shape() << endl;
      //cout << "f2 " << f2_p[0] << " d2 " << d2_p[0] << " uscale " << uscale_p << " vscal " << vscale_p << " origs " << uorigin_p << "  " << vorigin_p << endl; 
      String mapid=String::toString(msId)+String("_")+String::toString(fieldId);
      //cout << "min max gwt " << min(gwt_p[0]) << "    " << max(gwt_p[0]) << " mapid " << mapid <<endl; 
      if(!multiFieldMap_p.isDefined(mapid))
	throw(AipsError("Imaging weight calculation is requested for a data that was not selected"));
      
      Int fid=multiFieldMap_p(mapid);
      //Int ndrop=0;
    
      Double sumwt=0.0;
      Int nRow=imWeight.shape()(1);
      Int nChannel=imWeight.shape()(0);
      Int nChanWt=weight.shape()(0);
      Float u, v;
      for (Int row=0; row<nRow; row++) {
	for (Int chn=0; chn<nChannel; chn++) {
	  if (!flag(chn,row)) {
	    Float f=frequency(chn)/C::c;
	    u=uvw(0, row)*f;
	    v=uvw(1, row)*f;
	    Int ucell=Int(uscale_p*u+uorigin_p);
	    Int vcell=Int(vscale_p*v+vorigin_p);
	    imWeight(chn,row)=weight(chn%nChanWt,row);
	    if((ucell>0)&&(ucell<nx_p)&&(vcell>0)&&(vcell<ny_p) &&gwt_p[fid](ucell,vcell)>0.0) {
		imWeight(chn,row)/=gwt_p[fid](ucell,vcell)*f2_p[fid]+d2_p[fid];
		sumwt+=imWeight(chn,row);
	      
	    }
	    else {
	      imWeight(chn,row)=0.0;
	      //ndrop++;
	    }
	  }
	  else{
	    imWeight(chn,row)=0.0;
	  }
	}
      }
      
    }

  /*  unused version?
void VisImagingWeight::weightNatural(Matrix<Float>& imagingWeight, const Matrix<Bool>& flag,
                                         const Matrix<Float>& weight) const{

      

        Int nRow=imagingWeight.shape()(1);
        Vector<Float> wgtRow(nRow);
	
        for (Int row=0; row<nRow; row++) {
	  wgtRow(row)=max(weight.column(row));
        }
	weightNatural(imagingWeight, flag, wgtRow);

    }
  */
    void VisImagingWeight::weightNatural(Matrix<Float>& imagingWeight, const Matrix<Bool>& flag,
                                         const Matrix<Float>& weight) const{

        Double sumwt=0.0;

        Int nRow=imagingWeight.shape()(1);
        Int nChan=imagingWeight.shape()(0);
        Int nChanWt=weight.shape()(0);
        for (Int row=0; row<nRow; row++) {
            for (Int chn=0; chn<nChan; chn++) {
                if( !flag(chn,row) ) {
 		    imagingWeight(chn,row)=weight(chn%nChanWt,row);
                    sumwt+=imagingWeight(chn,row);
                }
                else {
                    imagingWeight(chn,row)=0.0;
                }
            }
        }


    }


    void VisImagingWeight::weightRadial(Matrix<Float>& imagingWeight,
                                        const Matrix<Bool>& flag,
                                        const Matrix<Double>& uvw,
                                        const Vector<Double>& frequency,
                                        const Matrix<Float>& weight) const{

        Double sumwt=0.0;
        Int nRow=imagingWeight.shape()(1);
        Int nChan=imagingWeight.shape()(0);
	Int nChanWt=weight.shape()(0);

        for (Int row=0; row<nRow; row++) {
            for (Int chn=0; chn< nChan; chn++) {
                Float f=frequency(chn)/C::c;
                if( !flag(chn,row) ) {
                    imagingWeight(chn,row)=
		      f*sqrt(square(uvw(0, row))+square(uvw(1, row)))
		            *weight(chn%nChanWt,row);
                    sumwt+=imagingWeight(chn,row);
                }
                else {
                    imagingWeight(chn,row)=0.0;
                }
            }
        }


    }

    VisImagingWeight& VisImagingWeight::operator=(const VisImagingWeight& other){
        if(this != &other){
            gwt_p=other.gwt_p;
            wgtType_p=other.wgtType_p;
            uscale_p=other.uscale_p;
            vscale_p=other.vscale_p;
	    f2_p.resize();
	    d2_p.resize();
            f2_p=other.f2_p;
            d2_p=other.d2_p;
            uorigin_p=other.uorigin_p;
            vorigin_p=other.vorigin_p;
            nx_p=other.nx_p;
            ny_p=other.ny_p;
	    doFilter_p=other.doFilter_p;
	    cospa_p=other.cospa_p;
	    sinpa_p=other.sinpa_p;
	    rbmaj_p=other.rbmaj_p;
	    rbmin_p=other.rbmin_p;
	    robust_p=other.robust_p;
	    rmode_p=other.rmode_p;
	    multiFieldMap_p=other.multiFieldMap_p;
        }
        return *this;
    }

    String VisImagingWeight::getType() const{

        return wgtType_p;

    }
  Bool VisImagingWeight::getWeightDensity (Block<Matrix<Float> >& density){
    if(wgtType_p != "uniform"){
      density.resize(0, True, False);
      return False;
    }
    density.resize(gwt_p.nelements(), True, False);
    for (uInt k=0; k < gwt_p.nelements(); ++k){
      density[k].resize();
      density[k].assign(gwt_p[k]);
    }
    return True;
  }
  void VisImagingWeight::setWeightDensity(const Block<Matrix<Float> >& density){
    if(wgtType_p=="uniform"){
      gwt_p.resize(density.nelements(), True, False);
      for (uInt k=0; k < gwt_p.nelements(); ++k){
	gwt_p[k].resize();
	gwt_p[k]=density[k];
      }
      //break any old reference
      f2_p.resize();
      d2_p.resize();
      f2_p.resize(gwt_p.nelements());
      d2_p.resize(gwt_p.nelements());
      f2_p.set(1.0);
      d2_p.set(0.0);
       //Float f2, d2;
      for(uInt fid=0; fid < gwt_p.nelements(); ++fid){
	if (rmode_p=="norm") {
	  Double sumlocwt = 0.;
	  for(Int vgrid=0;vgrid<gwt_p[fid].shape()(1);vgrid++) {
	      for(Int ugrid=0;ugrid<gwt_p[fid].shape()(0);ugrid++) {
		if(gwt_p[fid](ugrid, vgrid)>0.0) sumlocwt+=square(gwt_p[fid](ugrid,vgrid));
	      }
	  }
	  Double sumwt_fid=sum(gwt_p[fid]);
	  f2_p[fid] = square(5.0*pow(10.0,Double(-robust_p))) / (sumlocwt / sumwt_fid);
	  d2_p[fid] = 1.0;
	}
	else if (rmode_p=="abs") {
	  f2_p[fid] = square(robust_p);
	  d2_p[fid] = 2.0 * square(noise_p.get("Jy").getValue());
	  
	}
	else {
	  f2_p[fid] = 1.0;
	  d2_p[fid] = 0.0;
	}
      }
      
    }
    
    
  }
  void VisImagingWeight::cube2Matrix(const Cube<Bool>& fcube, Matrix<Bool>& fMat)
  {
	  fMat.resize(fcube.shape()[1], fcube.shape()[2]);
	  Bool deleteIt1;
	  Bool deleteIt2;
	  const Bool * pcube = fcube.getStorage (deleteIt1);
	  Bool * pflags = fMat.getStorage (deleteIt2);
	  for (uInt row = 0; row < fcube.shape()[2]; row++) {
		  for (Int chn = 0; chn < fcube.shape()[1]; chn++) {
			  *pflags = *pcube++;
			  for (Int pol = 1; pol < fcube.shape()[0]; pol++, pcube++) {
				  *pflags = *pcube ? *pcube : *pflags;
			  }
			  pflags++;
		  }
	  }
	  fcube.freeStorage (pcube, deleteIt1);
	  fMat.putStorage (pflags, deleteIt2);
  }


  void VisImagingWeight::unPolChanWeight(Matrix<Float>& chanRowWt, const Cube<Float>& corrChanRowWt) const {

    //cout << "VIW::uPCW" << endl;

    IPosition sh3=corrChanRowWt.shape();
    IPosition sh2=sh3.getLast(2);
    //cout << sh3 << sh2 << endl;
    chanRowWt.resize(sh2);
    //cout << "assign..." << endl;
    chanRowWt=corrChanRowWt(Slice(0,1,1),Slice(),Slice()).reform(sh2);
    const Int& nPol=sh3[0];
    if (nPol>1) {
      chanRowWt += corrChanRowWt(Slice(nPol-1,1,1),Slice(),Slice()).reform(sh2);
      chanRowWt /= 2.0f;
    }

    //    cout << "VIW::uPCW" << endl;

  }


}//# NAMESPACE CASA - END


