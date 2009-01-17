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
istream & operator >> ( istream &, Length&);

/**
 * The Length class implements a quantity of length in meters.
 * 
 * @version 1.00 Jan. 7, 2005
 * @author Allen Farris
 * @version 1.1 Aug 8, 2006
 * @author Michel Caillat 
 * added toBin/fromBin methods.
 */
class Length {
	friend Length operator * ( double , const Length & );
    friend ostream & operator << ( ostream &, const Length & );
	friend istream & operator >> ( istream &, Length&);

public:
	static double fromString(const string&);
	static string toString(double);
	static Length getLength(StringTokenizer &t) throw(NumberFormatException);
	
	/**
	 * Write the binary representation of this to a EndianOSStream.
	 */		
	void toBin(EndianOSStream& eoss);

	/**
	 * Write the binary representation of a vector of Length to a EndianOSStream.
	 * @param len the vector of Length to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<Length>& len,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of Length to a EndianOSStream.
	 * @param len the vector of vector of Length to be written
	 * @param eoss the EndianOSStream to be written to
	 */	
	static void toBin(const vector<vector<Length> >& len,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of vector of Length to a EndianOSStream.
	 * @param len the vector of vector of vector of Length to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<vector<vector<Length> > >& len,  EndianOSStream& eoss);

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
	 * @param dis the EndianISStream to be read
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
	 	

	Length();                              		// default constructor
	Length(const Length &);						// X const X& constructor
	Length(const string &s);
#ifndef WITHOUT_ACS	
	Length(const IDLLength &);
#endif
	Length(double value);
	virtual ~Length();							// destructor

	Length& operator = (const Length&);			// assignment operator
	Length& operator = (const double);			// assignment operator

	Length& operator += (const Length&);		// assignment with arithmetic
	Length& operator -= (const Length&);		//	operators
	Length& operator *= (const double);
	Length& operator /= (const double);

	Length operator + (const Length&) const;	// arithmetic operators
	Length operator - (const Length&) const;
	Length operator * (const double) const;
	Length operator / (const double) const;

	bool operator < (const Length&) const;		// comparison operators
	bool operator > (const Length&) const;
	bool operator <= (const Length&) const;
	bool operator >= (const Length&) const;
	bool operator == (const Length&) const;
	bool equals(const Length&) const;
	bool operator != (const Length&) const;

	bool isZero() const;

	Length operator - () const;					// unary minus
	Length operator + () const; 				// unary plus

	string toString() const;
	string toStringI() const;

	operator string () const;
	double get() const;
#ifndef WITHOUT_ACS
	IDLLength toIDLLength() const;
#endif
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
inline Length& Length::operator = ( const Length &t ) {
	value = t.value;
	return *this;
}

// assignment operator
inline Length& Length::operator = ( const double v ) {
	value = v;
	return *this;
}

// assignment with arithmetic operators
inline Length& Length::operator += ( const Length& t) {
	value += t.value;
	return *this;
}

inline Length& Length::operator -= ( const Length& t) {
	value -= t.value;
	return *this;
}

inline Length& Length::operator *= ( const double n) {
	value *= n;
	return *this;
}

inline Length& Length::operator /= ( const double n) {
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
inline bool Length::operator < (const Length& x) const {
	return (value < x.value);
}

inline bool Length::operator > (const Length& x) const {
	return (value > x.value);
}

inline bool Length::operator <= (const Length& x) const {
	return (value <= x.value);
}

inline bool Length::operator >= (const Length& x) const {
	return (value >= x.value);
}

inline bool Length::operator == (const Length& x) const {
	return (value == x.value);
}
inline bool Length::equals(const Length& x) const {
	return (value == x.value);
}

inline bool Length::operator != (const Length& x) const {
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
