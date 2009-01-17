#ifndef type_record_h_
#define type_record_h_

#include <WX/Utils/type.h>
#include <string>
#include <xmlcasa/value_record.h>

namespace casac {

class WX::Utils::Value;

class RecordType : public WX::Utils::Type {
  public:

    static const RecordType *instance( );

    RecordValue *narrow(WX::Utils::Value*) const;
    const RecordValue *const_narrow(const WX::Utils::Value*) const;

    RecordType();
    ~RecordType();

    /** The official "constructor" for a Value instance. */
    RecordValue *create() const;
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
    static const RecordValue *default_value_;
};
LTA_STATDEF(RecordType, 1);

}// /namespace

#endif
