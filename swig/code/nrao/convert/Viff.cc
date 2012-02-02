//# Viff.cc: Convert between Khoros Viff format and AIPS++ Arrays
//# Copyright (C) 1993,1994,1995,1999,2000
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

//# Includes

#include <aips/Viff.h>
#include <casa/BasicSL/String.h>
#include <casa/Exceptions/Error.h>
#include <casa/Arrays/Vector.h>
#include <casa/BasicSL/Complex.h>
#include <casa/Arrays/IPosition.h>

// Since Viff is C which will manipulate memory with malloc/free
#include <malloc.h>

extern "C" {
#include <viff.h>      // From khoros, for xvimage
    // Infortunately Khoros doesn't have prototypes, so we have to make them
    // "by hand".
    void freeimage(xvimage *);
    xvimage *createimage(unsigned long col_size,
			 unsigned long row_size,
			 unsigned long data_storage_type,
			 unsigned long num_of_images,
			 unsigned long num_data_bands,
			 char *comment,
			 unsigned long map_row_size,
			 unsigned long map_col_size,
			 unsigned long map_scheme,
			 unsigned long map_storage_type,
			 unsigned long location_type,
			 unsigned long location_dim);

    xvimage *readimage(const char *filename);
    int writeimage(const char *filename, xvimage *image);

};


void Viff::setDefaults()
{
    col_size = row_size = num_data_bands = 0;
    data_storage_type = VFF_TYP_FLOAT;
    num_of_images = 1;
    comment = "Created by AIPS++ Viff class\n";
    map_row_size = map_col_size = 0;
    map_scheme = VFF_MS_NONE;
    map_storage_type = VFF_MAPTYP_NONE;
    location_type = VFF_LOC_IMPLICIT;
    location_dim = 0;
}

Viff::Viff() : kimage(0)
{
    setDefaults();
}

Viff::~Viff()
{
    if (kimage) {
	freeimage(kimage);
    }
}


