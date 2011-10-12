#include "PrimitiveDataType.h"

string enum_map_traits<PrimitiveDataType,void>::typeName_="PrimitiveDataType";
string enum_map_traits<PrimitiveDataType,void>::enumerationDesc_="";
string enum_map_traits<PrimitiveDataType,void>::order_="";
string enum_map_traits<PrimitiveDataType,void>::xsdBaseType_="void";
map<PrimitiveDataType,EnumPar<void> > enum_map_traits<PrimitiveDataType,void>::m_;
bool   enum_map_traits<PrimitiveDataType,void>::init_=init();


