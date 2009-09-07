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
 * File Temperature.h
 */

#ifndef Temperature_CLASS
#define Temperature_CLASS

#include <vector>
#include <iostream>
#include <string>
using namespace std;

#ifndef WITHOUT_ACS
#include <asdmIDLTypesC.h>
using asdmIDLTypes::IDLTemperature;
#endif

#include <StringTokenizer.h>
#include <NumberFormatException.h>
using asdm::StringTokenizer;
using asdm::NumberFormatException;

#include "EndianStream.h"
using asdm::EndianOSStream;
using asdm::EndianISStream;

namespace asdm {

class Temperature;
Temperature operator * ( double , const Temperature & );
ostream & operator << ( ostream &, const Temperature & );
istream & operator >> ( istream &, Temperature&);

/**
 * The Temperature class implements a quantity of temperature in degrees Centigrade.
 * 
 * @version 1.00 Jan. 7, 2005
 * @author Allen Farris
 */
class Temperature {
	friend Temperature operator * ( double , const Temperature & );
    friend ostream & operator << ( ostream &, const Temperature & );
	friend istream & operator >> ( istream &, Temperature&);

public:
	static double fromString(const string&);
	static string toString(double);
	static Temperature getTemperature(StringTokenizer &t) throw(NumberFormatException);

	/**
	 * Write the binary representation of this to a EndianOSStream.
	 */		
	void toBin(EndianOSStream& eoss);

	/**
	 * Write the binary representation of a vector of Temperature to a EndianOSStream.
	 * @param temp the vector of Temperature to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<Temperature>& temp,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of Temperature to a EndianOSStream.
	 * @param temp the vector of vector of Temperature to be written
	 * @param eoss the EndianOSStream to be written to
	 */	
	static void toBin(const vector<vector<Temperature> >& temp,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of vector of Temperature to a EndianOSStream.
	 * @param temp the vector of vector of vector of Temperature to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<vector<vector<Temperature> > >& temp,  EndianOSStream& eoss);

	/**
	 * Read the binary representation of an Temperature from a EndianISStream
	 * and use the read value to set an  Temperature.
	 * @param eiss the EndianStream to be read
	 * @return an Temperature
	 */
	static Temperature fromBin(EndianISStream& eiss);
	
	/**
	 * Read the binary representation of  a vector of  Temperature from an EndianISStream
	 * and use the read value to set a vector of  Temperature.
	 * @param dis the EndianISStream to be read
	 * @return a vector of Temperature
	 */	 
	 static vector<Temperature> from1DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of Temperature from an EndianISStream
	 * and use the read value to set a vector of  vector of Temperature.
	 * @param eiis the EndianISStream to be read
	 * @return a vector of vector of Temperature
	 */	 
	 static vector<vector<Temperature> > from2DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of vector of Temperature from an EndianISStream
	 * and use the read value to set a vector of  vector of vector of Temperature.
	 * @param eiss the EndianISStream to be read
	 * @return a vector of vector of vector of Temperature
	 */	 
	 static vector<vector<vector<Temperature> > > from3DBin(EndianISStream & eiss);	 
	 		
	Temperature();                              		// default constructor
	Temperature(const Temperature &);						// X const X& constructor
	Temperature(const string &s);
#ifndef WITHOUT_ACS
	Temperature(const IDLTemperature &);
#endif
	Temperature(double value);
	virtual ~Temperature();							// destructor

	Temperature& operator = (const Temperature&);			// assignment operator
	Temperature& operator = (const double);			// assignment operator

	Temperature& operator += (const Temperature&);		// assignment with arithmetic
	Temperature& operator -= (const Temperature&);		//	operators
	Temperature& operator *= (const double);
	Temperature& operator /= (const double);

	Temperature operator + (const Temperature&) const;	// arithmetic operators
	Temperature operator - (const Temperature&) const;
	Temperature operator * (const double) const;
	Temperature operator / (const double) const;

	bool operator < (const Temperature&) const;		// comparison operators
	bool operator > (const Temperature&) const;
	bool operator <= (const Temperature&) const;
	bool operator >= (const Temperature&) const;
	bool operator == (const Temperature&) const;
	bool equals(const Temperature&) const;
	bool operator != (const Temperature&) const;

