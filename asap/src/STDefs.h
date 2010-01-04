//#---------------------------------------------------------------------------
//# STDefs.h: Global definitions
//#---------------------------------------------------------------------------
//# Copyright (C) 2004
//# ATNF
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but
//# WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
//# Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning this software should be addressed as follows:
//#        Internet email: Malte.Marquarding@csiro.au
//#        Postal address: Malte Marquarding,
//#                        Australia Telescope National Facility,
//#                        P.O. Box 76,
//#                        Epping, NSW, 2121,
//#                        AUSTRALIA
//#
//# $Id: STDefs.h 1388 2007-07-26 16:02:13Z TakTsutsumi $
//#---------------------------------------------------------------------------

#ifndef ASAPDEFS_H
#define ASAPDEFS_H

namespace asap {
  enum AxisNo { BeamAxis=0,
                IFAxis,
                PolAxis,
                ChanAxis,
                nAxes};

  enum Instrument {UNKNOWNINST=0,
                   ALMA,
                   ATPKSMB,
                   ATPKSHOH,
                   ATMOPRA,
                   TIDBINBILLA,
                   CEDUNA,
                   GBT,
                   HOBART,
                   N_INSTRUMENTS};

  enum FeedPolType {UNKNOWNFEED, LINEAR, CIRCULAR, N_POL};

  enum WeightType {NONE=0, VAR, TSYS, TINT, TINTSYS, N_WEIGHTTYPES};

  enum TableType {MEMORY=0, PERSISTENT};



  const std::string SEPERATOR =
   "--------------------------------------------------------------------------------";
}

#endif
