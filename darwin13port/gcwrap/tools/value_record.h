#ifndef value_record_h_
#define value_record_h_
#include <WX/Utils/value.h>
#include <xmlcasa/record.h>

namespace casac {

class RecordType;

class RecordValue : public WX::Utils::Value {
  public:
    RecordValue( );
    RecordValue( record *rec );
    Value &operator=( const Value &v );
    Value &operator=( const RecordValue &rv );

    void set_value(record *);
    record *value_castawayconst( ) const { return recordv; }
    const record *value( ) const { return recordv; }
    record *value( ) { return recordv; }

    ~RecordValue();

  private:
    record *recordv;
};

} // namespace

#endif
