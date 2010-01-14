//Speed.h generated on 'mer jan 13 11:28:27 CET 2010'. Edit at your own risk.
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
 * File Speed.h
 */
#ifndef Speed_CLASS
#define Speed_CLASS
#include <vector>
#include <iostream>
#include <string>
using namespace std;
#ifndef WITHOUT_ACS
#include <asdmIDLTypesC.h>
using asdmIDLTypes::IDLSpeed;
#endif
#include <StringTokenizer.h>
#include <NumberFormatException.h>
using asdm::StringTokenizer;
using asdm::NumberFormatException;
#include "EndianStream.h"
using asdm::EndianOSStream;
using asdm::EndianISStream;
namespace asdm {
class Speed;
Speed operator * ( double , const Speed & );
ostream & operator << ( ostream &, const Speed & );
istream & operator >> ( istream &, Speed &);
/**
 * The Speed class implements a quantity of speed in meters per second. class implements a quantity of angle in radians.
 * 
 * @version 1.00 Jan. 7, 2005
 * @author Allen Farris
 * 
 * @version 1.1 Aug 8, 2006
 * @author Michel Caillat 
 * added toBin/fromBin methods.
 */
class Speed {
  /**
   * Overloading of multiplication operator.
   * @param d a value in double precision .
   * @param x a const reference to a Speed .
   * @return a Speed 
   */
  friend Speed operator * ( double d, const Speed & x );
  /**
   * Overloading of << to output the value an Speed on an ostream.
   * @param os a reference to the ostream to be written on.
   * @param x a const reference to a Speed.
   */
  friend ostream & operator << ( ostream & os, const Speed & x);
  /**
   * Overloading of >> to read an Speed from an istream.
   */
  friend istream & operator >> ( istream & is, Speed & x);
public:
	/**
	 * The nullary constructor (default).
	 */
	Speed();
	/**
	 * The copy constructor.
	 */
	Speed(const Speed &);
	/**
	 * A constructor from a string representation.
	 * The string passed in argument must be parsable into a double precision
	 * number to express the value in radian of the angle.
	 *
	 * @param s a string.
	 */
	Speed(const string &s);
#ifndef WITHOUT_ACS
	/**
	 *
	 * A constructor from a CORBA/IDL representation.
	 * 
	 * @param a cons ref to an IDLSpeed.
	 */
	Speed(const IDLSpeed & idlSpeed);
#endif
	/**
	 * A constructor from a value in double precision.
	 * The value passed in argument defines the value of the Speed in radian.
	 */
	Speed(double value);
	/**
	 * The destructor.
	 */
	virtual ~Speed();
	/**
	 * A static method equivalent to the constructor from a string.
	 * @param s a string?.
	 */
	static double fromString(const string& s);
	/**
	 * Conversion into string.
	 * The resulting string contains the representation of the value of this Speed.
	 *
	 * @return string
	 */
	static string toString(double);
	/**
	 * Parse the next (string) token of a StringTokenizer into an angle.
	 * @param st a reference to a StringTokenizer.
	 * @return an Speed.
	 */
	static Speed getSpeed(StringTokenizer &t) throw(NumberFormatException);
			
	/**
	 * Write the binary representation of this to an EndianOSStream .
	 * @param eoss a reference to an EndianOSStream .
	 */		
	void toBin(EndianOSStream& eoss);
	/**
	 * Write the binary representation of a vector of Speed to a EndianOSStream.
	 * @param angle the vector of Speed to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<Speed>& angle,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of Speed to a EndianOSStream.
	 * @param angle the vector of vector of Speed to be written
	 * @param eoss the EndianOSStream to be written to
	 */	
	static void toBin(const vector<vector<Speed> >& angle,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of vector of Speed to a EndianOSStream.
	 * @param angle the vector of vector of vector of Speed to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<vector<vector<Speed> > >& angle,  EndianOSStream& eoss);
	/**
	 * Read the binary representation of an Speed from a EndianISStream
	 * and use the read value to set an  Speed.
	 * @param eiss the EndianStream to be read
	 * @return an Speed
	 */
	static Speed fromBin(EndianISStream& eiss);
	
	/**
	 * Read the binary representation of  a vector of  Speed from an EndianISStream
	 * and use the read value to set a vector of  Speed.
	 * @param eiis the EndianISStream to be read
	 * @return a vector of Speed
	 */	 
	 static vector<Speed> from1DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of Speed from an EndianISStream
	 * and use the read value to set a vector of  vector of Speed.
	 * @param eiis the EndianISStream to be read
	 * @return a vector of vector of Speed
	 */	 
	 static vector<vector<Speed> > from2DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of vector of Speed from an EndianISStream
	 * and use the read value to set a vector of  vector of vector of Speed.
	 * @param eiss the EndianISStream to be read
	 * @return a vector of vector of vector of Speed
	 */	 
	 static vector<vector<vector<Speed> > > from3DBin(EndianISStream & eiss);	 
	 
	 /**
	  * An assignment operator Speed = Speed.
	  * @param x a const reference to an Speed.
	  */
	 Speed & operator = (const Speed & x);
	 
