//Flux.h generated on 'Thu Feb 04 10:20:05 CET 2010'. Edit at your own risk.
/*
 * ALMA - Atacama Large Millimeter Array
 * (c) European Southern Observatory, 2002
 * (c) Associated Universities Inc., 2002
 * Copyright by ESO (in the framework of the ALMA collaboration),
 * Copyright by AUI (in the framework of the ALMA collaboration),
 * All rights reserved.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307  USA
 *
 * File Flux.h
 */
#ifndef Flux_CLASS
#define Flux_CLASS
#include <vector>
#include <iostream>
#include <string>
using namespace std;
#ifndef WITHOUT_ACS
#include <asdmIDLTypesC.h>
using asdmIDLTypes::IDLFlux;
#endif
#include <StringTokenizer.h>
#include <NumberFormatException.h>
using asdm::StringTokenizer;
using asdm::NumberFormatException;
#include "EndianStream.h"
using asdm::EndianOSStream;
using asdm::EndianISStream;
namespace asdm {
class Flux;
Flux operator * ( double , const Flux & );
ostream & operator << ( ostream &, const Flux & );
istream & operator >> ( istream &, Flux &);
/**
 * The Flux class implements a quantity of flux in janskys..
 * 
 * @version 1.00 Jan. 7, 2005
 * @author Allen Farris
 * 
 * @version 1.1 Aug 8, 2006
 * @author Michel Caillat 
 * added toBin/fromBin methods.
 */
class Flux {
  /**
   * Overloading of multiplication operator.
   * @param d a value in double precision .
   * @param x a const reference to a Flux .
   * @return a Flux 
   */
  friend Flux operator * ( double d, const Flux & x );
  /**
   * Overloading of << to output the value an Flux on an ostream.
   * @param os a reference to the ostream to be written on.
   * @param x a const reference to a Flux.
   */
  friend ostream & operator << ( ostream & os, const Flux & x);
  /**
   * Overloading of >> to read an Flux from an istream.
   */
  friend istream & operator >> ( istream & is, Flux & x);
public:
	/**
	 * The nullary constructor (default).
	 */
	Flux();
	/**
	 * The copy constructor.
	 */
	Flux(const Flux &);
	/**
	 * A constructor from a string representation.
	 * The string passed in argument must be parsable into a double precision
	 * number to express the value in radian of the angle.
	 *
	 * @param s a string.
	 */
	Flux(const string &s);
#ifndef WITHOUT_ACS
	/**
	 *
	 * A constructor from a CORBA/IDL representation.
	 * 
	 * @param idlFlux a cons ref to an IDLFlux.
	 */
	Flux(const IDLFlux & idlFlux);
#endif
	/**
	 * A constructor from a value in double precision.
	 * The value passed in argument defines the value of the Flux in radian.
	 */
	Flux(double value);
	/**
	 * The destructor.
	 */
	virtual ~Flux();
	/**
	 * A static method equivalent to the constructor from a string.
	 * @param s a string?.
	 */
	static double fromString(const string& s);
	/**
	 * Conversion into string.
	 * The resulting string contains the representation of the value of this Flux.
	 *
	 * @return string
	 */
	static string toString(double);
	/**
	 * Parse the next (string) token of a StringTokenizer into an angle.
	 * @param st a reference to a StringTokenizer.
	 * @return an Flux.
	 */
	static Flux getFlux(StringTokenizer &st) throw(NumberFormatException);
			
	/**
	 * Write the binary representation of this to an EndianOSStream .
	 * @param eoss a reference to an EndianOSStream .
	 */		
	void toBin(EndianOSStream& eoss);
	/**
	 * Write the binary representation of a vector of Flux to a EndianOSStream.
	 * @param angle the vector of Flux to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<Flux>& angle,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of Flux to a EndianOSStream.
	 * @param angle the vector of vector of Flux to be written
	 * @param eoss the EndianOSStream to be written to
	 */	
	static void toBin(const vector<vector<Flux> >& angle,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of vector of Flux to a EndianOSStream.
	 * @param angle the vector of vector of vector of Flux to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<vector<vector<Flux> > >& angle,  EndianOSStream& eoss);
	/**
	 * Read the binary representation of an Flux from a EndianISStream
	 * and use the read value to set an  Flux.
	 * @param eiss the EndianStream to be read
	 * @return an Flux
	 */
	static Flux fromBin(EndianISStream& eiss);
	
