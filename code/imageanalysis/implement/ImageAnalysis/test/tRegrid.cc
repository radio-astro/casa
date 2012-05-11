//# tRegrid.cc:  test the regridding using the ImageAnalysis::regrid( ) functions
//# Copyright (C) 2012
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or(at your option)
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
//# $Id: $

#include <images/Images/ImageUtilities.h>
#include <images/Images/FITSImage.h>
#include <images/Images/PagedImage.h>
#include <images/Images/ImageAnalysis.h>

#include <casa/OS/Directory.h>
#include <casa/OS/EnvVar.h>

#include <casa/namespace.h>

int main() {

    PagedImage<Float> *primary = new PagedImage<Float>( "g35.03_II_nh3_11.hline.image", TableLock::AutoNoReadLocking );
    PagedImage<Float> *secondary = new PagedImage<Float>( "g35_sma_usb_12co.image", TableLock::AutoNoReadLocking );
    ImageAnalysis ia(secondary);

    ia.regrid( "g35_sma_usb_12co.regrid.image", primary, "linear" );

}

