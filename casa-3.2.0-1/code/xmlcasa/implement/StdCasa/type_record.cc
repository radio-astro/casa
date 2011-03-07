#include <xmlcasa/type_record.h>
#include <xmlcasa/value_record.h>
#include <assert.h>

namespace casac {

LTA_MEMDEF(RecordType, 1, "$Id$");

const RecordValue *RecordType::default_value_ = 0;

const RecordType *RecordType::instance( ) {
    static const RecordType *type = new RecordType();
    return type;
}

RecordType::RecordType( ) { }
RecordType::~RecordType( ) { }

RecordValue *RecordType::narrow(WX::Utils::Value *v) const {
    return (v->type() == this) ? dynamic_cast<RecordValue*>(v) : NULL;
}

const RecordValue *RecordType::const_narrow(const WX::Utils::Value *v) const {
    return (v->type() == this) ? dynamic_cast<const RecordValue*>(v) : NULL;
}

RecordValue *RecordType::create() const { return new RecordValue( ); }

bool RecordType::can_assign(const WX::Utils::Value &v, WX::Utils::Error *e) const {
    if (dynamic_cast<const RecordValue*>(&v))
	return true;
    if (e) {
      COMPOSE_ERROR_MSG(WX::Utils::Error, err, v.typestr()<<" "
			"does not match "<<typestr());
      *e = err;
    }
    return false;
}

void RecordType::assign(WX::Utils::Value *self, const WX::Utils::Value *that) const {
    if (self == that) return;
    RecordValue *rself = this->narrow(self);
    assert(rself);
    const RecordValue *rthat = this->const_narrow(that);
    const record *rthatr = rthat->value();
    rself->set_value(rthatr ? rthatr->clone() : 0);
    return;
}

const std::string &RecordType::typestr( ) const {
    static std::string record_str("record/map<string,variant>");
    return record_str;
}

WX::Utils::Value *RecordType::fit(WX::Utils::Value *v) const {
    if (v->type() == this)
	return v;
    RecordValue *rv = dynamic_cast<RecordValue*>(v);
    if (rv)
	return new RecordValue(rv->value());
    THROW_ERROR_MSG(WX::Utils::Error, "Cannot convert "<<v->type()->typestr()<<" to "<<this->typestr());
}

WX::Utils::Value* RecordType::shallow_copy(const WX::Utils::Value* self) const {
    const RecordValue *rself = const_narrow(self);
    assert(rself);
    return new RecordValue(rself->value_castawayconst());
}

WX::Utils::Value* RecordType::toplevel_copy(const WX::Utils::Value* self) const {
    const RecordValue *rself = const_narrow(self);
    assert(rself);
    return new RecordValue(rself->value_castawayconst());
}
WX::Utils::Value* RecordType::deep_copy(const WX::Utils::Value* self) const {
    const RecordValue *rself = const_narrow(self);
    assert(rself);
    return new RecordValue(rself->value()->clone());
}

const WX::Utils::Value* RecordType::default_value() const {
    if (default_value_ == 0)
	default_value_ = new RecordValue( );
    return default_value_;
}

int RecordType::compare(const WX::Utils::Value *self, const WX::Utils::Value *that) const {
    if ( self == that )
	return 0;
    const RecordValue *rself = const_narrow(self);
    assert(rself);
    const RecordValue *rthat = const_narrow(that);
    if (rthat)
	return rself->value()->compare(rthat->value());
    THROW_ERROR_MSG(Type::IncompatibleComparison,
		    "Cannot compare "<<this->typestr()<<" with "<<that->typestr());
}

void RecordType::convert_from(WX::Utils::Value *self, const WX::Utils::Value *that) const {
    RecordValue *rself = narrow(self);
    assert(rself);
    const RecordValue *rthat = const_narrow(that);
    if (rthat) {
	const record *rthatr = rthat->value();
	rself->set_value(rthatr ? rthatr->clone() : 0);
	return;
    }
    assert(0);
}

} // namespace
