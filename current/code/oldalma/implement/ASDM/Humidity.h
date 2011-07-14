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
 * File Humidity.h
 */

#ifndef Humidity_CLASS
#define Humidity_CLASS

#include <vector>
#include <iostream>
#include <string>
using namespace std;

#ifndef WITHOUT_ACS
#include <asdmIDLTypesC.h>
using asdmIDLTypes::IDLHumidity;
#endif

#include <StringTokenizer.h>
#include <NumberFormatException.h>
using asdm::StringTokenizer;
using asdm::NumberFormatException;

#include "EndianStream.h"
using asdm::EndianOSStream;
using asdm::EndianISStream;

namespace asdm {

class Humidity;
Humidity operator * ( double , const Humidity & );
ostream & operator << ( ostream &, const Humidity & );
istream & operator >> ( istream &, Humidity&);

/**
 * The Humidity class implements a quantity of humidity in percent.
 * 
 * @version 1.00 Jan. 7, 2005
 * @author Allen Farris
 * @version 1.1 Aug 8, 2006
 * @author Michel Caillat 
 * added toBin/fromBin methods.
 */
class Humidity {
	friend Humidity operator * ( double , const Humidity & );
    friend ostream & operator << ( ostream &, const Humidity & );
	friend istream & operator >> ( istream &, Humidity&);

public:
	static double fromString(const string&);
	static string toString(double);
	static Humidity getHumidity(StringTokenizer &t) throw(NumberFormatException);

	/**
	 * Write the binary representation of this to a EndianOSStream.
	 */		
	void toBin(EndianOSStream& eoss);

	/**
	 * Write the binary representation of a vector of Humidity to a EndianOSStream.
	 * @param humidity the vector of Humidity to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<Humidity>& humidity,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of Humidity to a EndianOSStream.
	 * @param humidity the vector of vector of Humidity to be written
	 * @param eoss the EndianOSStream to be written to
	 */	
	static void toBin(const vector<vector<Humidity> >& humidity,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of vector of Humidity to a EndianOSStream.
	 * @param humidity the vector of vector of vector of Humidity to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<vector<vector<Humidity> > >& humidity,  EndianOSStream& eoss);

	/**
	 * Read the binary representation of an Humidity from a EndianISStream
	 * and use the read value to set an  Humidity.
	 * @param eiss the EndianStream to be read
	 * @return an Humidity
	 */
	static Humidity fromBin(EndianISStream& eiss);
	
	/**
	 * Read the binary representation of  a vector of  Humidity from an EndianISStream
	 * and use the read value to set a vector of  Humidity.
	 * @param dis the EndianISStream to be read
	 * @return a vector of Humidity
	 */	 
	 static vector<Humidity> from1DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of Humidity from an EndianISStream
	 * and use the read value to set a vector of  vector of Humidity.
	 * @param eiis the EndianISStream to be read
	 * @return a vector of vector of Humidity
	 */	 
	 static vector<vector<Humidity> > from2DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of vector of Humidity from an EndianISStream
	 * and use the read value to set a vector of  vector of vector of Humidity.
	 * @param eiss the EndianISStream to be read
	 * @return a vector of vector of vector of Humidity
	 */	 
	 static vector<vector<vector<Humidity> > > from3DBin(EndianISStream & eiss);	 
	Humidity();                              		// default constructor
	Humidity(const Humidity &);						// X const X& constructor
	Humidity(const string &s);
#ifndef WITHOUT_ACS
	Humidity(const IDLHumidity &);
#endif
	Humidity(double value);
	virtual ~Humidity();							// destructor

	Humidity& operator = (const Humidity&);			// assignment operator
	Humidity& operator = (const double);			// assignment operator

	Humidity& operator += (const Humidity&);		// assignment with arithmetic
	Humidity& operator -= (const Humidity&);		//	operators
	Humidity& operator *= (const double);
	Humidity& operator /= (const double);

	Humidity operator + (const Humidity&) const;	// arithmetic operators
	Humidity operator - (const Humidity&) const;
	Humidity operator * (const double) const;
	Humidity operator / (const double) const;