// Copy data from VIFF type 2, pointer 2 to a pointer "p1" which is typed.
// typesafe, but neither is Viff (and this function is not visible, i.e. it's
// pure implementation).
static Bool 
copyData(long type1, void *p1, long type2, void *p2, unsigned long npts)
{

    // Get all the variables defined first; not necessary but it keeps the
    // mega-switch cleaner.
    char *cp1 = (char *)p1; short *sp1 = (short *)p1; Int *ip1 = (Int *)p1;
    float *fp1 = (float *)p1; double *dp1 = (double *)p1;
    char *cp2 = (char *)p2; short *sp2 = (short *)p2; Int *ip2 = (Int *)p2;
    float *fp2 = (float *)p2; double *dp2 = (double *)p2;
    Complex *Cp1 = (Complex *)p1; DComplex *Dp1 = (DComplex *)p1;
    // Viff complex/dcomplex are just float/double streams
    float *Cp2 = (float *)p2; double *Dp2 = (double *)p2;

    // TBF - does not handle complex yet
    switch (type1) {
    case VFF_TYP_1_BYTE:
	switch (type2) {
	case VFF_TYP_1_BYTE: while (npts--) *cp1++ = *cp2++; break;
	case VFF_TYP_2_BYTE: while (npts--) *cp1++ = *sp2++; break;
	case VFF_TYP_4_BYTE: while (npts--) *cp1++ = *ip2++; break;
	case VFF_TYP_FLOAT:  while (npts--) *cp1++ = char(*fp2++); break;
	case VFF_TYP_DOUBLE: while (npts--) *cp1++ = char(*dp2++); break;
	    // Just use the real part
	case VFF_TYP_COMPLEX:  while(npts--) {*cp1++=char(*Cp2++); 
					      Cp2++;} break;
	case VFF_TYP_DCOMPLEX:  while(npts--) {*cp1++=char(*Dp2++); 
					       Dp2++;} break;
	default:
	    return False;
	}
	break;
    case VFF_TYP_2_BYTE:
	switch (type2) {
	case VFF_TYP_1_BYTE:  while (npts--) *sp1++ = *cp2++; break;
	case VFF_TYP_2_BYTE:  while (npts--) *sp1++ = *sp2++; break;
	case VFF_TYP_4_BYTE:  while (npts--) *sp1++ = *ip2++; break;
	case VFF_TYP_FLOAT:   while (npts--) *sp1++ = char(*fp2++); break;
	case VFF_TYP_DOUBLE:  while (npts--) *sp1++ = char(*dp2++); break;
	    // Just use the real part
	case VFF_TYP_COMPLEX:  while(npts--) {*sp1++=short(*Cp2++); 
					      Cp2++;} break;
	case VFF_TYP_DCOMPLEX:  while(npts--) {*sp1++=short(*Dp2++); 
					       Dp2++;} break;
	default:
	    return False;
	}
	break;
    case VFF_TYP_4_BYTE:
	switch (type2) {
	case VFF_TYP_1_BYTE: while (npts--) *ip1++ = *cp2++; break;
	case VFF_TYP_2_BYTE: while (npts--) *ip1++ = *sp2++; break;
	case VFF_TYP_4_BYTE: while (npts--) *ip1++ = *ip2++; break;
	case VFF_TYP_FLOAT:  while (npts--) *ip1++ = int(*fp2++); break;
	case VFF_TYP_DOUBLE: while (npts--) *ip1++ = int(*dp2++); break;
	    // Just use the real part
	case VFF_TYP_COMPLEX:  while(npts--) {*ip1++=int(*Cp2++);
					      Cp2++;} break;
	case VFF_TYP_DCOMPLEX:  while(npts--) {*ip1++=int(*Dp2++);
					       Dp2++;} break;
	default:
	    return False;
	}
	break;
    case VFF_TYP_FLOAT:
	switch (type2) {
	case VFF_TYP_1_BYTE: while (npts--) *fp1++ = *cp2++; break;
	case VFF_TYP_2_BYTE: while (npts--) *fp1++ = *sp2++; break;
	case VFF_TYP_4_BYTE: while (npts--) *fp1++ = *ip2++; break;
	case VFF_TYP_FLOAT:  while (npts--) *fp1++ = *fp2++; break;
	case VFF_TYP_DOUBLE: while (npts--) *fp1++ = *dp2++; break;
	    // Just use the real part
	case VFF_TYP_COMPLEX:  while(npts--) {*fp1++=*Cp2++; Cp2++;} break;
	case VFF_TYP_DCOMPLEX:  while(npts--) {*fp1++=*Dp2++; Dp2++;} break;
	default:
	    return False;
	}
	break;
    case VFF_TYP_DOUBLE:
	switch (type2) {
	case VFF_TYP_1_BYTE: while (npts--) *dp1++ = *cp2++; break;
	case VFF_TYP_2_BYTE: while (npts--) *dp1++ = *sp2++; break;
	case VFF_TYP_4_BYTE: while (npts--) *dp1++ = *ip2++; break;
	case VFF_TYP_FLOAT:  while (npts--) *dp1++ = *fp2++; break;
	case VFF_TYP_DOUBLE: while (npts--) *dp1++ = *dp2++; break;
	    // Just use the real part
	case VFF_TYP_COMPLEX:  while(npts--) {*dp1++=*Cp2++; Cp2++;} break;
	case VFF_TYP_DCOMPLEX:  while(npts--) {*dp2++=*Dp2++; Dp1++;} break;
	default:
	    return False;
	}
	break;
    case VFF_TYP_COMPLEX:
	switch (type2) {
	case VFF_TYP_1_BYTE:
	    while (npts--) {*Cp1++ = Complex(*cp2, 0.0); cp2 += 1; } break;
	case VFF_TYP_2_BYTE:
	    while (npts--) {*Cp1++ = Complex(*sp2, 0.0); sp2 += 1; } break;
	case VFF_TYP_4_BYTE:
	    while (npts--) {*Cp1++ = Complex(*ip2, 0.0); ip2 += 1; } break;
	case VFF_TYP_FLOAT:
	    while (npts--) {*Cp1++ = Complex(*fp2, 0.0); fp2 += 1; } break;
	case VFF_TYP_DOUBLE:
	    while (npts--) {*Cp1++ = Complex(*dp2, 0.0); dp2 += 1; } break;
	case VFF_TYP_COMPLEX:
	    while (npts--) {*Cp1++ = Complex(*Cp2, *(Cp2+1)); Cp2 += 2; } break;
	case VFF_TYP_DCOMPLEX:
	    while (npts--) {*Cp1++ = Complex(*Dp2, *(Dp2+1)); Dp2 += 2; } break;
	default:
	    return False;
	}
	break;
    case VFF_TYP_DCOMPLEX:
	switch (type2) {
	case VFF_TYP_1_BYTE:
	    while (npts--) {*Dp1++ = DComplex(*cp2, 0.0); cp2 += 1; } break;
	case VFF_TYP_2_BYTE:
	    while (npts--) {*Dp1++ = DComplex(*sp2, 0.0); sp2 += 1; } break;
	case VFF_TYP_4_BYTE:
	    while (npts--) {*Dp1++ = DComplex(*ip2, 0.0); ip2 += 1; } break;
	case VFF_TYP_FLOAT:
	    while (npts--) {*Dp1++ = DComplex(*fp2, 0.0); fp2 += 1; } break;
	case VFF_TYP_DOUBLE:
	    while (npts--) {*Dp1++ = DComplex(*dp2, 0.0); dp2 += 1; } break;
	case VFF_TYP_COMPLEX:
	    while (npts--) {*Dp1++ =DComplex(*Cp2, *(Cp2+1)); Cp2 += 2; } break;
	case VFF_TYP_DCOMPLEX:
	    while (npts--) {*Dp1++ =DComplex(*Dp2, *(Dp2+1)); Dp2 += 2; } break;
	default:
	    return False;
	}
	break;
    default:
	return False;
    }
    return True;
}

