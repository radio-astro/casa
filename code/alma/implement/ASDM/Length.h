//Length.h generated on 'mer jan 13 11:28:27 CET 2010'. Edit at your own risk.
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
 * File Length.h
 */
#ifndef Length_CLASS
#define Length_CLASS
#include <vector>
#include <iostream>
#include <string>
using namespace std;
#ifndef WITHOUT_ACS
#include <asdmIDLTypesC.h>
using asdmIDLTypes::IDLLength;
#endif
#include <StringTokenizer.h>
#include <NumberFormatException.h>
using asdm::StringTokenizer;
using asdm::NumberFormatException;
#include "EndianStream.h"
using asdm::EndianOSStream;
using asdm::EndianISStream;
namespace asdm {
class Length;
Length operator * ( double , const Length & );
ostream & operator << ( ostream &, const Length & );
istream & operator >> ( istream &, Length &);
/**
 * The Length class implements a quantity of length in meters. class implements a quantity of angle in radians.
 * 
 * @version 1.00 Jan. 7, 2005
 * @author Allen Farris
 * 
 * @version 1.1 Aug 8, 2006
 * @author Michel Caillat 
 * added toBin/fromBin methods.
 */
class Length {
  /**
   * Overloading of multiplication operator.
   * @param d a value in double precision .
   * @param x a const reference to a Length .
   * @return a Length 
   */
  friend Length operator * ( double d, const Length & x );
  /**
   * Overloading of << to output the value an Length on an ostream.
   * @param os a reference to the ostream to be written on.
   * @param x a const reference to a Length.
   */
  friend ostream & operator << ( ostream & os, const Length & x);
  /**
   * Overloading of >> to read an Length from an istream.
   */
  friend istream & operator >> ( istream & is, Length & x);
public:
	/**
	 * The nullary constructor (default).
	 */
	Length();
	/**
	 * The copy constructor.
	 */
	Length(const Length &);
	/**
	 * A constructor from a string representation.
	 * The string passed in argument must be parsable into a double precision
	 * number to express the value in radian of the angle.
	 *
	 * @param s a string.
	 */
	Length(const string &s);
#ifndef WITHOUT_ACS
	/**
	 *
	 * A constructor from a CORBA/IDL representation.
	 * 
	 * @param a cons ref to an IDLLength.
	 */
	Length(const IDLLength & idlLength);
#endif
	/**
	 * A constructor from a value in double precision.
	 * The value passed in argument defines the value of the Length in radian.
	 */
	Length(double value);
	/**
	 * The destructor.
	 */
	virtual ~Length();
	/**
	 * A static method equivalent to the constructor from a string.
	 * @param s a string?.
	 */
	static double fromString(const string& s);
	/**
	 * Conversion into string.
	 * The resulting string contains the representation of the value of this Length.
	 *
	 * @return string
	 */
	static string toString(double);
	/**
	 * Parse the next (string) token of a StringTokenizer into an angle.
	 * @param st a reference to a StringTokenizer.
	 * @return an Length.
	 */
	static Length getLength(StringTokenizer &t) throw(NumberFormatException);
			
	/**
	 * Write the binary representation of this to an EndianOSStream .
	 * @param eoss a reference to an EndianOSStream .
	 */		
	void toBin(EndianOSStream& eoss);
	/**
	 * Write the binary representation of a vector of Length to a EndianOSStream.
	 * @param angle the vector of Length to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<Length>& angle,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of Length to a EndianOSStream.
	 * @param angle the vector of vector of Length to be written
	 * @param eoss the EndianOSStream to be written to
	 */	
	static void toBin(const vector<vector<Length> >& angle,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of vector of Length to a EndianOSStream.
	 * @param angle the vector of vector of vector of Length to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<vector<vector<Length> > >& angle,  EndianOSStream& eoss);
	/**
	 * Read the binary representation of an Length from a EndianISStream
	 * and use the read value to set an  Length.
	 * @param eiss the EndianStream to be read
	 * @return an Length
	 */
	static Length fromBin(EndianISStream& eiss);
	
	/**
	 * Read the binary representation of  a vector of  Length from an EndianISStream
	 * and use the read value to set a vector of  Length.
	 * @param eiis the EndianISStream to be read
	 * @return a vector of Length
	 */	 
	 static vector<Length> from1DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of Length from an EndianISStream
	 * and use the read value to set a vector of  vector of Length.
	 * @param eiis the EndianISStream to be read
	 * @return a vector of vector of Length
	 */	 
	 static vector<vector<Length> > from2DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of vector of Length from an EndianISStream
	 * and use the read value to set a vector of  vector of vector of Length.
	 * @param eiss the EndianISStream to be read
	 * @return a vector of vector of vector of Length
	 */	 
	 static vector<vector<vector<Length> > > from3DBin(EndianISStream & eiss);	 
	 
	 /**
	  * An assignment operator Length = Length.
	  * @param x a const reference to an Length.
	  */
	 Length & operator = (const Length & x);
	 
