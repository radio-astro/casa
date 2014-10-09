/**
   \file radiometermeasure.cpp
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   
   Initial version February 2008
   Maintained by ESO since 2013.

*/

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/assign/list_of.hpp>

#include "radiometermeasure.hpp"
#include "radiometer_utils.hpp"

namespace LibAIR2 {

  Radiometer::Radiometer( const std::vector<double> & FGrid,
			  const std::vector<double> & coeffs):
    FGrid(FGrid),
    coeffs_v(1)
  {
    coeffs_v[0]=coeffs;
  }

  Radiometer::Radiometer( const std::vector<double> & FGrid,
			  const std::vector< std::vector<double> >  & coeffs_v):
    FGrid(FGrid),
    coeffs_v(coeffs_v)    
  {
  }
  

  DSBRadio::DSBRadio( double f_0,
		      double f_if):
    f_0(f_0),
    f_if(f_if)
  {
    std::vector<double> FGrid(2);
    FGrid[0] = f_0- f_if;
    FGrid[1] = f_0+ f_if;
    std::vector<double> coeffs(2, 0.5);
    
    r.reset( new Radiometer(FGrid, coeffs));
  }

  DSBRadio::DSBRadio( double f_0,
		      double f_if,
		      Radiometer * rr):
    r(rr),
    f_0(f_0),
    f_if(f_if)
  {

  }

  const Radiometer & DSBRadio::getRadiometer(void) const
  {
    return *r;
  }


  double DSBRadio::eval( const std::vector<double> & skyTb) const
  {
    return r->eval(skyTb);
  }

  DSBBWRadio::DSBBWRadio( double f_0,
			  double f_if,
			  double f_bw):
    DSBRadio(f_0, f_if, 
	     MkRadio( f_0, f_if, f_bw) ),
    f_bw(f_bw)
  {
  }

  Radiometer * DSBBWRadio::MkRadio( double f_0,
				    double f_if,
				    double f_bw)
  {
    const size_t nsamples=30;

    std::vector<double> FGrid(nsamples *2 );
    std::vector<double> coeffs(nsamples *2, 1.0/(nsamples *2));

    const double delta = f_bw / (nsamples -1);
    const double flow  = f_if - f_bw *0.5;

    for (size_t i =0 ; i < nsamples ; ++i)
    {
      const double fif_c = flow + delta * i;
      FGrid[nsamples-i-1] =   f_0- fif_c;
      FGrid[nsamples+i]   = f_0+ fif_c;
      
    }
    return new Radiometer(FGrid, coeffs);

  }

  DSBBW_QuadRadio::DSBBW_QuadRadio( double f_0,
				    double f_if,
				    double f_bw):
    DSBRadio(f_0, f_if, 
	     MkRadio( f_0, f_if, f_bw) )
  {
  }

  Radiometer * DSBBW_QuadRadio::MkRadio( double f_0,
					 double f_if,
					 double f_bw)
  {
    // 5 point Gauss-Legendre Rule.
    // See http://www.sitmo.com/eq/423
    const double x[] = {-9.06179845938663992811e-01,
			-5.38469310105683091018e-01,
			0.00000000000000000000e+00,    
			5.38469310105683091018e-01,
			9.06179845938663992811e-01};
    
    const double w[] = {2.36926885056189087515e-01,
			4.78628670499366468030e-01,
			5.68888888888888888883e-01,    
			4.78628670499366468030e-01,
			2.36926885056189087515e-01};

    const size_t nsamples = sizeof(x)/sizeof(double);

    std::vector<double> FGrid(nsamples*2);
    std::vector<double> coeffs(nsamples*2);

    /// Note we want the average signal over the bandwidth. Therefore
    /// integrathe the function over bandwidth then divide by
    /// bandwidth to get average value. As ressult coefficients below
    /// do not f_bw in their expressions.

    /// Note that there is an extra factor of * 0.5 becuase we have
    /// two bands

    /// Lower band
    for (size_t i =0 ; i < nsamples ; ++i)
    {
      const double fc = f_0 - f_if;
      FGrid[i] = fc + 0.5* f_bw * x[i];
      coeffs[i]= 0.5 * 0.5 * w[i];
    }

    /// Upper band
    for (size_t i =0 ; i < nsamples ; ++i)
    {
      const double fc = f_0 + f_if;
      FGrid [i+nsamples] = fc + 0.5* f_bw * x[i];
      coeffs[i+nsamples]= 0.5 * 0.5 * w[i];
    }
    return new Radiometer(FGrid, coeffs);
  }

  InvalidWVRChannel::InvalidWVRChannel(int chlow,
				       int chhigh,
				       int ch) :
    chlow(chlow),
    chhigh(chhigh),
    ch(ch)
  {
  }

