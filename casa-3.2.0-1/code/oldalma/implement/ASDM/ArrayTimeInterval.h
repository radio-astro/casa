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
 * File ArrayTimeInterval.h
 */

#ifndef ArrayTimeInterval_CLASS
#define ArrayTimeInterval_CLASS

#include <Long.h>
#include <ArrayTime.h>
#include <Interval.h>

#ifndef WITHOUT_ACS
#include <asdmIDLTypesC.h>
#endif

using asdm::Long;
using asdm::Interval;
using asdm::ArrayTime;

#ifndef WITHOUT_ACS
using asdmIDLTypes::IDLArrayTimeInterval;
#endif

namespace asdm {
	
	/**
	 * The ArrayTimeInterval class aggregates an instance of ArrayTime
	 * and an instance of Interval. Instances of ArrayTimeInterval are
	 * used in many ASDM tables whose rows contain information relevant
	 * during a certain laps of time and starting at a given date.
	 */
class ArrayTimeInterval {
friend ostream & operator << ( ostream &, ArrayTimeInterval& );

private:
	ArrayTime start;
	Interval duration;

public:


	// Constructors
	ArrayTimeInterval();
	ArrayTimeInterval(ArrayTime  start,
					  Interval  duration);
					  
	ArrayTimeInterval(double startInMJD,
			  double durationInDays);
			  
	ArrayTimeInterval(long long startInNanoSeconds,
			  long long durationInNanoSeconds);		  				  
					  
	ArrayTimeInterval(ArrayTime start);
	ArrayTimeInterval(double startInMJD);
	ArrayTimeInterval(long long startInNanoSeconds);
	
#ifndef WITHOUT_ACS
	ArrayTimeInterval (IDLArrayTimeInterval t); 
#endif	
	// Setters
	void setStart(ArrayTime start);
	void setStart(double start);
	void setStart(long long start);
	
	void setDuration(Interval duration);
	void setDuration(long long nanoSeconds);
	void setDuration(double days);
		
	// Getters	
	ArrayTime getStart() const ;
	double getStartInMJD() const ;
	long long getStartInNanoSeconds() const ;
	
	Interval getDuration() const ;
	long long getDurationInNanoSeconds() const ;
	double getDurationInDays() const ;
	
	// Checkers
	bool equals(ArrayTimeInterval ati);
	bool overlaps(ArrayTimeInterval ati);
	bool contains(ArrayTimeInterval ati);
	bool contains(ArrayTime at);

	// Operators
	bool operator == (ArrayTimeInterval&);
	bool operator != (ArrayTimeInterval&);
	 	
#ifndef WITHOUT_ACS
	// To IDL conversion
	const asdmIDLTypes::IDLArrayTimeInterval toIDLArrayTimeInterval() const; 
#endif

	/**
	 * Write the binary representation of this to a EndianOSStream.
	 */		
	void toBin(EndianOSStream& eoss);

	/**
	 * Write the binary representation of a vector of ArrayTime to a EndianOSStream.
	 * @param arrayTimeInterval the vector of ArrayTime to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<ArrayTimeInterval>& arrayTimeInterval,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of ArrayTimeInterval to a EndianOSStream.
	 * @param arrayTimeInterval the vector of vector of ArrayTimeInterval to be written
	 * @param eoss the EndianOSStream to be written to
	 */	
	static void toBin(const vector<vector<ArrayTimeInterval> >& arrayTimeInterval,  EndianOSStream& eoss);
	
	/**
	 * Write the binary representation of a vector of vector of vector of ArrayTimeInterval to a EndianOSStream.
	 * @param arrayTimeInterval the vector of vector of vector of ArrayTimeInterval to be written
	 * @param eoss the EndianOSStream to be written to
	 */
	static void toBin(const vector<vector<vector<ArrayTimeInterval> > >& arrayTimeInterval,  EndianOSStream& eoss);

	/**
	 * Read the binary representation of an ArrayTimeInterval from a EndianISStream
	 * and use the read value to set an  ArrayTimeInterval.
	 * @param eiss the EndianStream to be read
	 * @return an ArrayTimeInterval
	 */
	static ArrayTimeInterval fromBin(EndianISStream& eiss);
	
	/**
	 * Read the binary representation of  a vector of  ArrayTimeInterval from an EndianISStream
	 * and use the read value to set a vector of  ArrayTimeInterval.
	 * @param dis the EndianISStream to be read
	 * @return a vector of ArrayTimeInterval
	 */	 
	 static vector<ArrayTimeInterval> from1DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of ArrayTimeInterval from an EndianISStream
	 * and use the read value to set a vector of  vector of ArrayTimeInterval.
	 * @param eiis the EndianISStream to be read
	 * @return a vector of vector of ArrayTimeInterval
	 */	 
	 static vector<vector<ArrayTimeInterval> > from2DBin(EndianISStream & eiss);
	 
