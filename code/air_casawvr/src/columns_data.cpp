/**
   \file columns_data.cpp
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   
*/

#include "columns_data.hpp"

#include "columns.hpp"
#include "lineparams.hpp"
#include "partitionsum.hpp"
#include "partitionsum_testdata.hpp"
#include "basicphys.hpp"

namespace LibAIR2 {


  WaterData::WaterData(Lines line,
		       PartitionTreatment t,
		       Continuum c,
		       double n)
  {
    if (line==LALL)
    {
      pt.reset(new PartitionTable(getH2ORawTable()));
      _wcol.reset(new H2OCol(pt.get()));
      _wcol->setN(n*pmw_mm_to_n);
    }
    else
    {
      boost::scoped_ptr<HITRAN_entry> we;
      switch  (line)
      {
      case L183:
	we.reset(Mk183WaterEntry());
	break;
      case L22:
	we.reset(Mk22WaterEntry());
	break;
      default:
	break;
      };
      
      if ( t == PartTable)
      {
	pt.reset(new   PartitionTable( getH2ORawTable() )  );   
	
	_wcol.reset( new TrivialGrossColumn( *we, 
					     pt.get(),
					     n * pmw_mm_to_n ));
      }
      else
      {
	_wcol.reset( new TrivialGrossColumn( *we, 
					     n * pmw_mm_to_n ));
      }
    }

    if ( c == AirCont )
    {
      _wcont.reset( new ContinuumColumn( n * pmw_mm_to_n,
					 MkWaterGrossCont() ) );
    }
  }

  Water183Data::Water183Data(PartitionTreatment t,
			     Continuum c,
			     double n):
    WaterData(L183,
	      t,c,n)
    
  {
  }

  WaterData::~WaterData()
  {
  }

}


