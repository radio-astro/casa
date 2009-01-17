//# NFRA_MS.h: header file for NFRA_MS info class
//# Copyright (C) 1998,1999,2000,2001,2002,2003
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

#ifndef NFRA_MSshow_H
#define NFRA_MSshow_H


#include <nfra/Wsrt/NFRA_MS.h>

class NFRA_MSshow : public NFRA_MS {

  void showStd();
  void showStd(vector<NFRA_BaseLine>);
  void showStd(vector<NFRA_PW>);
  void showStd(vector<NFRA_PosMos>);
  void showStd(vector<NFRA_FW>);
  void showStd(vector<NFRA_FreqMos>);
  void showStd(vector<NFRA_IVC>);
  void showStd(NFRA_MFFE);
  void showStd(NFRA_Position);
  void showStdOnePosMos(NFRA_PosMos);
  void showStdPosMos(NFRA_PosMos);

  void showReadme();
  void showReadme(vector<NFRA_PW>);
  void showReadme(vector<NFRA_PosMos>, String);
  void showReadme(vector<NFRA_Position>, String);
  void showReadme(vector<NFRA_FW>);
  void showReadme(vector<NFRA_FreqMos>, String);
  void showReadme(vector<NFRA_IVC>, String);
  void showReadme(NFRA_MFFE M, String);

  void showArch();
  double HaMinCentroid(){return 0.0;}
  double HaMaxCentroid(){return 0.0;}
  double RACentroid(){return 0.0;}
  double DECCentroid(){return 0.0;}


 public:

  void show(String);

};

#endif
