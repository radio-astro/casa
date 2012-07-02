#include <ionosphere/Ionosphere/Ionosphere.h>
#include <casa/Exceptions.h>    
#include <scimath/Mathematics.h>    
#include <casa/Utilities/Sort.h>    
#include <casa/Utilities/Copy.h>    
#include <casa/System/Aipsrc.h> 
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MeasFrame.h>
#include <measures/Measures/EarthMagneticMachine.h>
#include <casa/Arrays/Slice.h>

#include <casa/stdio.h>
    
namespace casa { //# NAMESPACE CASA - BEGIN

// define to 1 to use PIM IGRF calls instead of Measures
#define USE_PIM_IGRF 0
        
uInt Ionosphere::debug_level=0;

// initializie static log sink
 LogIO Ionosphere::os(LogOrigin("Ionosphere")); 


// -----------------------------------------------------------------------
// Slant::set
// Sets up a slant
// -----------------------------------------------------------------------
void Slant::set ( Double mjd1,const MVDirection &dir1,const MVPosition &pos1,
                  const char *id_sta, const char *id_src )
{
// check that pos1 is truly in ITRF
  if( pos1.getLength().getValue("km") < 5000 )
  {
    Ionosphere::os<<"Slant position must be in ITRF frame\n"
                    "the offending position is: ";
    Ionosphere::os.output()<<pos1;
    Ionosphere::os<<LogIO::EXCEPTION;
  }
  mjd_=mjd1;
  pos_=pos1;
  dir_=dir1;
//  cerr<<"Slant mjd: "<<mjd<<"; pos: "<<pos.getAngle()<<"; dir: "<<dir.getAngle()<<endl;
  if( id_sta )
  {
    id_ = id_sta;
    if( id_src )
      id_ +=id_src;
  }
  else
    id_ = String();
}

// -----------------------------------------------------------------------
// Slant::string
// Returns slant as a string for printing
// -----------------------------------------------------------------------
String Slant::string () const
{
  static char s[256];
  Vector<Double> azel( azElDeg() ),lonlat( lonLatDeg() );
  sprintf(s,"[%s; %.1f %.1f / %.1f %.1f]",
      MVTime(mjd_).string(MVTime::YMD+MVTime::CLEAN,4).chars(),
      lonlat(0),lonlat(1),azel(0),azel(1));
  return String(s);
}

    
// -----------------------------------------------------------------------
// sortSlants
// Helper function to sort a slant set into unique time slots.
// Returns the sort indices and the unique indices in sidx and suniq.
// Returns the number of unique time slots
// -----------------------------------------------------------------------
uInt sortSlants( Vector<uInt> &sidx,Vector<uInt> &suniq,const SlantSet &sl_set)
{
  const Slant *sl=sl_set.storage();
  Sort sort(sl,sizeof(Slant));
  sort.sortKey((char*)&sl[0].mjd_-(char*)&sl[0],ObjCompare<Double>::compare);
  sort.sort(sidx,sl_set.nelements());
  return sort.unique(suniq,sidx);
}


// -----------------------------------------------------------------------
// FORTRAN declarations 
// I still call TECCLC for integration... gotta phase this out.
// -----------------------------------------------------------------------
extern "C" 
{
#if( USE_PIM_IGRF )
  struct { char pigrf[80]; } igrfwrapper_;
  void figrfload_(const float *year);
#endif
  void feldg_(const float*,const float*,const float*,float*,float*,float*,float*);
  void tecclc_(const int*,const float *,const float *,float *);
}

// -----------------------------------------------------------------------
// fortran_setstr
// Helper function to convert a null-terminated C string into a
// blank-padded FORTRAN character array
// -----------------------------------------------------------------------
void fortran_setstr(char *fstr,size_t fstr_size,const char *cstr)
{
  strncpy(fstr,cstr,fstr_size);
  for( size_t i=strlen(cstr); i<fstr_size; i++ )
    fstr[i]=' ';
}
  

// -----------------------------------------------------------------------
// Ionosphere constructor
// -----------------------------------------------------------------------
Ionosphere::Ionosphere ( IonosphModel *mod ) 
{
  model=NULL;
#if( USE_PIM_IGRF )
// get the IGRF path. Default is $AIPSROOT/$AIPSARCH/IGRFDB
  String igrf_path;
  if( !Aipsrc::find(igrf_path,"ionosphere.igrf.path") )
  {
    igrf_path+=Aipsrc::aipsArch();
    igrf_path+="/IGRFDB";
  }
  cerr<<"Ionosphere: IGRF database path is "<<igrf_path<<endl;
  igrf_path+="/";
  fsetstr(igrfwrapper_.pigrf,igrf_path.chars());
#endif
// set the apriori model, if available
  if( mod )
    setModel(mod);
}

// -----------------------------------------------------------------------
// setTargetSlants
// Set the target slants. Use (NULL,0) to clear the set
// -----------------------------------------------------------------------
uInt Ionosphere::setTargetSlants ( const Slant *sl,uInt count )
{
//  cerr<<"targ_sl old size: "<<targ_sl.nelements()<<endl;
//  cerr<<"targ_sl resizing to : "<<count<<endl;
  targ_sl.resize(count,True);
//  cerr<<"targ_sl new size: "<<targ_sl.nelements()<<endl;
  if( count ) // set
    objmove(targ_sl.storage(),sl,count);   // copy over the new slants
  return count;
}

// -----------------------------------------------------------------------
// addTargetSlants
// Adds 'count' slants to the target slants.
// -----------------------------------------------------------------------
uInt Ionosphere::addTargetSlants ( const Slant *sl,uInt count )
{
  int n=targ_sl.nelements();
  targ_sl.resize(n+count);                // resize the internal block 
  objmove(targ_sl.storage()+n,sl,count);  // copy over the new slants
  return n+count;
}

// -----------------------------------------------------------------------
// addData
// Adds a real-time data constraint
// -----------------------------------------------------------------------
void Ionosphere::addData ( IonosphData *data )
{
  int n=rtd.nelements();
  rtd.resize(n+1);
  rtd[n]=data;
}
    
// -----------------------------------------------------------------------
// compute ()
// Performs ionospheric computations and RTA fitting
// -----------------------------------------------------------------------
const Block<EDProfile> & Ionosphere::compute (LogicalVector &isUniq)
{
  if(!model)
    os<<"apriori model not set up"<<LogIO::EXCEPTION;
  
// output slants
//   os<<"compute() slants:\n";
//   for( uInt i=0; i<targ_sl.nelements(); i++ )
//   os<<i<<": "<<targ_sl[i].string()<<endl;
//   os.post();
  
// count total number of slants to be computed
  uInt ntarg=targ_sl.nelements(),num_sl=ntarg;
  for( uInt i=0; i<rtd.nelements(); i++ )
    num_sl+=rtd[i]->slants().nelements();
  
  if(!num_sl)
    os<<"no slants to compute"<<LogIO::EXCEPTION;

  os<<"Computing ionospheric profiles for "<<num_sl<<" slants"<<LogIO::POST;
  
// build up full slant set
  uInt idx=0;
  SlantSet slants(num_sl);
// slref will be used to point back at the RTD object respondible for this
// slant
  Vector<uInt> slref(num_sl); 
  Slant *sl=slants.storage();

// first put in the target slants
  if( ntarg )
  {
    objmove(sl,targ_sl.storage(),ntarg); // copy target slants
    slref(Slice(0,ntarg))=-1;         // -1: a target slant
    idx+=ntarg;
  }
// now add the RTD slants
  for( uInt i=0; i<rtd.nelements(); i++ )
  {
    const SlantSet &s=rtd[i]->slants();
    uInt n=s.nelements();
    objmove(sl+idx,s.storage(),n);
    slref(Slice(idx,n))=i;            // set the back-reference to this RTD object
    idx+=n;
  }
// sort the slants by increasing time slot
  Vector <uInt> sidx,suniq;
  sortSlants(sidx,suniq,slants);

#if( USE_PIM_IGRF )
// finally, initialize IGRF stuff (Earth magnetic field)
  // obtain a date smack in the middle of the obs. set (like Bob does it...)
  MVTime tm( slants[sidx(num_sl/2)].time() );  
  // convert the date into fractional years, as the IGRF routines expect...
  Int yr=tm.year();
  Float year=yr+tm.yearday()/( (yr%4)==0 ? 366.0 : 365.0 );

  // call the wrapper...
  figrfload_(&year);
#endif
  
// compute model ED profiles
  edp=model->getED(isUniq,slants,sidx,suniq);
  
  return edp;
}
    
// -----------------------------------------------------------------------
// getTecRot ()
// This routine computes the FR for the target slants.
// Note that the EDPs for the target slants are from edp(0)
// to edp(n-1), provided compute() has already been called.
// Based on Bob Campbell's TECFR subroutine (see tecvlbi4.f).
// -----------------------------------------------------------------------
void Ionosphere::getTecRot ( Vector<Double> &tec,Vector<Double> &rot,const Block<EDProfile> &ed )
{
// check that we have computed ED profiles, and initialize the result vector
  uInt n=ed.nelements();
  rot.resize(n);
  tec.resize(n);

  for( uInt i=0; i<n; i++ ) // loop over target slants 
    rot(i)=ed[i].getTecRot(tec(i));
}


// -----------------------------------------------------------------------
// EDProfile::copyData
// Copies data members from other profile
// -----------------------------------------------------------------------
void EDProfile::copyData ( const EDProfile &other )
{
  ed_=other.ed();
  alt_=other.alt();
  lon_=other.lon();
  lat_=other.lat();
  rng_=other.rng();
}

// -----------------------------------------------------------------------
// EDProfile::getLOSField
// Computes (if not already computed) the magnetic field along the line
// of sight. Returns the line-of-sight intensity (in Gauss) at each defined 
// altitude point.
// -----------------------------------------------------------------------
const Vector<Float> & EDProfile::getLOSField () const
{
  if( !bpar_.nelements() ) // does field need to be computed at all?
  {
  // Blatant const violation here. But that's OK since we know
  // what we're doing. We want the field to be computed only on demand, and 
  // only once. For a const object, this is impossible without violating
  // const here.
    Vector<Float> *bp = (Vector<Float>*) &bpar_;
    uInt n=alt_.nelements();
    bp->resize(n);
  // set up machine to calculate the magnetic field along LOS
    MDirection dir( sl.mdir() );
    EarthMagneticMachine emm(dir.getRef(),dir.getValue(),dir.getRef().getFrame());
  // compute the field  
    for( uInt i=0; i<n; i++ )
      (*bp)(i) = emm.getLOSField(Quantity(alt_(i),"km"),"G").getValue("G");
  }
  return bpar_;
}

#if( !USE_PIM_IGRF )
// -----------------------------------------------------------------------
// EDProfile::getTecRot
// Computes the TEC and Faraday rotation along this slant.
// This implemetation uses EarthMagneticMachine.
// Kludged together based on Bob Campbell's TECFR subroutine (see tecvlbi4.f),
// but using vector arithmetic.
// -----------------------------------------------------------------------
Double EDProfile::getTecRot ( Double &tec_out ) const
{
  uInt n=ed_.nelements();
// C  Now we have ED(k), BPAR(k), BPERP(k):  only integrating left
// C  For now, stick with only the linear TEC term for delay and
// C    the 1st order "Faraday rotation" term -- higher orders can
// C    be constructed from ED, BPAR, & BPERP.  If frequency also
// C    passed, Delay & Physical Faraday rotation can be returned
// C    to whatever order. 
// C  For the future, we can look into better integration schemes
// C    than that in regular PIM (QROMB, TECCLC), but for now, we'll
// C    stick with that, including the kludgy ROTMIN addition to the
// C    Faraday rotation calculation (because of the logrithmic
// C    interpolation in TECCLC)
// C    
// C  TEC finally in TECU  [10^{16}/m^2]
// C
  Float tec;
  tecclc_((int*)&n,rng_storage(),ed_storage(),&tec);
  tec_out = 1e-12*(Double)tec;
// 
// C  "Rotation measure" finally in RMU  [10^{12} T/m^2]
// C
  Float rmi=0.,rotmin=0.;
  Vector<Float> rotmeas( ed_*getLOSField() );
// C  ROTMIN (<0) is the minimum ROTMEAS, to be subtracted from all
// C    ROTMEAS(k) before passing to TECCLC (to ensure all positive)
// C    Subtract this extra area (\Delta RNG * ROTMIN) from the 
// C    returned RMI
// C
  rotmin = min(rotmeas);
  if( rotmin<0 )
    rotmeas -= (rotmin-=1.0);

  //*** call FORTRAN procedure to integrate... oh, the shame of it
  Bool dum;
  const Float *rmeas=rotmeas.getStorage(dum);
  tecclc_((int*)&n,rng_storage(),rmeas,&rmi);
  Double rmi_out=rmi;

  if( rotmin<0 )
    rmi_out += rotmin * abs(rng(n-1)-rng(0)) * 1.e5;
// use the Bob Campbell constant to convert RM into degrees/GHz^2
  rmi_out *= 1.e-12*0.677458;

  return rmi_out;
}
#else
// This implemetation uses the PIM IGRF code integrated by Bob Campbell
Double EDProfile::getTecRot ( Double &tec_out ) const
{
  uInt n=ed_.nelements();
  Vector<Double> azel=sl.azEl();
  Vector<Double> lonlat=sl.lonLat();
//  cerr<<"lon:"<<lonlat(0)<<" lat:"<<lonlat(1)<<"\n";
// LOS[xyz]: direction cosines of propagation in ECEF frame  (-LoS)
  Double sin_el=sin(azel(1)),cos_el=cos(azel(1)),
         sin_az=sin(azel(0)),cos_az=cos(azel(0)),
         sin_lon=sin(lonlat(0)),cos_lon=cos(lonlat(0)),
         sin_lat=sin(lonlat(1)),cos_lat=cos(lonlat(1));
  Float  losx = sin_el*cos_lat*cos_lon - cos_el*sin_az*sin_lon -
                cos_el*cos_az*sin_lat*cos_lon,
         losy = sin_el*cos_lat*sin_lon + cos_el*sin_az*cos_lon -
                cos_el*cos_az*sin_lat*sin_lon,
         losz = sin_el*sin_lat + cos_el*cos_az*cos_lat;
  losx = -losx;
  losy = -losy;
  losz = -losz;

// Kludge alert! Use PIM's subroutine to compute EMF component vectors at each sample
  Vector<Float> bnorth(n),beast(n),bdown(n),babs(n);
  for( uInt i=0; i<n; i++ )
    feldg_(&lat_(i),&lon_(i),&alt_(i),&bnorth(i),&beast(i),&bdown(i),&babs(i));
  
// B[xyz]   - ECEF components of Earth's magnetic field  (IGRF)
  Vector<Float> bx,by,bz,lonr,latr;
  lonr=lon_*(Float)C::degree;
  latr=lat_*(Float)C::degree;
  Vector<Float> c_lon=cos(lonr),s_lon=sin(lonr),
                c_lat=cos(latr),s_lat=sin(latr);
  bx = - (beast*s_lon + bnorth*s_lat*c_lon + bdown*c_lat*c_lon);
  by = beast*c_lon + bnorth*s_lat*s_lon - bdown*c_lat*s_lon;
  bz = bnorth*c_lat - bdown*s_lat;

//  BPAR     - Component of B parallel to propagation at each LoS sample
//  BPERP    - Component of B perpendicular to prop at each LoS sample
  Vector<Float> bpar,bperp;
  bpar = losx*bx + losy*by + losz*bz;
  bperp = sqrt(square(babs) - square(bpar));
  // Blatant const violation here. But that's OK since we know
  // what we're doing. We want the field to be computed and stored inside!
  *((Vector<Float> *)&bpar_) = bpar;
//  cerr<<"Bpar: "<<bpar<<endl;
  
// C  Now we have ED(k), BPAR(k), BPERP(k):  only integrating left
// C  For now, stick with only the linear TEC term for delay and
// C    the 1st order "Faraday rotation" term -- higher orders can
// C    be constructed from ED, BPAR, & BPERP.  If frequency also
// C    passed, Delay & Physical Faraday rotation can be returned
// C    to whatever order. 
// C  For the future, we can look into better integration schemes
// C    than that in regular PIM (QROMB, TECCLC), but for now, we'll
// C    stick with that, including the kludgy ROTMIN addition to the
// C    Faraday rotation calculation (because of the logrithmic
// C    interpolation in TECCLC)
// C    
// C  TEC finally in TECU  [10^{16}/m^2]
// C
  Float tec;
  tecclc_((int*)&n,rng_storage(),ed_storage(),&tec);
  tec_out = 1e-12*(Double)tec;
// 
// C  "Rotation measure" finally in RMU  [10^{12} T/m^2]
// C
  Float rmi=0.,rotmin=0.;
  Vector<Float> rotmeas;

// C  ROTMIN (<0) is the minimum ROTMEAS, to be subtracted from all
// C    ROTMEAS(k) before passing to TECCLC (to ensure all positive)
// C    Subtract this extra area (\Delta RNG * ROTMIN) from the 
// C    returned RMI
// C
  rotmeas = ed_*bpar;
  rotmin = min(rotmeas);
  if( rotmin<0 )
    rotmeas -= (rotmin-=1.0);

  //*** call FORTRAN procedure to integrate... oh, the shame of it
  Bool del_rmeas;
  const Float *rmeas=rotmeas.getStorage(del_rmeas);
  tecclc_((int*)&n,rng_storage(),rmeas,&rmi);
  Double rmi_out=rmi;

  if( rotmin<0 )
    rmi_out += rotmin * abs(rng(n-1)-rng(0)) * 1.e5;
// use the Bob Campbell constant to convert RM into degrees/GHz^2
  rmi_out *= 1.e-12*0.677458;

  return rmi_out;
}
#endif

} //# NAMESPACE CASA - END

