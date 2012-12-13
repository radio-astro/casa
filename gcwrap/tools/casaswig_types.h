#ifndef CASASWIG_TYPES_H
#define CASASWIG_TYPES_H 1

#include <stdarg.h>
#include <string>
#include <vector>
#include <stdcasa/Quantity.h>
#include <stdcasa/record.h>
#include <stdcasa/variant.h>
#define USING_NUMPY_ARRAYS 1

namespace casac {

struct complex {
	complex() {}
	complex(double arg0, double arg1): re(arg0), im(arg1){}
	double re;
	double im;
};

template<class T>
std::vector<T> initialize_vector(int count, T v1, ...) {
   va_list ap;
   va_start(ap, v1);
   std::vector<T> result(count);
   result[0] = v1;
   for ( int i=1; i < count; ++i ) {
       T val = va_arg(ap,T);
       result[i] = val;
   }
   return result;
}

struct BoolAry
{
  BoolAry( ) { }
  BoolAry(std::vector<bool> arg0, std::vector<int> arg1) : value(arg0), shape(arg1) { }
  std::vector<bool> value;
  std::vector<int> shape;

};

struct ComplexAry
{
  ComplexAry( ) { }
  ComplexAry(std::vector<casac::complex> arg0, std::vector<int> arg1) : value(arg0), shape(arg1) { }
  std::vector<casac::complex> value;
  std::vector<int> shape;

};

struct DoubleAry
{
  DoubleAry( ) { }
  DoubleAry(std::vector<double> arg0, std::vector<int> arg1) : value(arg0), shape(arg1) { }
  std::vector<double> value;
  std::vector<int> shape;

};

struct IntAry
{
  IntAry( ) { }
  IntAry(std::vector<int> arg0, std::vector<int> arg1) : value(arg0), shape(arg1) { }
  std::vector<int> value;
  std::vector<int> shape;

};

struct StringAry
{
  StringAry( ) { }
  StringAry(std::vector<std::string> arg0, std::vector<int> arg1) : value(arg0), shape(arg1) { }
  std::vector<std::string> value;
  std::vector<int> shape;

};
typedef  std::vector<record> RecordVec;
typedef StringAry StringVec;
typedef IntAry IntVec;
typedef DoubleAry DoubleVec;
typedef BoolAry BoolVec;
typedef ComplexAry ComplexVec;
/*
typedef  std::vector<std::string> StringVec;
typedef  std::vector<double> DoubleVec;
typedef  std::vector<int> IntVec;
typedef  std::vector<bool> BoolVec;
typedef  std::vector<std::complex<double> > ComplexVec;
*/
typedef std::string MDirection;
typedef std::string MRadialVelocity;
typedef std::string MPosition;
typedef std::string Region;

} // casac namespace
#endif