  DSBRadio * MkALMARadiometer(int ch,
			      double cf_off,
			      double bw_off) throw (InvalidWVRChannel)
  {
    const double filter_c[] = { 1.25, 3.25, 5.5, 7.25};
    const double filter_bw[]= { 1.5 , 2.5 , 2.0, 1.5};

    if ( ch<1 or ch > 4)
    {
      throw InvalidWVRChannel(1 , 4, ch);
    }

    return new DSBBW_QuadRadio( 183.31,
				filter_c[ch-1] +cf_off,
				filter_bw[ch-1]+bw_off);
			   

  }

  DSBRadio * MkALMADickeProto(int ch) throw (InvalidWVRChannel)
  {

    const double filter_c[] = { 0.88 , 1.94 , 3.175, 5.2 };
    const double filter_bw[]= { 0.16 , 0.75 , 1.25 , 2.5 };

    if ( ch<1 or ch > 4)
    {
      throw InvalidWVRChannel(1 , 4, ch);
    }

    return new DSBBW_QuadRadio( 183.31,
			   filter_c[ch-1],
			   filter_bw[ch-1]);
			   

  }

  /***
      Merge the four channels into single radiomer
   */
  Radiometer * MkFullWVR( const boost::function< DSBRadio*  (int ch) > & mkchn )
  {
    std::vector< boost::shared_ptr<DSBRadio>  > dsbv;
    std::vector<const Radiometer *>  rv;

    for (size_t j = 1 ; j < 5; ++j )
    {
      dsbv.push_back( boost::shared_ptr<DSBRadio> (mkchn(j)) );
      rv.push_back( & dsbv[ j-1]->getRadiometer());
    }

    return MergeRadiometers( rv).release();
  }

  Radiometer * MkFullALMAWVR(void)
  {
    return MkFullWVR(boost::bind(MkALMARadiometer,_1,0.0,0.0));
  }

  Radiometer * MkALMAWVR_offset(double cf,
				double bw)
  {
    return MkFullWVR(boost::bind(MkALMARadiometer,
				 _1,
				 cf,
				 bw));
  }

  ALMAWVRCharacter::ALMAWVRCharacter(void)
  {
    cf1=1.25;
    cf2=3.25;
    cf3=5.5;
    cf4=7.25;
    
    bw1=1.5;
    bw2=2.5;
    bw3=2.0;
    bw4=1.5;

  }

  Radiometer *MkALMAWVR(const ALMAWVRCharacter &c)
  {
    std::vector< boost::shared_ptr<DSBRadio> > dsbv=
      boost::assign::list_of(new DSBBW_QuadRadio(183.31,
						 c.cf1,
						 c.bw1))
      (new DSBBW_QuadRadio(183.31,
			   c.cf2,
			   c.bw2))
      (new DSBBW_QuadRadio(183.31,
			   c.cf3,
			   c.bw3))
      (new DSBBW_QuadRadio(183.31,
			   c.cf4,
			   c.bw4))
      ;

    std::vector<const Radiometer *>  rv;    
    for(size_t i=0; i<dsbv.size(); ++i)
    {
      rv.push_back(&dsbv[i]->getRadiometer());
    }
    return MergeRadiometers( rv).release();    
  }


  Radiometer * MkFullDickeProtoWVR(void)
  {
    return MkFullWVR( MkALMADickeProto);
  }

  Radiometer * MkSSBRadio(double f_c,
			  double f_bw)
  {
    // 5 point Gauss-Legendre Rule.  See http://www.sitmo.com/eq/423
    const double x[] = {-9.06179845938663992811e-01,
			-5.38469310105683091018e-01,
			0.00000000000000000000e+00,    
			5.38469310105683091018e-01,
			9.06179845938663992811e-01};
    
    const double w[] = {2.36926885056189087515e-01,
			4.78628670499366468030e-01,
			5.68888888888888888883e-01,    
			4.78628670499366468030e-01,
			2.36926885056189087515e-01};

    const size_t nsamples = sizeof(x)/sizeof(double);

    std::vector<double> FGrid(nsamples);
    std::vector<double> coeffs(nsamples);

    for (size_t i =0 ; i < nsamples ; ++i)
    {
      FGrid[i] = f_c + 0.5* f_bw * x[i];
      coeffs[i]= 0.5 * w[i];
    }
    
    return new Radiometer(FGrid, coeffs);

  }

  Radiometer * MkIRAM22(void)
  {
    const double filter_c[] = { 19.2, 22, 25.2};
    const double filter_bw[] = {1.15, 1.15, 1.15};
    
    std::vector< boost::shared_ptr<Radiometer> > rv;
    std::vector< const Radiometer * > rv_obs;
    
    for (size_t i =0 ; i < 3; ++i)
    {
      rv.push_back(boost::shared_ptr<Radiometer>(MkSSBRadio(filter_c[i],
							    filter_bw[i])));

      rv_obs.push_back(rv[i].get());
    }
    
    return MergeRadiometers(rv_obs).release();
    
  }

}



