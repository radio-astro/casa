//# FTMachine.tcc //For templated functions of FTMachine class 
//# Copyright (C) 2015
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


 template <class T> void FTMachine::getGrid(Array<T>& thegrid){
    thegrid.resize();
    void * ptr;
    Array<Float> tmpFloat;
    Array<Double> tmpDouble;
    IPosition shp;
    bool del;
    if(whatType(&thegrid)==TpArrayComplex){
      ptr=griddedData.getStorage(del);
      shp=griddedData.shape();
    }
    else if((whatType(&thegrid)==TpArrayDComplex)){
      ptr=griddedData2.getStorage(del);
      shp=griddedData2.shape();
    }
    else if(((whatType(&thegrid)==TpArrayFloat))){
      tmpFloat.resize(griddedData.shape());
      tmpFloat=real(griddedData);
      shp=tmpFloat.shape();
      ptr=tmpFloat.getStorage(del);
    }
    else if(((whatType(&thegrid)==TpArrayDouble))){
      tmpDouble.resize(griddedData2.shape());
      tmpDouble=real(griddedData2);
      shp=tmpDouble.shape();
      ptr=tmpDouble.getStorage(del);
    }  
    thegrid=Array<T>(shp, (T*)(ptr));
  };