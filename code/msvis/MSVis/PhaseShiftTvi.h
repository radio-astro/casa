/*
 * PhaseShiftTvi.h
 *
 *  Created on: Mar 12, 2015
 *      Author: jjacobs
 */

#ifndef MSVIS_MSVIS_PHASESHIFTTVI_H_
#define MSVIS_MSVIS_PHASESHIFTTVI_H_

#include <casacore/casa/Arrays/Vector.h>
#include <msvis/MSVis/TransformingVi2.h>

namespace casa {

namespace vi {

class PhaseShiftTvi : public TransformingVi2 {

public:

    PhaseShiftTvi (VisibilityIterator2 * vi, ViImplementation2 * inputVi, double dXArcseconds,
                   double dYArcseconds);

    virtual void visibilityCorrected (Cube<Complex> & vis) const;
    virtual void visibilityModel (Cube<Complex> & vis) const;
    virtual void visibilityObserved (Cube<Complex> & vis) const;


protected:

    // Rotate visibility phase for phase center offsets (arcsecs)

    void phaseCenterShift (Cube<Complex> & cube) const;

private:

    double dXArcseconds_p;
    double dYArcseconds_p;


};

} // end namespace vi

} // end namespace casa






#endif /* MSVIS_MSVIS_PHASESHIFTTVI_H_ */
