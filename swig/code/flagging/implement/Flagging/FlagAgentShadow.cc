//# FlagAgentShadow.cc: This file contains the implementation of the FlagAgentShadow class.
//#
//#  CASA - Common Astronomy Software Applications (http://casa.nrao.edu/)
//#  Copyright (C) Associated Universities, Inc. Washington DC, USA 2011, All rights reserved.
//#  Copyright (C) European Southern Observatory, 2011, All rights reserved.
//#
//#  This library is free software; you can redistribute it and/or
//#  modify it under the terms of the GNU Lesser General Public
//#  License as published by the Free software Foundation; either
//#  version 2.1 of the License, or (at your option) any later version.
//#
//#  This library is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY, without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//#  Lesser General Public License for more details.
//#
//#  You should have received a copy of the GNU Lesser General Public
//#  License along with this library; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//#  MA 02111-1307  USA
//# $Id: $

#include <flagging/Flagging/FlagAgentShadow.h>

namespace casa { //# NAMESPACE CASA - BEGIN

// Definition of static members for common pre-processing
vector<Int> FlagAgentShadow::shadowedAntennas_p;
casa::async::Mutex FlagAgentShadow::staticMembersMutex_p;
vector<bool> FlagAgentShadow::startedProcessing_p;
bool FlagAgentShadow::preProcessingDone_p = false;
uShort FlagAgentShadow::nAgents_p = 0;

FlagAgentShadow::FlagAgentShadow(FlagDataHandler *dh, Record config, Bool writePrivateFlagCube, Bool flag):
		FlagAgentBase(dh,config,ROWS_PREPROCESS_BUFFER,writePrivateFlagCube,flag)
{
	setAgentParameters(config);

	// Set preProcessingDone_p static member to false
	preProcessingDone_p = false;

	// Request loading antenna1,antenna2 and uvw
	flagDataHandler_p->preLoadColumn(VisBufferComponents::Ant1);
	flagDataHandler_p->preLoadColumn(VisBufferComponents::Ant2);
	flagDataHandler_p->preLoadColumn(VisBufferComponents::Uvw);

	// FlagAgentShadow counters and ids to handle static variables
	staticMembersMutex_p.acquirelock();
	agentNumber_p = nAgents_p;
	nAgents_p += 1;
	staticMembersMutex_p.unlock();

	// Set timekeeper to zero - this will later detect when the timestep changes.
        currTime_p=0.0;

        // Append the supplied additional antennas to COPIES of existing base-class lists.

	// Append to existing lists of antenna info.
	Int nAntsInMS = flagDataHandler_p->antennaNames_p->nelements();
	Int nNewAnts=0;


        //  antennaNames_p
        // antennaDiameters_p
	// antennaPositions_p
        if( additionalAntennas_p.nfields() )
	  {

            // For debugging...
	    //ostringstream recprint;
	    //additionalAntennas_p.print(recprint);
	    //cout << " Additional Antennas : " << recprint.str() << endl;

	    // TODO : Verify input Record. If invalid, print warning and proceed with no extra antennas.
            Bool validants=True;
	    String errorReason;
	    for(Int anew=0; anew<additionalAntennas_p.nfields(); anew++)
	      {
		// Extract the record.
		Record arec = additionalAntennas_p.subRecord(RecordFieldId(String::toString(anew)));

                if( ! arec.isDefined("diameter") || 
                    ( arec.type(arec.fieldNumber("diameter")) != casa::TpFloat && 
		      arec.type(arec.fieldNumber("diameter")) != casa::TpDouble  ) )
		  {
		    validants=False;
		    errorReason += String("Input Record [") + String::toString(anew) + ("] needs a field 'diameter' of type <double> \n");
		  }

                if( ! arec.isDefined("position") || 
                    ( arec.type(arec.fieldNumber("position")) != casa::TpArrayFloat && 
		      arec.type(arec.fieldNumber("position")) != casa::TpArrayDouble  ) )
		  {
		    validants=False;
		    errorReason += String("Input Record [") + String::toString(anew) + ("] needs a field 'position' of type Array<double>\n");
		  }
		else
		{		  
		  Array<Double> tpos;
		  arec.get( RecordFieldId(String("position")) , tpos );
                  if(tpos.shape() != IPosition(1,3))
		    {
		      validants=False;
		      errorReason += String("'position' for Record [") + String::toString(anew)+ ("] must be a vector of 3 floats or doubles\n");
		    }
		}
		
	      }// end of valid-ants loop
	    
	    // If antenna list is valid, set the number of new antennas to add.
            if(validants)
	      {
		nNewAnts = additionalAntennas_p.nfields();
	      }
	    else // warn and continue.
	      {
		*logger_p << LogIO::WARN << "NOT using additional antennas for shadow calculations, for the following reason(s) : " << errorReason << LogIO::POST;
	      }
	  }// if additionalAnts exist.
	

	// Make holders for cumulative information
	shadowAntennaPositions_p.resize(nAntsInMS+nNewAnts);
	///        shadowAntennaNames_p.resize(nAntsInMS+nNewAnts);
        shadowAntennaDiameters_p.resize(nAntsInMS+nNewAnts);	

	// Copy existing antennas into these arrays
	for(Int antid=0;antid<nAntsInMS;antid++)
	  {
	    shadowAntennaPositions_p[antid] = flagDataHandler_p->antennaPositions_p->operator()(antid);
	    ///shadowAntennaNames_p[antid] = flagDataHandler_p->antennaNames_p->operator()(antid);
	    shadowAntennaDiameters_p[antid] = flagDataHandler_p->antennaDiameters_p->operator()(antid);
	  } 
	
	// If any additional antennas are given, and are valid, add them to the lists
	for(Int antid=0;antid<nNewAnts;antid++)
	  {
            // Extract the record.
	    Record arec = additionalAntennas_p.subRecord(RecordFieldId(String::toString(antid)));

            // Extract and add new positions
	    Array<Double> aposarr;
	    arec.get( RecordFieldId(String("position")) , aposarr );
	    Vector<Double> aposvec(aposarr);
	    MVPosition apos(aposvec(0),aposvec(1),aposvec(2));
	    shadowAntennaPositions_p[nAntsInMS+antid] = MPosition(apos,MPosition::Types(MPosition::ITRF));

            // Extract and add new diameters
            Double adia;
            arec.get( RecordFieldId(String("diameter")) , adia );            
	    shadowAntennaDiameters_p[nAntsInMS+antid] = adia;

            // Extract and add new names
            ///String aname aname;
            ///arec.get( RecordFieldId(String("name")) , aname );            
	    ///shadowAntennaNames_p[nAntsInMS+antid] = aname;

	  }
	
	
}// end of constructor
  
