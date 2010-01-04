#if !defined(ATM_ENUMERATION_H)

namespace atm {

/** \file
 *  \brieg Global enumeration objects
 *  
 *  The ASDM uses the the item netSideband to describe the sidebands. It appears
 *  more convenient to use a pair of items, the side of the sideband and its type.
 *  The following two enumerations describes this.
 *
 */

/** \brief Defines the side of a sideband.
 *
 *  Heterodyne receivers have two sidebands, a lower and an upper sideband. On the
 *  other hand the detectors do not have sidebands. The Science Data Model assign
 *  one spectral window per sideband. Hence, with detectors, there is a single
 *  spectral window. SidebandSide differenciates between these various cases.<br>
 *
 *  The codes have the following meaning:
 *    <ul>
 *     <li> 0  No sideband (eg for a detector)
 *     <li> 1  Lower sideband (LSB)
 *     <li> 2  Upper sideband (USB)
 *    </ul>
 */
enum SidebandSide{ NOSB=0,         //!< No sideband (eg for HEMT)
                   LSB=1,          //!< Lower sideband (LSB)
                   USB=2           //!< Upper sideband (USB)
};


/** \brief Defines the kind of sideband separations.
 *
 *  With heterodyne receivers there are always two sidebands. In the
 *  interferometric case it is possible to separate these two sidebands.
 *  When there is no sideband separation two types are considered,
 *  single sideband (SSB) when the image sideband has been rejected
 *  and double sideband (DSB) when the siband gain ratio is about
 *  half half. In the case of DSB only one sideband is retained
 *  for the data because the other sideband has identical data.
 *  When modeling the atmospheric contribution both sidebands need to be
 *  considered because the contribution is not the same in the two
 *  sidebands. <br>
 *
 *  The codes have the following meaning:
 *   <ul>
 *     <li> -1 No type (NOTYPE): type is not relevant with a SidebandSide equal to NOSB
 *     <li> 0  Double sidebands (DSB): Case with no sideband rejection
 *     <li> 1  Single sideband (SSB): Case with sideband rejection
 *     <li> 2  Two sideband (TWOSB): Case with sideband separation (only for interferometric modes)
 *    </ul>
 *  </ul>
 */
enum SidebandType{ NOTYPE=-1,      //!< Not relevant (no sideband)
                   DSB=0,          //!< Double sideband (no rejection)
                   SSB=1,          //!< Single sideband (rejection)
                   TWOSB=2         //!< two sidebands (sideband separation)
};

}

#define ATM_ENUMERATION_H
#endif
