#ifndef __casac_array_h__
#define __casac_array_h__

#include <vector>

namespace casac {

template<class t> class array {
    public:
  	typedef std::vector<t> vtype;
	typedef std::vector<int> stype;

	array( ) : value_(0), shape_(0) { }
	array( const vtype &v, const stype &s ) : value_( new vtype(v) ),
    						  shape_( new stype(s) ) { }
	array( vtype *v, const stype &s ) : value_(v), shape_(new stype(s)) { }
	vtype &vec( ) { init( ); return *value_; }
	const vtype &vec( ) const { init(); return *value_; }
	stype &shape( ) { init(); return *shape_; }
	const stype &shape( ) const { init(); return *shape_; }

	void resize( const stype &shp ) { init(); if ( shape_ ) delete shape_;
					  shape_ = new stype(shp);
					  unsigned int size = 1;
					  for (stype::const_iterator it=shape_->begin(); it != shape_->end(); ++it) size *= *it;
					  value_->resize(size); }

	void set( const vtype &v, const stype &s )
		{ if ( value_ ) delete value_;
		  if ( shape_ ) delete shape_;
		  value_ = new vtype(v); shape_ = new stype(s); }
	void set( vtype *v, const stype &s )
		{ if ( value_ ) delete value_; 
		  if ( shape_ ) delete shape_;
		  value_ = v; shape_ = new stype(s); }

	~array( ) { if ( value_ ) delete value_;
		    if ( shape_ ) delete shape_; 
		    value_ = 0; shape_ = 0; }
    private:
	void init( ) const { 	if ( ! value_ ) ((array<t>*)this)->value_ = new vtype( );
				if ( ! shape_ ) ((array<t>*)this)->shape_ = new stype( ); }
	vtype *value_;
	stype *shape_;
};

}

#endif
