
#ifndef _ASAP_GIL_HANDLER_H_
#define _ASAP_GIL_HANDLER_H_

//#include <iostream>
#include <Python.h>

namespace {
  inline PyThreadState *SaveThreadState()
  {
    //std::cout << "SaveThreadState" << std::endl;
    return PyEval_SaveThread();
  }

  inline void RestoreThreadState(PyThreadState *state)
  {
    //std::cout << "RestoreThreadState" << std::endl;
    PyEval_RestoreThread(state);
  }
}

namespace asap {
  
class GILHandler {
public:
  GILHandler()
  {
#ifdef USE_CASAPY
    threadState_ = SaveThreadState();
#endif
  }

  ~GILHandler()
  {
#ifdef USE_CASAPY
    RestoreThreadState(threadState_);
    threadState_ = NULL;
#endif
  }

private:
  PyThreadState *threadState_;
};

}

#endif /* _ASAP_GIL_HANDLER_H_ */
