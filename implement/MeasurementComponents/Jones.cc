//# Jones.h: Definition of Jones
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


#include <synthesis/MeasurementComponents/Jones.h>
#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>
#include <casa/iostream.h>
#include <casa/Exceptions/Error.h>
#include <casa/namespace.h>

namespace casa { //# NAMESPACE CASA - BEGIN


// Constructor
Jones::Jones() : 
  j0_(NULL), 
  ok0_(NULL),
  j_(NULL), 
  ji_(NULL),
  ok_(NULL),
  oki_(NULL),
  cOne_(1.0),
  vtmp_(VisVector::Four,True)
{}

// In place invert
void Jones::invert() {

  if (!ok_) throw(AipsError("Illegal use of Jones::invert()"));

  if (ok_[0]&&ok_[1]&&ok_[2]&&ok_[3]) {
    Complex det,tmp;
    tmp=j_[0];
    det=j_[0]*j_[3]-j_[1]*j_[2];
    if (abs(det)>0.0) {
      j_[0]=j_[3]/det;
      j_[1]=-j_[1]/det;
      j_[2]=-j_[2]/det;
      j_[3]=tmp/det;
    } else {
      zero();
      ok_[0]=ok_[1]=ok_[2]=ok_[3]=False;
    }
  }
  else {
    ok_[0]=ok_[1]=ok_[2]=ok_[3]=False;
    zero();
  }

}

// In-place multipication with another Jones
void Jones::operator*=(const Jones& other) {

  switch(other.type()) {
  case Jones::General: 
  case Jones::Diagonal:
  case Jones::Scalar:
  default:
    throw(AipsError("General multiplication NYI"));
    break;
  }
}

// Apply rightward to a VisVector
void Jones::applyRight(VisVector& v) const {

  switch(v.type()) {
  case VisVector::Four: {
    vtmp_.v_[0] = v.v_[2];
    vtmp_.v_[1] = v.v_[3];
    vtmp_.v_[2] = v.v_[0];
    vtmp_.v_[3] = v.v_[1];
    
    v.v_[0]*=(j_[0]);
    v.v_[1]*=(j_[0]);
    v.v_[2]*=(j_[3]);
    v.v_[3]*=(j_[3]);
    
    vtmp_.v_[0]*=(j_[1]);
    vtmp_.v_[1]*=(j_[1]);
    vtmp_.v_[2]*=(j_[2]);
    vtmp_.v_[3]*=(j_[2]);
    
    for (Int i=0;i<4;++i)
      v.v_[i] += vtmp_.v_[i];
    break;
  }
  default:
    throw(AipsError("Jones matrix apply (J::aR) incompatible with VisVector."));
  }
}

// Apply rightward to a VisVector
void Jones::applyRight(VisVector& v, Bool& vflag) const {

  if (!ok_) throw(AipsError("Illegal use of Jones::applyRight(v,vflag)"));

  vflag|=(!(ok_[0]&&ok_[1]&&ok_[2]&&ok_[3]));
  if (!vflag) applyRight(v);
  else v.zero();
  
}

// Apply leftward (transposed) to a VisVector 
void Jones::applyLeft(VisVector& v) const {

  switch(v.type()) {
  case VisVector::Four: {
    vtmp_.v_[0] = v.v_[1];
    vtmp_.v_[1] = v.v_[0];
    vtmp_.v_[2] = v.v_[3];
    vtmp_.v_[3] = v.v_[2];
    
    Complex c;
    c=conj(j_[0]);
    v.v_[0]*=c;
    v.v_[1]*=c;
    c=conj(j_[3]);
    v.v_[2]*=c;
    v.v_[3]*=c;
    
    c=conj(j_[1]);
    vtmp_.v_[0]*=c;
    vtmp_.v_[1]*=c;
    c=conj(j_[2]);
    vtmp_.v_[2]*=c;
    vtmp_.v_[3]*=c;
  
    for (Int i=0;i<4;++i)
      v.v_[i] += vtmp_.v_[i];
    break;
  }
  default:
    throw(AipsError("Jones matrix apply (J::aL) incompatible with VisVector."));
  }
}

// Apply leftward to a VisVector
void Jones::applyLeft(VisVector& v, Bool& vflag) const {

  if (!ok_) throw(AipsError("Illegal use of Jones::applyLeft(v,vflag)"));

  vflag|=(!(ok_[0]&&ok_[1]&&ok_[2]&&ok_[3]));
  if (!vflag) applyLeft(v);
  else v.zero();

}

void Jones::zero() {
  ji_=j_;
  for (Int i=0;i<4;++i,++ji_)
    (*ji_)=0.0;
}

// Constructor
JonesDiag::JonesDiag() : Jones() {}

// In place invert
void JonesDiag::invert() {

  if (!ok_) throw(AipsError("Illegal use of JonesDiag::invert()"));

  ji_=j_;
  oki_=ok_;
  for (Int i=0;i<2;++i,++ji_,++oki_)
    if ((*oki_) && abs(*ji_)>0.0)
      (*ji_)=cOne_/(*ji_);
    else {
      (*ji_)=Complex(0.0);
      (*oki_)=False;
    }

}

// In-place multipication with another Jones
void JonesDiag::operator*=(const Jones& other) {

  if (!ok_) throw(AipsError("Illegal use of JonesDiag::operator*=()"));

  switch(other.type()) {
  case Jones::General: 
    throw(AipsError("Can't multiply Diagonal by General Jones matrix."));
    break;
  case Jones::Diagonal: {
    if (ok_[0]&=other.ok_[0]) j_[0]*=other.j_[0];
    if (ok_[1]&=other.ok_[1]) j_[1]*=other.j_[1];
    break;
  }
  case Jones::Scalar: {
    if (ok_[0]&=other.ok_[0]) j_[0]*=other.j_[0];
    if (ok_[1]&=other.ok_[0]) j_[1]*=other.j_[0];
    break;
  }
  }
}

// Apply rightward to a VisVector
void JonesDiag::applyRight(VisVector& v) const {

  switch(v.type()) {
  case VisVector::Four: {
    v.v_[0]*=j_[0];
    v.v_[1]*=j_[0];
    v.v_[2]*=j_[1];
    v.v_[3]*=j_[1];
    break;
  }
  case VisVector::Two: {
    v.v_[0]*=j_[0];
    v.v_[1]*=j_[1];
    break;
  }
  case VisVector::One: {
    v.v_[0]*=(*j_);
    break;
  }
  default:
    throw(AipsError("Jones matrix apply (JD::aR) incompatible with VisVector."));
  }    
}

void JonesDiag::applyRight(VisVector& v, Bool& vflag) const {

  if (!ok_) throw(AipsError("Illegal use of JonesDiag::applyRight(v,vflag)"));

  switch(v.type()) {
  // For scalar data, we only pay attention to the first flag
  case VisVector::One: 
    vflag|=(!ok_[0]);
    break;
  // ...otherwise, we flag outright
  default:
    vflag|=(!(ok_[0]&&ok_[1]));
    break;
  }
  if (!vflag) applyRight(v);
  else v.zero();

}

// Apply leftward (transposed) to a VisVector 
void JonesDiag::applyLeft(VisVector& v) const {

  switch(v.type()) {
  case VisVector::Four: {
    Complex c;
    c=conj(j_[0]);
    v.v_[0]*=c;
    v.v_[2]*=c;
    c=conj(j_[1]);
    v.v_[1]*=c;
    v.v_[3]*=c;
    break;
  }
  case VisVector::Two: {
    v.v_[0]*=conj(j_[0]);
    v.v_[1]*=conj(j_[1]);
    break;
  }
  case VisVector::One: {
    v.v_[0]*=conj(*j_);
    break;
  }
  default:
    throw(AipsError("Jones matrix apply (JD::aL) incompatible with VisVector."));
  }    
}

void JonesDiag::applyLeft(VisVector& v, Bool& vflag) const {

  if (!ok_) throw(AipsError("Illegal use of JonesDiag::applyLeft(v,vflag)"));

  switch(v.type()) {
  // For scalar data, we only pay attention to the first flag
  case VisVector::One: 
    vflag|=(!ok_[0]);
    break;
  // ...otherwise, we flag outright
  default:
    vflag|=(!(ok_[0]&&ok_[1]));
    break;
  }
  if (!vflag) applyLeft(v);
  else v.zero();
}

void JonesDiag::zero() {
  ji_=j_;
  for (Int i=0;i<2;++i,++ji_)
    (*ji_)=0.0;
}


// Constructor
JonesScal::JonesScal() : JonesDiag() {}

// In place invert
void JonesScal::invert() {
  
  if (!ok_) throw(AipsError("Illegal use of JonesScal::invert()"));

  if ((*ok_) && abs(*j_)>0.0)
    (*j_)=cOne_/(*j_);
  else {
    (*j_)=Complex(0.0);
    (*ok_)=False;
  }

}

// In-place multipication with another Jones
void JonesScal::operator*=(const Jones& other) {

  if (!ok_) throw(AipsError("Illegal use of JonesScal::operator*="));

  switch(other.type()) {
  case Jones::General: 
    throw(AipsError("Can't multiply Scalar Jones by General Jones matrix."));
    break;
  case Jones::Diagonal:
    throw(AipsError("Can't multiply Scalar Jones by Diagonal Jones matrix."));
    break;
  case Jones::Scalar: {
    if ((*ok_)&=(*other.ok_)) (*j_)*=(*other.j_);
    break;
  }
  }
}


// Apply rightward to a VisVector
void JonesScal::applyRight(VisVector& v) const {
  for (Int i=0;i<v.vistype_;++i) 
    v.v_[i] *= (*j_);
}

void JonesScal::applyRight(VisVector& v, Bool& vflag) const {

  if (!ok_) throw(AipsError("Illegal use of JonesScal::applyRight(v,vflag)"));

  vflag|=(!*ok_);
  if (!vflag) applyRight(v);
  else v.zero();
}

// Apply leftward (transposed) to a VisVector 
void JonesScal::applyLeft(VisVector& v) const {
  Complex c;
  c=conj(*j_);
  for (Int i=0;i<v.vistype_;++i) 
    v.v_[i] *= c;
}

void JonesScal::applyLeft(VisVector& v, Bool& vflag) const {

  if (!ok_) throw(AipsError("Illegal use of JonesScal::applyLeft(v,vflag)"));

  vflag|=(!*ok_);
  if (!vflag) applyLeft(v);
  else v.zero();
}

void JonesScal::zero() {
    (*j_)=0.0;
}


// GLOBALS---------------------------


Jones* createJones(const Jones::JonesType& jtype) {
  Jones *j(NULL);
  switch (jtype) {
  case Jones::General:
    j = new Jones();
    break;
  case Jones::Diagonal:
    j = new JonesDiag();
    break;
  case Jones::Scalar:
    j = new JonesScal();
    break;
  }
  return j;
}

void apply(const Jones& j1, VisVector& v, const Jones& j2) {

  // Apply Jones matrix from right, then left
  j1.applyRight(v);
  j2.applyLeft(v);

}

void apply(const Jones& j1, VisVector& v, const Jones& j2, Bool& vflag) {

  // Apply Jones matrix from right, then left
  j1.applyRight(v,vflag);
  j2.applyLeft(v,vflag);

}

// print out a Jones matrix (according to type)
ostream& operator<<(ostream& os, const Jones& mat) {

  Complex *ji;
  ji=mat.j_;

  switch (mat.type()) {
  case Jones::General:
    cout << "General Jones: " << endl;
    for (Int i=0;i<2;++i) {
      cout << " [" << *ji++;
      cout << ", " << *ji << "]";
      if (i<1) cout << endl;
    }
    break;
  case Jones::Diagonal:
    cout << "Diagonal Jones: " << endl;
    cout << " [" << *ji++;
    cout << ", " << *ji << "]";
    break;
  case Jones::Scalar:
    cout << "Scalar Jones: " << endl;
    cout << "[" << *ji << "]";
    break;
  }

  return os;
}

// Return the enum for from an int
Jones::JonesType jonesType(const Int& n) {
  switch (n) {
  case 1:
    return Jones::Scalar;
  case 2:
    return Jones::Diagonal;
  case 4:
    return Jones::General;
  default:
    throw(AipsError("Bad JonesType."));

  }

}

} //# NAMESPACE CASA - END
