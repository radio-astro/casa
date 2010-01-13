#include <EndianStream.h>
#define write5(x) write((const char*) &x, sizeof(x))


namespace asdm {
	
  EndianOSStream::EndianOSStream():byteOrder_(asdm::ByteOrder::Machine_Endianity)  {;}
  

  EndianOSStream::EndianOSStream(const asdm::ByteOrder* byteOrder):byteOrder_(byteOrder) {;}

  
  EndianOSStream::~EndianOSStream() { ; }

  const asdm::ByteOrder* EndianOSStream::byteOrder() const {
    return this->byteOrder_;
  }

  void EndianOSStream::writeBoolean(bool b) {
    write((const char*)&b,sizeof(bool));	
  }

  void EndianOSStream::writeBool(bool b) {
    EndianOSStream::writeBoolean(b);	
  }

  void EndianOSStream::writeByte(char c) {
    write((const char*)&c,sizeof(c));	
  }

  void EndianOSStream::writeShort(short s) {
    if ( byteOrder_ != ByteOrder::Machine_Endianity)
      ByteSwap5(s);
	
    //	write((const char*)&vn,sizeof(vn));
    //	write((const char*)&s,sizeof(s));
    write5(s);
				
  }

  void EndianOSStream::writeUShort(unsigned short s) {
    if ( byteOrder_ != ByteOrder::Machine_Endianity)
      ByteSwap5(s);
    write5(s);				
  }


  void EndianOSStream::writeInt(int i) {
    if ( byteOrder_ != ByteOrder::Machine_Endianity)
      ByteSwap5(i);
    write5(i);			
  }

  void EndianOSStream::writeUInt(unsigned int ui) {
    if ( byteOrder_ != ByteOrder::Machine_Endianity)    
      ByteSwap5(ui);
    write5(ui);		
  }

  void EndianOSStream::writeLongLong(long long int li) {
    if ( byteOrder_ != ByteOrder::Machine_Endianity)    
      ByteSwap5(li);
    write5(li);		
  }

  void EndianOSStream::writeLong(long long int li) {
    writeLongLong(li);
  }

  void EndianOSStream::writeULongLong(unsigned long long int li) {
    if ( byteOrder_ != ByteOrder::Machine_Endianity)    
      ByteSwap5(li);
    write5(li);		
  }

  void EndianOSStream::writeFloat(float f) {
    if ( byteOrder_ != ByteOrder::Machine_Endianity)    
      ByteSwap5(f);
    write5(f);	
  }
 
  void EndianOSStream::writeDouble(double d) {
    if ( byteOrder_ != ByteOrder::Machine_Endianity)    
      ByteSwap5(d);
    write5(d);		
  }

  void EndianOSStream::writeString(const string& s) {
    writeInt(s.length());
    *this  << s;
  }


  EndianISStream::EndianISStream(const string & s):byteOrder_(asdm::ByteOrder::Machine_Endianity) {
    this->str(s);
  }


  EndianISStream::EndianISStream(const string & s, const asdm::ByteOrder* byteOrder):byteOrder_(byteOrder) {
    this->str(s);
  }


  EndianISStream::~EndianISStream() { ; }


  const asdm::ByteOrder*  EndianISStream::byteOrder() const {
    return this->byteOrder_;
  }


  bool EndianISStream::readBoolean() {
    bool result;
    read ((char *) &result, sizeof(bool));
    return result;
  }

  char EndianISStream::readByte() {
    char result;
    read(&result, 1);
    return result;
  }

  short int EndianISStream::readShort() {
    union u { short i; char c[sizeof(short)]; } un;


    read((char *) &un, sizeof(un));

    if ( byteOrder_ != ByteOrder::Machine_Endianity) {	
      union v { short i; char c[sizeof(short)]; } vn;	
      vn.c[1] = un.c[0];
      vn.c[0] = un.c[1];
      return vn.i;
    }
    else
      return un.i;
  }

  unsigned short int EndianISStream::readUShort() {
    union u { unsigned short i; char c[sizeof( unsigned short)]; } un;


    read((char *) &un, sizeof(un));

    if ( byteOrder_ != ByteOrder::Machine_Endianity) {		
      union v { unsigned short i; char c[sizeof( unsigned short)]; } vn;	
      vn.c[1] = un.c[0];
      vn.c[0] = un.c[1];
      return vn.i;
    }
    else
      return un.i;
  }