  FlagAgentShadow::~FlagAgentShadow()
  {
    // Compiler automagically calls FlagAgentBase::~FlagAgentBase()
    
    // NOTE: The following is necessary because the static variables
    // persist even if all the instances of the class were deleted!
    staticMembersMutex_p.acquirelock();
    agentNumber_p = nAgents_p;
    nAgents_p -= 1;
    staticMembersMutex_p.unlock();
  }
  
  void
  FlagAgentShadow::setAgentParameters(Record config)
  {
    logger_p->origin(LogOrigin(agentName_p,__FUNCTION__,WHERE));
    int exists;
    
    // Amount of shadowing to allow. Float or Double, in units of Meters.
    exists = config.fieldNumber ("tolerance");
    if (exists >= 0)
      {
	shadowTolerance_p = config.asDouble("tolerance");
      }
    else
      {
	shadowTolerance_p = 0.0;
      }
    
    *logger_p << logLevel_p << " tolerance is " << shadowTolerance_p << " meters "<< LogIO::POST;
    
    // A list of antenna parameters, to add to those in the antenna subtable, to calculate shadows.
    exists = config.fieldNumber ("addantenna");
    if (exists >= 0)
      {
	additionalAntennas_p = config.subRecord( RecordFieldId("addantenna") );
      }
    else
      {
	additionalAntennas_p = Record();
      }
    
    ostringstream recprint;
    additionalAntennas_p.print(recprint);
    *logger_p << logLevel_p << " addantenna is " << recprint.str() << LogIO::POST;
    
    return;
  }
  
