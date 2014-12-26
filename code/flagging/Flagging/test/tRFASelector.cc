#include <flagging/Flagging/RFASelector.h>

#include <msvis/MSVis/VisSet.h>
#include <measures/Measures/Stokes.h>

#include <tables/DataMan/TiledShapeStMan.h>

#include <casa/aips.h>
#include <casa/namespace.h>
#include <cassert>


MeasurementSet create_ms()
{
    // Create table
    TableDesc simpleDesc = MS::requiredTableDesc();

    MS::addColumnToDesc(simpleDesc, MS::DATA, 2);
    const Vector<String> coordCols(0);
    const Vector<String> idCols(0);
    simpleDesc.defineHypercolumn("TiledData", 
                                 3, stringToVector(MS::columnName(MS::DATA)),
                                 coordCols, idCols);
    simpleDesc.defineHypercolumn("TiledFlag", 
                                 3, stringToVector(MS::columnName(MS::FLAG)));

    TiledShapeStMan dataStMan("TiledData", IPosition(3, 1, 1, 100));
    TiledShapeStMan flagStMan("TiledFlag", IPosition(3, 1, 1, 100));


    SetupNewTable newTab("simpleTab", simpleDesc, Table::New);
    newTab.bindColumn(MS::columnName(MS::DATA), dataStMan);
    newTab.bindColumn(MS::columnName(MS::FLAG), flagStMan);

    // Create MS
    MeasurementSet ms(newTab);

    // Subtables
    ms.createDefaultSubtables(Table::New);

    // MAIN
    ms.addRow();
    ms.addRow();
    ms.addRow();

    // MAIN - DATA, FLAG
    {
        ArrayColumn<Complex> data(ms, MS::columnName(MS::DATA));
        ArrayColumn<Bool>    flag(ms, MS::columnName(MS::FLAG));
        Matrix<Complex> visibility(2, 63);
        Matrix<Bool> flags(2, 63);
        data.put(0, visibility);
        data.put(1, visibility);
        data.put(2, visibility);
        flag.put(0, flags);
        flag.put(1, flags);
        flag.put(2, flags);
    }

    // MAIN - FEED1
    TableColumn feed(ms, MS::columnName(MS::FEED1));
    unsigned rownr = 0;
    feed.putScalar(rownr, 1);


    // MAIN - antenna
    {
        casa::MSMainColumns(ms).antenna1().put(0, 0);
        casa::MSMainColumns(ms).antenna1().put(1, 0);
        casa::MSMainColumns(ms).antenna1().put(2, 1);
        casa::MSMainColumns(ms).antenna2().put(0, 1);
        casa::MSMainColumns(ms).antenna2().put(1, 2);
        casa::MSMainColumns(ms).antenna2().put(2, 2);
    }


    // Antenna
    ArrayColumn<Double> antpos(ms.antenna(),
                               MSAntenna::columnName(MSAntenna::POSITION));
    ms.antenna().addRow();
    ms.antenna().addRow();
    ms.antenna().addRow();

    Array<Double> position(IPosition(1, 3)); 
    //position(0)=1.;     position(1)=2.;     position(2)=3.;
    antpos.put(0, position);

    // DATA_DESCRIPTION
    unsigned spectral_window_id = 0;
    unsigned polarization_id = 0;
    unsigned crow;
    MSDataDescription dd = ms.dataDescription();
    MSDataDescColumns ddCol(dd);
    
    crow = dd.nrow();
    dd.addRow();
    
    ddCol.spectralWindowId().put(crow, spectral_window_id);
    ddCol.polarizationId().put(crow, polarization_id);
    ddCol.flagRow().put(crow, False);
    
    // POLARIZATION
    MSPolarization pol = ms.polarization();
    pol.addRow();

    unsigned num_corr = 2;
    //Stokes::StokesTypes corr_type[2];
    int corr_type[2];
    corr_type[0] = 0;
    corr_type[1] = 1;
    Vector<Int> corrType(IPosition(1, num_corr), (int *)corr_type);
    Matrix<Int> corrProduct(2, num_corr);


    MSPolarizationColumns mspolarCol(ms.polarization());
    mspolarCol.numCorr().put(crow, num_corr);
    mspolarCol.corrType().put(crow,corrType);
    mspolarCol.corrProduct().put(crow, corrProduct);


    // FEED
    {
        ms.feed().addRow();
        MSFeedColumns msfeedCol(ms.feed());
        unsigned num_receptors = 2;

        Vector<Double>   receptorAngle(num_receptors);
        Vector<String>   polarizationType(num_receptors);
        Matrix<Double>   beamOffset(2, num_receptors);
        Matrix<Complex>  polResponse(num_receptors,
                                     num_receptors);


        polarizationType(0) = "R";
        polarizationType(1) = "L";
        receptorAngle(0) = 23.5;
        receptorAngle(1) = 22.5;

        msfeedCol.polarizationType().put(crow, polarizationType);
        msfeedCol.receptorAngle().put(crow, receptorAngle); 
        msfeedCol.beamOffset().put(crow, beamOffset);
        msfeedCol.polResponse().put(crow, polResponse);

    }
    
    // FIELD
    {
        crow = ms.field().nrow();

        ms.field().addRow();

        MSFieldColumns msfieldCol(ms.field());

        Matrix<Double>  delayDir(2,1);
        Matrix<Double>  referenceDir(2,1);
        Matrix<Double>  phaseDir(2,1);

        msfieldCol.delayDir().put(crow, delayDir);
        msfieldCol.phaseDir().put(crow, phaseDir);
        msfieldCol.referenceDir().put(crow, referenceDir);
    }


    // SPECTRAL_WINDOW
    {
        crow = ms.spectralWindow().nrow();
        ms.spectralWindow().addRow();

        MSSpWindowColumns msspwinCol(ms.spectralWindow());

        unsigned num_chan = 63;
        Vector<Double> chanFreq(num_chan);
        Vector<Double> chanWidth(num_chan);
        Vector<Double> effectiveBW(num_chan);
        Vector<Double> resolution(num_chan);
  
        double f0 = 1444404;
        double df = 10000;

        msspwinCol.numChan().put(crow, num_chan);
        msspwinCol.name().put(crow, String("a sprectral window"));
        msspwinCol.refFrequency().put(crow, f0);
  
        for (unsigned i=0; i < num_chan; i++) {
            chanFreq(i)    = f0 + i * df;//chan_freq_[i];
            chanWidth(i)   = df; //chan_width_[i];
            effectiveBW(i) = df; //effective_bw_[i];
            resolution(i)  = 1000*1000; //resolution_[i];
        }

        msspwinCol.chanFreq().put(crow, chanFreq);
        msspwinCol.chanWidth().put(crow, chanWidth);
        msspwinCol.effectiveBW().put(crow, effectiveBW);
        msspwinCol.resolution().put(crow, resolution);
        msspwinCol.measFreqRef().put(crow, 0);
        msspwinCol.totalBandwidth().put(crow, num_chan * df);
        msspwinCol.netSideband().put(crow, 1);
        //if (bbc_no_ >= 0) msspwinCol.bbcNo().put(crow, bbc_no_);
        msspwinCol.ifConvChain().put(crow, 0);
        msspwinCol.freqGroup().put(crow, 0);
        msspwinCol.freqGroupName().put(crow, "A frequency group");

        msspwinCol.flagRow().put(crow, False);
        
    }    

    return ms;
}


