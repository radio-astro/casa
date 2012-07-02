#ifndef IONOSPHERE_IONOSPHERE_H
#define IONOSPHERE_IONOSPHERE_H
    
#include <casa/aips.h>
#include <casa/Arrays/Vector.h>
#include <casa/Arrays/Matrix.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/LogiVector.h>
#include <casa/Containers/Block.h>
#include <casa/Containers/Record.h>
#include <casa/Quanta/MVEpoch.h>
#include <casa/Quanta/MVTime.h>
#include <measures/Measures/MEpoch.h>    
#include <measures/Measures/MDirection.h>    
#include <measures/Measures/MPosition.h>    
#include <casa/BasicSL/Constants.h>    
#include <casa/Logging/LogIO.h>    

namespace casa { //# NAMESPACE CASA - BEGIN

class Slant;
typedef Block<Slant> SlantSet;
// Helper function to sort slants into unique time slots
uInt sortSlants( Vector<uInt> &sidx,Vector<uInt> &suniq,const SlantSet &sl);

// -----------------------------------------------------------------------
// Slant
// <summary>
// Slant record
// </summary>
// The Slant class is a container for a {UT,Az,El,Lon,Lat}
// combo, defining a line-of-sight to (Az,El) from location (Lon,Lat)
// at a specific time slot UT.
// -----------------------------------------------------------------------
class Slant
{
  private:
      Double mjd_;          // time slot, MJD
      MVDirection dir_;     // direction of line-of-sight (AZEL)
      MVPosition  pos_;     // coordinates of observation site (ITRF)
      String      id_;        // ID 
      
  public:
      // default constructor
      Slant() : mjd_(0),dir_(),pos_(),id_() {}; 
  
      // data accessors
      Double       mjd () const          { return mjd_; };
      MVTime      time () const         { return MVTime(mjd_); }
      const MVDirection & dir () const   { return dir_; };
      const MVPosition  & pos () const   { return pos_; };
      const String & id() const          { return id_; };

      // returns data members as proper measures
      MEpoch      mep  () const  { return MEpoch(MVEpoch(mjd_)); }
      MPosition   mpos () const  { return MPosition(pos_,MPosition::Ref(MPosition::ITRF)); }
      MeasFrame   mdirframe () const 
          { return MeasFrame( mep(),mpos() ); }
      MDirection  mdir () const  
          { return MDirection(dir_,MDirection::Ref(MDirection::AZEL,mdirframe())); }
      
      // sets epoch (MJD), position (ITRF) and direction (AZEL)
      void set ( Double mjd,const MVDirection &dir,const MVPosition &pos,
                 const char *id_sta=NULL,const char *id_src=NULL );   
      
      // returns (Az,El) vector, default units are radians
      Vector <Double> azEl ( Bool degrees=False ) const
        { return dir_.get()/(degrees?C::degree:1); }
      // returns (Az,El) vector in degrees
      Vector <Double> azElDeg () const
        { return azEl(True); }
      
      // returns (Lon,Lat) vector, default units are radians
      Vector <Double> lonLat ( Bool degrees=False ) const
        { return pos_.getAngle().getBaseValue()/(degrees?C::degree:1); }
      // returns (Lon,Lat) vector in degrees
      Vector <Double> lonLatDeg () const
        { return lonLat(True); }
      
      // returns slant as a string (for printing)
      String     string () const;
  
// sorting function declared as friend -- must have access      
  friend uInt sortSlants( Vector<uInt> &sidx,Vector<uInt> &suniq,const SlantSet &sl);
};

typedef Block<Slant> SlantSet;
// Helper function to sort slants into unique time slots
uInt sortSlants( Vector<uInt> &sidx,Vector<uInt> &suniq,const SlantSet &sl);

// -----------------------------------------------------------------------
// EDProfile
// <summary>
// Ionospheric electron density profile.
// </summary>
// EDProfile contains a measurement or a prediction for an ED profile 
// along a particular slant.
// The ed vector holds values of ED, sampled at corresponding 
// altitudes (alt), ranges (rng) and coordinates (lat,lon). Note that any
// three out of four coordinates {alt,rng,lat,lon} are redundant (since a 
// slant is one-dimensional), but it is very useful in computations to 
// have all four available.
// -----------------------------------------------------------------------

class EDProfile
{
  private:
    Slant        sl;
    Vector<Float> ed_,alt_,lat_,lon_,rng_;
    Vector<Float> bpar_;
  
