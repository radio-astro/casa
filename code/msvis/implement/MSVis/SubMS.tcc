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
#include <tables/Tables/Table.h>
#include <tables/Tables/TableDesc.h>
#include <casa/BasicSL/String.h>

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
			const Int nrow)
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
  
  Matrix<Float> inwgtspectmp;
  Bool deleteIWptr;
  
  Matrix<M> outdata;
  Vector<M> outdatatmp;
  //    const Complex *optr = outdatatmp.getStorage(deleteOptr);

  Matrix<Float> outspweight;
  Vector<Float> outwgtspectmp;
  //   const Float *owptr = outwgtspectmp.getStorage(deleteOWptr);

  const ROScalarColumn<Int> dataDescIn(mscIn_p->dataDescId());

  // Guarantee oldDDID != ddID on 1st iteration.
  Int oldDDID = spwRelabel_p[oldDDSpwMatch_p[dataDescIn(0)]] - 1;

  for(Int row = 0; row < nrow; ++row){
    Int ddID = spwRelabel_p[oldDDSpwMatch_p[dataDescIn(row)]];
    Bool newDDID = (ddID != oldDDID);

    if(newDDID){
      oldDDID = ddID;

      if(ddID < 0){                      // Paranoia
	if(newDDID)
	  os << LogIO::WARN
	     << "Treating DATA_DESCRIPTION_ID " << ddID << " as 0."
	     << LogIO::POST;
	ddID = 0;
      }
      
      indatatmp.resize(npol_p[ddID], inNumChan_p[ddID]);
      inwgtspectmp.resize(npol_p[ddID], inNumChan_p[ddID]);
      inflagtmp.resize(npol_p[ddID], inNumChan_p[ddID]);
      outflag.resize(npol_p[ddID], nchan_p[ddID]);
      outdata.resize(npol_p[ddID], nchan_p[ddID]);
      outdatatmp.resize(npol_p[ddID]);
      if(doSpWeight){
	outspweight.resize(npol_p[ddID], nchan_p[ddID]);
	outwgtspectmp.resize(npol_p[ddID]);
      }
    }

    // Should come after any resize()s.
    const M *iptr = indatatmp.getStorage(deleteIptr);
    const Float *inwptr = inwgtspectmp.getStorage(deleteIWptr);
    const Bool *iflg = inflagtmp.getStorage(deleteIFptr);
    outflag.set(false);
    data.get(row, indatatmp);
    flag.get(row, inflagtmp);

    if(doSpWeight)
      wgtSpec.get(row, inwgtspectmp);

    uInt outChanInd = 0;
    Int chancounter = 0;
    Vector<Int> avcounter(npol_p[ddID]);
    outdatatmp.set(0); outwgtspectmp.set(0);
    avcounter.set(0);
    
    // chanStart_p is Int, therefore inChanInd is too.
    for(Int inChanInd = chanStart_p[ddID];
	inChanInd < (nchan_p[ddID] * chanStep_p[ddID] +
		     chanStart_p[ddID]); ++inChanInd){
      if(chancounter == chanStep_p[ddID]){
        outdatatmp.set(0); outwgtspectmp.set(0);
        chancounter = 0;
        avcounter.set(0);
      }
      ++chancounter;
      for(Int polInd = 0; polInd < npol_p[ddID]; ++polInd){
        Int offset = polInd + inChanInd * npol_p[ddID];
        if(!iflg[offset]){
          if(doSpWeight){
            outdatatmp[polInd] += iptr[offset] * inwptr[offset];
            outwgtspectmp[polInd] += inwptr[offset];
          }
          else
            outdatatmp[polInd] += iptr[offset];	   
          ++avcounter[polInd];
        }

        if(chancounter == chanStep_p[ddID]){
          if(avcounter[polInd] != 0){
            if(doSpWeight){
              if(outwgtspectmp[polInd] != 0.0){
                outdata(polInd,
			outChanInd) = outdatatmp[polInd] / outwgtspectmp[polInd];
	      }
              else{
                outdata(polInd, outChanInd) = 0.0;
                outflag(polInd, outChanInd) = True;
              }
              outspweight(polInd, outChanInd) = outwgtspectmp[polInd];
            }
            else{
              outdata(polInd, outChanInd) = outdatatmp[polInd] / avcounter[polInd];
            }
          }
          else{
            outdata(polInd, outChanInd) = 0;
            outflag(polInd, outChanInd) = True;
            if(doSpWeight)
              outspweight(polInd, outChanInd) = 0;
          }	
        }
      }
      if(chancounter == chanStep_p[ddID])
        ++outChanInd;
    }
    outDataCol.put(row, outdata);
    msc_p->flag().put(row, outflag);
    if(doSpWeight)
      msc_p->weightSpectrum().put(row, outspweight);
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


