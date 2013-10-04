#include <synthesis/MSVis/ViFrequencySelection.h>
#include <synthesis/MSVis/UtilJ.h>
#include <ms/MeasurementSets/MSSelection.h>

#include <utility>

using namespace std;

namespace casa { //# NAMESPACE CASA - BEGIN

namespace vi {

void
FrequencySelection::addCorrelationSlices (const Vector <Vector <Slice> > & slices)
{
    correlationSlices_p = slices;
}

void
FrequencySelection::filterByWindow (Int windowId) const
{
    filterWindowId_p = windowId;
}

Int
FrequencySelection::filterWindow() const
{
    return filterWindowId_p;
}

String
FrequencySelection::frameName (Int referenceFrame)
{
    String result;

    if (referenceFrame >= 0 && referenceFrame < MFrequency::N_Types){

        result = MFrequency::showType (referenceFrame);
    }
    else if (referenceFrame == ByChannel){
    }
    else{

        ThrowIf (True, String::format ("Unknown frame of reference: id=%d", referenceFrame));
    }

    return result;
}

Vector <Slice>
FrequencySelection::getCorrelationSlices (Int polarizationId) const
{
    if (correlationSlices_p.nelements() == 0){
        return Vector<Slice> (); // empty vector implies all elements
    }

    Assert (polarizationId >= 0 && polarizationId < (int) correlationSlices_p.nelements());

    return correlationSlices_p [polarizationId];
}

Int
FrequencySelection::getFrameOfReference () const
{
    return referenceFrame_p;
}

//set<Int>
//FrequencySelections::getSpectralWindowSelection (Int msIndex) const
//{
//    set<Int> result;
//
//    for (SelectedWindows::const_iterator i = selectedWindows_p.begin();
//            i != selectedWindows_p.end();
//            i ++){
//
//        if (i->first != msIndex){
//            continue;
//        }
//
//        result.insert (i->second);
//    }
//
//    return result;
//}


void
FrequencySelectionUsingChannels::add (Int spectralWindow, Int firstChannel,
                                      Int nChannels, Int increment)
{
    Assert (spectralWindow >= 0);
    Assert (firstChannel >= 0);
    Assert (nChannels > 0);
    Assert (increment != 0 || nChannels == 1);

    elements_p.push_back (Element (spectralWindow, firstChannel, nChannels, increment));
}

void
FrequencySelectionUsingChannels::add (const MSSelection & msSelection)
{
    // Meanings of columns in the "matrix" (actually used as a parallel array)

    enum {SpectralWindowId, FirstChannel, StopChannel, Step};

    Matrix<Int> channelList = const_cast <MSSelection &> (msSelection).getChanList();

    for (Int row = 0; row < (Int) channelList.nrow(); row ++){

        Int nChannels = channelList (row, StopChannel) - channelList (row, Step);
        nChannels = nChannels / channelList (row, Step) + 1;

        add (channelList (row, SpectralWindowId),
             channelList (row, FirstChannel),
             nChannels,
             channelList (row, Step));
    }
}


FrequencySelectionUsingChannels::const_iterator
FrequencySelectionUsingChannels::begin () const
{
    filtered_p.clear();

    for (Elements::const_iterator i = elements_p.begin();
         i != elements_p.end();
         i ++){

        if (filterWindow () < 0 || i->spectralWindow_p == filterWindow()){
            filtered_p.push_back (* i);
        }
    }

    return filtered_p.begin();
}


FrequencySelection *
FrequencySelectionUsingChannels::clone () const
{
    return new FrequencySelectionUsingChannels (* this);
}

Bool
FrequencySelectionUsingChannels::empty () const
{
    return elements_p.empty();
}


FrequencySelectionUsingChannels::const_iterator
FrequencySelectionUsingChannels::end () const
{
    return filtered_p.end();
}

set<int>
FrequencySelectionUsingChannels::getSelectedWindows () const
{
    set <int> result;
    for (Elements::const_iterator i = elements_p.begin();
         i != elements_p.end();
         i ++){

        result.insert (i->spectralWindow_p);

    }

    return result;
}

Int
FrequencySelectionUsingChannels::getNChannels (Int spectralWindowId) const
{

    Int result = 0;

    if (elements_p.empty()){

    }
    else {

        for (Elements::const_iterator i = elements_p.begin();
                i != elements_p.end();
                i ++){

            if (i->spectralWindow_p == spectralWindowId){
                result += i->nChannels_p;
            }
        }
    }
    return result;
}



String
FrequencySelectionUsingChannels::toString () const
{
    String s = String::format ("{frame='%s' {", frameName (getFrameOfReference()).c_str());

    for (Elements::const_iterator e = elements_p.begin();
         e != elements_p.end();
         e ++){

        s += String::format ("(spw=%d, 1st=%d, n=%d, inc=%d)",
                            e->spectralWindow_p,
                            e->firstChannel_p,
                            e->nChannels_p,
                            e->increment_p);
    }
    s += "}}";

    return s;
}

FrequencySelectionUsingFrame::FrequencySelectionUsingFrame (MFrequency::Types frameOfReference)
: FrequencySelection (frameOfReference)
{}

void
FrequencySelectionUsingFrame::add (Int spectralWindow, Double bottomFrequency,
                                   Double topFrequency)
{
    elements_p.push_back (Element (spectralWindow, bottomFrequency, topFrequency));
}

//void
//FrequencySelectionUsingFrame::add (Int spectralWindow, Double bottomFrequency,
//                                   Double topFrequency, Double increment)
//{
//    elements_p.push_back (Elements (spectralWindow, bottomFrequency, topFrequency, increment));
//}


FrequencySelectionUsingFrame::const_iterator
FrequencySelectionUsingFrame::begin () const
{
    filtered_p.clear();

    for (Elements::const_iterator i = elements_p.begin();
         i != elements_p.end();
         i ++){

        if (filterWindow () < 0 || i->spectralWindow_p == filterWindow()){
            filtered_p.push_back (* i);
        }
    }

    return filtered_p.begin();
}

FrequencySelection *
FrequencySelectionUsingFrame::clone () const
{
    return new FrequencySelectionUsingFrame (* this);
}

Bool
FrequencySelectionUsingFrame::empty () const
{
    return elements_p.empty();
}


FrequencySelectionUsingFrame::const_iterator
FrequencySelectionUsingFrame::end () const
{
    return filtered_p.end();
}

//Int
//FrequencySelectionUsingFrame::getNChannels (Int spectralWindowId) const
//{
//
//    Int result = 0;
//
//    if (elements_p.empty()){
//
//    }
//    else {
//
//        for (Elements::const_iterator i = elements_p.begin();
//                i != elements_p.end();
//                i ++){
//
//            if (i->spectralWindow_p == spectralWindowId){
//                result += i->nChannels_p;
//            }
//        }
//    }
//    return result;
//}


set<int>
FrequencySelectionUsingFrame::getSelectedWindows () const
{
    set<int> result;
    for (Elements::const_iterator i = elements_p.begin();
         i != elements_p.end();
         i ++){

        result.insert (i->spectralWindow_p);

    }

    return result;
}

Double
FrequencySelectionUsingFrame::Element::getBeginFrequency () const
{
    return beginFrequency_p;
}

Double
FrequencySelectionUsingFrame::Element::getEndFrequency () const
{
    return endFrequency_p;
}

String
FrequencySelectionUsingFrame::toString () const
{
    String s = String::format ("{frame='%s' {", frameName (getFrameOfReference()).c_str());

    for (Elements::const_iterator e = elements_p.begin();
         e != elements_p.end();
         e ++){

        s += String::format ("(spw=%d, 1st=%g, n=%g, inc=%g)",
                            e->spectralWindow_p,
                            e->beginFrequency_p,
                            e->endFrequency_p,
                            e->increment_p);
    }

    s += "}}";

    return s;
}

FrequencySelections::FrequencySelections ()
{}

FrequencySelections::FrequencySelections (const FrequencySelections & other)
{
    for (Selections::const_iterator s = other.selections_p.begin();
         s != other.selections_p.end(); s++){
        selections_p.push_back ((* s)->clone());
    }

    filterWindow_p = other.filterWindow_p;
    selectedWindows_p = other.selectedWindows_p;
}

FrequencySelections::~FrequencySelections ()
{
    for (Selections::const_iterator s = selections_p.begin();
         s != selections_p.end(); s++){
        delete (* s);
    }
}

void
FrequencySelections::add (const FrequencySelection & selection)
{
    if (! selections_p.empty()){
        ThrowIf (getFrameOfReference() != selection.getFrameOfReference(),
                 String::format ("Frequency selection #%d has incompatible frame of reference %d:%s "
                                "(!= %d:%s)",
                                selections_p.size() + 1,
                                selection.getFrameOfReference(),
                                FrequencySelection::frameName (selection.getFrameOfReference()).c_str(),
                                getFrameOfReference(),
                                FrequencySelection::frameName (getFrameOfReference()).c_str()));
    }

    selections_p.push_back (selection.clone());
    Int msIndex = selections_p.size() - 1;
    set<int> windows = selection.getSelectedWindows();

    for (set<int>::const_iterator w = windows.begin(); w != windows.end(); w++){
        selectedWindows_p.insert (make_pair (msIndex, * w));
    }

}

FrequencySelections *
FrequencySelections::clone () const
{
    return new FrequencySelections (* this);
}

const FrequencySelection &
FrequencySelections::get (Int msIndex) const
{
    if (selections_p.empty()){
        return defaultSelection_p;
    }

    ThrowIf (msIndex < 0 || msIndex >= (int) selections_p.size(),
             String::format ("MS index, %d, out of bounds [0,%d]", msIndex, selections_p.size() - 1));

    return * selections_p [msIndex];
}


Int
FrequencySelections::getFrameOfReference () const
{
    if (selections_p.empty()){
        return FrequencySelection::ByChannel;
    }
    else {
        return selections_p.front()->getFrameOfReference();
    }
}

Bool
FrequencySelections::isSpectralWindowSelected (Int msIndex, Int spectralWindowId) const
{
    // Empty selections means everything is selected

    if (selections_p.empty()){
        return True;
    }

    Assert (msIndex >= 0 && msIndex < (int) selections_p.size() && selections_p [msIndex] != 0);

    if (selections_p [msIndex]->empty()){
        return True;
    }

    SelectedWindows::const_iterator swi =
        selectedWindows_p.find (make_pair (msIndex, spectralWindowId));

    return swi != selectedWindows_p.end();
}


Int
FrequencySelections::size () const
{
    return (Int) selections_p.size();
}

} // end namespace vi

} // end namespace casa
