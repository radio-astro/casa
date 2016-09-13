#ifndef PLOTTERIMPLEMENTATIONS_H_
#define PLOTTERIMPLEMENTATIONS_H_

#include <graphics/GenericPlotter/SimplePlotter.h>

namespace casa {

// The plotterImplementation function is a global function
// within the casa namespace.  Any implementation of the
// generic plotter interface should:
// 1) add itself to the Implementation enum in the
//    graphics/implementation/Plotter.h file,
// 2) add a case statement to this function to return a
//    new instance of the factory for that implementation.
PlotFactoryPtr plotterImplementation(
        Plotter::Implementation which =  Plotter::DEFAULT,
        bool smartDelete = true);

// The simplePlotter function is a global function
// within the casa namespace.
SimplePlotterPtr simplePlotter(Plotter::Implementation which= Plotter::DEFAULT,
        bool smartDelete = true);
                               
class PlotterImplementations {                               
public:
    static casacore::String implementationName(Plotter::Implementation i);
    
    static casacore::String implementationAbout(Plotter::Implementation i,
                                      bool useHTML = true);
    
    static casacore::String implementationVersion(Plotter::Implementation i);
    
    static casacore::String implementationLink(Plotter::Implementation i,
                                     bool useHTML = true);
};

}

#endif /* PLOTTERIMPLEMENTATIONS_H_ */
