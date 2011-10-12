#ifndef type_variant_h_
#define type_variant_h_

#include <WX/Utils/type.h>
#include <string>
#include <xmlcasa/value_variant.h>

namespace casac {

class WX::Utils::Value;

class VariantType : public WX::Utils::Type {
  public:

    static const VariantType *instance( );

    VariantValue *narrow(WX::Utils::Value*) const;
    const VariantValue *const_narrow(const WX::Utils::Value*) const;

    VariantType();
    ~VariantType();

    /** The official "constructor" for a Value instance. */
    VariantValue *create() const;
    bool can_assign(const WX::Utils::Value&, WX::Utils::Error* =NULL) const;
    void assign(WX::Utils::Value* self, const WX::Utils::Value* that) const;

    /** Fit the given value to have myself as type. This can either
	create a new Value instance, or simply return the given value
	if that has already the right type. */
    WX::Utils::Value* fit(WX::Utils::Value*) const;

    /** Should only be used for informational purposes */
    const std::string& typestr() const;

    WX::Utils::Value* shallow_copy(const WX::Utils::Value* self) const;
    WX::Utils::Value* toplevel_copy(const WX::Utils::Value* self) const;
    WX::Utils::Value* deep_copy(const WX::Utils::Value* self) const;

    /** The default value of the concrete type */
    const WX::Utils::Value* default_value() const;

    int compare(const WX::Utils::Value* self, const WX::Utils::Value* that) const;

    // HACK ALERT!
    void convert_from(WX::Utils::Value* self, const WX::Utils::Value* that) const;

  public:
    LTA_MEMDECL(1);
  private:
    static const VariantValue *default_value_;
};
LTA_STATDEF(VariantType, 1);

}// /namespace

#endif
