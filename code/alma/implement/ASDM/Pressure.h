//Pressure.h generated on 'mer jan 13 11:28:27 CET 2010'. Edit at your own risk.
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
 * File Pressure.h
 */
#ifndef Pressure_CLASS
#define Pressure_CLASS
#include <vector>
#include <iostream>
#include <string>
using namespace std;
#ifndef WITHOUT_ACS
#include <asdmIDLTypesC.h>
using asdmIDLTypes::IDLPressure;
#endif
#include <StringTokenizer.h>
#include <NumberFormatException.h>
using asdm::StringTokenizer;
using asdm::NumberFormatException;
#include "EndianStream.h"
using asdm::EndianOSStream;
using asdm::EndianISStream;
namespace asdm {
class Pressure;
Pressure operator * ( double , const Pressure & );
ostream & operator << ( ostream &, const Pressure & );
istream & operator >> ( istream &, Pressure &);
/**
 * The Pressure class implements a quantity of pressure in hectopascals. class implements a quantity of angle in radians.
 * 
 * @version 1.00 Jan. 7, 2005
 * @author Allen Farris
 * 
 * @version 1.1 Aug 8, 2006
 * @author Michel Caillat 
 * added toBin/fromBin methods.
 */
class Pressure {
  /**
   * Overloading of multiplication operator.
   * @param d a value in double precision .
   * @param x a const reference to a Pressure .
   * @return a Pressure 
   */
  friend Pressure operator * ( double d, const Pressure & x );
  /**
   * Overloading of << to output the value an Pressure on an ostream.
   * @param os a reference to the ostream to be written on.
   * @param x a const reference to a Pressure.
   */
  friend ostream & operator << ( ostream & os, const Pressure & x);
  /**
   * Overloading of >> to read an Pressure from an istream.
   */
  friend istream & operator >> ( istream & is, Pressure & x);
public:
	/**
	 * The nullary constructor (default).
	 */
	Pressure();
	/**
	 * The copy constructor.
	 */
	Pressure(const Pressure &);
	/**
	 * A constructor from a string representation.
	 * The string passed in argument must be parsable into a double precision
	 * number to express the value in radian of the angle.
	 *
	 * @param s a string.
	 */
	Pressure(const string &s);
#ifndef WITHOUT_ACS
	/**
	 *
	 * A constructor from a CORBA/IDL representation.
	 * 
	 * @param a cons ref to an IDLPressure.
	 */
	Pressure(const IDLPressure & idlPressure);
#endif
	/**
	 * A constructor from a value in double precision.
	 * The value passed in argument defines the value of the Pressure in radian.
	 */
	Pressure(double value);
	/**
	 * The destructor.
	 */
	virtual ~Pressure();
	/**
	 * A static method equivalent to the constructor from a string.
	 * @param s a string?.
	 */
	static double fromString(const string& s);
	/**
	 * Conversion into string.
	 * The resulting string contains the representation of the value of this Pressure.
	 *
	 * @return string
	 */
	static string toString(double);
	/**
	 * Parse the next (string) token of a StringTokenizer into an angle.
	 * @param st a reference to a StringTokenizer.
	 * @return an Pressure.
	 */
	static Pressure getPressure(StringTokenizer &t) throw(NumberFormatException);
			
	/**
	 * Write the binary representation of this to an EndianOSStream .
	 * @param eoss a reference to an EndianOSStream .
	 */		
	void toBin(EndianOSStream& eoss);
	/**
	 * Write the binary representation of a vector of Pressure to a EndianOSStream.
	 * @param angle the vector of Pressure to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<Pressure>& angle,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of Pressure to a EndianOSStream.
	 * @param angle the vector of vector of Pressure to be written
	 * @param eoss the EndianOSStream to be written to
	 */	
	static void toBin(const vector<vector<Pressure> >& angle,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of vector of Pressure to a EndianOSStream.
	 * @param angle the vector of vector of vector of Pressure to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<vector<vector<Pressure> > >& angle,  EndianOSStream& eoss);
	/**
	 * Read the binary representation of an Pressure from a EndianISStream
	 * and use the read value to set an  Pressure.
	 * @param eiss the EndianStream to be read
	 * @return an Pressure
	 */
	static Pressure fromBin(EndianISStream& eiss);
	
