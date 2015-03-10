#include <fstream>

#include <casa/Logging/LogIO.h>
#include <casa/Logging/LogOrigin.h>
#include <casa/Utilities/Assert.h>

#include <singledish/SingleDish/BLParameterParser.h>

using namespace std;

#define _ORIGIN LogOrigin("BLParameterParser", __func__, WHERE)

namespace casa {

BLParameterParser::BLParameterParser(string const file_name)
{
  initialize();
  parse(file_name);
  blparam_file_ = file_name;
}

void BLParameterParser::initialize()
{
  baseline_types_.resize(0);
  // initialize max orders
  size_t num_type = static_cast<size_t>(LIBSAKURA_SYMBOL(BaselineType_kNumElements));
  for (size_t i=0; i<num_type; ++i) {
    max_orders_[i] = 0;
  }
}

uint16_t BLParameterParser::get_max_order(LIBSAKURA_SYMBOL(BaselineType) const type)
{
  
  for (size_t i=0; i<baseline_types_.size(); ++i) {
    if (type == baseline_types_[i]) {
      //type is in file
      return max_orders_[static_cast<size_t>(type)];
    }
  }
  // type is not in file
  throw(AipsError("The baseline type is not in file."));
}


void BLParameterParser::parse(string const file_name)
{
  LogIO os(_ORIGIN);
  ifstream ifs(file_name.c_str(),ifstream::in);
  AlwaysAssert(!ifs.fail(), AipsError);
  string linestr;
  char const separator[1] = {','};
  while (getline(ifs, linestr)) {
    // parse line here to construct BLParameterSet
    // The order should be
    // ROW,POL,MASK,NITERATION,CLIP_THRES,LF,LF_THRES,LEDGE,REDGE,CHANAVG,BL_TYPE,ORDER,N_PIECE,NWAVE
    // size_t,1,string,uint16_t,float,bool,float,size_t,size_t,size_t,sinusoidal,uint16_t,size_t,vector<size_t>
    BLParameterSet bl_param = ConvertLineToParam(&linestr);
    // update bealine_types_ list
    size_t curr_type_idx = static_cast<size_t>(bl_param.baseline_type);
    bool new_type = true;
    for (size_t i=0; i<baseline_types_.size(); ++i){
      if (bl_param.baseline_type==baseline_types_[i]){
	new_type = false;
	break;
      }
    }
    if (new_type) baseline_types_.push_back(bl_param.baseline_type);
    // update max_orders_
    size_t curr_order = GetTypeOrder(bl_param);
    if (curr_order > max_orders_[curr_type_idx])
      max_orders_[curr_type_idx] = curr_order;
  }
}

// vector<string> BLParameterParser::SplitLine(string const linestr, char const separator)
// {
//   vector<string> parsed_line(static_cast<uint16_t>(BLParameters_kNumElements));
//   string elem;
//       vector<string> splitted_line = SplitLine(linestr,separator);
//     AlwaysAssert(splitted_line.size()==BLParameters_kNumElements, AipsError);

//   return parsed_line;
// }

BLParameterSet BLParameterParser::ConvertLineToParam(string const *linestr){
  LineFinderParameter *lf_param;
  return BLParameterSet("",0,0,lf_param,LIBSAKURA_SYMBOL(BaselineType_kPolynomial));
}

uint16_t BLParameterParser::GetTypeOrder(BLParameterSet const &bl_param)
{
  LIBSAKURA_SYMBOL(BaselineType) const type = bl_param.baseline_type;
  switch (type)
  {
  case LIBSAKURA_SYMBOL(BaselineType_kPolynomial):
  case LIBSAKURA_SYMBOL(BaselineType_kChebyshev):
    return bl_param.order;
    break;
  case LIBSAKURA_SYMBOL(BaselineType_kCubicSpline):
    AlwaysAssert(bl_param.npiece<=USHRT_MAX, AipsError);//UINT16_MAX);
    return static_cast<uint16_t>(bl_param.npiece);
    break;
//   case LIBSAKURA_SYMBOL(BaselineType_kSinusoidal):
//     return static_cast<size_t>(bl_param.nwave.size());
//     break;
  default:
    throw(AipsError("Unsupported baseline type."));
  }
}

BLParameterSet* BLParameterParser::GetFitParameter(size_t const rowid,size_t const polid)
{
  LineFinderParameter *lf_param;
  // trunk/code/singledish/SingleDish/BLParameterParser.cc:115:10: error: taking the address of a temporary object of type
  //     'casa::BLParameterSet' [-Waddress-of-temporary]
  //      return &BLParameterSet("",0,0,lf_param,LIBSAKURA_SYMBOL(BaselineType_kPolynomial));
  //             ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  //return &BLParameterSet("",0,0,lf_param,LIBSAKURA_SYMBOL(BaselineType_kPolynomial));
  return new BLParameterSet("",0,0,lf_param,LIBSAKURA_SYMBOL(BaselineType_kPolynomial));
}


} //# NAMESPACE CASA - END
