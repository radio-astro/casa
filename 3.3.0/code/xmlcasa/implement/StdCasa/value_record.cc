#include <xmlcasa/value_record.h>
#include <xmlcasa/type_record.h>

namespace casac {

RecordValue::RecordValue() : WX::Utils::Value(RecordType::instance()), recordv(new record()) { }
RecordValue::RecordValue( record *rec ) : WX::Utils::Value(RecordType::instance()), recordv(rec)
	{ if ( ! recordv ) recordv = new record(); }

WX::Utils::Value &RecordValue::operator=( const Value &v ) {

    if ( recordv ) delete recordv;

    if ( v.type() != RecordType::instance() )
	recordv = new record();
    else {
	const RecordValue &other = dynamic_cast<const RecordValue&>(v);
	if ( other.recordv )
		recordv = other.recordv->clone();
	else
	    recordv = new record();
    }

    WX::Utils::Value::assign(v);
    return *this;
}

WX::Utils::Value &RecordValue::operator=( const RecordValue &other ) {

    if ( recordv ) delete recordv;

    if ( other.recordv )
	recordv = other.recordv->clone();
    else
	recordv = new record();

    WX::Utils::Value::assign(other);
    return *this;
}

void RecordValue::set_value(record *rec) {

    if ( recordv ) delete recordv;

    recordv = rec;

    if ( ! recordv )
	recordv = new record();
}

RecordValue::~RecordValue() {
    delete recordv;
}

}
