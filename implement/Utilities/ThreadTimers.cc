#include <synthesis/Utilities/ThreadTimers.h>
  
namespace casa{
  DT operator- (const Timers & tLater, const Timers & tEarlier)
  {
    return DT (tLater.elapsed() - tEarlier.elapsed(),
               tLater.cpu() - tEarlier.cpu(), 0);
  }
};
