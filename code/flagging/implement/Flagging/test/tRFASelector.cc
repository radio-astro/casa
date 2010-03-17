#include <flagging/Flagging/RFASelector.h>

#include <msvis/MSVis/VisSet.h>
#include <measures/Measures/Stokes.h>

#include <tables/Tables/TiledShapeStMan.h>

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
    String colData;
    colData = MS::columnName(MS::DATA);
    simpleDesc.defineHypercolumn("TiledData", 
                                 3, stringToVector(colData),
                                 coordCols, idCols);
    TiledShapeStMan dataStMan("TiledData", IPosition(3, 1, 1, 100));

    SetupNewTable newTab("simpleTab", simpleDesc, Table::New);
    newTab.bindColumn(colData, dataStMan);

    // Create MS
    MeasurementSet ms(newTab);

    // Subtables
    ms.createDefaultSubtables(Table::New);

    // MAIN
    ms.addRow();
    ms.addRow();
    ms.addRow();

    // MAIN - DATA
    {
        ArrayColumn<Complex> data(ms, colData);
        Matrix<Complex> visibility(2, 63);
        data.put(0, visibility);
        data.put(1, visibility);
        data.put(2, visibility);
    }

    // MAIN - FEED1
    TableColumn feed(ms, MS::columnName(MS::FEED1));
    unsigned rownr = 0;
    feed.putScalar(rownr, 1);


    // Antenna
    ArrayColumn<Double> antpos(ms.antenna(),
                               MSAntenna::columnName(MSAntenna::POSITION));
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
        ms.spectralWindow().addRow();
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

    PGPlotterInterface *pgp_screen(NULL);
    PGPlotterInterface *pgp_report(NULL);


    RFChunkStats chunk(vi, vb, flagger, pgp_screen, pgp_report);

    cout << "Parameters are " << parm  << endl;

    RFASelector s(chunk, parm); 
    
    cout << "Created RFASelector" << endl;
    
    // test parseMinMax

    // empty record
    cout << "Calling parseMinMax with: " << vmin << ", " << vmax << ", " << spec << ", " << f0 << endl;
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

    cout << "Calling parseMinMax with: " << vmin << ", " << vmax << ", " << clip << ", " << f0 << endl;

    assert( s.fortestingonly_parseMinMax( vmin, vmax, clip, f0) );

    cout << "vmin: " << vmin << endl;
    cout << "vmax: " << vmax << endl;

    Double epsilon(0.001);

    assert( fabs(vmin - 0.4) < epsilon );
    assert( fabs(vmax - 0.5) < epsilon );

    // test parseClipField
    Bool c = True;
    cout << "Calling parseClipField with: " << spec << ", " << c << endl;

    s.fortestingonly_parseClipField(clip, c);

    return 0;
}



int main ()
{
    run();
    try{
        run();
    }
    catch (AipsError &x) {
        cerr << x.getMesg() << endl;
        throw;
    }
    catch (...) {
        cerr << "Unknown exception caught!" << endl;
        throw;
    }
}
