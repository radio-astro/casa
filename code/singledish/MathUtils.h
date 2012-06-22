//#---------------------------------------------------------------------------
//# MathUtilities.h: General math operations
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
//# $Id:
//#---------------------------------------------------------------------------
#ifndef MATHUTILS_H
#define MATHUTILS_H

#include <string>
#include <vector>
#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/String.h>

namespace mathutil {

// Add new entry to list
template <class T>
casa::uInt addEntry(casa::Vector<T>& list, T val);

// Find the scan boundaries from a list of ScanIDs
void scanBoundaries(casa::Vector<casa::uInt>& startInt,  
		    casa::Vector<casa::uInt>& endInt,
		    const casa::Vector<casa::Int>& scanIDs);

// Hanning smoothing
template <class T>
void hanning(casa::Vector<T>& out, casa::Vector<casa::Bool>& outmask, 
	     const casa::Vector<T>& in, const casa::Vector<casa::Bool>& mask, 
	     casa::Bool relaxed=casa::False, 
	     casa::Bool ignoreOther=casa::False);

// Generate specified statistic
float statistics(const casa::String& which,  
		 const casa::MaskedArray<casa::Float>& data);

// Replace masked value by zero
void replaceMaskByZero(casa::Vector<casa::Float>& data, 
		       const casa::Vector<casa::Bool>& mask);

// Extend the 
template <class T>
void extendLastArrayAxis(casa::Array<T>& out, const casa::Array<T>& in,
			 const T& initVal);
  
std::vector<std::string> tovectorstring(const casa::Vector<casa::String>& in);

casa::Vector<casa::String> toVectorString(const std::vector<std::string>& in);

};

#endif