  int  EndianISStream::readInt() {
    union u { int i; char c[sizeof(int)]; } un;
	
    read((char *) & un, sizeof(un));

    if ( byteOrder_ != ByteOrder::Machine_Endianity) {	
      union v { int i; char c[sizeof(int)]; } vn;	
      vn.c[3] = un.c[0];
      vn.c[2] = un.c[1];
      vn.c[1] = un.c[2];
      vn.c[0] = un.c[3];	
      return vn.i;
    }
    else
      return un.i;
  }

  unsigned int EndianISStream::readUInt() {
    union u { unsigned int i; char c[sizeof(unsigned int)]; } un;
	
    read((char *) & un, sizeof(un));

    if ( byteOrder_ != ByteOrder::Machine_Endianity) {	
      union v { unsigned int i; char c[sizeof(unsigned int)]; } vn;	
      vn.c[3] = un.c[0];
      vn.c[2] = un.c[1];
      vn.c[1] = un.c[2];
      vn.c[0] = un.c[3];	
      return vn.i;
    }
    else
      return un.i;
  }

  long long int EndianISStream::readLongLong() {
    union u {long long  int i; char c[sizeof(long long  int)]; } un;
	
    read((char *) &un, sizeof(un));

    if ( byteOrder_ != ByteOrder::Machine_Endianity) {
      union v {long long  int i; char c[sizeof(long long  int)]; } vn;			
      vn.c[7] = un.c[0];
      vn.c[6] = un.c[1];
      vn.c[5] = un.c[2];
      vn.c[4] = un.c[3];
      vn.c[3] = un.c[4];
      vn.c[2] = un.c[5];
      vn.c[1] = un.c[6];
      vn.c[0] = un.c[7];	
      return vn.i;
    }
    else
      return un.i;
  }

  long long int EndianISStream::readLong() {
    return readLongLong();
  }

  unsigned long long int EndianISStream::readULongLong() {
    union u {unsigned  long long  int i; char c[sizeof(unsigned long long  int)]; } un;
	
    read((char *) &un, sizeof(un));

    if ( byteOrder_ != ByteOrder::Machine_Endianity) {		
      union v {unsigned long long  int i; char c[sizeof(unsigned long long  int)]; } vn;     
      vn.c[7] = un.c[0];
      vn.c[6] = un.c[1];
      vn.c[5] = un.c[2];
      vn.c[4] = un.c[3];
      vn.c[3] = un.c[4];
      vn.c[2] = un.c[5];
      vn.c[1] = un.c[6];
      vn.c[0] = un.c[7];	
      return vn.i;
    }
    else
      return un.i;
  }

  float EndianISStream::readFloat() {
    union u {float  f; char c[sizeof(float)]; } un;
	
    read((char *) &un, sizeof(un));

    if ( byteOrder_ != ByteOrder::Machine_Endianity) {		
      union v {float f;  char c[sizeof(float)]; } vn;	
      vn.c[3] = un.c[0];
      vn.c[2] = un.c[1];
      vn.c[1] = un.c[2];
      vn.c[0] = un.c[3];
      return vn.f;
    }
    else
      return un.f;

  }

  double EndianISStream::readDouble() {
    union u {double  d; char c[sizeof(double)]; } un;
	
    read((char *) &un, sizeof(un));

    if ( byteOrder_ != ByteOrder::Machine_Endianity) {
      union v {double d; char c[sizeof(double)]; } vn;			
      vn.c[7] = un.c[0];
      vn.c[6] = un.c[1];
      vn.c[5] = un.c[2];
      vn.c[4] = un.c[3];
      vn.c[3] = un.c[4];
      vn.c[2] = un.c[5];
      vn.c[1] = un.c[6];
      vn.c[0] = un.c[7];	
      return vn.d;	
    }
    else
      return un.d;
	

  }

  string EndianISStream::readString() {
    int len = readInt();
    char c[len];
    read(c, len);
    return string(c, len);
  }

} // end namespace asdm


