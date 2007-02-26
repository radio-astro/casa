//# Mueller.h: Definition of Mueller
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

#ifndef SYNTHESIS_MUELLER_H
#define SYNTHESIS_MUELLER_H

#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>
#include <casa/iostream.h>
#include <casa/Exceptions/Error.h>
#include <synthesis/MeasurementComponents/VisVector.h>
#include <synthesis/MeasurementComponents/Jones.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class Mueller {
  
public:
  
  enum MuellerType{General=16,Diagonal=4,Diag2=2,Scalar=1};
 
  // Construct 
  Mueller();
  
  // Dtor
  virtual ~Mueller() {};
  
  // Return type id
  inline virtual MuellerType type() const { return Mueller::General; };
  
  // Synchronize with leading element in external array
  inline void sync(Complex& mat) { m0_=&mat; origin(); };
  inline void sync(Complex& mat, Bool& ok) { m0_=&mat; ok0_=&ok; origin(); };
  
  // Reset to origin
  inline void origin() {m_=m0_;ok_=ok0_;};
  
  // Increment to next vector (according to type)
  inline void operator++()    { m_+=type(); if (ok_) ok_+=type();};
  inline void operator++(int) { m_+=type(); if (ok_) ok_+=type();};

  // Advance step matrices forward (according to type)
  inline void advance(const Int& step) { m_+=(step*type()); if (ok_) ok_+=(step*type());};

  // Formation from Jones matrix outer product: General version
  virtual void fromJones(const Jones& jones1, const Jones& jones2);

  // In-place invert
  virtual void invert();

  // In-place multiply onto a VisVector: General version
  virtual void apply(VisVector& v);
  virtual void apply(VisVector& v, Bool& vflag);

  // Multiply onto a vis VisVector, preserving input (copy then in-place apply)
  virtual void apply(VisVector& out, const VisVector& in);

  // print it out
  friend ostream& operator<<(ostream& os, const Mueller& mat);
    
protected:
  
  // Copy ctor protected 
  Mueller(const Mueller& mat);

  // Pointer to origin
  Complex *m0_;
  Bool *ok0_;

  // Moving pointer
  Complex *m_, *mi_;
  Bool *ok_, *oki_;

  // Complex unity (for use in invert methods)
  const Complex cOne_;

private: 

  // Zero the whole Mueller
  virtual void zero();

  // VisVector temporary (only relevant for Mueller::General)
  VisVector vtmp_;


  
};

class MuellerDiag : public Mueller {

public:
  
  // Construct 
  MuellerDiag();
  
  // Dtor
  virtual ~MuellerDiag() {};
  
  // Return type id
  inline virtual MuellerType type() const { return Mueller::Diagonal; };
  
  // Formation from Jones matrix outer product: optimized Diagonal version
  virtual void fromJones(const Jones& jones1, const Jones& jones2);
  
  // In-place invert
  virtual void invert();

  // In-place multiply onto a VisVector: optimized Diagonal version
  virtual void apply(VisVector& v);
  virtual void apply(VisVector& v, Bool& vflag);
  using Mueller::apply;

protected:
  
  // Default/Copy ctors are protected 
  MuellerDiag(const MuellerDiag& mat);

private: 

  // Zero the whole Mueller
  virtual void zero();
  
};

class MuellerDiag2 : public MuellerDiag {
  
public:

  // Construct 
  MuellerDiag2();

  // Dtor
  virtual ~MuellerDiag2() {};
  
  // Return type id
  inline virtual MuellerType type() const { return Mueller::Diag2; };

  // Formation from Jones matrix outer product: optimized Diag2 version
  virtual void fromJones(const Jones& jones1, const Jones& jones2);
  
  // In-place invert
  virtual void invert();

  // In-place multiply onto a VisVector: optimized Diag2 version
  virtual void apply(VisVector& v);
  virtual void apply(VisVector& v, Bool& vflag);
  using MuellerDiag::apply;

protected:
  
  // Default/Copy ctors are protected 
  MuellerDiag2(const MuellerDiag2& mat);
  
private: 

  // Zero the whole Mueller
  virtual void zero();

};


class MuellerScal : public MuellerDiag {
 
public:

  // Construct 
  MuellerScal();

  // Dtor
  virtual ~MuellerScal() {};

  // Return type id
  inline virtual MuellerType type() const { return Mueller::Scalar; }

  // Formation from Jones matrix outer product: optimized Scalar version
  virtual void fromJones(const Jones& jones1, const Jones& jones2);

  // In-place invert
  virtual void invert();

  // In-place multiply onto a VisVector: optimized Scalar version
  virtual void apply(VisVector& v);
  virtual void apply(VisVector& v, Bool& vflag);
  using MuellerDiag::apply;

protected:
  
  // Default/Copy ctors are protected 
  MuellerScal(const MuellerScal& mat);

private: 

  // Zero the whole Mueller
  virtual void zero();

};

// Globals

// Factory method
Mueller* createMueller(const Mueller::MuellerType& mtype);

// Return Mueller type according to Int
Mueller::MuellerType muellerType(const Int& n);

// Return Mueller type according to underlying Jones and VisVector types
Mueller::MuellerType muellerType(const Jones::JonesType& jtype, const VisVector::VisType& vtype);

} //# NAMESPACE CASA - END

#endif

