//# Mueller.cc: Implementation of Mueller
//# Copyright (C) 1996,1997,2000,2001,2002,2003
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
//# Correspondence concerning AIPS++ should be adressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//#

#include <synthesis/MeasurementComponents/Mueller.h>
#include <synthesis/MeasurementComponents/Jones.h>
#include <synthesis/MeasurementComponents/VisVector.h>
#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>
#include <casa/iostream.h>
#include <casa/Exceptions/Error.h>

#include <casa/namespace.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Constructor
Mueller::Mueller() : 
  m0_(NULL), 
  m_(NULL), 
  mi_(NULL), 
  cOne_(1.0), 
  vtmp_(VisVector::Four,True)
{}

// Formation from Jones matrix outer product: General version
void Mueller::fromJones(const Jones& jones1, const Jones& jones2) {
  mi_=m_;
  Int i,j,io2,jo2,im2,jm2;
  for (i=0;i<4;++i) {
    io2=(i/2)*2;
    im2=(i%2)*2;
    for (j=0;j<4;++j,++mi_){
      jo2=(j/2);
      jm2=(j%2);
      (*mi_) = jones1.j_[io2+jo2]*conj(jones2.j_[im2+jm2]);
    }
  }
}

// In-place multiply onto a VisVector: General version
void Mueller::apply(VisVector& v) {

  switch (v.type()) {
  case VisVector::Four: {
    vtmp_ = v;                          // remember input (necessarily)
              //  IS VV COPY COMPLETE?
    mi_=m_;                             // iteration within the current matrix
    Complex *vo,*vi;
    Complex tc;
    vo=v.v_;
    Int i,j;
    for (j=0;j<4;++j,++vo) {
      vi=vtmp_.v_;
      (*vo) = (*vi);
      (*vo) *= (*mi_);
      ++mi_;
      ++vi;
      for (i=1;i<4;++i,++mi_,++vi) {          // 
	tc = (*vi);
	tc *= (*mi_);
	(*vo) += (tc);   // ((*mi_)*(*vi));
      }
    }
    break;
  }
  default:
    throw(AipsError("Mueller/VisVector type mismatch in apply"));
    break;
  }  
}

Bool Mueller::invert() {

  throw(AipsError("Invalid attempt to directly invert a general Mueller."));
  
  return False;
}

// Multiply onto a vis VisVector, preserving input (copy then in-place apply)
void Mueller::apply(VisVector& out, const VisVector& in) {
  out=in;
  apply(out);
}

void Mueller::zero() {
  mi_=m_;
  for (Int i=0;i<16;++i,++mi_)
    *mi_=0.0;
}

// Constructor
MuellerDiag::MuellerDiag() : Mueller() {}

// Formation from Jones matrix outer product: optimized Diagonal version
void MuellerDiag::fromJones(const Jones& jones1, const Jones& jones2) {
  mi_=m_;
  for (Int i=0;i<4;++i,++mi_) {
    (*mi_) = jones1.j_[i/2]*conj(jones2.j_[i%2]);
  }
}

Bool MuellerDiag::invert() {
  mi_=m_;
  for (Int i=0;i<4;++i,++mi_) 
    if (abs(*mi_)>0.0)
      (*mi_) = cOne_/(*mi_);
    else {
      zero();
      return False;
    }

  // All ok if we reach here
  return True;
  
}


// In-place multiply onto a VisVector: optimized Diagonal version
void MuellerDiag::apply(VisVector& v) {

  mi_=m_;
  Complex *vi(v.v_);

  switch (v.type()) {
  case VisVector::Four: {
    // element-by-element apply of Mueller diagonal to VisVector 4-vector
    for (Int i=0;i<4;++i,++mi_,++vi) (*vi)*=(*mi_);
    break;
  }
  case VisVector::Two: {
    // Mueller corner elements apply to VisVector 2-vector
    for (Int i=0;i<2;++i,++vi,mi_+=3) (*vi)*=(*mi_);
    break;
  }
  case VisVector::One: {
    // Mueller corner element used as scalar (pol-sensitivity TBD)
    (*vi)*=(*mi_);
    break;
  }
  }
}

void MuellerDiag::zero() {
  mi_=m_;
  for (Int i=0;i<4;++i,++mi_)
    *mi_=0.0;
}

// Constructor
MuellerDiag2::MuellerDiag2() : MuellerDiag() {}
 
// Formation from Jones matrix outer product: optimized Diag2 version
void MuellerDiag2::fromJones(const Jones& jones1, const Jones& jones2) {
  mi_=m_;
  for (Int i=0;i<2;++i,++mi_) {
    (*mi_) = jones1.j_[i]*conj(jones2.j_[i]);
  }
}

