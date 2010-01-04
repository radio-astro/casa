#include <vector>
#include <map>
using namespace std;

#include "Enum.hpp"
#include "CorrelationMode.h"

#include <measures/Measures.h>
#include <measures/Measures/MEpoch.h>
#include <measures/Measures/MPosition.h>
#include <measures/Measures/MDirection.h>
#include <measures/Measures/MCBaseline.h>
#include <measures/Measures/MBaseline.h>
#include <measures/Measures/MCuvw.h>
#include <measures/Measures/Muvw.h>
#include <casa/Quanta.h>

using namespace casa;

#include "ASDMEntities.h"
using namespace asdm;

/** @brief SDM UVW engine
    This UvwCoords class is an engine to determine the UVW coordinates at any time for
    any antenna (sub)array. It is based on the Measure CASA and ASDM classes. It 
    requires as input the MS timeCentroids item which is an output in the ASDM DAMs.
    @todo 
    - the use-case when the timeCentroid is baseline-based and or spectral window based
    - correlationMode filter
    @note The current limitations are set by the status of the ASDM
    - the use-case when the phase direction is not assumed to be J2000
    - OTF not supported due to a limitation in the model of the phase direction item
      in the field table.
*/
class UvwCoords{
public:

  /** Constructor 
      @param datasetPtr The SDM dataset
      @post Data reduction done for static informations at the dataset level:
      - The positions for all the antennas in the Antenna table have been determined by applying
      the chain of offsets from the IERS station positions
      - All the (sub-)arrays present in the ConfigDescription table have been identified
  */
  UvwCoords(ASDM* const datasetPtr);

  /** Destructor */
  ~UvwCoords();

  /** Determine the baseline-based uvw in case timeCentroid is baseline-based, spw-based and bin-based independent within a dump
      @param configDescriptionId ConigDescription identifier
      @param phaseDir            Phase direction assumed to be J2000
      @param timeCentroid        Time centroid (unit second) assumed to be MJD TAI
      @param correlationMode     Correlation mode defined by the client (a use-query) to filter out data.
      @param reverse             Parameter characterizing the order of the resulting baselines. 
      @param autoTrailing        True if the cross baselines come first, the zero baselines trailing in the output v_uvw sequence
      @param v_uvw               The returned UVW coordinates for all the pair of antenna and, for 
                                 each pair, for all the spectral windows and, for each spectral window,
                                 for each bin. 
      @warning 
      - With this method the approximation is made that all these baselines share the same time centroid. 
      This approximation which should be adequate in most of the use-cases increases performances in term
      of time response.
      - This method will destroy the previous content of v_uvw.
      @note    The implementation is optimized to avoid to re-compute the uvw when the same antenna 
               (sub-)array is reused for the same timeCentroid and phaseDir in consecutive updates. 
               In practice this means, for ALMA, that when there is one sub-integration per integration
               the UVW are not determined twice.
  */
  void uvw_bl( Tag configDescriptionId, 
	       const vector<vector<Angle> >& phaseDir,
	       double timeCentroid,
	       Enum<CorrelationMode> correlationMode,
	       bool reverse, bool autoTrailing,
	       vector<Vector<casa::Double> >& v_uvw );

  /** Determine the baseline-based uvw in case timeCentroid may change vs  baseline, spw or bin within a dump
      @param configDescriptionId ConigDescription identifier
      @param phaseDir            Phase direction assumed to be J2000
      @param v_timeCentroid      Sequence of time centroids (unit second) assumed to be MJD TAI and to map the 
                                 structure parametrized by the attributes reverse and autoTrailing
      @param correlationMode     Correlation mode defined by the client (a use-query) to filter out data.
      @param reverse             Parameter characterizing the order of the resulting baselines. 
      @param autoTrailing        True if the cross baselines come first, the zero baselines trailing in the output v_uvw sequence
      @param v_uvw               The returned UVW coordinates for all the pair of antenna and, for 
                                 each pair, for all the spectral windows and, for each spectral window,
                                 for each bin. 
      @warning 
      - With this method every baseline,spectral window,bin triplet have its own time centroid. 
      Because this generality is expensive in term of computation, this method must be avoided when the time centroid 
      does not change vs  baseline, spw or bin within a dump.
      - This method will destroy the previous content of v_uvw.
  */
  void uvw_bl( Tag configDescriptionId, 
	       const vector<vector<Angle> >& phaseDir,
	       const vector<double>& v_timeCentroid,
	       Enum<CorrelationMode> correlationMode,
	       bool reverse, bool autoTrailing, 
	       vector<Vector<casa::Double> >& v_uvw);


