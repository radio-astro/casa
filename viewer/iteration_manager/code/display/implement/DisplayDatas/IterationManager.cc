#include <display/DisplayDatas/IterationManager.qo.h>
#include <display/QtViewer/QtDisplayPanelGui.qo.h>
#include <display/QtViewer/QtDisplayData.qo.h>
#include <display/DisplayDatas/IterationClient.qo.h>
#include <display/DisplayDatas/IterationClient.qo.h>
#include <coordinates/Coordinates/CoordinateSystem.h>
#include <images/Images/ImageInterface.h>
#include <iostream>
#include <iterator>


namespace casa {
    namespace viewer {

	struct merge_obj {
	    enum DISPLAY_TYPE { ERROR = 1 << 0, RASTER = 1 << 1, CONTOUR = 1 << 2, VECTOR = 1 << 3, MARKER = 1 << 4 };
	    merge_obj( ) : value(0), dd_index(0), type(ERROR) { }
	    merge_obj(double v) : value(v), dd_index(0), type(ERROR) { }
	    merge_obj( const merge_obj &o ) : value(o.value), dd_index(o.dd_index), chan_index(o.chan_index), type(o.type) { }
	    bool operator<(const merge_obj &o ) const { return value < o.value; }
	    double value;
	    unsigned int dd_index;
	    unsigned int chan_index;
	    DISPLAY_TYPE type;
	};

	struct merge_obj_setter {
	    merge_obj_setter( const std::string &t, unsigned int i ) : index(i), count(0),
						type( t == "raster" ? merge_obj::RASTER :
						      t == "contour" ? merge_obj::CONTOUR :
						      t == "vector" ? merge_obj::VECTOR :
						      t == "marker" ? merge_obj::MARKER : merge_obj::ERROR ) { }
	    merge_obj_setter( const merge_obj_setter &o ) : index(o.index) { }
	    void operator(  )( merge_obj &obj ) { obj.dd_index = index; obj.chan_index = count++; obj.type = type; }
	    unsigned int index;
	    unsigned int count;
	    merge_obj::DISPLAY_TYPE type;
	};

	struct merge_obj_collator {
	    struct list_head {
		list_head( double v, unsigned int t=0 ) : value(v), types(t) { }
		list_head( const list_head &o ) : value(o.value), types(o.types) { }
		double value;
		unsigned int types;
	    };
	    typedef std::list<std::pair<list_head,std::list<merge_obj> > > channel_list_type;
	    merge_obj_collator( const std::map<unsigned int, IterationClient*> &c, unsigned int min, unsigned int max ) : clients(c), count(0),
					iteration_begin(max), iteration_end(min) { }
	    merge_obj_collator( const merge_obj_collator &o ) : clients(o.clients), count(o.count),
					iteration_begin(o.iteration_begin), iteration_end(o.iteration_end), channels(o.channels) { }
	    void operator(  )( merge_obj &obj );
	    const std::map<unsigned int, IterationClient*> &clients;
	    channel_list_type channels;
	    unsigned int count;
	    unsigned int iteration_begin;
	    unsigned int iteration_end;
	};

	std::ostream &operator<<( std::ostream &out, const merge_obj &obj ) {
	    if ( obj.type == merge_obj::RASTER ) {
		out << obj.value;
	    } else if ( obj.type == merge_obj::CONTOUR ) {
		out << "{" << obj.value << "}";
	    } else {
		out << "(" << obj.value << ")";
	    }
	    cout << "<" << obj.dd_index << ":" << obj.chan_index << ">";
	    return out;
	}

	std::ostream &operator<<( std::ostream &out, const std::pair<merge_obj_collator::list_head,std::list<merge_obj> > &collation_list ) {
	    if (collation_list.first.types & merge_obj::RASTER) out << "R";
	    if (collation_list.first.types & merge_obj::CONTOUR) out << "C";
	    if (collation_list.first.types & merge_obj::VECTOR) out << "V";
	    if (collation_list.first.types & merge_obj::MARKER) out << "M";
	    out << "[";
	    unsigned int monum = collation_list.second.size( );
	    for ( std::list<merge_obj>::const_iterator moi = collation_list.second.begin( ); moi != collation_list.second.end( ); ++moi ) {
		out << (*moi);
		if ( --monum > 0 ) out << " ";
	    }
	    out << "]";
	    return out;
	}


	std::ostream &operator<<( std::ostream &out, const merge_obj_collator &collator ) {
	    unsigned int cnum = collator.channels.size( );
	    for ( merge_obj_collator::channel_list_type::const_iterator it=collator.channels.begin( );
		  it != collator.channels.end(); ++it ) {
		out << (*it);
		if ( --cnum > 0 ) out << " ";
	    }
	    return out;
	}

