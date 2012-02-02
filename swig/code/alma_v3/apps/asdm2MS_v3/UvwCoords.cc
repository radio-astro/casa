#include "UvwCoords.h"
#include "Error.h"

#include <measures/Measures/MFrequency.h>

UvwCoords::UvwCoords( ASDM* const datasetPtr )
{
  // All the antenna positions for antennas in the antenna table:
  // ------------------------------------------------------------ 
  AntennaTable&        antennas = datasetPtr->getAntenna(); 
  vector<AntennaRow*> v_antenna = antennas.get();

  vector<double> antOffset;   antOffset.reserve(3);  antOffset.resize(3);
  vector<double> stationPos; stationPos.reserve(3); stationPos.resize(3);
  vector<double> antennaPos;

  vector<Length> antOffsetCoords, staCoords;

  for(unsigned int na=0; na<v_antenna.size(); na++){

    antOffsetCoords = v_antenna[na]->getOffset();
    for(unsigned int nc=0; nc<3; nc++)antOffset[nc]=antOffsetCoords[nc].get();

    staCoords = v_antenna[na]->getStationUsingStationId()->getPosition();
    for(unsigned int nc=0; nc<3; nc++)stationPos[nc]=staCoords[nc].get();

    antennaPos = antPos(stationPos,antOffset);
    Vector<Quantity> vq; vq.resize(3);
    for(unsigned int nc=0; nc<3; nc++)vq[nc]=Quantity(antennaPos[nc],"m");
    MVPosition mvp(vq);
    MPosition posref( mvp, MPosition::ITRF );
    m_antPos_.insert(make_pair(v_antenna[na]->getAntennaId(),posref));

  }

  // The different (sub-)arrays present in the ConfigDescription table:
  // ------------------------------------------------------------------
  ConfigDescriptionTable& configDescriptions = datasetPtr->getConfigDescription();
  vector<ConfigDescriptionRow*> v_configDesc = configDescriptions.get();
  
  map<Tag,ArrayParam>::iterator it;

  for(unsigned int n=0; n<v_configDesc.size(); n++){
    ArrayParam arrayParam;
    Tag tag                    = v_configDesc[n]->getConfigDescriptionId();
    arrayParam.v_ant           = v_configDesc[n]->getAntennaId();
    arrayParam.e_correlationMode.set(v_configDesc[n]->getCorrelationMode());
    unsigned int numspw = v_configDesc[n]->getDataDescriptionId().size();
    arrayParam.nrepeat = 0;
    vector<SwitchCycleRow*> v_sr= v_configDesc[n]->getSwitchCyclesUsingSwitchCycleId();

    if (v_sr.size() != numspw)
      Error(FATAL, "It seems that the arrays 'switchCycleId' and 'dataDescriptionId' do not have the same size in one row of the ConfigDescription table !");

    for(unsigned int nspw=0; nspw<numspw; nspw++)
      arrayParam.nrepeat += v_sr[nspw]->getNumStep();
    // INFORM: that we assume that when there are 2 APCs the data are in two MS columns i.e. on the same row
    bool same=false;
    for(it=m_array_.begin(); it!=m_array_.end(); ++it){
      if(it->second==arrayParam){
	arrayParam.subarrayId = it->first;
	same     = true;
	break;
      }
    }
    pair<map<Tag,ArrayParam>::iterator,bool> p_it_ap = m_array_.insert(make_pair(tag,arrayParam));
    if(!same)p_it_ap.first->second.subarrayId = tag;
  }

  // the zero-baseline
  sduvw_.resize(3); sduvw_[0]=0.0;  sduvw_[1]=0.0;  sduvw_[2]=0.0; 

  // observations never occur at epoch 0 ==> the 3 state attributes will be necessarily set at the begining
  timeCentroid_ = 0.0;

}

UvwCoords::~UvwCoords(){
}

