//# RFASelector.h: this defines RFASelector
//# Copyright (C) 2000,2001
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
#ifndef FLAGGING_RFAAPPLYFLAGS_H
#define FLAGGING_RFAAPPLYFLAGS_H

#include <flagging/Flagging/RFAFlagCubeBase.h> 
#include <flagging/Flagging/FlagIndex.h>

#include <casa/Containers/RecordInterface.h>


namespace casa { //# NAMESPACE CASA - BEGIN

class RFAApplyFlags : public RFAFlagCubeBase {
  public:
    RFAApplyFlags ( RFChunkStats &ch,const RecordInterface &parm ); 
    virtual ~RFAApplyFlags ();

  static const RecordInterface & getDefaults();
  
  virtual void init ();

  virtual uInt estimateMemoryUse ();
  virtual Bool newChunk ( Int &maxmem );
  virtual RFABase::IterMode iterTime ( uInt it );
  virtual RFABase::IterMode iterRow  ( uInt ir );
  virtual void iterFlag(uInt it);
  virtual void startData (bool verbose);
  virtual void startDry (bool verbose);
  virtual void startFlag (bool verbose);
  virtual void endChunk ();
  virtual void endFlag ();
  virtual RFABase::IterMode endData();
  virtual RFABase::IterMode endDry();

  //  virtual RFABase::IterMode iterTime ( uInt itime ) ;

  // iterRow() is called once per each row in the VisBuffer.
  // Iterating over rows is perhaps preferrable in terms of performance,
  // at least for data iterations.
  //  virtual RFABase::IterMode iterRow  ( uInt irow ) ;

  //  virtual String getDesc ();
  //  static const RecordInterface & getDefaults ();


  static void setIndices(const std::vector<FlagIndex> *fi);

private:
  static const std::vector<FlagIndex> * flagIndex;
  
};

} //# NAMESPACE CASA - END

#endif
