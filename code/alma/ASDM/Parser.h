
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
 * Warning!
 *  -------------------------------------------------------------------- 
 * | This is generated code!  Do not modify this file.                  |
 * | If you do, all changes will be lost when the file is re-generated. |
 *  --------------------------------------------------------------------
 *
 * File Parser.h
 */
#ifndef Parser_CLASS
#define Parser_CLASS

#include <stdint.h>
#include <string>
#include <vector>
#include <set>

using std::string;
using std::vector;
using std::set;

#include <Angle.h>
#include <AngularRate.h>
#include <ArrayTime.h>
#include <ArrayTimeInterval.h>
#include <ComplexWrapper.h>
#include <Entity.h>
#include <EntityId.h>
#include <EntityRef.h>
#include <Flux.h>
#include <Frequency.h>
#include <Humidity.h>
#include <Interval.h>
#include <Length.h>
#include <Pressure.h>
#include <Speed.h>
#include <Tag.h>
#include <Temperature.h>
 
#include <StringTokenizer.h>
#include <OutOfBoundsException.h>
#include <LongWrapper.h>
#include <ConversionException.h>

using namespace std;
using asdm::Angle;
using asdm::AngularRate;
using asdm::ArrayTime;
using asdm::Complex;
using asdm::Entity;
using asdm::EntityId;
using asdm::EntityRef;
using asdm::Flux;
using asdm::Frequency;
using asdm::Humidity;
using asdm::Interval;
using asdm::Length;
using asdm::Pressure;
using asdm::Speed;
using asdm::Tag;
using asdm::Temperature;

using asdm::StringTokenizer;
using asdm::OutOfBoundsException;
using asdm::Long;
using asdm::ConversionException;

namespace asdm {
/**
 * A Parser of for XML representation of ASDM datasets.
 */
class Parser {

public:

	Parser(const string &s);

	/**
	 * Is s in the string being parsed?
	 */
	bool isStr(const string &) const;

	/**
	 * Get the portion of the string bounded by s1 and s2, inclusive.
	 * @param s1
	 * @param s2
	 * @return
	 */
	string getElement(const string &s1, const string &s2);

	/**
	 * Get the portion of the string bounded by s1 and s2, exclusive.
	 * @param s1
	 * @param s2
	 * @return
	 */
	string getElementContent(const string &s1, const string &s2);

	string getField(const string &field);

	static string getField(const string &xml, const string &field);

	// The follwing is a special case.
	static string getString(const string &name, const string &tableName, const string &xmlDoc) ;
	//  throw (ConversionException);
	static vector<string> get1DString(const string &name, const string &tableName, const string &xmlDoc) ;
	//	throw (ConversionException);
	static vector <vector<string> > get2DString(const string &name, const string &tableName, const string &xmlDoc) ;
	//	throw (ConversionException);
	static vector <vector <vector<string> > > get3DString(const string &name, const string &tableName, const string &xmlDoc);
	//	throw (ConversionException);

	// Generated methods for conversion to and from XML
	// data representations for all types, both primitive
	// and extended.  Also included are 1, 2, and 3 
	// dimensional arrays of these types.

	
	// Field type: int

	static void toXML(int data, const string &name, string &buf);

	static void toXML(vector<int> data, const string &name, string &buf);

	static void toXML(vector< vector<int> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<int> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<int> > > >data, const string &name, string &buf);
	
	
	
	static void toXML(set < int > data, const string &name, string &buf);
	

		
	
	
	