	bool operator < (const Humidity&) const;		// comparison operators
	bool operator > (const Humidity&) const;
	bool operator <= (const Humidity&) const;
	bool operator >= (const Humidity&) const;
	bool operator == (const Humidity&) const;
	bool equals(const Humidity&) const;
	bool operator != (const Humidity&) const;

	bool isZero() const;

	Humidity operator - () const;					// unary minus
	Humidity operator + () const; 				// unary plus

	string toString() const;
	string toStringI() const;

	operator string () const;
	double get() const;
#ifndef WITHOUT_ACS
	IDLHumidity toIDLHumidity() const;
#endif
	static string unit();

private:
	double value;

};

// Humidity constructors
inline Humidity::Humidity() : value(0.0) {
}

inline Humidity::Humidity(const Humidity &t) : value(t.value) {
}

#ifndef WITHOUT_ACS
inline Humidity::Humidity(const IDLHumidity &l) : value(l.value) {
}
#endif

inline Humidity::Humidity(const string &s) : value(fromString(s)) {
}

inline Humidity::Humidity(double v) : value(v) {
}

// Humidity destructor
inline Humidity::~Humidity() { }

// assignment operator
inline Humidity& Humidity::operator = ( const Humidity &t ) {
	value = t.value;
	return *this;
}

// assignment operator
inline Humidity& Humidity::operator = ( const double v ) {
	value = v;
	return *this;
}

// assignment with arithmetic operators
inline Humidity& Humidity::operator += ( const Humidity& t) {
	value += t.value;
	return *this;
}

inline Humidity& Humidity::operator -= ( const Humidity& t) {
	value -= t.value;
	return *this;
}

inline Humidity& Humidity::operator *= ( const double n) {
	value *= n;
	return *this;
}

inline Humidity& Humidity::operator /= ( const double n) {
	value /= n;
	return *this;
}

// arithmetic functions
inline Humidity Humidity::operator + ( const Humidity &t2 ) const {
	Humidity tmp;
	tmp.value = value + t2.value;
	return tmp;
}

inline Humidity Humidity::operator - ( const Humidity &t2 ) const {
	Humidity tmp;
	tmp.value = value - t2.value;
	return tmp;
}
inline Humidity Humidity::operator * ( const double n) const {
	Humidity tmp;
	tmp.value = value * n;
	return tmp;
}

inline Humidity Humidity::operator / ( const double n) const {
	Humidity tmp;
	tmp.value = value / n;
	return tmp;
}

// comparison operators
inline bool Humidity::operator < (const Humidity& x) const {
	return (value < x.value);
}

inline bool Humidity::operator > (const Humidity& x) const {
	return (value > x.value);
}

inline bool Humidity::operator <= (const Humidity& x) const {
	return (value <= x.value);
}

inline bool Humidity::operator >= (const Humidity& x) const {
	return (value >= x.value);
}

inline bool Humidity::operator == (const Humidity& x) const {
	return (value == x.value);
}
inline bool Humidity::equals(const Humidity& x) const {
	return (value == x.value);
}

inline bool Humidity::operator != (const Humidity& x) const {
	return (value != x.value);
}

// unary - and + operators
inline Humidity Humidity::operator - () const {
	Humidity tmp;
        tmp.value = -value;
	return tmp;
}

inline Humidity Humidity::operator + () const {
	Humidity tmp;
    tmp.value = value;
	return tmp;
}

// Conversion functions
inline Humidity::operator string () const {
	return toString();
}

inline string Humidity::toString() const {
	return toString(value);
}

inline string Humidity::toStringI() const {
	return toString(value);
}

inline double Humidity::get() const {
	return value;
}

#ifndef WITHOUT_ACS
inline IDLHumidity Humidity::toIDLHumidity() const {
	IDLHumidity tmp;
	tmp.value = value;
	return tmp;
}
#endif

// Friend functions

inline Humidity operator * ( double n, const Humidity &x) {
	Humidity tmp;
	tmp.value = x.value * n;
	return tmp;
}

inline ostream & operator << ( ostream &o, const Humidity &x ) {
	o << x.value;
	return o;
}

inline istream & operator >> ( istream &i, Humidity &x ) {
	i >> x.value;
	return i;
}

inline string Humidity::unit() {
	return string ("%");
}

} // End namespace asdm

#endif /* Humidity_CLASS */
