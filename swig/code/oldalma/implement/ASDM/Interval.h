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
 * File Interval.h
 */

#ifndef Interval_CLASS
#define Interval_CLASS

#include <vector>
#include <iostream>
#include <string>
using namespace std;

#ifndef WITHOUT_ACS
#include <asdmIDLTypesC.h>
using asdmIDLTypes::IDLInterval;
#endif

#include <StringTokenizer.h>
#include <NumberFormatException.h>
using asdm::StringTokenizer;
using asdm::NumberFormatException;

#include <EndianStream.h>
using asdm::EndianOSStream;
using asdm::EndianISStream;

namespace asdm {

class Interval;
Interval operator * ( long long , const Interval & );
ostream & operator << ( ostream &, const Interval & );
istream & operator >> ( istream &, Interval&);

/**
 * The Interval class implements an interval of time in units
 * of nanoseconds.
 * 
 * @version 1.00 Jan. 7, 2005
 * @author Allen Farris
 */
class Interval {
	friend Interval operator * ( long long , const Interval & );
    friend ostream & operator << ( ostream &, const Interval & );
	friend istream & operator >> ( istream &, Interval&);

public:
	static long long fromString(const string&);
	static string toString(long long);
	static Interval getInterval(StringTokenizer &t) throw(NumberFormatException);

	Interval();                              		// default constructor
	Interval(const Interval &);						// X const X& constructor
	Interval(const string &s);
#ifndef WITHOUT_ACS
	Interval(const IDLInterval &);
#endif
	Interval(long long value);
	virtual ~Interval();							// destructor

	Interval& operator = (const Interval&);			// assignment operator
	Interval& operator = (const long long);			// assignment operator

	Interval& operator += (const Interval&);		// assignment with arithmetic
	Interval& operator -= (const Interval&);		//	operators
	Interval& operator *= (const long long);
	Interval& operator /= (const long long);

	Interval operator + (const Interval&) const;	// arithmetic operators
	Interval operator - (const Interval&) const;
	Interval operator * (const long long) const;
	Interval operator / (const long long) const;

	bool operator < (const Interval&) const;		// comparison operators
	bool operator > (const Interval&) const;
	bool operator <= (const Interval&) const;
	bool operator >= (const Interval&) const;
	bool operator == (const Interval&) const;
	bool equals(const Interval&) const;
	bool operator != (const Interval&) const;

	bool isZero() const;

	Interval operator - () const;					// unary minus
	Interval operator + () const; 				// unary plus

	string toString() const;
	string toStringI() const;
	
	/**
	 * Write the binary representation of this into an EndianOSStream
	 * @param eoss
	 *
	 */
	void  toBin(EndianOSStream& eoss) ;
	
	/**
	 * Write the binary representation of a vector of Interval into a EndianOSStream.
	 * @param interval
	 * @param eoss
	 */
	static void toBin(vector<Interval> interval, EndianOSStream& eoss) ;
	
	/**
	 * Write the binary representation of a vector of vector  of Interval into a EndianOSStream.
	 * @param interval
	 * @param eoss
	 */
	static void toBin(vector<vector<Interval> > interval, EndianOSStream& eoss) ;
	
	/**
	 * Read the binary representation of an Interval from an EndianISStream
	 * and use the read value to set an  Interval.
	 * @param eiss the EndianISStream to be read
	 * @return an Interval
	 */
	static Interval fromBin(EndianISStream& eiss);
	
	/**
	 * Read the binary representation of a vector of Interval from a EndianISStream
	 * and use the read value to set a vector of Interval.
	 * @param eiss the EndianISStream to be read
	 * @return a vector of Interval
	 */
	static vector<Interval> from1DBin(EndianISStream& eiss);
	
	/**
	 * Read the binary representation of a vector of vector of Interval from a EndianISStream
	 * and use the read value to set a vector of vector of Interval.
	 * @param eiss the EndianISStream  to be read
	 * @return a vector of vector of Interval
	 */
	static vector<vector<Interval> >from2DBin(EndianISStream& eiss);	
	
	operator string () const;
	long long get() const;
#ifndef WITHOUT_ACS
	IDLInterval toIDLInterval() const;
#endif
	static string unit();

private:
	long long value;

};

// Interval constructors
inline Interval::Interval() : value(0L) {
}

inline Interval::Interval(const Interval &t) : value(t.value) {
}

#ifndef WITHOUT_ACS
inline Interval::Interval(const IDLInterval &l) : value(l.value) {
}
#endif

inline Interval::Interval(const string &s) : value(fromString(s)) {
}

inline Interval::Interval(long long v) : value(v) {
}

// Interval destructor
inline Interval::~Interval() { }

// assignment operator
inline Interval& Interval::operator = ( const Interval &t ) {
	value = t.value;
	return *this;
}

// assignment operator
inline Interval& Interval::operator = ( const long long v ) {
	value = v;
	return *this;
}

// assignment with arithmetic operators
inline Interval& Interval::operator += ( const Interval& t) {
	value += t.value;
	return *this;
}

inline Interval& Interval::operator -= ( const Interval& t) {
	value -= t.value;
	return *this;
}

inline Interval& Interval::operator *= ( const long long n) {
	value *= n;
	return *this;
}

inline Interval& Interval::operator /= ( const long long n) {
	value /= n;
	return *this;
}

// arithmetic functions
inline Interval Interval::operator + ( const Interval &t2 ) const {
	Interval tmp;
	tmp.value = value + t2.value;
	return tmp;
}

inline Interval Interval::operator - ( const Interval &t2 ) const {
	Interval tmp;
	tmp.value = value - t2.value;
	return tmp;
}
inline Interval Interval::operator * ( const long long n) const {
	Interval tmp;
	tmp.value = value * n;
	return tmp;
}

inline Interval Interval::operator / ( const long long n) const {
	Interval tmp;
	tmp.value = value / n;
	return tmp;
}

// comparison operators
inline bool Interval::operator < (const Interval& x) const {
	return (value < x.value);
}

inline bool Interval::operator > (const Interval& x) const {
	return (value > x.value);
}

inline bool Interval::operator <= (const Interval& x) const {
	return (value <= x.value);
}

inline bool Interval::operator >= (const Interval& x) const {
	return (value >= x.value);
}

inline bool Interval::operator == (const Interval& x) const {
	return (value == x.value);
}
inline bool Interval::equals(const Interval& x) const {
	return (value == x.value);
}

inline bool Interval::operator != (const Interval& x) const {
	return (value != x.value);
}

// unary - and + operators
inline Interval Interval::operator - () const {
	Interval tmp;
        tmp.value = -value;
	return tmp;
}

inline Interval Interval::operator + () const {
	Interval tmp;
    tmp.value = value;
	return tmp;
}

// Conversion functions
inline Interval::operator string () const {
	return toString();
}

inline string Interval::toString() const {
	return toString(value);
}

inline string Interval::toStringI() const {
	return toString(value);
}

inline long long Interval::get() const {
	return value;
}

#ifndef WITHOUT_ACS
inline IDLInterval Interval::toIDLInterval() const {
	IDLInterval tmp;
	tmp.value = value;
	return tmp;
}
#endif

// Friend functions

inline Interval operator * ( long long n, const Interval &x) {
	Interval tmp;
	tmp.value = x.value * n;
	return tmp;
}

inline ostream & operator << ( ostream &o, const Interval &x ) {
	o << x.value;
	return o;
}

inline istream & operator >> ( istream &i, Interval &x ) {
	i >> x.value;
	return i;
}

inline string Interval::unit() {
	return string ("nanosec");
}

} // End namespace asdm

#endif /* Interval_CLASS */
