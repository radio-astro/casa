#include <casaqt/PlotterImplementations/PlotterImplementations.h>

#ifdef AIPS_HAS_QWT
#include <casaqt/QwtPlotter/QPFactory.h>
#include <qwt_global.h>
#endif

#include <casa/namespace.h>

namespace casa {

PlotFactoryPtr plotterImplementation(Plotter::Implementation which) {
    switch(which) {
#ifdef AIPS_HAS_QWT
    case Plotter::QWT: return PlotFactoryPtr(new QPFactory());
#endif

    default: return PlotFactoryPtr();
    }
}

SimplePlotterPtr simplePlotter(Plotter::Implementation which) {
    return SimplePlotterPtr(new SimplePlotter(plotterImplementation(which)));
}


///////////////////////////////////////
// PLOTTERIMPLEMENTATION DEFINITIONS //
///////////////////////////////////////

String PlotterImplementations::implementationName(Plotter::Implementation i) {
    switch(i) {
    case Plotter::QWT: return "QWT";
    default: return "?";
    }
}

String PlotterImplementations::implementationAbout(Plotter::Implementation i,
                                                   bool useHTML) {
    stringstream ss;
    switch(i) {
    case Plotter::QWT:
        ss << "QWT (";
        if(useHTML) ss << "<b>";
        ss << "Q";
        if(useHTML) ss << "</b>";
        ss << "t ";
        if(useHTML) ss << "<b>";
        ss << "W";
        if(useHTML) ss << "</b>";
        ss << "idgets for ";
        if(useHTML) ss << "<b>";
        ss << "T";
        if(useHTML) ss << "</b>";
        ss << "echnical applications) is a library that provides plotting";
        ss << " functionality on top of the Qt framework.";
        break;
            
    default: break;
    }
    return ss.str();
}

String
PlotterImplementations::implementationVersion(Plotter::Implementation i) {
    switch(i) {
#ifdef AIPS_HAS_QWT
    case Plotter::QWT: return QWT_VERSION_STR;
#endif
        
    default: return "?";
    }
}

String PlotterImplementations::implementationLink(Plotter::Implementation i,
                                                  bool useHTML) {
    stringstream ss;
    switch(i) {
    case Plotter::QWT:
        ss << "Qwt Homepage: ";
        if(useHTML) ss << "<a href=\"http://qwt.sourceforge.net/\">";
        ss << "http://qwt.sourceforge.net/";
        if(useHTML) ss << "</a>";
        break;
            
    default: break;
    }
    return ss.str();
}

}
