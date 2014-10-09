/**
   \file radiometer_utils.cpp
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   
   Initial version March 2008
   Maintained by ESO since 2013.

*/

#include "radiometer_utils.hpp"

#include <algorithm>

#include "radiometermeasure.hpp"


namespace LibAIR2 {

  /**
     Helper class for merging of radiometers
   */
  struct RadioIter 
  {
    std::vector<double>::const_iterator f_i;
    std::vector<double>::const_iterator c_i;

    std::vector<double>::const_iterator f_end;

    RadioIter ( const Radiometer * r, 
		size_t ch) :
      f_i  ( r->getFGrid().begin() ),
      c_i  ( r->getCoeffs(ch).begin()),
      f_end( r->getFGrid().end() )
    {
    }

    bool atend(void) const
    {
      return ( f_i == f_end  );
    }

    bool operator< (  const RadioIter & other )
    {
      if (atend())
      {
	return false;
      }
      else if ( other.atend() )
      {
	return true;
      }
      else
      {
	return (*f_i) < (*other.f_i);
      }
    }

    void advance(void)
    {
      ++f_i;
      ++c_i;
    }



  };

  std::auto_ptr<Radiometer> MergeRadiometers( std::vector<const Radiometer *> & vr)
  {
    
    // Number of radiometers
    const size_t nr( vr.size() );
    
    // The merged frequency grid
    std::vector<double> fg;

    // The vector of coeffiecients
    std::vector< std::vector<double> >  cv( nr);

    // The vector iterators
    std::vector< RadioIter >  iv;    
    for ( size_t i =0 ; i < nr ; ++i )
      iv.push_back(RadioIter(vr[i], 0));

    std::vector< RadioIter >::iterator iv_first= iv.begin();

    while (true)
    {
      // i is the radiometer with the smallest current frequency in
      // the set.
      std::vector< RadioIter >::iterator i =
	std::min_element( iv.begin(), iv.end() );

      if (i->atend())
      {
	// we are done
	break;
      }

      fg.push_back( * (i->f_i) );
      for (size_t j = 0  ; j < nr ; ++j)
      {
	if ( (i- iv_first)  == (int)j) 
	{
	  cv[j].push_back( *(i->c_i)  );
	}
	else
	{
	  cv[j].push_back(0.0 );
	}
      }
      
      i->advance();

    }
    
    return std::auto_ptr<Radiometer> ( new Radiometer( fg, cv));

  }


}