  void
  FlagAgentShadow::preProcessBuffer(const VisBuffer &visBuffer)
  {
    if (nAgents_p > 1)
      {
	staticMembersMutex_p.acquirelock();
	
	if (!preProcessingDone_p)
	  {
	    // Reset processing state variables
	    if (startedProcessing_p.size() != nAgents_p) startedProcessing_p.resize(nAgents_p,false);
	    for (vector<bool>::iterator iter = startedProcessing_p.begin();iter != startedProcessing_p.end();iter++)
	      {
		*iter = false;
	      }
	    
	    // Do actual pre-processing
	    preProcessBufferCore(visBuffer);
	    
	    // Mark pre-processing as done so that other agents don't redo it
	    preProcessingDone_p = true;
	  }
	
	staticMembersMutex_p.unlock();
      }
    else
      {
	preProcessBufferCore(visBuffer);
      }
    
    return;
  }
  
  void
  FlagAgentShadow::preProcessBufferCore(const VisBuffer &visBuffer)
  {
    // This function is empty, because shadowedAntennas_p needs to be re-calculated for
    // every new timestep, and it is done inside computeRowFlags(), whenever the
    // timestep changes. 

    // (1) For agent-parallelization across "rows", which may include baseline-groups and different timesteps,
    // one possible optimization is to pre-calculate shadowed antennas (calculateShadowedAntennas()) 
    // and store  it in a 'map' indexed by 'time'. This way, all agents can access the same 
    // 'shadowedAntenna_p' for their baselines.  The first agent to advance to the next timestep should
    // compute a new list of shadowed antennas, and append to this map.  After the last agent is finished
    // with a given timestep, it can be removed from the map. 

    // (2) An additional optimization idea is to re-use existing uvw values from the visbuffer whenever possible,
    // but this may not be helpful because you anyway have to compute shadowed antennas based on
    // positions of all antennas (not just the ones seen by the current visbuffer), and after that, 
    // the cost of per-baseline computations is reduced to a subtraction (u1-u2, v1-v2, w1-w2). 
    // It's cost may be comparable to reading from the uvw() of the visbuffer.  

    // Bottom line : Check scaling numbers, if required, try (1).  

  }
  
