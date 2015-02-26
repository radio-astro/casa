//#---------------------------------------------------------------------------
//# STFillerWrapper.h: Wrapper classes to use CountedPtr
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
//# $Id: STFillerWrapper.h 1353 2007-04-26 04:55:17Z mar637 $
//#---------------------------------------------------------------------------
#ifndef STFILLERWRAPPER_H
#define STFILLERWRAPPER_H

#include <string>

#include "ScantableWrapper.h"
#include "STFiller.h"

namespace asap {

class STFillerWrapper : public STFiller {
public:
  STFillerWrapper() {;}
  explicit STFillerWrapper(const std::string& filename, int ifSel=-1, int beamSel=-1) :
     STFiller(filename, ifSel, beamSel) {;}
  explicit STFillerWrapper(ScantableWrapper tbl) :
    STFiller(tbl.getCP()){;}
  ScantableWrapper getScantable() const {
    return ScantableWrapper(getTable());
  }
};

} // namespace
#endif