	void merge_obj_collator::operator(  )( merge_obj &obj ) {
	    bool inserted = false;
	    for ( channel_list_type::reverse_iterator it=channels.rbegin( );
		  it != channels.rend() && (*it).first.value >= obj.value; ++it ) {
		if ( (*it).first.value == obj.value ) {
		    (*it).second.push_back(obj);
		    (*it).first.types |= obj.type;
		    inserted = true;
		    break;
		}
	    }
	    if ( ! inserted ) {
		channels.push_back( channel_list_type::value_type(list_head(obj.value),std::list<merge_obj>( )) );
		channel_list_type::reverse_iterator it=channels.rbegin( );
		(*it).second.push_back(obj);
		(*it).first.types |= obj.type;
	    }

	    std::map<unsigned int, IterationClient*>::const_iterator it = clients.find(obj.dd_index);
	    if ( it == clients.end( ) ) throw internal_error("failed to find iteration client");
	    if ( (*it).second->displayType( ) == "raster" ) {
		if ( count < iteration_begin ) iteration_begin = count;
		if ( count > iteration_end ) iteration_end = count;
	    }
	    count++;
	}

	static int find_coordinate_index_from_world_index( unsigned int data_axis, const CoordinateSystem &csys ) {
	    for ( int i=0; i < csys.nCoordinates( ); ++i ) {
		Vector<Int> windexes = csys.worldAxes(i);
		for ( int x=0; x < windexes.size( ); ++x )
		    if ( data_axis == windexes[x] ) return i;
	    }
	    return -1;
	}

	IterationManager::IterationManager( QtDisplayPanelGui *gui ) : refresh_needed(true), dpg(gui) { }
	IterationManager::~IterationManager( ) { }

	void IterationManager::registerClient( IterationClient *c ) {
	    clients.push_back(c);

	}
	void IterationManager::unregisterClient( IterationClient *c ) {
	    std::list<IterationClient*>::iterator it = find( clients.begin( ), clients.end( ), c );
	    if ( it != clients.end( ) ) clients.erase(it);
	}