  void FlagAgentShadow::calculateShadowedAntennas(const VisBuffer &visBuffer, Int rownr)
  {
    
    //  (1) For the current timestep, compute UVWs for all antennas.
    //    uvwAnt_p will be filled these values.
    computeAntUVW(visBuffer, rownr);
    
    //  (2) For all antenna pairs, calculate UVW of the baselines, and check for shadowing.
    shadowedAntennas_p.clear();
    Double u,v,w, uvDistance;
    Double antennaDiameter1,antennaDiameter2, antennaDistance;
    ///Int nAnt = flagDataHandler_p->antennaNames_p->nelements(); 
    Int nAnt = shadowAntennaDiameters_p.nelements(); 
    //    Vector<Double> *antennaDiameters = flagDataHandler_p->antennaDiameters_p;
    
    for (Int antenna1=0; antenna1<nAnt; antenna1++)
      {	    
	Double x1=uvwAnt_p(0,antenna1), y1=uvwAnt_p(1,antenna1), z1=uvwAnt_p(2,antenna1);
	for (Int antenna2=antenna1; antenna2<nAnt; antenna2++)
	  {
	    // Check if this row corresponds to autocorrelation (Antennas don't shadow themselves)
	    if (antenna1 == antenna2) continue;
	    
	    ///// TODO : For baselines between non-existant antennas, don't bother calculating. 
	    Double x2=uvwAnt_p(0,antenna2), y2=uvwAnt_p(1,antenna2), z2=uvwAnt_p(2,antenna2);
	    
	    // Get antenna diameter
	    //	    antennaDiameter1 = (*antennaDiameters)[antenna1];
	    //antennaDiameter2 = (*antennaDiameters)[antenna2];
	    antennaDiameter1 = shadowAntennaDiameters_p[antenna1];
	    antennaDiameter2 = shadowAntennaDiameters_p[antenna2];
	    
	    //  Compute effective distance for shadowing
	    ///antennaDistance = (antennaDiameter1+antennaDiameter2)*(antennaDiameter1+antennaDiameter2)/4.0;
	    
	    antennaDistance = (antennaDiameter1+antennaDiameter2)/2.0;
	    
	    u = x2-x1;
	    v = y2-y1;
	    w = z2-z1;
	    uvDistance = sqrt(u*u + v*v);
	    
	    // Check if one of the antennas can be shadowed
	    if (uvDistance < antennaDistance - shadowTolerance_p)
	      {
		if (w>0)
		  {
		    if (std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna1) == shadowedAntennas_p.end())
		      {
			shadowedAntennas_p.push_back(antenna1);
		      }
		  }
		else
		  {
		    if (std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna2) == shadowedAntennas_p.end())
		      {
			shadowedAntennas_p.push_back(antenna2);
		      }
		  }
	      }
	  }// end for antenna2
      }// end for antenna1
  }// end of calculateShadowedAntennas
  
  /// NOTE : This function is almost a copy of
  ///  ms/MeasurementSets/NewMSSimulator::calcAntUVW
  ///  -- TODO : try to re-use that code by moving out all private-member accesses in the simulator.
  Bool FlagAgentShadow::computeAntUVW(const VisBuffer &vb, Int rownr)
  {
    // Get time and timeinterval from the visbuffer.
    Double Time,Tint;
    Time = vb.time()(rownr);
    Tint = vb.timeInterval()(rownr);
    
    MEpoch epoch(Quantity((Time + Tint/2), "s"), MEpoch::UT1);
    MDirection refdir(vb.direction1()(rownr));  // Get the MDirection of the feed of antenna 1. Assume all ants point in the same direction.
    
    MPosition obsPos( shadowAntennaPositions_p[0] ); // read position of first antenna as reference. Does not matter, since uvws are only differences.
    
    MVPosition basePos=obsPos.getValue();
    MeasFrame measFrame(obsPos);
    measFrame.set(epoch);
    measFrame.set(refdir);
    MVBaseline mvbl;
    MBaseline basMeas;
    MBaseline::Ref basref(MBaseline::ITRF, measFrame);
    basMeas.set(mvbl, basref);
    basMeas.getRefPtr()->set(measFrame);
    // going to convert from ITRF vector to J2000 baseline vector I guess !
    if(refdir.getRef().getType() != MDirection::J2000)
      throw(AipsError("Internal FlagAgentShadow restriction : Ref direction must be in  J2000 "));
    
    Int nAnt = shadowAntennaDiameters_p.nelements();
    if(uvwAnt_p.shape() != IPosition(2,3,nAnt)) 
      {
	uvwAnt_p.resize(3,nAnt);
      }
    
    MBaseline::Convert elconv(basMeas, MBaseline::Ref(MBaseline::J2000));
    Muvw::Convert uvwconv(Muvw(), Muvw::Ref(Muvw::J2000, measFrame));
    for(Int k=0; k< nAnt; ++k)
      {
	MPosition antpos=shadowAntennaPositions_p(k);   // msc.antenna().positionMeas()(k);
	
	MVBaseline mvblA(obsPos.getValue(), antpos.getValue());
	basMeas.set(mvblA, basref);
	MBaseline bas2000 =  elconv(basMeas);
	MVuvw uvw2000 (bas2000.getValue(), refdir.getValue());
	const Vector<double>& xyz = uvw2000.getValue();
	uvwAnt_p.column(k)=xyz;
      }
    
    return True;
  }
  
  
  bool
  FlagAgentShadow::computeRowFlags(const VisBuffer &visBuffer, FlagMapper &flags, uInt row)
  {
    // If we have advanced to a new timestep, calculate new antenna UVW values and shadowed antennas
    // This function resets and fills 'shadowedAntennas_p'.
    if( currTime_p != visBuffer.time()(row) )
      {  
	currTime_p = visBuffer.time()(row) ;
	calculateShadowedAntennas(visBuffer, row);
      }
    
    bool flagRow = false;
    // Flag row if either antenna1 or antenna2 are in the list of shadowed antennas
    Int antenna1 = visBuffer.antenna1()[row];
    Int antenna2 = visBuffer.antenna2()[row];
    if (	(std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna1) != shadowedAntennas_p.end()) or
		(std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna2) != shadowedAntennas_p.end()) )
      {
	flagRow = true;
      }
    
    if ((nAgents_p > 1) and preProcessingDone_p)
      {
	startedProcessing_p[agentNumber_p] = true;
	if (std::find (startedProcessing_p.begin(), startedProcessing_p.end(), false) == startedProcessing_p.end())
	  {
	    preProcessingDone_p = false;
	  }
      }
    
    return flagRow;
  }
  
  
