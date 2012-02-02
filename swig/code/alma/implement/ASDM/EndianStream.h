#ifndef EndianStream_CLASS
#define EndianStream_CLASS

#include <stdint.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "Misc.h"

using  namespace std;

namespace asdm {
	
  class EndianOSStream : public ostringstream {
  protected :
    //ostringstream _oss(stringstream::out | stringstream::binary);
			
  public :
    EndianOSStream() ;
    EndianOSStream(const asdm::ByteOrder* byteOrder);
    ~EndianOSStream();
   
    const asdm::ByteOrder* byteOrder() const;
		
    void writeBoolean(bool b);
    void writeBool(bool b);
    void writeByte(char c);
    void writeShort(short int s);
    void writeUShort(unsigned short int s);
    void writeInt(int i);
    void writeUInt(unsigned int ui);
    void writeLongLong(int64_t li);
    void writeLong(int64_t li);
    void writeULongLong(uint64_t li);
    void writeFloat(float f);
    void writeDouble(double d);	
    void writeString(const string& s);

  private:
    const asdm::ByteOrder* byteOrder_;
			
  };	
	
  class EndianISStream : public istringstream {			
  public :
    EndianISStream(const string& s) ;
    EndianISStream(const string& s, const asdm::ByteOrder* byteOrder);
    ~EndianISStream();

    const asdm::ByteOrder* byteOrder() const ;
    bool readBoolean();
    char readByte();
    short int readShort();
    unsigned short int readUShort();
    int readInt();
    unsigned int readUInt();
    int64_t readLongLong();
    int64_t readLong();
    uint64_t readULongLong();
    float readFloat();
    double readDouble();
    string readString();

  private :
    const asdm::ByteOrder* byteOrder_;
  };	
}; // end namespace asdm

#endif  // EndianStream_CLASS