	void IterationManager::locateFrame( unsigned int movie_axis, IPosition &start, IterationClient *it ) {
	    std::vector<merge_obj> refvec;
	    std::map<unsigned int, IterationClient*> clientmap;
	    if ( clients.size() < 2 ) return;
	    fprintf( stderr, ">>>---------------------------------------- locate-frame ----------------------------------------\n" );
	    fprintf( stderr, "movie axis:\t%u\n", movie_axis );
	    cout << "start:\t\t" << start << endl;

	    std::ostream_iterator< double > errout( cerr, " " );
	    std::ostream_iterator<merge_obj> myobjout( cerr, " " );
	    unsigned int count = 0;
	    std::vector<merge_obj>::difference_type pos = std::distance(refvec.begin( ), refvec.end( ));
	    bool all_one_display_type = true;
	    unsigned int at_least_one_raster = 0;
	    std::string all_one_type = (*clients.begin( ))->displayType( );	// assumes non-null list...
	    for ( std::list<IterationClient*>::iterator it = clients.begin( ); it != clients.end(); ++it ) {
		std::string current_display_type = (*it)->displayType( );
		if ( current_display_type == "raster" ) at_least_one_raster += 1;
		if ( current_display_type != all_one_type ) all_one_display_type = false;
		clientmap.insert(std::map<unsigned int, IterationClient*>::value_type(count,*it));
		(*it)->refvector( movie_axis, back_inserter(refvec) );
		std::vector<merge_obj>::iterator begin = refvec.begin( );
		std::advance(begin,pos);
		cout << "  (" << count << ")  " << (*it)->name( ) << ", " << distance(begin,refvec.end()) << " channels, [" << (*it)->displayType( ) << "]" << endl;
		pos = std::distance(refvec.begin( ), refvec.end( ));
		std::for_each( begin, refvec.end( ), merge_obj_setter((*it)->displayType( ), count++) );
	    }

	    fprintf( stderr, "-----------------------------ORIGINAL-REFERENCE-VECTORS-FROM-CUBES------------------------------------------------------\n" );
	    unsigned int orig_count=0;
	    for ( std::list<IterationClient*>::iterator it = clients.begin( ); it != clients.end(); ++it ) {
		std::vector<double> vecref = (*it)->refvector<std::vector<double> >( movie_axis );
		cout << "  (" << orig_count++ << ")  " << (*it)->name( ) << ":" << endl;
		fprintf( stderr, "  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---\n" );
		std::copy( vecref.begin( ), vecref.end( ), errout );
		fprintf( stderr, "  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---  ---\n" );
	    }
	    fprintf( stderr, "------------------------------------------------------------------------------------------------------------------------\n" );
	    std::sort( refvec.begin(), refvec.end() );
	    merge_obj_collator collation = std::for_each( refvec.begin( ), refvec.end( ), merge_obj_collator(clientmap,0,refvec.size( )) );
	    if ( all_one_display_type ) {
		cerr << "All cubes are of the same display type (" << all_one_type << ") yields a virtual cube of " << collation.channels.size( ) << " channels:" << endl;
		fprintf( stderr, "------------------------------------------------------------------------------------------------------------------------\n" );
		cerr << collation << endl;
		fprintf( stderr, "------------------------------------------------------------------------------------------------------------------------\n" );
	    } else {
		if ( at_least_one_raster >= 1 ) {
		    //*
		    //* find the first raster channel... trim non-rasters...
		    //*
		    merge_obj_collator::channel_list_type::iterator first_raster=collation.channels.begin( );
		    while ( first_raster != collation.channels.end() && ! ((*first_raster).first.types & merge_obj::RASTER) ) {
			++first_raster;
		    }
		    // step back to include one non raster frame
		    if ( first_raster == collation.channels.begin( ) ) {
			if ( ! ((*first_raster).first.types & merge_obj::RASTER) )
			    fprintf( stderr, "could not find first raster" );
		    } else {
			merge_obj_collator::channel_list_type::iterator prev = first_raster;
			--prev;
			if ( (*prev).first.types ^ ((*first_raster).first.types & ~ merge_obj::RASTER)) {
			    // channel before the first raster has some unique (non-raster) overlay
			    first_raster = prev;
			}
			collation.channels.erase(collation.channels.begin( ),first_raster);
		    }
		    //*
		    //* find the last raster channel... trim non-rasters...
		    //*
		    merge_obj_collator::channel_list_type::reverse_iterator last_raster=collation.channels.rbegin( );
		    while ( last_raster != collation.channels.rend( ) && ! ((*last_raster).first.types & merge_obj::RASTER) ) {
			++last_raster;
		    }
		    if ( last_raster == collation.channels.rbegin( ) ) {
			if ( ! ((*last_raster).first.types & merge_obj::RASTER) )
			    fprintf( stderr, "could not find last raster" );
		    } else {
			collation.channels.erase(last_raster.base( ), collation.channels.rbegin( ).base( ));
		    }

		    //*
		    //* loop over the collation and populate empty types...
		    //*
		    std::vector<merge_obj> last_dd(clients.size( ));
		    for ( merge_obj_collator::channel_list_type::iterator it = collation.channels.begin( ); it != collation.channels.end(); ++it ) {
			std::vector<bool> has_obj(clients.size( ),false);
			for ( std::list<merge_obj>::iterator moit = (*it).second.begin( ); moit != (*it).second.end( ); ++moit ) {
			    has_obj[(*moit).dd_index] = true;
			    last_dd[(*moit).dd_index] = (*moit);
			}
			for ( unsigned int i=0; i < has_obj.size( ); ++i ) {
			    if ( has_obj[i] == false && last_dd[i].type != merge_obj::ERROR ) {
				(*it).second.push_back(last_dd[i]);
				(*it).first.types |= last_dd[i].type;
			    }
			}
		    }


		    cerr << "One or more original raster cube yields a virtual cube of " << collation.channels.size( ) << " channels:" << endl;
		    fprintf( stderr, "------------------------------------------------------------------------------------------------------------------------\n" );
		    cerr << collation << endl;
		    fprintf( stderr, "------------------------------------------------------------------------------------------------------------------------\n" );
		}
	    }


	    fprintf( stderr, "------------------------------------------------------------------------------------------------------------------------\n" );

	    fprintf( stderr, "\n" );
	    fprintf( stderr, "<<<---------------------------------------- locate-frame ----------------------------------------\n" );

	}

	std::vector<std::string> IterationManager::worldAxisNames( ) const {
	    if ( dpg && dpg->displayPanel( ) ) return dpg->displayPanel( )->worldAxisNames( );
	    return std::vector<std::string>( );
	}

	std::vector<std::string> IterationManager::worldAxisUnits( ) const {
	    if ( dpg && dpg->displayPanel( ) ) return dpg->displayPanel( )->worldAxisUnits( );
	    return std::vector<std::string>( );
	}

    }
}

