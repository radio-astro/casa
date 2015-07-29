//# CasaImageOpener.cc: A class with static functions to open an image of any type
//# Copyright (C) 2015
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by
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
#include <casa/OS/Directory.h>
#include <casa/OS/File.h>
#include <casa/OS/DirectoryIterator.h>
#include <casa/IO/AipsIO.h>
#include <casa/IO/MemoryIO.h>
#include <casa/IO/RawIO.h>
#include <images/Images/ImageConcat.h>
#include <imageanalysis/IO/CasaImageOpener.h>
#include <tables/Tables/Table.h>
namespace casa{ //

  ImageOpener::ImageTypes CasaImageOpener::imageType (const String& name){

    File elfile(name);
    if(elfile.exists() && elfile.isDirectory()){
      Directory eldir(elfile);
      DirectoryIterator iter(eldir);
      for(iter.reset(); !iter.pastEnd(); iter++){
	File sousFile=iter.file();
	if(sousFile.isRegular()){
	  ImageOpener::ImageTypes testType=ImageOpener::imageType(sousFile.path().expandedName());
	  if(testType==ImageOpener::IMAGECONCAT)
	    return testType;
	}
      }


    }
   
    return ImageOpener::imageType(name);
  }

  LatticeBase* CasaImageOpener::openImage (const String& fileName,
					   const MaskSpecifier& spec){
    if (fileName.empty()) {
      return nullptr;
    }
    LatticeBase* retval=nullptr;
    ImageOpener::ImageTypes type = CasaImageOpener::imageType(fileName);
    if(File(fileName).isDirectory() && type==IMAGECONCAT){
      Directory eldir(fileName);
      DirectoryIterator iter(eldir);
      for(iter.reset(); !iter.pastEnd(); iter++){
	File sousFile=iter.file();
	if(sousFile.isRegular()){
	  ImageOpener::ImageTypes testType=ImageOpener::imageType(sousFile.path().expandedName()); 
	  if(testType==IMAGECONCAT){
	    MemoryIO membuf;
	    RawIO rawio(&membuf);
	    AipsIO newaio(&rawio);
	    AipsIO oldaio(sousFile.path().expandedName());
	    AlwaysAssert (oldaio.getstart("CompoundImage-Conc") == 0, AipsError);
	    Int dtype;
	    oldaio >> dtype;
	    //newaio.putstart("CompoundImage-Conc",0);
	    //newaio << dtype;
	    newaio.putstart("ImageConcat",1);
	    AlwaysAssert (oldaio.getstart ("ImageConcat") == 1, AipsError);
	    uInt axis, nlatt;
	    Bool tmpClose;
	    String subname;
	    oldaio >> axis >> tmpClose >> nlatt;
	    newaio << axis << tmpClose << nlatt;
	     for (uInt i=0; i<nlatt; ++i) {
	       oldaio>> subname;
	       if(File(subname).exists()){
		 newaio << subname;
	       }
	       else{
		 //Image is in subdirectory may be
		 String newsub=fileName+"/"+Path(subname).baseName();
		 if(Table::isReadable(newsub))
		   newaio << newsub;
		 else
		   throw(AipsError("SubImage " + subname + " or " + newsub +" could not be found"));
	       }
	       
	     }
	     oldaio.getend();
	     newaio.putend();
	     newaio.setpos(0);
	     
	     
	     switch (dtype) {
	     case TpFloat:
	       retval = new ImageConcat<Float> (newaio, fileName);
	       break;
	     case TpDouble:
	       retval = new ImageConcat<Double> (newaio, fileName);
	       break;
	     case TpComplex:
	       retval = new ImageConcat<Complex> (newaio, fileName);
	       break;
	     case TpDComplex:
	       retval = new ImageConcat<DComplex> (newaio, fileName);
	       break;
	     default:
	       break;
	     }
	  }

	}
      }


    }
    else{
      retval=ImageOpener::openImage(fileName, spec);
    }
    return retval;
  }

} //# NAMESPACE CASA - END
