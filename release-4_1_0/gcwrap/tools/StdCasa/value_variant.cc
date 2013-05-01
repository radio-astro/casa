#include <xmlcasa/value_variant.h>
#include <xmlcasa/type_variant.h>

namespace casac {

VariantValue::VariantValue() : WX::Utils::Value(VariantType::instance()), variantv(new variant()) { }
VariantValue::VariantValue( variant *rec ) : WX::Utils::Value(VariantType::instance()), variantv(rec)
	{ if ( ! variantv ) variantv = new variant(); }

WX::Utils::Value &VariantValue::operator=( const Value &v ) {

    if ( variantv ) delete variantv;

    if ( v.type() != VariantType::instance() )
	variantv = new variant();
    else {
	const VariantValue &other = dynamic_cast<const VariantValue&>(v);
	if ( other.variantv )
		variantv = other.variantv->clone();
	else
	    variantv = new variant();
    }

    WX::Utils::Value::assign(v);
    return *this;
}

WX::Utils::Value &VariantValue::operator=( const VariantValue &other ) {

    if ( variantv ) delete variantv;

    if ( other.variantv )
	variantv = other.variantv->clone();
    else
	variantv = new variant();

    WX::Utils::Value::assign(other);
    return *this;
}

void VariantValue::set_value(variant *rec) {

    if ( variantv ) delete variantv;

    variantv = rec;

    if ( ! variantv )
	variantv = new variant();
}

VariantValue::~VariantValue() {
    delete variantv;
}

}
