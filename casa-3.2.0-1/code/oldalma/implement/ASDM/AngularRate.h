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
 * File AngularRate.h
 */

#ifndef AngularRate_CLASS
#define AngularRate_CLASS

#include <iostream>
#include <string>
#include <vector>
using namespace std;

#ifndef WITHOUT_ACS
#include <asdmIDLTypesC.h>
using asdmIDLTypes::IDLAngularRate;
#endif

#include <StringTokenizer.h>
#include <NumberFormatException.h>
using asdm::StringTokenizer;
using asdm::NumberFormatException;

#include "EndianStream.h"
using asdm::EndianOSStream;
using asdm::EndianISStream;

namespace asdm {

class AngularRate;
AngularRate operator * ( double , const AngularRate & );
ostream & operator << ( ostream &, const AngularRate & );
istream & operator >> ( istream &, AngularRate&);

/**
 * The AngularRate class implements a quantity of AngularRate in radians per second.
 * 
 * @version 1.00 Jan. 7, 2005
 * @author Allen Farris
 * 
 * @version 1.1 Aug 8, 2006
 * @author Michel Caillat 
 * added toBin/fromBin methods.
 */
class AngularRate {
	friend AngularRate operator * ( double , const AngularRate & );
    friend ostream & operator << ( ostream &, const AngularRate & );
	friend istream & operator >> ( istream &, AngularRate&);

public:
	AngularRate();                              		// default constructor
	AngularRate(const AngularRate &);						// X const X& constructor
	AngularRate(const string &s);
#ifndef WITHOUT_ACS
	AngularRate(const IDLAngularRate &);
#endif
	AngularRate(double value);
	virtual ~AngularRate();							// destructor
	static double fromString(const string&);
	static string toString(double);
	static AngularRate getAngularRate(StringTokenizer &t) throw(NumberFormatException);

	/**
	 * Write the binary representation of this to a EndianOSStream.
	 */		
	void toBin(EndianOSStream& eoss);

	/**
	 * Write the binary representation of a vector of AngularRate to a EndianOSStream.
	 * @param angularRate the vector of AngularRate to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<AngularRate>& angularRate,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of AngularRate to a EndianOSStream.
	 * @param angularRate the vector of vector of AngularRate to be written
	 * @param eoss the EndianOSStream to be written to
	 */	
	static void toBin(const vector<vector<AngularRate> >& angularRate,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of vector of AngularRate to a EndianOSStream.
	 * @param angularRate the vector of vector of vector of AngularRate to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<vector<vector<AngularRate> > >& angularRate,  EndianOSStream& eoss);

	/**
	 * Read the binary representation of an AngularRate from a EndianISStream
	 * and use the read value to set an  AngularRate.
	 * @param eiss the EndianStream to be read
	 * @return an AngularRate
	 */
	static AngularRate fromBin(EndianISStream& eiss);
	
	/**
	 * Read the binary representation of  a vector of  AngularRate from an EndianISStream
	 * and use the read value to set a vector of  AngularRate.
	 * @param dis the EndianISStream to be read
	 * @return a vector of AngularRate
	 */	 
	 static vector<AngularRate> from1DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of AngularRate from an EndianISStream
	 * and use the read value to set a vector of  vector of AngularRate.
	 * @param eiis the EndianISStream to be read
	 * @return a vector of vector of AngularRate
	 */	 
	 static vector<vector<AngularRate> > from2DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of vector of AngularRate from an EndianISStream
	 * and use the read value to set a vector of  vector of vector of AngularRate.
	 * @param eiss the EndianISStream to be read
	 * @return a vector of vector of vector of AngularRate
	 */	 
	 static vector<vector<vector<AngularRate> > > from3DBin(EndianISStream & eiss);	

	AngularRate& operator = (const AngularRate&);			// assignment operator
	AngularRate& operator = (const double);			// assignment operator

	AngularRate& operator += (const AngularRate&);		// assignment with arithmetic
	AngularRate& operator -= (const AngularRate&);		//	operators
	AngularRate& operator *= (const double);
	AngularRate& operator /= (const double);

	AngularRate operator + (const AngularRate&) const;	// arithmetic operators
	AngularRate operator - (const AngularRate&) const;
	AngularRate operator * (const double) const;
	AngularRate operator / (const double) const;