void UvwCoords::uvw_an( double                        timeCentroid, 
			const vector<vector<Angle> >& phaseDir,
			const vector<Tag>&            v_antId){

  std::map<Tag,MPosition>::iterator 
    it, itr, 
    itb=m_antPos_.begin(),
    ite=m_antPos_.end();


  MEpoch    epoch( Quantity(timeCentroid,"s"),
  		   MEpoch::UTC);

  casa::Double dirx=phaseDir[0][0].get();
  casa::Double diry=phaseDir[0][1].get();
  MVDirection mvd( Quantity(dirx,"rad"),
		   Quantity(diry,"rad") );
  MDirection phasedir( mvd,
		       MDirection::Ref(MDirection::J2000) );               

//   if(phasedir.getRef().getType() != MDirection::J2000)
//     throw(AipsError("Phase direction is not in  J2000 "));


  itr = itb;                                                              // we use the first antenna in the map for the reference
  if(v_antId.size()!=m_antPos_.size())
    itr = m_antPos_.find(v_antId[0]);

  MeasFrame      measFrame( itr->second,                                  
			    epoch,
			    phasedir);

  MVBaseline     mvbl;
  MBaseline      basMeas;
  MBaseline::Ref basref(MBaseline::ITRF, measFrame);                      // at ref ant, at epoch, for the phaseDir
  basMeas.set(mvbl, basref);
  basMeas.getRefPtr()->set(measFrame);

  MBaseline::Convert elconv(basMeas, MBaseline::Ref(MBaseline::J2000) );  // convert from ITRF vector to J2000 baseline vector

  Muvw          uvwMeas;
  Muvw::Ref     uvwref(Muvw::J2000, measFrame);
  Muvw::Convert uvwconv(uvwMeas, uvwref);
  
  m_antUVW_.clear();

  if(v_antId.size()==m_antPos_.size()){                                   // all the antennas from the antenna table are used
    for(it=itb; it!=ite; ++it){
      MVBaseline mvblA(itr->second.getValue(), it->second.getValue());
      basMeas.set(mvblA, basref);
      MBaseline bas2000 =  elconv(basMeas);
      MVuvw uvw2000 (bas2000.getValue(), phasedir.getValue());
      m_antUVW_.insert(make_pair(it->first,uvw2000.getValue() ));
    }
  }else{
    for(unsigned int na=0; na<v_antId.size(); na++){
      it=m_antPos_.find(v_antId[na]);
      MVBaseline mvblA(itr->second.getValue(), it->second.getValue());
      basMeas.set(mvblA, basref);
      MBaseline bas2000 =  elconv(basMeas);
      MVuvw uvw2000 (bas2000.getValue(), phasedir.getValue());
      m_antUVW_.insert(make_pair(it->first,uvw2000.getValue() ));
    }
  }
  
}


void UvwCoords::uvw_bl( const vector<Tag>& v_antennaId, unsigned int nrep, bool reverse,  vector<Vector<casa::Double> >& v_uvw ){

  Vector<casa::Double> uvw; uvw.resize(3);
  Vector<casa::Double> uvw_i, uvw_j;

  v_uvw.clear();
  unsigned int k=nrep*(v_antennaId.size()*(v_antennaId.size()-1))/2;
  v_uvw.resize(k);
  if(reverse){
    vector<Tag>::const_reverse_iterator
      rit, rit_i, rit_ie, rit_j,
      ritb=v_antennaId.rbegin(),
      rite=v_antennaId.rend();
    rit=ritb;
    rit_ie = rite--;
    while( rit!=rit_ie ){
      rit_i=rit;
      rit++;
      uvw_i = m_antUVW_.find(*rit_i)->second;
      for(rit_j=rit; rit_j!=rite; ++rit_j){
	uvw_j = m_antUVW_.find(*rit_j)->second;
	for(unsigned int n=0; n<3; n++)
	  uvw[n] = uvw_i[n]-uvw_j[n];
	for(unsigned int n=0; n<nrep; n++)v_uvw[--k]=uvw;
      }
    }
  }else{
    unsigned int k=0;
    vector<Tag>::const_iterator
      it, it_i, it_j, it_ie,
      itb=v_antennaId.begin(),
      ite=v_antennaId.end();
    it_ie=ite; it_ie--;
    for(it_i=itb; it_i!=it_ie; ++it_i){
      it_j=it_i;
      it_j++;
      uvw_i = m_antUVW_.find(*it_i)->second;
      for(; it_j!=ite; ++it_j){
	uvw_j = m_antUVW_.find(*it_j)->second;
	for(unsigned int n=0; n<3; n++)
	  uvw[n] = uvw_j[n]-uvw_i[n];
	for(unsigned int n=0; n<nrep; n++)v_uvw[k++]=uvw;
      }
    }
  }
  return;
}


