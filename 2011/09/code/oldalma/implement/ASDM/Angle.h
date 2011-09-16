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
 * File Angle.h
 */

#ifndef Angle_CLASS
#define Angle_CLASS

#include <vector>
#include <iostream>
#include <string>
using namespace std;

#ifndef WITHOUT_ACS
#include <asdmIDLTypesC.h>
using asdmIDLTypes::IDLAngle;
#endif

#include <StringTokenizer.h>
#include <NumberFormatException.h>
using asdm::StringTokenizer;
using asdm::NumberFormatException;

#include "EndianStream.h"
using asdm::EndianOSStream;
using asdm::EndianISStream;

/**
 * The Angle class implements a quantity of angle in radians.
 * 
 * @version 1.00 Jan. 7, 2005
 * @author Allen Farris
 */
namespace asdm {

class Angle;
Angle operator * ( double , const Angle & );
ostream & operator << ( ostream &, const Angle & );
istream & operator >> ( istream &, Angle&);

/**
 * The Angle class implements a quantity of angle in radians.
 * 
 * @version 1.00 Jan. 7, 2005
 * @author Allen Farris
 * 
 * @version 1.1 Aug 8, 2006
 * @author Michel Caillat 
 * added toBin/fromBin methods.
 */
class Angle {
	friend Angle operator * ( double , const Angle & );
    friend ostream & operator << ( ostream &, const Angle & );
	friend istream & operator >> ( istream &, Angle&);

public:
	Angle();                              		// default constructor
	Angle(const Angle &);						// X const X& constructor
	Angle(const string &s);
#ifndef WITHOUT_ACS
	Angle(const IDLAngle &);
#endif
	Angle(double value);
	virtual ~Angle();							// destructor
	static double fromString(const string&);
	static string toString(double);
	static Angle getAngle(StringTokenizer &t) throw(NumberFormatException);
			
	/**
	 * Write the binary representation of this to an EndianOSStream.
	 */		
	void toBin(EndianOSStream& eoss);

	/**
	 * Write the binary representation of a vector of Angle to a EndianOSStream.
	 * @param angle the vector of Angle to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<Angle>& angle,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of Angle to a EndianOSStream.
	 * @param angle the vector of vector of Angle to be written
	 * @param eoss the EndianOSStream to be written to
	 */	
	static void toBin(const vector<vector<Angle> >& angle,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of vector of Angle to a EndianOSStream.
	 * @param angle the vector of vector of vector of Angle to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<vector<vector<Angle> > >& angle,  EndianOSStream& eoss);

	/**
	 * Read the binary representation of an Angle from a EndianISStream
	 * and use the read value to set an  Angle.
	 * @param eiss the EndianStream to be read
	 * @return an Angle
	 */
	static Angle fromBin(EndianISStream& eiss);
	
	/**
	 * Read the binary representation of  a vector of  Angle from an EndianISStream
	 * and use the read value to set a vector of  Angle.
	 * @param eiis the EndianISStream to be read
	 * @return a vector of Angle
	 */	 
	 static vector<Angle> from1DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of Angle from an EndianISStream
	 * and use the read value to set a vector of  vector of Angle.
	 * @param eiis the EndianISStream to be read
	 * @return a vector of vector of Angle
	 */	 
	 static vector<vector<Angle> > from2DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of vector of Angle from an EndianISStream
	 * and use the read value to set a vector of  vector of vector of Angle.
	 * @param eiss the EndianISStream to be read
	 * @return a vector of vector of vector of Angle
	 */	 
	 static vector<vector<vector<Angle> > > from3DBin(EndianISStream & eiss);	 
	 	
	Angle& operator = (const Angle&);			// assignment operator
	Angle& operator = (const double);			// assignment operator

	Angle& operator += (const Angle&);		// assignment with arithmetic
	Angle& operator -= (const Angle&);		//	operators
	Angle& operator *= (const double);
	Angle& operator /= (const double);

	Angle operator + (const Angle&) const;	// arithmetic operators
	Angle operator - (const Angle&) const;
	Angle operator * (const double) const;
	Angle operator / (const double) const;

