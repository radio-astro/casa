/**
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   Initial version January 2010.
   Maintained by ESO since 2013.

   This file is part of LibAIR and is licensed under GNU Public
   License Version 2

   \file arraygains.cpp

   Structure to hold gains derived from WVR data
*/

#include "arraygains.hpp"
#include "arraydata.hpp"
#include "dtdlcoeffs.hpp"


namespace LibAIR2 {
  

  ArrayGains::ArrayGains(const std::vector<double> &time, 
			 const std::vector<double> &el, 
			 const std::vector<size_t> &state, 
			 const std::vector<size_t> &field, 
			 const std::vector<size_t> &source, 
			   size_t nAnt):
    time(time),
    el(el),
    state(state), field(field), source(source),
    path(boost::extents[time.size()][nAnt]),
    nAnt(nAnt)
  {
  }

  void ArrayGains::calc(const InterpArrayData &wvrdata,
			const std::vector<double> &coeffs)
  {
    std::vector<double> c;
    reweight_thermal(coeffs, c);

    const size_t ntimes=wvrdata.nTimes();
    for (size_t i=0; i<ntimes; ++i)
    {
      for(size_t j=0; j<wvrdata.nWVRs; ++j)
      {
	double cpath=0;
	for(size_t k=0; k<4; ++k)
	{
	  double T = wvrdata.g_wvrdata()[i][j][k];

	  if(T>0. && coeffs[k]>0)
	  {
	    cpath+=T*c[k];
	  }
	}
	path[i][j]=cpath;
      }
    }
  }

  void ArrayGains::calc(const InterpArrayData &wvrdata,
			const std::vector<double> &coeffs,
			const std::vector<double> &coeffs2,
			const std::vector<double> &TRef)
  {
    std::vector<double> c, c2;
    reweight_thermal(coeffs, coeffs2, c, c2);

    const size_t ntimes=wvrdata.nTimes();
    for (size_t i=0; i<ntimes; ++i)
    {
      for(size_t j=0; j<wvrdata.nWVRs; ++j)
      {
	double cpath=0;
	for(size_t k=0; k<4; ++k)
	{
	  const double T = wvrdata.g_wvrdata()[i][j][k];
	  if(T>0. && coeffs[k]>0)
	  {
	    cpath+=(T-TRef[k])*c[k]+ 0.5*std::pow(T-TRef[k], 2)*c2[k];
	  }
	}
	path[i][j]=cpath;
      }
    }
  }

  void ArrayGains::calc(const InterpArrayData &wvrdata,
			const std::vector<double> &coeffs,
			const std::vector<double> &weights)
  {
    std::vector<double> c(4);
    for(size_t i=0; i<4; ++i)
      c[i]=weights[i]/coeffs[i];

    const size_t ntimes=wvrdata.nTimes();
    for (size_t i=0; i<ntimes; ++i)
    {
      for(size_t j=0; j<wvrdata.nWVRs; ++j)
      {
	double cpath=0;
	for(size_t k=0; k<4; ++k)
	{
	  const double T = wvrdata.g_wvrdata()[i][j][k];
	  if(T>0. && coeffs[k]>0)
	  {
	    cpath+=T*c[k];
	  }
	}
	path[i][j]=cpath;
      }
    }
  }

  void ArrayGains::calc(const InterpArrayData &wvrdata,
			const dTdLCoeffsBase &coeffs)
  {
    const size_t ntimes=wvrdata.nTimes();

    // c are the original coefficients, cw are the reweighted
    // coefficients
    std::vector<double> c, c2, cw;

    for (size_t i=0; i<ntimes; ++i)
    {
      for(size_t j=0; j<wvrdata.nWVRs; ++j)
      {
	double cpath=0;
	coeffs.get(j, 
		   wvrdata.g_time()[i], 
		   M_PI/2, 
		   c,
		   c2);
	reweight_thermal(c, cw);
	for(size_t k=0; k<4; ++k)
	{
	  const double T = wvrdata.g_wvrdata()[i][j][k];
	  if(T>0. && c[k]!=0)
	  {
	    cpath+=T*cw[k];
	  }
	}
	path[i][j]=cpath;
      }
    }
  }

