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
 * File Frequency.h
 */

#ifndef Frequency_CLASS
#define Frequency_CLASS

#include <vector>
#include <iostream>
#include <string>
using namespace std;

#ifndef WITHOUT_ACS
#include <asdmIDLTypesC.h>
using asdmIDLTypes::IDLFrequency;
#endif

#include <StringTokenizer.h>
#include <NumberFormatException.h>
using asdm::StringTokenizer;
using asdm::NumberFormatException;

#include "EndianStream.h"
using asdm::EndianOSStream;
using asdm::EndianISStream;

namespace asdm {

class Frequency;
Frequency operator * ( double , const Frequency & );
ostream & operator << ( ostream &, const Frequency & );
istream & operator >> ( istream &, Frequency&);

/**
 * The Frequency class implements a quantity of frequency in hertz.
 * 
 * @version 1.00 Jan. 7, 2005
 * @author Allen Farris
 */
class Frequency {
	friend Frequency operator * ( double , const Frequency & );
    friend ostream & operator << ( ostream &, const Frequency & );
	friend istream & operator >> ( istream &, Frequency&);

public:
	static double fromString(const string&);
	static string toString(double);
	static Frequency getFrequency(StringTokenizer &t) throw(NumberFormatException);
	
		/**
	 * Write the binary representation of this to a EndianOSStream.
	 */		
	void toBin(EndianOSStream& eoss);

	/**
	 * Write the binary representation of a vector of Frequency to a EndianOSStream.
	 * @param frequency the vector of Frequency to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<Frequency>& frequency,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of Frequency to a EndianOSStream.
	 * @param frequency the vector of vector of Frequency to be written
	 * @param eoss the EndianOSStream to be written to
	 */	
	static void toBin(const vector<vector<Frequency> >& frequency,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of vector of Frequency to a EndianOSStream.
	 * @param frequency the vector of vector of vector of Frequency to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<vector<vector<Frequency> > >& frequency,  EndianOSStream& eoss);

	/**
	 * Read the binary representation of an Frequency from a EndianISStream
	 * and use the read value to set an  Frequency.
	 * @param eiss the EndianStream to be read
	 * @return an Frequency
	 */
	static Frequency fromBin(EndianISStream& eiss);
	
	/**
	 * Read the binary representation of  a vector of  Frequency from an EndianISStream
	 * and use the read value to set a vector of  Frequency.
	 * @param dis the EndianISStream to be read
	 * @return a vector of Frequency
	 */	 
	 static vector<Frequency> from1DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of Frequency from an EndianISStream
	 * and use the read value to set a vector of  vector of Frequency.
	 * @param eiis the EndianISStream to be read
	 * @return a vector of vector of Frequency
	 */	 
	 static vector<vector<Frequency> > from2DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of vector of Frequency from an EndianISStream
	 * and use the read value to set a vector of  vector of vector of Frequency.
	 * @param eiss the EndianISStream to be read
	 * @return a vector of vector of vector of Frequency
	 */	 
	 static vector<vector<vector<Frequency> > > from3DBin(EndianISStream & eiss);	

	Frequency();                              		// default constructor
	Frequency(const Frequency &);						// X const X& constructor
	Frequency(const string &s);
#ifndef WITHOUT_ACS
	Frequency(const IDLFrequency &);
#endif
	Frequency(double value);
	virtual ~Frequency();							// destructor

	Frequency& operator = (const Frequency&);			// assignment operator
	Frequency& operator = (const double);			// assignment operator

	Frequency& operator += (const Frequency&);		// assignment with arithmetic
	Frequency& operator -= (const Frequency&);		//	operators
	Frequency& operator *= (const double);
	Frequency& operator /= (const double);

	Frequency operator + (const Frequency&) const;	// arithmetic operators
	Frequency operator - (const Frequency&) const;
	Frequency operator * (const double) const;
	Frequency operator / (const double) const;

	bool operator < (const Frequency&) const;		// comparison operators
	bool operator > (const Frequency&) const;
	bool operator <= (const Frequency&) const;
	bool operator >= (const Frequency&) const;
	bool operator == (const Frequency&) const;
	bool equals(const Frequency&) const;
	bool operator != (const Frequency&) const;

