#ifndef _casac_conversions_h__
#define _casac_conversions_h__
#include <xmlcasa/complex.h>
#include <complex>
#include <WX/Utils/smartptr.h>
#include <WX/Utils/value.h>
#include <xmlcasa/record.h>
#include <xmlcasa/array.h>
#include <xmlcasa/BoolAry.h>
#include <xmlcasa/IntAry.h>
#include <xmlcasa/DoubleAry.h>
#include <xmlcasa/ComplexAry.h>
#include <xmlcasa/StringAry.h>
#include <string>

namespace casac {

void convert_idl_BoolAry_to_cplus_BoolAry(const BoolAry &idl_ary, casac::array<bool> &ary);
BoolAry convert_idl_BoolAry_from_cplus_BoolAry( casac::array<bool> *ary );

void convert_idl_IntAry_to_cplus_IntAry(const IntAry &idl_ary, casac::array<int> &ary);
IntAry convert_idl_IntAry_from_cplus_IntAry( casac::array<int> *ary );

void convert_idl_DoubleAry_to_cplus_DoubleAry(const DoubleAry &idl_ary, casac::array<double> &ary);
DoubleAry convert_idl_DoubleAry_from_cplus_DoubleAry( casac::array<double> *ary );

void convert_idl_ComplexAry_to_cplus_ComplexAry(const ComplexAry &idl_ary, casac::array<std::complex<double> > &ary);
ComplexAry convert_idl_ComplexAry_from_cplus_ComplexAry( casac::array<std::complex<double> > *ary );

void convert_idl_StringAry_to_cplus_StringAry(const StringAry &idl_ary, casac::array<std::string> &ary);
StringAry convert_idl_StringAry_from_cplus_StringAry( casac::array<std::string> *ary );

void convert_idl_complex_to_cplus_complex(const casac::complex &idl_cpx, std::complex<double> &cpx);
record *convert_record_value_to_record(const WX::Utils::SmartPtr<WX::Utils::Value> &);
variant *convert_variant_value_to_variant(const WX::Utils::SmartPtr<WX::Utils::Value> &);
WX::Utils::SmartPtr<WX::Utils::Value> convert_variant_value_from_variant(variant*);
WX::Utils::SmartPtr<WX::Utils::Value> convert_record_value_from_record(record *);
record initialize_record( const std::string & );
variant initialize_variant( const std::string & );

}

#endif
