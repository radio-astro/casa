//# MeasRef.cc:  Reference frame for physical measures
//# Copyright (C) 1995-2001,2007
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
//# $Id: MeasRef.tcc 21024 2011-03-01 11:46:18Z gervandiepen $

//# Includes
#include <casa/Exceptions/Error.h>
#include <casa/BasicSL/String.h>
#include <measures/Measures/MeasRef.h>
#include <casa/iostream.h>


namespace casa { //# NAMESPACE CASA - BEGIN

//# Constructors

template<class Ms>
MeasRef<Ms>::MeasRef()
: empty_p (True), rep_p ()
{}

template<class Ms>
MeasRef<Ms>::MeasRef(const MeasRef<Ms> &other)
: MRBase(other)
{
  empty_p = other.empty_p;

  if (! other.empty_p){
    rep_p = other.rep_p;
  }
  else{
    rep_p = CountedPtr<RefRep> ();
  }
}

template<class Ms>
MeasRef<Ms> &
MeasRef<Ms>::operator=(const MeasRef<Ms> &other) {

  if (this != &other) {

    empty_p = other.empty_p;

    if (! other.empty_p){
      rep_p = other.rep_p;
    }
    else{
      rep_p = CountedPtr<RefRep> ();
    }
  }

  return *this;
}

template<class Ms>
MeasRef<Ms>::MeasRef(const uInt tp)
: empty_p (True), rep_p ()
{
  create();
  rep_p->type = Ms::castType(tp);
}

template<class Ms>
MeasRef<Ms>::MeasRef(const uInt tp, const Ms &ep)
: empty_p (True), rep_p ()
{
  create();
  rep_p->type = Ms::castType(tp);
  rep_p->offmp = new Ms(ep);
}

template<class Ms>
MeasRef<Ms>::MeasRef(const uInt tp, const MeasFrame &mf)
: empty_p (True), rep_p ()
{
  create();
  rep_p->type = Ms::castType(tp);
  rep_p->frame = mf;
}

template<class Ms>
MeasRef<Ms>::MeasRef(const uInt tp, const MeasFrame &mf, const Ms &ep)
: empty_p (True), rep_p ()
{
  create();
  rep_p->type = Ms::castType(tp);
  rep_p->offmp = new Ms(ep);
  rep_p->frame = mf;
}

template<class Ms>
void MeasRef<Ms>::create() {
  if (empty_p){
    rep_p = new RefRep();
    empty_p = False;
  }
}

//# Destructor
template<class Ms>
MeasRef<Ms>::~MeasRef()
{}

//# Operators
template<class Ms>
Bool MeasRef<Ms>::operator==(const MeasRef<Ms> &other) const {
  return (rep_p == other.rep_p);
}

template<class Ms>
Bool MeasRef<Ms>::operator!=(const MeasRef<Ms> &other) const {
  return (rep_p != other.rep_p);
}

//# Member functions
template<class Ms>
Bool MeasRef<Ms>::empty() const {
  return (empty_p);
}

template<class Ms>
const String &MeasRef<Ms>::showMe() {
  return Ms::showMe();
}

template<class Ms>
uInt MeasRef<Ms>::getType() const{
  return (! empty_p ? rep_p->type : 0);
}

template<class Ms>
MeasFrame &MeasRef<Ms>::getFrame() {
  create();
  return (rep_p->frame);
}

template<class Ms>
const MeasFrame &MeasRef<Ms>::framePosition(MRBase &ref1,
					    MRBase &ref2) {
  if (!ref1.empty() && ref1.getFrame().position()) {
    return ref1.getFrame();
  } else if (!ref2.empty() && ref2.getFrame().position()) {
  } else {
    throw(AipsError("No MeasFrame specified for conversion of " + 
		    Ms::showMe()));
  }
  return ref2.getFrame();
}

template<class Ms>
const MeasFrame &MeasRef<Ms>::frameEpoch(MRBase &ref1,
					 MRBase &ref2) {
  if (!ref1.empty() && ref1.getFrame().epoch()) {
    return ref1.getFrame();
  } else if (!ref2.empty() && ref2.getFrame().epoch()) {
  } else {
    throw(AipsError("No MeasFrame specified for conversion of " + 
		    Ms::showMe()));
  }
  return ref2.getFrame();
}

template<class Ms>
const MeasFrame &MeasRef<Ms>::frameDirection(MRBase &ref1,
					     MRBase &ref2) {
  if (!ref1.empty() && ref1.getFrame().direction()) {
    return ref1.getFrame();
  } else if (!ref2.empty() && ref2.getFrame().direction()) {
  } else {
    throw(AipsError("No MeasFrame specified for conversion of " + 
		    Ms::showMe()));
  }
  return ref2.getFrame();
}

template<class Ms>
const MeasFrame &MeasRef<Ms>::frameRadialVelocity(MRBase &ref1,
						  MRBase &ref2) {
  if (!ref1.empty() && ref1.getFrame().radialVelocity()) {
    return ref1.getFrame();
  } else if (!ref2.empty() && ref2.getFrame().radialVelocity()) {
  } else {
    throw(AipsError("No MeasFrame specified for conversion of " + 
		    Ms::showMe()));
  }
  return ref2.getFrame();
}

template<class Ms>
const MeasFrame &MeasRef<Ms>::frameComet(MRBase &ref1,
					 MRBase &ref2) {
  if (!ref1.empty() && ref1.getFrame().comet()) {
    return ref1.getFrame();
  } else if (!ref2.empty() && ref2.getFrame().comet()) {
  } else {
    throw(AipsError("No MeasFrame specified for conversion of " + 
		    Ms::showMe()));
  }
  return ref2.getFrame();
}

template<class Ms>
const Measure* MeasRef<Ms>::offset() const {
  return ( ! empty_p ? rep_p->offmp : 0);
}

template<class Ms>
void MeasRef<Ms>::setType(uInt tp) {
  set(tp);
}

template<class Ms>
void MeasRef<Ms>::set(uInt tp) {
  create();
  rep_p->type = Ms::castType(tp);
}

template<class Ms>
void MeasRef<Ms>::set(const Ms &ep) {
  create();
  if (rep_p->offmp) {
    delete rep_p->offmp; rep_p->offmp = 0;
  }
  rep_p->offmp = new Ms(ep);
}

template<class Ms>
void MeasRef<Ms>::set(const Measure &ep) {
  create();
  if (rep_p->offmp) {
    delete rep_p->offmp; rep_p->offmp = 0;
  }
  rep_p->offmp = ep.clone();
}

template<class Ms>
void MeasRef<Ms>::set(const MeasFrame &mf) {
  create();
  rep_p->frame = mf;
}

template<class Ms>
MeasRef<Ms> MeasRef<Ms>::copy() {
  MeasRef<Ms> tmp;
  tmp.create();
  tmp.rep_p->type = rep_p->type;
  if (rep_p->offmp) tmp.rep_p->offmp = rep_p->offmp->clone();
  tmp.rep_p->frame = rep_p->frame;
  return tmp;
}

template<class Ms>
void MeasRef<Ms>::print(ostream &os) const {
  os << "Reference for an " << showMe(); 
  os << " with Type: " << Ms::showType(getType());
  if (offset()) {
    os << ", Offset: " << *(offset());
  }
  // Get rid of const
  if (!((MeasRef<Ms> *)(this))->getFrame().empty()) {
    os << "," << endl << ((MeasRef<Ms> *)(this))->getFrame();
  }
}

} //# NAMESPACE CASA - END

