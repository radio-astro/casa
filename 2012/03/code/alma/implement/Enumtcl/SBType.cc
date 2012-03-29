#include "SBType.h"

string enum_map_traits<SBTypeMod::SBType,void>::typeName_="SBType";
string enum_map_traits<SBTypeMod::SBType,void>::enumerationDesc_="";
string enum_map_traits<SBTypeMod::SBType,void>::order_="";
string enum_map_traits<SBTypeMod::SBType,void>::xsdBaseType_="void";
map<SBTypeMod::SBType,EnumPar<void> > enum_map_traits<SBTypeMod::SBType,void>::m_;
bool   enum_map_traits<SBTypeMod::SBType,void>::init_=init();


