/**
   \file rtranfer.cpp
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   
   Initial version February 2008
   Maintained by ESO since 2013.

   Some code was previously in slice.hpp/slice.cpp 

*/

#include "rtranfer.hpp"

#include "slice.hpp"
#include "basicphys.hpp"
#include "layers.hpp"

namespace LibAIR2 {
  
  RTResult::RTResult(const std::vector<double> &fp):
    I(fp.size(), 0.0),
    Tb(fp.size()),
    _f(fp),
    f(_f)    
  {
  }

  const std::vector<double> &  RTResult::UpdateTb()
  {
    for (size_t i =0 ; i< f.size() ; ++i)
    {
      Tb[i]  = I[i] / std::pow(f[i],2)  * CSquared_On_TwoK;  ;
    }
    return Tb;
  }  


  SliceResult::SliceResult( const Slice & p_slice,
			    const std::vector<double> & f ) :
    RTResult(f),
    tx(f.size()),
    slice(p_slice)
  {

  }

  void SliceResult::UpdateI(const SliceResult & bckg)
  {

    slice.ComputeTx( f, tx);
    
    for (size_t i =0 ; i< f.size() ; ++i)
    {
      I[i]  = tx[i] * bckg.I[i] + 
	BPlanck(f[i], slice.getT())  * (1-tx[i]) ;
    }
  }

  void SliceResult::UpdateI(void)
  {
    slice.ComputeTx( f, tx);

    for (size_t i =0 ; i< f.size() ; ++i)
    {
      I[i]  = BPlanck(f[i], slice.getT())  * (1-tx[i]) ;
    }

  }

  LayerResult::LayerResult(const Layer & p_layer,
			   const std::vector<double> & f ):
    RTResult(f),
    layer(p_layer)
  {
  }

  void LayerResult::UpdateI(const SliceResult & bckg)
  {
    typedef boost::shared_ptr<SliceResult> sr_p;

    sr_p current( new SliceResult(bckg));
    for ( Layer::sliceL_t::const_iterator slice = layer.getFar();
	  slice != layer.getEnd();
	  ++slice)
    {
      sr_p newslice( new SliceResult( **slice,
				      f));
      newslice->UpdateI( *current);
      current=newslice;
    }
    
    // Copy result
    I=current->getI();
    
  }
  
}


