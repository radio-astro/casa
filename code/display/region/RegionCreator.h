
#ifndef REGION_REGIONCREATOR_H_
#define REGION_REGIONCREATOR_H_
#include <display/region/Region.h>
#include <display/Utilities/VOID.h>
#include <set>
#include <map>
#include <list>

namespace casa {
namespace viewer {
class RegionCreator {
public:
    typedef std::list <RegionCreator*> creator_list_type;
    // called when the user indicates that a region should be deleted...
    virtual void revokeRegion (Region *) = 0;
    // returns the set of region types which this creator will create
    virtual const std::set <Region::RegionTypes> &regionsCreated () const = 0;
    virtual bool create( Region::RegionTypes /*region_type*/, WorldCanvas */*wc*/, const std::vector <std::pair <double, double> > &/*pts*/,
			 const std::string &/*label*/, viewer::Region::TextPosition /*label_pos*/, const std::vector<int> &/*label_off*/,
			 const std::string &/*font*/, int /*font_size*/, int /*font_style*/, const std::string &/*font_color*/,
			 const std::string &/*line_color*/, viewer::Region::LineStyle /*line_style*/, unsigned int /*line_width*/,
			 bool /*annotation*/, VOID */*region_specific_state*/ )= 0;
//			DISPLAY_PURE_VIRTUAL(RegionCreator::create,true);

    RegionCreator ();
    virtual ~RegionCreator ();

    static const creator_list_type &findCreator (Region::RegionTypes type);

private:
    typedef std::map <Region::RegionTypes, creator_list_type*> creator_map_type;
    static creator_map_type creator_map;
    static creator_list_type unsorted_creators;
};

} // end namespace viewer
} // end namespace casa

#endif
