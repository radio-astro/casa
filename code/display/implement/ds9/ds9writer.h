#ifndef DISPLAY_DS9_DS9WRITER_H_
#define DISPLAY_DS9_DS9WRITER_H_
#include <stdio.h>

namespace casa {

    class WorldCanvas;

    namespace viewer {

	class ds9writer {
	    public:
		ds9writer( const char *output_path, const char *coord_sys );
		virtual ~ds9writer( );
		void setCsysSource(const char *);
		bool open( );
		bool rectangle( WorldCanvas *, const std::vector<std::pair<double,double> > &pts );
		bool ellipse( WorldCanvas *, const std::vector<std::pair<double,double> > &pts );
		bool polygon( WorldCanvas *, const std::vector<std::pair<double,double> > &pts );
		bool point( WorldCanvas *, const std::vector<std::pair<double,double> > &pts );
	    private:
		typedef std::map<std::string,std::string> str_map_type;
		str_map_type defaults;

		struct cs {
		    typedef void (*from_linear_2_type)(WorldCanvas*wc,double,double,double&,double&);
		    typedef void (*from_linear_4_type)(WorldCanvas*wc,double,double,double,double,double&,double&,double&,double&);
		    cs( const char *ds9_type, MDirection::Types t,
			from_linear_2_type func2, from_linear_4_type func4 ) : ds9(ds9_type), type(t), cvt2(func2), cvt4(func4) { }
		    ~cs( ) { }
		    const std::string ds9;
		    const MDirection::Types type;
		    from_linear_2_type cvt2;
		    from_linear_4_type cvt4;
		};
		typedef std::map<std::string,cs> coord_map_type;
		coord_map_type coord_systems;
		char *path;
		char *csys;
		char *csys_file_path;
		bool opened;
		FILE *fptr;
	};
    }
}

#endif