// If array is > 3D, it will try to make it nonDegenerate() before giving up.
Bool Viff::put(const Array<float> &array)
{
    // We could be cheaper than this - extra copy made
    Array<float> image;
    image = array;
    if (image.ndim() > 3) {
	image = image.nonDegenerate();
    }
    if (image.ndim() > 3) {
	return False;   // Could not make into a <=3-D image
    }

    // If we already have a viff image delete it and create a new one
    if (kimage) {
	freeimage(kimage);
    }

    // OK, let's create it.
    switch (image.ndim()) {
    case 0:
	col_size = row_size = num_data_bands = 0;
	break;
    case 1:
	col_size = num_data_bands = 1;
	row_size = array.shape()(0);
	break;
    case 2:
	num_data_bands = 1;
	row_size = array.shape()(0);
	col_size = array.shape()(1);
	break;
    case 3:
	row_size = array.shape()(0);
	col_size = array.shape()(1);
	num_data_bands = array.shape()(2);
	break;
    default:
	// Should never happen
	throw(AipsError("Viff::put - impossible dimensionality"));
    }
    // OK, let's make it.
    kimage = createimage(col_size, row_size, data_storage_type,
			 num_of_images, num_data_bands, comment,
			 map_row_size, map_col_size, map_scheme,
			 map_storage_type, location_type, location_dim);
    if (kimage == 0) {
	return False; // Could not allocate
    }

    // OK, we have an allocated data structure, let's write into it.
    Bool deleteIt;
    float *storage = image.getStorage(deleteIt);  
    if (copyData(kimage->data_storage_type, kimage->imagedata, 
		 VFF_TYP_FLOAT, storage, image.nelements()) == False) {
	throw(AipsError("Viff::put - unknown type"));
    }
    if (deleteIt) {
	delete [] storage;
    }
    return True;
}

// There's a lot of code duplication here we should try to eliminate.
// If array is > 3D, it will try to make it nonDegenerate() before giving up.
Bool Viff::put(const Array<Complex> &array)
{
    // We could be cheaper than this - extra copy made
    Array<Complex> image;
    image = array;
    if (image.ndim() > 3) {
	image = image.nonDegenerate();
    }
    if (image.ndim() > 3) {
	return False;   // Could not make into a <=3-D image
    }

    // If we already have a viff image delete it and create a new one
    if (kimage) {
	freeimage(kimage);
    }

    // OK, let's create it.
    switch (image.ndim()) {
    case 0:
	col_size = row_size = num_data_bands = 0;
	break;
    case 1:
	col_size = num_data_bands = 1;
	row_size = array.shape()(0);
	break;
    case 2:
	num_data_bands = 1;
	row_size = array.shape()(0);
	col_size = array.shape()(1);
	break;
    case 3:
	row_size = array.shape()(0);
	col_size = array.shape()(1);
	num_data_bands = array.shape()(2);
	break;
    default:
	// Should never happen
	throw(AipsError("Viff::put - impossible dimensionality"));
    }
    // OK, let's make it.
    data_storage_type = VFF_TYP_COMPLEX;
    kimage = createimage(col_size, row_size, data_storage_type,
			 num_of_images, num_data_bands, comment,
			 map_row_size, map_col_size, map_scheme,
			 map_storage_type, location_type, location_dim);
    if (kimage == 0) {
	return False; // Could not allocate
    }

    // OK, we have an allocated data structure, let's write into it.
    Bool deleteIt;
    Complex *storage = image.getStorage(deleteIt);  
    if (copyData(kimage->data_storage_type, kimage->imagedata, 
		 VFF_TYP_COMPLEX, storage, image.nelements()) == False) {
	throw(AipsError("Viff::put - unknown type"));
    }
    image.freeStorage(storage, deleteIt);
    return True;
}

// If "name" starts with a "-" write to stdout, if it begins with a "#" write
// to stderr.
Bool Viff::write(String name)
{
    if (kimage == 0) {
	return False;
    }

    if (writeimage(name, kimage)) {
	return True;
    }
    return False;
}

// If "name" begins with a "-" read from stdin.
Bool Viff::read(String name)
{
    // If we already have a viff image delete it and create a new one
    if (kimage) {
	freeimage(kimage);
    }
    kimage = readimage(name);
    if (kimage == 0) {
	return False;
    }
    return True;
}