	/**
	 * Read the binary representation of  a vector of vector of vector of ArrayTimeInterval from an EndianISStream
	 * and use the read value to set a vector of  vector of vector of ArrayTimeInterval.
	 * @param eiss the EndianISStream to be read
	 * @return a vector of vector of vector of ArrayTimeInterval
	 */	 
	 static vector<vector<vector<ArrayTimeInterval> > > from3DBin(EndianISStream & eiss);
};


// inline constructors

 inline ArrayTimeInterval::ArrayTimeInterval(): start((long long)0), duration(0) {}
 inline ArrayTimeInterval::ArrayTimeInterval(ArrayTime start_, Interval duration_) {
   start = start_;
   duration = Interval(min(duration_.get(), Long::MAX_VALUE - start.get()));
 }
 
 inline ArrayTimeInterval::ArrayTimeInterval(double startInMJD, double durationInDays) :
   start(startInMJD), 
   duration((long long) (ArrayTime::unitsInADay * durationInDays)){}
 
 inline ArrayTimeInterval::ArrayTimeInterval(long long startInNanoSeconds,
					     long long durationInNanoSeconds){
   start = startInNanoSeconds;
   duration = min(durationInNanoSeconds, Long::MAX_VALUE - startInNanoSeconds);
 }				
 
 inline ArrayTimeInterval::ArrayTimeInterval(ArrayTime  start_):
   start(start_) {
   duration = Interval(Long::MAX_VALUE - start.get());
 }
 
 inline ArrayTimeInterval::ArrayTimeInterval(double startInMJD):
   start(startInMJD) {
   this->duration = Interval(Long::MAX_VALUE - start.get());
 }
 
 inline	ArrayTimeInterval::ArrayTimeInterval(long long startInNanoSeconds):
   start(startInNanoSeconds) {
   this->duration = Interval(Long::MAX_VALUE - start.get());
 }
 
 // Inline setters
 inline void ArrayTimeInterval::setStart(ArrayTime start) {
   this->start = ArrayTime(start);
 }
 
 inline void ArrayTimeInterval::setStart(double start) {
   this->start = ArrayTime(start);
 }
 
 inline void ArrayTimeInterval::setStart(long long start) {
   this->start = ArrayTime(start);
 }
 
 inline void ArrayTimeInterval::setDuration(Interval duration) {
   this->duration = Interval(duration);	
 }
 
 
 inline void ArrayTimeInterval::setDuration(double duration) {
   this->duration = Interval((long long) (ArrayTime::unitsInADay * duration));	
 }
 
 inline void ArrayTimeInterval::setDuration(long long duration) {
   this->duration = Interval(duration);	
 }
 
 // inline getters
 inline ArrayTime ArrayTimeInterval::getStart() const {
   return start;	
 }
 
 inline double ArrayTimeInterval::getStartInMJD() const {
   return start.getMJD();	
 }
 
 inline long long ArrayTimeInterval::getStartInNanoSeconds() const  {
   return start.get();	
 }
 
 inline Interval ArrayTimeInterval::getDuration() const {
   return duration;	
 }
 
 inline double ArrayTimeInterval::getDurationInDays() const {
   return (((double) duration.get()) / ArrayTime::unitsInADay);	
 }
 
 inline long long ArrayTimeInterval::getDurationInNanoSeconds() const {
   return duration.get();	
 }
 
 // inline checkers
 inline bool ArrayTimeInterval::equals(ArrayTimeInterval ati) {
   return ((start.get() == ati.getStart().get()) && 
	   (duration.get() == ati.getDuration().get()));
 }
 
 inline bool ArrayTimeInterval::overlaps(ArrayTimeInterval ati) {
   long long start1 = start.get();
   long long end1 = start1 + duration.get();
   
   
   long long start2 = ati.getStart().get();
   long long end2   = start2 + ati.getDuration().get();
   
   return (start2 <= start1 && end2 >= start1) ||
     (start2 >= start1 && start2 <= end1);
 }
 
 inline bool ArrayTimeInterval::contains(ArrayTimeInterval ati) {
   long long start1 = start.get();;
   long long end1 = start1 + duration.get();
   
   long long start2 = ati.getStart().get();
   long long end2   = start2 + ati.getDuration().get();
   
   return (start2>=start1 && end2<=end1);
 }
 
 inline bool ArrayTimeInterval::contains(ArrayTime ati) {
   long long start1 = start.get();
   long long end1 = start1 + duration.get();
   
   long long time = ati.get();
   return (time >= start1 && time < end1);
 }
 
 inline bool ArrayTimeInterval::operator == (ArrayTimeInterval &ati){
   return (start == ati.start) && (duration == ati.duration);
 }
 
 inline bool ArrayTimeInterval::operator != (ArrayTimeInterval &ati){
   return (start != ati.start) || (duration != ati.duration);
 }
 
 inline ostream & operator << ( ostream &o, ArrayTimeInterval &ati ) {
   o << "(start=" << ati.getStart().get() << ",duration=" << ati.getDuration().get() << ")";
   return o;
	
 } // End namespace asdm
} // ArrayTimeInterval_CLASS
#endif
