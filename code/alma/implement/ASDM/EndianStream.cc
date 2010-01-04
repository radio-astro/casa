#include <EndianStream.h>
#include <Misc.h> //required for std::swap

#define write5(x) write((const char*) &x, sizeof(x))

namespace asdm {
	
EndianOSStream::EndianOSStream()  {
	;
}

EndianOSStream::~EndianOSStream() { ; }

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
//	union u { short i; char c[sizeof(short)]; } un;
//	union v { short i; char c[sizeof(short)]; } vn;	
//	un.i = s;
//	vn.c[1] = un.c[0];
//	vn.c[0] = un.c[1];

	ByteSwap5(s);
	
//	write((const char*)&vn,sizeof(vn));
//	write((const char*)&s,sizeof(s));
	write5(s);
				
}

void EndianOSStream::writeUShort(unsigned short s) {
//	union u {unsigned short i; char c[sizeof(unsigned short)]; } un;
//	union v {unsigned short i; char c[sizeof(unsigned short)]; } vn;	
//	un.i = s;
//	vn.c[1] = un.c[0];
//	vn.c[0] = un.c[1];
//	
//	write((const char*)&vn,sizeof(vn));
	ByteSwap5(s);
	write5(s);				
}


void EndianOSStream::writeInt(int i) {
//	union u { int i; char c[sizeof(int)]; } un;
//	union v { int i; char c[sizeof(int)]; } vn;	
//	un.i = i;
//	vn.c[3] = un.c[0];
//	vn.c[2] = un.c[1];
//	vn.c[1] = un.c[2];
//	vn.c[0] = un.c[3];
//	
//	write((const char*)&vn,sizeof(vn));
	ByteSwap5(i);
	write5(i);			
}

void EndianOSStream::writeUInt(unsigned int ui) {
//	union u { unsigned int i; char c[sizeof(unsigned int)]; } un;
//	union v { unsigned int i; char c[sizeof(unsigned int)]; } vn;	
//	un.i = ui;
//	vn.c[3] = un.c[0];
//	vn.c[2] = un.c[1];
//	vn.c[1] = un.c[2];
//	vn.c[0] = un.c[3];
//	
//	write((const char*)&vn,sizeof(vn));
	ByteSwap5(ui);
	write5(ui);		
}

void EndianOSStream::writeLongLong(long long int li) {
//	union u {long long  int i; char c[sizeof(long long  int)]; } un;
//	union v {long long  int i; char c[sizeof(long long  int)]; } vn;	
//	un.i = li;
//	vn.c[7] = un.c[0];
//	vn.c[6] = un.c[1];
//	vn.c[5] = un.c[2];
//	vn.c[4] = un.c[3];
//	vn.c[3] = un.c[4];
//	vn.c[2] = un.c[5];
//	vn.c[1] = un.c[6];
//	vn.c[0] = un.c[7];
//	
//	write((const char*)&vn,sizeof(vn));
	ByteSwap5(li);
	write5(li);		
}

void EndianOSStream::writeLong(long long int li) {
	writeLongLong(li);
}

void EndianOSStream::writeULongLong(unsigned long long int li) {
//	union u {unsigned long long  int i; char c[sizeof(unsigned long long  int)]; } un;
//	union v {unsigned long long  int i; char c[sizeof(unsigned long long  int)]; } vn;	
//	un.i = li;
//	vn.c[7] = un.c[0];
//	vn.c[6] = un.c[1];
//	vn.c[5] = un.c[2];
//	vn.c[4] = un.c[3];
//	vn.c[3] = un.c[4];
//	vn.c[2] = un.c[5];
//	vn.c[1] = un.c[6];
//	vn.c[0] = un.c[7];
//	
//	write((const char*)&vn,sizeof(vn));
	ByteSwap5(li);
	write5(li);		
}