void UvwCoords::uvw_bl( Tag configDescriptionId, 
			const vector<vector<Angle> >& phaseDir,
			double timeCentroid,
			Enum<CorrelationMode> correlationMode,
			bool reverse, bool autoTrailing, 
			vector<Vector<casa::Double> >& v_uvw){

  map<Tag,ArrayParam>::const_iterator itf=m_array_.find(configDescriptionId);

  if(correlationMode[CorrelationModeMod::AUTO_ONLY]){
    v_uvw.assign( itf->second.v_ant.size() * itf->second.nrepeat, 
		  sduvw_ );
    return;
  }

  Tag subarrayId=itf->second.subarrayId;
  if( timeCentroid_ != timeCentroid ||          // new eopoch 
      phaseDir_     != phaseDir     ||          // new (sub)array phase direction
      subarrayId_   != subarrayId   ){          // new (sub)array
    uvw_an(timeCentroid, phaseDir, itf->second.v_ant);
    timeCentroid_        = timeCentroid;
    phaseDir_            = phaseDir;
    subarrayId_          = subarrayId;
  }

  v_uvw.clear();

  if(autoTrailing){
    // the cross baselines
    uvw_bl( itf->second.v_ant, itf->second.nrepeat, reverse, v_uvw );
    if(correlationMode[CorrelationModeMod::CROSS_AND_AUTO]){
      // the auto baselines
      vector<Vector<casa::Double> > vV_sd;     
      vV_sd.assign( itf->second.v_ant.size() * itf->second.nrepeat, 
		    sduvw_ );
      // append the auto to the cross baselines
      v_uvw.insert( v_uvw.end(), vV_sd.begin(),  vV_sd.end() );
    }
  }else{
    if(correlationMode[CorrelationModeMod::CROSS_AND_AUTO]){
      // the auto baselines
      v_uvw.assign( itf->second.v_ant.size() * itf->second.nrepeat, 
		    sduvw_ );
    }
    // the cross baselines
    vector<Vector<casa::Double> > vV_bl;     
    uvw_bl( itf->second.v_ant, itf->second.nrepeat, reverse, vV_bl );
    // append the cross to the auto baselines
    v_uvw.insert( v_uvw.end(), vV_bl.begin(),  vV_bl.end() );
  }
  return;
}

void UvwCoords::uvw_bl( Tag configDescriptionId, 
			const vector<vector<Angle> >& phaseDir,
			const vector<double>& v_timeCentroid,
			Enum<CorrelationMode> correlationMode,
			bool reverse, bool autoTrailing, 
			vector<Vector<casa::Double> >& v_uvw){

  map<Tag,ArrayParam>::const_iterator itf=m_array_.find(configDescriptionId);

  if(correlationMode[CorrelationModeMod::AUTO_ONLY]){
    v_uvw.assign( itf->second.v_ant.size() * itf->second.nrepeat, 
		  sduvw_ );
    return;
  }

  v_uvw.clear();
  if(correlationMode[CorrelationModeMod::CROSS_AND_AUTO] && autoTrailing==false){
    // the auto baselines
    v_uvw.assign( itf->second.v_ant.size() * itf->second.nrepeat, 
		  sduvw_ );
  }

  casa::Vector<casa::Double> V_bl; V_bl.resize(3);
  vector<Tag> v_a; v_a.reserve(2); v_a.resize(2);

  unsigned int k=0;
  //  v_uvw.clear(); FV remove

  // the cross baselines
  if(reverse){
    vector<Tag>::const_reverse_iterator ita, itai, itaje, itae=itf->second.v_ant.rend();
    itaje=itae; itaje--;
    ita=itf->second.v_ant.rbegin();
    while(ita!=itaje){
      v_a[1]=*ita;
      ita++;
      for(itai=ita; itai!=itae; ++itai){
	v_a[0]=*itai;
	for(unsigned int ns=0; ns<itf->second.nrepeat; ns++){
	  uvw_an(v_timeCentroid[k++], phaseDir, v_a);
	  for(unsigned int n=0; n<3; n++)
	    V_bl[n]= m_antUVW_.find(v_a[1])->second[n] - m_antUVW_.find(v_a[0])->second[n];
	  v_uvw.push_back(V_bl);
	}
      }
    }
  }else{
    vector<Tag>::const_iterator ita, itaj, itaie, itae=itf->second.v_ant.end();
    ita=itf->second.v_ant.begin(); itaie=itae; itaie--;
    while(ita!=itaie){
      v_a[0]=*ita;
      ita++;
      for(itaj=ita; itaj!=itae; ++itaj){
	v_a[1]=*itaj;
	for(unsigned int ns=0; ns<itf->second.nrepeat; ns++){
	  uvw_an(v_timeCentroid[k++], phaseDir, v_a);
	  for(unsigned int n=0; n<3; n++)
	    V_bl[n]= m_antUVW_.find(v_a[1])->second[n] - m_antUVW_.find(v_a[0])->second[n];
	  v_uvw.push_back(V_bl);
	}
      }
    }
  }
  if(correlationMode[CorrelationModeMod::CROSS_AND_AUTO] && autoTrailing){
    // the auto baselines
    vector<Vector<casa::Double> > vV_sd;     
    vV_sd.assign( itf->second.v_ant.size() * itf->second.nrepeat, 
		  sduvw_ );
    // append these auto to the cross baselines
    v_uvw.insert( v_uvw.end(), vV_sd.begin(),  vV_sd.end() );
  }
  return;
}