	bool isZero() const;

	Frequency operator - () const;					// unary minus
	Frequency operator + () const; 				// unary plus

	string toString() const;
	string toStringI() const;

	operator string () const;
	double get() const;
#ifndef WITHOUT_ACS
	IDLFrequency toIDLFrequency() const;
#endif
	static string unit();

private:
	double value;

};

// Frequency constructors
inline Frequency::Frequency() : value(0.0) {
}

inline Frequency::Frequency(const Frequency &t) : value(t.value) {
}

#ifndef WITHOUT_ACS
inline Frequency::Frequency(const IDLFrequency &l) : value(l.value) {
}
#endif

inline Frequency::Frequency(const string &s) : value(fromString(s)) {
}

inline Frequency::Frequency(double v) : value(v) {
}

// Frequency destructor
inline Frequency::~Frequency() { }

// assignment operator
inline Frequency& Frequency::operator = ( const Frequency &t ) {
	value = t.value;
	return *this;
}

// assignment operator
inline Frequency& Frequency::operator = ( const double v ) {
	value = v;
	return *this;
}

// assignment with arithmetic operators
inline Frequency& Frequency::operator += ( const Frequency& t) {
	value += t.value;
	return *this;
}

inline Frequency& Frequency::operator -= ( const Frequency& t) {
	value -= t.value;
	return *this;
}

inline Frequency& Frequency::operator *= ( const double n) {
	value *= n;
	return *this;
}

inline Frequency& Frequency::operator /= ( const double n) {
	value /= n;
	return *this;
}

// arithmetic functions
inline Frequency Frequency::operator + ( const Frequency &t2 ) const {
	Frequency tmp;
	tmp.value = value + t2.value;
	return tmp;
}

inline Frequency Frequency::operator - ( const Frequency &t2 ) const {
	Frequency tmp;
	tmp.value = value - t2.value;
	return tmp;
}
inline Frequency Frequency::operator * ( const double n) const {
	Frequency tmp;
	tmp.value = value * n;
	return tmp;
}

inline Frequency Frequency::operator / ( const double n) const {
	Frequency tmp;
	tmp.value = value / n;
	return tmp;
}

// comparison operators
inline bool Frequency::operator < (const Frequency& x) const {
	return (value < x.value);
}

inline bool Frequency::operator > (const Frequency& x) const {
	return (value > x.value);
}

inline bool Frequency::operator <= (const Frequency& x) const {
	return (value <= x.value);
}

inline bool Frequency::operator >= (const Frequency& x) const {
	return (value >= x.value);
}

inline bool Frequency::operator == (const Frequency& x) const {
	return (value == x.value);
}
inline bool Frequency::equals(const Frequency& x) const {
	return (value == x.value);
}

inline bool Frequency::operator != (const Frequency& x) const {
	return (value != x.value);
}

// unary - and + operators
inline Frequency Frequency::operator - () const {
	Frequency tmp;
        tmp.value = -value;
	return tmp;
}

inline Frequency Frequency::operator + () const {
	Frequency tmp;
    tmp.value = value;
	return tmp;
}

// Conversion functions
inline Frequency::operator string () const {
	return toString();
}

inline string Frequency::toString() const {
	return toString(value);
}

inline string Frequency::toStringI() const {
	return toString(value);
}

inline double Frequency::get() const {
	return value;
}

#ifndef WITHOUT_ACS
inline IDLFrequency Frequency::toIDLFrequency() const {
	IDLFrequency tmp;
	tmp.value = value;
	return tmp;
}
#endif

// Friend functions

inline Frequency operator * ( double n, const Frequency &x) {
	Frequency tmp;
	tmp.value = x.value * n;
	return tmp;
}

inline ostream & operator << ( ostream &o, const Frequency &x ) {
	o << x.value;
	return o;
}

inline istream & operator >> ( istream &i, Frequency &x ) {
	i >> x.value;
	return i;
}

inline string Frequency::unit() {
	return string ("hz");
}

} // End namespace asdm

#endif /* Frequency_CLASS */
