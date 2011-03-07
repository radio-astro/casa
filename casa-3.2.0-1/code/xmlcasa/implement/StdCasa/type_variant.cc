#include <xmlcasa/type_variant.h>
#include <xmlcasa/value_variant.h>
#include <assert.h>

namespace casac {

LTA_MEMDEF(VariantType, 1, "$Id$");

const VariantValue *VariantType::default_value_ = 0;

const VariantType *VariantType::instance( ) {
    static const VariantType *type = new VariantType();
    return type;
}

VariantType::VariantType( ) { }
VariantType::~VariantType( ) { }

VariantValue *VariantType::narrow(WX::Utils::Value *v) const {
    return (v->type() == this) ? dynamic_cast<VariantValue*>(v) : NULL;
}

const VariantValue *VariantType::const_narrow(const WX::Utils::Value *v) const {
    return (v->type() == this) ? dynamic_cast<const VariantValue*>(v) : NULL;
}

VariantValue *VariantType::create() const { return new VariantValue( ); }

bool VariantType::can_assign(const WX::Utils::Value &v, WX::Utils::Error *e) const {
    if (dynamic_cast<const VariantValue*>(&v))
	return true;
    if (e) {
      COMPOSE_ERROR_MSG(WX::Utils::Error, err, v.typestr()<<" "
			"does not match "<<typestr());
      *e = err;
    }
    return false;
}

void VariantType::assign(WX::Utils::Value *self, const WX::Utils::Value *that) const {
    if (self == that) return;
    VariantValue *rself = this->narrow(self);
    assert(rself);
    const VariantValue *rthat = this->const_narrow(that);
    const variant *rthatr = rthat->value();
    rself->set_value(rthatr ? rthatr->clone() : 0);
    return;
}

const std::string &VariantType::typestr( ) const {
    static std::string variant_str("variant");
    return variant_str;
}

WX::Utils::Value *VariantType::fit(WX::Utils::Value *v) const {
    if (v->type() == this)
	return v;
    VariantValue *rv = dynamic_cast<VariantValue*>(v);
    if (rv)
	return new VariantValue(rv->value());
    THROW_ERROR_MSG(WX::Utils::Error, "Cannot convert "<<v->type()->typestr()<<" to "<<this->typestr());
}

WX::Utils::Value* VariantType::shallow_copy(const WX::Utils::Value* self) const {
    const VariantValue *rself = const_narrow(self);
    assert(rself);
    return new VariantValue(rself->value_castawayconst());
}

WX::Utils::Value* VariantType::toplevel_copy(const WX::Utils::Value* self) const {
    const VariantValue *rself = const_narrow(self);
    assert(rself);
    return new VariantValue(rself->value_castawayconst());
}
WX::Utils::Value* VariantType::deep_copy(const WX::Utils::Value* self) const {
    const VariantValue *rself = const_narrow(self);
    assert(rself);
    return new VariantValue(rself->value()->clone());
}

const WX::Utils::Value* VariantType::default_value() const {
    if (default_value_ == 0)
	default_value_ = new VariantValue( );
    return default_value_;
}

int VariantType::compare(const WX::Utils::Value *self, const WX::Utils::Value *that) const {
    if ( self == that )
	return 0;
    const VariantValue *rself = const_narrow(self);
    assert(rself);
    const VariantValue *rthat = const_narrow(that);
    if (rthat)
	return rself->value()->compare(rthat->value());
    THROW_ERROR_MSG(Type::IncompatibleComparison,
		    "Cannot compare "<<this->typestr()<<" with "<<that->typestr());
}

void VariantType::convert_from(WX::Utils::Value *self, const WX::Utils::Value *that) const {
    VariantValue *rself = narrow(self);
    assert(rself);
    const VariantValue *rthat = const_narrow(that);
    if (rthat) {
	const variant *rthatr = rthat->value();
	rself->set_value(rthatr ? rthatr->clone() : 0);
	return;
    }
    assert(0);
}

} // namespace
