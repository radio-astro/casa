#include "SBType.h"

string enum_map_traits<SBType,void>::typeName_="SBType";
string enum_map_traits<SBType,void>::enumerationDesc_="";
string enum_map_traits<SBType,void>::order_="";
string enum_map_traits<SBType,void>::xsdBaseType_="void";
map<SBType,EnumPar<void> > enum_map_traits<SBType,void>::m_;
bool   enum_map_traits<SBType,void>::init_=init();