int run()
{
    /*
      First work a bit to get an RFASelector object,
      so that we can test its methods
     */
    Float vmin, vmax;
    uInt f0(1);
    Record spec, parm;


    Block<Int> sortcol(0);
    MeasurementSet ms = create_ms();

    cout << "Created MS" << endl;

    VisibilityIterator vi(ms, sortcol);
    VisBuffer vb(vi);

    Flagger flagger(ms);

    cout << "flagger.numAnt = " << flagger.numAnt() << endl;
    cout << "flagger.numIfr = " << flagger.numIfr() << endl;
    cout << "flagger.numFeed = " << flagger.numFeed() << endl;
    cout << "flagger.numFeedCorr = " << flagger.numFeedCorr() << endl;


    RFChunkStats chunk(vi, vb, flagger);
    
    cout << "Parameters are " << parm  << endl;


    RFASelector s(chunk, parm); 
    
    cout << "Created RFASelector" << endl;
    
    /*
     *   test parseMinMax
     */

    // cout << "Calling parseMinMax with: " << vmin << ", " << vmax << ", " << spec << ", " << f0 << endl;
    assert( !s.fortestingonly_parseMinMax( vmin, vmax, spec, f0) );

    // longer record
    spec.define("name", "Selector");
    spec.define("spwid", False);
    spec.define("field", False);
    spec.define("fq", False);
    spec.define("chan", False);
    spec.define("corr", False);
    spec.define("ant", False);
    spec.define("baseline", False);
    spec.define("timerng", False);
    spec.define("autocorr", False);

    Record clip;
    clip.define(RF_EXPR, "ABS RR");
    clip.define(RF_MIN, 0.4);
    clip.define(RF_MAX, 0.5);
    clip.define(RF_CHANAVG, false);

    spec.defineRecord("clip", clip);

    // cout << "Calling parseMinMax with: " << vmin << ", " << vmax << ", " << clip << ", " << f0 << endl;

    assert( s.fortestingonly_parseMinMax( vmin, vmax, clip, f0) );

    //cout << "vmin: " << vmin << endl;
    //cout << "vmax: " << vmax << endl;

    Double epsilon(0.001);

    assert( fabs(vmin - 0.4) < epsilon );
    assert( fabs(vmax - 0.5) < epsilon );

    // test parseClipField
    Bool c = True;
    // cout << "Calling parseClipField with: " << spec << ", " << c << endl;

    s.fortestingonly_parseClipField(clip, c);



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
    cliprange[0] = 0.5;
    cliprange[1] = 1.0;

    flagger.setmanualflags(false, false, "", cliprange, "",
                           false, false);

    cout << "run..." << endl;
    flagger.run(trial, reset);

    return 0;
}



int main ()
{
    run();
}