	/**
	 * Read the binary representation of  a vector of  Flux from an EndianISStream
	 * and use the read value to set a vector of  Flux.
	 * @param eiss a reference to the EndianISStream to be read
	 * @return a vector of Flux
	 */	 
	 static vector<Flux> from1DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of Flux from an EndianISStream
	 * and use the read value to set a vector of  vector of Flux.
	 * @param eiss the EndianISStream to be read
	 * @return a vector of vector of Flux
	 */	 
	 static vector<vector<Flux> > from2DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of vector of Flux from an EndianISStream
	 * and use the read value to set a vector of  vector of vector of Flux.
	 * @param eiss the EndianISStream to be read
	 * @return a vector of vector of vector of Flux
	 */	 
	 static vector<vector<vector<Flux> > > from3DBin(EndianISStream & eiss);	 
	 
	 /**
	  * An assignment operator Flux = Flux.
	  * @param x a const reference to an Flux.
	  */
	 Flux & operator = (const Flux & x);
	 
	 /**
	  * An assignment operator Flux = double.
	  * @param d a value in double precision.
	  */
	 Flux & operator = (const double d);
	 /**
	  * Operator increment and assign.
	  * @param x a const reference to an Flux.
	  */
	Flux & operator += (const Flux & x);
	/**
	 * Operator decrement and assign.
	 * @param x a const reference to an Flux.
	 */
	Flux & operator -= (const Flux & x);
	/**
	 * Operator multiply and assign.
	 * @param x a value in double precision.
	 */
	Flux & operator *= (const double x);
	/**
	 * Operator divide and assign.
	 * @param x a valye in double precision.
	 */
	Flux & operator /= (const double x);
	/**
	 * Addition operator.
	 * @param x a const reference to a Flux.
	 */
	Flux operator + (const Flux & x) const;
	/**
	 * Substraction operator.
	 * @param x a const reference to a Flux.
	 */
	Flux operator - (const Flux & x) const;
	/**
	 * Multiplication operator.
	 * @param x a value in double precision.
	 */
	Flux operator * (const double x) const;
	/**
	 * Division operator.
	 * @param d a value in double precision.
	 */
	Flux operator / (const double x) const;
	/**
	 * Comparison operator. Less-than.
	 * @param x a const reference to a Flux.
	 */
	bool operator < (const Flux & x) const;
	/**
	 * Comparison operator. Greater-than.
	 * @param x a const reference to a Flux.
	 */
	bool operator > (const Flux & x) const;
	/**
	 * Comparison operator. Less-than or equal.
	 * @param x a const reference to a Flux.
	 */	
	bool operator <= (const Flux & x) const;
	/**
	 * Comparison operator. Greater-than or equal.
	 * @param x a const reference to a Flux.
	 */
	bool operator >= (const Flux & x) const;
	/**
	 * Comparision operator. Equal-to.
	 * @param x a const reference to a Flux.
	 */
	bool operator == (const Flux & x) const;
	/** 
	 * Comparison method. Equality.
	 * @param x a const reference to a Flux.
	 */
	bool equals(const Flux & x) const;
	/**
	 * Comparison operator. Not-equal.
	 * @param x a const reference to a Flux.
	 */
	bool operator != (const Flux & x) const;
	/**
	 * Comparison method. Test nullity.
	 * @return a bool.
	 */
	bool isZero() const;
	/**
	 * Unary operator. Opposite.
	 */
	Flux operator - () const;
	/**
	 * Unary operator. Unary plus.
	 */
	Flux operator + () const;
	/**
	 * Converts into a string.
	 * @return a string containing the representation of a the value in double precision.
	 */
	string toString() const;
	/** 
	 * Idem toString.
	 */
	string toStringI() const;
	/**
	 * Conversion operator.
	 * Converts into a string.
	 */
	operator string () const;
	/**
	 * Return the double precision value of the Flux.
	 * @return double
	 */
	double get() const;
#ifndef WITHOUT_ACS
	/**
	 * Return the IDLFlux representation of the Flux.
	 * @return IDLFlux 
	 */
	IDLFlux toIDLFlux() const;
#endif
	/**
	 * Returns the abbreviated name of the unit implicitely associated to any Flux.
	 * @return string
	 */
	static string unit();
private:
	double value;
};
// Flux constructors
inline Flux::Flux() : value(0.0) {
}
inline Flux::Flux(const Flux &t) : value(t.value) {
}
#ifndef WITHOUT_ACS
inline Flux::Flux(const IDLFlux &l) : value(l.value) {
}
#endif
inline Flux::Flux(const string &s) : value(fromString(s)) {
}
inline Flux::Flux(double v) : value(v) {
}
// Flux destructor
inline Flux::~Flux() { }
// assignment operator
inline Flux & Flux::operator = ( const Flux &t ) {
	value = t.value;
	return *this;
}
// assignment operator
inline Flux & Flux::operator = ( const double v ) {
	value = v;
	return *this;
}
// assignment with arithmetic operators
inline Flux & Flux::operator += ( const Flux & t) {
	value += t.value;
	return *this;
}
inline Flux & Flux::operator -= ( const Flux & t) {
	value -= t.value;
	return *this;
}
inline Flux & Flux::operator *= ( const double n) {
	value *= n;
	return *this;
}
inline Flux & Flux::operator /= ( const double n) {
	value /= n;
	return *this;
}
// arithmetic functions
inline Flux Flux::operator + ( const Flux &t2 ) const {
	Flux tmp;
	tmp.value = value + t2.value;
	return tmp;
}
inline Flux Flux::operator - ( const Flux &t2 ) const {
	Flux tmp;
	tmp.value = value - t2.value;
	return tmp;
}
inline Flux Flux::operator * ( const double n) const {
	Flux tmp;
	tmp.value = value * n;
	return tmp;
}
inline Flux Flux::operator / ( const double n) const {
	Flux tmp;
	tmp.value = value / n;
	return tmp;
}
// comparison operators
inline bool Flux::operator < (const Flux & x) const {
	return (value < x.value);
}
inline bool Flux::operator > (const Flux & x) const {
	return (value > x.value);
}
inline bool Flux::operator <= (const Flux & x) const {
	return (value <= x.value);
}
inline bool Flux::operator >= (const Flux & x) const {
	return (value >= x.value);
}
inline bool Flux::equals(const Flux & x) const {
	return (value == x.value);
}
inline bool Flux::operator == (const Flux & x) const {
	return (value == x.value);
}
inline bool Flux::operator != (const Flux & x) const {
	return (value != x.value);
}
// unary - and + operators
inline Flux Flux::operator - () const {
	Flux tmp;
        tmp.value = -value;
	return tmp;
}
inline Flux Flux::operator + () const {
	Flux tmp;
    tmp.value = value;
	return tmp;
}
// Conversion functions
inline Flux::operator string () const {
	return toString();
}
inline string Flux::toString() const {
	return toString(value);
}
inline string Flux::toStringI() const {
	return toString(value);
}
inline double Flux::get() const {
	return value;
}
#ifndef WITHOUT_ACS
inline IDLFlux Flux::toIDLFlux() const {
	IDLFlux tmp;
	tmp.value = value;
	return tmp;
}
#endif
// Friend functions
inline Flux operator * ( double n, const Flux &x) {
	Flux tmp;
	tmp.value = x.value * n;
	return tmp;
}
inline ostream & operator << ( ostream &o, const Flux &x ) {
	o << x.value;
	return o;
}
inline istream & operator >> ( istream &i, Flux &x ) {
	i >> x.value;
	return i;
}
inline string Flux::unit() {
	return string ("jy");
}
} // End namespace asdm
#endif /* Flux_CLASS */