// Leaves array unchanged in the event of an error (False return). Either the
// array is 0-sized, in which case it is resized appropriately, or it is
// conformant in which case it has to "fit", with the exception that only
// non-degenerate axes are considered.
Bool Viff::get(Array<float> &array)
{
    if (kimage == 0) {
	return False;
    }

    Array<float> tmp;
    // Make an array the same size as the kimage
    unsigned long nx = kimage->row_size;
    unsigned long ny = kimage->col_size;
    unsigned long nz = kimage->num_data_bands;
    IPosition size(3);
    size(0) = Int(nx); size(1) = Int(ny); size(2) = Int(nz);
    
    // OK, make the tmp array the same size as the Viff "cube"
    tmp.resize(size);

    // And copy the Viff data into it
    Bool deleteIt;
    float *storage = tmp.getStorage(deleteIt);
    Bool ok = copyData(VFF_TYP_FLOAT, storage, kimage->data_storage_type,
		  kimage->imagedata, tmp.nelements());
    tmp.putStorage(storage, deleteIt);
    if (! ok) {
	return False;
    }

    Array<float> tmpNonDegenerate(tmp.nonDegenerate());

    // Now we attempt to copy
    // First, if array is zero sized then just resize it (to the non-degenerate
    // size).
    if (array.nelements() == 0) {
	array.resize(tmpNonDegenerate.shape());
    }

    // Are we the "minimum" size
    if (array.conform(tmpNonDegenerate)) {
	// Yes - assign
	array = tmpNonDegenerate;
    } else if (array.conform(tmp)) {
	// We're the degenerate size
	array = tmp;
    } else {
	// We just don't conform
	return False;
    }

    return True;
}

// A lot of code duplication
Bool Viff::get(Array<Complex> &array)
{
    if (kimage == 0) {
	return False;
    }

    Array<Complex> tmp;
    // Make an array the same size as the kimage
    unsigned long nx = kimage->row_size;
    unsigned long ny = kimage->col_size;
    unsigned long nz = kimage->num_data_bands;
    IPosition size(3);
    size(0) = Int(nx); size(1) = Int(ny); size(2) = Int(nz);
    
    // OK, make the tmp array the same size as the Viff "cube"
    tmp.resize(size);

    // And copy the Viff data into it
    Bool deleteIt;
    Complex *storage = tmp.getStorage(deleteIt);
    Bool ok = copyData(VFF_TYP_COMPLEX, storage, kimage->data_storage_type,
		  kimage->imagedata, tmp.nelements());
    tmp.putStorage(storage, deleteIt);
    if (! ok) {
	return False;
    }

    Array<Complex> tmpNonDegenerate(tmp.nonDegenerate());

    // Now we attempt to copy
    // First, if array is zero sized then just resize it (to the non-degenerate
    // size).
    if (array.nelements() == 0) {
	array.resize(tmpNonDegenerate.shape());
    }

    // Are we the "minimum" size
    if (array.conform(tmpNonDegenerate)) {
	// Yes - assign
	array = tmpNonDegenerate;
    } else if (array.conform(tmp)) {
	// We're the degenerate size
	array = tmp;
    } else {
	// We just don't conform
	return False;
    }

    return True;
}

uInt Viff::nx() const
{
    if (kimage == 0) {
	return 0;
    }
    return kimage->row_size;
}

uInt Viff::ny() const
{
    if (kimage == 0) {
	return 0;
    }
    return kimage->col_size;
}

uInt Viff::nz() const
{
    if (kimage == 0) {
	return 0;
    }
    return kimage->num_data_bands;
}

Bool Viff::putLocations(const Cube<float> &locations)
{
    if (kimage == 0) {
	return False;
    }

    // We could be smarter than this
    Cube<float> places = locations;
    Int cubex, cubey, cubez;
    places.shape(cubex, cubey, cubez);
    // Are the locations conformant with this Viff object?
    if (cubex != nx() || cubey != ny()) {
	return False;
    }

    // If we already have locations, free them.
    if (kimage->location != 0) {
	free((char *)kimage->location);
    }

    // Try to make enough space for the locations. We could be a bit smarter and
    // see if the existing location storage is the right size, we could save some
    // free/malloc's.
    kimage->location = (float *)malloc(sizeof(float)*cubex*cubey);
    if (kimage->location == 0) {
	kimage->location_type = VFF_LOC_IMPLICIT;
	return False;
    }

    // And set up the location information
    kimage->location_type = VFF_LOC_EXPLICIT;
    kimage->location_dim  = cubez;

    // Now copy the positions
    Bool deleteIt;
    float *storage;
    storage = places.nonDegenerate().getStorage(deleteIt);
    uInt ntotal = cubex*cubey*cubez;
    float *klocation = kimage->location;
    while (ntotal--) {
	*klocation++ = *storage++;
    }
    
    if (deleteIt) {
	delete [] storage;
    }

    return True;
}