	static set< int >  getIntSet(const string &name, const string &tableName, const string &xmlDoc);
	

			
	static int getInteger(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<int> get1DInteger(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<int> > get2DInteger(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<int> > > get3DInteger(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<int> > > >get4DInteger(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: short

	static void toXML(short data, const string &name, string &buf);

	static void toXML(vector<short> data, const string &name, string &buf);

	static void toXML(vector< vector<short> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<short> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<short> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static short getShort(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<short> get1DShort(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<short> > get2DShort(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<short> > > get3DShort(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<short> > > >get4DShort(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: int64_t

	static void toXML(int64_t data, const string &name, string &buf);

	static void toXML(vector<int64_t> data, const string &name, string &buf);

	static void toXML(vector< vector<int64_t> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<int64_t> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<int64_t> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static int64_t getLong(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<int64_t> get1DLong(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<int64_t> > get2DLong(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<int64_t> > > get3DLong(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<int64_t> > > >get4DLong(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: char

	static void toXML(char data, const string &name, string &buf);

	static void toXML(vector<char> data, const string &name, string &buf);

	static void toXML(vector< vector<char> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<char> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<char> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static char getByte(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<char> get1DByte(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<char> > get2DByte(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<char> > > get3DByte(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<char> > > >get4DByte(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: float

	static void toXML(float data, const string &name, string &buf);

	static void toXML(vector<float> data, const string &name, string &buf);

	static void toXML(vector< vector<float> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<float> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<float> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static float getFloat(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<float> get1DFloat(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<float> > get2DFloat(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<float> > > get3DFloat(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<float> > > >get4DFloat(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: double

	static void toXML(double data, const string &name, string &buf);

	static void toXML(vector<double> data, const string &name, string &buf);

	static void toXML(vector< vector<double> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<double> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<double> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static double getDouble(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<double> get1DDouble(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<double> > get2DDouble(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<double> > > get3DDouble(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<double> > > >get4DDouble(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: unsigned char

	static void toXML(unsigned char data, const string &name, string &buf);

	static void toXML(vector<unsigned char> data, const string &name, string &buf);

	static void toXML(vector< vector<unsigned char> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<unsigned char> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<unsigned char> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static unsigned char getCharacter(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<unsigned char> get1DCharacter(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<unsigned char> > get2DCharacter(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<unsigned char> > > get3DCharacter(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<unsigned char> > > >get4DCharacter(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: bool

	static void toXML(bool data, const string &name, string &buf);

	static void toXML(vector<bool> data, const string &name, string &buf);

	static void toXML(vector< vector<bool> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<bool> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<bool> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static bool getBoolean(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<bool> get1DBoolean(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<bool> > get2DBoolean(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<bool> > > get3DBoolean(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<bool> > > >get4DBoolean(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: string

	static void toXML(string data, const string &name, string &buf);

	static void toXML(vector<string> data, const string &name, string &buf);

	static void toXML(vector< vector<string> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<string> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<string> > > >data, const string &name, string &buf);
	
	
	

		


	// Field type: Angle

	static void toXML(Angle data, const string &name, string &buf);

	static void toXML(vector<Angle> data, const string &name, string &buf);

	static void toXML(vector< vector<Angle> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<Angle> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<Angle> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static Angle getAngle(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<Angle> get1DAngle(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<Angle> > get2DAngle(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<Angle> > > get3DAngle(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<Angle> > > >get4DAngle(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: AngularRate

	static void toXML(AngularRate data, const string &name, string &buf);

	static void toXML(vector<AngularRate> data, const string &name, string &buf);

	static void toXML(vector< vector<AngularRate> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<AngularRate> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<AngularRate> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static AngularRate getAngularRate(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<AngularRate> get1DAngularRate(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<AngularRate> > get2DAngularRate(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<AngularRate> > > get3DAngularRate(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<AngularRate> > > >get4DAngularRate(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: ArrayTime

	static void toXML(ArrayTime data, const string &name, string &buf);

	static void toXML(vector<ArrayTime> data, const string &name, string &buf);

	static void toXML(vector< vector<ArrayTime> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<ArrayTime> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<ArrayTime> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static ArrayTime getArrayTime(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<ArrayTime> get1DArrayTime(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<ArrayTime> > get2DArrayTime(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<ArrayTime> > > get3DArrayTime(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<ArrayTime> > > >get4DArrayTime(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: ArrayTimeInterval

	static void toXML(ArrayTimeInterval data, const string &name, string &buf);

	static void toXML(vector<ArrayTimeInterval> data, const string &name, string &buf);

	static void toXML(vector< vector<ArrayTimeInterval> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<ArrayTimeInterval> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<ArrayTimeInterval> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static ArrayTimeInterval getArrayTimeInterval(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<ArrayTimeInterval> get1DArrayTimeInterval(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<ArrayTimeInterval> > get2DArrayTimeInterval(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<ArrayTimeInterval> > > get3DArrayTimeInterval(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<ArrayTimeInterval> > > >get4DArrayTimeInterval(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: Complex

	static void toXML(Complex data, const string &name, string &buf);

	static void toXML(vector<Complex> data, const string &name, string &buf);

	static void toXML(vector< vector<Complex> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<Complex> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<Complex> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static Complex getComplex(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<Complex> get1DComplex(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<Complex> > get2DComplex(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<Complex> > > get3DComplex(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<Complex> > > >get4DComplex(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: Entity

	static void toXML(Entity data, const string &name, string &buf);

	static void toXML(vector<Entity> data, const string &name, string &buf);

	static void toXML(vector< vector<Entity> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<Entity> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<Entity> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static Entity getEntity(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<Entity> get1DEntity(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<Entity> > get2DEntity(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<Entity> > > get3DEntity(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<Entity> > > >get4DEntity(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: EntityId

	static void toXML(EntityId data, const string &name, string &buf);

	static void toXML(vector<EntityId> data, const string &name, string &buf);

	static void toXML(vector< vector<EntityId> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<EntityId> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<EntityId> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static EntityId getEntityId(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<EntityId> get1DEntityId(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<EntityId> > get2DEntityId(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<EntityId> > > get3DEntityId(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<EntityId> > > >get4DEntityId(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: EntityRef

	static void toXML(EntityRef data, const string &name, string &buf);

	static void toXML(vector<EntityRef> data, const string &name, string &buf);

	static void toXML(vector< vector<EntityRef> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<EntityRef> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<EntityRef> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static EntityRef getEntityRef(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<EntityRef> get1DEntityRef(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<EntityRef> > get2DEntityRef(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<EntityRef> > > get3DEntityRef(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<EntityRef> > > >get4DEntityRef(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: Flux

	static void toXML(Flux data, const string &name, string &buf);

	static void toXML(vector<Flux> data, const string &name, string &buf);

	static void toXML(vector< vector<Flux> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<Flux> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<Flux> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static Flux getFlux(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<Flux> get1DFlux(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<Flux> > get2DFlux(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<Flux> > > get3DFlux(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<Flux> > > >get4DFlux(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: Frequency

	static void toXML(Frequency data, const string &name, string &buf);

	static void toXML(vector<Frequency> data, const string &name, string &buf);

	static void toXML(vector< vector<Frequency> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<Frequency> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<Frequency> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static Frequency getFrequency(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<Frequency> get1DFrequency(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<Frequency> > get2DFrequency(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<Frequency> > > get3DFrequency(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<Frequency> > > >get4DFrequency(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: Humidity

	static void toXML(Humidity data, const string &name, string &buf);

	static void toXML(vector<Humidity> data, const string &name, string &buf);

	static void toXML(vector< vector<Humidity> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<Humidity> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<Humidity> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static Humidity getHumidity(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<Humidity> get1DHumidity(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<Humidity> > get2DHumidity(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<Humidity> > > get3DHumidity(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<Humidity> > > >get4DHumidity(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: Interval

	static void toXML(Interval data, const string &name, string &buf);

	static void toXML(vector<Interval> data, const string &name, string &buf);

	static void toXML(vector< vector<Interval> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<Interval> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<Interval> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static Interval getInterval(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<Interval> get1DInterval(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<Interval> > get2DInterval(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<Interval> > > get3DInterval(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<Interval> > > >get4DInterval(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: Length

	static void toXML(Length data, const string &name, string &buf);

	static void toXML(vector<Length> data, const string &name, string &buf);

	static void toXML(vector< vector<Length> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<Length> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<Length> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static Length getLength(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<Length> get1DLength(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<Length> > get2DLength(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<Length> > > get3DLength(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<Length> > > >get4DLength(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: Pressure

	static void toXML(Pressure data, const string &name, string &buf);

	static void toXML(vector<Pressure> data, const string &name, string &buf);

	static void toXML(vector< vector<Pressure> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<Pressure> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<Pressure> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static Pressure getPressure(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<Pressure> get1DPressure(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<Pressure> > get2DPressure(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<Pressure> > > get3DPressure(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<Pressure> > > >get4DPressure(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: Speed

	static void toXML(Speed data, const string &name, string &buf);

	static void toXML(vector<Speed> data, const string &name, string &buf);

	static void toXML(vector< vector<Speed> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<Speed> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<Speed> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static Speed getSpeed(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<Speed> get1DSpeed(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<Speed> > get2DSpeed(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<Speed> > > get3DSpeed(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<Speed> > > >get4DSpeed(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: Tag

	static void toXML(Tag data, const string &name, string &buf);

	static void toXML(vector<Tag> data, const string &name, string &buf);

	static void toXML(vector< vector<Tag> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<Tag> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<Tag> > > >data, const string &name, string &buf);
	
	
	
	static void toXML(set < Tag > data, const string &name, string &buf);
	

		
	
	
	
	static set< Tag >  getTagSet(const string &name, const string &tableName, const string &xmlDoc);
	

			
	static Tag getTag(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<Tag> get1DTag(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<Tag> > get2DTag(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<Tag> > > get3DTag(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<Tag> > > >get4DTag(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	// Field type: Temperature

	static void toXML(Temperature data, const string &name, string &buf);

	static void toXML(vector<Temperature> data, const string &name, string &buf);

	static void toXML(vector< vector<Temperature> > data, const string &name, string &buf);
	
	static void toXML(vector< vector< vector<Temperature> > > data, const string &name, string &buf);
	
	static void toXML(vector<vector< vector< vector<Temperature> > > >data, const string &name, string &buf);
	
	
	

		
	
	
	

			
	static Temperature getTemperature(const string &name, const string &tableName, const string &xmlDoc) ;
	// throw (ConversionException);

	static vector<Temperature> get1DTemperature(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);
		
	static vector< vector<Temperature> > get2DTemperature(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector< vector< vector<Temperature> > > get3DTemperature(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	
	
	static vector<vector< vector< vector<Temperature> > > >get4DTemperature(const string &name, const string &tableName, const string &xmlDoc);
	// throw (ConversionException);	

		


	
	// Generated methods for conversion to and from XML
	// data representations with  a Base64 encoded content.
	// The methods are generated only for 1, 2 and 3 dimensional arrays
	// of data whose BasicType have a non null BaseWrapperName.
	// In practice this represents data whose type is one of the basic numeric types
	// or is built upon a basic numeric type. 
	
	
	
	static void toXMLBase64(vector<int> data, const string &name, string &buf);
	static void toXMLBase64(vector< vector<int> > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector<int> > > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector< vector<int> > > >data, const string &name, string &buf);
	
    
	static vector<int>& get1DIntegerFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector<int>& attribute);
	// throw (ConversionException);
	
	 
	static vector <vector<int> >& get2DIntegerFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector<int> >& attribute);
	// throw (ConversionException);
	
 	
	static vector <vector <vector<int> > >& get3DIntegerFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector<int> > >& attribute);
	// throw (ConversionException);
	
	
	static vector <vector <vector <vector<int> > > >& get4DIntegerFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector <vector<int> > > >& attribute);
	// throw (ConversionException);
	

	
	
	static void toXMLBase64(vector<short> data, const string &name, string &buf);
	static void toXMLBase64(vector< vector<short> > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector<short> > > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector< vector<short> > > >data, const string &name, string &buf);
	
    
	static vector<short>& get1DShortFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector<short>& attribute);
	// throw (ConversionException);
	
	 
	static vector <vector<short> >& get2DShortFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector<short> >& attribute);
	// throw (ConversionException);
	
 	
	static vector <vector <vector<short> > >& get3DShortFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector<short> > >& attribute);
	// throw (ConversionException);
	
	
	static vector <vector <vector <vector<short> > > >& get4DShortFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector <vector<short> > > >& attribute);
	// throw (ConversionException);
	

	
	
	static void toXMLBase64(vector<int64_t> data, const string &name, string &buf);
	static void toXMLBase64(vector< vector<int64_t> > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector<int64_t> > > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector< vector<int64_t> > > >data, const string &name, string &buf);
	
    
	static vector<int64_t>& get1DLongFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector<int64_t>& attribute);
	// throw (ConversionException);
	
	 
	static vector <vector<int64_t> >& get2DLongFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector<int64_t> >& attribute);
	// throw (ConversionException);
	
 	
	static vector <vector <vector<int64_t> > >& get3DLongFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector<int64_t> > >& attribute);
	// throw (ConversionException);
	
	
	static vector <vector <vector <vector<int64_t> > > >& get4DLongFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector <vector<int64_t> > > >& attribute);
	// throw (ConversionException);
	

	
	
	static void toXMLBase64(vector<char> data, const string &name, string &buf);
	static void toXMLBase64(vector< vector<char> > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector<char> > > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector< vector<char> > > >data, const string &name, string &buf);
	
    
	static vector<char>& get1DByteFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector<char>& attribute);
	// throw (ConversionException);
	
	 
	static vector <vector<char> >& get2DByteFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector<char> >& attribute);
	// throw (ConversionException);
	
 	
	static vector <vector <vector<char> > >& get3DByteFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector<char> > >& attribute);
	// throw (ConversionException);
	
	
	static vector <vector <vector <vector<char> > > >& get4DByteFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector <vector<char> > > >& attribute);
	// throw (ConversionException);
	

	
	
	static void toXMLBase64(vector<float> data, const string &name, string &buf);
	static void toXMLBase64(vector< vector<float> > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector<float> > > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector< vector<float> > > >data, const string &name, string &buf);
	
    
	static vector<float>& get1DFloatFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector<float>& attribute);
	// throw (ConversionException);
	
	 
	static vector <vector<float> >& get2DFloatFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector<float> >& attribute);
	// throw (ConversionException);
	
 	
	static vector <vector <vector<float> > >& get3DFloatFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector<float> > >& attribute);
	// throw (ConversionException);
	
	
	static vector <vector <vector <vector<float> > > >& get4DFloatFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector <vector<float> > > >& attribute);
	// throw (ConversionException);
	

	
	
	static void toXMLBase64(vector<double> data, const string &name, string &buf);
	static void toXMLBase64(vector< vector<double> > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector<double> > > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector< vector<double> > > >data, const string &name, string &buf);
	
    
	static vector<double>& get1DDoubleFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector<double>& attribute);
	// throw (ConversionException);
	
	 
	static vector <vector<double> >& get2DDoubleFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector<double> >& attribute);
	// throw (ConversionException);
	
 	
	static vector <vector <vector<double> > >& get3DDoubleFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector<double> > >& attribute);
	// throw (ConversionException);
	
	
	static vector <vector <vector <vector<double> > > >& get4DDoubleFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector <vector<double> > > >& attribute);
	// throw (ConversionException);
	

	
	
	static void toXMLBase64(vector<unsigned char> data, const string &name, string &buf);
	static void toXMLBase64(vector< vector<unsigned char> > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector<unsigned char> > > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector< vector<unsigned char> > > >data, const string &name, string &buf);
	
    
	static vector<unsigned char>& get1DCharacterFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector<unsigned char>& attribute);
	// throw (ConversionException);
	
	 
	static vector <vector<unsigned char> >& get2DCharacterFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector<unsigned char> >& attribute);
	// throw (ConversionException);
	
 	
	static vector <vector <vector<unsigned char> > >& get3DCharacterFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector<unsigned char> > >& attribute);
	// throw (ConversionException);
	
	
	static vector <vector <vector <vector<unsigned char> > > >& get4DCharacterFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector <vector<unsigned char> > > >& attribute);
	// throw (ConversionException);
	

	
	
	static void toXMLBase64(vector<bool> data, const string &name, string &buf);
	static void toXMLBase64(vector< vector<bool> > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector<bool> > > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector< vector<bool> > > >data, const string &name, string &buf);
	
    
	static vector<bool>& get1DBooleanFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector<bool>& attribute);
	// throw (ConversionException);
	
	 
	static vector <vector<bool> >& get2DBooleanFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector<bool> >& attribute);
	// throw (ConversionException);
	
 	
	static vector <vector <vector<bool> > >& get3DBooleanFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector<bool> > >& attribute);
	// throw (ConversionException);
	
	
	static vector <vector <vector <vector<bool> > > >& get4DBooleanFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector <vector<bool> > > >& attribute);
	// throw (ConversionException);
	

	
	

	
	
	static void toXMLBase64(vector<Angle> data, const string &name, string &buf);
	static void toXMLBase64(vector< vector<Angle> > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector<Angle> > > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector< vector<Angle> > > >data, const string &name, string &buf);
	
    
	static vector<Angle>& get1DAngleFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector<Angle>& attribute);
	// throw (ConversionException);
	
	 
	static vector <vector<Angle> >& get2DAngleFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector<Angle> >& attribute);
	// throw (ConversionException);
	
 	
	static vector <vector <vector<Angle> > >& get3DAngleFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector<Angle> > >& attribute);
	// throw (ConversionException);
	
	
	static vector <vector <vector <vector<Angle> > > >& get4DAngleFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector <vector<Angle> > > >& attribute);
	// throw (ConversionException);
	

	
	
	static void toXMLBase64(vector<AngularRate> data, const string &name, string &buf);
	static void toXMLBase64(vector< vector<AngularRate> > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector<AngularRate> > > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector< vector<AngularRate> > > >data, const string &name, string &buf);
	
    
	static vector<AngularRate>& get1DAngularRateFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector<AngularRate>& attribute);
	// throw (ConversionException);
	
	 
	static vector <vector<AngularRate> >& get2DAngularRateFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector<AngularRate> >& attribute);
	// throw (ConversionException);
	
 	
	static vector <vector <vector<AngularRate> > >& get3DAngularRateFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector<AngularRate> > >& attribute);
	// throw (ConversionException);
	
	
	static vector <vector <vector <vector<AngularRate> > > >& get4DAngularRateFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector <vector<AngularRate> > > >& attribute);
	// throw (ConversionException);
	

	
	
	static void toXMLBase64(vector<ArrayTime> data, const string &name, string &buf);
	static void toXMLBase64(vector< vector<ArrayTime> > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector<ArrayTime> > > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector< vector<ArrayTime> > > >data, const string &name, string &buf);
	
    
	static vector<ArrayTime>& get1DArrayTimeFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector<ArrayTime>& attribute);
	// throw (ConversionException);
	
	 
	static vector <vector<ArrayTime> >& get2DArrayTimeFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector<ArrayTime> >& attribute);
	// throw (ConversionException);
	
 	
	static vector <vector <vector<ArrayTime> > >& get3DArrayTimeFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector<ArrayTime> > >& attribute);
	// throw (ConversionException);
	
	
	static vector <vector <vector <vector<ArrayTime> > > >& get4DArrayTimeFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector <vector<ArrayTime> > > >& attribute);
	// throw (ConversionException);
	

	
	

	
	

	
	

	
	

	
	

	
	
	static void toXMLBase64(vector<Flux> data, const string &name, string &buf);
	static void toXMLBase64(vector< vector<Flux> > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector<Flux> > > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector< vector<Flux> > > >data, const string &name, string &buf);
	
    
	static vector<Flux>& get1DFluxFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector<Flux>& attribute);
	// throw (ConversionException);
	
	 
	static vector <vector<Flux> >& get2DFluxFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector<Flux> >& attribute);
	// throw (ConversionException);
	
 	
	static vector <vector <vector<Flux> > >& get3DFluxFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector<Flux> > >& attribute);
	// throw (ConversionException);
	
	
	static vector <vector <vector <vector<Flux> > > >& get4DFluxFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector <vector<Flux> > > >& attribute);
	// throw (ConversionException);
	

	
	
	static void toXMLBase64(vector<Frequency> data, const string &name, string &buf);
	static void toXMLBase64(vector< vector<Frequency> > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector<Frequency> > > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector< vector<Frequency> > > >data, const string &name, string &buf);
	
    
	static vector<Frequency>& get1DFrequencyFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector<Frequency>& attribute);
	// throw (ConversionException);
	
	 
	static vector <vector<Frequency> >& get2DFrequencyFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector<Frequency> >& attribute);
	// throw (ConversionException);
	
 	
	static vector <vector <vector<Frequency> > >& get3DFrequencyFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector<Frequency> > >& attribute);
	// throw (ConversionException);
	
	
	static vector <vector <vector <vector<Frequency> > > >& get4DFrequencyFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector <vector<Frequency> > > >& attribute);
	// throw (ConversionException);
	

	
	
	static void toXMLBase64(vector<Humidity> data, const string &name, string &buf);
	static void toXMLBase64(vector< vector<Humidity> > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector<Humidity> > > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector< vector<Humidity> > > >data, const string &name, string &buf);
	
    
	static vector<Humidity>& get1DHumidityFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector<Humidity>& attribute);
	// throw (ConversionException);
	
	 
	static vector <vector<Humidity> >& get2DHumidityFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector<Humidity> >& attribute);
	// throw (ConversionException);
	
 	
	static vector <vector <vector<Humidity> > >& get3DHumidityFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector<Humidity> > >& attribute);
	// throw (ConversionException);
	
	
	static vector <vector <vector <vector<Humidity> > > >& get4DHumidityFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector <vector<Humidity> > > >& attribute);
	// throw (ConversionException);
	

	
	
	static void toXMLBase64(vector<Interval> data, const string &name, string &buf);
	static void toXMLBase64(vector< vector<Interval> > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector<Interval> > > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector< vector<Interval> > > >data, const string &name, string &buf);
	
    
	static vector<Interval>& get1DIntervalFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector<Interval>& attribute);
	// throw (ConversionException);
	
	 
	static vector <vector<Interval> >& get2DIntervalFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector<Interval> >& attribute);
	// throw (ConversionException);
	
 	
	static vector <vector <vector<Interval> > >& get3DIntervalFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector<Interval> > >& attribute);
	// throw (ConversionException);
	
	
	static vector <vector <vector <vector<Interval> > > >& get4DIntervalFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector <vector<Interval> > > >& attribute);
	// throw (ConversionException);
	

	
	
	static void toXMLBase64(vector<Length> data, const string &name, string &buf);
	static void toXMLBase64(vector< vector<Length> > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector<Length> > > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector< vector<Length> > > >data, const string &name, string &buf);
	
    
	static vector<Length>& get1DLengthFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector<Length>& attribute);
	// throw (ConversionException);
	
	 
	static vector <vector<Length> >& get2DLengthFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector<Length> >& attribute);
	// throw (ConversionException);
	
 	
	static vector <vector <vector<Length> > >& get3DLengthFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector<Length> > >& attribute);
	// throw (ConversionException);
	
	
	static vector <vector <vector <vector<Length> > > >& get4DLengthFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector <vector<Length> > > >& attribute);
	// throw (ConversionException);
	

	
	
	static void toXMLBase64(vector<Pressure> data, const string &name, string &buf);
	static void toXMLBase64(vector< vector<Pressure> > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector<Pressure> > > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector< vector<Pressure> > > >data, const string &name, string &buf);
	
    
	static vector<Pressure>& get1DPressureFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector<Pressure>& attribute);
	// throw (ConversionException);
	
	 
	static vector <vector<Pressure> >& get2DPressureFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector<Pressure> >& attribute);
	// throw (ConversionException);
	
 	
	static vector <vector <vector<Pressure> > >& get3DPressureFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector<Pressure> > >& attribute);
	// throw (ConversionException);
	
	
	static vector <vector <vector <vector<Pressure> > > >& get4DPressureFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector <vector<Pressure> > > >& attribute);
	// throw (ConversionException);
	

	
	
	static void toXMLBase64(vector<Speed> data, const string &name, string &buf);
	static void toXMLBase64(vector< vector<Speed> > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector<Speed> > > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector< vector<Speed> > > >data, const string &name, string &buf);
	
    
	static vector<Speed>& get1DSpeedFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector<Speed>& attribute);
	// throw (ConversionException);
	
	 
	static vector <vector<Speed> >& get2DSpeedFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector<Speed> >& attribute);
	// throw (ConversionException);
	
 	
	static vector <vector <vector<Speed> > >& get3DSpeedFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector<Speed> > >& attribute);
	// throw (ConversionException);
	
	
	static vector <vector <vector <vector<Speed> > > >& get4DSpeedFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector <vector<Speed> > > >& attribute);
	// throw (ConversionException);
	

	
	

	
	
	static void toXMLBase64(vector<Temperature> data, const string &name, string &buf);
	static void toXMLBase64(vector< vector<Temperature> > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector<Temperature> > > data, const string &name, string &buf);
	static void toXMLBase64(vector< vector< vector< vector<Temperature> > > >data, const string &name, string &buf);
	
    
	static vector<Temperature>& get1DTemperatureFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector<Temperature>& attribute);
	// throw (ConversionException);
	
	 
	static vector <vector<Temperature> >& get2DTemperatureFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector<Temperature> >& attribute);
	// throw (ConversionException);
	
 	
	static vector <vector <vector<Temperature> > >& get3DTemperatureFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector<Temperature> > >& attribute);
	// throw (ConversionException);
	
	
	static vector <vector <vector <vector<Temperature> > > >& get4DTemperatureFromBase64(const string &name, const string &tableName, const string &xmlDoc, vector <vector <vector <vector<Temperature> > > >& attribute);
	// throw (ConversionException);
	

	
private:
	string str;		// The string being parsed.
	string::size_type pos;		// The current position in the string.
	string::size_type beg;		// The beginning and end of a fragement
	string::size_type end;		//    in the string.
	
public:
	static string substring(const string &s, int a, int b);
	static string trim(const string &s);

}; // End class Parser

inline Parser::Parser(const string &s) : str(s), pos(0), beg(0), end(0) {
}

inline bool Parser::isStr(const string &s) const {
	return str.find(s,pos) == string::npos ? false : true;
}

} // End namespace asdm

#endif /* Parser_CLASS */

