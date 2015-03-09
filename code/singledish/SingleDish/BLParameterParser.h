#ifndef _CASA_BLPARAMETER_PARSER_H_
#define _CASA_BLPARAMETER_PARSER_H_

#include <string>
#include <vector>
//#include<cstdint> //with c++11 support
#include <climits>

#include <libsakura/sakura.h>

#include <casa/aipstype.h>

namespace casa { //# NAMESPACE CASA - BEGIN

struct LineFinderParameter {
  LineFinderParameter(bool const use_lf=false, float const thresh=0.0,
		      std::vector<size_t> const &edges=std::vector<size_t>(2,0),
		      size_t const chavglim=0)
  {
    use_line_finder=use_lf;
    threshold = thresh;
    chan_avg_limit = chavglim;
    for (size_t iedge=0; iedge < 2; ++ iedge) {
	edge[iedge] = edges[iedge % edges.size()] ;
    }
  }
  bool use_line_finder;
  float threshold;
  size_t edge[2];
  size_t chan_avg_limit;
};

struct BLParameterSet {
  BLParameterSet(string const blmask, uint16_t const nfit_max,
		 float const clipthres,
		 LineFinderParameter *lf_param,
		 LIBSAKURA_SYMBOL(BaselineType) const bl_type,
		 uint16_t const fit_order = USHRT_MAX, //UINT16_MAX,
		 size_t const num_piece = USHRT_MAX, //SIZE_MAX,
		 std::vector<size_t> const &nwaves = std::vector<size_t>()
		 )
  {
    baseline_mask = blmask;
    num_fitting_max = nfit_max;
    clip_threshold_sigma = clipthres;
    line_finder = *lf_param;
    baseline_type = bl_type;
    order = fit_order;
    npiece = num_piece;
    nwave = nwaves;
  }
  string baseline_mask;
  uint16_t num_fitting_max;
  float clip_threshold_sigma;
  LineFinderParameter line_finder;
  LIBSAKURA_SYMBOL(BaselineType) baseline_type;
  uint16_t order; //initialize with invalid parameters
  size_t npiece;
  std::vector<size_t> nwave;
};


class BLParameterParser {
public:

  explicit BLParameterParser(string const file_name);

  //Returns a baseline fitting parameter of a certain row and pol ID in MS
  BLParameterSet* GetFitParameter(size_t const rowid,size_t const polid);

  //Returns the name of file that stores 
  inline string get_file_name(){return blparam_file_;};
  //Returns a list of baseline type in the file
  inline std::vector<LIBSAKURA_SYMBOL(BaselineType)>
    get_function_types(){return baseline_types_;};
  //Returns the maximum fit order of specified baseline type to construct BaselineContext
  uint16_t get_max_order(LIBSAKURA_SYMBOL(BaselineType) const type);

private:
  void initialize();
  void parse(string const file_name);
  BLParameterSet ConvertLineToParam(string const *linestr);

  uint16_t GetTypeOrder(BLParameterSet const &bl_param);
  
  string blparam_file_;
  std::vector<LIBSAKURA_SYMBOL(BaselineType)> baseline_types_;
  uint16_t max_orders_[static_cast<size_t>(LIBSAKURA_SYMBOL(BaselineType_kNumElements))];

  typedef enum {BLParameters_kRow = 0,
		BLParameters_kPol,
		BLParameters_kMask,
		BLParameters_kNumIteration,
		BLParameters_kClipThreshold,
		BLParameters_kLineFinder,
		BLParameters_kLFThreshold,
		BLParameters_kLeftEdge,
		BLParameters_kRightEdge,
		BLParameters_kChanAverageLim,
		BLParameters_kBaselineType,
		BLParameters_kOrder,
		BLParameters_kNPiece,
		BLParameters_kNWave,
		BLParameters_kNumElements
  } BLParameters;

}; // class BLParameterParser -END


} //# NAMESPACE CASA - END
  
#endif /* _CASA_BLPARAMETER_PARSER_H_ */