  void ArrayGains::calcLinI(const ArrayGains &o)
  {
    const std::vector<double> &otime=o.g_time();
    const path_t & opath=o.g_path();
    const size_t ntimes=time.size();
    const size_t notimes=otime.size();

    size_t oi=0;

    for (size_t i=0; i<ntimes; ++i)
    {
      while(oi < notimes and otime[oi]<time[i])
      {
	++oi;
      }
	
      if (oi==0)
      {
	for(size_t j=0; j<nAnt; ++j)
	  path[i][j]=opath[0][j];
      }
      else if (oi == notimes)
      {
	for(size_t j=0; j<nAnt; ++j)
	  path[i][j]=opath[notimes-1][j];
      }
      else
      {
	const double tdelta=otime[oi]-otime[oi-1];
	const double w=(time[i]-otime[oi-1])/tdelta;
	for(size_t j=0; j<nAnt; ++j)
	{
	  path[i][j]=opath[oi-1][j]*(1-w) + opath[oi][j]*w;
	}
      }
    }
  }

  void ArrayGains::scale(double s)
  {
    const size_t ntimes=time.size();
    for (size_t i=0; i<ntimes; ++i)
    {
      for(size_t j=0; j<nAnt; ++j)
      {
        path[i][j]=path[i][j]*s;
      }
    }
  }

  const double ArrayGains::deltaPath(size_t timei,
				     size_t i,
				     size_t j) const
  {
    return path[timei][i]-path[timei][j];
  }

  const double ArrayGains::absPath(size_t timei,
				   size_t i) const
  {
    return path[timei][i];
  }

  double ArrayGains::greatestRMSBl(const std::vector<std::pair<double, double> > &tmask) const
  {
    const size_t ntimes=time.size();
    double maxrms=0;
    for(size_t i=0; i<nAnt; ++i)
      for(size_t j=i+1; j<nAnt;++j)
      {
	double sx=0, sx2=0;
	size_t csegment=0;
	size_t npoints=0;
	for (size_t k=0; k<ntimes; ++k)
	{
	  if (time[k]<tmask[csegment].first)
	    continue;
	  if (time[k]>=tmask[csegment].first && time[k]<=tmask[csegment].second)
	  {
	    if(absPath(k, i)>0. && absPath(k,j)>0.)
	    {
	      const double d=deltaPath(k, i, j);
	      sx+=d;
	      sx2+= (d*d);
	      ++npoints;
	    }
	  }
	  if (time[k]>=tmask[csegment].second && csegment<tmask.size()-1)
	    ++csegment;
	}
	if (npoints>0)
	{
	  const double rms= pow(sx2/((double)npoints)-pow(sx/((double)npoints), 2), 0.5);
	  if (rms>maxrms)
	  {
	    maxrms=rms;
	  }
	}
      }
    return maxrms;
  }

  void ArrayGains::pathRMSAnt(std::vector<double> &res) const 
  {
    std::vector<std::pair<double, double> > tmask;
    tmask.push_back(std::pair<double, double>(g_time()[0], g_time()[g_time().size()-1]));
    pathRMSAnt(tmask, res);
  }

  void ArrayGains::pathRMSAnt(const std::vector<std::pair<double, double> > &tmask,
			      std::vector<double> &res) const
  {
    res.resize(nAnt);
    for(size_t i=0; i<nAnt; ++i)
    {
      double sx=0, sx2=0;
      size_t csegment=0;
      size_t npoints=0;
      for (size_t k=0; k<time.size(); ++k)
      {
	if (time[k]<tmask[csegment].first)
	  continue;
	
	if (time[k]>=tmask[csegment].first && time[k]<=tmask[csegment].second)
	{
	  if(absPath(k, i)>0.)
	  {
	    const double d=absPath(k, i) * std::sin(el[k]);
	    sx+=d;
	    sx2+= (d*d);
	    ++npoints;
	  }
	}
	if (time[k]>=tmask[csegment].second && csegment<tmask.size()-1)
	  ++csegment;
	if(npoints>0)
	{
	  const double rms= pow(sx2/((double)npoints)-pow(sx/((double)npoints), 2), 0.5);
	  res[i]=rms;
	}
	else
	{
	  res[i]=0.;
	}
      }
    }
  }
  
  void ArrayGains::pathDiscAnt(const ArrayGains &other,
			       std::vector<double> &res) const
  {
    res.resize(nAnt);
    size_t ntimes=time.size();
    for(size_t i=0; i<nAnt; ++i)
    {
      double sx=0, sx2=0;
      double np = 0;
      for (size_t k=0; k<ntimes; ++k)
      {
	if(absPath(k, i)>0. && other.absPath(k,i)>0.)
	{
	  const double d=(absPath(k, i)-other.absPath(k,i));
	  sx+=d;
	  sx2+= (d*d);
	  np+=1.;
	}
      }
      if(np>0)
      {
	const double rms= pow(sx2/np-pow(sx/np, 2), 0.5);
	res[i]=rms;
      }
      else
      {
	res[i] = 0.;
      }
    }
  }

