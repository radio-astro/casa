// -*- C++ -*-
//# VBStore.h: Definition of the VBStore class
//# Copyright (C) 1997,1998,1999,2000,2001,2002,2003
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
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$
#ifndef SYNTHESIS_VBSTORE_H
#define SYNTHESIS_VBSTORE_H
#include <synthesis/TransformMachines/Utils.h>
#include <synthesis/TransformMachines/CFBuffer.h>

namespace casa { //# NAMESPACE CASA - BEGIN
  class VBStore
  {
  public:
    VBStore():dopsf_p(False) {};
    ~VBStore() {};
    inline Int nRow()              {return nRow_p;};
    inline Int beginRow()          {return beginRow_p;}
    inline Int endRow()            {return endRow_p;}
    inline Int spwID()             {return spwID_p;}
    inline Bool dopsf()            {return dopsf_p;}
    inline Bool useCorrected()     {return useCorrected_p;};
    Matrix<Double>& uvw()          {return uvw_p;};
    Vector<Bool>& rowFlag()        {return rowFlag_p;};
    Cube<Bool>& flagCube()         {return flagCube_p;};
    Matrix<Float>& imagingWeight() {return imagingWeight_p;};
    Cube<Complex>& visCube()       {return visCube_p;};
    Vector<Double>& freq()         {return freq_p;};
    Cube<Complex>& modelCube()     {return modelCube_p;};
    Cube<Complex>& correctedCube() {return correctedCube_p;};
    Quantity pa()                  {return paQuant_p;}
    const VisBuffer& vb()          {return *vb_p;}
    Double imRefFreq()             {return imRefFreq_p;}

    void reference(const VBStore& other)
    {
      nRow_p=other.nRow_p;  beginRow_p=other.beginRow_p; endRow_p=other.endRow_p;
      dopsf_p = other.dopsf_p;
      useCorrected_p = other.useCorrected_p;

      uvw_p.reference(other.uvw_p);
      rowFlag_p.reference(other.rowFlag_p);
      flagCube_p.reference(other.flagCube_p);
      imagingWeight_p.reference(other.imagingWeight_p);
      freq_p.reference(other.freq_p);
      // if (useCorrected_p) correctedCube_p.reference(other.correctedCube_p);
      // else visCube_p.reference(other.visCube_p);
      // if (useCorrected_p) 
      // 	{
      // 	  correctedCube_p.reference(other.correctedCube_p);
      // 	  visCube_p.reference(other.correctedCube_p);
      // 	}
      // else visCube_p.reference(other.visCube_p);
      correctedCube_p.reference(other.correctedCube_p);
      visCube_p.reference(other.visCube_p);
      modelCube_p.reference(other.modelCube_p);

      // uvw_p.assign(other.uvw_p);
      // rowFlag_p.assign(other.rowFlag_p);
      // flagCube_p.assign(other.flagCube_p);
      // imagingWeight_p.assign(other.imagingWeight_p);
      // freq_p.assign(other.freq_p);
      // visCube_p.assign(other.visCube_p);
      // modelCube_p.assign(other.modelCube_p);
      // correctedCube_p.assign(other.correctedCube_p);
    }

    Int nRow_p, beginRow_p, endRow_p, spwID_p, startChan_p, endChan_p,nDataChan_p, nDataPol_p;
    Matrix<Double> uvw_p;
    Vector<Bool> rowFlag_p;
    Cube<Bool> flagCube_p;
    Matrix<Float> imagingWeight_p;
    Cube<Complex> visCube_p, modelCube_p, correctedCube_p;
    Vector<Double> freq_p;
    Bool dopsf_p,useCorrected_p, conjBeams_p;
    Vector<Int> corrType_p;
    Quantity paQuant_p;
    Vector<Int> antenna1_p, antenna2_p;
    const VisBuffer *vb_p;
    Double imRefFreq_p;
    CFBStruct cfBSt_p;
    Bool accumCFs_p;
    Matrix<uInt> BLCXi, BLCYi, TRCXi, TRCYi;
  };

} //# NAMESPACE CASA - END
#endif
