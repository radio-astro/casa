/*
  Performance test of flagger iterations.
*/



#include <flagging/Flagging/Flagger.h>

#include <msvis/MSVis/VisSet.h>

using namespace casa;


void loop_visiter(MeasurementSet &ms)
{
    Matrix<Int> noselection;


    Block<int> sort(4);

    //sort2[0] = MS::SCAN_NUMBER;
    // Do scan priority only if quacking

    sort[0]= MS::ARRAY_ID;
    sort[1]= MS::FIELD_ID;
    sort[2]= MS::DATA_DESC_ID;
    sort[3] = MS::TIME;



    Double timeInterval = 7.0e9; //a few thousand years
    
    Bool addScratch = False;
    
    VisSet *vs_p = new VisSet(ms, sort, noselection, 
                              addScratch, timeInterval);

    // Use default sort order - and no scratch cols....
    //vs_p = new VisSet(*mssel_p,noselection,0.0);

    vs_p->resetVisIter(sort, timeInterval);
        
    VisibilityIterator &vi(vs_p->iter()); 
    
    //VisBuffer vb(vi);

    Vector<Int> scans;
    Cube<Bool> flags;

    unsigned nchunk = 0;
    for (vi.originChunks(); 
         vi.moreChunks(); 
         vi.nextChunk(), nchunk++) {
        
        unsigned ntime = 0;

        for( vi.origin(); 
             vi.more(); 
             vi++, ntime++) {

        }
        if (nchunk % 10 == 0)
        cout << "Looping chunk = " << nchunk
             << ", scan = " << vi.scan(scans)
             << ", field = " << vi.fieldId() 
             << ", spw = " << vi.spectralWindow()
             << ", ntimes = " << ntime 
             << ", dim = " << vi.flag(flags).shape()
             << endl;
    }
}



void loop_flagger(MeasurementSet &ms)
{
    Flagger flagger(ms);

    cout << "flagger.numAnt = " << flagger.numAnt() << endl;
    cout << "flagger.numIfr = " << flagger.numIfr() << endl;
    cout << "flagger.numFeed = " << flagger.numFeed() << endl;
    cout << "flagger.numFeedCorr = " << flagger.numFeedCorr() << endl;



    /*
     *  Test Flagger
     */

    cout << "Run flagger..." << endl;


    //LogIO l(LogOrigin("Flagger","FlagCube"));
    //    l << "helo l" << LogIO::POST;
    //    cerr << "survived" << endl;


    bool trial = false;
    bool reset = false;

    string baseline = "";

    baseline = "";
    flagger.setdata("", "", "", "", "",
                    baseline, "", "", ""); 
    
    cout << "setmanualflags..." << endl;
    Vector<Double> cliprange(2);
    //cliprange[0] = 0.5;
    //cliprange[1] = 1.0;

    flagger.setmanualflags(false, false, "", cliprange, "", false, false);

    cout << "run..." << endl;
    flagger.run(trial, reset);
}

int main()
{
    MeasurementSet ms("/tmp/n7538_usb.ms", Table::Update);
    //MeasurementSet ms("/tmp/field3.ms", Table::Update);
    
    loop_visiter(ms);

    loop_flagger(ms);

    return 0;
}
