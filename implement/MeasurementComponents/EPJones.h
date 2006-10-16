//# EPJones.h: Definition for EPJones matrices
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

#ifndef SYNTHESIS_EPJONES_H
#define SYNTHESIS_EPJONES_H

#include <synthesis/MeasurementComponents/PBWProjectFT.h>
#include <casa/BasicSL/Constants.h>
#include <scimath/Mathematics/SquareMatrix.h>
#include <synthesis/MeasurementComponents/EPTimeVarVisJones.h>
#include <casa/OS/File.h>

#include <casa/Logging/LogMessage.h>
#include <casa/Logging/LogSink.h>
#include <casa/Logging/LogIO.h>

namespace casa {

//# forward
  class VisEquation;
  class PBWProjectFT;
class EPJones : public EPTimeVarVisJones {
public:

  EPJones();
  EPJones(VisSet& vs, PBWProjectFT& ftmachine);
  EPJones(VisSet& vs);
  EPJones(const EPJones& other);
  ~EPJones();
  virtual Bool isSolveable() {return True;};
  virtual Double preavg() {return preavg_;};
  virtual Int refant() {return refant_;};
  void store(const String& file, const Bool& append);
  virtual void load(const String& file, const String& select="", 
		    const String& type="general");
  virtual VisBuffer& apply(VisBuffer& vb,Int Conj=0);
  virtual VisBuffer& apply(VisBuffer& vb,
			   VisBuffer& gradAzVB, 
			   VisBuffer& gradElVB,
			   Int Conj=0);
  virtual VisBuffer& applyInv(VisBuffer& vb) {return vb;};
  virtual void addGradients(const VisBuffer& vb, Int row, const Antenna& a,
			    const Vector<Float>& sumwt, 
			    const Vector<Float>& chisq,
			    const Vector<SquareMatrix<Complex,2> >& c, 
			    const Vector<SquareMatrix<Float,2> >& f);

  virtual Type type() {return EP;};
  virtual Bool freqDep(){return False;};
  virtual Bool solve(class VisEquation &);
  virtual void updateAntGain();
  virtual void setAntPar(Int whichSlot,Array<Float>& Par, Bool solOK=False);
  void setSolverParam(const String& tableName,
		      const Double& integTime,
		      const Double& preAvgTime);


  void setFTMachine(PBWProjectFT* ftmac) {ftmac_p = ftmac;}
  void keep(const Int& slot) ;
  void initializeGradients();

  void initSolveCache();
  void deleteSolveCache();
  void reset();

  Cube<Float>& getPar(const VisBuffer& vb);
  Vector<Bool>& getFlags(const VisBuffer& vb);

protected:
  // Cal table filename
  String calTableName_;

  // Selecttion on cal table
  String calSelect_;

  // Pre-averaging interval for solution
  Double preavg_;

  // Reference antenna for solution
  Int refant_;
  // Indicates which elements of Jones matrix are required
  Matrix<Bool> required_;

  // Fit status
  Float sumwt_;
  Float chisq_;

  // New PB versions
  PtrBlock<Matrix<Bool>*> iSolutionOK_;  // [numberSpw_](numberAnt_,numberSlots_)
  PtrBlock<Matrix<Float>*> iFit_;        // [numberSpw_](numberAnt_,numberSlots_)
  PtrBlock<Matrix<Float>*> iFitwt_;      // [numberSpw_](numberAnt_,numberSlots_)
  
  PtrBlock<Vector<Bool>*> solutionOK_;   // [numberSpw_](numberSlots_)
  PtrBlock<Vector<Float>*> fit_;         // [numberSpw_](numberSlots_)
  PtrBlock<Vector<Float>*> fitwt_;       // [numberSpw_](numberSlots_)

  Matrix< mjJones2 > gS_;     // (nSolnChan,numberAnt_)
  Matrix< mjJones2F > ggS_;   // (nSolnChan,numberAnt_)


  LogSink logSink_p;
  virtual LogSink& logSink() {return logSink_p;};

  LogIO logSink2_p;
  LogIO& logSink2() {return logSink2_p;};

  //  PBFTMachine &ftmac_p;
  PBWProjectFT *ftmac_p;
};
}

#endif