  public:
    EDProfile ()  {};
    ~EDProfile () {};  

// resizes internal vectors
    void resize( uInt n )
        { ed_.resize(n); alt_.resize(n); lat_.resize(n); lon_.resize(n); rng_.resize(n); 
          bpar_.resize(0);
        }

    uInt nelements ()                          { return ed_.nelements(); }
    
// accessors for the slant memeber
    const Slant & slant () const                { return sl; };
    void set_slant      ( const Slant &other )  { sl=other; bpar_.resize(0); };
    
// accessors for the other members.
// Each data member (ed,alt,lat,lon,rng) has the following accessor forms:
// ed()         returns the whole vector (by const reference)
// ed(i)        returns value of element i
// set_ed(ed1)  assigns the whole vector
// ed_storage() returns a 'Float *' to the member's storage
#define ACCESSOR(memb) \
    const Vector<Float> & memb () const         { return memb##_; }; \
    Float memb (uInt i) const                   { return memb##_(i); } \
    void set_##memb ( const Vector<Float> memb) { memb##_ = memb; }; \
    Float * memb##_storage ()                   { Bool dum; return memb##_.getStorage(dum); } \
    const Float * memb##_storage () const       { Bool dum; return memb##_.getStorage(dum); }
    ACCESSOR(ed);    
    ACCESSOR(alt);    
    ACCESSOR(lat);    
    ACCESSOR(lon);    
    ACCESSOR(rng);    

// computes (if not already computed) the magnetic field along the line
// of sight. Returns the line-of-sight intensity (in Gauss) at each defined 
// altitude point.
    const Vector<Float> & getLOSField() const; 
    
// copies data members directly from other EDProfile
    void copyData( const EDProfile &other );
    
// The TEC function determines the total electron content for this profile.
// For now, it's a simple sum.
// We really should do some sophisticated integration here...
    Double tec () const { return sum(ed_); }

// Computes the TEC and the Faraday Rotation for this profile.
// Uses a more sophisticated integration technique
    Double getTecRot     (Double &tec) const;
};

// Some forward declarations 
class IonosphModel;
class IonosphData;    
    
// -----------------------------------------------------------------------
// Ionosphere
// <summary>
// Ionospheric modelling and prediction
// </summary>
// -----------------------------------------------------------------------
class Ionosphere
{
  protected:
    SlantSet targ_sl,slants;
    PtrBlock <IonosphData*> rtd;
    IonosphModel *model;
    Block <EDProfile> edp;
      
  public:
    static uInt   debug_level;
    static LogIO  os;
      
    Ionosphere  ( IonosphModel *mod=NULL );
    ~Ionosphere () {};
    
    void setRTA   ();
    void setModel ( IonosphModel *mod ) { model=mod; }
    void addData  ( IonosphData *data );

// setModelOptions() and getModelOptions() map to IonosphModel::get/setOptions.
    Record getModelOptDesc () const;
    Record getModelOptions () const;
    void    setModelOptions ( const Record &opt );
// define quick shortcuts for setting model options of the most common types
    #define setModelOpt(Type) \
        void setModelOption ( const String &name,const Type &value ) {\
           if( !model ) return;\
           Record opt(RecordInterface::Fixed);\
           opt.define(name,value);\
           setModelOptions(opt);\
        }
    setModelOpt(Bool);
    setModelOpt(Int);
    setModelOpt(Double);
    setModelOpt(String);
    
// setTargetSlant() sets up the target slants. When called with no arguments,
// clears the set.
    uInt setTargetSlants ( const Slant *sl=NULL,uInt count=0 );
    uInt setTargetSlants ( const SlantSet &set )
            { return setTargetSlants(set.storage(),set.nelements()); }
// addTargetSlant() adds one or more slants to the target slants.
// Main version takes a simple pointer; two "aliases" are provided for 
// convenience.    
    uInt addTargetSlants ( const Slant *sl,uInt count=1 );
    uInt addTargetSlant  ( const Slant &sl )     
            { return addTargetSlants(&sl,1); }
    uInt addTargetSlants ( const SlantSet &set ) 
            { return addTargetSlants(set.storage(),set.nelements()); }
    
// computes ED profiles for "interesting" slants. Returns a const
// reference to the internal ED profile object, so don't do anything to it!
    const Block<EDProfile> & compute (LogicalVector &isUniq);

// computes the TEC (int TECU) and rotation measure (in RMI), given a set 
// of ED profiles
    void getTecRot ( Vector<Double> &tec,Vector<Double> &rmi,const Block<EDProfile> &ed );
};

// -----------------------------------------------------------------------
// IonosphModel
// <summary>
// Abstract base class for apriori ionospheric models
// </summary>
// -----------------------------------------------------------------------
class IonosphModel
{
  protected:
    Vector <Float>  altgrid;
    Record     options,option_desc;
  
  public:
// Constructor. Sets up the model.
    IonosphModel () {};
    virtual ~IonosphModel() {}

// Sets up the altitude sampling grid      
    virtual void setAlt( const Float *alt,uInt nalt) 
      { altgrid.takeStorage(IPosition(1,nalt),alt); }
    
    virtual void setAlt( const Vector<Float> &alt )
    {
      Bool deleteIt;
      const Float *a=alt.getStorage(deleteIt);
      setAlt(a,alt.nelements()); 
      alt.freeStorage(a,deleteIt);
    }
    
// Returns a "descriptor" record of available options. 
    virtual Record getOptDesc() const        { return option_desc; };
// Returns currently active model options
    virtual Record getOptions() const        { return options; };
// Sets up model options. 
    virtual void setOptions( const Record &opt )  { options.merge(opt,RecordInterface::OverwriteDuplicates); };
    
// Performs the actual model computations for a number of slants, and 
// returns the corresponding ED profile estimates.
// Slants should be sorted by time, into unique time slots, with sidx and
// suniq containing the sorted and unique index vectors.
    virtual Block<EDProfile> getED ( LogicalVector &isUniq, 
                                    const SlantSet &sl_set,
                                    const Vector<uInt> &sidx,
                                    const Vector<uInt> &suniq )=0;

// Version of getED for an unsorted time slot array. Default implementation
// does the sort, then calls getED() above.
    virtual Block<EDProfile> getED ( LogicalVector &isUniq,const SlantSet &sl_set ) 
    {
      Vector<uInt> sidx,suniq;
      sortSlants(sidx,suniq,sl_set);
      return getED(isUniq,sl_set,sidx,suniq);
    };
    
};

// -----------------------------------------------------------------------
// IonosphData
// <summary>
// Abstract base class for a real-time dataset (RTD)
// </summary>
// -----------------------------------------------------------------------
class IonosphData
{
  protected:
    SlantSet dsl;
      
  public:
    IonosphData () {};
    virtual ~IonosphData () {};  
      
// returns a referece to the "interesting" slant set. 
    virtual const SlantSet& slants () { return dsl; };
  
// returns True if this RTD class is capable of fitting TECs
    virtual Bool fitsTEC     () { return False; }
    
// returns True if this RTD class is capable of fitting ED profiles directly
    virtual Bool fitsProfile () { return False; }

// returns weight of dataset
    virtual Float getWeight ()  { return 1.; }
    
// returns "degree of contraint" of dataset
    virtual Float getDegree ()  { return 1.; }
    
// TEC fitting.
// Given a TEC value along slant number isl, and a vector
// JT(j)=dTEC/dP(j) (where P(j) is the j-th parameter), computes chi-square 
// for this slant, plus a vector of derivatives JChi(j)=dChiSq/dP(j).
    virtual Float chiSquare (Vector<Float> &JChi,
                            int isl,Float TEC,
                            const Vector<Float> &JT) =0;

// Profile fitting.
// Given an ED profile (edp) along slant number isl, and
// a matrix JD(i,j)=dED(i)/dP(j) (where ED(i) is the i-th point on the
// ED profile, and P(j) is the j-th parameter), computes chi-square 
// for this slant, plus a vector of derivatives JChi(j)=dChiSq/dP(j).
    virtual Float chiSquare (Vector<Float> &JChi,int isl,const EDProfile &edp,
                            const Matrix<Float> &JD) =0;
};


inline Record Ionosphere::getModelOptDesc () const
{
  return model ? model->getOptDesc() : Record();
}

inline Record Ionosphere::getModelOptions () const
{
  return model ? model->getOptions() : Record();
}

inline void Ionosphere::setModelOptions ( const Record &opt ) 
{
  if( model )
    model->setOptions(opt);
}


// Helper function to convert a null-terminated C string into a
// blank-padded FORTRAN character array
void fortran_setstr(char *fstr,size_t fstr_size,const char *cstr);

// Macro to assign a string to a fixed-size array
#define fsetstr(fstr,cstr) fortran_setstr(fstr,sizeof(fstr),cstr)


} //# NAMESPACE CASA - END

#endif
