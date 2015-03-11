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
  while (getline(ifs, linestr)) {
    // parse line here to construct BLParameterSet
    // The order should be
    // ROW,POL,MASK,NITERATION,CLIP_THRES,LF,LF_THRES,LEDGE,REDGE,CHANAVG,BL_TYPE,ORDER,N_PIECE,NWAVE
    // size_t,1,string,uint16_t,float,bool,float,size_t,size_t,size_t,sinusoidal,uint16_t,size_t,vector<size_t>
    BLParameterSet bl_param;
    size_t row_idx, pol_idx;
    ConvertLineToParam(linestr, row_idx, pol_idx, bl_param);
    // DEBUG OUTPUT
    {
      os << LogIO::DEBUG1 << "Summary of parsed Parameter" << LogIO::POST;
      os << LogIO::DEBUG1 << "[ROW" << row_idx << ", POL" << pol_idx << "]"
	 << LogIO::POST;
      os << LogIO::DEBUG1 << "- mask: " << bl_param.baseline_mask
	 << LogIO::POST;
      os << LogIO::DEBUG1 << "- clip: iteration=" << bl_param.num_fitting_max
	 << ", threshold=" << bl_param.clip_threshold_sigma << LogIO::POST;
      os << LogIO::DEBUG1
	 << "- line finder: "
	 << (bl_param.line_finder.use_line_finder==1 ? "true" : "false")
	 << ", threshold=" << bl_param.line_finder.threshold
	 << ", edge=[" << bl_param.line_finder.edge[0] << ","
	 << bl_param.line_finder.edge[1] << "], chan_average="
	 << bl_param.line_finder.chan_avg_limit << LogIO::POST;
      os << LogIO::DEBUG1 << "- baseline: type=" << bl_param.baseline_type
	 << ", order=" << bl_param.order << ", npiece=" << bl_param.npiece
	 << LogIO::POST;
    }
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

void BLParameterParser::SplitLine(string const &linestr,
				  char const separator,
				  vector<string> &strvec)
{
  istringstream iss(linestr);
  string selem;
  size_t ielem(0);
  while(getline(iss, selem, separator)) {
    if (ielem >= strvec.size()) {
      strvec.resize(ielem+1);
    }
    strvec[ielem] = selem;
    ++ielem;
  }
}

void BLParameterParser::ConvertLineToParam(string const &linestr,
					   size_t &rowid, size_t &polid,
					   BLParameterSet &paramset){
  //split a line by ',' and make a vector of strings
  std::vector<string> svec(BLParameters_kNumElements,"");
  SplitLine(linestr,',', svec);
  // parse mandatory data
  rowid = ConvertString<size_t>(svec[BLParameters_kRow]);
  polid = ConvertString<size_t>(svec[BLParameters_kPol]);
  size_t num_piece=USHRT_MAX;//SIZE_MAX;
  string const bltype_str = svec[BLParameters_kBaselineType];
  if (bltype_str == "cspline")
  {
    if (svec[BLParameters_kNPiece].size()==0)
      throw(AipsError("Baseline type 'cspline' requires num_piece value."));
    num_piece = ConvertString<size_t>(svec[BLParameters_kNPiece]);
    // CreateBaselineContext does not supoort elements number > max(uint16_t)
    if (num_piece > USHRT_MAX)
      throw(AipsError("num_piece is larger than max of uint16_t"));
    paramset.npiece = num_piece;
    paramset.baseline_type = LIBSAKURA_SYMBOL(BaselineType_kCubicSpline);
  }
  else if (bltype_str == "sinusoid")
  {
    // sinusoid is not supported yet
    throw(AipsError("Unsupported baseline type, sinusoid"));
  }
  else
  { // poly or chebyshev
    if (svec[BLParameters_kOrder].size()==0)
      throw(AipsError("Baseline type 'poly' and 'chebyshev' require order value."));
    paramset.baseline_type = bltype_str == "chebyshev" ?
      LIBSAKURA_SYMBOL(BaselineType_kChebyshev) :
      LIBSAKURA_SYMBOL(BaselineType_kPolynomial);
    paramset.order = ConvertString<uint16_t>(svec[BLParameters_kOrder]);
  }
  // parse clipping parameters
  if (svec[BLParameters_kNumIteration].size() == 0)
    throw(AipsError("Number of maximum clip iteration is mandatory"));
  paramset.num_fitting_max
    = ConvertString<uint16_t>(svec[BLParameters_kNumIteration]);
  if (svec[BLParameters_kClipThreshold].size()>0)
    paramset.clip_threshold_sigma
      = ConvertString<float>(svec[BLParameters_kClipThreshold]);
  // parse line finder parameter
  LineFinderParameter &lf_param = paramset.line_finder;
  lf_param.use_line_finder = svec[BLParameters_kLineFinder]=="true" ? true : false;;
  if (!lf_param.use_line_finder)
  { // use line finder
    if (svec[BLParameters_kLFThreshold].size()>0)
    {
      lf_param.threshold
	= ConvertString<float>(svec[BLParameters_kLFThreshold]);
    }
    vector<size_t> edge(2,0);
    if (svec[BLParameters_kLeftEdge].size() > 0)
      lf_param.edge[0] = ConvertString<size_t>(svec[BLParameters_kLeftEdge]);
    if (svec[BLParameters_kRightEdge].size() > 0)
      lf_param.edge[1] = ConvertString<size_t>(svec[BLParameters_kRightEdge]);
    if (svec[BLParameters_kChanAverageLim].size()>0)
    {
      lf_param.chan_avg_limit
	= ConvertString<size_t>(svec[BLParameters_kChanAverageLim]);
    }
  }
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

bool BLParameterParser::GetFitParameter(size_t const rowid,size_t const polid, BLParameterSet &bl_param)
{
  return true;
}


} //# NAMESPACE CASA - END
