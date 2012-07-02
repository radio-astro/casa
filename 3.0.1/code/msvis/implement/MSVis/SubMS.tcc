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
void SubMS::chanAvgSameShapes(const ROArrayColumn<M>& data,
                              const MS::PredefinedColumns columnName,
                              const Bool doSpWeight,
                              ROArrayColumn<Float>& wgtSpec,
                              Cube<Float>& outspweight,
                              Vector<Float>& outwgtspectmp,
                              Matrix<Float>& inwgtspectmp,
                              const Float *inwptr,
                              ROArrayColumn<Bool>& flag,
                              Matrix<Bool>& inflagtmp,
                              const Bool *iflg,
                              const Int nrow, Cube<Bool>& outflag,
                              const Bool writeToDataCol)
{
  Bool deleteIptr;
  Matrix<M> indatatmp(npol_p[0], inNumChan_p[0]);
  const M *iptr = indatatmp.getStorage(deleteIptr);

  // Sigh, iflg itself is const, but it points at the start of inflagtmp,
  // which is continually refreshed by a row of flag.
  
  Cube<M> outdata(npol_p[0], nchan_p[0], nrow);
  Vector<M> outdatatmp(npol_p[0]);
  //    const Complex *optr = outdatatmp.getStorage(deleteOptr);

  for (Int row = 0; row < nrow; ++row){
    data.get(row, indatatmp);
    flag.get(row, inflagtmp);

    if(doSpWeight)
      wgtSpec.get(row, inwgtspectmp);

    Int ck = 0;
    Int chancounter = 0;
    Vector<Int> avcounter(npol_p[0]);
    outdatatmp.set(0); outwgtspectmp.set(0);
    avcounter.set(0);
      
    for(Int k = chanStart_p[0]; k < (nchan_p[0] * chanStep_p[0] +
                                     chanStart_p[0]); ++k){
      if(chancounter == chanStep_p[0]){
        outdatatmp.set(0); outwgtspectmp.set(0);
        chancounter = 0;
        avcounter.set(0);
      }
      ++chancounter;
      for(Int j = 0; j < npol_p[0]; ++j){
        Int offset = j + k * npol_p[0];
        if(!iflg[offset]){
          if(doSpWeight){
            outdatatmp[j] += iptr[offset] * inwptr[offset];
            outwgtspectmp[j] += inwptr[offset];
          }
          else
            outdatatmp[j] += iptr[offset];	   
          ++avcounter[j];
        }

        if(chancounter == chanStep_p[0]){
          if(avcounter[j] != 0){
            // Should there be a warning if one data column wants flagging
            // and another does not?
            if(doSpWeight){
              if(outwgtspectmp[j] != 0.0)
                outdata(j, ck, row) = outdatatmp[j] / outwgtspectmp[j];
              else{
                outdata(j, ck, row) = 0.0;
                outflag(j, ck, row) = True;
              }
              outspweight(j, ck, row) = outwgtspectmp[j];
            }
            else{
              outdata(j, ck, row) = outdatatmp[j] / avcounter[j];	    
            }
          }
          else{
            outdata(j, ck, row) = 0;
            outflag(j, ck, row) = True;
            if(doSpWeight)
              outspweight(j, ck, row) = 0;
          }	
        }
      }
      if(chancounter == chanStep_p[0])
        ++ck;
    }
  }
  putDataColumn(*msc_p, outdata, columnName, writeToDataCol);
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