#if 0
  void
  FlagAgentShadow::preProcessBufferCore(const VisBuffer &visBuffer)
  {
    Vector<Int> antenna1list =  visBuffer.antenna1();
    Vector<Int> antenna2list =  visBuffer.antenna2();
    shadowedAntennas_p.clear();
    Double u,v,w, uvDistance;
    Int antenna1, antenna2;
    Double antennaDiameter1,antennaDiameter2, antennaDistance;
    for (Int row_i=0;row_i<antenna1list.size();row_i++)
      {
	// Retrieve antenna ids
	antenna1 = antenna1list[row_i];
	antenna2 = antenna2list[row_i];
	
	// Check if this row corresponds to autocorrelation (Antennas don't shadow themselves)
	if (antenna1 == antenna2) continue;
	
	// Get antenna diameter
	if (antennaDiameter_p>0)
	  {
	    antennaDiameter1 = antennaDiameter_p;
	    antennaDiameter2 = antennaDiameter_p;
	  }
	else
	  {
	    Vector<Double> *antennaDiameters = flagDataHandler_p->antennaDiameters_p;
	    antennaDiameter1 = (*antennaDiameters)[antenna1];
	    antennaDiameter2 = (*antennaDiameters)[antenna2];
	  }
	
	// Compute effective distance for shadowing
	antennaDistance = (antennaDiameter1+antennaDiameter2)*(antennaDiameter1+antennaDiameter2)/4.0;
	
	// Compute uv distance
	u = visBuffer.uvw()(row_i)(0);
	v = visBuffer.uvw()(row_i)(1);
	w = visBuffer.uvw()(row_i)(2);
	uvDistance = u*u + v*v;
	
	// Check if one of the antennas can be shadowed
	if (uvDistance < antennaDistance)
	  {
	    if (w>0)
	      {
		if (std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna1) == shadowedAntennas_p.end())
		  {
		    shadowedAntennas_p.push_back(antenna1);
		  }
	      }
	    else
	      {
		if (std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna2) == shadowedAntennas_p.end())
		  {
		    shadowedAntennas_p.push_back(antenna2);
		  }
	      }
	  }
      }
  }
  
  bool
  FlagAgentShadow::computeRowFlags(const VisBuffer &visBuffer, FlagMapper &flags, uInt row)
  {
    bool flagRow = false;
    // Flag row if either antenna1 or antenna2 are in the list of shadowed antennas
    Int antenna1 = visBuffer.antenna1()[row];
    Int antenna2 = visBuffer.antenna2()[row];
    if (	(std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna1) != shadowedAntennas_p.end()) or
		(std::find (shadowedAntennas_p.begin(), shadowedAntennas_p.end(), antenna2) != shadowedAntennas_p.end()) )
      {
	flagRow = true;
      }
    
    if ((nAgents_p > 1) and preProcessingDone_p)
      {
	startedProcessing_p[agentNumber_p] = true;
	if (std::find (startedProcessing_p.begin(), startedProcessing_p.end(), false) == startedProcessing_p.end())
	  {
	    preProcessingDone_p = false;
	  }
      }
    
    return flagRow;
  }
  
  
  
#endif
  
  
} //# NAMESPACE CASA - END