	/**
	 * Read the binary representation of  a vector of  Pressure from an EndianISStream
	 * and use the read value to set a vector of  Pressure.
	 * @param eiis the EndianISStream to be read
	 * @return a vector of Pressure
	 */	 
	 static vector<Pressure> from1DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of Pressure from an EndianISStream
	 * and use the read value to set a vector of  vector of Pressure.
	 * @param eiis the EndianISStream to be read
	 * @return a vector of vector of Pressure
	 */	 
	 static vector<vector<Pressure> > from2DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of vector of Pressure from an EndianISStream
	 * and use the read value to set a vector of  vector of vector of Pressure.
	 * @param eiss the EndianISStream to be read
	 * @return a vector of vector of vector of Pressure
	 */	 
	 static vector<vector<vector<Pressure> > > from3DBin(EndianISStream & eiss);	 
	 
	 /**
	  * An assignment operator Pressure = Pressure.
	  * @param x a const reference to an Pressure.
	  */
	 Pressure & operator = (const Pressure & x);
	 
	 /**
	  * An assignment operator Pressure = double.
	  * @param d a value in double precision.
	  */
	 Pressure & operator = (const double d);
	 /**
	  * Operator increment and assign.
	  * @param x a const reference to an Pressure.
	  */
	Pressure & operator += (const Pressure & x);
	/**
	 * Operator decrement and assign.
	 * @param x a const reference to an Pressure.
	 */
	Pressure & operator -= (const Pressure & x);
	/**
	 * Operator multiply and assign.
	 * @param x a value in double precision.
	 */
	Pressure & operator *= (const double d);
	/**
	 * Operator divide and assign.
	 * @param x a valye in double precision.
	 */
	Pressure & operator /= (const double d);
	/**
	 * Addition operator.
	 * @param x a const reference to a Pressure.
	 */
	Pressure operator + (const Pressure & x) const;
	/**
	 * Substraction operator.
	 * @param x a const reference to a Pressure.
	 */
	Pressure operator - (const Pressure &) const;
	/**
	 * Multiplication operator.
	 * @param d a value in double precision.
	 */
	Pressure operator * (const double d) const;
	/**
	 * Division operator.
	 * @param d a value in double precision.
	 */
	Pressure operator / (const double) const;
	/**
	 * Comparison operator. Less-than.
	 * @param x a const reference to a Pressure.
	 */
	bool operator < (const Pressure & x) const;
	/**
	 * Comparison operator. Greater-than.
	 * @param x a const reference to a Pressure.
	 */
	bool operator > (const Pressure &) const;
	/**
	 * Comparison operator. Less-than or equal.
	 * @param x a const reference to a Pressure.
	 */	
	bool operator <= (const Pressure &) const;
	/**
	 * Comparison operator. Greater-than or equal.
	 * @param x a const reference to a Pressure.
	 */
	bool operator >= (const Pressure &) const;
	/**
	 * Comparision operator. Equal-to.
	 * @param x a const reference to a Pressure.
	 */
	bool operator == (const Pressure &) const;
	/** 
	 * Comparison method. Equality.
	 * @param x a const reference to a Pressure.
	 */
	bool equals(const Pressure & x) const;
	/**
	 * Comparison operator. Not-equal.
	 * @param x a const reference to a Pressure.
	 */
	bool operator != (const Pressure & x) const;
	/**
	 * Comparison method. Test nullity.
	 * @param x a const reference to a Pressure.
	 */
	bool isZero() const;
	/**
	 * Unary operator. Opposite.
	 */
	Pressure operator - () const;
	/**
	 * Unary operator. Unary plus.
	 */
	Pressure operator + () const;
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
	 * Return the double precision value of the Pressure.
	 * @return double
	 */
	double get() const;
#ifndef WITHOUT_ACS
	/**
	 * Return the IDLPressure representation of the Pressure.
	 * @return IDLPressure 
	 */
	IDLPressure toIDLPressure() const;
#endif
	/**
	 * Returns the abbreviated name of the unit implicitely associated to any Pressure.
	 * @return string
	 */
	static string unit();
private:
	double value;
};
// Pressure constructors
inline Pressure::Pressure() : value(0.0) {
}
inline Pressure::Pressure(const Pressure &t) : value(t.value) {
}
#ifndef WITHOUT_ACS
inline Pressure::Pressure(const IDLPressure &l) : value(l.value) {
}
#endif
inline Pressure::Pressure(const string &s) : value(fromString(s)) {
}
inline Pressure::Pressure(double v) : value(v) {
}
// Pressure destructor
inline Pressure::~Pressure() { }
// assignment operator
inline Pressure & Pressure::operator = ( const Pressure &t ) {
	value = t.value;
	return *this;
}
// assignment operator
inline Pressure & Pressure::operator = ( const double v ) {
	value = v;
	return *this;
}
// assignment with arithmetic operators
inline Pressure & Pressure::operator += ( const Pressure & t) {
	value += t.value;
	return *this;
}
inline Pressure & Pressure::operator -= ( const Pressure & t) {
	value -= t.value;
	return *this;
}
inline Pressure & Pressure::operator *= ( const double n) {
	value *= n;
	return *this;
}
inline Pressure & Pressure::operator /= ( const double n) {
	value /= n;
	return *this;
}
// arithmetic functions
inline Pressure Pressure::operator + ( const Pressure &t2 ) const {
	Pressure tmp;
	tmp.value = value + t2.value;
	return tmp;
}
inline Pressure Pressure::operator - ( const Pressure &t2 ) const {
	Pressure tmp;
	tmp.value = value - t2.value;
	return tmp;
}
inline Pressure Pressure::operator * ( const double n) const {
	Pressure tmp;
	tmp.value = value * n;
	return tmp;
}
inline Pressure Pressure::operator / ( const double n) const {
	Pressure tmp;
	tmp.value = value / n;
	return tmp;
}
// comparison operators
inline bool Pressure::operator < (const Pressure & x) const {
	return (value < x.value);
}
inline bool Pressure::operator > (const Pressure & x) const {
	return (value > x.value);
}
inline bool Pressure::operator <= (const Pressure & x) const {
	return (value <= x.value);
}
inline bool Pressure::operator >= (const Pressure & x) const {
	return (value >= x.value);
}
inline bool Pressure::equals(const Pressure & x) const {
	return (value == x.value);
}
inline bool Pressure::operator == (const Pressure & x) const {
	return (value == x.value);
}
inline bool Pressure::operator != (const Pressure & x) const {
	return (value != x.value);
}
// unary - and + operators
inline Pressure Pressure::operator - () const {
	Pressure tmp;
        tmp.value = -value;
	return tmp;
}
inline Pressure Pressure::operator + () const {
	Pressure tmp;
    tmp.value = value;
	return tmp;
}
// Conversion functions
inline Pressure::operator string () const {
	return toString();
}
inline string Pressure::toString() const {
	return toString(value);
}
inline string Pressure::toStringI() const {
	return toString(value);
}
inline double Pressure::get() const {
	return value;
}
#ifndef WITHOUT_ACS
inline IDLPressure Pressure::toIDLPressure() const {
	IDLPressure tmp;
	tmp.value = value;
	return tmp;
}
#endif
// Friend functions
inline Pressure operator * ( double n, const Pressure &x) {
	Pressure tmp;
	tmp.value = x.value * n;
	return tmp;
}
inline ostream & operator << ( ostream &o, const Pressure &x ) {
	o << x.value;
	return o;
}
inline istream & operator >> ( istream &i, Pressure &x ) {
	i >> x.value;
	return i;
}
inline string Pressure::unit() {
	return string ("hPa");
}
} // End namespace asdm
#endif /* Pressure_CLASS */
