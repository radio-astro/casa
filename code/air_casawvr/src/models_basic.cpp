/**
   \file models_basic.cpp
   Bojan Nikolic <b.nikolic@mrao.cam.ac.uk>, <bojan@bnikolic.co.uk>
   
   Initial version February 2008
   Revised 2009
   Maintained by ESO since 2013.

*/

#include <stdexcept>

#include "models_basic.hpp"

#include "radiometermeasure.hpp"
#include "slice.hpp"
#include "columns.hpp"
#include "lineparams.hpp"
#include "basicphys.hpp"
#include "partitionsum.hpp"
#include "partitionsum_testdata.hpp"
#include "rtranfer.hpp"
#include "layers.hpp"

namespace LibAIR2 {

  boost::shared_ptr<Radiometer> SwitchRadiometer(RadiometerT r)
  {
    Radiometer *res;
    switch (r)
    {
    case ALMAProd:
      res=MkFullALMAWVR();
      break;
    case ALMADickeProto:
      res=MkFullDickeProtoWVR();
      break;
    case IRAM22GHz:
      res=MkIRAM22();
      break;
    default:
      throw std::runtime_error("Unknown radiometer type");
    }
    return boost::shared_ptr<Radiometer>(res);
  }
}