Bool MuellerDiag2::invert() {
  mi_=m_;
  for (Int i=0;i<2;++i,++mi_) 
    if (abs(*mi_)>0.0)
      (*mi_) = cOne_/(*mi_);
    else {
      zero();
      return False;
    }

  // All ok if we reach here
  return True;
  
}

// In-place multiply onto a VisVector: optimized Diag2 version
void MuellerDiag2::apply(VisVector& v) {

  switch (v.type()) {
  case VisVector::Four: 
    // Apply of "corners-only" Mueller to VisVector 4-vector (x-hands zeroed)
    v.v_[0]*=m_[0];
    v.v_[1]*=0.0;
    v.v_[2]*=0.0;
    v.v_[3]*=m_[1];
    break;
  case VisVector::Two: 
    // Element-by-element apply of "corners-only" Mueller to VisVector 2-vector
    for (Int i=0;i<2;++i) v.v_[i]*=m_[i];
    break;
  case VisVector::One: {
    // Mueller corner element used as scalar (pol-sensitivity TBD)
    v.v_[0]*=(*m_);
    break;
  }
  }
 
}

void MuellerDiag2::zero() {
  mi_=m_;
  for (Int i=0;i<2;++i,++mi_)
    *mi_=0.0;
}

// Constructor
MuellerScal::MuellerScal() : MuellerDiag() {}

// Formation from Jones matrix outer product: optimized Scalar version
void MuellerScal::fromJones(const Jones& jones1, const Jones& jones2) {
  (*m_) = (*jones1.j_)*conj(*jones2.j_);
}

Bool MuellerScal::invert() {
  if (abs(*m_)>0.0)
    (*m_) = cOne_/(*m_);
  else {
    zero();
    return False;
  }

  // All ok if we reach here
  return True;

}

// In-place multiply onto a VisVector: optimized Scalar version
void MuellerScal::apply(VisVector& v) {

  // Apply single value to all vector elements
  for (Int i=0;i<v.vistype_;i++) v.v_[i]*=(*m_);
}

void MuellerScal::zero() {
    *m_=0.0;
}


// GLOBALS---------------------------

Mueller* createMueller(const Mueller::MuellerType& mtype) {
  Mueller *m(NULL);
  switch (mtype) {
  case Mueller::General:
    m = new Mueller();
    break;
  case Mueller::Diagonal:
    m = new MuellerDiag();
    break;
  case Mueller::Diag2:
    m = new MuellerDiag2();
    break;
  case Mueller::Scalar:
    m = new MuellerScal();
    break;
  }
  return m;
}

// Return the enum for from an int
Mueller::MuellerType muellerType(const Int& n) {
  switch (n) {
  case 1:
    return Mueller::Scalar;
  case 2:
    return Mueller::Diag2;
  case 4:
    return Mueller::Diagonal;
  case 16:
    return Mueller::General;
  default:
    throw(AipsError("Bad MuellerType."));

  }

}
Mueller::MuellerType muellerType(const Jones::JonesType& jtype,const VisVector::VisType& vtype) {
  switch(jtype) {
  case Jones::General:
    switch(vtype) {
    case VisVector::Four:
      return Mueller::General;
      break;
    default:
      throw(AipsError("Cannot apply General calibration matrices to incomplete visibilities."));
      // TBD: Must cater for the case of some spws with differing VisVector VisTypes
    }
    break;
  case Jones::Diagonal:
    switch(vtype) {
    case VisVector::Four:
      return Mueller::Diagonal;
      break;
    default:
      return Mueller::Diag2;
      break;
    }
    break;
  case Jones::Scalar:
    return Mueller::Scalar;
    break;
  }
  // Signature consistency (can't reach here)
  return Mueller::General;
}


// print out a Mueller matrix (according to type)
ostream& operator<<(ostream& os, const Mueller& mat) {

  Complex *mi;
  mi=mat.m_;

  switch (mat.type()) {
  case Mueller::General:
    cout << "General Mueller: " << endl;;
    for (Int i=0;i<4;++i) {
      cout << " [" << *mi++;
      for (Int j=1;j<4;++j,++mi) cout << ", " << *mi;
      cout << "]";
      if (i<3) cout << endl;
    }
    break;
  case Mueller::Diagonal:
    cout << "Diagonal Mueller: " << endl;
    cout << " [";
    cout << *mi;
    ++mi;
    for (Int i=1;i<4;++i,++mi) cout << ", " << *mi;
    cout << "]";
    break;
  case Mueller::Diag2:
    cout << "Diag2 Mueller: " << endl;
    cout << " [" << *mi++ << ", " << *mi << "]";
    break;
  case Mueller::Scalar:
    cout << "Scalar Mueller: " << endl;
    cout << " [ " << *mi << " ]";
    break;
  }

  return os;
}

} //# NAMESPACE CASA - END