	 /**
	  * An assignment operator Speed = double.
	  * @param d a value in double precision.
	  */
	 Speed & operator = (const double d);
	 /**
	  * Operator increment and assign.
	  * @param x a const reference to an Speed.
	  */
	Speed & operator += (const Speed & x);
	/**
	 * Operator decrement and assign.
	 * @param x a const reference to an Speed.
	 */
	Speed & operator -= (const Speed & x);
	/**
	 * Operator multiply and assign.
	 * @param x a value in double precision.
	 */
	Speed & operator *= (const double d);
	/**
	 * Operator divide and assign.
	 * @param x a valye in double precision.
	 */
	Speed & operator /= (const double d);
	/**
	 * Addition operator.
	 * @param x a const reference to a Speed.
	 */
	Speed operator + (const Speed & x) const;
	/**
	 * Substraction operator.
	 * @param x a const reference to a Speed.
	 */
	Speed operator - (const Speed &) const;
	/**
	 * Multiplication operator.
	 * @param d a value in double precision.
	 */
	Speed operator * (const double d) const;
	/**
	 * Division operator.
	 * @param d a value in double precision.
	 */
	Speed operator / (const double) const;
	/**
	 * Comparison operator. Less-than.
	 * @param x a const reference to a Speed.
	 */
	bool operator < (const Speed & x) const;
	/**
	 * Comparison operator. Greater-than.
	 * @param x a const reference to a Speed.
	 */
	bool operator > (const Speed &) const;
	/**
	 * Comparison operator. Less-than or equal.
	 * @param x a const reference to a Speed.
	 */	
	bool operator <= (const Speed &) const;
	/**
	 * Comparison operator. Greater-than or equal.
	 * @param x a const reference to a Speed.
	 */
	bool operator >= (const Speed &) const;
	/**
	 * Comparision operator. Equal-to.
	 * @param x a const reference to a Speed.
	 */
	bool operator == (const Speed &) const;
	/** 
	 * Comparison method. Equality.
	 * @param x a const reference to a Speed.
	 */
	bool equals(const Speed & x) const;
	/**
	 * Comparison operator. Not-equal.
	 * @param x a const reference to a Speed.
	 */
	bool operator != (const Speed & x) const;
	/**
	 * Comparison method. Test nullity.
	 * @param x a const reference to a Speed.
	 */
	bool isZero() const;
	/**
	 * Unary operator. Opposite.
	 */
	Speed operator - () const;
	/**
	 * Unary operator. Unary plus.
	 */
	Speed operator + () const;
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
	 * Return the double precision value of the Speed.
	 * @return double
	 */
	double get() const;
#ifndef WITHOUT_ACS
	/**
	 * Return the IDLSpeed representation of the Speed.
	 * @return IDLSpeed 
	 */
	IDLSpeed toIDLSpeed() const;
#endif
	/**
	 * Returns the abbreviated name of the unit implicitely associated to any Speed.
	 * @return string
	 */
	static string unit();
private:
	double value;
};
// Speed constructors
inline Speed::Speed() : value(0.0) {
}
inline Speed::Speed(const Speed &t) : value(t.value) {
}
#ifndef WITHOUT_ACS
inline Speed::Speed(const IDLSpeed &l) : value(l.value) {
}
#endif
inline Speed::Speed(const string &s) : value(fromString(s)) {
}
inline Speed::Speed(double v) : value(v) {
}
// Speed destructor
inline Speed::~Speed() { }
// assignment operator
inline Speed & Speed::operator = ( const Speed &t ) {
	value = t.value;
	return *this;
}
// assignment operator
inline Speed & Speed::operator = ( const double v ) {
	value = v;
	return *this;
}
// assignment with arithmetic operators
inline Speed & Speed::operator += ( const Speed & t) {
	value += t.value;
	return *this;
}
inline Speed & Speed::operator -= ( const Speed & t) {
	value -= t.value;
	return *this;
}
inline Speed & Speed::operator *= ( const double n) {
	value *= n;
	return *this;
}
inline Speed & Speed::operator /= ( const double n) {
	value /= n;
	return *this;
}
// arithmetic functions
inline Speed Speed::operator + ( const Speed &t2 ) const {
	Speed tmp;
	tmp.value = value + t2.value;
	return tmp;
}
inline Speed Speed::operator - ( const Speed &t2 ) const {
	Speed tmp;
	tmp.value = value - t2.value;
	return tmp;
}
inline Speed Speed::operator * ( const double n) const {
	Speed tmp;
	tmp.value = value * n;
	return tmp;
}
inline Speed Speed::operator / ( const double n) const {
	Speed tmp;
	tmp.value = value / n;
	return tmp;
}
// comparison operators
inline bool Speed::operator < (const Speed & x) const {
	return (value < x.value);
}
inline bool Speed::operator > (const Speed & x) const {
	return (value > x.value);
}
inline bool Speed::operator <= (const Speed & x) const {
	return (value <= x.value);
}
inline bool Speed::operator >= (const Speed & x) const {
	return (value >= x.value);
}
inline bool Speed::equals(const Speed & x) const {
	return (value == x.value);
}
inline bool Speed::operator == (const Speed & x) const {
	return (value == x.value);
}
inline bool Speed::operator != (const Speed & x) const {
	return (value != x.value);
}
// unary - and + operators
inline Speed Speed::operator - () const {
	Speed tmp;
        tmp.value = -value;
	return tmp;
}
inline Speed Speed::operator + () const {
	Speed tmp;
    tmp.value = value;
	return tmp;
}
// Conversion functions
inline Speed::operator string () const {
	return toString();
}
inline string Speed::toString() const {
	return toString(value);
}
inline string Speed::toStringI() const {
	return toString(value);
}
inline double Speed::get() const {
	return value;
}
#ifndef WITHOUT_ACS
inline IDLSpeed Speed::toIDLSpeed() const {
	IDLSpeed tmp;
	tmp.value = value;
	return tmp;
}
#endif
// Friend functions
inline Speed operator * ( double n, const Speed &x) {
	Speed tmp;
	tmp.value = x.value * n;
	return tmp;
}
inline ostream & operator << ( ostream &o, const Speed &x ) {
	o << x.value;
	return o;
}
inline istream & operator >> ( istream &i, Speed &x ) {
	i >> x.value;
	return i;
}
inline string Speed::unit() {
	return string ("m/s");
}
} // End namespace asdm
#endif /* Speed_CLASS */
