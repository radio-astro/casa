//# Viff.h: Convert between Khoros Viff format and AIPS++ Arrays
//# Copyright (C) 1993,1994,1995,2000
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

#ifndef NRAO_VIFF_H
#define NRAO_VIFF_H

// .LIB(aips)

// This class encapsulates the Khoros "Viff" format. It will normally be
// used to interconvert AIPS++ in-memory arrayss and Viff disk files. The
// primary limitation of the Viff format is that it can only deal with up-to
// 3-dimensional arrays. If an array of greater than 3-dimensions is put into
// a Viff structure an error is returned (first, however an attempt is made
// to remove degenerate (length==1) axes).

// All Viff types are handled, and conversions will occur automatically.
// The conversions use the normal C++ assignments, e.g. converting from floats
// to bytes is not apt to be very meaningful (maybe we should put in automatic
// scalings?).

// TBF - At the moment the user array types are only floats, although any underlying
// Viff format is supported, except for the complex types.

#include <casa/aips.h>

// These should be forward declared; how do you do that with templates?
#include <casa/Arrays/Array.h>
#include <casa/Arrays/Cube.h>
#include <casa/BasicSL/Complex.h>

struct xvimage; // From Khoros

class Viff {
public:
    enum {BYTE, SHORT, INTEGER, FLOAT, DOUBLE, COMPLEX, DCOMPLEX};
    Viff();

    ~Viff();

    // Put casacore::Array (or any class derived from it, casacore::Vector, casacore::Matrix, Cube...)
    // into this Viff structure. Non-degenerate arrays of dimension >=4
    // will cause a "false" to be returned, since Viff images are restricted
    // to 3 dimensions. When the Viff type and the casacore::Array types differ,
    // a conversion will be done. casacore::Complex types are turned into real and
    // integer by taking the real component.
    casacore::Bool put(const casacore::Array<float> &array);
    casacore::Bool put(const casacore::Array<casacore::Complex> &array);

    // Get the casacore::Array out of this Viff structre. If the Viff structure isn't 
    // initialized it will return a 0-sized array and set the flag to false.
    // This will convert from the internal Viff type to the desired casacore::Array<T>
    // type. casacore::Complex types are turned into real
    // and integer types by taking the real component.
    casacore::Bool get(casacore::Array<float> &array);
    casacore::Bool get(casacore::Array<casacore::Complex> &array);

    // Read the Viff file specified by name; return false if it fails
    // (file does not exist, no permissions, etc).
    casacore::Bool read(casacore::String name);

    // Write the current Viff structure to a file; create a 0-sized Viff
    // image if this object has not been previously filled (by read or by
    // put). Return true if this succeeds, false otherwise. Note that this
    // will overwrite an existing file.
    casacore::Bool write(casacore::String name);

    // Viff can be thought of as always containing a "3D" data structure, and
    // a vector is when only one of the dimentions is non-unity. nx(), ny() and
    // nx() return 0 if the Viff object isn't defined.
    casacore::uInt nx() const;
    casacore::uInt ny() const;
    casacore::uInt nz() const;

    // We can assign an N dimensional location to each (x,y) position in
    // the Viff object (yes, this is strange - ask the authors of Viff), 
    // i.e. we can assign nx()*ny() N-dimensional locations. Fundamentally
    // this is done by giving a casacore::Cube<float> of values where the first two
    // indices in the casacore::Cube correspond to the (x,y) in the Viff structure, and
    // the Z-depth of the cube corresponds to the dimensionality of the
    // positions we want to assign.
    // Because a casacore::Cube can be constructed from a casacore::Vector, to create a file which
    // contains a y vs x vector plot, you merely have to do something like:
    // viff.put(y); viff.putLocations(x);
    casacore::Bool putLocations(const casacore::Cube<float> &);

private:
    // These members are the arguments to create image;
    void setDefaults();
    unsigned long col_size, row_size, num_data_bands; // shape parameters
    unsigned long data_storage_type;                  // Defaults to float
    unsigned long num_of_images;                      // always 1
    char *comment;
    unsigned long map_row_size, map_col_size,         // Maps aren't used
              map_scheme, map_storage_type;
    unsigned long location_type, location_dim;

    xvimage *kimage;
};

#endif
