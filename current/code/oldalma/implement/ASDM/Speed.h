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
istream & operator >> ( istream &, Speed&);

/**
 * The Speed class implements a quantity of speed in meters per second.
 * 
 * @version 1.00 Jan. 7, 2005
 * @author Allen Farris
 * @version 1.1 Aug 8, 2006
 * @author Michel Caillat 
 * added toBin/fromBin methods.
 */
class Speed {
	friend Speed operator * ( double , const Speed & );
    friend ostream & operator << ( ostream &, const Speed & );
	friend istream & operator >> ( istream &, Speed&);

public:
	static double fromString(const string&);
	static string toString(double);
	static Speed getSpeed(StringTokenizer &t) throw(NumberFormatException);
	
		/**
	 * Write the binary representation of this to a EndianOSStream.
	 */		
	void toBin(EndianOSStream& eoss);

	/**
	 * Write the binary representation of a vector of Speed to a EndianOSStream.
	 * @param speed the vector of Speed to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<Speed>& speed,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of Speed to a EndianOSStream.
	 * @param speed the vector of vector of Speed to be written
	 * @param eoss the EndianOSStream to be written to
	 */	
	static void toBin(const vector<vector<Speed> >& speed,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of vector of Speed to a EndianOSStream.
	 * @param speed the vector of vector of vector of Speed to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<vector<vector<Speed> > >& speed,  EndianOSStream& eoss);

	/**
	 * Read the binary representation of an Speed from a EndianISStream
	 * and use the read value to set an  Speed.
	 * @param eiss the EndianISStream to be read
	 * @return an Speed
	 */
	static Speed fromBin(EndianISStream& eiss);
	
	/**
	 * Read the binary representation of  a vector of  Speed from an EndianISStream
	 * and use the read value to set a vector of  Speed.
	 * @param dis the EndianISStream to be read
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

	Speed();                              		// default constructor
	Speed(const Speed &);						// X const X& constructor
	Speed(const string &s);
#ifndef WITHOUT_ACS
	Speed(const IDLSpeed &);
#endif
	Speed(double value);
	virtual ~Speed();							// destructor

	Speed& operator = (const Speed&);			// assignment operator
	Speed& operator = (const double);			// assignment operator

	Speed& operator += (const Speed&);		// assignment with arithmetic
	Speed& operator -= (const Speed&);		//	operators
	Speed& operator *= (const double);
	Speed& operator /= (const double);

	Speed operator + (const Speed&) const;	// arithmetic operators
	Speed operator - (const Speed&) const;
	Speed operator * (const double) const;
	Speed operator / (const double) const;

	bool operator < (const Speed&) const;		// comparison operators
	bool operator > (const Speed&) const;
	bool operator <= (const Speed&) const;
	bool operator >= (const Speed&) const;
	bool operator == (const Speed&) const;
	bool equals(const Speed&) const;
	bool operator != (const Speed&) const;

	bool isZero() const;

	Speed operator - () const;					// unary minus
	Speed operator + () const; 				// unary plus

	string toString() const;
	string toStringI() const;

	operator string () const;
	double get() const;
#ifndef WITHOUT_ACS
	IDLSpeed toIDLSpeed() const;
#endif
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
inline Speed& Speed::operator = ( const Speed &t ) {
	value = t.value;
	return *this;
}

// assignment operator
inline Speed& Speed::operator = ( const double v ) {
	value = v;
	return *this;
}

// assignment with arithmetic operators
inline Speed& Speed::operator += ( const Speed& t) {
	value += t.value;
	return *this;
}

inline Speed& Speed::operator -= ( const Speed& t) {
	value -= t.value;
	return *this;
}

inline Speed& Speed::operator *= ( const double n) {
	value *= n;
	return *this;
}

inline Speed& Speed::operator /= ( const double n) {
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
inline bool Speed::operator < (const Speed& x) const {
	return (value < x.value);
}

inline bool Speed::operator > (const Speed& x) const {
	return (value > x.value);
}

inline bool Speed::operator <= (const Speed& x) const {
	return (value <= x.value);
}

inline bool Speed::operator >= (const Speed& x) const {
	return (value >= x.value);
}

inline bool Speed::operator == (const Speed& x) const {
	return (value == x.value);
}
inline bool Speed::equals(const Speed& x) const {
	return (value == x.value);
}

inline bool Speed::operator != (const Speed& x) const {
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
	return string ("m/sec");
}

} // End namespace asdm

#endif /* Speed_CLASS */