	bool operator < (const AngularRate&) const;		// comparison operators
	bool operator > (const AngularRate&) const;
	bool operator <= (const AngularRate&) const;
	bool operator >= (const AngularRate&) const;
	bool operator == (const AngularRate&) const;
	bool equals(const AngularRate&) const;
	bool operator != (const AngularRate&) const;

	bool isZero() const;

	AngularRate operator - () const;					// unary minus
	AngularRate operator + () const; 				// unary plus

	string toString() const;
	string toStringI() const;

	operator string () const;
	double get() const;
#ifndef WITHOUT_ACS
	IDLAngularRate toIDLAngularRate() const;
#endif
	static string unit();

private:
	double value;

};

// AngularRate constructors
inline AngularRate::AngularRate() : value(0.0) {
}

inline AngularRate::AngularRate(const AngularRate &t) : value(t.value) {
}

#ifndef WITHOUT_ACS
inline AngularRate::AngularRate(const IDLAngularRate &l) : value(l.value) {
}
#endif

inline AngularRate::AngularRate(const string &s) : value(fromString(s)) {
}

inline AngularRate::AngularRate(double v) : value(v) {
}

// AngularRate destructor
inline AngularRate::~AngularRate() { }

// assignment operator
inline AngularRate& AngularRate::operator = ( const AngularRate &t ) {
	value = t.value;
	return *this;
}

// assignment operator
inline AngularRate& AngularRate::operator = ( const double v ) {
	value = v;
	return *this;
}

// assignment with arithmetic operators
inline AngularRate& AngularRate::operator += ( const AngularRate& t) {
	value += t.value;
	return *this;
}

inline AngularRate& AngularRate::operator -= ( const AngularRate& t) {
	value -= t.value;
	return *this;
}

inline AngularRate& AngularRate::operator *= ( const double n) {
	value *= n;
	return *this;
}

inline AngularRate& AngularRate::operator /= ( const double n) {
	value /= n;
	return *this;
}

// arithmetic functions
inline AngularRate AngularRate::operator + ( const AngularRate &t2 ) const {
	AngularRate tmp;
	tmp.value = value + t2.value;
	return tmp;
}

inline AngularRate AngularRate::operator - ( const AngularRate &t2 ) const {
	AngularRate tmp;
	tmp.value = value - t2.value;
	return tmp;
}
inline AngularRate AngularRate::operator * ( const double n) const {
	AngularRate tmp;
	tmp.value = value * n;
	return tmp;
}

inline AngularRate AngularRate::operator / ( const double n) const {
	AngularRate tmp;
	tmp.value = value / n;
	return tmp;
}

// comparison operators
inline bool AngularRate::operator < (const AngularRate& x) const {
	return (value < x.value);
}

inline bool AngularRate::operator > (const AngularRate& x) const {
	return (value > x.value);
}

inline bool AngularRate::operator <= (const AngularRate& x) const {
	return (value <= x.value);
}

inline bool AngularRate::operator >= (const AngularRate& x) const {
	return (value >= x.value);
}

inline bool AngularRate::operator == (const AngularRate& x) const {
	return (value == x.value);
}
inline bool AngularRate::equals(const AngularRate& x) const {
	return (value == x.value);
}

inline bool AngularRate::operator != (const AngularRate& x) const {
	return (value != x.value);
}

// unary - and + operators
inline AngularRate AngularRate::operator - () const {
	AngularRate tmp;
        tmp.value = -value;
	return tmp;
}

inline AngularRate AngularRate::operator + () const {
	AngularRate tmp;
    tmp.value = value;
	return tmp;
}

// Conversion functions
inline AngularRate::operator string () const {
	return toString();
}

inline string AngularRate::toString() const {
	return toString(value);
}

inline string AngularRate::toStringI() const {
	return toString(value);
}

inline double AngularRate::get() const {
	return value;
}

#ifndef WITHOUT_ACS
inline IDLAngularRate AngularRate::toIDLAngularRate() const {
	IDLAngularRate tmp;
	tmp.value = value;
	return tmp;
}
#endif

// Friend functions

inline AngularRate operator * ( double n, const AngularRate &x) {
	AngularRate tmp;
	tmp.value = x.value * n;
	return tmp;
}

inline ostream & operator << ( ostream &o, const AngularRate &x ) {
	o << x.value;
	return o;
}

inline istream & operator >> ( istream &i, AngularRate &x ) {
	i >> x.value;
	return i;
}

inline string AngularRate::unit() {
	return string ("rad/sec");
}

} // End namespace asdm

#endif /* AngularRate_CLASS */
