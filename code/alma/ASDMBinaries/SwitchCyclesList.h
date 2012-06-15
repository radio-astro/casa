#if     !defined(_SWITCHCYCLESLIST_H) 

#include <vector>
#include <map>

#include "ASDMEntities.h"

#include <BasebandName.h>

using namespace std;
using namespace asdm;

namespace sdmbin {

  /** Definition of the root of the tree hierarchy */
  class SwitchCyclesList
  {
  public:

    /** Accessor on the array of SwitchCycle identifiers. 
     * @return A flat vector of SwitchCycle identifiers of asdm type Tag. Its size 
     * corresponds to the number of dataDescriptions used for the setup of the
     * configuration.
     */
    vector<Tag>  getSwitchCyclesList(); 

    /** Accessor on the array of SwitchCycle identifiers given a named baseband. 
     * @return A vector of SwitchCycle identifiers of asdm type Tag. Its size 
     * corresponds to the number of spectral windows in that baseband which has
     * been used for the configuration setup.
     */
    vector<Tag>  getSwitchCyclesList(BasebandName basebandName); 

    /** Accessor on the number of steps for the SwitchCycles for every of the 
     *  dataDesription in the list in configDescription.
     * @param basebandName baseband name
     */
    vector<int>  getNumBin();

    /** Accessor on the number of steps for the SwitchCycles given a named
     * basebaband.
     * @param basebandName baseband name
     */
    vector<int>  getNumBin(BasebandName basebandName);

    /** Copy constructor
     */
    SwitchCyclesList(const SwitchCyclesList &);

    virtual ~SwitchCyclesList();

  protected:

    map<BasebandName,vector<Tag> > m_bn_v_scId_;   //<! map of vectors of SwitchCycle identifiers, one vector per named baseband
    map<BasebandName,vector<int> > m_bn_v_numBin_; //<! number of steps for every spw in every baseband (size numBaseband)
    vector<vector<int> >           vv_numBin_;     //<! number of steps for every spw in every baseband (size numBaseband)
    vector<int>                    v_numBin_;      //<! number of steps in every datadescription in the sequence in configDescription
    ASDM*                          datasetPtr_;    //<! pointer on an ASDM dataset.

    SwitchCyclesList();

    /** Class to define the root of the tree hierarchy 
     * @pre   The asdm dataset to work with is known.
     * @param datasetPtr A pointer on an ASDM dataset
     * @param v_switchCycleId An array of switchCycleId identifiers, one per DataDescription.
     * @post The number of steps in switch cycles for every spectral window is known 
     */
    SwitchCyclesList(ASDM* const datasetPtr, vector<Tag> v_switchCycleId);

    /** Class to define the root of the tree hierarchy 
     * @pre   The asdm dataset to work with is known.
     * @param datasetPtr A pointer on an ASDM dataset
     * @param vv_switchCycleId An array of switchCycleId identifiers, one vector per baseband.
     * @post The number of steps in switch cycles known for every baseband 
     */
    SwitchCyclesList(ASDM* const datasetPtr, vector<vector<Tag> > vv_switchCycleId);

    /** Class to define the root of the tree hierarchy
     * @pre   The asdm dataset to work with is known.
     * @param datasetPtr A pointer on an ASDM dataset
     * @param v_switchCycleId An array of switchCycleId identifiers, one per baseband.
     * @post  The number of steps in switch cycles known for every baseband
     * \note Use the alternate constructor, SwitchCyclesList(ASDM* datasetPtr, vector<vector<Tag> > vv_switchCycleId),
     *       in the more conventional case ,the SwitchCycle identifiers being expressed using the asdm type Tag.
     */
    SwitchCyclesList(ASDM* const datasetPtr, vector<vector<int> > vv_switchCycleId);

    /** SwitchCyclesList defines the abstract base type of a hierarchy
     * \note there are various possible levels of constraints for the number of bins. For example, from the highest
     * to the lowest constraint:
     * <ol> <li> With a nutating sub-reflector the number of bins must be the same in all the spectral windows,
     *           whatever the baseband. </li>
     *      <li> With frequency-switch observing modes, the number of bins may require to be unique on a baseband
     *           basis. </li>
     *      <li> With dedispersion for pulsar observations the number of bins may be solely spectral-window based.</li>
     * </ol>
     * mapScToBaseband(), implemented in the derived class DataDescriptionSets, maps vectors to baseband names to
     * support this, the baseband names being attributes in the definition of the spectral windows. 
     */
    virtual void mapScToBaseband() = 0;

    vector<vector<Tag> > vv_switchCycleId_ ; //<! list of vector of SwitchCycle identifiers for the sequence of basebands
    vector<Tag>           v_switchCycleId_ ; //<! list of SwitchCycle id (would they be spw indepedent in a bb) for the sequence of basebands
  };
}

#define _SWITCHCYCLESLIST_H
#endif
