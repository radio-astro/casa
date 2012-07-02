#include <xmlcasa/conversions.h>
#include <xmlcasa/type_record.h>
#include <xmlcasa/type_variant.h>
#include <xmlcasa/casac.h>

namespace casac {

void convert_idl_complex_to_cplus_complex(const casac::complex &idl_cpx,std::complex<double> &cpx) {
    cpx = std::complex<double>(idl_cpx.re,idl_cpx.im);
}

#define CODADIRECTCPY(ARY,TYPE)								\
void convert_idl_ ## ARY ## Ary_to_cplus_ ## ARY ## Ary(const casac::ARY ## Ary &idl_ary, casac::array<TYPE> &ary) { \
    ary.set(idl_ary.value, idl_ary.shape);						\
}											\
											\
casac::ARY ## Ary convert_idl_ ## ARY ## Ary_from_cplus_ ## ARY ## Ary( casac::array<TYPE> *ary ) { \
    casac::ARY ## Ary result( (*ary).vec(), (*ary).shape() );				\
    /** this seems somewhat problematic... the *_cmpt member functions return an   **/	\
    /** alloced obj, but here it is copied and returned by value (as an IntAry)    **/	\
    /** so... this must consume the converted object... sigh.		           **/	\
    delete ary;										\
    return result;									\
}

CODADIRECTCPY(Bool,bool)
CODADIRECTCPY(Int,int)
CODADIRECTCPY(Double,double)
CODADIRECTCPY(String,std::string)

void convert_idl_ComplexAry_to_cplus_ComplexAry(const casac::ComplexAry &idl_ary, casac::array<std::complex<double> > &ary) {
    ary.resize(idl_ary.shape);
    std::vector<std::complex<double> >::iterator to = ary.vec().begin();
    for ( std::vector<casac::complex>::const_iterator from = idl_ary.value.begin(); from != idl_ary.value.end(); ++from, ++to ) {
	(*to).real() = (*from).re;
	(*to).imag() = (*from).im;
    }
}

casac::ComplexAry convert_idl_ComplexAry_from_cplus_ComplexAry( casac::array<std::complex<double> > *ary ) {
    casac::ComplexAry result;
    result.value.resize(ary->vec().size());
    result.shape.resize(ary->shape().size());
    copy(ary->shape().begin(),ary->shape().end(),result.shape.begin());
    std::vector<casac::complex>::iterator to = result.value.begin();
    for( std::vector<std::complex<double> >::const_iterator from = ary->vec().begin(); from != ary->vec().end(); ++from, ++to ) {
	(*to).re = (*from).real();
	(*to).im = (*from).imag();
    }

    /** this seems somewhat problematic... the *_cmpt member functions return an   **/	\
    /** alloced obj, but here it is copied and returned by value (as an IntAry)    **/	\
    /** so... this must consume the converted object... sigh.		           **/	\
    delete ary;										\
    return result;
}

record *convert_record_value_to_record( const ::WX::Utils::SmartPtr<WX::Utils::Value> &ptr ) {

    WX::Utils::Value *val = ptr.ptr();

    if ( ! val || val->type() != RecordType::instance() )
	return 0;

    RecordValue *rval = dynamic_cast<RecordValue*>( val );
    if ( ! rval ) 
	return 0;

    record *recptr = rval->value();
    if ( ! recptr ) 
	return 0;

   return recptr;
}

variant *convert_variant_value_to_variant( const ::WX::Utils::SmartPtr<WX::Utils::Value> &ptr ) {

    WX::Utils::Value *val = ptr.ptr();

    if ( ! val || val->type() != VariantType::instance() )
	return 0;

    VariantValue *vval = dynamic_cast<VariantValue*>( val );
    if ( ! vval ) 
	return 0;

    variant *varptr = vval->value();
    if ( ! varptr ) 
	return 0;

   return varptr;
}

::WX::Utils::SmartPtr<WX::Utils::Value> convert_variant_value_from_variant( variant *ptr ) {

    return ::WX::Utils::SmartPtr<WX::Utils::Value>(new VariantValue(ptr));
}

WX::Utils::SmartPtr<WX::Utils::Value> convert_record_value_from_record(record *r) {
    return WX::Utils::SmartPtr<WX::Utils::Value>(new RecordValue(r));
}

record initialize_record( const std::string & ) {
    return record();
}

variant initialize_variant( const std::string & ) {
    return variant();
}

}