  void ArrayGains::pathDiscAnt(const ArrayGains &other,
			       const std::vector<std::pair<double, double> > &tmask,
			       std::vector<double> &res) const
  {
    res.resize(nAnt);
    const size_t ntimes=time.size();
    //std::cerr << ntimes << std::endl; 
    for(size_t i=0; i<nAnt; ++i)
    {
      double sx=0, sx2=0;
      size_t np=0;
      size_t ns=0;
      for (size_t k=0; k<ntimes; ++k)
      {
	if (time[k]<tmask[ns].first)
	  continue;
	if (time[k]>=tmask[ns].first && time[k]<=tmask[ns].second)
	{	
	  //std::cerr << "k i absPath(k, i) other.absPath(k,i) " << k << " " << i << " " << absPath(k, i) << " " << other.absPath(k,i) << std:: endl;
	  if(absPath(k, i)>0. && other.absPath(k,i)>0.)
	  {
	    const double d=(absPath(k, i)-other.absPath(k,i));
	    sx+=d;
	    sx2+= (d*d);
	    ++np;
	  }
	}
	if (time[k]>=tmask[ns].second && ns<tmask.size()-1)
          ++ns;
      }
      if(np>0){
	const double rms= pow(sx2/((double)np)-pow(sx/((double)np), 2), 0.5);
	res[i]=rms;
	//std::cerr << "rms " << rms << std::endl;
      }
      else
      {
	res[i]=0.;
      }
    }
  }

  void ArrayGains::blankSources(std::set<size_t> &flagsrc)
  {
    const size_t ntimes=time.size();
    for (size_t i=0; i<ntimes; ++i)
    {
      if (flagsrc.count(source[i]))
      {
	for(size_t j=0; j<nAnt; ++j)
	{
	  path[i][j]=0;
	}
      }
    }
  }


  void reweight(const std::vector<double> &coeffs,
		std::vector<double> &res )
  {
    const size_t n=coeffs.size();
    res.resize(n);

    double sum=0;
    for(size_t i=0; i<n; ++i)
    {
      if (coeffs[i] != 0)
      {
	res[i]=1.0/coeffs[i];
	sum+=1.0;
      }
      else
      {
	res[i]=0;
      }
    }

    for(size_t i=0; i<n; ++i)
    {
      res[i]/=sum;
    }
  }

  void reweight_thermal(const std::vector<double> &coeffs,
			std::vector<double> &res)
  {
    std::vector<double> c2(4,0.0);
    std::vector<double> res2(4,0.0);
    reweight_thermal(coeffs, c2, res, res2);
  }

  void reweight_thermal(const std::vector<double> &coeffs,
			const std::vector<double> &c2,
			std::vector<double> &res,
			std::vector<double> &res2)
  {
    boost::array<double, 4> noise =  { { 0.1, 0.08, 0.08, 0.09} };
    const size_t n=coeffs.size();
    res.resize(n); 
    res2.resize(n);

    double sum=0;
    for(size_t i=0; i<n; ++i)
    {
      if (coeffs[i] != 0)
      {
	const double dLdT = 1.0/coeffs[i];
	const double w=std::pow(noise[i] * dLdT, -2);
	const double d2LdT2 = -1.0 * c2[i] * std::pow(coeffs[i], -3);
	res[i]=w*dLdT;
	res2[i]=w*d2LdT2;
	sum+=w;
      }
      else
      {
	res[i]=0;
	res2[i]=0;
      }
    }

    for(size_t i=0; i<n; ++i)
    {
      res[i]/=sum;
      res2[i]/=sum;
    }
    
  }

  double thermal_error(const std::vector<double> &coeffs)
  {
    std::vector<double> rwc;
    reweight_thermal(coeffs, rwc);
    boost::array<double, 4> noise =  { { 0.1, 0.08, 0.08, 0.09} };
    double sum=0;
    for(size_t i=0; i<4; ++i)
    {
      if (rwc[i] != 0)
      {
	sum+=std::pow( noise[i]*rwc[i], 2);
      }
    }
    return pow(sum, 0.5);    
  }


}

