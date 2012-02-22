
#ifndef REGION_REGIONCREATOR_H_
#define REGION_REGIONCREATOR_H_
#include <display/region/Region.h>
#include <set>
#include <map>
#include <list>

namespace casa {
    namespace viewer {
	class RegionCreator {
	    public:
		typedef std::list<RegionCreator*> creator_list_type;
		// region types created
		enum Types { RECTANGLE, POLYGON, ELLIPSE, POINT };
		// called when the user indicates that a region should be deleted...
		virtual void revokeRegion( Region * ) = 0;
		// returns the set of region types which this creator will create
		virtual const std::set<Types> &regionsCreated( ) const = 0;
		virtual bool create( Types region_type, WorldCanvas *wc, const std::vector<std::pair<double,double> > &pts, const std::string &label,
				     const std::string &font, int font_size, int font_style, const std::string &font_color,
				     const std::string &line_color, viewer::Region::LineStyle line_style ) = 0;

		RegionCreator( );
		virtual ~RegionCreator( );

		static const creator_list_type &findCreator( Types type );

	    private:
		typedef std::map<Types,creator_list_type*> creator_map_type;
		static creator_map_type creator_map;
		static creator_list_type unsorted_creators;
	};
    }
}

#endif