	bool operator < (const Angle&) const;		// comparison operators
	bool operator > (const Angle&) const;
	bool operator <= (const Angle&) const;
	bool operator >= (const Angle&) const;
	bool operator == (const Angle&) const;
	bool equals(const Angle&) const;
	bool operator != (const Angle&) const;

	bool isZero() const;

	Angle operator - () const;					// unary minus
	Angle operator + () const; 				// unary plus

	string toString() const;
	string toStringI() const;

	operator string () const;
	double get() const;
#ifndef WITHOUT_ACS
	IDLAngle toIDLAngle() const;
#endif
	static string unit();

private:
	double value;

};

// Angle constructors
inline Angle::Angle() : value(0.0) {
}

inline Angle::Angle(const Angle &t) : value(t.value) {
}

#ifndef WITHOUT_ACS
inline Angle::Angle(const IDLAngle &l) : value(l.value) {
}
#endif

inline Angle::Angle(const string &s) : value(fromString(s)) {
}

inline Angle::Angle(double v) : value(v) {
}

// Angle destructor
inline Angle::~Angle() { }

// assignment operator
inline Angle& Angle::operator = ( const Angle &t ) {
	value = t.value;
	return *this;
}

// assignment operator
inline Angle& Angle::operator = ( const double v ) {
	value = v;
	return *this;
}

// assignment with arithmetic operators
inline Angle& Angle::operator += ( const Angle& t) {
	value += t.value;
	return *this;
}

inline Angle& Angle::operator -= ( const Angle& t) {
	value -= t.value;
	return *this;
}

inline Angle& Angle::operator *= ( const double n) {
	value *= n;
	return *this;
}

inline Angle& Angle::operator /= ( const double n) {
	value /= n;
	return *this;
}

// arithmetic functions
inline Angle Angle::operator + ( const Angle &t2 ) const {
	Angle tmp;
	tmp.value = value + t2.value;
	return tmp;
}

inline Angle Angle::operator - ( const Angle &t2 ) const {
	Angle tmp;
	tmp.value = value - t2.value;
	return tmp;
}
inline Angle Angle::operator * ( const double n) const {
	Angle tmp;
	tmp.value = value * n;
	return tmp;
}

inline Angle Angle::operator / ( const double n) const {
	Angle tmp;
	tmp.value = value / n;
	return tmp;
}

// comparison operators
inline bool Angle::operator < (const Angle& x) const {
	return (value < x.value);
}

inline bool Angle::operator > (const Angle& x) const {
	return (value > x.value);
}

inline bool Angle::operator <= (const Angle& x) const {
	return (value <= x.value);
}

inline bool Angle::operator >= (const Angle& x) const {
	return (value >= x.value);
}

inline bool Angle::equals(const Angle& x) const {
	return (value == x.value);
}


inline bool Angle::operator == (const Angle& x) const {
	return (value == x.value);
}

inline bool Angle::operator != (const Angle& x) const {
	return (value != x.value);
}

// unary - and + operators
inline Angle Angle::operator - () const {
	Angle tmp;
        tmp.value = -value;
	return tmp;
}

inline Angle Angle::operator + () const {
	Angle tmp;
    tmp.value = value;
	return tmp;
}

// Conversion functions
inline Angle::operator string () const {
	return toString();
}

inline string Angle::toString() const {
	return toString(value);
}

inline string Angle::toStringI() const {
	return toString(value);
}

inline double Angle::get() const {
	return value;
}

#ifndef WITHOUT_ACS
inline IDLAngle Angle::toIDLAngle() const {
	IDLAngle tmp;
	tmp.value = value;
	return tmp;
}
#endif

// Friend functions

inline Angle operator * ( double n, const Angle &x) {
	Angle tmp;
	tmp.value = x.value * n;
	return tmp;
}

inline ostream & operator << ( ostream &o, const Angle &x ) {
	o << x.value;
	return o;
}

inline istream & operator >> ( istream &i, Angle &x ) {
	i >> x.value;
	return i;
}

inline string Angle::unit() {
	return string ("rad");
}

} // End namespace asdm

#endif /* Angle_CLASS */
