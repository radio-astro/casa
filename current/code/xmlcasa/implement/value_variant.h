#ifndef value_variant_h_
#define value_variant_h_
#include <WX/Utils/value.h>
#include <xmlcasa/variant.h>

namespace casac {

class VariantType;

class VariantValue : public WX::Utils::Value {
  public:
    VariantValue( );
    VariantValue( variant *rec );
    Value &operator=( const Value &v );
    Value &operator=( const VariantValue &rv );

    void set_value(variant *);
    variant *value_castawayconst( ) const { return variantv; }
    const variant *value( ) const { return variantv; }
    variant *value( ) { return variantv; }

    ~VariantValue();

  private:
    variant *variantv;
};

} // namespace

#endif
