//# SubMS.tcc 
//# Copyright (C) 2009
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This library is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//# 
//# You should have received a copy of the GNU General Public License
//# along with this library; if not, write to the Free Software
//# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
#include <msvis/MSVis/SubMS.h>
#include <tables/Tables/ArrayColumn.h>
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <casa/Arrays/Array.h>
#include <casa/BasicSL/String.h>
#include <casa/System/ProgressMeter.h>
#include <cmath>

namespace casa { //# NAMESPACE CASA - BEGIN

  // Add optional columns to outTab if present in inTab and possColNames.
  // Returns the number of added columns.
  template<class M>
  uInt SubMS::addOptionalColumns(const M& inTab, M& outTab, const Bool beLazy)
  {
    uInt nAdded = 0;    
    const TableDesc& inTD = inTab.actualTableDesc();
    
    // Only rely on the # of columns if you are sure that inTab and outTab
    // can't have the same # of columns without having _different_ columns,
    // i.e. use beLazy if outTab.actualTableDesc() is in its default state.
    uInt nInCol = inTD.ncolumn();
    if(!beLazy || nInCol > outTab.actualTableDesc().ncolumn()){
      LogIO os(LogOrigin("SubMS", "addOptionalColumns()"));

      Vector<String> oldColNames = inTD.columnNames();
      
      for(uInt k = 0; k < nInCol; ++k){
        if(!outTab.actualTableDesc().isColumn(oldColNames[k])){
          TableDesc tabDesc;
          try{
            M::addColumnToDesc(tabDesc, M::columnType(oldColNames[k]));
            if(tabDesc.ncolumn())                 // The tabDesc[0] is too 
              outTab.addColumn(tabDesc[0]);       // dangerous otherwise - it 
            else                                  // can dump core without
              throw(AipsError("Unknown column")); // throwing an exception.
            ++nAdded;
          }
          catch(...){   // NOT AipsError x
            os << LogIO::WARN 
               << "Could not add column " << oldColNames[k] << " to "
               << outTab.tableName()
               << LogIO::POST;
          }
	}
      }
    }
    return nAdded;
  }

template<class M>
void SubMS::filterChans(const ROArrayColumn<M>& data, ArrayColumn<M>& outDataCol,
			const Bool doSpWeight, ROArrayColumn<Float>& wgtSpec,
			const Int nrow, const Bool calcImgWts, 
			const Bool calcWtSig, const ROArrayColumn<Float>& rowWt,
			const ROArrayColumn<Float>& sigma)
{
  Bool deleteIptr;
  Matrix<M> indatatmp;

  LogIO os(LogOrigin("SubMS", "filterChans()"));

  // Sigh, iflg itself is const, but it points at the start of inflagtmp,
  // which is continually refreshed by a row of flag.
  ROArrayColumn<Bool> flag(mscIn_p->flag());
  Matrix<Bool> inflagtmp;
  Bool deleteIFptr;
  Matrix<Bool> outflag;
  
  Vector<Float> inrowwttmp;
  Vector<Float> outrowwt;
  
  Vector<Float> inrowsigtmp;
  Vector<Float> outrowsig;
  
  Matrix<Float> inwgtspectmp;
  Bool deleteIWptr;
  
  ROArrayColumn<Float> inImgWts;
  if(calcImgWts)
    inImgWts.reference(mscIn_p->imagingWeight());
  Float outImgWtsAccum = 0;
  Vector<Float> inImgWtsSpectrum;
  Vector<Float> outImgWts;
  
  Matrix<M> outdata;
  Vector<M> outdatatmp;
  //    const Complex *optr = outdatatmp.getStorage(deleteOptr);

  Matrix<Float> outspweight;
  Vector<Float> outwgtspectmp;
  //   const Float *owptr = outwgtspectmp.getStorage(deleteOWptr);

  Vector<Int> avcounter;

  const ROScalarColumn<Int> dataDescIn(mscIn_p->dataDescId());

  // Guarantee oldDDID != ddID on 1st iteration.
  Int oldDDID = spwRelabel_p[oldDDSpwMatch_p[dataDescIn(0)]] - 1;

  // chanStart_p is Int, therefore inChanInd is too.
  Int inChanInc;
  Int nperbin;

  Double rowwtfac, sigfac;
  
  ProgressMeter meter(0.0, nrow * 1.0, "split", "rows filtered", "", "",
		      True, nrow / 100);

  for(Int row = 0; row < nrow; ++row){
    Int ddID = spwRelabel_p[oldDDSpwMatch_p[dataDescIn(row)]];
    Bool newDDID = (ddID != oldDDID);

    if(newDDID){
      if(ddID < 0){                      // Paranoia
        os << LogIO::WARN
           << "Treating DATA_DESCRIPTION_ID " << ddID << " as 0."
           << LogIO::POST;
	ddID = 0;
      }
      
      inChanInc = averageChannel_p ? 1 : chanStep_p[ddID];
      nperbin = averageChannel_p ? chanStep_p[ddID] : 1;
      // .tcc files are hard to debug without print statements,
      //  but it is too easy to  make the logger thrash
      // the disk if these are left in.
      // os << LogIO::DEBUG1
      // 	 << ddID << ": inChanInc = " << inChanInc
      // 	 << " nperbin = " << nperbin
      // 	 << "\nrow " << row << ": inNumCorr_p[ddID] = "
      //         << inNumCorr_p[ddID]
      // 	 << ", ncorr_p[ddID] = " << ncorr_p[ddID]
      // 	 << "\ninNumChan_p[ddID] = " << inNumChan_p[ddID]
      // 	 << ", nchan_p[ddID] = " << nchan_p[ddID]
      // 	 << LogIO::POST;
      
      // resize() will return right away if the size does not change, so
      // it is not essential to check ncorr_p[ddID] != ncorr_p[oldDDID], etc.
      indatatmp.resize(inNumCorr_p[ddID], inNumChan_p[ddID]);
      inflagtmp.resize(inNumCorr_p[ddID], inNumChan_p[ddID]);
      outflag.resize(ncorr_p[ddID], nchan_p[ddID]);
      outdata.resize(ncorr_p[ddID], nchan_p[ddID]);
      outdatatmp.resize(ncorr_p[ddID]);
      if(doSpWeight){
        inwgtspectmp.resize(inNumCorr_p[ddID], inNumChan_p[ddID]);
	outspweight.resize(ncorr_p[ddID], nchan_p[ddID]);
	outwgtspectmp.resize(ncorr_p[ddID]);
      }
      if(calcImgWts){
        inImgWtsSpectrum.resize(inNumChan_p[ddID]);
        outImgWts.resize(nchan_p[ddID]);
      }

      if(calcWtSig){
	rowwtfac = static_cast<Float>(nchan_p[ddID]) / inNumChan_p[ddID];
	if(averageChannel_p)
	  rowwtfac *= chanStep_p[ddID];
	sigfac = 1.0 / sqrt(rowwtfac);
	os << LogIO::DEBUG1
	   << ddID << ": inNumChan_p[ddID] = " << inNumChan_p[ddID]
	   << ", nchan_p[ddID] = " << nchan_p[ddID]
	   << "\nrowwtfac = " << rowwtfac
	   << ", sigfac = " << sigfac
	   << LogIO::POST;
	inrowwttmp.resize(inNumCorr_p[ddID]);
	outrowwt.resize(ncorr_p[ddID]);
	inrowsigtmp.resize(inNumCorr_p[ddID]);
	outrowsig.resize(ncorr_p[ddID]);
	os << LogIO::DEBUG1
	   << "inNumCorr_p[ddID] = " << inNumCorr_p[ddID]
	   << ", ncorr_p[ddID] = " << ncorr_p[ddID]
	   << LogIO::POST;
      }

      avcounter.resize(ncorr_p[ddID]);

      oldDDID = ddID;
    }

    // Should come after any resize()s.
    outflag.set(false);
    data.get(row, indatatmp);
    flag.get(row, inflagtmp);
    // os << LogIO::DEBUG1 << "calcImgWts: " << calcImgWts << LogIO::POST;
    if(calcImgWts)
      inImgWts.get(row, inImgWtsSpectrum);
    // These were more to say "I made it here!" than anything.
    //os << LogIO::DEBUG1 << "doSpWeight: " << doSpWeight << LogIO::POST;
    //os << LogIO::DEBUG1 << "calcWtSig: " << calcWtSig << LogIO::POST;
    if(doSpWeight){
      outrowwt.set(0.0);
      if(calcWtSig)
	wgtSpec.get(row, inwgtspectmp);
    }
    else if(calcWtSig)
      rowWt.get(row, inrowwttmp);
    if(calcWtSig)
      sigma.get(row, inrowsigtmp);
    
    uInt outChanInd = 0;
    Int chancounter = 0;
    outdatatmp.set(0); outwgtspectmp.set(0);
    avcounter.set(0);
    
    const M *iptr = indatatmp.getStorage(deleteIptr);
    const Float *inwptr = inwgtspectmp.getStorage(deleteIWptr);
    const Bool *iflg = inflagtmp.getStorage(deleteIFptr);
    for(Int inChanInd = chanStart_p[ddID];
	inChanInd < (nchan_p[ddID] * chanStep_p[ddID] +
		     chanStart_p[ddID]); inChanInd += inChanInc){
      if(chancounter == nperbin){
        outdatatmp.set(0); outwgtspectmp.set(0);
        chancounter = 0;
        outImgWtsAccum = 0;
        avcounter.set(0);
      }
      ++chancounter;

      if(calcImgWts){
        outImgWtsAccum += inImgWtsSpectrum[inChanInd];
        if(chancounter == chanStep_p[0])
          outImgWts[outChanInd] = outImgWtsAccum / chancounter;
      }

      for(Int outCorrInd = 0; outCorrInd < ncorr_p[ddID]; ++outCorrInd){
        Int offset = inPolOutCorrToInCorrMap_p[polID_p[ddID]][outCorrInd]
	             + inChanInd * inNumCorr_p[ddID];
	// //if(ncorr_p[ddID] != inNumCorr_p[ddID])
	//   os << LogIO::DEBUG2		       // 
	//      << "outCorrInd = " << outCorrInd  //
	//      << "\ninChanInd = " << inChanInd	//
	//      << "\noffset = " << offset		// 
	//      << LogIO::POST;			// 
	// os << LogIO::DEBUG2
	//    << "iflg[offset] = " << iflg[offset]
	//    << "\niptr[offset] = " << iptr[offset]
	//   //<< "\ninwptr[offset] = " << inwptr[offset]
	//    << LogIO::POST;
        if(!iflg[offset]){
          if(doSpWeight){
            outdatatmp[outCorrInd] += iptr[offset] * inwptr[offset];
            outwgtspectmp[outCorrInd] += inwptr[offset];
          }
          else
            outdatatmp[outCorrInd] += iptr[offset];	   
          ++avcounter[outCorrInd];
        }

        if(chancounter == nperbin){
	  // //if(ncorr_p[ddID] != inNumCorr_p[ddID])
	  //   os << LogIO::DEBUG2
	  //      << "row " << row
	  //      << ": avcounter[outCorrInd] = " << avcounter[outCorrInd]
	  //     << LogIO::POST;
          if(avcounter[outCorrInd] != 0){
            if(doSpWeight){
              if(outwgtspectmp[outCorrInd] != 0.0){
                outdata(outCorrInd,
			outChanInd) = outdatatmp[outCorrInd] / 
		                      outwgtspectmp[outCorrInd];
		outrowwt[outCorrInd] += outwgtspectmp[outCorrInd];
	      }
              else{
                outdata(outCorrInd, outChanInd) = 0.0;
                outflag(outCorrInd, outChanInd) = True;
              }
              outspweight(outCorrInd, outChanInd) = outwgtspectmp[outCorrInd];
            }
            else{
              outdata(outCorrInd,
		      outChanInd) = outdatatmp[outCorrInd] / avcounter[outCorrInd];
            }
          }
          else{
            outdata(outCorrInd, outChanInd) = 0;
            outflag(outCorrInd, outChanInd) = True;
            if(doSpWeight)
              outspweight(outCorrInd, outChanInd) = 0;
          }	
        }
      }
      if(chancounter == chanStep_p[ddID])
        ++outChanInd;
    }
    outDataCol.put(row, outdata);
    msc_p->flag().put(row, outflag);
    if(calcWtSig){
      for(Int outCorrInd = 0; outCorrInd < ncorr_p[ddID]; ++outCorrInd){
        Int inCorr = inPolOutCorrToInCorrMap_p[polID_p[ddID]][outCorrInd];
	if(!doSpWeight)
	  outrowwt[outCorrInd] = rowwtfac * inrowwttmp[inCorr];
	outrowsig[outCorrInd] = sigfac * inrowsigtmp[inCorr];
      }
      msc_p->weight().put(row, outrowwt);
      msc_p->sigma().put(row, outrowsig);
    }
    if(doSpWeight)
      msc_p->weightSpectrum().put(row, outspweight);
    if(calcImgWts)
      msc_p->imagingWeight().put(row, outImgWts);

    meter.update(row);
  }
}

template<class M>
void SubMS::accumUnflgDataWS(Array<M>& data_toikit,
                             const Array<Float>& unflgWtSpec,
                             const Array<M>& inData, const Array<Bool>& flag,
                             Matrix<M>& outData)
{
  data_toikit = inData * unflgWtSpec;

  // It's already multiplied by a zero weight, but zero it again
  // just in case some flagged NaNs or infinities snuck in there.
  data_toikit(flag) = 0.0;
              
  outData += data_toikit;
}

template<class M>
void SubMS::accumUnflgData(Array<M>& data_toikit,
                           const Vector<Float>& unflaggedwt,
                           const Array<M>& inData, const Array<Bool>& flag,
                           Matrix<M>& outData)
{
  data_toikit = inData;
  binOpExpandInPlace(data_toikit, unflaggedwt, Multiplies<M, Float>());
  data_toikit(flag) = 0.0;  // Do this AFTER the multiplication to catch NaNs.
  outData += data_toikit;
}

} //# NAMESPACE CASA - END


