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
istream & operator >> ( istream &, Flux&);

/**
 * The Flux class implements a quantity of flux in janskys.
 * 
 * @version 1.00 Jan. 7, 2005
 * @author Allen Farris
 * @version 1.1 Aug 8, 2006
 * @author Michel Caillat 
 * added toBin/fromBin methods.
 */
class Flux {
	friend Flux operator * ( double , const Flux & );
    friend ostream & operator << ( ostream &, const Flux & );
	friend istream & operator >> ( istream &, Flux&);

public:
	static double fromString(const string&);
	static string toString(double);
	
	/**
	 * Write the binary representation of this to a EndianOSStream.
	 */		
	void toBin(EndianOSStream& eoss);

	/**
	 * Write the binary representation of a vector of Flux to a EndianOSStream.
	 * @param flux the vector of Flux to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<Flux>& flux,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of Flux to a EndianOSStream.
	 * @param flux the vector of vector of Flux to be written
	 * @param eoss the EndianOSStream to be written to
	 */	
	static void toBin(const vector<vector<Flux> >& flux,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of vector of Flux to a EndianOSStream.
	 * @param flux the vector of vector of vector of Flux to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<vector<vector<Flux> > >& flux,  EndianOSStream& eoss);

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
	 * @param dis the EndianISStream to be read
	 * @return a vector of Flux
	 */	 
	 static vector<Flux> from1DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of Flux from an EndianISStream
	 * and use the read value to set a vector of  vector of Flux.
	 * @param eiis the EndianISStream to be read
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
	
	static Flux getFlux(StringTokenizer &t) throw(NumberFormatException);

	Flux();                              		// default constructor
	Flux(const Flux &);						// X const X& constructor
	Flux(const string &s);
#ifndef WITHOUT_ACS
	Flux(const IDLFlux &);
#endif
	Flux(double value);
	virtual ~Flux();							// destructor

	Flux& operator = (const Flux&);			// assignment operator
	Flux& operator = (const double);			// assignment operator

	Flux& operator += (const Flux&);		// assignment with arithmetic
	Flux& operator -= (const Flux&);		//	operators
	Flux& operator *= (const double);
	Flux& operator /= (const double);

	Flux operator + (const Flux&) const;	// arithmetic operators
	Flux operator - (const Flux&) const;
	Flux operator * (const double) const;
	Flux operator / (const double) const;

	bool operator < (const Flux&) const;		// comparison operators
	bool operator > (const Flux&) const;
	bool operator <= (const Flux&) const;
	bool operator >= (const Flux&) const;
	bool operator == (const Flux&) const;
	bool equals(const Flux&) const;
	bool operator != (const Flux&) const;

	bool isZero() const;

	Flux operator - () const;					// unary minus
	Flux operator + () const; 				// unary plus

	string toString() const;
	string toStringI() const;

	operator string () const;
	double get() const;
#ifndef WITHOUT_ACS
	IDLFlux toIDLFlux() const;
#endif

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
inline Flux& Flux::operator = ( const Flux &t ) {
	value = t.value;
	return *this;
}

// assignment operator
inline Flux& Flux::operator = ( const double v ) {
	value = v;
	return *this;
}

// assignment with arithmetic operators
inline Flux& Flux::operator += ( const Flux& t) {
	value += t.value;
	return *this;
}

inline Flux& Flux::operator -= ( const Flux& t) {
	value -= t.value;
	return *this;
}

inline Flux& Flux::operator *= ( const double n) {
	value *= n;
	return *this;
}

inline Flux& Flux::operator /= ( const double n) {
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
inline bool Flux::operator < (const Flux& x) const {
	return (value < x.value);
}

inline bool Flux::operator > (const Flux& x) const {
	return (value > x.value);
}

inline bool Flux::operator <= (const Flux& x) const {
	return (value <= x.value);
}

inline bool Flux::operator >= (const Flux& x) const {
	return (value >= x.value);
}

inline bool Flux::operator == (const Flux& x) const {
	return (value == x.value);
}
inline bool Flux::equals(const Flux& x) const {
	return (value == x.value);
}

inline bool Flux::operator != (const Flux& x) const {
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