void UvwCoords::uvw_bl( asdm::MainRow* mainRow, vector<pair<unsigned int,double> > v_tci, 
			Enum<CorrelationMode> correlationMode,
 			pair<bool,bool> dataOrder, vector<Vector<casa::Double> >& v_uvw){

  vector<double> v_timeCentroid;
  for(unsigned int n=0;n<v_tci.size();n++)v_timeCentroid.push_back(v_tci[n].second);
  vector<Vector<casa::Double> > vV;
  uvw_bl( mainRow, v_timeCentroid, correlationMode, dataOrder, vV);
  v_uvw.clear(); v_uvw.resize(vV.size());
  //for(unsigned int n=0;n<v_tci.size();n++)v_uvw[v_tci[n].first]=vV[n++];
  for(unsigned int n=0;n<v_tci.size();n++)v_uvw[v_tci[n].first]=vV[n];
  return;
}


void UvwCoords::uvw_bl( asdm::MainRow* mainRow, vector<double> v_timeCentroid, Enum<CorrelationMode> correlationMode,
 			pair<bool,bool> dataOrder, vector<Vector<casa::Double> >& v_uvw){

  bool coutest=false;

  bool reverse      = dataOrder.first;
  bool autoTrailing = dataOrder.second;
  Tag                     configDescId = mainRow->getConfigDescriptionId();
  vector<vector<Angle> >  phaseDir     = mainRow->getFieldUsingFieldId()->getPhaseDir();
  map<Tag,ArrayParam>::const_iterator itf=m_array_.find(configDescId);

  v_uvw.clear();
  if(correlationMode[CorrelationModeMod::AUTO_ONLY]){
    if(itf->second.e_correlationMode[CorrelationModeMod::CROSS_ONLY])return;
  }else if(correlationMode[CorrelationModeMod::CROSS_ONLY]){
    if(itf->second.e_correlationMode[CorrelationModeMod::AUTO_ONLY])return;
  }else if(correlationMode[CorrelationModeMod::CROSS_AND_AUTO]){
    if(itf->second.e_correlationMode[CorrelationModeMod::CROSS_ONLY])
      correlationMode.set(CorrelationModeMod::CROSS_ONLY);
    else if(itf->second.e_correlationMode[CorrelationModeMod::AUTO_ONLY])
      correlationMode.set(CorrelationModeMod::AUTO_ONLY);
  }

  unsigned int nrepeat = itf->second.nrepeat;           
  unsigned int nant    = itf->second.v_ant.size();
  unsigned int nbl     = (nant*(nant-1))/2;
  unsigned int nmsrow  = 0;
  if(correlationMode[CorrelationModeMod::AUTO_ONLY])
    nmsrow=itf->second.v_ant.size();
  else
    nmsrow += nbl;
  if(correlationMode[CorrelationModeMod::CROSS_AND_AUTO])
    nmsrow += itf->second.v_ant.size();                 
  nmsrow *= itf->second.nrepeat;                        

  if(coutest){
    cout<<"Number of MS rows per dump: "<<nmsrow<<endl;
    cout<<"v_timeCentroid.size="<<v_timeCentroid.size()<<endl;
  }

  unsigned int ndump=v_timeCentroid.size()/nmsrow;
  if(v_timeCentroid.size()%nmsrow)
    cerr<<"ERROR v_timeCentroid.size="<<v_timeCentroid.size()<<endl;
  else
    if(coutest)cout<<"This SDM main row has "<<ndump<<" dumps"<<endl; 

  unsigned int offset=ndump*nant*nrepeat;
  if(correlationMode[CorrelationModeMod::CROSS_ONLY])offset=0;

  // Desole a propos de ces tests sur correlationMode! Raisonner par la negation est contre intuitif et
  // source d'erreurs. Dommage qu'un modele d'enumerations avec une vue ensembliste n'ai pas ete pris 
  // en consideration ...

  if(autoTrailing){

    if(!correlationMode[CorrelationModeMod::AUTO_ONLY]){                   // The cross data
      vector<Vector<casa::Double> > v_bluvw;
      for(unsigned int nt=0; nt<ndump; nt++){
	unsigned int cnt=0;
	for(unsigned int n=0; n<nbl*nrepeat; n++){
	  if(v_timeCentroid[offset+nt*nbl*nrepeat+n]==v_timeCentroid[offset+nt*nbl*nrepeat])cnt++;
	  ArrayTime arti(v_timeCentroid[offset+nt*nbl*nrepeat]);
	  //cout<<arti.toString()<<endl;
	}
	if(cnt==nbl*nrepeat){                 // duration baseline, spw and bin independent in this dump
	  uvw_bl( configDescId, 
		  phaseDir,
		  v_timeCentroid[offset+nt*nbl*nrepeat],
		  CorrelationModeMod::CROSS_ONLY,
		  reverse, autoTrailing, 
		  v_bluvw  );
	  v_uvw.insert( v_uvw.end(),v_bluvw.begin(), v_bluvw.end() );
	}else{
	  cout<<"cnt="<<cnt<<"   "<<v_timeCentroid[nt*nbl*nrepeat]<<endl;
	  cout<<"WARNING: baseline-based timeCentroid within a dump not yet tested"<<endl; 
	  uvw_bl( configDescId, 
		  phaseDir,
		  v_timeCentroid,
		  CorrelationModeMod::CROSS_ONLY,
		  reverse, autoTrailing, 
		  v_bluvw  );
	  v_uvw.insert( v_uvw.end(),v_bluvw.begin(), v_bluvw.end() ); 
	}
      }
    }
    if(!correlationMode[CorrelationModeMod::CROSS_ONLY]){                  // The single dish:
      vector<Vector<casa::Double> > v_sduvw;
      v_sduvw.assign( ndump*nant*nrepeat, sduvw_ ); 
      v_uvw.insert( v_uvw.end(),v_sduvw.begin(), v_sduvw.end() ); 
    }

  }else{                                                                  // the use-case currently implemented in SDMBinaryData

    if(!correlationMode[CorrelationModeMod::CROSS_ONLY])                   // The single dish:
      v_uvw.assign( ndump*nant*nrepeat, sduvw_ ); 

    if(!correlationMode[CorrelationModeMod::AUTO_ONLY]){                   // The cross data
      vector<Vector<casa::Double> > v_bluvw;
      for(unsigned int nt=0; nt<ndump; nt++){
	unsigned int cnt=0;                            //  cout<<"nt="<<nt<<" nbl="<<nbl<<" nrepeat="<<nrepeat<<endl;
	for(unsigned int n=0; n<nbl*nrepeat; n++){
	  if(v_timeCentroid[offset+nt*nbl*nrepeat+n]==v_timeCentroid[offset+nt*nbl*nrepeat])cnt++;
	}
	if(cnt==nbl*nrepeat){                 // duration baseline, spw and bin independent in this dump
	  uvw_bl( configDescId, 
		  phaseDir,
		  v_timeCentroid[offset+nt*nbl*nrepeat],
		  CorrelationModeMod::CROSS_ONLY,
		  reverse, autoTrailing, 
		  v_bluvw);
	  v_uvw.insert( v_uvw.end(),v_bluvw.begin(), v_bluvw.end() ); 
	}else{
	  cout<<"cnt="<<cnt<<"   "<<v_timeCentroid[nt*nbl*nrepeat]<<endl;
	  cout<<"WARNING: baseline-based timeCentroid within a dump not yet tested"<<endl; 
	  uvw_bl( configDescId, 
		  phaseDir,
		  v_timeCentroid,
		  CorrelationModeMod::CROSS_ONLY,
		  reverse, autoTrailing, 
		  v_bluvw);
	}
      }
    }

  }
  if (coutest) { 
    cout << "About to return v_uvw =" << endl;
    for (unsigned int i = 0; i < v_uvw.size(); i++)
      cout << v_uvw[i] << endl;
  }
  return;
}


vector<double> UvwCoords::antPos(const vector<double>& itrfPos,const vector<double>& hoffset){

  // passage du systeme de coords STATION au CS YOKE retire car devra etre integre dans Measure
  return itrfPos;
}


