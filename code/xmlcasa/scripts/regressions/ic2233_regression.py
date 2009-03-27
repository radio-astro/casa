import os
import time
import regression_utility as regutl

THISHOME  = "ic2233_regression_data/";
IMSIZE    = 2048;
CELLSIZE  = "4arcsec";
STOKES    = "IV";
CHSTART   = 69;
NCHAN     = 11;
PASTEP    = 360.0;
IMAGE     = "imIC2233";
ALGORITHM = "cs";
#
EPS       = 1e-6;  # Logical "zero"
#
#--------------------------------------------------------------
#
def ic2233_reg():
    os.system("rm -rf "+THISHOME);
    os.mkdir(THISHOME);

    REPOSNAME = os.environ.get('CASAPATH').split()[0]
    PREFIX    = REPOSNAME + "/data/regression/IC2233/";
    FITSFILE  = PREFIX    + "ic2233.lband.fits";
    MSFILE    = THISHOME  + "ic2233.lband.ms";
    MYIMAGE   = THISHOME  + IMAGE;

    ms.fromfits(fitsfile=FITSFILE,msfile=MSFILE);
    ms.done();

    im.open(MSFILE);
    
    im.selectvis(nchan=NCHAN,start=CHSTART,step=1,spw=0);
    im.defineimage(nx=IMSIZE,ny=IMSIZE,cellx=CELLSIZE,celly=CELLSIZE,stokes=STOKES,
                   nchan=1,start=CHSTART,step=NCHAN-1);
    im.setoptions(cache=IMSIZE*IMSIZE*4,ftmachine='pbwproject',
                  applypointingoffsets=false, dopbgriddingcorrections=true,
                  cfcachedirname=MYIMAGE+".cf", pastep=360.0);
    im.clean(algorithm='cs',niter=6000,
             model=MYIMAGE+".mod",
             image=MYIMAGE+".image",
             residual=MYIMAGE+".res");
    im.done();
#
#--------------------------------------------------------------
# The objective truth!
#
StokesIPeak         = 0.86263674;  # Jy/beam
StokesIRMS          = 0.00138958;  # Jy/beam
StokesIPeakPosWorld = '08:20:22.869, +44.40.38.993'; #J2000
StokesIPeakPos      = [942,1130,0,0]; #Pixels

StokesVPeak         = 0.00063984;  # Jy/beam
StokesVRMS          = 5.07250807e-05;  # Jy/beam
StokesVPeakPosWorld = '08:11:29.219, +45.48.26.199'; #J2000
StokesVPeakPos      = [1415,1008,1,0]; #Pixels
#
#--------------------------------------------------------------
#
try:
    startTime = time.time();
    startProc = time.clock();
    regstate = False;
    ic2233_reg();
    endTime = time.time();
    endProc = time.clock();
    #
    #--------------------------------------------------------------
    # Get the image statistics
    #
    ia.open(THISHOME+IMAGE+".image");
    ibox=ia.setboxregion(blc=[0,0,0,0],trc=[2048,2048,0,0]);
    vbox=ia.setboxregion(blc=[0,0,1,0],trc=[2048,2048,1,0]);
    
    istats=ia.statistics(region=ibox);
    vstats=ia.statistics(region=vbox);

    ia.done();
    #
    #--------------------------------------------------------------
    # The subjective truth!
    #
    import datetime
    datestring=datetime.datetime.isoformat(datetime.datetime.today())
    outfile='ic2233.'+datestring+'.log'
    logfile=open(outfile,'w')

    dIMax    = istats["max"]-StokesIPeak;
    dIRMS    = istats["rms"]-StokesIRMS;
    dIMaxPos = istats["maxpos"]-StokesIPeakPos;
    dVMax    = vstats["max"]-StokesVPeak;
    dVRMS    = vstats["rms"]-StokesVRMS;
    dVMaxPos = vstats["maxpos"]-StokesVPeakPos;

    print >>logfile, "Stokes-I Statistics:";
    print >>logfile, "-------------------------------------------------";
    print >>logfile, "Max = ",istats["max"], "  RMS = ",istats["rms"];
    print >>logfile, "MaxPos = ",istats["maxpos"];
    print >>logfile, "";
    print >>logfile, "Stokes-V Statistics:";
    print >>logfile, "-------------------------------------------------";
    print >>logfile, "Max = ",vstats["max"], "  RMS = ",vstats["rms"];
    print >>logfile, "MaxPos = ",vstats["maxpos"];
    print >>logfile, "";
    print >>logfile, "Stokes-I Delta-regression Statistics:"
    print >>logfile, "-------------------------------------------------";
    print >>logfile, "dMax = ",dIMax, "  dRMS = ",dIRMS;
    print >>logfile, "dMaxPos = ",dIMaxPos;
    print >>logfile, "";
    print >>logfile, "Stokes-V Delta-regression Statistics:"
    print >>logfile, "-------------------------------------------------";
    print >>logfile, "dMax = ",dVMax, "  dRMS = ",dVRMS;
    print >>logfile, "dMaxPos = ",dVMaxPos;
    print >>logfile,"";

    if ((abs(dIMax) < EPS) &
        (abs(dIRMS) < EPS) &
        (abs(dVMax) < EPS) &
        (abs(dVRMS) < EPS) &
        (dIMaxPos==0).all() &
        (dVMaxPos==0).all()):
        regstate=True;
        print >>logfile,"IC2233 Regression passed.";
    else:
        regstate=False;
        print >>logfile,"IC2233 Regression failed.";
        
    print >>logfile,''
    print >>logfile,''
    print >>logfile,'********* Benchmarking *****************'
    print >>logfile,'*                                      *'
    print >>logfile,'Total wall clock time was: ', endTime - startTime
    print >>logfile,'Total CPU        time was: ', endProc - startProc


    logfile.close();

except Exception, instance:
    print "###Error in ic2233 regression: ",instance;
