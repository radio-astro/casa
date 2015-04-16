/**
   \file columns.cpp
   Bojan Nikolic <bn204@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   
   Initial version February 2008
   Maintained by ESO since 2013.


*/

#include "columns.hpp"

#include "slice.hpp"
#include "lineparams.hpp"
#include "lineshapes.hpp"
#include "basicphys.hpp"

namespace LibAIR2 {

  void Column::setN(double nnew)
  {
    n=nnew;
  }

  TrivialGrossColumn::TrivialGrossColumn(const HITRAN_entry  & he,
					 double n):
    Column(n),
    he( new HITRAN_entry(he) ),
    pt(NULL)
  {

  }

  TrivialGrossColumn::TrivialGrossColumn(const HITRAN_entry  & he,
					 const PartitionTable *pt,
					 double n) :
    Column(n),
    he( new HITRAN_entry(he) ),
    pt(pt)
  {
  }

  void TrivialGrossColumn::ComputeTau( const std::vector<double> & f,
				       const Slice & s,
				       std::vector<double> & res) const
  {
    res.resize(f.size() );

    CLineParams cp;

    if ( pt == NULL)
    {
      ComputeLinePars( *he, 
		       s.getT() , s.getP(),
		       0, 296,
		       cp);
    }
    else
    {
      ComputeLineParsWQ( *he, 
			 *pt,
			 s.getT() , s.getP(),
			 0, 296,
			 cp);
    }

    for(size_t i =0 ; i < f.size() ; ++i )
    {
      res[i]= GrossLine( f[i],
			 cp.f0, cp.gamma, cp.S) * getN();
    }
    
    
    
  }

  H2OCol::H2OCol(const PartitionTable * pt,
		 size_t nl):
    Column(0),
    ltable(get_h2o_lines()),
    pt(pt)
  {
    if (nl==0)
    {
      nlines=get_h2o_lines_n();
    }
    else
    {
      nlines=nl;
    }
  }

  void H2OCol::ComputeTau(const std::vector<double> &f,
			  const Slice &s,
			  std::vector<double> &res) const
  {
    res=std::vector<double>(f.size(), 0.0);
    const double N=getN();
    CLineParams cp;

    for(size_t i=0; i<nlines; ++i)
    {
      ComputeLineParsWQ(ltable[i], 
			*pt,
			s.getT(),
			s.getP(),
			0, 296,
			cp);

      for(size_t i =0 ; i < f.size() ; ++i )
      {
	res[i]+=GrossLine(f[i],
			  cp.f0,
			  cp.gamma,
			  cp.S)*N;
      }
    }
  }

  ContinuumColumn::ContinuumColumn( double n,
				    ContinuumParams * cp) :
    Column(n),
    cp(cp)
  {

  }

  void ContinuumColumn::ComputeTau( const std::vector<double> & f,
				    const Slice & s,
				    std::vector<double> & res) const
  {

    const double nair = ( s.getP() / STP2::P_STP) * 
      (STP2::T_STP / s.getT() ) * STP2::N_STP;


    const double r = cp->C0 * pow( cp->T0 / s.getT() , cp->m) * getN() * nair;

    for(size_t i =0 ; i < f.size() ; ++i )
    {
      res[i]= r * pow(f[i],2) ; 
    }

  }
  
  EmpContColumn::EmpContColumn(double n,
			       double f0):
    Column(n),
    ff(1.0/f0)
  {
  }
  
  void EmpContColumn::ComputeTau(const std::vector<double> &f,
				 const Slice &s,
				 std::vector<double> &res) const
  {
    const double r=getN();
    for(size_t i=0 ; i<f.size(); ++i)
    {
      res[i]= r * pow(f[i]*ff,2) ; 
    }
    
  }

}