	bool isZero() const;

	Temperature operator - () const;					// unary minus
	Temperature operator + () const; 				// unary plus

	string toString() const;
	string toStringI() const;

	operator string () const;
	double get() const;
#ifndef WITHOUT_ACS
	IDLTemperature toIDLTemperature() const;
#endif
	static string unit();

private:
	double value;

};

// Temperature constructors
inline Temperature::Temperature() : value(0.0) {
}

inline Temperature::Temperature(const Temperature &t) : value(t.value) {
}

#ifndef WITHOUT_ACS
inline Temperature::Temperature(const IDLTemperature &l) : value(l.value) {
}
#endif

inline Temperature::Temperature(const string &s) : value(fromString(s)) {
}

inline Temperature::Temperature(double v) : value(v) {
}

// Temperature destructor
inline Temperature::~Temperature() { }

// assignment operator
inline Temperature& Temperature::operator = ( const Temperature &t ) {
	value = t.value;
	return *this;
}

// assignment operator
inline Temperature& Temperature::operator = ( const double v ) {
	value = v;
	return *this;
}

// assignment with arithmetic operators
inline Temperature& Temperature::operator += ( const Temperature& t) {
	value += t.value;
	return *this;
}

inline Temperature& Temperature::operator -= ( const Temperature& t) {
	value -= t.value;
	return *this;
}

inline Temperature& Temperature::operator *= ( const double n) {
	value *= n;
	return *this;
}

inline Temperature& Temperature::operator /= ( const double n) {
	value /= n;
	return *this;
}

// arithmetic functions
inline Temperature Temperature::operator + ( const Temperature &t2 ) const {
	Temperature tmp;
	tmp.value = value + t2.value;
	return tmp;
}

inline Temperature Temperature::operator - ( const Temperature &t2 ) const {
	Temperature tmp;
	tmp.value = value - t2.value;
	return tmp;
}
inline Temperature Temperature::operator * ( const double n) const {
	Temperature tmp;
	tmp.value = value * n;
	return tmp;
}

inline Temperature Temperature::operator / ( const double n) const {
	Temperature tmp;
	tmp.value = value / n;
	return tmp;
}

// comparison operators
inline bool Temperature::operator < (const Temperature& x) const {
	return (value < x.value);
}

inline bool Temperature::operator > (const Temperature& x) const {
	return (value > x.value);
}

inline bool Temperature::operator <= (const Temperature& x) const {
	return (value <= x.value);
}

inline bool Temperature::operator >= (const Temperature& x) const {
	return (value >= x.value);
}

inline bool Temperature::operator == (const Temperature& x) const {
	return (value == x.value);
}
inline bool Temperature::equals(const Temperature& x) const {
	return (value == x.value);
}

inline bool Temperature::operator != (const Temperature& x) const {
	return (value != x.value);
}

// unary - and + operators
inline Temperature Temperature::operator - () const {
	Temperature tmp;
        tmp.value = -value;
	return tmp;
}

inline Temperature Temperature::operator + () const {
	Temperature tmp;
    tmp.value = value;
	return tmp;
}

// Conversion functions
inline Temperature::operator string () const {
	return toString();
}

inline string Temperature::toString() const {
	return toString(value);
}

inline string Temperature::toStringI() const {
	return toString(value);
}

inline double Temperature::get() const {
	return value;
}

#ifndef WITHOUT_ACS
inline IDLTemperature Temperature::toIDLTemperature() const {
	IDLTemperature tmp;
	tmp.value = value;
	return tmp;
}
#endif

// Friend functions

inline Temperature operator * ( double n, const Temperature &x) {
	Temperature tmp;
	tmp.value = x.value * n;
	return tmp;
}

inline ostream & operator << ( ostream &o, const Temperature &x ) {
	o << x.value;
	return o;
}

inline istream & operator >> ( istream &i, Temperature &x ) {
	i >> x.value;
	return i;
}

inline string Temperature::unit() {
	return string ("degC");
}

} // End namespace asdm

#endif /* Temperature_CLASS */
