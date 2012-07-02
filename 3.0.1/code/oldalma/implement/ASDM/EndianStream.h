#ifndef EndianStream_CLASS
#define EndianStream_CLASS

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using  namespace std;

namespace asdm {
	
	class EndianOSStream : public ostringstream {
		protected :
			//ostringstream _oss(stringstream::out | stringstream::binary);
			
		public :
			EndianOSStream() ;
			~EndianOSStream();
		
			void writeBoolean(bool b);
			void writeByte(char c);
			void writeShort(short int s);
			void writeUShort(unsigned short int s);
			void writeInt(int i);
			void writeUInt(unsigned int ui);
			void writeLongLong(long long int li);
			void writeULongLong(unsigned long long int li);
			void writeFloat(float f);
			void writeDouble(double d);	
			void writeString(const string& s);
	};	
	
	class EndianISStream : public istringstream {			
		public :
			EndianISStream(const string& s) ;
			~EndianISStream();
		
			bool readBoolean();
			char readByte();
			short int readShort();
			unsigned short int readUShort();
			int readInt();
			unsigned int readUInt();
			long long int readLongLong();
			unsigned long long int readULongLong();
			float readFloat();
			double readDouble();
			string readString();
	};	
}; // end namespace asdm

#endif  // EndianStream_CLASS
