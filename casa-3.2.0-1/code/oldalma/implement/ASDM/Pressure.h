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
istream & operator >> ( istream &, Pressure&);

/**
 * The Pressure class implements a quantity of pressure in hectopascals.
 * 
 * @version 1.00 Jan. 7, 2005
 * @author Allen Farris
 * @version 1.1 Aug 8, 2006
 * @author Michel Caillat 
 * added toBin/fromBin methods.
 */
class Pressure {
	friend Pressure operator * ( double , const Pressure & );
	friend ostream & operator << ( ostream &, const Pressure & );
	friend istream & operator >> ( istream &, Pressure&);

public:
	static double fromString(const string&);
	static string toString(double);
	static Pressure getPressure(StringTokenizer &t) throw(NumberFormatException);
	
	/**
	 * Write the binary representation of this to a EndianOSStream.
	 */		
	void toBin(EndianOSStream& eoss);

	/**
	 * Write the binary representation of a vector of Pressure to a EndianOSStream.
	 * @param pressure the vector of Pressure to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<Pressure>& pressure,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of Pressure to a EndianOSStream.
	 * @param pressure the vector of vector of Pressure to be written
	 * @param eoss the EndianOSStream to be written to
	 */	
	static void toBin(const vector<vector<Pressure> >& pressure,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of vector of Pressure to a EndianOSStream.
	 * @param pressure the vector of vector of vector of Pressure to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<vector<vector<Pressure> > >& pressure,  EndianOSStream& eoss);

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
	 * @param dis the EndianISStream to be read
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

	Pressure();                              		// default constructor
	Pressure(const Pressure &);						// X const X& constructor
	Pressure(const string &s);
#ifndef WITHOUT_ACS
	Pressure(const IDLPressure &);
#endif
	Pressure(double value);
	virtual ~Pressure();							// destructor

	Pressure& operator = (const Pressure&);			// assignment operator
	Pressure& operator = (const double);			// assignment operator

	Pressure& operator += (const Pressure&);		// assignment with arithmetic
	Pressure& operator -= (const Pressure&);		//	operators
	Pressure& operator *= (const double);
	Pressure& operator /= (const double);

	Pressure operator + (const Pressure&) const;	// arithmetic operators
	Pressure operator - (const Pressure&) const;
	Pressure operator * (const double) const;
	Pressure operator / (const double) const;

	bool operator < (const Pressure&) const;		// comparison operators
	bool operator > (const Pressure&) const;
	bool operator <= (const Pressure&) const;
	bool operator >= (const Pressure&) const;
	bool operator == (const Pressure&) const;
	bool equals(const Pressure&) const;
	bool operator != (const Pressure&) const;

	bool isZero() const;

	Pressure operator - () const;					// unary minus
	Pressure operator + () const; 				// unary plus

	string toString() const;
	string toStringI() const;

	operator string () const;
	double get() const;
#ifndef WITHOUT_ACS
	IDLPressure toIDLPressure() const;
#endif
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
inline Pressure& Pressure::operator = ( const Pressure &t ) {
	value = t.value;
	return *this;
}

// assignment operator
inline Pressure& Pressure::operator = ( const double v ) {
	value = v;
	return *this;
}

// assignment with arithmetic operators
inline Pressure& Pressure::operator += ( const Pressure& t) {
	value += t.value;
	return *this;
}

inline Pressure& Pressure::operator -= ( const Pressure& t) {
	value -= t.value;
	return *this;
}

inline Pressure& Pressure::operator *= ( const double n) {
	value *= n;
	return *this;
}

inline Pressure& Pressure::operator /= ( const double n) {
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
inline bool Pressure::operator < (const Pressure& x) const {
	return (value < x.value);
}

inline bool Pressure::operator > (const Pressure& x) const {
	return (value > x.value);
}

inline bool Pressure::operator <= (const Pressure& x) const {
	return (value <= x.value);
}

inline bool Pressure::operator >= (const Pressure& x) const {
	return (value >= x.value);
}

inline bool Pressure::operator == (const Pressure& x) const {
	return (value == x.value);
}
inline bool Pressure::equals(const Pressure& x) const {
	return (value == x.value);
}

inline bool Pressure::operator != (const Pressure& x) const {
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
	return string ("hp");
}

} // End namespace asdm

#endif /* Pressure_CLASS */