	 /**
	  * An assignment operator Length = double.
	  * @param d a value in double precision.
	  */
	 Length & operator = (const double d);
	 /**
	  * Operator increment and assign.
	  * @param x a const reference to an Length.
	  */
	Length & operator += (const Length & x);
	/**
	 * Operator decrement and assign.
	 * @param x a const reference to an Length.
	 */
	Length & operator -= (const Length & x);
	/**
	 * Operator multiply and assign.
	 * @param x a value in double precision.
	 */
	Length & operator *= (const double d);
	/**
	 * Operator divide and assign.
	 * @param x a valye in double precision.
	 */
	Length & operator /= (const double d);
	/**
	 * Addition operator.
	 * @param x a const reference to a Length.
	 */
	Length operator + (const Length & x) const;
	/**
	 * Substraction operator.
	 * @param x a const reference to a Length.
	 */
	Length operator - (const Length &) const;
	/**
	 * Multiplication operator.
	 * @param d a value in double precision.
	 */
	Length operator * (const double d) const;
	/**
	 * Division operator.
	 * @param d a value in double precision.
	 */
	Length operator / (const double) const;
	/**
	 * Comparison operator. Less-than.
	 * @param x a const reference to a Length.
	 */
	bool operator < (const Length & x) const;
	/**
	 * Comparison operator. Greater-than.
	 * @param x a const reference to a Length.
	 */
	bool operator > (const Length &) const;
	/**
	 * Comparison operator. Less-than or equal.
	 * @param x a const reference to a Length.
	 */	
	bool operator <= (const Length &) const;
	/**
	 * Comparison operator. Greater-than or equal.
	 * @param x a const reference to a Length.
	 */
	bool operator >= (const Length &) const;
	/**
	 * Comparision operator. Equal-to.
	 * @param x a const reference to a Length.
	 */
	bool operator == (const Length &) const;
	/** 
	 * Comparison method. Equality.
	 * @param x a const reference to a Length.
	 */
	bool equals(const Length & x) const;
	/**
	 * Comparison operator. Not-equal.
	 * @param x a const reference to a Length.
	 */
	bool operator != (const Length & x) const;
	/**
	 * Comparison method. Test nullity.
	 * @param x a const reference to a Length.
	 */
	bool isZero() const;
	/**
	 * Unary operator. Opposite.
	 */
	Length operator - () const;
	/**
	 * Unary operator. Unary plus.
	 */
	Length operator + () const;
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
	 * Return the double precision value of the Length.
	 * @return double
	 */
	double get() const;
#ifndef WITHOUT_ACS
	/**
	 * Return the IDLLength representation of the Length.
	 * @return IDLLength 
	 */
	IDLLength toIDLLength() const;
#endif
	/**
	 * Returns the abbreviated name of the unit implicitely associated to any Length.
	 * @return string
	 */
	static string unit();
private:
	double value;
};
// Length constructors
inline Length::Length() : value(0.0) {
}
inline Length::Length(const Length &t) : value(t.value) {
}
#ifndef WITHOUT_ACS
inline Length::Length(const IDLLength &l) : value(l.value) {
}
#endif
inline Length::Length(const string &s) : value(fromString(s)) {
}
inline Length::Length(double v) : value(v) {
}
// Length destructor
inline Length::~Length() { }
// assignment operator
inline Length & Length::operator = ( const Length &t ) {
	value = t.value;
	return *this;
}
// assignment operator
inline Length & Length::operator = ( const double v ) {
	value = v;
	return *this;
}
// assignment with arithmetic operators
inline Length & Length::operator += ( const Length & t) {
	value += t.value;
	return *this;
}
inline Length & Length::operator -= ( const Length & t) {
	value -= t.value;
	return *this;
}
inline Length & Length::operator *= ( const double n) {
	value *= n;
	return *this;
}
inline Length & Length::operator /= ( const double n) {
	value /= n;
	return *this;
}
// arithmetic functions
inline Length Length::operator + ( const Length &t2 ) const {
	Length tmp;
	tmp.value = value + t2.value;
	return tmp;
}
inline Length Length::operator - ( const Length &t2 ) const {
	Length tmp;
	tmp.value = value - t2.value;
	return tmp;
}
inline Length Length::operator * ( const double n) const {
	Length tmp;
	tmp.value = value * n;
	return tmp;
}
inline Length Length::operator / ( const double n) const {
	Length tmp;
	tmp.value = value / n;
	return tmp;
}
// comparison operators
inline bool Length::operator < (const Length & x) const {
	return (value < x.value);
}
inline bool Length::operator > (const Length & x) const {
	return (value > x.value);
}
inline bool Length::operator <= (const Length & x) const {
	return (value <= x.value);
}
inline bool Length::operator >= (const Length & x) const {
	return (value >= x.value);
}
inline bool Length::equals(const Length & x) const {
	return (value == x.value);
}
inline bool Length::operator == (const Length & x) const {
	return (value == x.value);
}
inline bool Length::operator != (const Length & x) const {
	return (value != x.value);
}
// unary - and + operators
inline Length Length::operator - () const {
	Length tmp;
        tmp.value = -value;
	return tmp;
}
inline Length Length::operator + () const {
	Length tmp;
    tmp.value = value;
	return tmp;
}
// Conversion functions
inline Length::operator string () const {
	return toString();
}
inline string Length::toString() const {
	return toString(value);
}
inline string Length::toStringI() const {
	return toString(value);
}
inline double Length::get() const {
	return value;
}
#ifndef WITHOUT_ACS
inline IDLLength Length::toIDLLength() const {
	IDLLength tmp;
	tmp.value = value;
	return tmp;
}
#endif
// Friend functions
inline Length operator * ( double n, const Length &x) {
	Length tmp;
	tmp.value = x.value * n;
	return tmp;
}
inline ostream & operator << ( ostream &o, const Length &x ) {
	o << x.value;
	return o;
}
inline istream & operator >> ( istream &i, Length &x ) {
	i >> x.value;
	return i;
}
inline string Length::unit() {
	return string ("m");
}
} // End namespace asdm
#endif /* Length_CLASS */
