/**
   \file layers.cpp
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version February 2008
   Maintained by ESO since 2013.

*/

#include "layers.hpp"
#include "slice.hpp"

namespace LibAIR2 {

  Layer::Layer(void)
  {
  }

  Layer::~Layer(void)
  {
  }

  void Layer::addSliceFar(pSlice_t s)
  {
    sliceL.push_front( s);
  }

  void Layer::AddColumn (const Column & c)
  {
    for (sliceL_t::iterator i=sliceL.begin();
	 i!=sliceL.end();
	 i++)
    {
      (*i)->AddColumn(c);
    }
  }

  Layer::sliceL_t::const_iterator 
  Layer::getFar(void) const
  {
    return sliceL.begin();
  }
  
  Layer::sliceL_t::const_iterator  
  Layer::getEnd(void) const
  {
    return sliceL.end();
  }

  const Slice & Layer::getNear(void) const
  {
    return  *sliceL.back();
  }

  const double IsoTLayer::slice_dP=5;

  IsoTLayer::IsoTLayer(double T, double PBase, double PDrop)
  {
    for ( double c=0; c <= PDrop ; c += slice_dP)
    {
      double f;
      if ( c + slice_dP <= PDrop)
	f=slice_dP/PDrop;
      else
	f=(PDrop-c)/PDrop;
	   
      addSliceFar(pSlice_t(new Slice(T, 
				     PBase-c,
				     f)));
    }
  }

  void IsoTLayer::setT(double T)
  {
    for (sliceL_t::iterator i=sliceL.begin();
	 i!=sliceL.end();
	 i++)
    {
      (*i)->setT(T);
    }
  }

  void IsoTLayer::setP(double P)
  {
    for (sliceL_t::iterator i= sliceL.end();
	 i!=sliceL.begin();
	 i--)
    {
      const double newP= P - (sliceL.end()-i)* slice_dP;
      (*(i-1))->setP(newP);
    }

  }
  



}


