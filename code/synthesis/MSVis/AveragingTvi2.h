#if ! defined (Msvis_AveragingTvi2_H_121211_1236)
#define Msvis_AveragingTvi2_H_121211_1236

#include <casa/aips.h>
#include <synthesis/MSVis/TransformingVi2.h>

namespace casa {

namespace vi {

class AveragingTvi2 : public TransformingVi2 {

public:

    AveragingTvi2 (ViImplementation2 * inputVii);
    ~AveragingTvi2 ();

    void originChunks ();
    void nextChunk ();
    Bool moreChunks ();

    void origin ();
    void next ();
    Bool more ();

protected:

private:


};

} // end namespace vi

} // end namespace casa

#endif // ! defined (Msvis_AveragingTvi2_H_121211_1236)