void EndianOSStream::writeFloat(float f) {
//	union u {float  f; char c[sizeof(float)]; } un;
//	union v {float f;  char c[sizeof(float)]; } vn;	
//	
//	un.f = f;
//	vn.c[3] = un.c[0];
//	vn.c[2] = un.c[1];
//	vn.c[1] = un.c[2];
//	vn.c[0] = un.c[3];
//	
//	write((const char*)&vn,sizeof(vn));
	ByteSwap5(f);
	write5(f);	
}
 
void EndianOSStream::writeDouble(double d) {
//	union u {double  d; char c[sizeof(double)]; } un;
//	union v {double d; char c[sizeof(double)]; } vn;	
//	un.d = d;
//	vn.c[7] = un.c[0];
//	vn.c[6] = un.c[1];
//	vn.c[5] = un.c[2];
//	vn.c[4] = un.c[3];
//	vn.c[3] = un.c[4];
//	vn.c[2] = un.c[5];
//	vn.c[1] = un.c[6];
//	vn.c[0] = un.c[7];
//	
//	write((const char*)&vn,sizeof(vn));
	ByteSwap5(d);
	write5(d);		
}

void EndianOSStream::writeString(const string& s) {
		writeInt(s.length());
		*this  << s;
}

EndianISStream::EndianISStream(const string & s) {
	this->str(s);
}

EndianISStream::~EndianISStream() { ; }

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
	union v { short i; char c[sizeof(short)]; } vn;	

	read((char *) &un, sizeof(un));
	
	vn.c[1] = un.c[0];
	vn.c[0] = un.c[1];
	return vn.i;
}

unsigned short int EndianISStream::readUShort() {
	union u { unsigned short i; char c[sizeof( unsigned short)]; } un;
	union v { unsigned short i; char c[sizeof( unsigned short)]; } vn;	

	read((char *) &un, sizeof(un));
	
	vn.c[1] = un.c[0];
	vn.c[0] = un.c[1];
	return vn.i;
}

int  EndianISStream::readInt() {
	union u { int i; char c[sizeof(int)]; } un;
	union v { int i; char c[sizeof(int)]; } vn;	
	
	read((char *) & un, sizeof(un));

	vn.c[3] = un.c[0];
	vn.c[2] = un.c[1];
	vn.c[1] = un.c[2];
	vn.c[0] = un.c[3];	
	return vn.i;
}

unsigned int EndianISStream::readUInt() {
	union u { unsigned int i; char c[sizeof(unsigned int)]; } un;
	union v { unsigned int i; char c[sizeof(unsigned int)]; } vn;	
	
	read((char *) & un, sizeof(un));

	vn.c[3] = un.c[0];
	vn.c[2] = un.c[1];
	vn.c[1] = un.c[2];
	vn.c[0] = un.c[3];	
	return vn.i;
}

long long int EndianISStream::readLongLong() {
	union u {long long  int i; char c[sizeof(long long  int)]; } un;
	union v {long long  int i; char c[sizeof(long long  int)]; } vn;	
	
	read((char *) &un, sizeof(un));
	
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

long long int EndianISStream::readLong() {
	return readLongLong();
}

unsigned long long int EndianISStream::readULongLong() {
	union u {unsigned  long long  int i; char c[sizeof(unsigned long long  int)]; } un;
	union v {unsigned long long  int i; char c[sizeof(unsigned long long  int)]; } vn;	
	
	read((char *) &un, sizeof(un));
	
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

float EndianISStream::readFloat() {
	union u {float  f; char c[sizeof(float)]; } un;
	union v {float f;  char c[sizeof(float)]; } vn;	
	
	read((char *) &un, sizeof(un));
	
	vn.c[3] = un.c[0];
	vn.c[2] = un.c[1];
	vn.c[1] = un.c[2];
	vn.c[0] = un.c[3];
	return vn.f;
}

double EndianISStream::readDouble() {
	union u {double  d; char c[sizeof(double)]; } un;
	union v {double d; char c[sizeof(double)]; } vn;	
	
	read((char *) &un, sizeof(un));
	
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

string EndianISStream::readString() {
	int len = readInt();
	char c[len];
	read(c, len);
	return string(c, len);
}

} // end namespace asdm