  /** Determine the baseline-based uvw for a sequence of epochs
      @param mainRow         Pointer to a SDM main table row
      @param v_timeCentroid  Time centroid for every (sub)integration, baseline, spectral window and bin
      @param correlationMode Correlation mode defined by the client (a use-query) to filter out data. 
      @param dataOrder       A pair of booleans characterizing the order of the time centroids
                             meta-data in the sequence v_timeCentroid. The first member
                             of this pair is for the order of the baselines and the second to tell
			     if the cross-data come first or not relative to the auto data. This 
			     dataOrder attribute is available via a static method of the class SDMBinData.
      @param v_uvw           The returned sequence UVW coordinates for all the pair of antenna and, for 
                             each pair, for all the spectral windows and, for each spectral window,
                             for each bin. Hence the returned size will be equal to the size of the 
                             input sequence v_timeCentroid.
      @warning This function will destroy the previous content of v_uvw.
      @note 
      - correlationMode must be identical to the one used with the DAMs. 
      - v_timeCentroid is a product of the DAMs. Its size is the number of MS rows per SDM row in the Main tables
      - for dataOrder (reverse and autoTrailing) it is available using a static method of the DAMs (SDMBinData).
  */
  void uvw_bl( asdm::MainRow* mainRow, vector<double> v_timeCentroid, 
	       Enum<CorrelationMode> correlationMode,
	       pair<bool,bool> dataOrder,
	       vector<casa::Vector<casa::Double> >& v_uvw );

private:

  /** (Sub-)array parameters constraining order and size of the output vector 
      of 3D UVW coords triplets.
  */
  struct ArrayParam{
  public:
    Tag                   subarrayId;        //<! (sub)array identifier
    vector<Tag>           v_ant;             //<! sequence of antennas
    unsigned int          nrepeat;           //<! number of MS main table rows per antenna baseline
    Enum<CorrelationMode> e_correlationMode; //<! correlation mode (original mode passed through the user filter)
    /** Concept Equality Comparable: 
	@note Two subarrays are equals if they correspond to the same set (in mathematical sense)
	of antennas
     */
    bool operator== ( ArrayParam & rhs){
      set<Tag> a, b;
      for(unsigned int na=0; na<v_ant.size(); na++){
	a.insert(v_ant[na]);
	b.insert(rhs.v_ant[na]);
      }
      if( a != b ) return false;
      return true;
    }
    string show(){
      ostringstream os;
      for(unsigned int n=0; n<v_ant.size(); n++)os << v_ant[n].toString() << " ";
      os << " nrepeat " << nrepeat;
      return os.str();
    }
  };

  map<Tag,ArrayParam>            m_array_;
  map<Tag,MPosition>             m_antPos_;
  map<Tag,Vector<casa::Double> > m_antUVW_;

  // the 3 fundamental attributes of the state machine
  double                         timeCentroid_;
  vector<vector<Angle> >         phaseDir_; 
  Tag                            subarrayId_;

  Vector<casa::Double>           sduvw_;

  /** Determine the position of the phase reference point in an antenna in IERS by applying
      the chain of offsets up to the station position which is defined in the IERS.
      @todo Give the definition of these offsets and apply them.
  */
  vector<double> antPos(const vector<double>& stationPos, const vector<double>& antOffset);

  /** Determine antenna-based uvw for every member of the sequence of antennas defining a (sub)array
      @param  timeCentroid An epoch, the 'when' characterization.
      @param  phaseDir     The (sub)array phase direction
      @param  v_antId      The sequence of antenna identifiers defining the (sub)array
      @note 
      - this method is temporary given the fact that the SDM is waiting for its version 2 to implement
      adequately not only timeCentroid but also phaseDir (will be required for OTF)
      @warning The current implementation assumes that 
      - timeCentroid is MJD in TAI, the unit in second. Futhermore the accuracy is limited because there is no 
      extra precision attribute (see Main table of MS v2)
      - phaseDir is in J2000, the unit of the coordinates in radian
      @todo remove this note and all these warnings (to be coordinated with the release of SDM vers. 2)
  */ 
  void uvw_an(double timeCentroid, const vector<vector<Angle> >& phaseDir, const vector<Tag>& v_antId);

  /** Determine the baseline-based uvw for a sequence of antenna defining a (sub)array
      @param v_antennaId The sequence of antenna identifiers defining the (sub)array
      @param nrep        The number of repeats per baseline (i.e. SUM_k(nstep_k) where k is the spectral window index )
                         In case of no BIN axis nstep_k=1 and this sum is simply equal to the number of spectral windows.
      @param reverse     Parameter charcterising the order of the baselines. 

                         Let consider v_antennaId={1,2,3,4}. 
                         - With reverse=true the resulting order is (1,2) (1,3) (2,3) (1,4) (2,4) and (3,4)
                         - with reverse=false the resulting order is (1,2) (1,3) (1,4) (2,3) (2,4) and (3,4)
      @param v_uvw       The resulting sequence of uvw 'points'
      @note
      - A resulting baseline is defined using the first member of the pair as a the reference and the second 
        as relative. The pair (a,b) means  a |----> b i.e. u_ab = u_b - u_a.
      - The SDM BDF is defined with reverse=true. The current implementation of the DAM produces a sequence
        corresponding to reverse=false.
      @warning This function will destroy the previous content of v_uvw.
  */
  void uvw_bl( const vector<Tag>& v_antennaId, unsigned int nrep, bool reverse, vector<Vector<casa::Double> >& v_uvw);

};
