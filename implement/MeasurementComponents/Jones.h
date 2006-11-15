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

#ifndef SYNTHESIS_JONES_H
#define SYNTHESIS_JONES_H

#include <casa/aips.h>
#include <casa/BasicSL/Complex.h>
#include <casa/iostream.h>
#include <casa/Exceptions/Error.h>
//#include <synthesis/MeasurementComponents/Mueller.h>
#include <synthesis/MeasurementComponents/VisVector.h>

namespace casa { //# NAMESPACE CASA - BEGIN

class Jones {
  
public:
  
  enum JonesType{General=4,Diagonal=2,Scalar=1};

  // Construct 
  Jones();
  
  // Dtor
  virtual ~Jones() {};
  
  // Return type id
  inline virtual JonesType type() const { return Jones::General; };
  
  // Synchronize with leading element in external array
  inline void sync(Complex& mat) { j0_=&mat; origin(); };
  
  // Reset to origin
  inline void origin() {j_=j0_;};
  
  // Increment to next matrix (according to type)
  inline void operator++()    { j_+=type(); };
  inline void operator++(int) { j_+=type(); };

  // Advance step matrices forward (according to type)
  inline void advance(const Int& step) { j_+=(step*type()); };

  // In-place invert
  virtual Bool invert();

  // In-place multipication with another Jones
  virtual void operator*=(const Jones& other);

  // Apply rightward to a VisVector
  virtual void applyRight(VisVector& v) const;

  // Apply leftward (transposed) to a VisVector 
  virtual void applyLeft(VisVector& v) const;

  // print it out
  friend ostream& operator<<(ostream& os, const Jones& mat);

  // Give access to Mueller formation method
  friend class Mueller;
  friend class MuellerDiag;
  friend class MuellerDiag2;
  friend class MuellerScal;
    
  friend class JonesDiag;
  friend class JonesScal;

protected:
  
  // Copy ctor protected 
  Jones(const Jones& mat);

  // Pointer to origin
  Complex *j0_;
  
  // Moving pointer
  Complex *j_, *ji_;

  // Complex unity
  const Complex cOne_;

private:

  // Zero the Jones matrix
  virtual void zero();

  // Temporary VisVector
  VisVector vtmp_;
  
};

class JonesDiag : public Jones {
  
public:
  
  // Construct 
  JonesDiag();
  
  // Dtor
  virtual ~JonesDiag() {};
  
  // Return type id
  inline virtual JonesType type() const { return Jones::Diagonal; };

  // In-place invert
  virtual Bool invert();

  // In-place multipication with another Jones
  virtual void operator*=(const Jones& other);

  // Apply rightward to a VisVector
  virtual void applyRight(VisVector& v) const;

  // Apply leftward (transposed) to a VisVector
  virtual void applyLeft(VisVector& v) const;

  // Give access to Mueller formation methods
  friend class MuellerDiag;
  friend class MuellerDiag2;
    
protected:
  
  // Copy ctor protected 
  JonesDiag(const JonesDiag& mat);

private:

  // Zero the Jones matrix
  virtual void zero();

};


class JonesScal : public JonesDiag {
  
public:
  
  // Construct 
  JonesScal();
  
  // Dtor
  virtual ~JonesScal() {};
  
  // Return type id
  inline virtual JonesType type() const { return Jones::Scalar; };

  // In-place invert
  virtual Bool invert();

  // In-place multipication with another Jones
  virtual void operator*=(const Jones& other);

  // Apply rightward to a VisVector
  virtual void applyRight(VisVector& v) const;

  // Apply leftward (transposed) to a VisVector
  virtual void applyLeft(VisVector& v) const;

  // Give access to Mueller formation methods
  friend class MuellerScal;
    
protected:
  
  // Copy ctor protected 
  JonesScal(const JonesScal& mat);

private:

  // Zero the Jones matrix
  virtual void zero();

};


// Global methods:

// Factory method for creation of Jones
Jones* createJones(const Jones::JonesType& jtype);

// Apply a pair of Jones to a VisVector:
void apply(const Jones& j1, VisVector& v, const Jones& j2);

// Return enum from integer
Jones::JonesType jonesType(const Int& n);

} //# NAMESPACE CASA - END

#endif


