//# <VLATapeIO.h>: this defines <VLATapeIO>, which ...
//# Copyright (C) 1997,1999,2002
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
//#
//# $Id$
//#! ========================================================================
//#!                Attention!  Programmers read this!
//#!
//#! This file is a template to guide you in creating a header file
//#! for your new class.   By following this template, you will create
//#! a permanent reference document for your class, suitable for both
//#! the novice client programmer, the seasoned veteran, and anyone in 
//#! between.  It is essential that you write the documentation portions 
//#! of this file with as much care as you do the source code.
//#!
//#! If you are unfamilar with the AIPS++ header style please refer to
//#! template-class-h.
//#!
//#!                         Replacement Tokens
//#!                         ------------------
//#!
//#! These are character strings enclosed in angle brackets, on a commented
//#! line.  Two are found on the first line of this file:
//#!
//#!   <ClassFileName.h> <ClassName>
//#!
//#! You should remove the angle brackets, and replace the characters within
//#! the brackets with names specific to your class.  Mimic the capitalization
//#! and punctuation of the original.  For example, you would change
//#!
//#!   <ClassFileName.h>  to   LatticeIter.h
//#!   <ClassName>        to   LatticeIterator
//#!
//#! Another replacement token will be found in the "include guard" just
//#! a few lines below.
//#!
//#!  #define <AIPS_CLASSFILENAME_H>  to  #define AIPS_LATTICEITER_H
//#!

#ifndef NRAO_VLATAPEIO_H
#define NRAO_VLATAPEIO_H

//# Forward Declarations

// <summary>
// This class this defines VLATapeIO
// </summary>

// <use visibility=local>   or   <use visibility=export>

// <reviewed reviewer="" date="yyyy/mm/dd" tests="" demos="">
// </reviewed>

// <prerequisite>
//   <li> SomeClass
//   <li> SomeOtherClass
//   <li> some concept
// </prerequisite>
//
// <etymology>
// </etymology>
//
// <synopsis>
// </synopsis>
//
// <example>
// </example>
//
// <motivation>
// </motivation>
//
// <templating arg=T>
//    <li>
//    <li>
// </templating>
//
// <thrown>
//    <li>
//    <li>
// </thrown>
//
// <todo asof="yyyy/mm/dd">
//   <li> add this feature
//   <li> fix this bug
//   <li> start discussion of this possible extension
// </todo>



#include <casa/aips.h>
#include <casa/BasicSL/String.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <casa/namespace.h>
// kludge for tapes until we do something specific.
//  It''s pretty much a thin vaneer that sits on top of rmt.

Int rmtopen(casacore::Char *, casacore::Int, casacore::Int);
Int rmtread(casacore::Int, casacore::Char *, casacore::uInt);
Int rmtwrite(casacore::Int, casacore::Char *, casacore::uInt);
Int rmtaccess(casacore::Char *, casacore::uInt);
Int rmtclose(casacore::Int);

class VLATapeIO {
   public :
      VLATapeIO() : tapeId(-1){}
      VLATapeIO(const casacore::String &a) : inOut(O_RDONLY), tPosition(0){
                                tapeId = open((casacore::Char *)(a.chars()), inOut, tPosition);
                                }
      VLATapeIO(const casacore::String &a, casacore::Int rwFlag, casacore::Int tpos) : inOut(rwFlag),
                                                      tPosition(tpos){
                              tapeId = open((casacore::Char *)(a.chars()), inOut, tPosition);
                              }
     ~VLATapeIO(){if(tapeId != -1)rmtclose(tapeId);}
      casacore::Int  open(const casacore::String &a, casacore::Int b, casacore::Int c){
                tapeId = rmtopen((casacore::Char *)(a.chars()), b, c);
                return tapeId;}
      casacore::Int  read(casacore::Char *buf, casacore::uInt nsize){return rmtread(tapeId, buf, nsize);}
      //casacore::Int  write(casacore::Char *buf, casacore::uInt nsize){return rmtwrite(tapeId, buf, nsize);}
      casacore::Int  access(const casacore::Char *a, casacore::uInt b){return rmtaccess((casacore::Char *)a, b);}
      casacore::Int close(){return rmtclose(tapeId);}
   private :
      casacore::Int inOut;
      casacore::Int tapeId;
      casacore::Int tPosition;
};
#endif
